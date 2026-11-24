#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

/*
 * GAME.EXE player state functions (src/game/player.c, WIP module) over the
 * player record DAT_801b24e4 (game/data.h). Signatures follow the call sites
 * reconstructed so far.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>

void func_80028d54(void);
s32 func_80028e48(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
void func_80028e9c(void);
void func_80028f90(void);
int func_800290d8(int part);
void func_80029188(s32 area0, s32 area1, s32 area2, s32 area3, s32 area4, s32 part);
s32 func_80029390(s32 magic, s32 weight);
void func_800293e4(s32 id);
void func_80029500(void);
void func_80029f1c(u32 element);
void func_8002a074(void);
void func_8002a10c(u8 *spell);
void func_8002a210(void);
void func_8002a310(int amount);
s32 func_8002a5f8(s32 power, s32 defense, s32 attack);
void func_8002a6a0(s32 amount);
void func_8002a6f4(VECTOR *from, s32 damage, s32 flags);
void func_8002aa54(s32 flags);
void func_8002ab18(u16 slash, u16 blow, u16 stab, u16 type, u16 element0, u16 element1, u16 element2,
                   u16 element3, u16 element4, u16 scale, u16 knockback, VECTOR *from);
void func_8002b114(VECTOR *pos, s32 reach, s32 radius, s32 mode, u16 falloff, u16 slash, u16 blow,
                   u16 stab, u16 type, u16 element0, u16 element1, u16 element2, u16 element3,
                   u16 element4, s32 flags, u16 knockback);
void func_8002b330(VECTOR *pos, SVECTOR *angle);
void func_8002b3a8(void);
void func_8002b468(void);
void func_8002b64c(void);
void func_8002b6f0(void);
void func_8002b760(void);
s32 func_8002b80c(VECTOR *pos, s32 angle, s32 radius, s32 range);
s32 func_8002b8d8(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
void func_8002b928(u8 id);
void func_8002ba38(u8 id);
void func_8002bb5c(u8 id);
void func_8002bb84(s32 id, u8 slot);
void func_8002bdc0(u8 id);
void func_8002c040(s32 hand);
struct Object2c *func_8002c164(s32 speed, VECTOR *offset, SVECTOR *dir, s32 kind);

/* Later in the module (not reconstructed yet). */
void func_80030908(SVECTOR *dir, SVECTOR *push, s16 lift);
void func_800309c8(SVECTOR *dir, SVECTOR *push, s16 lift);

#endif
