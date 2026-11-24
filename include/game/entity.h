#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

/*
 * GAME.EXE Object88 records (src/game/entity*.c, WIP names): the 200
 * records of DAT_80185da8 made from its Object78 records, their placement
 * on the map, the script records each runs, their movement and the searches
 * over them. DAT_80185da8.current and .def name the record being updated.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>

/* A script record's fields past the id and chances depend on the id (WIP view). */
#define SCRIPT_U16(script, offset) (*(u16 *)((script) + (offset)))

/* Object88.unk0 of a free record. */
#define OBJECT88_FREE 0xff

/* The shape word the collision tests take for an Object88 record. */
#define OBJECT88_SHAPE(obj) ((obj)->unk1e | (((obj)->unk28 & 0xc000) << 16))

/* A placement record of an area: one per Object88 record (func_800530f8). */
typedef struct Object88Place {
    u8 unk0;        /* Object88.unk0, 0xff for none */
    u8 unk1;        /* Object78 record index */
    u8 unk2;
    u8 unk3;        /* map cell z */
    u8 unk4;        /* map cell x */
    u8 unk5;
    u8 unk6;
    u8 unk7;        /* area */
    u16 unk8;       /* yaw, negated */
    s16 unka;       /* z offset */
    s16 unkc;       /* x offset; -1 takes Object78.unk1a */
    s16 unke;       /* height; -1 takes Object78.unk1c */
} Object88Place;

extern Object88 *DAT_80199148;
extern u8 DAT_80199150;

Object88 *func_8004b524(void);
void func_8004b560(Object88 *obj);
void func_8004b624(Object88 *obj);
void func_8004b698(Object88 *obj);
void func_8004b770(Object88 *obj);
void func_8004b794(void);
void func_8004b868(Object88 *obj);
void func_8004b8c4(u16 index);
void func_8004b8fc(void);
void func_8004b94c(Object88 *obj, u8 *script);
s32 func_8004b984(u8 *script, s32 dist);
void func_8004bf1c(s32 dist);
void func_8004c01c(void);
u8 *func_8004c068(Object78 *def, u8 id);
void func_8004c0b0(Object88 *obj, u8 id);
void func_8004c104(void);
void func_8004c130(u8 anim);
void func_8004c184(u8 anim);
void func_8004c1f0(void);
void func_8004c52c(u16 kind);
s32 func_8004c5ec(s32 a, s32 b, s32 c);
s32 func_8004d254(s32 near, s32 range, s32 height, s32 cone, u16 a4, u16 a5, u16 a6, u16 a7);
Object88 *func_8004d3c4(VECTOR *pos, s16 yaw, s16 pitch, s32 range, s32 max_yaw, s32 max_pitch,
                        s32 *dist, s32 jitter);
s32 func_8004d644(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
s32 func_8004d838(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
void func_8004da2c(Object88 *obj);
void func_8004db08(Object88 *obj, s16 step);
void func_8004db3c(Object88 *obj, s16 step);
s32 func_8004db98(Object88 *obj, u16 time);
s32 func_8004dbc8(SVECTOR *vel, s32 mode);
s32 func_8004e094(SVECTOR *vel);
s32 func_8004e27c(s32 high, s32 x, s32 y, s32 z, s32 gravity, s32 speed);
void func_8004e31c(void);
void func_8004e330(void);
s32 func_8004e6f8(s32 speed, s32 mode);
s32 func_8004e824(s32 angle, s32 scale_x, s32 scale_z, s32 mode);
s32 func_8004e8ec(s32 angle, s32 target);
s32 func_8004e914(s32 angle, s32 target);
void func_8004e928(Object88 *obj, s16 target, s32 max, s32 accel);
void func_8004ea7c(s16 target, s32 scale_x, s32 max, s32 scale_z, s32 accel, s32 mode);
s32 func_8004eaf0(s32 x, s32 z, s32 scale_x, s32 max, s16 face, s32 scale_z, s32 accel, s32 mode);
s32 func_8004ec08(SVECTOR *angle, s32 scale_a, s32 scale_b, s32 mode);
void func_8004ed50(SVECTOR *angle, s32 scale_a, s32 max, s32 scale_b, s32 accel, s32 mode);
s32 func_8004ede0(Object88 *obj, s32 vertex, VECTOR *out);
VECTOR *func_8004eee0(Object88 *obj, VECTOR *out);
void func_8004f008(s32 idle, s32 left, s32 right, s32 up, s32 down, s32 speed);
s32 func_8004f1d0(Object88 *obj, VECTOR *from, s32 speed, VECTOR *target, SVECTOR *out, s32 pitch,
                  s16 turn, s32 steps);
s32 func_8004ffa0(Object88 *obj);
void func_80050010(Object88 *obj);
void func_800500a8(void);
void func_80052e5c(void);
void func_80053084(void);
void func_800530f8(Object88Place *place);

/* Callees outside the module; signatures from these call sites. */
void func_8002ab18(s32 a0, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9,
                   s32 a10, VECTOR *pos);
void func_80033b10(s32 x, s32 y, s32 z, s32 radius, s32 shape);
void func_80033d08(s32 x, s32 y, s32 z, s32 radius, s32 shape);
void func_80033d84(void);
void func_80033c4c(s32 x, s32 z, s32 a2, s32 a3);
s32 func_80033f38(s32 x, s32 y, s32 z, s32 radius, s32 shape, s32 mask);

#endif
