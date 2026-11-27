/*
 * GAME.EXE: KF2's cd_memory.c (the same functions in the same order): VRAM
 * image streaming (cd_map_stream_read queues an archive entry as an image
 * request; cd_request_service_stream uploads each 16-sector part and asks
 * for the next, or restarts the read when a part's RECT header is damaged),
 * memory helpers (word and halfword copies and fills, a block arena with
 * cached blocks that an allocation may reclaim and a compaction that moves
 * used blocks down, a block's header bytes, malloc/free wrappers), frame
 * pacing and CD access (the root counter 3 event counts VSyncs; the CD event
 * handlers walk a ring of read requests: whole reads checked by a word
 * checksum, sector streams, sound banks and VRAM images; synchronous reads,
 * BCD sector positions and the archive file table). Its unit is -G8: the
 * "\CD\" and ";1" path literals are .sdata, while the CD state is one
 * aggregate reached absolutely.
 */
/* The CD state's VSync counter is volatile only here (see game/cd.h). */
#define GAME_CD_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <psyq/kernel.h>
#include <game/cd.h>
#include <game/image.h>
#include <MALLOC.H>
#include <game/memory.h>
#include <psyq/libc.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/funcs.h>

/* One part: 16 sectors. */
#define IMAGE_PART_SECTORS 16
#define IMAGE_PART_BYTES 0x8000

/* RECT.x of the header that ends the image. */
#define IMAGE_END 0xffff

void cd_stream_mark_complete(CdRequest *request);
void cd_stream_limit_chunk(CdRequest *request);

ADDRESS(0x80018c54, 0xc)
void cd_stream_mark_complete(CdRequest *request)
{
    request->state = 1;
}

ADDRESS(0x80018c60, 0x34)
void cd_map_stream_read(int file_in, int entry_in)
{
    u16 file = file_in;
    u16 entry = entry_in;

    cd_archive_read_chunked(file, entry, stream_work_buffer, (CdCallback)cd_stream_mark_complete);
}

ADDRESS(0x80018c94, 0x3c)
void cd_stream_limit_chunk(CdRequest *request)
{
    if (request->u.image.remaining > IMAGE_PART_SECTORS) {
        request->count = IMAGE_PART_SECTORS;
        request->u.image.remaining -= IMAGE_PART_SECTORS;
    } else {
        request->count = request->u.image.remaining;
        request->u.image.remaining = 0;
    }
}

/*
 * Uploads the image records streamed into the read request's buffer: each
 * 16-byte header (RECT twice) is followed by its pixels; a record cut by the
 * end of the part is uploaded by whole rows and finished from the next part.
 */
ADDRESS(0x80018cd0, 0x28c)
void cd_request_service_stream(void)
{
    CdRequest *request;
    u16 *header;
    s32 done;
    CdlLOC *pos;

    EnterCriticalSection();
    request = cdrom_ctl.read;
    if (request->kind != CD_REQUEST_IMAGE || request->state != 1) {
        goto leave;
    }
    done = 0;
    header = (u16 *)request->buffer;
    request->state = 0;
    ExitCriticalSection();

    for (;;) {
        s32 left = IMAGE_PART_BYTES - done;
        s16 height = request->u.image.rect.h;

        if (height != 0) {
            s32 width = request->u.image.rect.w;
            s32 bytes = (height * width) << 1;

            if (left < bytes) {
                s32 rows = (left / width) >> 1;

                if ((s16)rows != 0) {
                    request->u.image.rect.h = rows;
                    LoadImage(&request->u.image.rect, (u_long *)header);
                    request->u.image.rect.h = height - rows;
                    request->u.image.rect.y = rows + request->u.image.rect.y;
                }
                break;
            }
            request->u.image.rect.h = height;
            LoadImage(&request->u.image.rect, (u_long *)header);
            request->u.image.rect.h = 0;
            done += bytes;
            header = (u16 *)((u8 *)header + bytes);
            continue;
        }

        if ((u32)left < 16) {
            break;
        }
        if (header[0] != header[4] || header[1] != header[5] || header[2] != header[6]
            || header[3] != header[7] || header[2] == 0 || header[3] == 0) {
            request->u.image.rect.h = 0;
            request->state = 0;
            request->u.image.remaining = request->u.image.sectors;
            request->pos = request->retry_pos;
            cd_stream_limit_chunk(request);
            request->issued = 0;
            CdSeekL(&request->pos);
            return;
        }
        done += 16;
        if (header[0] == IMAGE_END) {
            DrawSync(0);
            request->count = 0;
            cd_request_advance(request);
            return;
        }
        setRECT(&request->u.image.rect, header[0], header[1], header[2], header[3]);
        header += 8;
    }

    pos = &request->pos;
    cd_location_add(pos, request->count, pos);
    cd_stream_limit_chunk(request);
    request->issued = 0;
    DrawSync(0);
    CdSeekL(pos);
    return;

leave:
    ExitCriticalSection();
}

