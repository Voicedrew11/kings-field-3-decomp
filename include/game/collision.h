#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

/*
 * GAME.EXE collision tests (src/game/collision.c, WIP). A shape word carries
 * the height in its low 28 bits; results land in DAT_801d4464 from +0x12000
 * (cell, floor height, hit bits and indices, the hit point and size).
 */
#include <kf/lib/types.h>
#include <sys/types.h>

s32 func_80033b10(s32 x, s32 y, s32 z, s32 radius, s32 shape);
s32 func_80033b8c(u8 area, s32 x, s32 z, s32 radius, s32 height);
void func_80033c4c(s32 x, s32 z, s32 radius, s32 count);
s32 func_80033d08(s32 x, s32 y, s32 z, s32 radius, s32 shape);
void func_80033d84(void);
s32 func_80033f38(s32 x, s32 y, s32 z, s32 radius, s32 shape, u16 mask);

/* Callees outside the module; signatures from these call sites. */
s32 func_8003260c(s32 x, s32 y, s32 z, s32 radius, s32 shape);

#endif
