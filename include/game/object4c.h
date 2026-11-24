#ifndef GAME_OBJECT4C_H
#define GAME_OBJECT4C_H

/*
 * GAME.EXE Object4c records (src/game/object4c*.c, WIP names): the 128
 * records of DAT_801b77ec.objects, made by func_80053c84 from an id that
 * indexes the Record18 records at the start of DAT_801b77ec, then moved,
 * tested for hits and drawn. DAT_801b77ec.unk2f04 names the record being
 * updated and unk2f00 its Record18 record.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/player.h>

/* Object4c.unk0 of a free record. */
#define OBJECT4C_FREE 0xff

s32 func_800532d4(Object4c *obj, s32 se);
s32 func_80053310(VECTOR *pos, s32 radius, s32 height);
/* K&R definition: its u16/u8 parameters arrive as int. */
void func_800533e8(s32 flags, s32 kind, Record18 *rec, int a3, int a4, int a5, int a6, int a7,
                   int a8, int a9, int a10, int a11, int a12, int a13, int a14, VECTOR *target);
u16 func_80053624(Object4c *obj);
void func_800536f8(s32 flags, s32 a1, VECTOR *target);
void func_800537e8(s32 flags);
void func_80053a0c(Object4c *obj, s32 vertex, VECTOR *out, SVECTOR *scale);
void func_80053af4(Object4c *obj, s32 vertex, VECTOR *out, SVECTOR *scale);
void func_80053850(VECTOR *pos, s32 reach, s32 radius, s32 mode, s32 falloff, s32 flags);
Object4c *func_80053b64(void);
void func_80053ba8(Object4c *obj, s32 a1, s32 scale, s32 a3);
void func_80053be4(Object4c *obj, s32 a1, s32 scale, s32 a3);
void func_80053c20(Object4c *obj, s32 a1, s32 a2, s32 a3);
/* Called with 7 to 12 arguments: no prototype (WIP). */
Object4c *func_80053c84();
void func_80055b4c(SVECTOR *dir, VECTOR *out, s32 yaw, s32 scale);
s32 func_80055bd8(s32 scale, s32 max, s32 radius, s32 height, SVECTOR *accel);
s32 func_80055db0(s32 max, s32 scale, s32 turn, s32 radius, s32 height, s32 range, s32 near_scale,
                  s32 cone);
s32 func_80055f74(VECTOR *target, s32 max, s32 scale, s32 slow, s32 near, s32 radius, s32 height);
void func_80056134(s32 rate, s32 limit, s32 grow, s32 falloff, s32 mode);
void func_800561d8(Object4c *obj, s32 vertex);
s32 func_80056268(VECTOR *pos, s32 a1, s32 a2, s32 height);
/* Reads up to six more arguments after a7 through &a7 (see its definition). */
void func_800562f0(Object4c *obj, s32 vertex, s32 mode, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
s32 func_80056580(VECTOR *pos, s32 count, s32 spread, s32 a3, s32 a4, s32 range);
s32 func_800566d4(s32 radius, s32 height, s32 spin);
void func_80056800(void);
void func_80056840(void);

extern SVECTOR DAT_8009c33c;

/* Callees outside the module; signatures from these call sites. */
void func_8004c668(u16 index, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6, u16 a7, u16 a8,
                   u16 a9, u16 a10, u16 a11, Record18 *rec, VECTOR *target);
void func_8004ceac(VECTOR *pos, s32 a1, s32 a2, s32 a3, u16 a4, u16 a5, u16 a6, u16 a7, u16 a8,
                   u16 a9, u16 a10, u16 a11, u16 a12, u16 a13, s32 a14, u16 a15, Record18 *rec);

#endif