/* Main RAM: 2 MiB from 0x80000000. */
#define RAM_BASE 0x80000000
#define RAM_SIZE 0x200000

/* A free remainder smaller than this stays inside the allocated block. */
#define HEAP_SPLIT_MIN 2060

void memory_arena_coalesce_free(HeapBlock *block);
void memory_arena_free(HeapBlock *block);
HeapBlock *memory_arena_find_block(HeapBlock *arena, u32 size);
void memory_arena_wait_pending(HeapBlock *arena);
void memory_arena_compact(HeapBlock *arena);

ADDRESS(0x80018f5c, 0x30)
u32 *resource_copy_words(u32 *dst, u32 *src, s32 count)
{
    while (count--) {
        *dst++ = *src++;
    }
    return src;
}

ADDRESS(0x80018f8c, 0x30)
u16 *resource_copy_halfwords(u16 *dst, u16 *src, s32 count)
{
    while (count--) {
        *dst++ = *src++;
    }
    return src;
}

ADDRESS(0x80018fbc, 0x24)
u32 *repeat_store_word(u32 *dst, u32 value, s32 count)
{
    while (count--) {
        *dst++ = value;
    }
    return dst;
}

ADDRESS(0x80018fe0, 0x24)
u16 *repeat_store_halfword(u16 *dst, u16 value, s32 count)
{
    while (count--) {
        *dst++ = value;
    }
    return dst;
}

ADDRESS(0x80019004, 0x84)
void memory_arena_coalesce_free(HeapBlock *block)
{
    HeapBlock *next = block;

    while (next->state != HEAP_END) {
        if (next->state == HEAP_FREE) {
            next = HEAP_NEXT(next);
            if (next->state != HEAP_FREE) {
                return;
            }
            block->size += next->size + sizeof(HeapBlock);
        } else {
            next = HEAP_NEXT(next);
        }
    }
}

ADDRESS(0x80019088, 0x20)
void memory_arena_free(HeapBlock *block)
{
    block->state = HEAP_FREE;
    if (block->owner) {
        *block->owner = 0;
        block->owner = 0;
    }
}

ADDRESS(0x800190a8, 0x150)
HeapBlock *memory_arena_find_block(HeapBlock *arena, u32 size)
{
    HeapBlock *block = arena;
    HeapBlock *start;

    while (block->state != HEAP_END) {
        if (block->state == HEAP_FREE && block->size >= size) {
            return block;
        }
        block = HEAP_NEXT(block);
    }

    block = arena;
    while (block->state != HEAP_END) {
        start = block;
        if (block->state == HEAP_CACHED) {
            while (block->state == HEAP_CACHED) {
                block = HEAP_NEXT(block);
            }
            if ((u32)((u8 *)block - (u8 *)start - sizeof(HeapBlock)) >= size) {
                block = start;
                while (block->state == HEAP_CACHED) {
                    memory_arena_free(block);
                    block = HEAP_NEXT(block);
                }
                memory_arena_coalesce_free(start);
                return start;
            }
        } else {
            block = HEAP_NEXT(block);
        }
    }
    return 0;
}

ADDRESS(0x800191f8, 0x9c)
void memory_arena_wait_pending(HeapBlock *arena)
{
    HeapBlock *block;

    for (block = arena; block->state != HEAP_END; block = HEAP_NEXT(block)) {
        while (block->state == HEAP_LOADING) {
            cd_request_wait_idle();
        }
    }
}

