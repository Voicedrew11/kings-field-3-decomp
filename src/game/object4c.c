/*
 * GAME.EXE: Object4c records (WIP module): their sound, collision and hit
 * tests, record allocation and the set-up helpers of func_80053c84.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/object4c.h>

/* Plays sound effect se at the record. */
ADDRESS(0x800532d4, 0x3c)
s32 func_800532d4(Object4c *obj, s32 se)
{
    return func_80015688(se, &obj->pos, 0x6e, 0x6d60, 0x7148, 0);
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
        return func_80033f38(pos->vx, y, pos->vz, radius, height, mask);
    }
    obj->unkb--;
    return 0;
}

/*
 * Hit by the current record: the player (kind 0x80) or the Object88 record
 * of unk12028 (0x10). K&R definition: func_800536f8 passes a3 unmasked, so
 * the narrow parameters arrive as int. Not exact: retail spills a3 with sw
 * and reads it back with lhu; this source spills it with sh.
 */
ADDRESS(0x800533e8, 0x23c)
void func_800533e8(flags, kind, rec, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, target)
s32 flags;
s32 kind;
Record18 *rec;
u16 a3;
u16 a4;
u8 a5;
u16 a6;
u16 a7;
u16 a8;
u16 a9;
u16 a10;
u16 a11;
u16 a12;
u16 a13;
u16 a14;
VECTOR *target;
{
    s32 mode;
    Object88 *obj;
    Object78 *def;

    mode = flags & 0xf0000;
    flags &= 0xfff0ffff;
    if (flags == 0x80) {
        func_8002ab18(a6, a7, a8, a9, a10, a11, a12, a13, a14, a3, a5, target);
    } else if (flags == 0x10) {
        obj = &DAT_80185da8.objects[DAT_801d4464.unk12028];
        def = &DAT_80185da8.defs[obj->unk2];
        if (target != NULL &&
            !func_80016a2c(obj->angle.vy,
                           func_80016ab8(obj->pos.vx - target->vx, obj->pos.vz - target->vz) + 2048,
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
        func_8002b114(pos, reach, radius, mode, falloff, rec->unk6, rec->unk8, rec->unka, rec->unk4,
                      rec->unkc, rec->unke, rec->unk10, rec->unk12, rec->unk14, (u16)flags,
                      obj->unk6);
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
    func_800168a8(&rotation, &v, out);
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
