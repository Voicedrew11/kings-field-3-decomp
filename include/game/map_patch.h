#ifndef GAME_MAP_PATCH_H
#define GAME_MAP_PATCH_H

/*
 * GAME.EXE map cell patches (src/game/map_patch.c, WIP).
 */
#include <kf/lib/types.h>
#include <sys/types.h>

/* A cell patch record (10 bytes; lists end with bytes[0][0] == 0xff): per
 * variant the own and other layer's unk3, then their unk0, and the cell
 * offset from the patch origin before the turn. */
typedef struct MapPatch {
    u8 bytes[2][4];
    s8 dx;
    s8 dz;
} MapPatch;

void func_800443c8(u8 area, s32 x, s32 z, s32 angle, MapPatch *rec, s32 index, s32 flags);
void func_800445b8(s32 flags, s32 sx, s32 sz, s32 dx, s32 dz, s32 w, s32 h, s32 angle,
                   s32 mask);

#endif