ADDRESS(0x80019294, 0xe8)
void memory_arena_compact(HeapBlock *arena)
{
    HeapBlock *block = arena;
    HeapBlock *free = arena;
    HeapBlock *next;
    u32 *src;
    u32 *dst;
    u32 words;

    memory_arena_wait_pending(arena);
    while (block->state != HEAP_END) {
        next = (HeapBlock *)((u8 *)block + block->size + sizeof(HeapBlock));
        if (block->state == HEAP_CACHED) {
            memory_arena_free(block);
        }
        if (block->state == HEAP_USED) {
            if (block != free) {
                src = (u32 *)block;
                dst = (u32 *)free;
                words = (block->size + sizeof(HeapBlock)) >> 2;
                do {
                    *dst++ = *src++;
                } while (--words);
                *free->owner = (u8 *)free + sizeof(HeapBlock);
                free = (HeapBlock *)dst;
            } else {
                free = next;
            }
        }
        block = next;
    }
    free->state = HEAP_FREE;
    free->size = (u8 *)block - (u8 *)free - sizeof(HeapBlock);
}

ADDRESS(0x8001937c, 0x20)
void memory_arena_initialize_blocks(void *arena, s32 size)
{
    HeapBlock *block = (HeapBlock *)arena;

    block->state = HEAP_FREE;
    block->size = size - 16;
    size -= 4;
    ((HeapBlock *)((u8 *)arena + size))->state = HEAP_END;
}

ADDRESS(0x8001939c, 0xb8)
u8 *memory_arena_allocate_block(HeapBlock *arena, u32 size, u8 **owner)
{
    HeapBlock *block;
    HeapBlock *rest;
    s32 left;
    u32 avail;
    u8 *data;

    block = memory_arena_find_block(arena, size);
    if (block == 0) {
        memory_arena_compact(arena);
        block = memory_arena_find_block(arena, size);
        if (block == 0) {
            return 0;
        }
    }
    avail = block->size - sizeof(HeapBlock);
    left = avail - size;
    if (left >= HEAP_SPLIT_MIN) {
        rest = (HeapBlock *)((u8 *)block + size + sizeof(HeapBlock));
        rest->state = HEAP_FREE;
        rest->size = left;
        rest->owner = 0;
    } else {
        size = block->size;
    }
    block->state = HEAP_USED;
    data = (u8 *)block + sizeof(HeapBlock);
    block->size = size;
    block->owner = owner;
    if (owner) {
        *owner = data;
    }
    return data;
}

ADDRESS(0x80019454, 0x20)
void memory_block_release(u8 *data)
{
    memory_arena_free((HeapBlock *)(data - sizeof(HeapBlock)));
}

ADDRESS(0x80019474, 0x8)
void memory_block_set_kind(u8 *data, u8 state)
{
    ((HeapBlock *)(data - sizeof(HeapBlock)))->state = state;
}

ADDRESS(0x8001947c, 0xc)
u8 memory_block_kind(u8 *data)
{
    return ((HeapBlock *)(data - sizeof(HeapBlock)))->state;
}

ADDRESS(0x80019488, 0x8)
void memory_block_set_flags(u8 *data, u8 value)
{
    ((HeapBlock *)(data - sizeof(HeapBlock)))->flags = value;
}

ADDRESS(0x80019490, 0xc)
u8 memory_block_flags(u8 *data)
{
    return ((HeapBlock *)(data - sizeof(HeapBlock)))->flags;
}

ADDRESS(0x8001949c, 0x8)
void memory_block_set_tag(u8 *data, u16 value)
{
    ((HeapBlock *)(data - sizeof(HeapBlock)))->tag = value;
}

ADDRESS(0x800194a4, 0xc)
u16 memory_block_tag(u8 *data)
{
    return ((HeapBlock *)(data - sizeof(HeapBlock)))->tag;
}

ADDRESS(0x800194b0, 0x40)
void *memory_malloc_checked(u32 size)
{
    void *p = malloc(size);

    return (u32)p - RAM_BASE < RAM_SIZE ? p : 0;
}

ADDRESS(0x800194f0, 0x28)
void *memory_allocate(u32 size)
{
    return memory_malloc_checked((size + 3) & ~3);
}

