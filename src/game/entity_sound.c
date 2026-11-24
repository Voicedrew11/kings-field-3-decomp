/*
 * GAME.EXE: Object88 attacks and sound helpers (WIP module): the attack
 * spawner of the current record, the note offset from the record's scale
 * and the sound effect of its running script record.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/data.h>
#include <game/sound.h>
#include <game/entity.h>
#include <game/object4c.h>

RODATA(0x800126e8, 0x208)

/* Aims a shot of speed from pos at target. */
#define AIM(speed) func_8004f1d0(obj, target, speed, &pos, &dir, -1, 0x400, 1)

/*
 * Fires attack id of the current record from its model vertex (-1: the
 * offset in the next three arguments rotated by its angle; -2: between two
 * vertices, aimed at a point eased toward the player): Object4c records
 * through func_80053c84, aimed at the player. The arguments past vertex
 * are read through its address: retail homes a2/a3 like a `...` function
 * and stores vertex to its slot.
 */
ADDRESS(0x8004f414, 0xb8c)
void func_8004f414(s32 id, s32 kind, s32 vertex, ...)
{
    Object88 *obj;
    Object88 *o;
    Object4c *shot;
    Object78 *def;
    VECTOR *target;
    s32 *args;
    VECTOR pos;
    VECTOR off;
    VECTOR from;
    VECTOR aim;
    SVECTOR dir;
    SVECTOR sv;
    SVECTOR rot;
    VECTOR p;
    s32 time;
    s32 pitch;
    s32 speed;
    s32 i;
    s32 delay;
    u16 n;

    args = &vertex;
    target = &DAT_801b24e4.pos;
    obj = DAT_80185da8.current;
    if (vertex == -1) {
        rot.vx = args[1];
        rot.vy = args[2];
        rot.vz = args[3];
        args += 3;
        func_800168a8(&obj->angle, &rot, &off);
    } else if (vertex == -2) {
        func_8004ede0(obj, args[1], &from);
        func_8004ede0(obj, args[2], &off);
        args += 3;
        speed = args[0];
        aim.vx = func_80017158(DAT_801b24e4.pos.vx, ((off.vx - from.vx) << 8) + obj->pos.vx, speed);
        aim.vy = func_80017158(DAT_801b24e4.pos.vy, ((off.vy - from.vy) << 8) + obj->pos.vy, speed);
        aim.vz = func_80017158(DAT_801b24e4.pos.vz, ((off.vz - from.vz) << 8) + obj->pos.vz, speed);
        target = &aim;
    } else {
        func_8004ede0(obj, vertex, &off);
    }
    pos.vx = obj->pos.vx + off.vx;
    pos.vy = obj->pos.vy + off.vy;
    pos.vz = obj->pos.vz + off.vz;
    switch (id) {
    case 0:
    case 0x56:
    case 0x66:
        func_80015654(0x31, &pos, 0x6e, 0);
        AIM(500);
        shot = func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        if (shot != NULL) {
            shot->unkb = 3;
        }
        break;
    case 0xf:
        func_8004f1d0(obj, target, 0x200, &pos, &dir, 0, 0x400, 1);
        func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir, 0, 0);
        func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir, 0, 1);
        break;
    case 0x27:
        if (DAT_801b24e4.unk84 != 0) {
            obj->unk28 |= 0x100;
        }
    case 0x26:
        func_8004f1d0(obj, target, 0, &pos, &dir, -1, 0x400, 1);
        shot = func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir);
        if (shot != NULL) {
            shot->unkb = 0;
        }
        break;
    case 0x13:
        id = 0x71;
        goto lob;
    case 0x29:
    case 0x7d:
        id = 0x7d;
        goto lob;
    case 0x2d:
    case 0x81:
        id = 0x81;
    lob:
        AIM(600);
        delay = (func_80016c44(pos.vx - target->vx, pos.vy - target->vy, pos.vz - target->vz) - 2000) / 600;
        if (delay < 0) {
            delay = 0;
        }
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, delay);
        break;
    case 0xd:
        AIM(0x2a8);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0xb:
        for (i = 0; i < 4; i++) {
            AIM(200);
            func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, i);
        }
        break;
    case 0x32:
        for (i = 0; i < 4; i++) {
            AIM(200);
            func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        }
        break;
    case 0x2a:
        AIM(600);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &obj->angle);
        break;
    case 0x2c:
        AIM(0x1a4);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x35:
        AIM(300);
        func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir);
        break;
    case 0x31:
        AIM(600);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x44:
        AIM(0x226);
        func_80053c84((u8)kind, 0x83, 0x7c, &pos, &dir);
        break;
    case 0x45:
        AIM(700);
        func_80053c84((u8)kind, 0x83, 0x7b, &pos, &dir);
        break;
    case 0x42:
        func_80016c90(target->vx - pos.vx, -(pos.vy + 1600) + target->vy, target->vz - pos.vz, &sv);
        func_800167cc(&sv, &dir);
        func_80016928(0x400, &dir);
        pos.vx += dir.vx;
        pos.vy += dir.vy;
        pos.vz += dir.vz;
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &sv);
        break;
    case 2:
    case 0x57:
        AIM(400);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, 0xfe, 2);
        break;
    case 0x2e:
        id = 100;
    case 0x64:
        AIM(0x3c0);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x2f:
        AIM(0x1a4);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x30:
        AIM(0x1e0);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 7:
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, 0x1000, 0x100, 0x1000, 1);
        break;
    case 0x55:
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, NULL, DAT_80185da8.unk9d18, vertex,
                      SCRIPT_U16((u8 *)args[1], 4));
        break;
    case 0x28:
        func_800167cc(&obj->angle, &dir);
        func_80016928(0x226, &dir);
        shot = func_80053c84((u8)kind, 0x83, 0x28, &pos, &dir);
        if (shot != NULL) {
            shot->unkb = 3;
        }
        break;
    case 6:
    case 0x33:
    case 0x34:
        AIM(300);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x1f:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
        AIM(300);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &obj->angle);
        break;
    case 0x12:
    case 0x14:
        func_80016c90(target->vx - pos.vx, -(pos.vy + 1600) + target->vy, target->vz - pos.vz, &sv);
        func_800167cc(&sv, &dir);
        if (id == 0x12) {
            func_80016928(200, &dir);
        } else {
            func_80016928(20, &dir);
        }
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &obj->angle, 600, 60, 0x80, 0x8c, 0xa0);
        break;
    case 0x76:
        n = SCRIPT_U16((u8 *)args[1], 4);
        o = func_8004b524();
        if (o == NULL) {
            break;
        }
        AIM(400);
        o->unk0 = 5;
        o->unk2 = n;
        o->unk4 = 0;
        o->unk5 = 0;
        o->unk3 = obj->unk6;
        o->unk9 = 1;
        def = &DAT_80185da8.defs[n];
        o->unk28 = def->unk34;
        o->unk15 = def->unk9;
        o->pos.vx = pos.vx;
        o->pos.vy = pos.vy + 4096;
        o->pos.vz = pos.vz;
        func_8004b698(o);
        o->vel = dir;
        o->angle = obj->angle;
        func_8004c0b0(o, 0x1d);
        break;
    case 0x50:
    case 0x53:
        p = *target;
        i = 6;
        for (;;) {
            if (func_80017210(0, func_80016c08(p.vx - pos.vx, p.vz - pos.vz), pos.vy + (1400 - p.vy), 10, 800,
                              &time, &pitch)) {
                pitch = 0x100;
            }
            if (--i == 0) {
                break;
            }
            func_800175a8(target, &DAT_801b24e4.unk11c, time >> 6, &p);
        }
        sv.vy = func_8004f1d0(obj, &p, 800, &pos, &dir, (s16)pitch, 0xc00, 1);
        sv.vx = pitch;
        sv.vz = 0;
        shot = func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &sv);
        if (shot != NULL) {
            shot->unk44 = pos.vy;
            shot->unk10 = 0x32;
            shot->unk7 = 0;
        }
        break;
    }
}

/* Note offset for obj's sound: lower for a larger record, plus a little noise. */
ADDRESS(0x8004ffa0, 0x70)
s32 func_8004ffa0(Object88 *obj)
{
    s32 note;

    note = 16 - (s8)(obj->scale.vy >> 8);
    if (note > 12) {
        note = 12;
    } else if (note < -12) {
        note = -12;
    }
    return note - 2 + ((rand() * 5) >> 15);
}

/* Plays the sound effect of obj's running script record at obj. */
ADDRESS(0x80050010, 0x98)
void func_80050010(Object88 *obj)
{
    u8 *script;

    script = obj->unk60;
    if (obj->unk28 & 0x800000) {
        func_80015688(script[4] + 240, &obj->pos, 0x7f, 0x6000, 0x7800, func_8004ffa0(obj));
    } else {
        func_80015654(script[4] + 240, &obj->pos, 0x6e, func_8004ffa0(obj));
    }
}
