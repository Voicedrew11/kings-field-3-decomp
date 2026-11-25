/*
 * GAME.EXE: frame pacing and CD access (WIP module). The root counter 3
 * event counts VSyncs; the CD event handlers walk a ring of read requests
 * (whole reads checked by a word checksum, sector streams, sound banks and
 * VRAM images); synchronous reads, BCD sector positions and the archive file
 * table round it off. Its unit is -G8: the "\CD\" and ";1" path literals are
 * .sdata, while the CD state is one aggregate reached absolutely.
 */
/* The CD state's VSync counter is volatile only here (see game/cd.h). */
#define GAME_CD_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <game/sound.h>
#include <game/cd.h>
#include <game/data.h>
#include <game/memory.h>
#include <game/image.h>
#include <game/funcs.h>

/* Checksum seed; the last word of a checked read holds the sum of the rest. */
#define CD_CHECKSUM_SEED 0x12345678

/* 60 VSyncs a second. */
#define FRAMES_PER_MINUTE 3600

/* Root counter 3 (VSync); later LIBAPI.H spells it RCntCNT3. */
#define RCNT_VSYNC (DescRC | 3)

RODATA(0x8001108c, 0x21)
SDATA(0x8009c220, 0xb)

DATA(0x801c12e4, 0x4c0, ".bss")
CdState DAT_801c12e4;

void func_80019570(void);
void func_800196e8(void);
void func_800198d0(void);
s32 func_800198d8(u8 bcd);
s32 func_800198f8(u8 value);
s32 func_80019934(CdlLOC *pos);
void func_800199b8(CdlLOC *pos, u32 sector);
CdRequest *func_80019bbc(s32 kind, CdlLOC *pos, u32 bytes, u8 *buffer, CdCallback callback);

ADDRESS(0x80019538, 0x38)
void func_80019538(void)
{
    func_80015a48();
    func_80018cd0();
    VSync(0);
    VSync(0);
}

ADDRESS(0x80019570, 0xa4)
void func_80019570(void)
{
    DAT_801c12e4.frames++;
    DAT_801c12e4.frame_gate++;
    DAT_801c12e4.unkc++;
    if (DAT_801c12e4.frames % FRAMES_PER_MINUTE == 0) {
        DAT_801b24e4.play_minutes++;
    }
}

ADDRESS(0x80019614, 0x68)
void func_80019614(void)
{
    while (DAT_801c12e4.frame_gate < 4) {
        VSync(0);
    }
    DAT_801c12e4.frame_gate = 0;
}

ADDRESS(0x8001967c, 0x6c)
void func_8001967c(CdRequest *request)
{
    request->kind = CD_REQUEST_FREE;
    request->issued = 0;
    request++;
    if (request == &DAT_801c12e4.requests[24]) {
        request -= 24;
    }
    DAT_801c12e4.read = request;
    if (request->kind != CD_REQUEST_FREE) {
        CdSeekL(&request->pos);
    } else {
        CdControl(CdlPause, 0, 0);
    }
}

ADDRESS(0x800196e8, 0x160)
void func_800196e8(void)
{
    CdRequest *request = DAT_801c12e4.read;

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
                if (func_80019b68(request->buffer, request->count)) {
                    request->issued = 0;
                    CdSeekL(&request->retry_pos);
                } else {
                    if (request->callback) {
                        request->callback(request->buffer);
                    }
                    func_8001967c(request);
                }
            } else if (request->callback) {
                request->callback(request);
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
void func_80019848(void)
{
    CdRequest *request = DAT_801c12e4.read;

    switch (request->kind) {
    case CD_REQUEST_FREE:
        break;
    case CD_REQUEST_STREAM:
        CdGetSector(request->buffer, 0x200);
        request->count--;
        if (request->callback) {
            request->callback(request);
        }
        if (request->count == 0) {
            func_8001967c(request);
        }
        break;
    }
}

ADDRESS(0x800198d0, 0x8)
void func_800198d0(void)
{
}

ADDRESS(0x800198d8, 0x20)
s32 func_800198d8(u8 bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0xf);
}

ADDRESS(0x800198f8, 0x3c)
s32 func_800198f8(u8 value)
{
    return ((u8)(value / 10) << 4) | (u8)(value % 10);
}

ADDRESS(0x80019934, 0x84)
s32 func_80019934(CdlLOC *pos)
{
    s32 minute = func_800198d8(pos->minute);
    s32 second = func_800198d8(pos->second);
    s32 sector = func_800198d8(pos->sector);

    return minute * 4500 + second * 75 + sector;
}

ADDRESS(0x800199b8, 0xbc)
void func_800199b8(CdlLOC *pos, u32 sector)
{
    pos->minute = func_800198f8(sector / 4500);
    sector %= 4500;
    pos->second = func_800198f8(sector / 75);
    pos->sector = func_800198f8(sector % 75);
    pos->track = 0;
}

ADDRESS(0x80019a74, 0x40)
void func_80019a74(CdlLOC *pos, s32 offset, CdlLOC *out)
{
    func_800199b8(out, func_80019934(pos) + offset);
}

