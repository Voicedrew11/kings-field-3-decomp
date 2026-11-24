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
void func_8002a10c(struct Record18 *spell);
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
void func_8002bb84(u8 id, u8 slot);
void func_8002bdc0(u8 id);
void func_8002c040(s32 hand);
struct Object88 *func_8002c164(s32 speed, VECTOR *offset, SVECTOR *dir, s32 *dist);

/* Pad bit per action (WIP), defined in player.c; [7] toggles a setting in
 * Data801b24e4.unk178 (func_80043bb8). */
extern u16 DAT_80081868[14];

void func_8002c30c(s32 id, ...);

/*
 * The same WIP module after func_8002c30c: src/game/player_cast.c,
 * player_spell.c and player_action.c (func_8002d2a0 is not exact
 * yet).
 */
void func_8002cea8(s32 vertex, VECTOR *out);
s32 func_8002cfe0(s32 hand);
void func_8002d130(s32 id, s32 pay, s32 dir);
void func_8002d2a0(void);
void func_8002deec(s32 id);
s32 func_8002e3f8(s32 angle, s32 speed);
void func_8002ecbc(void);
void func_8002ed1c(s32 speed);
void func_8002ed60(void);
s32 func_8002f320(void);
void func_8002f5c0(void);
void func_8002f9bc(void);
s32 func_8002fddc(s32 id);
void func_8002fe1c(void);
void func_8003047c(void);
void func_80030568(void);
void func_800305d8(void);
void func_8003078c(void);
void func_800307f4(u8 id);
void func_80030810(u8 id);
void func_80030864(u8 id);
void func_800308b8(SVECTOR *dir);
void func_800308f4(void);
void func_80030908(SVECTOR *dir, SVECTOR *push, s16 lift);
void func_800309c8(SVECTOR *dir, SVECTOR *push, s16 lift);
void func_80030a6c(SVECTOR *dir);
void func_80030af8(void);
s16 func_80030b24(s16 *count, s16 *timer, s32 range, s32 scale);
void func_80030be0(s32 amount);
void func_80030c4c(s32 amount);
void func_80030c6c(s32 amount);
void func_80030cbc(struct Record20 *r);
void func_80030d68(struct Record20 *r);
void func_80030e14(void);

/* Callees outside the module (no definition reconstructed yet). */
void func_8005cbe0(VECTOR *pos, SVECTOR *angle, s32 id);
void func_8005e2d0(VECTOR *pos, SVECTOR *angle);

#endif
