#ifndef GAME_OBJECT44_H
#define GAME_OBJECT44_H

/*
 * GAME.EXE Object44 records (src/game/object44.c, WIP): the 396 records of
 * DAT_8018fb3c.objects and their Object44Def table.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/data.h>

void func_8004482c(Object44 *obj, s32 se);
void func_8004485c(void);
void func_800448b8(Object44 *obj);
void func_80044900(s32 id, s32 mode, ...);
void func_80044b40(Object44 *obj, s32 mode, u8 value);
void func_80044c34(s32 mode);
s32 func_80045ac8(s32 x, s32 y, s32 z, s32 radius, s32 shape);
s16 func_80045c7c(s16 start, VECTOR *pos, s32 height, s32 reach_up, s16 angle, s16 range);
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
void func_80046c00(u8 mode, s32 level, s32 end, s32 step);
void func_80046cb0(s32 dx, s32 dy, s32 dz);
s16 func_80046dcc(Object44 *obj, VECTOR *from, s32 dist, VECTOR *to, SVECTOR *out, s16 pitch,
                  s16 rate, s32 count);

#endif