ADDRESS(0x80019518, 0x20)
void memory_free(void *data)
{
    free(data);
}

/* Checksum seed; the last word of a checked read holds the sum of the rest. */
#define CD_CHECKSUM_SEED 0x12345678

/* 60 VSyncs a second. */
#define FRAMES_PER_MINUTE 3600

/* Root counter 3 (VSync); later LIBAPI.H spells it RCntCNT3. */
#define RCNT_VSYNC (DescRC | 3)

RODATA(0x8001108c, 0x21)
SDATA(0x8009c220, 0xb)

DATA(0x801c12e4, 0x4c0, ".bss")
CdState cdrom_ctl;

void cd_vsync_handler(void);
void cd_complete_handler(void);
void cd_error_handler(void);
s32 cd_bcd_to_int(u8 bcd);
s32 cd_int_to_bcd(u8 value);
s32 cd_location_to_sector(CdlLOC *pos);
void cd_sector_to_location(CdlLOC *pos, u32 sector);
CdRequest *cd_request_enqueue(s32 kind, CdlLOC *pos, u32 bytes, u8 *buffer, CdCallback callback);

ADDRESS(0x80019538, 0x38)
void cd_request_yield(void)
{
    cd_request_service_vab();
    cd_request_service_stream();
    VSync(0);
    VSync(0);
}

ADDRESS(0x80019570, 0xa4)
void cd_vsync_handler(void)
{
    cdrom_ctl.frames++;
    cdrom_ctl.frame_gate++;
    cdrom_ctl.unkc++;
    if (cdrom_ctl.frames % FRAMES_PER_MINUTE == 0) {
        cur_avatar_status.play_minutes++;
    }
}

ADDRESS(0x80019614, 0x68)
void cd_wait_four_vsyncs(void)
{
    while (cdrom_ctl.frame_gate < 4) {
        VSync(0);
    }
    cdrom_ctl.frame_gate = 0;
}

ADDRESS(0x8001967c, 0x6c)
void cd_request_advance(CdRequest *request)
{
    request->kind = CD_REQUEST_FREE;
    request->issued = 0;
    request++;
    if (request == &cdrom_ctl.requests[24]) {
        request -= 24;
    }
    cdrom_ctl.read = request;
    if (request->kind != CD_REQUEST_FREE) {
        CdSeekL(&request->pos);
    } else {
        CdControl(CdlPause, 0, 0);
    }
}

ADDRESS(0x800196e8, 0x160)
void cd_complete_handler(void)
{
    CdRequest *request = cdrom_ctl.read;

    switch (request->kind) {
    case CD_REQUEST_FREE:
        break;
    case CD_REQUEST_READ:
    case CD_REQUEST_SOUND:
    case CD_REQUEST_IMAGE:
        switch (request->issued) {
        case 0:
            if (CdRead(request->count, (u_long *)request->buffer, CdlModeSpeed)) {
                request->issued = 1;
            } else {
                CdSeekL(&request->retry_pos);
            }
            break;
        case 1:
            if (request->kind == CD_REQUEST_READ) {
                if (cd_sectors_corrupt(request->buffer, request->count)) {
                    request->issued = 0;
                    CdSeekL(&request->retry_pos);
                } else {
                    if (request->callback) {
                        ((CdBufferCallback)request->callback)(request->buffer);
                    }
                    cd_request_advance(request);
                }
            } else if (request->callback) {
                ((CdRequestCallback)request->callback)(request);
            }
            break;
        }
        break;
    case CD_REQUEST_STREAM:
        if (request->issued == 0) {
            CdRead2(CdlModeSpeed);
            request->issued = 1;
        }
        break;
    }
}

ADDRESS(0x80019848, 0x88)
void cd_data_ready_handler(void)
{
    CdRequest *request = cdrom_ctl.read;

    switch (request->kind) {
    case CD_REQUEST_FREE:
        break;
    case CD_REQUEST_STREAM:
        CdGetSector((void *)request->buffer, 0x200);
        request->count--;
        if (request->callback) {
            ((CdRequestCallback)request->callback)(request);
        }
        if (request->count == 0) {
            cd_request_advance(request);
        }
        break;
    }
}

