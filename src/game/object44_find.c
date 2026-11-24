/*
 * GAME.EXE: Object44 searches and spawns (WIP module): the collision search
 * over the records, the facing test, and the pools of spawned records at the
 * end of the table.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <RAND.H>
#include <game/math.h>
#include <game/sound.h>
#include <game/image.h>
#include <game/player.h>
#include <game/render.h>
#include <game/data.h>
#include <game/object44.h>

RODATA(0x80012054, 0x44)

/* Index of the first record a shape at (x, y, z) hits, -1 for none. */
ADDRESS(0x80045ac8, 0x1b4)
s32 func_80045ac8(s32 x, s32 y, s32 z, s32 radius, s32 shape)
{
    Object44 *obj;
    Object44Def *def;
    s32 i;
    s32 r;

    obj = DAT_8018fb3c.objects;
    for (i = 0; i < 396; i++, obj++) {
        if (obj->unk6 == 0xff) {
            continue;
        }
        if (obj->unk0 == 0) {
            continue;
        }
        if (obj == DAT_8018fb3c.unk8858) {
            continue;
        }
        def = &DAT_8018fb3c.defs[obj->unk6];
        if (def->unk3 & 0x10) {
            r = def->unk4 * obj->arg.b.unk38 >> 7;
        } else {
            r = def->unk4;
        }
        if (r != 0) {
            if (func_80016ec8(&obj->pos, x, y, z, r + radius, obj->unkc, shape) != -1) {
                return i;
            }
        } else if (obj->unk3 & 4) {
            if (func_80016d3c(&obj->angle, &obj->pos, x, y, z, def->unke + radius,
                              def->unk10 + radius, obj->unkc, shape) != -1) {
                return i;
            }
        }
    }
    return -1;
}

/*
 * Index of the first record from start that the point at pos (radius
 * widened by height) touches and that faces it within range of angle, -1 for
 * none. Types 0x1b, 0 and 1 are tested 0x700, -0x700 and -0x400 along their
 * yaw.
 */
