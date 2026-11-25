/*
 * GAME.EXE: Object4c movement (WIP module): steering and speed limits of
 * the current record, spawning further records from it, its step and
 * collision test, and func_800568bc, the per-frame update of the current
 * record by its id (Object4c.unk1).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/collision.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/object4c.h>

RODATA(0x80012ebc, 0x234)

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
    vector3s_scale_shift12(scale, accel);
    obj->vel.vx += accel->vx;
    obj->vel.vy += accel->vy;
    obj->vel.vz += accel->vz;
    speed = fixed_vector3_length(obj->vel.vx, obj->vel.vy, obj->vel.vz);
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
            vector_displacement_to_pitch_yaw(target.vx - obj->pos.vx, target.vy - obj->pos.vy,
                                             target.vz - obj->pos.vz, &dir);
            obj->angle.vx = angle_approach(obj->angle.vx, dir.vx, turn);
            obj->angle.vy = angle_approach(obj->angle.vy, dir.vy, turn);
        }
    } else {
        dist = func_8002b80c(&obj->pos, obj->angle.vy, 24000, 0x1000);
        if (dist != -1) {
            target.vx = DAT_801b24e4.pos.vx;
            target.vy = DAT_801b24e4.pos.vy - 1600;
            target.vz = DAT_801b24e4.pos.vz;
            goto aim;
        }
    }
    pitch_yaw_to_forward_vector(&obj->angle, &accel);
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
    dist = fixed_vector3_length(d.vx, d.vy, d.vz);
    if (dist <= near) {
        return -2;
    }
    if (dist <= slow) {
        obj->vel.vx = fixed_lerp_q12(0, obj->vel.vx, 0xc00);
        obj->vel.vy = fixed_lerp_q12(0, obj->vel.vy, 0xc00);
        obj->vel.vz = fixed_lerp_q12(0, obj->vel.vz, 0xc00);
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
        pitch_yaw_to_forward_vector(&obj->angle, &dir);
        vector3s_scale_shift12(30, &dir);
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
            r = random_centered_triangular_scaled(range) + 4096;
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

/*
 * Per-frame update of the current Object4c record (DAT_801b77ec.unk2f04),
 * switched on its id; obj->unk7 is the phase of the multi-step ids. Most
 * ids move the record, test it for hits and leave a particle trail through
 * func_80053c84; a record is freed by setting unk0 to OBJECT4C_FREE.
 *
 * Stack frame: a case block opened at a case label stays open to the end
 * of the switch (the closing braces at the bottom), so its locals keep
 * their slots for the rest of the switch while locals of inner blocks are
 * freed and reused (gcc temp-slot levels; locals only passed as stack
 * arguments are otherwise freed with their block). The spare* locals are
 * never referenced; they keep the retail slot layout.
 */