ADDRESS(0x800198d0, 0x8)
void cd_error_handler(void)
{
}

ADDRESS(0x800198d8, 0x20)
s32 cd_bcd_to_int(u8 bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0xf);
}

ADDRESS(0x800198f8, 0x3c)
s32 cd_int_to_bcd(u8 value)
{
    return ((u8)(value / 10) << 4) | (u8)(value % 10);
}

ADDRESS(0x80019934, 0x84)
s32 cd_location_to_sector(CdlLOC *pos)
{
    s32 minute = cd_bcd_to_int(pos->minute);
    s32 second = cd_bcd_to_int(pos->second);
    s32 sector = cd_bcd_to_int(pos->sector);

    return minute * 4500 + second * 75 + sector;
}

ADDRESS(0x800199b8, 0xbc)
void cd_sector_to_location(CdlLOC *pos, u32 sector)
{
    pos->minute = cd_int_to_bcd(sector / 4500);
    sector %= 4500;
    pos->second = cd_int_to_bcd(sector / 75);
    pos->sector = cd_int_to_bcd(sector % 75);
    pos->track = 0;
}

ADDRESS(0x80019a74, 0x40)
void cd_location_add(CdlLOC *pos, s32 offset, CdlLOC *out)
{
    cd_sector_to_location(out, cd_location_to_sector(pos) + offset);
}

ADDRESS(0x80019ab4, 0x68)
void cd_request_wait_idle(void)
{
    while (cdrom_ctl.read != cdrom_ctl.write || cdrom_ctl.read->kind != CD_REQUEST_FREE) {
        cd_request_yield();
    }
}

ADDRESS(0x80019b1c, 0x4c)
void cd_request_wait_done(CdRequest *request)
{
    while (request->kind != CD_REQUEST_FREE) {
        cd_request_yield();
    }
}

ADDRESS(0x80019b68, 0x54)
s32 cd_sectors_corrupt(u8 *buffer, s32 sectors)
{
    u32 sum = CD_CHECKSUM_SEED;
    u32 *word = (u32 *)buffer;
    s32 last = (sectors << 9) - 1;
    s32 i;

    for (i = last - 1; i != -1; i--) {
        sum += *word++;
    }
    return sum != ((u32 *)buffer)[last];
}

ADDRESS(0x80019bbc, 0x108)
CdRequest *cd_request_enqueue(s32 kind, CdlLOC *pos, u32 bytes, u8 *buffer, CdCallback callback)
{
    CdRequest *request = cdrom_ctl.write;

    cd_request_wait_done(request);
    request->kind = kind;
    request->issued = 0;
    request->pos = *pos;
    request->retry_pos = request->pos;
    request->buffer = buffer;
    request->count = (bytes + 2047) >> 11;
    request->callback = callback;
    if (++cdrom_ctl.write == &cdrom_ctl.requests[24]) {
        cdrom_ctl.write = cdrom_ctl.requests;
    }
    if (cdrom_ctl.read == request) {
        CdSeekL(pos);
    }
    return request;
}

ADDRESS(0x80019cc4, 0x84)
s32 cd_archive_entry_extent(s32 file, s32 entry, CdlLOC *pos)
{
    CdArchive *f = &cdrom_ctl.archives[file];
    s32 first = f->sectors[entry];

    if (pos) {
        cd_location_add(&f->extent.pos, first, pos);
    }
    return (f->sectors[entry + 1] - first) << 11;
}

ADDRESS(0x80019d48, 0x50)
void cd_archive_queue_read(s32 file, s32 entry, u8 *buffer, CdCallback callback)
{
    CdlLOC pos;

    cd_request_enqueue(CD_REQUEST_READ, &pos, cd_archive_entry_extent(file, entry, &pos), buffer, callback);
}

ADDRESS(0x80019d98, 0x58)
void cd_archive_queue_sector_callback_read(int file_in, int entry_in, u8 *buffer, CdCallback callback)
{
    u16 file = file_in;
    u16 entry = entry_in;
    CdlLOC pos;

    cd_request_enqueue(CD_REQUEST_STREAM, &pos, cd_archive_entry_extent(file, entry, &pos), buffer, callback);
}