ADDRESS(0x80045c7c, 0x398)
s32 func_80045c7c(start, pos, height, reach_up, angle, range)
    s32 start;
    VECTOR *pos;
    s32 height;
    s32 reach_up;
    s16 angle;
    s16 range;
{
    SVECTOR offset;
    VECTOR at;
    MATRIX m;
    Object44 *obj;
    Object44Def *def;
    s16 i;

    obj = &DAT_8018fb3c.objects[start];
    for (i = start; i < 396; i++, obj++) {
        if (obj->unk6 == 0xff) {
            continue;
        }
        def = &DAT_8018fb3c.defs[obj->unk6];
        if (def->unk0 == 0x1b) {
            offset.vx = 0x700;
            offset.vy = 0;
            offset.vz = 0;
            func_8001660c(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (func_80016ec8(&obj->pos, at.vx, pos->vy, at.vz, def->unk6 + height, obj->unkc,
                              reach_up) == -1) {
                continue;
            }
            if (func_80016a2c(angle, func_80016ab8(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                              range)) {
                return i;
            }
        } else if (def->unk0 == 0) {
            offset.vx = -0x700;
            offset.vy = 0;
            offset.vz = 0;
            func_8001660c(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (func_80016ec8(&obj->pos, at.vx, pos->vy, at.vz, def->unk6 + height, obj->unkc,
                              reach_up) == -1) {
                continue;
            }
            if (func_80016a2c(angle, func_80016ab8(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                              range)) {
                return i;
            }
        } else if (def->unk0 == 1) {
            offset.vx = -0x400;
            offset.vy = 0;
            offset.vz = 0;
            func_8001660c(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (func_80016ec8(&obj->pos, at.vx, pos->vy, at.vz, def->unk6 + height, obj->unkc,
                              reach_up) == -1) {
                continue;
            }
            if (func_80016a2c(angle, func_80016ab8(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                              range)) {
                return i;
            }
        } else {
            if (func_80016ec8(&obj->pos, pos->vx, pos->vy, pos->vz, def->unk6 + height, def->unk8,
                              reach_up) == -1) {
                continue;
            }
            if (obj->unk6 == 0x95) {
                return i;
            }
            if (func_80016a2c(angle, func_80016ab8(obj->pos.vx - pos->vx, obj->pos.vz - pos->vz),
                              range)) {
                return i;
            }
        }
    }
    return -1;
}

/* Gives a record without a kind the kind `kind`. */
ADDRESS(0x80046014, 0x20)
void func_80046014(Object44 *obj, u8 kind)
{
    if (obj->unk4 == 0xff) {
        obj->unk4 = kind;
        obj->unk8 = 0;
    }
}

/*
 * A free record among the count from first, else (stamp not -1) the one
 * whose arg.h.unk3c stamp is oldest, else NULL.
 */
ADDRESS(0x80046034, 0x88)
Object44 *func_80046034(s32 first, s32 count, s32 stamp)
{
    Object44 *obj;
    Object44 *oldest;
    s32 best;
    s32 age;

    obj = &DAT_8018fb3c.objects[first];
    oldest = NULL;
    best = 0;
    do {
        if (obj->unk6 == 0xff) {
            return obj;
        }
        if (stamp != -1) {
            age = stamp - obj->arg.h.unk3c;
            if (age < 0) {
                age += 0x10000;
            }
            if (best < age) {
                oldest = obj;
                best = age;
            }
        }
        obj++;
    } while (--count != 0);
    return oldest;
}

/*
 * Spawns a record of Object44Def def_id at pos raised by dy in one of the
 * two ten-record pools (which 0: from 370, else from 360), replacing the
 * oldest when the pool is full.
 */
ADDRESS(0x800460bc, 0x1d8)
void func_800460bc(u8 which, u8 def_id, VECTOR *pos, s32 dy)
{
    Object44 *obj;
    u16 *stamp;
    s32 first;
    Object44Def *def;

    if (which == 0) {
        stamp = &DAT_8018fb3c.unk8862;
        first = 370;
    } else {
        first = 360;
        stamp = &DAT_8018fb3c.unk8860;
    }
    obj = func_80046034(first, 10, *stamp);
    obj->arg.h.unk3c = (*stamp)++;
    obj->unk6 = def_id;
    func_800448b8(obj);
    def = &DAT_8018fb3c.defs[obj->unk6];
    if (def->unk3 & 0x10) {
        obj->scale.vx <<= 1;
        obj->scale.vy <<= 1;
        obj->scale.vz <<= 1;
    }
    obj->pos.vx = pos->vx;
    obj->pos.vy = dy + pos->vy;
    obj->pos.vz = pos->vz;
    obj->angle.vy = rand() >> 3;
    switch (def->unk1) {
    case 16:
    case 19:
    case 22:
        func_80046014(obj, 0x60);
        break;
    case 23:
        func_80046014(obj, 0x61);
        break;
    case 17:
    case 18:
    case 20:
    case 21:
    case 24:
    case 25:
    case 32:
        func_80046014(obj, 0x62);
        if (def_id == 0x5c || def_id == 0x86 || def_id == 0x88 || def_id == 0x8d || def_id == 0x8f) {
            obj->angle.pad = 0;
        } else {
            obj->angle.pad = 0x400;
        }
        break;
    }
    obj->arg.h.unk3e = 0;
    obj->arg.b.unk38 = 0xff;
}

/*
 * Spawns an Object44Def 0x95 record (kind 0x62) for Object44 target in the
 * ten-record pool from 350, at a random point 600 from pos raised by dy.
 */
ADDRESS(0x80046294, 0x128)
void func_80046294(u16 target, VECTOR *pos, s32 dy)
{
    Object44 *obj;
    u16 angle;

    obj = func_80046034(350, 10, DAT_8018fb3c.unk885e);
    func_800448b8(obj);
    obj->arg.h.unk3c = DAT_8018fb3c.unk885e++;
    obj->unk6 = 0x95;
    obj->arg.h.unk3a = target;
    angle = rand() >> 3;
    obj->pos.vx = (rsin(angle) * 600 >> 12) + pos->vx;
    obj->pos.vy = dy + pos->vy;
    obj->pos.vz = (rcos(angle) * 600 >> 12) + pos->vz;
    obj->angle.vy = rand() >> 3;
    obj->arg.b.unk38 = 0xff;
    func_80046014(obj, 0x62);
    obj->arg.h.unk3e = -120;
}

/* Passes the switch event id to every record whose kind listens for it. */
ADDRESS(0x800463bc, 0x24c)
void func_800463bc(u8 id)
{
    Object44 *obj;
    u16 i;
    u8 v;

    obj = DAT_8018fb3c.objects;
    if (id == 0xff) {
        return;
    }
    i = 396;
    while (i--) {
        switch (obj->unk4) {
        case 0x54:
        case 162:
        case 163:
            if (obj->arg.b.unk38 == id) {
                obj->arg.b.unk38 = 0xff;
            }
            break;
        case 0x5f:
            if (obj->arg.b.unk38 == id) {
                obj->unk8 = 1;
                obj->arg.b.unk3f = obj->arg.b.unk3f == 0;
            }
            break;
        case 0x58:
            if (obj->arg.b.unk39 == id) {
                obj->unk8 = 1;
                obj->arg.b.unk38 = obj->arg.b.unk38 == 0;
            }
            break;
        case 0x51:
            if (obj->arg.b.unk3d[0] == id) {
                obj->arg.b.unk38 = obj->arg.b.unk38 == 0 ? 0xff : 0;
            }
            break;
        case 0x52:
        case 0x55:
        case 0x5a:
            if (obj->arg.b.unk3f == id) {
                obj->arg.b.unk38 = obj->arg.b.unk38 == 0 ? 0xff : 0;
            }
            break;
        case 0x59:
            if (obj->arg.b.unk38 == id) {
                obj->unk8 = 1;
            }
            break;
        case 0x56:
            if (obj->arg.b.unk3a.bytes[0] == id) {
                obj->arg.b.unk39 = obj->arg.b.unk39 == 0 ? 0xff : 0;
            }
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 0x1b:
            if (id >= 150 && id < 199) {
                v = obj->arg.b.unk38;
                if ((v & 0xfe) == id) {
                    obj->arg.b.unk38 = v ^ 1;
                }
            } else {
                v = obj->arg.b.unk38;
                if (v == id) {
                    if (v < 200) {
                        if (obj->unk8 != 0) {
                            break;
                        }
                        obj->unk8 = 1;
                        if (v < 100) {
                            break;
                        }
                    }
                    obj->arg.b.unk38 = 0xff;
                }
            }
            break;
        }
        obj++;
    }
}

/*
 * Answers switch event id for one record: 0 no switch (or a used one-shot),
 * 1 it was waiting for id (now cleared), 2 none pending, 3 another id,
 * 4 a kind that always reacts.
 */
ADDRESS(0x80046608, 0xe0)
s32 func_80046608(Object44 *obj, s32 id)
{
    switch (obj->unk4) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 0x1b:
        if (obj->unk8 != 0) {
            return 0;
        }
        /* fall through */
    case 6:
    case 7:
    case 8:
    case 0x17:
    case 0x18:
    case 0x1a:
        if (obj->arg.b.unk38 >= 254) {
            return 2;
        }
        if (obj->arg.b.unk38 == id) {
            obj->arg.b.unk38 = 0xff;
            return 1;
        }
        return 3;
    case 0xf:
    case 0x11:
    case 0xa7:
        return 4;
    }
    return 0;
}

/* func_80046608 for every record. */
ADDRESS(0x800466e8, 0x78)
void func_800466e8(u8 id)
{
    Object44 *obj;
    u16 i;

    obj = DAT_8018fb3c.objects;
    if (id == 0xff) {
        return;
    }
    i = 396;
    while (i--) {
        func_80046608(obj, id);
        obj++;
    }
}

/* World position of vertex of the record's animated model. */
ADDRESS(0x80046760, 0x124)
void func_80046760(Object44 *obj, s32 vertex, VECTOR *out)
{
    SVECTOR rot;
    SVECTOR v;

    func_8004352c(func_80040568(obj->unk6), obj->unk1, obj->unka, vertex, &v);
    v.vx = v.vx * obj->scale.vx >> 12;
    v.vy = v.vy * obj->scale.vy >> 12;
    v.vz = v.vz * obj->scale.vz >> 12;
    rot.vx = obj->angle.vx;
    rot.vy = obj->angle.vy + 2048;
    rot.vz = obj->angle.vz;
    func_800168a8(&rot, &v, out);
    out->vx += obj->pos.vx;
    out->vy += obj->pos.vy;
    out->vz += obj->pos.vz;
}

/*
 * 1 when the player stands in the map cell rectangle (x, z, w, h) and, unless
 * y is 0x8000, between 2048 above and 3200 below y.
 */
ADDRESS(0x80046884, 0x90)
s32 func_80046884(s32 x, s32 z, s32 w, s32 h, s32 y)
{
    s32 px;
    s32 pz;

    px = DAT_801b24e4.pos.vx >> 11;
    pz = DAT_801b24e4.pos.vz >> 11;
    if (x <= px && px < x + w && z <= pz && pz < z + h) {
        if (y != 0x8000) {
            if (y + 2048 < DAT_801b24e4.pos.vy) {
                return 0;
            }
            if (DAT_801b24e4.pos.vy < y - 3200) {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

/*
 * Moves record dst (made kind 0x70 on the first call) from offset from to
 * offset to of record src in steps calls, fading it in when fade is set.
 * Returns 1 on the last step.
 */
ADDRESS(0x80046914, 0x2ec)
s32 func_80046914(Object44 *src, Object44 *dst, SVECTOR *from, SVECTOR *to, s32 fade, s32 steps)
{
    VECTOR a;
    VECTOR b;
    s32 t;

    switch (src->unk8) {
    case 0:
        if (src->arg.b.unk38 == 0xff) {
            return 0;
        }
        if (dst->arg.b.unk38 == 0) {
            src->arg.b.unk40 = 0;
        } else {
            src->arg.b.unk40 = steps - 1;
        }
        src->unk8 = 1;
        func_800448b8(dst);
        dst->unk4 = 0x70;
        dst->unk0 = src->unk0;
        switch (dst->unk6) {
        case 0x88:
            dst->angle.vx = 0x400;
            dst->unke = 100;
            break;
        case 0x85:
            dst->unke = -220;
            break;
        }
        if (fade) {
            dst->unk1 = 0;
        } else {
            dst->unk1 = 0x80;
        }
        func_800168a8(&src->angle, from, &dst->pos);
        dst->pos.vx += src->pos.vx;
        dst->pos.vy += src->pos.vy;
        dst->pos.vz += src->pos.vz;
        dst->angle.vy = src->angle.vy;
        dst->arg.h.unk40 = src - DAT_8018fb3c.objects;
        /* fall through */
    case 1:
        if (fade) {
            dst->unka += 204;
            if (dst->unka >= 4096) {
                dst->unka = 0xfff;
            }
        }
        func_800168a8(&src->angle, from, &a);
        func_800168a8(&src->angle, to, &b);
        src->arg.b.unk40++;
        t = (src->arg.b.unk40 << 12) / steps;
        dst->pos.vx = func_80017158(a.vx, b.vx, t) + src->pos.vx;
        dst->pos.vy = func_80017158(a.vy, b.vy, t) + src->pos.vy;
        dst->pos.vz = func_80017158(a.vz, b.vz, t) + src->pos.vz;
        if (src->arg.b.unk40 < steps) {
            return 0;
        }
        if (fade) {
            dst->unka = 0xfff;
        }
        if ((s16)DAT_8018fad4.loading == 0) {
            func_8004482c(dst, 0x12);
        }
        dst->arg.b.unk38 = 0xff;
        src->unk8 = 0x63;
        return 1;
    }
    return 0;
}

/* Runs frames while the screen tint (mode) steps its level from level to end. */
ADDRESS(0x80046c00, 0xb0)
void func_80046c00(mode, level, end, step)
    u8 mode;
    s32 level;
    s32 end;
    s32 step;
{
    VECTOR pos;
    SVECTOR angle;
    s32 v;

    for (;;) {
        v = level * level >> 16;
        if (v >= 256) {
            v = 0xff;
        }
        func_8003d624(mode, v, v, v);
        func_80015a48();
        func_80018cd0();
        func_8002b330(&pos, &angle);
        func_800422b8(&pos, &angle);
        if (level == end) {
            break;
        }
        level += step;
    }
}

/* Moves the player, the Object4c and the Object88 records by (dx, dy, dz). */
ADDRESS(0x80046cb0, 0x11c)
void func_80046cb0(s32 dx, s32 dy, s32 dz)
{
    Object4c *o4c;
    Object88 *o88;
    u16 i;

    DAT_801b24e4.pos.vx += dx;
    DAT_801b24e4.pos.vz += dz;
    DAT_801b24e4.pos.vy += dy;
    o4c = DAT_801b77ec.objects;
    i = 128;
    while (i--) {
        if (o4c->unk0 != 0xff && (o4c->unk8 & 0xc) != 0xc) {
            o4c->pos.vx += dx;
            o4c->pos.vz += dz;
            o4c->pos.vy += dy;
        }
        o4c++;
    }
    o88 = DAT_80185da8.objects;
    i = 200;
    while (i--) {
        if (o88->unk0 != 0xff) {
            o88->pos.vx += dx;
            o88->pos.vz += dz;
            o88->pos.vy += dy;
        }
        o88++;
    }
}

/*
 * Turns the view at from toward to (1600 above it) in count steps, the yaw
 * eased from obj's by rate per step and the pitch fixed unless pitch is -1;
 * out is the view direction scaled by dist. Returns the last yaw.
 */
ADDRESS(0x80046dcc, 0x244)
s16 func_80046dcc(Object44 *obj, VECTOR *from, s32 dist, VECTOR *to, SVECTOR *out, s16 pitch,
                  s16 rate, s32 count)
{
    VECTOR pos;
    SVECTOR rot;
    s32 d;

    pos = *from;
    for (;;) {
        func_80016c90(pos.vx - to->vx, -(to->vy + 1600) + pos.vy, pos.vz - to->vz, &rot);
        d = (rot.vy - obj->angle.vy) & 0xfff;
        if (d >= 2048) {
            d = 4096 - d;
        }
        d = (d << 12) / rate;
        if (d > 4096) {
            d = 4096;
        }
        rot.vy = func_80017170(rot.vy, obj->angle.vy, d);
        if (pitch == -1) {
            d = (rot.vx - obj->angle.vx) & 0xfff;
            if (d >= 2048) {
                d = 4096 - d;
            }
            rot.vx = func_80017170(rot.vx, obj->angle.vx, d);
        } else {
            rot.vx = pitch;
        }
        func_800167cc(&rot, out);
        func_80016928(dist, out);
        if (--count == 0) {
            break;
        }
        func_800175a8(from, &DAT_801b24e4.unk11c,
                      func_80016c44(to->vx - pos.vx, to->vy - pos.vy, to->vz - pos.vz) / dist, &pos);
    }
    return rot.vy;
}