ADDRESS(0x80019ab4, 0x68)
void func_80019ab4(void)
{
    while (DAT_801c12e4.read != DAT_801c12e4.write || DAT_801c12e4.read->kind != CD_REQUEST_FREE) {
        func_80019538();
    }
}

ADDRESS(0x80019b1c, 0x4c)
void func_80019b1c(CdRequest *request)
{
    while (request->kind != CD_REQUEST_FREE) {
        func_80019538();
    }
}

ADDRESS(0x80019b68, 0x54)
s32 func_80019b68(u8 *buffer, s32 sectors)
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
CdRequest *func_80019bbc(s32 kind, CdlLOC *pos, u32 bytes, u8 *buffer, CdCallback callback)
{
    CdRequest *request = DAT_801c12e4.write;

    func_80019b1c(request);
    request->kind = kind;
    request->issued = 0;
    request->pos = *pos;
    request->retry_pos = request->pos;
    request->buffer = buffer;
    request->count = (bytes + 2047) >> 11;
    request->callback = callback;
    if (++DAT_801c12e4.write == &DAT_801c12e4.requests[24]) {
        DAT_801c12e4.write = DAT_801c12e4.requests;
    }
    if (DAT_801c12e4.read == request) {
        CdSeekL(pos);
    }
    return request;
}

ADDRESS(0x80019cc4, 0x84)
s32 func_80019cc4(s32 file, s32 entry, CdlLOC *pos)
{
    CdFile *f = &DAT_801c12e4.files[file];
    s32 first = f->sectors[entry];

    if (pos) {
        func_80019a74(&f->extent.pos, first, pos);
    }
    return (f->sectors[entry + 1] - first) << 11;
}

ADDRESS(0x80019d48, 0x50)
void func_80019d48(s32 file, s32 entry, u8 *buffer, CdCallback callback)
{
    CdlLOC pos;

    func_80019bbc(CD_REQUEST_READ, &pos, func_80019cc4(file, entry, &pos), buffer, callback);
}

ADDRESS(0x80019d98, 0x58)
void func_80019d98(int file_in, int entry_in, u8 *buffer, CdCallback callback)
{
    u16 file = file_in;
    u16 entry = entry_in;
    CdlLOC pos;

    func_80019bbc(CD_REQUEST_STREAM, &pos, func_80019cc4(file, entry, &pos), buffer, callback);
}

ADDRESS(0x80019df0, 0x58)
void func_80019df0(int file_in, int entry_in, u8 *buffer, CdCallback callback)
{
    u16 file = file_in;
    u16 entry = entry_in;
    CdlLOC pos;

    func_80019bbc(CD_REQUEST_SOUND, &pos, func_80019cc4(file, entry, &pos), buffer, callback);
}

ADDRESS(0x80019e48, 0xcc)
void func_80019e48(u16 file, u16 entry, u8 *buffer, CdCallback callback)
{
    CdlLOC pos;
    CdRequest *request = DAT_801c12e4.write;
    u32 bytes;

    func_80019b1c(request);
    request->u.image.rect.h = 0;
    request->state = 0;
    bytes = func_80019cc4(file, entry, &pos);
    request->u.image.remaining = bytes >> 11;
    request->u.image.sectors = bytes >> 11;
    if (request->u.image.remaining < 17) {
        request->u.image.remaining = 0;
        func_80019bbc(CD_REQUEST_IMAGE, &pos, bytes, buffer, callback);
    } else {
        request->u.image.remaining -= 16;
        func_80019bbc(CD_REQUEST_IMAGE, &pos, 0x8000, buffer, callback);
    }
}

ADDRESS(0x80019f14, 0x44)
s32 func_80019f14(int file_in, int entry_in)
{
    u16 file = file_in;
    u16 entry = entry_in;
    s32 first;

    first = DAT_801c12e4.files[file].sectors[entry];
    return (DAT_801c12e4.files[file].sectors[entry + 1] - first) << 11;
}

ADDRESS(0x80019f58, 0x8)
void func_80019f58(s32 fatal)
{
}

ADDRESS(0x80019f60, 0x15c)
void func_80019f60(CdlLOC *pos, u8 *buffer, s32 sectors)
{
    u_char result[8];
    s32 failed;
    s32 tries;
    s32 status;

    func_80019ab4();
    DisableEvent(DAT_801c12e4.error_event);
    DisableEvent(DAT_801c12e4.complete_event);
    DisableEvent(DAT_801c12e4.ready_event);
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
    EnableEvent(DAT_801c12e4.error_event);
    EnableEvent(DAT_801c12e4.complete_event);
    EnableEvent(DAT_801c12e4.ready_event);
    if (failed == 1) {
        func_80019f58(1);
    }
}

ADDRESS(0x8001a0bc, 0x5c)
u8 *func_8001a0bc(CdlFILE *file)
{
    s32 bytes = (file->size + 2047) & ~2047;
    u8 *buffer = (u8 *)func_800194f0(bytes);

    func_80019f60(&file->pos, buffer, bytes >> 11);
    return buffer;
}

