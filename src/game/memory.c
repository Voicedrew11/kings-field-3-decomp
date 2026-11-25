/*
 * GAME.EXE: memory helpers (WIP module): word and halfword copies and fills,
 * a block arena with cached blocks that an allocation may reclaim and a
 * compaction that moves used blocks down, accessors for a block's header
 * bytes, and malloc/free wrappers. It references no data.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <MALLOC.H>
#include <game/memory.h>
#include <game/cd.h>

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
    ((HeapBlock *)(data - sizeof(HeapBlock)))->unk1 = value;
}

ADDRESS(0x80019490, 0xc)
u8 memory_block_flags(u8 *data)
{
    return ((HeapBlock *)(data - sizeof(HeapBlock)))->unk1;
}

ADDRESS(0x8001949c, 0x8)
void memory_block_set_tag(u8 *data, u16 value)
{
    ((HeapBlock *)(data - sizeof(HeapBlock)))->unk2 = value;
}

ADDRESS(0x800194a4, 0xc)
u16 memory_block_tag(u8 *data)
{
    return ((HeapBlock *)(data - sizeof(HeapBlock)))->unk2;
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
