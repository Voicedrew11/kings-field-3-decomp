/*
 * GAME.EXE: Object4c movement (WIP module): steering and speed limits of
 * the current record, spawning further records from it, and its step and
 * collision test. The per-kind update switch func_800568bc follows.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/data.h>
#include <game/entity.h>
#include <game/object4c.h>

/* Zero velocity for the records func_800561d8 spawns. */
DATA(0x8009c33c, 0x8, ".sdata")
SVECTOR DAT_8009c33c = {0, 0, 0, 0};

/* dir's x and z scaled by scale and turned by yaw into out. */
ADDRESS(0x80055b4c, 0x8c)
void func_80055b4c(SVECTOR *dir, VECTOR *out, s32 yaw, s32 scale)
{
    SVECTOR v;
    SVECTOR rotation;
    MATRIX m;

    v.vx = dir->vx * scale >> 12;
    v.vy = 0;
    v.vz = dir->vz * scale >> 12;
    rotation.vx = 0;
    rotation.vy = yaw;
    rotation.vz = 0;
    RotMatrix(&rotation, &m);
    ApplyMatrix(&m, &v, out);
}

/* Accelerates the current record by accel (scaled), limits its speed to max, moves it and tests it. */
ADDRESS(0x80055bd8, 0x1d8)
s32 func_80055bd8(s32 scale, s32 max, s32 radius, s32 height, SVECTOR *accel)
{
    Object4c *obj;
    s32 speed;

    obj = DAT_801b77ec.unk2f04;
    func_80016928(scale, accel);
    obj->vel.vx += accel->vx;
    obj->vel.vy += accel->vy;
    obj->vel.vz += accel->vz;
    speed = func_80016c44(obj->vel.vx, obj->vel.vy, obj->vel.vz);
    if (max < speed) {
        obj->vel.vx = obj->vel.vx * max / speed;
        obj->vel.vy = obj->vel.vy * max / speed;
        obj->vel.vz = obj->vel.vz * max / speed;
    }
    obj->pos.vx += obj->vel.vx;
    obj->pos.vy += obj->vel.vy;
    obj->pos.vz += obj->vel.vz;
    if (height == -1) {
        return 0;
    }
    if (func_80053310(&obj->pos, radius, height) != 0) {
        return -1;
    }
    return 0;
}

/* Homing: turns the current record toward an Object88 record (unk0 bit 6) or the player, then func_80055bd8. */
ADDRESS(0x80055db0, 0x1c4)
s32 func_80055db0(s32 max, s32 scale, s32 turn, s32 radius, s32 height, s32 range, s32 near_scale,
                  s32 cone)
{
    Object4c *obj;
    VECTOR target;
    SVECTOR dir;
    SVECTOR accel;
    s32 dist;
    Object88 *found;

    obj = DAT_801b77ec.unk2f04;
    if (obj->unk0 & 0x40) {
        found = func_8004d3c4(&obj->pos, obj->angle.vy, obj->angle.vx, 24000, cone, cone, &dist, 0);
        if (found != NULL) {
            target.vx = found->pos.vx;
            target.vy = found->pos.vy - (found->unk1e >> 1);
            target.vz = found->pos.vz;
        aim:
            func_80016c90(target.vx - obj->pos.vx, target.vy - obj->pos.vy, target.vz - obj->pos.vz,
                          &dir);
            obj->angle.vx = func_80015f18(obj->angle.vx, dir.vx, turn);
            obj->angle.vy = func_80015f18(obj->angle.vy, dir.vy, turn);
        }
    } else {
        dist = func_8002b80c(&obj->pos, obj->angle.vy, 24000, 0x1000);
        if (dist != -1) {
            target.vx = DAT_801b24e4.x;
            target.vy = DAT_801b24e4.y - 1600;
            target.vz = DAT_801b24e4.z;
            goto aim;
        }
    }
    func_800167cc(&obj->angle, &accel);
    if (dist >= 0 && dist <= range) {
        scale = near_scale;
    }
    return func_80055bd8(scale, max, radius, height, &accel);
}

