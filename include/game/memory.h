#ifndef GAME_MEMORY_H
#define GAME_MEMORY_H

/*
 * GAME.EXE word and halfword copy/fill helpers and the block arena
 * (src/game/memory.c, WIP names). An arena is a run of blocks, each a
 * 12-byte header followed by its data and ended by a header whose state is
 * HEAP_END. A used block records where its owner keeps the data pointer, so
 * compaction can move the block and update that pointer.
 */
#include <kf/lib/types.h>

/* HeapBlock.state values. */
#define HEAP_FREE 0
#define HEAP_CACHED 1  /* may be freed when an allocation needs the room */
#define HEAP_USED 2
#define HEAP_LOADING 3 /* a CD read is still filling it */
#define HEAP_END 0xff

typedef struct HeapBlock {
    u8 state;      /* +0x0: HEAP_* */
    u8 unk1;       /* +0x1 */
    u16 unk2;      /* +0x2 */
    u32 size;      /* +0x4: data bytes after the header */
    u8 **owner;    /* +0x8: where the owner keeps the data pointer, or 0 */
} HeapBlock;

/* The block after this one, past its header and data. */
#define HEAP_NEXT(block) ((HeapBlock *)((u8 *)(block) + ((block)->size + sizeof(HeapBlock))))

u32 *resource_copy_words(u32 *dst, u32 *src, s32 count);
u16 *resource_copy_halfwords(u16 *dst, u16 *src, s32 count);
u32 *repeat_store_word(u32 *dst, u32 value, s32 count);
u16 *repeat_store_halfword(u16 *dst, u16 value, s32 count);
void memory_arena_initialize_blocks(void *arena, s32 size);
u8 *memory_arena_allocate_block(HeapBlock *arena, u32 size, u8 **owner);
void memory_block_release(u8 *data);
void memory_block_set_kind(u8 *data, u8 state);
u8 memory_block_kind(u8 *data);
void memory_block_set_flags(u8 *data, u8 value);
u8 memory_block_flags(u8 *data);
void memory_block_set_tag(u8 *data, u16 value);
u16 memory_block_tag(u8 *data);
void *memory_malloc_checked(u32 size);
void *memory_allocate(u32 size);
void memory_free(void *data);

#endif
