#ifndef GAME_MATH_H
#define GAME_MATH_H

/*
 * GAME.EXE angle, vector and matrix helpers (src/game/math.c). Angles are in
 * 4096ths of a turn; GTE fixed point is 4.12 (0x1000 = 1.0).
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

s16 func_80015f18(s16 angle, s16 target, s16 step);
s32 func_80015fdc(s32 value, s32 target, s32 step);
void func_80016044(VECTOR *from, VECTOR *to, s32 length, SVECTOR *out);
s32 func_80016168(s32 angle, s32 target, s32 speed, s32 accel, s32 drag);
void func_80016240(s16 angle, s32 *out);
void func_80016290(MATRIX *src, MATRIX *dst, s32 quarter);
void func_800164b4(SVECTOR *src, SVECTOR *dst, s32 quarter);
void func_80016598(s16 angle, MATRIX *m);
void func_8001660c(s16 angle, MATRIX *m);
void func_80016680(s16 angle, MATRIX *m);
void func_800166f4(SVECTOR *rotation, MATRIX *m);
void func_80016760(SVECTOR *rotation, MATRIX *m);
void func_800167cc(SVECTOR *rotation, SVECTOR *out);
void func_800168a8(SVECTOR *rotation, SVECTOR *v, VECTOR *out);
void func_800168f0(s16 scale, s32 *v);
void func_80016928(s16 scale, SVECTOR *v);
void func_80016978(s16 scale, s32 *v);
void func_800169b0(s16 scale, SVECTOR *v);
void func_80016a00(VECTOR *v, s32 *delta);
s32 func_80016a2c(s16 angle, s16 target, s16 range);
s32 func_80016a68(s16 angle, s16 target);
s32 func_80016a78(s32 angle, s32 target);
s32 func_80016a98(s32 angle, s32 target);
s32 func_80016ab8(s32 dx, s32 dz);
s32 func_80016c08(s32 dx, s32 dz);
s32 func_80016c44(s32 dx, s32 dy, s32 dz);
void func_80016c90(s32 dx, s32 dy, s32 dz, SVECTOR *out);
s32 func_80016d0c(s32 a, s32 b, s32 c, s32 d);
s32 func_80016d3c(SVECTOR *rotation, VECTOR *pos, s32 x, s32 y, s32 z, s32 half_width,
                  s32 half_depth, s32 reach_down, s32 reach_up);
s32 func_80016ec8(VECTOR *pos, s32 x, s32 y, s32 z, s32 radius, s32 reach_down, s32 reach_up);
s32 func_80016fb4(VECTOR *a, s32 radius_a, VECTOR *b, s32 radius_b, s32 height);
s32 func_800170c0(s32 range);
s32 func_80017108(s32 range);
s32 func_80017158(s32 from, s32 to, s32 t);
s32 func_80017170(s32 from, s32 to, s32 t);
void func_800171b0(MATRIX *from, MATRIX *to, MATRIX *out, s16 t);
s32 func_80017210(s32 high, s32 distance, s32 height, s32 gravity, s32 speed, s32 *time,
                  s32 *angle);
s32 func_8001749c(s32 high, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1, s32 gravity,
                  s32 speed, s16 *time, s16 *horizontal, s16 *vertical);
void func_800175a8(VECTOR *base, SVECTOR *dir, s32 scale, VECTOR *out);
s32 func_80017600(SVECTOR *normal, SVECTOR *point, s32 *d);
void func_8001765c(void);

#endif