ADDRESS(0x80019df0, 0x58)
void cd_archive_queue_stream_read(int file_in, int entry_in, u8 *buffer, CdCallback callback)
{
    u16 file = file_in;
    u16 entry = entry_in;
    CdlLOC pos;

    cd_request_enqueue(CD_REQUEST_SOUND, &pos, cd_archive_entry_extent(file, entry, &pos), buffer, callback);
}

ADDRESS(0x80019e48, 0xcc)
void cd_archive_read_chunked(u16 file, u16 entry, u8 *buffer, CdCallback callback)
{
    CdlLOC pos;
    CdRequest *request = cdrom_ctl.write;
    u32 bytes;

    cd_request_wait_done(request);
    request->u.image.rect.h = 0;
    request->state = 0;
    bytes = cd_archive_entry_extent(file, entry, &pos);
    request->u.image.remaining = bytes >> 11;
    request->u.image.sectors = bytes >> 11;
    if (request->u.image.remaining < 17) {
        request->u.image.remaining = 0;
        cd_request_enqueue(CD_REQUEST_IMAGE, &pos, bytes, buffer, callback);
    } else {
        request->u.image.remaining -= 16;
        cd_request_enqueue(CD_REQUEST_IMAGE, &pos, 0x8000, buffer, callback);
    }
}

ADDRESS(0x80019f14, 0x44)
s32 cd_archive_entry_size(int file_in, int entry_in)
{
    u16 file = file_in;
    u16 entry = entry_in;
    s32 first;

    first = cdrom_ctl.archives[file].sectors[entry];
    return (cdrom_ctl.archives[file].sectors[entry + 1] - first) << 11;
}

ADDRESS(0x80019f58, 0x8)
void cd_report_error(s32 fatal)
{
}

ADDRESS(0x80019f60, 0x15c)
void cd_read_sectors(CdlLOC *pos, u8 *buffer, s32 sectors)
{
    u_char result[8];
    s32 failed;
    s32 tries;
    s32 status;

    cd_request_wait_idle();
    DisableEvent(cdrom_ctl.error_event);
    DisableEvent(cdrom_ctl.complete_event);
    DisableEvent(cdrom_ctl.ready_event);
    tries = 0;
    failed = 1;
    CdControl(CdlPause, 0, 0);
    do {
    retry:
        while (CdControl(CdlSetloc, (u_char *)pos, 0) == 0) {
        }
        if (CdRead(sectors, (u_long *)buffer, CdlModeSpeed) == 0) {
            goto retry;
        }
        while ((status = CdReadSync(1, result)) > 0) {
        }
        CdControl(CdlPause, 0, 0);
        if (status == 0) {
            failed = 0;
            break;
        }
        tries++;
    } while (tries < 3);
    EnableEvent(cdrom_ctl.error_event);
    EnableEvent(cdrom_ctl.complete_event);
    EnableEvent(cdrom_ctl.ready_event);
    if (failed == 1) {
        cd_report_error(1);
    }
}

ADDRESS(0x8001a0bc, 0x5c)
u8 *cd_extent_load(CdlFILE *file)
{
    s32 bytes = (file->size + 2047) & ~2047;
    u8 *buffer = (u8 *)memory_allocate(bytes);

    cd_read_sectors(&file->pos, buffer, bytes >> 11);
    return buffer;
}

ADDRESS(0x8001a118, 0x3c)
void cd_extent_read_into(u8 *buffer, CdExtent *extent, u32 bytes)
{
    if (bytes == 0) {
        bytes = extent->size;
    }
    cd_read_sectors(&extent->pos, buffer, (bytes + 2047) >> 11);
}

ADDRESS(0x8001a154, 0x94)
void cd_archive_read(int file_in, int entry_in, u8 *buffer)
{
    u16 file = file_in;
    u16 entry = entry_in;
    CdlLOC pos;
    CdArchive *f = &cdrom_ctl.archives[file];
    s32 first = f->sectors[entry];
    s32 sectors = f->sectors[entry + 1] - first;

    cd_location_add(&f->extent.pos, first, &pos);
    do {
        cd_read_sectors(&pos, buffer, sectors);
    } while (cd_sectors_corrupt(buffer, sectors));
}