ADDRESS(0x8001a118, 0x3c)
void func_8001a118(u8 *buffer, CdExtent *extent, u32 bytes)
{
    if (bytes == 0) {
        bytes = extent->size;
    }
    func_80019f60(&extent->pos, buffer, (bytes + 2047) >> 11);
}

ADDRESS(0x8001a154, 0x94)
void func_8001a154(int file_in, int entry_in, u8 *buffer)
{
    u16 file = file_in;
    u16 entry = entry_in;
    CdlLOC pos;
    CdFile *f = &DAT_801c12e4.files[file];
    s32 first = f->sectors[entry];
    s32 sectors = f->sectors[entry + 1] - first;

    func_80019a74(&f->extent.pos, first, &pos);
    do {
        func_80019f60(&pos, buffer, sectors);
    } while (func_80019b68(buffer, sectors));
}

ADDRESS(0x8001a1e8, 0xb0)
u8 *func_8001a1e8(char *name)
{
    char path[64];
    CdlFILE file;
    u32 bytes;
    u8 *buffer;

    strcpy(path, "\\CD\\");
    strcat(path, name);
    strcat(path, ";1");
    func_80019ab4();
    if (CdSearchFile(&file, path) == 0) {
        func_80019f58(0);
    }
    bytes = (file.size + 2047) & ~2047;
    buffer = (u8 *)func_800194f0(bytes);
    func_80019f60(&file.pos, buffer, bytes >> 11);
    return buffer;
}

ADDRESS(0x8001a298, 0xb4)
s32 func_8001a298(u8 *buffer, char *name, u32 bytes)
{
    char path[64];
    CdlFILE file;
    CdlLOC unused; /* the frame keeps 8 unused bytes here */

    strcpy(path, "\\CD\\");
    strcat(path, name);
    strcat(path, ";1");
    func_80019ab4();
    if (CdSearchFile(&file, path) == 0) {
        func_80019f58(0);
    }
    if (bytes == 0) {
        bytes = file.size;
    }
    func_80019f60(&file.pos, buffer, (bytes + 2047) >> 11);
    return 0;
}

ADDRESS(0x8001a34c, 0xec)
void func_8001a34c(int index_in, char *name)
{
    u16 index = index_in;
    u8 buffer[0x800];
    char unused[64]; /* the frame keeps 64 unused bytes here */
    CdlFILE file;
    char path[64];
    CdFile *f = &DAT_801c12e4.files[index];

    strcpy(path, "\\CD\\");
    strcat(path, name);
    strcat(path, ";1");
    if (CdSearchFile(&file, path) == 0) {
        printf("CdSearchFile error  FileName=%s\n", path);
    }
    f->extent.pos = file.pos;
    f->extent.size = file.size;
    func_8001a118(buffer, &f->extent, 0x800);
    f->sectors = (u16 *)func_800194f0((*(u16 *)buffer + 1) * 2);
    func_80018f8c(f->sectors, (u16 *)buffer + 1, *(u16 *)buffer + 1);
}

ADDRESS(0x8001a438, 0x188)
void func_8001a438(void)
{
    CdRequest *request;
    s32 i;

    EnterCriticalSection();
    DAT_801c12e4.vsync_event = OpenEvent(RCNT_VSYNC, EvSpINT, EvMdINTR, func_80019570);
    EnableEvent(DAT_801c12e4.vsync_event);
    SetRCnt(RCNT_VSYNC, 60000, RCntMdINTR);
    StartRCnt(RCNT_VSYNC);
    DAT_801c12e4.frames = DAT_801c12e4.frame_gate = DAT_801c12e4.unkc = 0;
    request = DAT_801c12e4.requests;
    i = 0;
    while (i < 24) {
        i++;
        request->kind = CD_REQUEST_FREE;
        request->issued = 0;
        request++;
    }
    DAT_801c12e4.write = DAT_801c12e4.requests;
    DAT_801c12e4.read = DAT_801c12e4.requests;
    DAT_801c12e4.error_event = OpenEvent(HwCdRom, EvSpERROR, EvMdINTR, func_800198d0);
    DAT_801c12e4.complete_event = OpenEvent(HwCdRom, EvSpCOMP, EvMdINTR, func_800196e8);
    DAT_801c12e4.ready_event = OpenEvent(HwCdRom, EvSpDR, EvMdINTR, func_80019848);
    EnableEvent(DAT_801c12e4.error_event);
    EnableEvent(DAT_801c12e4.complete_event);
    EnableEvent(DAT_801c12e4.ready_event);
    ExitCriticalSection();
}

ADDRESS(0x8001a5c0, 0x6c)
void func_8001a5c0(void)
{
    EnterCriticalSection();
    CloseEvent(DAT_801c12e4.vsync_event);
    CloseEvent(DAT_801c12e4.error_event);
    CloseEvent(DAT_801c12e4.complete_event);
    CloseEvent(DAT_801c12e4.ready_event);
    ExitCriticalSection();
}

ADDRESS(0x8001a62c, 0x8)
void func_8001a62c(void)
{
}