/* Steers the current record straight at target; -2 once within near, braking within slow. */
ADDRESS(0x80055f74, 0x1c0)
s32 func_80055f74(VECTOR *target, s32 max, s32 scale, s32 slow, s32 near, s32 radius, s32 height)
{
    Object4c *obj;
    VECTOR d;
    SVECTOR accel;
    s32 dist;

    obj = DAT_801b77ec.unk2f04;
    d.vx = target->vx - obj->pos.vx;
    d.vy = target->vy - obj->pos.vy;
    d.vz = target->vz - obj->pos.vz;
    dist = func_80016c44(d.vx, d.vy, d.vz);
    if (dist <= near) {
        return -2;
    }
    if (dist <= slow) {
        obj->vel.vx = func_80017158(0, obj->vel.vx, 0xc00);
        obj->vel.vy = func_80017158(0, obj->vel.vy, 0xc00);
        obj->vel.vz = func_80017158(0, obj->vel.vz, 0xc00);
    }
    accel.vx = (d.vx << 12) / dist;
    accel.vy = (d.vy << 12) / dist;
    accel.vz = (d.vz << 12) / dist;
    return func_80055bd8(scale, max, radius, height, &accel);
}

/* Grows the current record by grow, hits around it, frees it at limit. */
ADDRESS(0x80056134, 0xa4)
void func_80056134(s32 rate, s32 limit, s32 grow, s32 falloff, s32 mode)
{
    Object4c *obj;
    s32 r;

    obj = DAT_801b77ec.unk2f04;
    obj->scale.vy = obj->scale.vz = obj->scale.vx += grow;
    r = rate * obj->scale.vx >> 12;
    func_80053850(&obj->pos, r - (rate * grow >> 12), r - 1, mode, falloff, 0x1000);
    if (obj->scale.vx >= limit) {
        obj->unk0 = OBJECT4C_FREE;
    }
}

/* Spawns record 0x6e at vertex of obj, yaw jittered. */
ADDRESS(0x800561d8, 0x90)
void func_800561d8(Object4c *obj, s32 vertex)
{
    VECTOR pos;
    SVECTOR angle;
    Object4c *made;

    func_80053af4(obj, vertex, &pos, &obj->scale);
    angle.vx = obj->angle.vx;
    angle.vy = obj->angle.vy + (rand() >> 7) - 128;
    angle.vz = obj->angle.vz;
    made = func_80053c84(10, obj->unk0, 0x6e, &pos, &DAT_8009c33c, &angle);
    if (made != NULL) {
        made->unk7 = 2;
    }
}

/* Spawns record 0x70 on the floor below pos if pos is no more than height above it. */
ADDRESS(0x80056268, 0x88)
s32 func_80056268(VECTOR *pos, s32 a1, s32 a2, s32 height)
{
    VECTOR p;
    SVECTOR v;
    s32 floor;

    floor = DAT_801d4464.unk12018[0];
    if (pos->vy < floor) {
        return 0;
    }
    if (height + floor < pos->vy) {
        return 0;
    }
    p.vx = pos->vx;
    p.vz = pos->vz;
    p.vy = floor;
    func_80053c84(10, 0, 0x70, &p, &v, a1, a2);
    return 1;
}

/*
 * Spawns record 0x6f at vertex of obj (-1: its position). mode picks the
 * velocity and how many of the arguments after a7 it reads: -2 three random
 * ranges and bases, -1 a speed and a vertex and scale, -3 a speed, other
 * values are a speed backwards along the record's angle.
 */