ADDRESS(0x8001a1e8, 0xb0)
u8 *cd_file_load(char *name)
{
    char path[64];
    CdlFILE file;
    u32 bytes;
    u8 *buffer;

    strcpy(path, "\\CD\\");
    strcat(path, name);
    strcat(path, ";1");
    cd_request_wait_idle();
    if (CdSearchFile(&file, path) == 0) {
        cd_report_error(0);
    }
    bytes = (file.size + 2047) & ~2047;
    buffer = (u8 *)memory_allocate(bytes);
    cd_read_sectors(&file.pos, buffer, bytes >> 11);
    return buffer;
}

ADDRESS(0x8001a298, 0xb4)
s32 cd_file_load_into(u8 *buffer, char *name, u32 bytes)
{
    char path[64];
    CdlFILE file;
    CdlLOC unused; /* the frame keeps 8 unused bytes here */

    strcpy(path, "\\CD\\");
    strcat(path, name);
    strcat(path, ";1");
    cd_request_wait_idle();
    if (CdSearchFile(&file, path) == 0) {
        cd_report_error(0);
    }
    if (bytes == 0) {
        bytes = file.size;
    }
    cd_read_sectors(&file.pos, buffer, (bytes + 2047) >> 11);
    return 0;
}

ADDRESS(0x8001a34c, 0xec)
void cd_archive_open(int index_in, char *name)
{
    u16 index = index_in;
    u8 buffer[0x800];
    char unused[64]; /* the frame keeps 64 unused bytes here */
    CdlFILE file;
    char path[64];
    CdArchive *f = &cdrom_ctl.archives[index];

    strcpy(path, "\\CD\\");
    strcat(path, name);
    strcat(path, ";1");
    if (CdSearchFile(&file, path) == 0) {
        printf("CdSearchFile error  FileName=%s\n", path);
    }
    f->extent.pos = file.pos;
    f->extent.size = file.size;
    cd_extent_read_into(buffer, &f->extent, 0x800);
    f->sectors = (u16 *)memory_allocate((*(u16 *)buffer + 1) * 2);
    resource_copy_halfwords(f->sectors, (u16 *)buffer + 1, *(u16 *)buffer + 1);
}

ADDRESS(0x8001a438, 0x188)
void cd_initialize(void)
{
    CdRequest *request;
    s32 i;

    EnterCriticalSection();
    cdrom_ctl.vsync_event = OpenEvent(RCNT_VSYNC, EvSpINT, EvMdINTR, cd_vsync_handler);
    EnableEvent(cdrom_ctl.vsync_event);
    SetRCnt(RCNT_VSYNC, 60000, RCntMdINTR);
    StartRCnt(RCNT_VSYNC);
    cdrom_ctl.frames = cdrom_ctl.frame_gate = cdrom_ctl.unkc = 0;
    request = cdrom_ctl.requests;
    i = 0;
    while (i < 24) {
        i++;
        request->kind = CD_REQUEST_FREE;
        request->issued = 0;
        request++;
    }
    cdrom_ctl.write = cdrom_ctl.requests;
    cdrom_ctl.read = cdrom_ctl.requests;
    cdrom_ctl.error_event = OpenEvent(HwCdRom, EvSpERROR, EvMdINTR, cd_error_handler);
    cdrom_ctl.complete_event = OpenEvent(HwCdRom, EvSpCOMP, EvMdINTR, cd_complete_handler);
    cdrom_ctl.ready_event = OpenEvent(HwCdRom, EvSpDR, EvMdINTR, cd_data_ready_handler);
    EnableEvent(cdrom_ctl.error_event);
    EnableEvent(cdrom_ctl.complete_event);
    EnableEvent(cdrom_ctl.ready_event);
    ExitCriticalSection();
}

ADDRESS(0x8001a5c0, 0x6c)
void cd_close_events(void)
{
    EnterCriticalSection();
    CloseEvent(cdrom_ctl.vsync_event);
    CloseEvent(cdrom_ctl.error_event);
    CloseEvent(cdrom_ctl.complete_event);
    CloseEvent(cdrom_ctl.ready_event);
    ExitCriticalSection();
}

ADDRESS(0x8001a62c, 0x8)
void func_8001a62c(void)
{
}
