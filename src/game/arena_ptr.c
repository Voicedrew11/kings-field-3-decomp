/*
 * GAME.EXE: the DAT_801ba984 block arena's 32 saved pointers (WIP module):
 * pointers to and from arena offsets and moving the blocks' owner
 * pointers by a delta. func_8005efd4 (switch) follows.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/memory.h>
#include <game/data.h>
#include <game/misc.h>

/* out[i] = the arena address of saved offset i, NULL for 0xffff. */
ADDRESS(0x8005ee58, 0x50)
void func_8005ee58(u8 **out)
{
    u16 *off;
    u16 o;
    s32 i;

    off = DAT_801ba984.unk5104;
    for (i = 31; i != -1; i--) {
        o = *off++;
        if (o == 0xffff) {
            *out = NULL;
        } else {
            *out = DAT_801ba984.unk104 + o;
        }
        out++;
    }
}

/* Adds delta to the owner pointer of every used block. */
ADDRESS(0x8005eea8, 0x70)
void func_8005eea8(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)DAT_801ba984.unk104;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner + delta);
        }
        block = HEAP_NEXT(block);
    }
}

/* Saves in[i] as an arena offset, 0xffff for NULL. */
ADDRESS(0x8005ef18, 0x4c)
void func_8005ef18(u8 **in)
{
    u16 *off;
    u8 *p;
    s32 i;

    off = DAT_801ba984.unk5104;
    for (i = 31; i != -1; i--) {
        p = *in++;
        if (p == NULL) {
            *off = 0xffff;
        } else {
            *off = p - DAT_801ba984.unk104;
        }
        off++;
    }
}

/* Subtracts delta from the owner pointer of every used block. */
ADDRESS(0x8005ef64, 0x70)
void func_8005ef64(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)DAT_801ba984.unk104;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner - delta);
        }
        block = HEAP_NEXT(block);
    }
}
