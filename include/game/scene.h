#ifndef GAME_SCENE_H
#define GAME_SCENE_H

/*
 * GAME.EXE area set-up (src/game/scene.c, WIP names): start-up loading of
 * the archive index tables and the first FDAT.T entry, the area load loop,
 * and the shift that moves every object when the area origin changes.
 */
#include <kf/lib/types.h>

/* The loaded FDAT module's handler table and the FDAT window base. */
extern u32 DAT_8009c3fc;
extern u32 DAT_8009c400[];

void func_80017664(void);
void func_8001796c(void);
void func_80017aa4(s32 dx, s32 dy, s32 dz);

#endif
