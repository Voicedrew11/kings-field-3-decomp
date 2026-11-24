#ifndef GAME_SCENE_H
#define GAME_SCENE_H

/*
 * GAME.EXE area set-up (src/game/scene.c, WIP names): start-up loading of
 * the archive index tables and the first FDAT.T entry, the area load loop,
 * and the shift that moves every object when the area origin changes.
 */
#include <kf/lib/types.h>
#include <game/data.h>

/* The loaded FDAT module's handler table and the FDAT window base. */
extern u32 DAT_8009c3fc;
extern u32 DAT_8009c400[];

/* Handler table installed while an FDAT module loads: 32 copies of the empty
 * func_8001765c (.data, owner unit unknown). */
extern FdatHandler DAT_8007e4e4[32];

void func_80017664(void);
void func_8001796c(void);
void func_80017aa4(s32 dx, s32 dy, s32 dz);
void func_80017c78(u8 area0, u8 area1, u8 area2, u8 area3, u8 area4, s8 shift0, s8 shift1,
                   s8 shift2);
void func_800182e0(void);
void func_800182f8(void);
void func_80018310(void);
void func_80018328(void);
void func_80018340(void);
void func_80018358(void);

#endif
