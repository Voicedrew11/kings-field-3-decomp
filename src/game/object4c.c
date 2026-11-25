/*
 * GAME.EXE: Object4c records (WIP module): their sound, collision and hit
 * tests, record allocation and func_80053c84, which makes a record of an id
 * (with its set-up helpers).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/cd.h>
#include <game/collision.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/object4c.h>

RODATA(0x80012cb4, 0x208)

/* Index of the DAT_801e7594 trail set the next id 5 or 16 record takes. */
DATA(0x8009c338, 0x4, ".sdata")
static s32 DAT_8009c338 = 0;

/* Frame before which id 112 does not refresh it (DAT_801c12e4.frames). */
DATA(0x8009c3f0, 0x4, ".sbss")
static s32 DAT_8009c3f0;

/* Plays sound effect se at the record. */
ADDRESS(0x800532d4, 0x3c)
s32 func_800532d4(Object4c *obj, s32 se)
{
    return audio_play_spatial_range(se, &obj->pos, 0x6e, 0x6d60, 0x7148, 0);
}

/* Collision test of the current record at pos, once its delay has run out. */
ADDRESS(0x80053310, 0xd8)
s32 func_80053310(VECTOR *pos, s32 radius, s32 height)
{
    Object4c *obj;
    s32 y;
    s32 mask;

    obj = DAT_801b77ec.unk2f04;
    if (obj->unkb == 0) {
        y = pos->vy + ((height & 0xfff) >> 1);
        switch (obj->unk0 & 7) {
        case 2:
            mask = 0x31;
            break;
        case 1:
            mask = 0xa1;
            break;
        case 3:
            mask = 0xb1;
            break;
        case 4:
            mask = 1;
            break;
        default:
            return KF_VALUELESS;
        }
        return collision_query_world(pos->vx, y, pos->vz, radius, height, mask);
    }
    obj->unkb--;
    return 0;
}

/*
 * Hit by the current record: the player (kind 0x80) or the Object88 record
 * of unk12028 (0x10). func_800536f8 passes a3 unmasked, so the narrow
 * parameters arrive as int and are narrowed at entry. Not exact: retail spills a3 with sw
 * and reads it back with lhu; this source spills it with sh.
 */
ADDRESS(0x800533e8, 0x23c)
void func_800533e8(s32 flags, s32 kind, Record18 *rec, int a3_in, int a4_in, int a5_in, int a6_in,
                   int a7_in, int a8_in, int a9_in, int a10_in, int a11_in, int a12_in, int a13_in,
                   int a14_in, VECTOR *target)
{
    u16 a3 = a3_in;
    u16 a4 = a4_in;
    u8 a5 = a5_in;
    u16 a6 = a6_in;
    u16 a7 = a7_in;
    u16 a8 = a8_in;
    u16 a9 = a9_in;
    u16 a10 = a10_in;
    u16 a11 = a11_in;
    u16 a12 = a12_in;
    u16 a13 = a13_in;
    u16 a14 = a14_in;
    s32 mode;
    Object88 *obj;
    Object78 *def;

    mode = flags & 0xf0000;
    flags &= 0xfff0ffff;
    if (flags == 0x80) {
        player_apply_damage(a6, a7, a8, a9, a10, a11, a12, a13, a14, a3, a5, target);
    } else if (flags == 0x10) {
        obj = &DAT_80185da8.objects[DAT_801d4464.unk12028];
        def = &DAT_80185da8.defs[obj->unk2];
        if (target != NULL &&
            !angle_within_tolerance(obj->angle.vy,
                                    vector_xz_to_angle(obj->pos.vx - target->vx,
                                                       obj->pos.vz - target->vz) + 2048,
                                    def->unk18)) {
            return;
        }
        kind &= 0xc0;
        if (mode & 0x20000) {
            kind |= 0x10;
        } else {
            kind |= 0x20;
        }
        func_8004c668(DAT_801d4464.unk12028, a4, a6, a7, a8, a10, a11, a12, a13, a14, a3, kind, rec,
                      target);
        if (mode & 0x10000) {
            obj->unk28 |= 0x800;
        }
    }
}

/* Mean of the Data801b24e4.unk42 stats the record's Record18.unk5 bits pick (5 without 0x40). */
ADDRESS(0x80053624, 0xd4)
u16 func_80053624(Object4c *obj)
{
    u16 sum;
    s32 bit;
    s32 n;
    s32 i;
    u8 bits;

    sum = 0;
    if (obj->unk0 & 0x40) {
        bit = 0x10;
        n = 0;
        bits = DAT_801b77ec.unk0[obj->unk1].unk5;
        for (i = 4; i != -1; i--) {
            if (!(bits & bit)) {
                bit >>= 1;
            } else {
                n++;
                sum += DAT_801b24e4.unk42[i];
                bit >>= 1;
            }
        }
        if (n != 0) {
            sum = sum / n;
        }
        return sum;
    }
    return 5;
}