ADDRESS(0x800562f0, 0x290)
void func_800562f0(Object4c *obj, s32 vertex, s32 mode, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7)
{
    s32 *args;
    SVECTOR dir;
    VECTOR off;
    VECTOR p;

    args = &a7;
    if (vertex == -1) {
        off.vx = off.vy = off.vz = 0;
    } else {
        func_80053a0c(obj, vertex, &off, &obj->scale);
    }
    switch (mode) {
    case -1:
        func_80053a0c(obj, args[2], &p, (SVECTOR *)args[3]);
        dir.vx = p.vx;
        dir.vy = p.vy;
        dir.vz = p.vz;
        mode = args[1];
        break;
    case -2:
        dir.vx = (rand() * args[1] >> 15) + args[2];
        dir.vy = (rand() * args[3] >> 15) + args[4];
        dir.vz = (rand() * args[5] >> 15) + args[6];
        goto spawn;
    case -3:
        dir.vx = dir.vy = dir.vz = 0;
        mode = args[1];
        break;
    default:
        func_800167cc(&obj->angle, &dir);
        func_80016928(30, &dir);
        dir.vx = -dir.vx;
        dir.vy = -dir.vy;
        dir.vz = -dir.vz;
        break;
    }
    dir.vx += obj->vel.vx * mode >> 12;
    dir.vy += obj->vel.vy * mode >> 12;
    dir.vz += obj->vel.vz * mode >> 12;
spawn:
    off.vx += obj->pos.vx;
    off.vy += obj->pos.vy;
    off.vz += obj->pos.vz;
    func_80053c84(10, 0, 0x6f, &off, &dir, a3, a4, a5, a6, a7, 1, 0);
}

/* Spawns count records 0x70 on the floor around pos, scattered by spread. */
ADDRESS(0x80056580, 0x154)
s32 func_80056580(VECTOR *pos, s32 count, s32 spread, s32 a3, s32 a4, s32 range)
{
    VECTOR p;
    SVECTOR v;
    s32 dx;
    s32 dz;
    s32 r;
    s32 floor;

    dx = 0;
    dz = 0;
    floor = DAT_801d4464.unk12018[0];
    if (pos->vy >= floor && floor + 500 >= pos->vy) {
        while (--count != -1) {
            p.vx = dx + pos->vx;
            p.vz = dz + pos->vz;
            p.vy = DAT_801d4464.unk12018[0];
            r = func_80017108(range) + 4096;
            func_80053c84(10, 0, 0x70, &p, &v, a3 * r >> 12, a4 * r >> 12);
            dx = (rand() * spread >> 14) - spread;
            dz = (rand() * spread >> 14) - spread;
        }
        return 1;
    }
    return 0;
}

/* Moves the current record one step, tests it (also half a step back when unkd), spins it. */
ADDRESS(0x800566d4, 0x12c)
s32 func_800566d4(s32 radius, s32 height, s32 spin)
{
    Object4c *obj;
    VECTOR p;
    s32 hit;

    obj = DAT_801b77ec.unk2f04;
    obj->pos.vx += obj->vel.vx;
    obj->pos.vy += obj->vel.vy;
    obj->pos.vz += obj->vel.vz;
    hit = func_80053310(&obj->pos, radius, height);
    if (obj->unkd != 0 && hit == 0) {
        p.vx = obj->pos.vx - (obj->vel.vx >> 1);
        p.vy = obj->pos.vy - (obj->vel.vy >> 1);
        p.vz = obj->pos.vz - (obj->vel.vz >> 1);
        hit = func_80053310(&p, radius, height);
    }
    obj->unka = DAT_801d4464.unk1200a == 0 ? 1 : 2;
    obj->angle.vz = (obj->angle.vz - spin) & 0xfff;
    return hit;
}

/* Moves the current record one step back. */
ADDRESS(0x80056800, 0x40)
void func_80056800(void)
{
    Object4c *obj;

    obj = DAT_801b77ec.unk2f04;
    obj->pos.vx -= obj->vel.vx;
    obj->pos.vy -= obj->vel.vy;
    obj->pos.vz -= obj->vel.vz;
}

/* Moves the current record half a step back. */
ADDRESS(0x80056840, 0x7c)
void func_80056840(void)
{
    Object4c *obj;

    obj = DAT_801b77ec.unk2f04;
    obj->pos.vx -= obj->vel.vx / 2;
    obj->pos.vy -= obj->vel.vy / 2;
    obj->pos.vz -= obj->vel.vz / 2;
}
