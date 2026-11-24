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

u32 *func_80018f5c(u32 *dst, u32 *src, s32 count);
u16 *func_80018f8c(u16 *dst, u16 *src, s32 count);
u32 *func_80018fbc(u32 *dst, u32 value, s32 count);
u16 *func_80018fe0(u16 *dst, u16 value, s32 count);
void func_8001937c(void *arena, s32 size);
u8 *func_8001939c(HeapBlock *arena, u32 size, u8 **owner);
void func_80019454(u8 *data);
void func_80019474(u8 *data, u8 state);
u8 func_8001947c(u8 *data);
void func_80019488(u8 *data, u8 value);
u8 func_80019490(u8 *data);
void func_8001949c(u8 *data, u16 value);
u16 func_800194a4(u8 *data);
void *func_800194b0(u32 size);
void *func_800194f0(u32 size);
void func_80019518(void *data);

#endif