/* func_800533e8 with the current record and its Record18 record. */
ADDRESS(0x800536f8, 0xf0)
void func_800536f8(s32 flags, s32 a1, VECTOR *target)
{
    Object4c *obj;
    Record18 *rec;

    obj = DAT_801b77ec.unk2f04;
    rec = DAT_801b77ec.unk2f00;
    func_800533e8(flags, obj->unk0, rec, a1, func_80053624(obj), obj->unk6, rec->unk6, rec->unk8,
                  rec->unka, rec->unk4, rec->unkc, rec->unke, rec->unk10, rec->unk12, rec->unk14,
                  target);
}

/* func_800536f8 from the current record's position one step back. */
ADDRESS(0x800537e8, 0x68)
void func_800537e8(s32 flags)
{
    Object4c *obj;
    VECTOR pos;

    obj = DAT_801b77ec.unk2f04;
    pos.vx = obj->pos.vx - obj->vel.vx * 8;
    pos.vy = obj->pos.vy - obj->vel.vy * 8;
    pos.vz = obj->pos.vz - obj->vel.vz * 8;
    func_800536f8(flags, 5000, &pos);
}

/*
 * Area hit of the current record: the player (unk0 bit 0) and the Object88
 * records (bit 1). Both callees read flags as a word, but retail passes it
 * masked to 16 bits (andi) to both.
 */
ADDRESS(0x80053850, 0x1bc)
void func_80053850(VECTOR *pos, s32 reach, s32 radius, s32 mode, s32 falloff, s32 flags)
{
    Object4c *obj;
    Record18 *rec;

    obj = DAT_801b77ec.unk2f04;
    rec = DAT_801b77ec.unk2f00;
    if (obj->unk0 & 1) {
        player_apply_radial_damage(pos, reach, radius, mode, falloff, rec->unk6, rec->unk8, rec->unka,
                                   rec->unk4, rec->unkc, rec->unke, rec->unk10, rec->unk12, rec->unk14,
                                   (u16)flags, obj->unk6);
    }
    if (obj->unk0 & 2) {
        func_8004ceac(pos, reach, radius, mode, falloff, func_80053624(obj), rec->unk6, rec->unk8,
                      rec->unka, rec->unkc, rec->unke, rec->unk10, rec->unk12, rec->unk14,
                      (u16)flags, (obj->unk0 & 0xc0) | 0x20, rec);
    }
}

/* Point vertex of the record's model, scaled and rotated (yaw + 2048). */
ADDRESS(0x80053a0c, 0xe8)
void func_80053a0c(Object4c *obj, s32 vertex, VECTOR *out, SVECTOR *scale)
{
    SVECTOR rotation;
    SVECTOR v;

    func_8004352c(obj->unk3 + 40, obj->unk4, obj->unk14, vertex, &v);
    v.vx = v.vx * scale->vx >> 12;
    v.vy = v.vy * scale->vy >> 12;
    v.vz = v.vz * scale->vz >> 12;
    rotation.vx = obj->angle.vx;
    rotation.vy = obj->angle.vy + 2048;
    rotation.vz = obj->angle.vz;
    vector_rotate_yxz(&rotation, &v, out);
}

/* func_80053a0c in world coordinates. */
ADDRESS(0x80053af4, 0x70)
void func_80053af4(Object4c *obj, s32 vertex, VECTOR *out, SVECTOR *scale)
{
    func_80053a0c(obj, vertex, out, scale);
    out->vx += obj->pos.vx;
    out->vy += obj->pos.vy;
    out->vz += obj->pos.vz;
}

/* A free record, or NULL. */
ADDRESS(0x80053b64, 0x44)
Object4c *func_80053b64(void)
{
    Object4c *obj;
    u16 i;

    obj = DAT_801b77ec.objects;
    i = 128;
    do {
        if (obj->unk0 == OBJECT4C_FREE) {
            return obj;
        }
        obj++;
    } while (--i != 0);
    return NULL;
}

ADDRESS(0x80053ba8, 0x3c)
void func_80053ba8(Object4c *obj, s32 a1, s32 scale, s32 a3)
{
    obj->unk8 = 1;
    obj->unk4 = 0x80;
    obj->unkc = 0x34;
    obj->unk2 = a1;
    obj->unk3 = a1;
    obj->unk9 = a3;
    obj->unk12 = 0x1000;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = scale;
}

ADDRESS(0x80053be4, 0x3c)
void func_80053be4(Object4c *obj, s32 a1, s32 scale, s32 a3)
{
    obj->unk8 = 5;
    obj->unk4 = 0x80;
    obj->unkc = 0x33;
    obj->unk2 = a1;
    obj->unk3 = a1;
    obj->unk9 = a3;
    obj->unk12 = 0x1000;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = scale;
}

