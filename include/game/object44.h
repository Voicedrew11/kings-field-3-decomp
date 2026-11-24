#ifndef GAME_OBJECT44_H
#define GAME_OBJECT44_H

/*
 * GAME.EXE Object44 records (src/game/object44.c, WIP): the 396 records of
 * DAT_8018fb3c.objects and their Object44Def table.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/data.h>

/* One record of an area's Object44 placement table (func_80044d9c reads 350). */
typedef struct Object44Place {
    u8 unk0;        /* +0x00: Object44.unk0 (area) */
    u8 unk1;        /* +0x01: map cell z */
    u8 unk2;        /* +0x02: map cell x */
    u8 unk3;
    u16 unk4;       /* +0x04: Object44Def index, 0xffff for none */
    u16 unk6;       /* +0x06: yaw, negated */
    s16 unk8;       /* +0x08: z offset in the cell */
    s16 unka;       /* +0x0a: x offset in the cell; kind 0xe0: 0xff or a target cell x */
    s16 unkc;       /* +0x0c: height */
    u8 unke[2];
    u8 unk10[8];    /* +0x10: copied to Object44.arg from +0x38; [0] the scale, 0x80 = 1 */
} Object44Place;

void func_8004482c(Object44 *obj, s32 se);
void func_8004485c(void);
void func_800448b8(Object44 *obj);
void func_80044900(s32 id, s32 mode, ...);
void func_80044b40(Object44 *obj, s32 mode, u8 value);
void func_80044c34(s32 mode);
void func_80044d9c(Object44Place *place);
s32 func_80045ac8(s32 x, s32 y, s32 z, s32 radius, s32 shape);
/* angle and range travel as int (FDAT callers); the callee narrows them to s16 at entry. */
s32 func_80045c7c(s32 start, VECTOR *pos, s32 height, s32 reach_up, int angle, int range);
void func_80046014(Object44 *obj, u8 kind);
Object44 *func_80046034(s32 first, s32 count, s32 stamp);
void func_800460bc(u8 which, u8 def_id, VECTOR *pos, s32 dy);
void func_80046294(u16 target, VECTOR *pos, s32 dy);
void func_800463bc(u8 id);
s32 func_80046608(Object44 *obj, s32 id);
void func_800466e8(u8 id);
void func_80046760(Object44 *obj, s32 vertex, VECTOR *out);
s32 func_80046884(s32 x, s32 z, s32 w, s32 h, s32 y);
s32 func_80046914(Object44 *src, Object44 *dst, SVECTOR *from, SVECTOR *to, s32 fade, s32 steps);
/* K&R definition: mode is u8 in the callee, int at the call (func_80030fcc passes an s16). */
void func_80046c00(int mode, s32 level, s32 end, s32 step);
void func_80046cb0(s32 dx, s32 dy, s32 dz);
s16 func_80046dcc(Object44 *obj, VECTOR *from, s32 dist, VECTOR *to, SVECTOR *out, s16 pitch,
                  s16 rate, s32 count);

#endif