ADDRESS(0x800568bc, 0x5338)
void func_800568bc(void)
{
    Object4c *obj;
    s32 id;
    s32 phase;
    s32 hit;
    s32 i;
    VECTOR p;
    VECTOR spare;

    obj = DAT_801b77ec.unk2f04;
    id = obj->unk1;
    phase = obj->unk7;
    switch (id) {
    case 0:
    case 40:
    case 62:
    case0:
        if (phase == 0) {
            hit = func_800566d4(180, 0x80000000, -300);
            if (hit == 0) {
                break;
            }
            func_80056580(&obj->pos, 3, 400, 0x2000, 0x2000, 0x400);
            /* XOR, not a square: the retail code takes the root of dx ^ 2. */
            if (SquareRoot0((obj->pos.vx - DAT_801b24e4.pos.vx) ^ 2) < 1024U) {
                func_80056840();
                goto hit86b;
            }
            goto hit86;
        }
        if (phase < 3) {
            goto grow86;
        }
        obj->unk0 = OBJECT4C_FREE;
        break;
    case 3:
        obj->scale.vy += 1200;
        obj->unk4 = (obj->unk10 & 1) - 128;
        obj->unke = (obj->unk10 % 2) << 6;
        break;
    case 6:
    case 51:
        if (obj->arg.b.unk44 == 2) {
            obj->unk12 += 256;
            if (obj->unk12 >= 0x1000) {
                obj->unk0 = OBJECT4C_FREE;
            }
            break;
        }
        if (obj->arg.b.unk44 == 1) {
            obj->vel.vy += 13;
        }
        obj->angle.vx += 200;
        hit = func_800566d4(500, 500, 250);
        if (hit != 0) {
            func_800537e8(hit);
            if (obj->arg.b.unk44 == 1) {
                obj->arg.b.unk44 = 2;
                obj->unk9 = 5;
                obj->unkc = 0x32;
                obj->unk12 = 0x400;
                break;
            }
            func_80056800();
            obj->arg.b.unk44 = 1;
            obj->vel.vz = 0;
            obj->vel.vx = 0;
            obj->vel.vy = -100;
        }
        func_80056268(&obj->pos, 0x4000, 0x4000, 500);
        break;
    case 15:
        {
            SVECTOR v;
            VECTOR at;

            hit = func_800566d4(180, 0, 0);
            if (hit != 0) {
                if (hit & 3) {
                    func_80056800();
                    obj->unk0 = OBJECT4C_FREE;
                }
                if (obj->arg.b.unk44 == 0) {
                    obj->arg.b.unk44 = 1;
                    func_800537e8(hit);
                }
            } else {
                obj->arg.b.unk44 = 0;
            }
            at.vx = v.vx + obj->pos.vx;
            at.vy = v.vy + obj->pos.vy;
            at.vz = v.vz + obj->pos.vz;
            func_80053850(&at, 0, obj->scale.vx * 512 >> 12, obj->scale.vy * 2048 >> 12, 0x400, 0x1000);
            v.vx = rand() * 512 / 32767 - 256;
            v.vy = rand() * -256 / 32767;
            v.vz = rand() * 512 / 32767 - 256;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 9999, -128, 5, 22, 0, 1, 1);
            obj->angle.vy += 350;
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            break;
    case 75:
        {
            obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
            obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
            obj->angle.vz += 200;
            hit = func_800566d4(180, 0, 0);
            if (hit != 0) {
                if (hit & 0xf) {
                    obj->unk0 = OBJECT4C_FREE;
                } else {
                    func_800537e8(hit);
                    obj->unk0 = OBJECT4C_FREE;
                }
                func_800532d4(obj, 0x27);
                func_80053c84(10, 3, 0x58, &obj->pos, NULL, 0, 2);
                func_80053c84(10, 3, 0x58, &obj->pos, NULL, 0x40, 2);
                func_80053c84(10, 3, 0x58, &obj->pos, NULL, 0x80, 2);
            }
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            {
                SVECTOR v;

                v.vx = v.vy = v.vz = 0;
                if (obj->unk10 % 3 == 0) {
                    func_80053c84(10, 0, 3, &obj->pos, &v);
                }
            }
            break;
    case 38:
    case 39:
        if (obj->arg.s.unk44 != 0) {
            obj->scale.vx += 150;
            obj->scale.vz += 150;
            obj->arg.s.unk44--;
        }
        if (obj->scale.vy < 0x1000) {
            obj->scale.vy += 150;
        }
        if (obj->unk10 > 32) {
            obj->pos.vy -= 40;
        } else {
            obj->pos.vy += 85;
        }
        obj->unk0 = 0x81;
        func_80053850(&obj->pos, 0, obj->scale.vx, 0x8000, 0x400, 0x1000);
        obj->unk0 = 0x84;
        if (func_800566d4(100, 200, 0) != 0) {
            obj->unk0 = OBJECT4C_FREE;
        }
        break;
    case 80:
    case 81:
    case 83:
        {
            VECTOR q;
            s32 t;
            s32 g;
            s32 y;

            g = 10;
            if (phase != 0) {
                break;
            }
            if (obj->arg.s.unk46 == 0) {
                audio_play_spatial_range(4, &obj->pos, 0x6e, 0x6d60, 0x7148, 0);
            }
            t = ++obj->arg.s.unk46;
            y = obj->pos.vy;
            p.vy = obj->arg.s.unk44 + obj->vel.vy * t + (t * g * t >> 1);
            p.vx = obj->vel.vx + obj->pos.vx;
            p.vz = obj->vel.vz + obj->pos.vz;
            hit = func_80053310(&p, 20, 20);
            if (hit == 0) {
                q.vx = (obj->vel.vx >> 1) + obj->pos.vx;
                q.vy = obj->pos.vy;
                q.vz = (obj->vel.vz >> 1) + obj->pos.vz;
                hit = func_80053310(&q, 20, 20);
                if (hit == 0) {
                    q.vx = (obj->vel.vx >> 2) + obj->pos.vx;
                    q.vy = obj->pos.vy;
                    q.vz = (obj->vel.vz >> 2) + obj->pos.vz;
                    hit = func_80053310(&q, 20, 20);
                }
            }
            obj->pos.vx = p.vx;
            obj->pos.vy = p.vy;
            obj->pos.vz = p.vz;
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            if (hit != 0) {
                func_800537e8(hit | 0x20000);
                obj->unk0 = OBJECT4C_FREE;
                break;
            }
            obj->unka = DAT_801d4464.unk1200a == 0 ? 1 : 2;
            vector_displacement_to_pitch_yaw(obj->vel.vx, obj->pos.vy - y, obj->vel.vz, &obj->angle);
            break;
    case 82:
        {
            VECTOR q;
            VECTOR np;
            s32 t;
            s32 g;
            s32 y;

            g = 5;
            if (phase != 0) {
                break;
            }
            if (obj->vel.vx > 0 || obj->vel.vz > 0) {
                func_80055db0(300, 10, 0x100, 20, 360, 0x2000, 0x30, 0x800);
            }
            if (obj->arg.s.unk46 == 0) {
                audio_play_spatial_range(4, &obj->pos, 0x6e, 0x6d60, 0x7148, 0);
            }
            t = ++obj->arg.s.unk46;
            y = obj->pos.vy;
            np.vy = obj->vel.vy + y + g * (t >> 1);
            np.vx = obj->vel.vx + obj->pos.vx;
            np.vz = obj->vel.vz + obj->pos.vz;
            hit = func_80053310(&np, 20, 20);
            if (hit == 0) {
                q.vx = (np.vx + obj->pos.vx) >> 1;
                q.vy = (np.vy + obj->pos.vy) >> 1;
                q.vz = (np.vz + obj->pos.vz) >> 1;
                hit = func_80053310(&q, 20, 20);
            }
            obj->pos.vx = np.vx;
            obj->pos.vy = np.vy;
            obj->pos.vz = np.vz;
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            if (hit != 0) {
                func_800537e8(hit | 0x20000);
                obj->unk0 = OBJECT4C_FREE;
                break;
            }
            obj->unka = DAT_801d4464.unk1200a == 0 ? 1 : 2;
            vector_displacement_to_pitch_yaw(obj->vel.vx, obj->pos.vy - y, obj->vel.vz, &obj->angle);
            break;
    case 86:
        if (phase == 0) {
            hit = func_800566d4(180, 0, -300);
            if (hit == 0) {
                func_80056268(&obj->pos, 0x2000, 0x2000, 500);
                break;
            }
        hit86:
            func_80056800();
        hit86b:
            func_800537e8(hit);
            obj->unk7 = 1;
        } else if (phase >= 3) {
            obj->unk0 = OBJECT4C_FREE;
            break;
        }
    grow86:
        obj->unk4 = phase - 128;
        obj->scale.vz = obj->scale.vy = obj->scale.vx += 2048;
        obj->unk7++;
        break;
    case 85:
        if (phase == 9) {
            Object4cTrail spare2;
            VECTOR a;
            VECTOR c;
            VECTOR old;
            Object88 *o;
            VECTOR *b;
            VECTOR *pos;

            o = &DAT_80185da8.objects[obj->arg.s.unk44];
            if (o->unk9 != 1) {
                obj->unk0 = OBJECT4C_FREE;
                break;
            }
            if (o->unke != 0x19) {
                obj->unk0 = OBJECT4C_FREE;
                break;
            }
            func_8004ede0(o, obj->arg.s.unk46, &a);
            b = func_8004eee0(o, &c);
            pos = &obj->pos;
            old = *pos;
            pos->vx = a.vx + b->vx;
            pos->vy = a.vy + b->vy;
            pos->vz = a.vz + b->vz;
            obj->scale.vx += 384;
            if (obj->scale.vx > 0x1800) {
                obj->scale.vx = 0x1800;
            }
            obj->scale.vy = obj->scale.vz = obj->scale.vx;
            obj->vel.vx = obj->pos.vx - old.vx;
            obj->vel.vy = obj->pos.vy - old.vy;
            obj->vel.vz = obj->pos.vz - old.vz;
            func_800562f0(obj, -1, -700, obj->scale.vx, -300, 3, phase, 0);
            if (o->unk18 < obj->arg.h.unk48) {
                break;
            }
            func_800532d4(obj, 0x33);
            func_8004f1d0(o, &DAT_801b24e4.pos, 650, pos, &obj->vel, -1, 0x400, 5);
            obj->unk7 = 0;
            obj->unk10 = 50;
            break;
        }
        func_800562f0(obj, -1, 0x100, obj->scale.vx, -300, 2, 9, 0);
        goto case0;
    case 13:
    case 56:
        hit = func_800566d4(180, 0x80000000, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->unk0 = OBJECT4C_FREE;
            }
            if (obj->arg.b.unk44 == 0) {
                obj->arg.b.unk44 = 1;
                func_800537e8(hit);
            }
        } else {
            obj->arg.b.unk44 = 0;
        }
        func_80056268(&obj->pos, 0x2000, 0x2000, 500);
        obj->angle.vy += 700;
        break;
    case 52:
        hit = func_800566d4(180, 0x80000000, 0);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                func_800537e8(hit);
            }
            obj->unk0 = OBJECT4C_FREE;
        }
        func_80056268(&obj->pos, 0x2000, 0x2000, 500);
        obj->angle.vy += 300;
        break;
    case 76:
        {
            SVECTOR v;

            obj->angle.vy += 200;
            obj->angle.vx += 200;
            obj->vel.vy += 5;
            v.vx = v.vy = v.vz = 0;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0xc00, -256, 8, 0x1c, 0, 1, 2);
            hit = func_800566d4(180, 0x80000000, 0);
            if (hit != 0) {
                obj->unk0 = OBJECT4C_FREE;
                func_800537e8(hit);
            }
            break;
    case 64:
        hit = func_800566d4(180, 0x80000000, -500);
        if (hit != 0) {
            SVECTOR v;

            obj->unk0 = OBJECT4C_FREE;
            func_800537e8(hit);
            for (i = 15; i != -1; i--) {
                v.vx = (rand() * 500 >> 15) - 250;
                v.vy = (rand() * 500 >> 15) - 250;
                v.vz = (rand() * 500 >> 15) - 250;
                func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1400, -24, 10, 0x26, 0, 1, 2);
            }
        }
        break;
    case 70:
        {
            SVECTOR v;

            obj->angle.vz += 400;
            obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
            obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
            if (obj->unk10 == 45 && obj->arg.s.unk46 == 0) {
                v.vx = (-obj->vel.vz + obj->vel.vx + obj->vel.vx) >> 1;
                v.vz = (obj->vel.vx + obj->vel.vz + obj->vel.vz) >> 1;
                v.vy = obj->vel.vy;
                func_80053c84(10, 0x43, 0x46, &obj->pos, &v, 1);
                v.vx = (obj->vel.vz + obj->vel.vx + obj->vel.vx) >> 1;
                v.vz = (-obj->vel.vx + obj->vel.vz + obj->vel.vz) >> 1;
                v.vy = obj->vel.vy;
                func_80053c84(10, 0x43, 0x46, &obj->pos, &v, 1);
            }
            hit = func_800566d4(180, 0x80000000, 0);
            if (hit != 0) {
                func_800537e8(hit);
                obj->unk1 = 0x58;
                obj->unk8 = 1;
                obj->unk9 = 5;
                obj->unk2 = 0x15;
                obj->unk3 = 0x15;
                obj->unkc = 0x34;
                obj->unk12 = 0x1000;
                obj->unk10 = 40;
                obj->unk4 = 0;
                obj->scale.vx = 0;
                obj->scale.vy = 0;
                obj->scale.vz = 0;
                obj->angle.vx = 0;
                obj->angle.vy = 0;
                obj->angle.vz = 0;
                obj->unke = 0x40;
                func_800532d4(obj, 0x28);
                func_80053c84(10, 3, 0x58, &obj->pos, NULL, 0x80, 1);
                break;
            }
            v.vx = (rand() * 20 >> 15) - 10;
            v.vy = (rand() * 20 >> 15) - 10;
            v.vz = (rand() * 20 >> 15) - 10;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1000, -640, 5, 12, 0, 1, 0);
            break;
    case 44:
        {
            SVECTOR v;

            obj->angle.vz += 400;
            obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
            obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
            hit = func_800566d4(180, 0x80000000, 0);
            if (hit != 0) {
                func_800537e8(hit);
                obj->unk0 = OBJECT4C_FREE;
            }
            v.vx = (rand() * 20 >> 15) - 10;
            v.vy = (rand() * 20 >> 15) - 10;
            v.vz = (rand() * 20 >> 15) - 10;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1400, -256, 4, 0x16, 0, 2, 5);
            break;
    case 49:
    case 54:
    case 55:
        {
            SVECTOR v;

            hit = func_800566d4(180, 0x80000000, 0);
            if (hit != 0) {
                if (hit & 0xf) {
                    obj->unk0 = OBJECT4C_FREE;
                }
                if (obj->arg.b.unk44 == 0) {
                    obj->arg.b.unk44 = 1;
                    func_800537e8(hit);
                    for (i = 6; i != -1; i--) {
                        v.vx = (rand() * 240 >> 15) - 120;
                        v.vy = (rand() * 240 >> 15) - 120;
                        v.vz = (rand() * 240 >> 15) - 120;
                        func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1400, 0, 7, 12, 0, 1, 1);
                    }
                }
            } else {
                obj->arg.b.unk44 = 0;
            }
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            break;
    case 60:
        switch (obj->arg.s.unk44) {
        case 0:
            if (func_800566d4(180, 0x80000000, 0) != 0) {
                obj->arg.s.unk44++;
            }
            obj->vel.vy += 5;
            break;
        case 1:
            obj->scale.vx += 256;
            obj->scale.vz += 256;
            if (obj->scale.vx > 0x1000) {
                obj->unk0 = OBJECT4C_FREE;
            }
            break;
        }
        break;
    case 7:
    case 22:
        obj->scale.vz = obj->scale.vx += obj->arg.s.unk46;
        if (obj->scale.vx >= 768) {
            VECTOR p;

            p.vx = obj->pos.vx;
            p.vy = obj->pos.vy + 1000;
            p.vz = obj->pos.vz;
            func_80053850(&p, (obj->scale.vx - obj->arg.s.unk46) * 4, obj->scale.vx * 4 - 1, 2000, 0x1000,
                          obj->arg.s.unk48);
        }
        if (obj->scale.vx > obj->arg.s.unk44) {
            obj->unk0 = OBJECT4C_FREE;
        }
        break;
    case 12:
        if (obj->arg.s.unk44 == 0) {
            func_80033f38(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x400, obj->scale.vx, 0x30);
            if (DAT_801d4464.unk12024 == 0x10) {
                func_800536f8(0x10010, 5000, NULL);
            }
            obj->arg.s.unk44++;
            func_80053c84(10, 0, 0x65, &obj->pos, NULL, DAT_801d4464.unk12028);
            obj->arg.s.unk48 = 0;
            obj->arg.s.unk4a = 0;
        }
        obj->angle.vy += 420;
        break;
    case 101:
        {
            SVECTOR v;

            if (obj->arg.s.unk44 < 12) {
                obj->scale.vy += 440;
            }
            if (obj->arg.s.unk44 >= 48) {
                obj->scale.vy -= 440;
                if (obj->arg.s.unk46 != -1) {
                    Object88 *t;

                    t = &DAT_80185da8.objects[obj->arg.s.unk46];
                    t->unk28 &= ~0x800;
                }
            }
            if (obj->arg.s.unk44 >= 12 && obj->arg.s.unk44 < 48) {
                v.vx = rand() * 512 / 32767 - 256;
                v.vy = rand() * -256 / 32767;
                v.vz = rand() * 512 / 32767 - 256;
                func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1000, -128, 5, 12, 0, 1, 0);
            }
            obj->angle.vy -= 400;
            obj->arg.s.unk44++;
            break;
    case 102:
        hit = func_800566d4(180, 0, -600);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                func_800537e8(hit);
            }
            obj->unk0 = OBJECT4C_FREE;
        }
        break;
    case 50:
        obj->scale.vx += 32;
        obj->scale.vy += 32;
        obj->scale.vz += 32;
        hit = func_800566d4(180, 0, -300);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                func_800537e8(hit);
            }
            obj->unk0 = OBJECT4C_FREE;
        }
        break;
    case 58:
        obj->unke = (obj->unke + 64) & 0x40;
        hit = func_800566d4(180, 0, -600);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->unk0 = OBJECT4C_FREE;
            }
            func_800537e8(hit);
            obj->unk0 = OBJECT4C_FREE;
        }
        break;
    case 21:
        {
            SVECTOR v;
            VECTOR ring;

            v.vx = (rand() * 32 >> 15) - 16;
            v.vy = (rand() * 32 >> 15) - 16;
            v.vz = (rand() * 32 >> 15) - 16;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1a90, -1000, 4, 0x25, 0, 1, 1);
            hit = func_800566d4(180, 0, 0);
            if (hit == 0) {
                break;
            }
            if (hit & 0xf) {
                obj->unk0 = OBJECT4C_FREE;
            }
            if (!(hit & 0x10)) {
                break;
            }
            func_80053c84(10, 0x43, 0x58, &obj->pos, NULL, 0, 1);
            func_80053c84(10, 0x43, 0x58, &obj->pos, NULL, 0x80, 1);
            func_800532d4(obj, 0x25);
            for (i = 0; i < 4096; i += 256) {
                v.vx = rsin(i) * 360 >> 12;
                v.vz = rcos(i) * 360 >> 12;
                v.vy = 0;
                ring.vx = obj->pos.vx;
                ring.vy = obj->pos.vy + 1200;
                ring.vz = obj->pos.vz;
                func_80053c84(10, 0, 0x5b, &ring, &v);
            }
            break;
    case 11:
        hit = func_800566d4(180, 0, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->unk0 = OBJECT4C_FREE;
            }
            if (obj->arg.b.unk44 == 0) {
                func_800537e8(hit);
                obj->arg.b.unk44 = 1;
            }
        } else {
            obj->arg.b.unk44 = 0;
        }
        break;
    case 9:
        hit = func_800566d4(250, 0x80000000, 0);
        if (hit != 0 && (hit & 0xf)) {
            obj->unk0 = OBJECT4C_FREE;
        }
        if (obj->arg.s.unk44 >= 11 && obj->arg.s.unk44 % 6 == 0) {
            func_80053c84(10, 0x42, 0x5e, &obj->pos, NULL, obj->arg.s.unk46);
        }
        obj->unk14 = (obj->unk14 + 128) & 0xfff;
        obj->arg.s.unk44++;
        break;
    case 17:
        {
            SVECTOR v;

            hit = func_800566d4(250, 0x80000000, 0);
            if (hit != 0) {
                if (hit & 0xf) {
                    obj->unk0 = OBJECT4C_FREE;
                }
                func_800537e8(hit);
            }
            if (obj->arg.s.unk44 >= 16 && obj->arg.s.unk44 % 10 == 0) {
                v.vy = 84;
                v.vx = 0;
                v.vz = 0;
                func_80053c84(10, 0x42, 0x59, &obj->pos, &v);
            }
            obj->unk14 = (obj->unk14 + 128) & 0xfff;
            obj->arg.s.unk44++;
            break;
    case 89:
        obj->angle.vy += 320;
        switch (obj->arg.s.unk44) {
        case 0:
            hit = func_800566d4(180, 0x80000000, 0);
            if (hit != 0 && (hit & 0xf)) {
                obj->arg.s.unk44++;
                func_800532d4(obj, 0x27);
            }
            break;
        case 1:
            obj->pos.vy -= 160;
            obj->arg.s.unk44++;
            break;
        case 2:
            obj->scale.vx += 640;
            obj->scale.vz += 640;
            if (obj->scale.vx > 14000) {
                obj->unk0 = OBJECT4C_FREE;
            }
            if (obj->scale.vx >= 768) {
                VECTOR p;

                p.vx = obj->pos.vx;
                p.vy = obj->pos.vy + 1000;
                p.vz = obj->pos.vz;
                func_80053850(&p, obj->scale.vx * 29 >> 5, obj->scale.vx * 35 >> 5, 0x800, 0x400, 0x1000);
            }
            break;
        }
        break;
    case 72:
        if (obj->arg.c.unk44 == 0) {
            func_80056134(0x3800, 504, 70, 0x800, 0x8000);
        } else {
            obj->arg.c.unk44--;
        }
        break;
    case 122:
        if (func_80055db0(600, 40, 36, 180, 360, 0x1000, 0x104, 0x800) != -1 && obj->unk10 >= 2) {
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            break;
        }
        func_80056800();
        func_80053c84(10, obj->unk0 | 3, 0x48, &obj->pos, NULL, 0);
        func_80053c84(10, obj->unk0 | 3, 0x48, &obj->pos, NULL, 1);
        func_80053c84(10, obj->unk0 | 3, 0x48, &obj->pos, NULL, 2);
        func_800532d4(obj, 0x27);
        obj->unk0 = OBJECT4C_FREE;
        break;
    case 120:
        if (func_800566d4(180, 360, 0) == 0 && obj->unk10 >= 2) {
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            break;
        }
        func_80053c84(10, obj->unk0 | 3, 0x48, &obj->pos, NULL, 0);
        func_80053c84(10, obj->unk0 | 3, 0x48, &obj->pos, NULL, 1);
        func_80053c84(10, obj->unk0 | 3, 0x48, &obj->pos, NULL, 2);
        func_800532d4(obj, 0x27);
        obj->unk0 = OBJECT4C_FREE;
        break;
    case 92:
    case 93:
        {
            Object4c *link;
            s32 d;

            obj->pos.vx += obj->vel.vx;
            obj->pos.vz += obj->vel.vz;
            link = &DAT_801b77ec.objects[obj->arg.c.unk45];
            switch (obj->arg.c.unk44) {
            case 0:
                func_800537e8(func_80033f38(obj->pos.vx, obj->pos.vy, obj->pos.vz, 10, obj->scale.vy, 0x30));
                func_80033b10(obj->pos.vx, link->pos.vy, obj->pos.vz, 0, 0);
                obj->pos.vy = DAT_801d4464.unk12010;
                d = DAT_801d4464.unk12010 - DAT_801d4464.unk12014;
                obj->scale.vy = d > 0x7fff ? 0x4000 : d >> 1;
                if (link->unk0 == OBJECT4C_FREE) {
                    obj->arg.c.unk44 = 1;
                    obj->arg.s.unk46 = 0;
                    obj->scale.pad = obj->scale.vy;
                }
                break;
            case 1:
                obj->scale.vy = fixed_lerp_q12(obj->scale.pad, 0, obj->arg.s.unk46);
                obj->arg.s.unk46 += 512;
                if (obj->arg.s.unk46 >= 0x1000) {
                    obj->unk0 = OBJECT4C_FREE;
                }
                break;
            }
            break;
    case 68:
    case 69:
        func_80056134(0x4000, obj->arg.s.unk44, 70, 0x800, 0x8000);
        break;
    case 123:
        {
            VECTOR mid;

            mid.vx = (obj->vel.vx >> 1) + obj->pos.vx;
            mid.vy = (obj->vel.vy >> 1) + obj->pos.vy;
            mid.vz = (obj->vel.vz >> 1) + obj->pos.vz;
            obj->pos.vx += obj->vel.vx;
            obj->pos.vy += obj->vel.vy;
            obj->pos.vz += obj->vel.vz;
            if (func_80033d08(mid.vx, mid.vy, mid.vz, 5, 10) != 0 ||
                func_80033d08(obj->pos.vx, obj->pos.vy, obj->pos.vz, 5, 10) != 0) {
                obj->unk0 = OBJECT4C_FREE;
            }
            if (obj->unk0 & 0x40) {
                func_80053c84(10, obj->unk0, 0x45, &obj->pos, NULL, 420);
            } else {
                func_80053c84(obj->unk6, obj->unk0, 0x45, &obj->pos, NULL, 420);
            }
            break;
    case 124:
        {
            Object88 *o;
            s32 n;

            obj->angle.vy += 400;
            obj->pos.vx += obj->vel.vx;
            obj->pos.vy += obj->vel.vy;
            obj->pos.vz += obj->vel.vz;
            p.vx = obj->pos.vx;
            p.vy = obj->pos.vy + 5000;
            p.vz = obj->pos.vz;
            n = func_8004d644(p.vx, p.vy, p.vz, 100, 10000);
            if (n != -1) {
                o = &DAT_80185da8.objects[n];
                func_800532d4(obj, 0x27);
                func_80053c84(10, obj->unk0, 0x44, &o->pos, NULL, 1260);
            } else {
                func_80033f38(p.vx, p.vy, p.vz, 100, 10000, 0xb0);
                if (DAT_801d4464.unk12024 == 0x80) {
                    VECTOR q;

                    q = DAT_801b24e4.pos;
                    q.vy -= 1700;
                    func_800532d4(obj, 0x27);
                    func_80053c84(obj->unk6, obj->unk0, 0x44, &DAT_801b24e4.pos, NULL, 1260);
                }
            }
            func_80056268(&p, 0x2000, 0x7fff, 10000);
            break;
    case 66:
        hit = func_800566d4(100, 200, 0);
        if (hit == 0) {
            func_80056268(&obj->pos, 0x2000, 0x2000, 500);
            break;
        }
        func_800537e8(hit);
        obj->unk0 = OBJECT4C_FREE;
        break;
    case 31:
        {
            SVECTOR v;

            obj->unk0 = 0x81;
            if (obj->arg.s.unk44 == 0) {
                obj->arg.s.unk46 -= 10;
            } else {
                obj->arg.s.unk46 += 10;
            }
            obj->pos.vy += obj->arg.s.unk46;
            if (obj->arg.s.unk46 < -69 && obj->arg.s.unk44 == 0) {
                obj->arg.s.unk44 = 1;
            }
            if (obj->arg.s.unk46 >= 70 && obj->arg.s.unk44 == 1) {
                obj->arg.s.unk44 = 0;
            }
            func_80053850(&obj->pos, 0, obj->scale.vx >> 4, 0x8000, 0x400, 0x1000);
            obj->unk0 = 0x84;
            if (func_800566d4(100, 200, 0) != 0) {
                obj->unk0 = OBJECT4C_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 2, 5);
            break;
    case 34:
        {
            SVECTOR v;

            obj->unk0 = 0x81;
            func_80053850(&obj->pos, 0, obj->scale.vx >> 4, 0x8000, 0x400, 0x1000);
            obj->unk0 = 0x84;
            if (func_800566d4(100, 200, 0) != 0) {
                func_80056800();
                obj->unk0 = OBJECT4C_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 4);
            break;
    case 32:
        {
            SVECTOR v;

            obj->unk0 = 0x81;
            func_80053850(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->unk0 = 0x84;
            if (func_800566d4(100, 200, 0) != 0) {
                func_80056800();
                obj->unk0 = OBJECT4C_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 5);
            obj->angle.vz += 300;
            break;
    case 37:
        {
            SVECTOR v;

            obj->unk0 = 0x81;
            func_80053850(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->unk0 = 0x84;
            if (func_800566d4(100, 200, 0) != 0) {
                func_80056800();
                obj->unk0 = OBJECT4C_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 7);
            obj->angle.vz += 300;
            break;
    case 35:
    case 36:
        {
            SVECTOR v;

            obj->unk0 = 0x81;
            func_80053850(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->unk0 = 0x81;
            if (func_800566d4(100, 200, 0) != 0) {
                func_80056800();
                obj->unk0 = OBJECT4C_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 3);
            break;
    case 33:
        {
            SVECTOR v;

            obj->unk0 = 0x81;
            func_80053850(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->unk0 = 0x81;
            if (func_800566d4(100, 200, 0) != 0) {
                func_80056800();
                obj->unk0 = OBJECT4C_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 8);
            break;
    case 113:
    case 125:
    case 129:
        {
            VECTOR at;
            SVECTOR v;
            s32 d;

            if (obj->unk7 == 2) {
                obj->scale.vy = obj->scale.vx -= 128;
                if (obj->scale.vx <= 0) {
                    obj->unk0 = OBJECT4C_FREE;
                }
                break;
            }
            hit = func_800566d4(50, 0x80000000, -300);
            if (hit != 0) {
                if (hit & 0xf) {
                    if (phase == 0) {
                        func_80056800();
                        obj->vel.vx = obj->vel.vz = 0;
                    } else {
                        func_80056800();
                        func_80033b10(obj->pos.vx, obj->pos.vy, obj->pos.vz, 50, 0);
                        if (DAT_801d4464.unk12010 < DAT_801d4464.unk12018[0]) {
                            at.vy = DAT_801d4464.unk12010;
                        } else {
                            at.vy = DAT_801d4464.unk12018[0];
                        }
                        d = at.vy - obj->pos.vy;
                        goto burst;
                    }
                }
                obj->unk7 = 1;
            }
            if (phase == 1) {
                obj->vel.vy -= 70;
                obj->vel.vx = fixed_lerp_q12(0, obj->vel.vx, 3000);
                obj->vel.vz = fixed_lerp_q12(0, obj->vel.vz, 3000);
                if (DAT_801d4464.unk12010 < DAT_801d4464.unk12018[0]) {
                    at.vy = DAT_801d4464.unk12010;
                } else {
                    at.vy = DAT_801d4464.unk12018[0];
                }
                d = at.vy - obj->pos.vy;
                if (d >= 7000) {
                burst:
                    obj->unk7 = 2;
                    at.vx = obj->pos.vx;
                    at.vz = obj->pos.vz;
                    switch (id) {
                    case 113:
                        func_80053c84(10, obj->unk0 | 3, 0x72, &at, &v, d);
                        func_80053c84(10, obj->unk0, 0x16, &at, &v, d >> 1, d >> 4, 0x800, 1);
                        break;
                    case 125:
                        func_80053c84(10, obj->unk0 | 3, 0x7e, &at, &v, d);
                        func_80053c84(10, obj->unk0, 0x16, &at, &v, d >> 1, d >> 4, 0x800, 3);
                        break;
                    case 129:
                        func_80053c84(10, obj->unk0 | 3, 0x80, &at, &v, d);
                        func_80053c84(10, obj->unk0, 0x16, &at, &v, d >> 1, d >> 4, 0x800, 0);
                        break;
                    }
                    func_800532d4(obj, 0x27);
                }
            } else if (phase == 0) {
                if (obj->arg.s.unk44-- <= 0) {
                    obj->unk7 = 1;
                }
            }
            for (i = 3; i != -1; i--) {
                v.vx = (rand() >> 7) - 128;
                v.vy = (rand() >> 7) - 128;
                v.vz = (rand() >> 7) - 128;
                func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x800, -128, 5, 12, 0, 1, 1);
            }
            break;
    case 114:
    case 126:
    case 128:
        {
            s32 d;
            s32 r;

            if (phase < 3) {
                SVECTOR v;
                SVECTOR spare3;

                r = obj->scale.vy;
                r = SquareRoot12(SquareRoot12(r * (r * r >> 12))) >> 3;
                switch (id) {
                case 114:
                    func_80053c84(10, obj->unk0 | 3, 0x13, &obj->pos, &v, r);
                    break;
                case 126:
                    func_80053c84(10, obj->unk0 | 3, 0x29, &obj->pos, &v, r);
                    break;
                case 128:
                    func_80053c84(10, obj->unk0 | 3, 0x2d, &obj->pos, &v, r);
                    break;
                }
            }
            obj->unk4 = (phase & 1) - 128;
            if (obj->unk7 < 9) {
                d = fixed_vector3_length(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vy - obj->pos.vy,
                                         DAT_801b24e4.pos.vz - obj->pos.vz);
                if (d <= 32000) {
                    d = ((rsin(obj->unk7 << 8) >> 1) + 1024) * (32000 - d) / 32000;
                    func_8003456c(200, 180, 160, 0, 32000, d);
                }
            }
            obj->unk7++;
            break;
    case 19:
    case 41:
    case 45:
        {
            SVECTOR v;

            func_80056134(0x3800, obj->arg.s.unk44, 0x80, 0x400, 0x8000);
            v.vx = (rand() >> 6) - 256;
            v.vz = (rand() >> 6) - 256;
            v.vy = -(rand() >> 7) - 128;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x2000, -128, 15, 12, 10, 1, 1);
            obj->angle.vy += 64;
            break;
    case 42:
        hit = func_800566d4(180, 0, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->unk0 = OBJECT4C_FREE;
            }
            if (obj->arg.b.unk44 == 0) {
                obj->arg.b.unk44 = 1;
                func_800537e8(hit);
            }
        } else {
            obj->arg.b.unk44 = 0;
        }
        func_80056268(&obj->pos, 0x2000, 0x2000, 500);
        break;
    case 48:
    case 53:
        obj->scale.vx += 32;
        obj->scale.vy += 32;
        obj->scale.vz += 32;
        obj->angle.vz += 200;
        hit = func_800566d4(180, 0, 0);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                func_800537e8(hit);
            }
            obj->unk0 = OBJECT4C_FREE;
        }
        func_80056268(&obj->pos, 0x2000, 0x2000, 500);
        break;
    case 46:
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        hit = func_800566d4(180, 0, 0);
        if (hit != 0) {
            func_800537e8(hit);
            obj->unk1 = 0x58;
            obj->unk8 = 1;
            obj->unk9 = 5;
            obj->unk2 = 0x15;
            obj->unk3 = 0x15;
            obj->unkc = 0x34;
            obj->unk12 = 0x1000;
            obj->unk10 = 40;
            obj->unk4 = 0;
            obj->scale.vx = 0;
            obj->scale.vy = 0;
            obj->scale.vz = 0;
            obj->unke = 0x40;
            break;
        }
        func_80056268(&obj->pos, 0x2000, 0x2000, 500);
        if ((obj->unk0 & 0x40) && obj->vel.vy < 800) {
            obj->vel.vy += 10;
        }
        break;
    case 100:
        switch (obj->arg.s.unk44) {
        case 0:
            obj->arg.s.unk46 = 0;
            obj->arg.s.unk44++;
            break;
        case 1:
            if (obj->arg.s.unk46 & 1) {
                obj->vel.vx = obj->vel.vx + (rand() >> 6) - 256;
                func_80053c84(10, 0x81, 0x2e, &obj->pos, &obj->vel);
            }
            if (++obj->arg.s.unk46 >= 7) {
                obj->unk0 = OBJECT4C_FREE;
            }
            break;
        }
        break;
    case 47:
        hit = func_800566d4(180, 0x80000000, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->unk0 = OBJECT4C_FREE;
            } else {
                func_800537e8(hit);
            }
        }
        func_80056268(&obj->pos, 0x2000, 0x2000, 500);
        if (++obj->arg.s.unk44 >= 6) {
            obj->arg.s.unk44 = 0;
        }
        obj->unke = obj->arg.s.unk44 << 6;
        break;
    case 90:
        obj->scale.vz += obj->arg.s.unk46;
        obj->scale.vx += obj->arg.s.unk46;
        if (obj->scale.vx >= 768) {
            VECTOR p;

            p.vx = obj->pos.vx;
            p.vy = obj->pos.vy + 1000;
            p.vz = obj->pos.vz;
            func_80053850(&p, (obj->scale.vx - obj->arg.s.unk46) * 4, obj->scale.vx * 4 - 1, 2000, 0x1000,
                          obj->arg.s.unk48);
        }
        if (obj->scale.vx > obj->arg.s.unk44) {
            obj->unk0 = OBJECT4C_FREE;
        }
        obj->angle.vy += 512;
        break;
    case 94:
        obj->scale.vz += 192;
        obj->scale.vx += 192;
        if (obj->scale.vx >= 768) {
            VECTOR p;

            p.vx = obj->pos.vx;
            p.vy = obj->pos.vy + 1000;
            p.vz = obj->pos.vz;
            p.pad = obj->angle.vy;
            func_80053850(&p, obj->scale.vx * 15 >> 4, obj->scale.vx * 9 >> 3, 0x8002, 0x400, 0x1000);
        }
        if (obj->scale.vx > 3200) {
            obj->unk0 = OBJECT4C_FREE;
        }
        break;
    case 88:
        func_80056134(0x4000, 0x600, 0x80, 0x400, 0x8000);
        obj->angle.vy += 64;
        break;
    case 18:
    case 20:
    case 71:
        switch (phase) {
        case 101:
            obj->unk7++;
            break;
        case 100:
            goto spawn18;
        case 102:
            goto become18;
        case 110:
        shrink18:
            if (id == 18) {
                obj->unke = 0;
            }
            func_80056134(0x2000, 799, 70, 0xc00, 0x8000);
            obj->angle.vy += 64;
        break;
        default:
            if (obj->unk7 == 0) {
                audio_play_spatial_range(0x2c, &obj->pos, 0x6e, 0x6d60, 0x7148, 0);
            }
            obj->unk7++;
            if (func_80055db0(obj->arg.s.unk44, obj->arg.s.unk46, obj->arg.s.unk48, 160, 0, 6000,
                              obj->arg.s.unk4a, 0x800) == -1) {
                Object4c *made;

            spawn18:
                made = func_80053c84(10, obj->unk0 | 3, id, &obj->pos, NULL, &obj->angle);
                made->unk7 = 101;
            become18:
                obj->unk3 = 0x15;
                obj->unk7 = 110;
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->unke = 0x40;
                obj->unk0 |= 3;
                func_800532d4(obj, 0x28);
                goto shrink18;
            } else {
                SVECTOR v;

                func_80056268(&obj->pos, 0x2000, 0x2000, 500);
                obj->angle.vz += 128;
                v.vx = v.vy = v.vz = 0;
                if (id == 18) {
                    func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 3, 0x25, 0, 1, 0);
                } else {
                    func_80053c84(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 3, 0x25, 0, 1, 1);
                }
            }
            break;
        }
        break;
    case 2:
        {
            SVECTOR v;
            VECTOR p;
            Object88 *o;

            if (obj->arg.b.unk44 == 0xfe) {
                p.vx = DAT_801b24e4.pos.vx;
                p.vy = DAT_801b24e4.pos.vy - 1600;
                p.vz = DAT_801b24e4.pos.vz;
                hit = func_80055f74(&p, 500, 60, 3000, 0, 10, 0x80000000);
            } else if (obj->arg.b.unk44 == 0xff) {
                goto fall2;
            } else {
                o = &DAT_80185da8.objects[obj->arg.b.unk44];
                p.vx = o->pos.vx;
                p.vy = o->pos.vy - (o->unk1e >> 1);
                p.vz = o->pos.vz;
                hit = func_80055f74(&p, 450, 60, 100, 0, 10, 0x80000000);
            }
            if (hit != -1) {
                goto trail2;
            }
            {
            boom2:
                func_800537e8(DAT_801d4464.unk12024);
                for (i = 5; i != -1; i--) {
                    v.vx = (rand() * 400 >> 15) - 200;
                    v.vz = (rand() * 400 >> 15) - 200;
                    v.vy = (rand() * 400 >> 15) - 200;
                    func_80053c84(10, 0, 0x6f, &obj->pos, &v, 7500, -750, 8, 22, 10, 1, 2);
                }
                func_80056580(&obj->pos, 5, 0x400, 0x2000, 0x4000, 0x1000);
                obj->unk0 = OBJECT4C_FREE;
                break;
            }
        fall2:
            obj->vel.vy += 10;
            if (func_800566d4(10, 0x80000000, 0) != 0) {
                goto boom2;
            }
        trail2:
            if (obj->unk10 < 74) {
                v.vx = (rand() * 60 >> 15) - 30;
                v.vy = (rand() * 60 >> 15) - 30;
                v.vz = (rand() * 60 >> 15) - 30;
                func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1000, -256, 13, 0x1c, 10, 1, 2);
            }
        }
        break;
    case 1:
        {
            SVECTOR v;
            VECTOR p;

            hit = func_800566d4(180, 0x80000000, 0);
            if (hit != 0 && (hit & 0x10)) {
                func_800537e8(hit);
            }
            p.vx = v.vx + obj->pos.vx;
            p.vy = v.vy + obj->pos.vy;
            p.vz = v.vz + obj->pos.vz;
            func_80053850(&p, 0, obj->scale.vx * 1024 >> 12, obj->scale.vy, 0x400, 0x1000);
            switch (obj->arg.s.unk44) {
            case 0:
                obj->scale.vx += 560;
                obj->scale.vz += 560;
                obj->scale.vy += 500;
                obj->angle.vy += 500;
                if (obj->scale.vy > 4200) {
                    obj->arg.s.unk46 = 0;
                    obj->arg.s.unk44++;
                }
                break;
            case 1:
                obj->arg.s.unk46++;
                obj->angle.vy += 500;
                if (obj->arg.s.unk46 >= 3) {
                    obj->arg.s.unk44++;
                }
                break;
            case 2:
                obj->scale.vy -= 500;
                obj->scale.vx -= 560;
                obj->scale.vz -= 560;
                obj->angle.vy += 500;
                if (obj->scale.vy < 100) {
                    obj->unk0 = OBJECT4C_FREE;
                }
                break;
            }
            v.vx = 0;
            v.vy = -(rand() * 100 >> 15) - 100;
            v.vz = 0;
            func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1000, 0, 8, 0x1c, 2, 1, 2);
            break;
    case 108:
        hit = func_800566d4(180, 0, 0);
        if (hit != 0 && (hit & 0x10)) {
            func_800537e8(hit);
        }
        switch (obj->arg.s.unk44) {
        case 0:
            obj->scale.vy += 256;
            obj->angle.vy += 200;
            if (obj->scale.vy > 4200) {
                obj->arg.s.unk46 = 0;
                obj->arg.s.unk44++;
            }
            break;
        case 1:
            obj->arg.s.unk46++;
            obj->angle.vy += 200;
            if (obj->arg.s.unk46 >= 31) {
                obj->arg.s.unk44++;
            }
            break;
        case 2:
            obj->scale.vy -= 256;
            obj->scale.vx -= 128;
            obj->scale.vz -= 128;
            obj->angle.vy += 200;
            if (obj->scale.vy < 120) {
                obj->unk0 = OBJECT4C_FREE;
            }
            break;
        }
        break;
    case 4:
        switch (phase) {
        case 0:
            if (func_800566d4(250, 0x80000000, 0) != 0 || obj->unk10 < 2) {
                Object4c *made;

                func_80056580(&obj->pos, 8, 400, 0x2000, 0x8000, 0x400);
                made = func_80053c84(10, obj->unk0 | 3, 4, &obj->pos, NULL, &obj->angle);
                made->unk7 = 2;
                func_800532d4(obj, 0x27);
            case 5:
                obj->unk7 = 1;
                obj->unk3 = 0x15;
                obj->unkc = 0x34;
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->unk10 = -1;
                obj->unk0 |= 3;
                goto shrink4;
            } else {
                SVECTOR dir;
                VECTOR t;
                VECTOR a;
                Object88 *o;
                s32 dist;

                obj->unk14 = (obj->unk14 + 128) & 0xfff;
                if (obj->arg.b.unk45 != 0) {
                    func_80053af4(obj, 0, &a, &obj->scale);
                    o = &DAT_80185da8.objects[obj->arg.b.unk44];
                    t.vx = o->pos.vx;
                    t.vy = o->pos.vy - (o->unk1e >> 1);
                    t.vz = o->pos.vz;
                    vector_direction_scaled(&a, &t, 800, &dir);
                    func_80053c84(10, obj->unk0, 0, &a, &dir);
                    obj->arg.b.unk45--;
                } else if (rand() < 2048) {
                    o = func_8004d3c4(&obj->pos, obj->angle.vy, obj->angle.vx, 25000, 800, 800, &dist, 0x200);
                    if (o != NULL) {
                        obj->arg.b.unk44 = o - DAT_80185da8.objects;
                        obj->arg.b.unk45 = 5;
                    }
                }
                if (rand() < 1024 &&
                    func_8004d3c4(&obj->pos, obj->angle.vy, obj->angle.vx, 30000, 800, 800, &dist, 0x200) != NULL) {
                    func_800561d8(obj, 0x2f);
                    func_800561d8(obj, 0x32);
                }
            }
            break;
        case 1:
        shrink4:
            func_80056134(0x4000, 0x800, 0x4b, 0x400, 0x8000);
            obj->angle.vy += 64;
            break;
        default:
            obj->unk7++;
            break;
        }
        break;
    case 16:
        {
            Object88 *o;
            Object4cTrail *trail;
            s32 a;
            VECTOR tmp;

            switch (phase) {
            case 0:
                {
                    Object4c *made;

                    for (i = 0; i < 8; i++) {
                        made = func_80053c84(10, 0, 0x74, &obj->pos, &obj->vel, &obj->angle, i);
                        if (i == 0) {
                            made->unk3 = 0x10;
                        }
                        made->arg.b.unk44 = DAT_801b77ec.unk2f08;
                        made->arg.b.unk45 = i;
                    }
                    made->unk3 = 0x12;
                    obj->unk7 = 1;
                }
            case 1:
                if (obj->unk10 < 3) {
                fail16:
                    obj->unk7 = 3;
                    obj->unk10 = -1;
                    obj->arg.b.unk49 = 24;
                    break;
                }
                if (func_80055db0(250, 25, 32, 200, 0, 0x400, 100, 0x800) == -1) {
                    obj->unk10 = -1;
                    if (DAT_801d4464.unk12024 != 0x10) {
                        goto fail16;
                    }
                    func_800536f8(0x10010, 5000, NULL);
                    obj->arg.b.unk4a = DAT_801d4464.unk12028;
                    o = &DAT_80185da8.objects[obj->arg.b.unk4a];
                    if (o->unke != 2 && o->unke != 3) {
                        goto fail16;
                    }
                    obj->unk8 = 1;
                    obj->unk3 = 0x13;
                    obj->unk7 = 2;
                    obj->arg.b.unk49 = 0;
                    obj->angle.vz = 0;
                    obj->angle.vx = 0;
                    obj->angle.pad = -vector_xz_to_angle(obj->pos.vx - o->pos.vx, obj->pos.vz - o->pos.vz);
                    goto case16_2;
                } else {
                    if (++obj->arg.b.unk48 >= 24) {
                        obj->arg.b.unk48 = 0;
                    }
                    trail = &obj->arg.w.unk44[obj->arg.b.unk48];
                    a = obj->arg.b.unk49 << 4;
                    obj->arg.b.unk49 += 8;
                    trail->pos.vx = obj->pos.vx;
                    trail->pos.vz = obj->pos.vz;
                    trail->pos.vy = (rsin(a) >> 3) + obj->pos.vy;
                    trail->angle.vy = obj->angle.vy;
                    trail->angle.vz = obj->angle.vz;
                    trail->angle.vx = obj->angle.vx + (rcos(a) >> 4);
                }
                break;
            case 2:
            case16_2:
                {
                    s32 n;
                    s32 d;
                    u32 k;

                    o = &DAT_80185da8.objects[obj->arg.b.unk4a];
                    o->unk28 |= 0x800;
                    n = o->unk1c;
                    if (++obj->arg.b.unk48 >= 24) {
                        obj->arg.b.unk48 = 0;
                    }
                    obj->pos = *func_8004eee0(o, &tmp);
                    obj->pos.vy -= o->unk1e >> 1;
                    k = obj->arg.b.unk49;
                    if (k >= 17) {
                        if (k >= 60) {
                            obj->unk7 = 4;
                            break;
                        }
                    } else {
                        obj->scale.vx = obj->scale.vz = fixed_lerp_q12(0, n, k << 9);
                        obj->scale.vy = fixed_lerp_q12(0, o->unk1e, obj->arg.b.unk49 * 350);
                    }
                    d = 100000 / n;
                    n = n * 6400 >> 12;
                    a = obj->angle.pad;
                    obj->arg.b.unk49++;
                    trail = &obj->arg.w.unk44[obj->arg.b.unk48];
                    obj->angle.pad = a + d;
                    trail->pos.vx = (rsin(a) * n >> 12) + obj->pos.vx;
                    trail->pos.vz = (rcos(a) * n >> 12) + obj->pos.vz;
                    trail->pos.vy = (rsin(a * 2) >> 4) + obj->pos.vy;
                    trail->angle.vy = -a - 1024;
                    trail->angle.vz = 0;
                    trail->angle.vx = rcos(a * 2) >> 4;
                }
                break;
            case 3:
                if (++obj->arg.b.unk48 >= 24) {
                    obj->arg.b.unk48 = 0;
                }
                if (obj->arg.b.unk49 == 0) {
                    goto release16;
                }
                obj->arg.b.unk49--;
                break;
            case 4:
                {
                    SVECTOR v;

                    for (i = 23; i != -1; i--) {
                        if (!(i & 7)) {
                            func_800532d4(obj, 0x27);
                        }
                        v.vx = rand() * 600 / 32767 - 300;
                        v.vy = rand() * -300 / 32767;
                        v.vz = rand() * 600 / 32767 - 300;
                        func_80053c84(10, 0, 0x6f, &obj->pos, &v, 1500, -128, 9, 0x30, 0, 1, 0);
                    }
                release16:
                    o = &DAT_80185da8.objects[obj->arg.b.unk4a];
                    o->unk28 &= ~0x800;
                    obj->unk0 = OBJECT4C_FREE;
                }
                break;
            }
            break;
    case 5:
        {
            Object88 *o;
            Object4cTrail *trail;
            s32 a;
            SVECTOR v;
            VECTOR tmp;

            switch (phase) {
            case 0:
                {
                    Object4c *made;

                    for (i = 0; i < 8; i++) {
                        made = func_80053c84(10, 0, 0x67, &obj->pos, &obj->vel, &obj->angle, i);
                        if (i == 0) {
                            made->unk3 = 0x10;
                        }
                        made->arg.b.unk44 = DAT_801b77ec.unk2f08;
                        made->arg.b.unk45 = i;
                    }
                    made->unk3 = 0x12;
                    obj->unk7 = 1;
                }
            case 1:
                if (obj->unk10 < 3) {
                fail5:
                    obj->unk7 = 3;
                    obj->unk10 = -1;
                    obj->arg.b.unk49 = 24;
                    break;
                }
                if (func_80055db0(250, 25, 32, 200, 0, 0x400, 100, 0x800) == -1) {
                    obj->unk10 = -1;
                    if (DAT_801d4464.unk12024 != 0x10) {
                        goto fail5;
                    }
                    func_800536f8(0x10010, 5000, NULL);
                    obj->arg.b.unk4a = DAT_801d4464.unk12028;
                    o = &DAT_80185da8.objects[obj->arg.b.unk4a];
                    if (o->unke != 2 && o->unke != 3) {
                        goto fail5;
                    }
                    obj->unk8 = 1;
                    obj->unk3 = 0x13;
                    obj->unk7 = 2;
                    obj->arg.b.unk49 = 0;
                    obj->angle.vz = 0;
                    obj->angle.vx = 0;
                    obj->angle.pad = -vector_xz_to_angle(obj->pos.vx - o->pos.vx, obj->pos.vz - o->pos.vz);
                    for (i = 0; i < 8; i++) {
                        func_80053c84(10, 0, 0x6c, &o->pos, NULL, i << 9);
                    }
                    goto case5_2;
                } else {
                    if (++obj->arg.b.unk48 >= 24) {
                        obj->arg.b.unk48 = 0;
                    }
                    trail = &obj->arg.w.unk44[obj->arg.b.unk48];
                    a = obj->arg.b.unk49 << 4;
                    obj->arg.b.unk49 += 8;
                    trail->pos.vx = obj->pos.vx;
                    trail->pos.vz = obj->pos.vz;
                    trail->pos.vy = (rsin(a) >> 3) + obj->pos.vy;
                    trail->angle.vy = obj->angle.vy;
                    trail->angle.vz = obj->angle.vz;
                    trail->angle.vx = obj->angle.vx + (rcos(a) >> 4);
                }
                break;
            case 2:
            case5_2:
                {
                    s32 n;
                    s32 d;
                    u32 k;

                    o = &DAT_80185da8.objects[obj->arg.b.unk4a];
                    o->unk28 |= 0x800;
                    v.vx = rand() * 100 / 32767 - 50;
                    v.vy = -(rand() * 160 / 32767) - 200;
                    v.vz = rand() * 100 / 32767 - 50;
                    func_80053c84(10, 0, 0x6f, &o->pos, &v, 0x1000, 0x200, 6, 0x1c, 2, 1, 2);
                    n = o->unk1c;
                    if (++obj->arg.b.unk48 >= 24) {
                        obj->arg.b.unk48 = 0;
                    }
                    obj->pos = *func_8004eee0(o, &tmp);
                    obj->pos.vy -= o->unk1e >> 1;
                    k = obj->arg.b.unk49;
                    if (k >= 17) {
                        if (k >= 60) {
                            obj->unk7 = 4;
                            break;
                        }
                    } else {
                        obj->scale.vx = obj->scale.vz = fixed_lerp_q12(0, n, k << 9);
                        obj->scale.vy = fixed_lerp_q12(0, o->unk1e, obj->arg.b.unk49 * 350);
                    }
                    d = 100000 / n;
                    n = n * 6400 >> 12;
                    a = obj->angle.pad;
                    obj->arg.b.unk49++;
                    trail = &obj->arg.w.unk44[obj->arg.b.unk48];
                    obj->angle.pad = a + d;
                    trail->pos.vx = (rsin(a) * n >> 12) + obj->pos.vx;
                    trail->pos.vz = (rcos(a) * n >> 12) + obj->pos.vz;
                    trail->pos.vy = (rsin(a * 2) >> 4) + obj->pos.vy;
                    trail->angle.vy = -a - 1024;
                    trail->angle.vz = 0;
                    trail->angle.vx = rcos(a * 2) >> 4;
                }
                break;
            case 3:
                if (++obj->arg.b.unk48 >= 24) {
                    obj->arg.b.unk48 = 0;
                }
                if (obj->arg.b.unk49 == 0) {
                    goto release5;
                }
                obj->arg.b.unk49--;
                break;
            case 4:
                {
                    for (i = 1; i != -1; i--) {
                        v.vx = rand() * 200 / 32767 - 100;
                        v.vy = -(rand() * 200 / 32767) - 200;
                        v.vz = rand() * 200 / 32767 - 100;
                        func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1000, 0, 5, 0x1c, 10, 1, 1);
                    }
                release5:
                    o = &DAT_80185da8.objects[obj->arg.b.unk4a];
                    o->unk28 &= ~0x800;
                    obj->unk0 = OBJECT4C_FREE;
                }
                break;
            }
            break;
    case 116:
        {
            Object4c *link;
            Object4cTrail *t;
            s32 k;

            link = &DAT_801b77ec.objects[obj->arg.b.unk44];
            if (phase == 0 && link->unk7 >= 3) {
                obj->unk7 = 1;
                obj->unk10 = obj->arg.b.unk45 * 3;
            }
            if (link->unk7 == 4) {
                obj->unk0 = OBJECT4C_FREE;
                break;
            }
            k = link->arg.b.unk48 - obj->arg.b.unk45 * 3;
            if (k < 0) {
                k += 24;
            }
            t = &link->arg.w.unk44[k];
            obj->pos = t->pos;
            obj->angle = t->angle;
            break;
    case 103:
        {
            Object4c *link;
            Object4cTrail *t;
            SVECTOR v;
            s32 k;

            link = &DAT_801b77ec.objects[obj->arg.b.unk44];
            if (obj->unk10 < 188) {
                v.vx = (rand() * 100 >> 15) - 50;
                v.vz = (rand() * 100 >> 15) - 50;
                v.vy = -120;
                func_80053c84(10, 0, 0x6f, &obj->pos, &v, 0x1000, -256, 4, 0x16, 0, 2, 5);
            }
            if (phase == 0 && link->unk7 >= 3) {
                obj->unk7 = 1;
                obj->unk10 = obj->arg.b.unk45 * 3;
            }
            if (link->unk7 == 4) {
                obj->unk0 = OBJECT4C_FREE;
                break;
            }
            k = link->arg.b.unk48 - obj->arg.b.unk45 * 3;
            if (k < 0) {
                k += 24;
            }
            t = &link->arg.w.unk44[k];
            obj->pos = t->pos;
            obj->angle = t->angle;
            break;
    case 111:
        obj->vel.vy += obj->arg.s.unk46;
        obj->pos.vx += obj->vel.vx;
        obj->pos.vz += obj->vel.vz;
        obj->pos.vy += obj->vel.vy;
        obj->scale.vx += obj->arg.s.unk44;
        obj->scale.vy = obj->scale.vz = obj->scale.vx;
        break;
    case 91:
        {
            VECTOR p;

            if (obj->unk10 >= 10) {
                obj->pos.vx += obj->vel.vx;
                obj->pos.vz += obj->vel.vz;
                obj->unk4 = (obj->unk10 & 1) - 128;
            } else {
                obj->pos.vx -= obj->vel.vx;
                obj->pos.vz -= obj->vel.vz;
                obj->scale.vy += 512;
                obj->unk4 = (obj->unk10 & 1) - 128;
            }
            p.vx = obj->vel.vx + obj->pos.vx;
            p.vy = obj->vel.vy + obj->pos.vy;
            p.vz = obj->vel.vz + obj->pos.vz;
            func_80053850(&p, 0, obj->scale.vx, 0x8000, 0x400, 0x1000);
            break;
    case 112:
        obj->unk7++;
        obj->scale.vy = rsin(obj->unk7 << 8) * obj->arg.s.unk44 >> 12;
        if (obj->unk7 >= 8) {
            obj->unk0 = OBJECT4C_FREE;
        }
        break;
    case 26:
        if (DAT_801b24e4.unk86 == 0) {
            obj->unk0 = OBJECT4C_FREE;
        }
        obj->angle.vy += 64;
        break;
    case 27:
        if (DAT_801b24e4.unk88 == 0) {
            obj->unk0 = OBJECT4C_FREE;
        }
        obj->angle.vy -= 256;
        break;
    case 28:
        if (DAT_801b24e4.unk8a == 0) {
            obj->unk0 = OBJECT4C_FREE;
        }
        obj->angle.vy -= 64;
        break;
    case 23:
        if (obj->unk10 == 4) {
            DAT_801b24e4.hp += 80;
            if (DAT_801b24e4.hp > DAT_801b24e4.hp_max) {
                DAT_801b24e4.hp = DAT_801b24e4.hp_max;
            }
        }
        func_8003456c(230, 200, 160, 0, 23000, rsin(obj->unk10 << 8));
        break;
    case 25:
        {
            VECTOR p;
            SVECTOR v;
            Object4c *made;

            if (obj->unk10 == 12) {
                DAT_801b24e4.unk78 = 0;
            }
            p.vx = (rand() >> 5) - 512;
            p.vy = (rand() >> 8) + 200;
            p.vz = 0x400;
            v.vz = 0;
            v.vx = 0;
            v.vy = 0;
            made = func_80053c84(10, 0, 0x6f, &p, &v, 700, -30, 10, 0x26, -10, 1, 2);
            made->unka = 1;
            made->unk8 = 14;
            func_8003456c(160, 180, 220, 0, 18000, rsin(obj->unk10 << 7));
            break;
    case 30:
        {
            VECTOR p;
            SVECTOR v;
            Object4c *made;

            if (obj->unk10 == 12) {
                DAT_801b24e4.hp += 200;
                if (DAT_801b24e4.hp > DAT_801b24e4.hp_max) {
                    DAT_801b24e4.hp = DAT_801b24e4.hp_max;
                }
                func_8002aa54(7);
            }
            v.vz = 0;
            v.vx = 0;
            v.vy = 0;
            for (i = 3; i != 0; i--) {
                p.vx = (rand() >> 5) - 512;
                p.vy = (rand() >> 8) + 200;
                p.vz = 0x400;
                made = func_80053c84(10, 0, 0x6f, &p, &v, 700, -30, 10, 0x26, -10, 1, 1);
                made->unka = 1;
                made->unk8 = 14;
            }
            func_8003456c(240, 240, 160, 0, 18000, rsin(obj->unk10 << 7));
            break;
    case 8:
        func_800532d4(obj, 0x23);
        func_80056134(0x4000, 480, 80, 0xc00, 0x8000);
        break;
    case 95:
        {
            SVECTOR v;
            VECTOR p;
            SVECTOR spare4[6];

            obj->pos.vx += obj->vel.vx;
            obj->pos.vy += obj->vel.vy;
            obj->pos.vz += obj->vel.vz;
            obj->angle.vy += 256;
            hit = func_80033f38(obj->pos.vx, obj->pos.vy, obj->pos.vz, 10, 10, 0xb0);
            if (hit == 0) {
                func_80033b10(obj->pos.vx, obj->arg.w.unk48, obj->pos.vz, 0, 0);
                if (obj->pos.vy < DAT_801d4464.unk12010) {
                    goto fly95;
                }
                obj->pos.vy = DAT_801d4464.unk12010;
            }
            p = obj->pos;
            v.vx = 0;
            v.vy = -32;
            v.vz = 0;
            func_800537e8(hit);
            func_80053c84(10, obj->unk0, 0x5a, &p, &v, 1600, 100, 0x800);
            for (i = 3; i != -1; i--) {
                v.vx = 0;
                v.vy = -360;
                v.vz = 0;
                p = obj->pos;
                p.vx += (rand() * 1000 >> 15) - 500;
                p.vz += (rand() * 1000 >> 15) - 500;
                func_80053c84(10, 0, 0x6f, &p, &v, 0x4000, -128, 5, 0x1d, 0, 1, 2);
            }
            obj->unk0 = OBJECT4C_FREE;
            break;
        fly95:
            for (i = 5; i != -1; i--) {
                v = obj->vel;
                v.vx = (v.vx >> 3) * (i + 1);
                v.vy = (v.vy >> 3) * (i + 1);
                v.vz = (v.vz >> 3) * (i + 1);
                func_80053c84(10, 0, 0x6f, &obj->pos, &v, i * 350 + 1200, -128, 5, 0x1c, 0, 1, 2);
            }
            break;
    default:
        break;
        /* end of the case scopes left open above */
        }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
    }
}