ADDRESS(0x80053c20, 0x64)
void func_80053c20(Object4c *obj, s32 a1, s32 a2, s32 y)
{
    obj->unk8 = 0xe;
    obj->unk4 = 0x80;
    obj->unk9 = 5;
    obj->unkc = 0x34;
    obj->unk12 = 0x1000;
    obj->scale.vx = obj->scale.vz = obj->scale.vy = 0x100;
    obj->unk2 = a1;
    obj->unk3 = a1;
    obj->pos.vx = 0xa0;
    obj->pos.vy = y + 120;
    obj->pos.vz = 0;
    obj->angle.vx = a2;
    obj->angle.vy = obj->angle.vz = 0;
}

/*
 * Makes a record of the given id from a free one (NULL when none is free):
 * kind goes to unk6, flags to unk0, pos and vel are copied (vel may be NULL).
 * Each id sets its model, size, life and payload; some read further
 * arguments after vel through &vel (callers pass 5 to 12 arguments).
 * Unknown ids free the record again. kind, flags and id arrive as int and
 * are narrowed at entry.
 */
ADDRESS(0x80053c84, 0x1ec8)
Object4c *func_80053c84(int kind_in, int flags_in, int id_in, VECTOR *pos, SVECTOR *vel, ...)
{
    u8 kind = kind_in;
    u8 flags = flags_in;
    u8 id = id_in;
    Object4c *obj;
    s32 *args;
    Object4cTrail *trail;
    VECTOR p;
    s32 i;
    s32 n;

    args = (s32 *)&vel;
    obj = func_80053b64();
    if (obj == NULL) {
        goto end;
    }
    obj->unk0 = flags;
    obj->unk1 = id;
    if (pos != NULL) {
        obj->pos = *pos;
    }
    obj->unka = 3;
    if (vel != NULL) {
        obj->vel = *vel;
    } else {
        obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
    }
    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
    obj->unk7 = 0;
    obj->unk6 = kind;
    obj->angle.vx = obj->angle.vy = obj->angle.vz = 0;
    obj->unk14 = 0;
    obj->unk5 = 0;
    obj->unk8 = 1;
    if ((obj->unk0 & 0x40) && DAT_801b24e4.unk101 == 1) {
        obj->unkb = 8;
    } else {
        obj->unkb = 1;
    }
    obj->unk12 = 0;
    obj->unk9 = 0;
    obj->unkc = 0xff;
    obj->unk10 = -1;
    if (obj->vel.vx * obj->vel.vx + obj->vel.vy * obj->vel.vy + obj->vel.vz * obj->vel.vz > 810000) {
        obj->unkd = 1;
    } else {
        obj->unkd = 0;
    }
    obj->unke = 0;
    switch (obj->unk1) {
    case 86:
        func_80053be4(obj, 9, 0x1000, 5);
        obj->unke = 0x80;
        obj->unkb = 3;
        goto common0;
    case 40:
        func_80053be4(obj, 9, 0x1000, 5);
        obj->unke = 0x80;
        goto common0;
    case 62:
        func_80053be4(obj, 0x3b, 0x1000, 5);
        obj->unke = 0x40;
        func_800532d4(obj, 0x31);
        goto common0;
    case 0:
        func_80053be4(obj, 9, 0x1000, 5);
        obj->unke = 0x40;
        func_800532d4(obj, 0x31);
    common0:
        obj->unk10 = 50;
        obj->unkd = 1;
        break;
    case 100:
        obj->unk8 = 0;
        obj->unk9 = 5;
        obj->unk4 = 0;
        obj->arg.h.unk44 = 0;
        obj->arg.h.unk46 = 0;
        obj->unk10 = 250;
        break;
    case 47:
        func_80053ba8(obj, 0x40, 0x1000, 5);
        obj->unk10 = 50;
        obj->unke = 0;
        obj->unkd = 1;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->angle.vz = 0;
        obj->arg.h.unk44 = 0;
        break;
    case 48:
        func_80053ba8(obj, 0x41, 0x100, 5);
        obj->unk10 = 50;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->unke = 0x140;
        break;
    case 1:
        obj->unk8 = 1;
        obj->unk9 = 5;
        obj->unk2 = 40;
        obj->unk3 = 40;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->unk4 = 0;
        obj->scale.vx = 2000;
        obj->scale.vy = 10;
        obj->scale.vz = 2000;
        obj->angle = *(SVECTOR *)args[1];
        obj->angle.vx = 0;
        obj->angle.vz = 0;
        obj->vel.vy = 0;
        obj->arg.h.unk44 = 0;
        obj->unk10 = 120;
        obj->unke = 0;
        obj->unkd = 1;
        func_800532d4(obj, 0x31);
        break;
    case 108:
        obj->unk8 = 1;
        obj->unk9 = 5;
        obj->unk2 = 0x38;
        obj->unk3 = 0x38;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->unk4 = 0;
        obj->scale.vx = 4000;
        obj->scale.vy = 0x100;
        obj->scale.vz = 4000;
        obj->angle.vy = args[1];
        obj->angle.vx = 0;
        obj->angle.vz = 0;
        obj->arg.h.unk44 = 0;
        obj->unk10 = 50;
        obj->unke = 0;
        obj->vel.vx = 0;
        obj->vel.vy = 0;
        obj->vel.vz = 0;
        break;
    case 2:
        func_80053be4(obj, 9, 7000, 5);
        obj->vel.vx += (rand() >> 8) - 64;
        obj->vel.vy += (rand() >> 8) - 64;
        obj->vel.vz += (rand() >> 8) - 64;
        obj->arg.b.unk44 = args[1];
        obj->unk10 = 80;
        obj->unkb = 3;
        obj->unke = 0x40;
        if (args[2] == 2) {
            func_800532d4(obj, 0x2d);
        }
        break;
    case 3:
        func_80053be4(obj, 0x45, 0x1000, 5);
        obj->unk10 = 12;
        obj->scale.vy = 0;
        obj->unke = 0x40;
        break;
    case 75:
        func_80053ba8(obj, 0x44, 0x1400, 5);
        obj->angle = DAT_801b24e4.unk124;
        obj->unkd = 1;
        obj->unk10 = 100;
        obj->unkb = 5;
        obj->arg.b.unk44 = 0;
        obj->unke = 0;
        func_800532d4(obj, 0x66);
        break;
    case 51:
        obj->unke = 0;
        goto common6;
    case 6:
        obj->unke = 0x40;
    common6:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 15;
        obj->unk3 = 15;
        obj->arg.b.unk44 = 0;
        obj->unk10 = 70;
        func_800532d4(obj, 0x32);
        break;
    case 76:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 15;
        obj->unk3 = 15;
        obj->unk10 = 80;
        obj->unke = 0x80;
        func_800532d4(obj, 0x36);
        break;
    case 52:
        func_80053ba8(obj, 0x3d, 3600, 5);
        obj->scale.vx = 0x1400;
        obj->scale.vz = 0x1400;
        obj->unk10 = 70;
        obj->unke = 0x100;
        break;
    case 15:
        /* args[2] (0 or 1) is read once for the size, the yaw and the sound. */
        n = args[2];
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 8;
        obj->unk3 = 8;
        obj->unk9 = 5;
        obj->unk10 = 50;
        obj->scale.vy = 3200;
        obj->arg.b.unk44 = 0;
        obj->unkd = 1;
        obj->unke = 0x40;
        obj->scale.vx = n * 2000 + 3200;
        obj->scale.vz = n * 2000 + 3200;
        obj->angle.vy = n << 11;
        if (n == 0) {
            func_800532d4(obj, 0x96);
        }
        break;
    case 39:
        func_80053ba8(obj, 0x24, 0x400, 5);
        obj->unke = 0x80;
        obj->unk10 = 50;
        obj->unkd = 1;
        obj->arg.h.unk44 = 20;
        obj->pos.vy -= 200;
        break;
    case 38:
        func_80053ba8(obj, 0x24, 0x400, 5);
        obj->unke = 0x40;
        obj->unk10 = 50;
        obj->unkd = 1;
        obj->arg.h.unk44 = 20;
        obj->pos.vy -= 200;
        break;
    case 102:
        func_80053be4(obj, 10, 0xc00, 5);
        obj->unk10 = 50;
        obj->unkd = 1;
        func_800532d4(obj, 0x31);
        break;
    case 58:
        func_80053be4(obj, 10, 0xc00, 5);
        obj->unk10 = 50;
        obj->unkd = 1;
        obj->unke = 0x40;
        func_800532d4(obj, 100);
        break;
    case 21:
        func_80053be4(obj, 0x37, 0x1800, 5);
        obj->unk10 = 120;
        obj->unkd = 1;
        obj->arg.h.unk44 = 0;
        obj->unke = 0x40;
        func_800532d4(obj, 0x9a);
        break;
    case 50:
        func_80053be4(obj, 0x42, 0x800, 5);
        obj->unk10 = 70;
        obj->unkb = 3;
        obj->unke = 0;
        obj->vel.vx += (rand() * 20 >> 15) - 10;
        obj->vel.vy += (rand() * 20 >> 15) - 10;
        obj->vel.vz += (rand() * 20 >> 15) - 10;
        break;
    case 13:
    case 56:
        func_800532d4(obj, 0x2b);
        func_80053ba8(obj, 13, 0x1400, 5);
        obj->unk10 = 45;
        obj->arg.b.unk44 = 0;
        obj->unkd = 1;
        obj->angle = DAT_801b24e4.unk124;
        obj->scale.vx = 0x1800;
        obj->scale.vy = 0x1000;
        obj->angle.vx = 0;
        obj->angle.vz = 0;
        obj->scale.vz = 0x2000;
        break;
    case 54:
        func_80053ba8(obj, 0x3d, 0x1000, 5);
        obj->scale.vx = args[1] * 1024 + 0x1800;
        obj->scale.vz = args[1] * 1024 + 0x1800;
        obj->scale.vy = args[1] * 1024 + 0xc00;
        obj->unk10 = 40;
        obj->arg.b.unk44 = 0;
        obj->unkd = 0;
        obj->angle = DAT_801b24e4.unk124;
        obj->unke = 0x40;
        break;
    case 55:
        func_80053ba8(obj, 0x39, 7000, 5);
        obj->unk10 = 48;
        obj->arg.b.unk44 = 0;
        obj->unkd = 0;
        obj->unke = 0x40;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        func_800532d4(obj, 99);
        break;
    case 49:
        func_80053ba8(obj, 0x39, 6000, 5);
        obj->unk10 = 48;
        obj->arg.b.unk44 = 0;
        obj->unkd = 0;
        obj->unke = 0xc0;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        break;
    case 70:
        func_80053ba8(obj, 0x3f, 0x800, 5);
        obj->scale.vz = 0xc00;
        obj->unk10 = 60;
        obj->unkd = 1;
        obj->unkb = 3;
        obj->angle = DAT_801b24e4.unk124;
        obj->arg.h.unk44 = 0;
        obj->arg.h.unk46 = args[1];
        obj->unke = 0x100;
        func_800532d4(obj, 0x2d);
        break;
    case 44:
        func_80053ba8(obj, 0x3f, 0x800, 5);
        obj->unk10 = 55;
        obj->unkd = 1;
        obj->arg.h.unk44 = 0;
        obj->unke = 0x40;
        func_800532d4(obj, 4);
        break;
    case 64:
        func_80053be4(obj, 0x43, 0x1400, 5);
        obj->unk10 = 70;
        obj->arg.b.unk44 = 0;
        obj->unkd = 0;
        obj->unke = 0;
        func_800532d4(obj, 0x67);
        break;
    case 60:
        func_80053ba8(obj, 30, 100, 0);
        obj->scale.vx = 100;
        obj->scale.vy = 0x800;
        obj->scale.vz = 100;
        obj->unk10 = 45;
        obj->angle = DAT_801b24e4.unk124;
        obj->vel.vy = -30;
        obj->angle.vx = 0;
        obj->angle.vz = 0;
        obj->unkb = 5;
        obj->unke = 0;
        obj->arg.h.unk44 = 0;
        obj->pos.vy -= 200;
        break;
    case 17:
        obj->unk8 = 1;
        obj->unk9 = 5;
        obj->unk2 = 50;
        obj->unk3 = 50;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->unk4 = 0;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 3000;
        obj->angle = *(SVECTOR *)args[1];
        obj->angle.vx = 0;
        obj->angle.vz = 0;
        obj->vel.vy = 0;
        obj->unk10 = 150;
        obj->arg.h.unk44 = 0;
        func_800532d4(obj, 0x98);
        break;
    case 89:
        obj->unk8 = 1;
        obj->unk4 = 0;
        obj->unk9 = 5;
        obj->unk2 = 0x1f;
        obj->unk3 = 0x1f;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->scale.vx = 40;
        obj->scale.vy = 2000;
        obj->scale.vz = 40;
        obj->unk10 = 80;
        obj->arg.h.unk44 = 0;
        obj->unkb = 5;
        break;
    case 88:
        func_80053ba8(obj, 0x15, 0, 5);
        obj->scale.vx = args[1];
        obj->scale.vy = args[1];
        obj->scale.vz = args[1];
        obj->unk10 = 45;
        obj->angle.vx = 0;
        obj->angle.vy = 0;
        obj->angle.vz = 0;
        obj->unke = args[2] << 6;
        break;
    case 66:
        if (obj->unk0 & 0x40) {
            func_800532d4(obj, 0x62);
        } else {
            func_800532d4(obj, 0x2c);
        }
    case 42:
        func_80053ba8(obj, 0x2d, 0x1000, 5);
        obj->scale.vz = 0x1400;
        obj->unk10 = 50;
        obj->arg.b.unk44 = 0;
        obj->unkd = 1;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->unke = 0x40;
        break;
    case 46:
        func_80053ba8(obj, 0x2d, 0x1000, 5);
        obj->scale.vz = 0x2000;
        obj->unk10 = 80;
        obj->arg.b.unk44 = 0;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->unke = 0x80;
        break;
    case 53:
        func_80053ba8(obj, 0x41, 0x100, 5);
        obj->unk10 = 50;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->unke = 0x40;
        break;
    case 12:
        func_80053ba8(obj, 0x1f, 2800, 5);
        obj->unk10 = 60;
        obj->scale.vy = 0x1400;
        obj->arg.h.unk44 = obj->arg.h.unk46 = obj->arg.h.unk48 = obj->arg.h.unk4a = 0;
        obj->unke = 0x80;
        func_800532d4(obj, 0x95);
        break;
    case 101:
        func_80053ba8(obj, 0x20, 7000, 5);
        obj->unk10 = 60;
        obj->arg.h.unk44 = obj->arg.h.unk48 = obj->arg.h.unk4a = 0;
        obj->arg.h.unk46 = args[1];
        obj->scale.vy = 0;
        obj->unke = 0x40;
        break;
    case 11:
        func_80053be4(obj, 0x16, 0x1000, 5);
        obj->unk10 = 70;
        obj->unke = 0xc0;
        if ((obj->unk0 & 0x40) && args[1] == 3) {
            func_800532d4(obj, 0x94);
        }
        break;
    case 34:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 0x21;
        obj->unk3 = 0x21;
        obj->unk9 = 5;
        obj->unk10 = 70;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1800;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->unke = 0;
        break;
    case 94:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 30;
        obj->unk3 = 30;
        obj->unk10 = 30;
        obj->unk9 = 0;
        obj->scale.vx = 10;
        obj->scale.vy = 0x1000;
        obj->scale.vz = 10;
        obj->angle.vx = 0x400;
        obj->angle.vz = 0;
        obj->angle.vy = args[1];
        obj->unke = 0;
        func_800532d4(obj, 0x26);
        break;
    case 35:
        obj->unk8 = 0;
        obj->unk10 = 20;
        obj->angle = *(SVECTOR *)args[1];
        obj->unke = 0xc0;
        obj->unkd = 1;
        break;
    case 36:
        obj->unk8 = 0;
        obj->unk10 = 60;
        obj->angle = *(SVECTOR *)args[1];
        obj->unke = 0xc0;
        break;
    case 32:
    case 37:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 0x2b;
        obj->unk3 = 0x2b;
        obj->unk9 = 5;
        obj->unk10 = 70;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->angle = *(SVECTOR *)args[1];
        obj->unke = 0x140;
        break;
    case 33:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 0x2c;
        obj->unk3 = 0x2c;
        obj->unk9 = 5;
        obj->unk10 = 70;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1800;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->unke = 0;
        break;
    case 31:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 0x22;
        obj->unk3 = 0x22;
        obj->unk10 = 70;
        obj->unk9 = 0;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1800;
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vz = 0;
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        obj->vel.vy = 0;
        obj->arg.h.unk44 = 1;
        obj->arg.h.unk46 = 0;
        obj->unke = 0;
        break;
    case 4:
        obj->unk8 = 1;
        obj->unk9 = 5;
        obj->unk2 = 0x29;
        obj->unk3 = 0x29;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->unk4 = 0;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 3000;
        obj->angle = *(SVECTOR *)args[1];
        obj->unk10 = 150;
        obj->arg.b.unk45 = 0;
        obj->unke = 0x80;
        func_800532d4(obj, 0x90);
        break;
    case 9:
        obj->unk8 = 1;
        obj->unk9 = 5;
        obj->unk2 = 0x33;
        obj->unk3 = 0x33;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->unk4 = 0;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 1200;
        obj->angle = *(SVECTOR *)args[1];
        obj->angle.vx = 0;
        obj->angle.vz = 0;
        obj->vel.vy = 0;
        obj->unk10 = 150;
        obj->arg.h.unk44 = 0;
        obj->arg.h.unk46 = DAT_801b24e4.unk124.vy;
        obj->unke = 0;
        func_800532d4(obj, 0x92);
        break;
    case 7:
    case 22:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 14;
        obj->unk3 = 14;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->scale.vz = 0;
        obj->scale.vx = 0;
        obj->arg.h.unk44 = args[1];
        obj->arg.h.unk46 = args[2];
        obj->arg.h.unk48 = args[3];
        obj->unke = args[4] << 6;
        func_800532d4(obj, 0x23);
        break;
    case 16:
        obj->unk4 = 0x80;
        obj->unk9 = 5;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->unkc = 0x34;
        obj->unk8 = 0;
        obj->unk2 = 0;
        obj->unk3 = 0;
        obj->unk12 = 0x800;
        obj->angle = *(SVECTOR *)args[1];
        obj->arg.w.unk44 = DAT_801e7594[DAT_8009c338];
        trail = obj->arg.w.unk44;
        DAT_8009c338 = (DAT_8009c338 + 1) & 3;
        for (i = 23; i != -1; i--) {
            trail->pos = obj->pos;
            trail->angle = obj->angle;
            trail++;
        }
        obj->unk10 = 150;
        obj->arg.b.unk48 = 0;
        obj->arg.b.unk49 = 0;
        obj->unke = 0x40;
        func_800532d4(obj, 0x97);
        break;
    case 5:
        obj->unk4 = 0x80;
        obj->unk9 = 4;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->unkc = 0x34;
        obj->unk8 = 0;
        obj->unk2 = 0;
        obj->unk3 = 0;
        obj->unk12 = 0x800;
        obj->angle = *(SVECTOR *)args[1];
        obj->arg.w.unk44 = DAT_801e7594[DAT_8009c338];
        trail = obj->arg.w.unk44;
        DAT_8009c338 = (DAT_8009c338 + 1) & 3;
        for (i = 23; i != -1; i--) {
            trail->pos = obj->pos;
            trail->angle = obj->angle;
            trail++;
        }
        obj->arg.b.unk48 = 0;
        obj->arg.b.unk49 = 0;
        obj->unk10 = 150;
        obj->unke = 0;
        func_800532d4(obj, 0x91);
        break;
    case 116:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk9 = 5;
        obj->unk2 = 0x11;
        obj->unk3 = 0x11;
        obj->unkc = 0x34;
        obj->unk12 = 0x800;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->angle = *(SVECTOR *)args[1];
        obj->unke = 0x40;
        break;
    case 103:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk9 = 4;
        obj->unk2 = 0x11;
        obj->unk3 = 0x11;
        obj->unkc = 0x34;
        obj->unk12 = 0x800;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->angle = *(SVECTOR *)args[1];
        obj->unk10 = 200;
        obj->unke = 0x80;
        break;
    case 125:
        func_80053be4(obj, 10, 0x1000, 5);
        obj->unke = 0;
        goto common113;
    case 129:
        func_80053be4(obj, 10, 0x1000, 5);
        obj->unke = 0xc0;
        goto common113;
    case 113:
        func_80053be4(obj, 10, 0x800, 5);
        obj->unke = 0x40;
    common113:
        obj->unk10 = 100;
        obj->arg.h.unk44 = args[1];
        func_800532d4(obj, 0x2e);
        break;
    case 114:
    case 126:
    case 128:
        func_80053be4(obj, 11, 0x1000, 5);
        obj->unke = 0x40;
        obj->scale.vy = args[1];
        obj->unk10 = 12;
        break;
    case 41:
        obj->unk2 = 0x15;
        obj->unk3 = 0x15;
        obj->unke = 0xc0;
        goto common19;
    case 45:
        obj->unk2 = 0x15;
        obj->unk3 = 0x15;
        obj->unke = 0;
        goto common19;
    case 19:
        obj->unk2 = 0x15;
        obj->unk3 = 0x15;
        obj->unke = 0x40;
    common19:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk9 = 5;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
        obj->angle.vx = obj->angle.vy = obj->angle.vz = 0;
        obj->arg.h.unk44 = args[1] / 4;
        func_800532d4(obj, 0x26);
        break;
    case 90:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 30;
        obj->unk3 = 30;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->scale.vx = 10;
        obj->scale.vz = 10;
        obj->arg.h.unk44 = args[1];
        obj->arg.h.unk46 = args[2];
        obj->arg.h.unk48 = args[3];
        obj->unke = 0;
        func_800532d4(obj, 0x23);
        break;
    case 18:
        obj->unkb = 3;
        obj->unke = 0x140;
        obj->unk8 = 1;
        obj->unk9 = 5;
        obj->unk4 = 0x80;
        obj->unk2 = 0x23;
        obj->unk3 = 0x23;
        obj->unkc = 0x39;
        obj->unk12 = 0x1000;
        obj->angle = *(SVECTOR *)args[1];
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->arg.h.unk44 = args[2];
        obj->arg.h.unk46 = args[3];
        obj->arg.h.unk48 = args[4];
        obj->arg.h.unk4a = args[5];
        obj->unk10 = args[6];
        break;
    case 20:
    case 71:
        obj->unke = 0x100;
        obj->unk8 = 1;
        obj->unk9 = 5;
        obj->unk4 = 0x80;
        obj->unk2 = 0x23;
        obj->unk3 = 0x23;
        obj->unkc = 0x39;
        obj->unk12 = 0x1000;
        obj->angle = *(SVECTOR *)args[1];
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->arg.h.unk44 = args[2];
        obj->arg.h.unk46 = args[3];
        obj->arg.h.unk48 = args[4];
        obj->arg.h.unk4a = args[5];
        obj->unk10 = args[6];
        break;
    case 72:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk9 = 5;
        obj->unk2 = 0x15;
        obj->unk3 = 0x15;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
        obj->arg.b.unk44 = args[1];
        obj->unke = 0x40;
        break;
    case 120:
    case 122:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk9 = 5;
        obj->unk2 = 0x3a;
        obj->unk3 = 0x3a;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->unk10 = 45;
        obj->angle = *(SVECTOR *)args[1];
        obj->scale.vx = 0x1800;
        obj->scale.vy = 0xa00;
        obj->scale.vz = 0xc00;
        obj->unke = 0;
        func_800532d4(obj, 0x66);
        break;
    case 92:
        func_80053be4(obj, 11, 0x1800, 5);
        obj->scale.vy = 0;
        obj->arg.b.unk44 = 0;
        obj->arg.b.unk45 = args[1];
        obj->vel.vy = 0;
        obj->unke = 0x40;
        break;
    case 93:
        func_80053be4(obj, 11, 0x1800, 5);
        obj->scale.vy = 0;
        obj->arg.b.unk44 = 0;
        obj->arg.b.unk45 = args[1];
        obj->vel.vy = 0;
        obj->unke = 0;
        break;
    case 68:
    case 69:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk9 = 5;
        obj->unk2 = 0x15;
        obj->unk3 = 0x15;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
        obj->unke = 0xc0;
        obj->arg.h.unk44 = args[1];
        break;
    case 124:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk9 = 5;
        obj->unk2 = 0x20;
        obj->unk3 = 0x20;
        obj->unkc = 0x34;
        obj->unk12 = 0x1000;
        obj->unk10 = 50;
        obj->angle = DAT_801b24e4.unk124;
        obj->scale.vx = 350;
        obj->scale.vy = 0x2000;
        obj->scale.vz = 350;
        obj->angle.vx = 0;
        obj->angle.vz = 0;
        obj->vel.vy = 0;
        obj->unke = 0x40;
        obj->pos.vy += 2400;
        func_800532d4(obj, 0x65);
        break;
    case 123:
        obj->unk8 = 0;
        obj->unk10 = 20;
        func_800532d4(obj, 0x65);
        break;
    case 111:
        func_80053be4(obj, args[4], args[1], args[6] | 4);
        obj->arg.h.unk44 = args[2];
        obj->unk10 = args[3];
        obj->arg.h.unk46 = args[5];
        obj->unke = args[7] << 6;
        break;
    case 91:
        obj->unk8 = 5;
        obj->unk4 = 0x80;
        obj->unk2 = 11;
        obj->unk3 = 11;
        obj->unk9 = 5;
        obj->unkc = 0x33;
        obj->unk12 = 0x1000;
        obj->scale.vx = 0x1000;
        obj->scale.vy = 0xc00;
        obj->scale.vz = 0x1000;
        obj->unk10 = 20;
        obj->unke = 0x40;
        break;
    case 112:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 0x2a;
        obj->unk3 = 0x2a;
        obj->scale.vy = 0;
        obj->scale.vx = obj->scale.vz = args[1];
        obj->arg.h.unk44 = args[2];
        if (DAT_8009c3f0 - (s32)DAT_801c12e4.frames < 0) {
            DAT_8009c3f0 = DAT_801c12e4.frames + 30;
            /* A volume that nothing uses any more: retail keeps the multiply. */
            i = (s16)obj->arg.h.unk44 / 45;
            if (i > 0x7f) {
                i = 0x7f;
            }
        }
        break;
    case 26:
        func_80053c20(obj, 0x18, args[1] << 11, args[1] * 14 - 7);
        obj->unke = 0xc0;
        break;
    case 27:
        func_80053c20(obj, 0x2f, 0, 0);
        obj->unke = 0x80;
        break;
    case 28:
        func_80053c20(obj, 0x18, args[1] << 11, args[1] * 14 - 7);
        obj->unke = 0x80;
        break;
    case 23:
        obj->unk8 = 0;
        obj->unk10 = 8;
        audio_play_sound(9, 120);
        break;
    case 25:
        obj->unk8 = 0;
        obj->unk10 = 16;
        audio_play_sound(9, 120);
        obj->unke = 0x80;
        break;
    case 30:
        obj->unk8 = 0;
        obj->unk10 = 16;
        audio_play_sound(9, 120);
        obj->unke = 0x40;
        break;
    case 95:
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unk2 = 15;
        obj->unk3 = 15;
        obj->unk10 = 45;
        obj->arg.w.unk48 = obj->pos.vy;
        p.vx = obj->pos.vx + ((rand() * 2600 >> 15) - 1300);
        p.vz = obj->pos.vz + ((rand() * 2600 >> 15) - 1300);
        if (collision_query_shapes_with_layer_sample(p.vx, obj->pos.vy, p.vz, 10, 10) != 0) {
            p.vx = obj->pos.vx;
            p.vz = obj->pos.vz;
        }
        obj->pos.vx = p.vx - 2520;
        obj->pos.vz = p.vz - 2520;
        obj->pos.vy -= 16384;
        obj->vel.vx = 100;
        obj->vel.vy = 650;
        obj->vel.vz = 100;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->unke = 0x80;
        func_800532d4(obj, 0x2d);
        break;
    case 82:
        obj->unk2 = 6;
        obj->unke = 0;
        goto common80;
    case 80:
    case 81:
    case 83:
        obj->unk2 = 5;
        obj->unke = 0;
    common80:
        obj->unk3 = obj->unk2;
        obj->unk8 = 1;
        obj->unk4 = 0x80;
        obj->unkc = 0xff;
        obj->angle = *(SVECTOR *)args[1];
        obj->angle.vz = 0;
        obj->arg.h.unk46 = 0;
        break;
    case 85:
        func_80053be4(obj, 9, 0x400, 5);
        obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
        obj->unk7 = 9;
        obj->arg.h.unk44 = args[1];
        obj->arg.h.unk46 = args[2];
        obj->arg.h.unk48 = args[3];
        obj->unke = 0x40;
        break;
    default:
        obj->unk0 = OBJECT4C_FREE;
        break;
    }
end:
    return obj;
}
