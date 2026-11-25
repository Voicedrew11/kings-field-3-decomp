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
#include <game/cd.h>
#include <game/collision.h>
#include <game/entity.h>
#include <game/map_patch.h>
#include <game/object4c.h>
#include <game/scene.h>

RODATA(0x80012054, 0x47c)
SDATA(0x8009c2f4, 0x44)

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
            if (vector_distance_to_point(&obj->pos, x, y, z, r + radius, obj->unkc, shape) != -1) {
                return i;
            }
        } else if (obj->unk3 & 4) {
            if (vector_distance_to_point_in_rotated_box(&obj->angle, &obj->pos, x, y, z,
                                                        def->arg.h.unke + radius, def->arg.h.unk10 + radius,
                                                        obj->unkc, shape) != -1) {
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
s32 func_80045c7c(s32 start, VECTOR *pos, s32 height, s32 reach_up, int angle_in, int range_in)
{
    s16 angle = angle_in;
    s16 range = range_in;
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
            matrix_set_rotation_y(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (vector_distance_to_point(&obj->pos, at.vx, pos->vy, at.vz, def->unk6 + height, obj->unkc,
                                         reach_up) == -1) {
                continue;
            }
            if (angle_within_tolerance(angle, vector_xz_to_angle(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                                       range)) {
                return i;
            }
        } else if (def->unk0 == 0) {
            offset.vx = -0x700;
            offset.vy = 0;
            offset.vz = 0;
            matrix_set_rotation_y(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (vector_distance_to_point(&obj->pos, at.vx, pos->vy, at.vz, def->unk6 + height, obj->unkc,
                                         reach_up) == -1) {
                continue;
            }
            if (angle_within_tolerance(angle, vector_xz_to_angle(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                                       range)) {
                return i;
            }
        } else if (def->unk0 == 1) {
            offset.vx = -0x400;
            offset.vy = 0;
            offset.vz = 0;
            matrix_set_rotation_y(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (vector_distance_to_point(&obj->pos, at.vx, pos->vy, at.vz, def->unk6 + height, obj->unkc,
                                         reach_up) == -1) {
                continue;
            }
            if (angle_within_tolerance(angle, vector_xz_to_angle(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                                       range)) {
                return i;
            }
        } else {
            if (vector_distance_to_point(&obj->pos, pos->vx, pos->vy, pos->vz, def->unk6 + height, def->unk8,
                                         reach_up) == -1) {
                continue;
            }
            if (obj->unk6 == 0x95
                || angle_within_tolerance(angle, vector_xz_to_angle(obj->pos.vx - pos->vx,
                                                                    obj->pos.vz - pos->vz),
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
    Object44Def *def;

    if (which == 0) {
        obj = func_80046034(370, 10, DAT_8018fb3c.unk8862);
        obj->arg.h.unk3c = DAT_8018fb3c.unk8862++;
        obj->unk6 = def_id;
    } else {
        obj = func_80046034(360, 10, DAT_8018fb3c.unk8860);
        obj->arg.h.unk3c = DAT_8018fb3c.unk8860++;
        obj->unk6 = def_id;
    }
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
    obj->arg.sh.unk3e = 0;
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
    obj->arg.sh.unk3e = -120;
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
    vector_rotate_yxz(&rot, &v, out);
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
        vector_rotate_yxz(&src->angle, from, &dst->pos);
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
        vector_rotate_yxz(&src->angle, from, &a);
        vector_rotate_yxz(&src->angle, to, &b);
        src->arg.b.unk40++;
        t = (src->arg.b.unk40 << 12) / steps;
        dst->pos.vx = fixed_lerp_q12(a.vx, b.vx, t) + src->pos.vx;
        dst->pos.vy = fixed_lerp_q12(a.vy, b.vy, t) + src->pos.vy;
        dst->pos.vz = fixed_lerp_q12(a.vz, b.vz, t) + src->pos.vz;
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

/* Runs frames while the screen tint (mode) steps its level from phase to last. */
ADDRESS(0x80046c00, 0xb0)
void func_80046c00(int mode, s32 phase, s32 last, s32 step)
{
    VECTOR pos;
    SVECTOR angle;
    s32 end = last;
    s32 increment = step;
    s32 level = phase;

    for (;;) {
        s32 v = level * level >> 16;

        if (v >= 256) {
            v = 0xff;
        }
        func_8003d624(mode, v, v, v);
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        func_800422b8(&pos, &angle);
        if (level == end) {
            break;
        }
        level += increment;
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
        vector_displacement_to_pitch_yaw(pos.vx - to->vx, -(to->vy + 1600) + pos.vy, pos.vz - to->vz, &rot);
        d = (rot.vy - obj->angle.vy) & 0xfff;
        if (d >= 2048) {
            d = 4096 - d;
        }
        d = (d << 12) / rate;
        if (d > 4096) {
            d = 4096;
        }
        rot.vy = angle_lerp_shortest_q12(rot.vy, obj->angle.vy, d);
        if (pitch == -1) {
            d = (rot.vx - obj->angle.vx) & 0xfff;
            if (d >= 2048) {
                d = 4096 - d;
            }
            rot.vx = angle_lerp_shortest_q12(rot.vx, obj->angle.vx, d);
        } else {
            rot.vx = pitch;
        }
        pitch_yaw_to_forward_vector(&rot, out);
        vector3s_scale_shift12(dist, out);
        if (--count == 0) {
            break;
        }
        vector_add_scaled_delta(from, &DAT_801b24e4.unk11c,
                                fixed_vector3_length(to->vx - pos.vx, to->vy - pos.vy,
                                                     to->vz - pos.vz) / dist, &pos);
    }
    return rot.vy;
}

/* Frame-time reference yaws of the door kinds 0x1b, 0 and 1 while they swing (func_80047010). */
DATA(0x8009c3d4, 0x2, ".sbss")
static s16 DAT_8009c3d4;
DATA(0x8009c3d8, 0x2, ".sbss")
static s16 DAT_8009c3d8;
DATA(0x8009c3dc, 0x2, ".sbss")
static s16 DAT_8009c3dc;

/* Moves an Object44 forward along its yaw by DIST (vector2i_scale_shift11 scale). */
#define OBJ44_ADVANCE(obj, dist)                            \
    {                                                       \
        s32 dir[2];                                         \
                                                            \
        angle_to_forward_xz((obj)->angle.vy + 2048, dir);         \
        vector2i_scale_shift11(dist, dir);                           \
        (obj)->pos.vx += dir[0];                            \
        (obj)->pos.vz += dir[1];                            \
    }

/*
 * Steps every live Object44 record by its kind (doors, lifts, levers, switches,
 * warps, falling items); kinds without a case go to FDAT handler 9 with the
 * record and its def in DAT_8018fb3c.unk8858 / unk8854.
 */
ADDRESS(0x80047010, 0x4514)
void func_80047010(void)
{
    Object44 *obj;
    Object44Def *def;
    s32 i;

    obj = DAT_8018fb3c.objects;
    i = 0x18c;
    do {
        if (obj->unk4 == 0xff) {
            continue;
        }
        DAT_8018fb3c.unk8858 = obj;
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_8018fb3c.unk8854 = def;
        switch (obj->unk4) {
        case 3:
            switch (obj->unk8) {
            case 0:
                break;
            case 1:
                if (obj->unka == 0) {
                    func_8004482c(obj, def->arg.b.unkf);
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828fc, 1,
                                  0xff);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                obj->unka += 72;
                if (obj->unka == 0xc18) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828dc, 1,
                                  0xff);
                }
                if (obj->unka >= 0x1000) {
                    obj->unka = 0xfff;
                    obj->unk8 = 2;
                }
                break;
            case 20:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x700, 0xc80, 0xc0) == 0) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828dc, 0,
                                  0xff);
                    obj->unk8 = 21;
                    func_8004482c(obj, def->arg.b.unkf);
                }
                break;
            case 21:
                obj->unka -= 72;
                if ((s16)obj->unka <= 0) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828fc, 0,
                                  0xff);
                    obj->unka = 0;
                    obj->unk8 = 0;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            default:
                obj->unk8++;
                break;
            }
            break;
        case 4:
            switch (obj->unk8) {
            case 0:
                break;
            case 1:
                if (obj->unka == 0) {
                    func_8004482c(obj, def->arg.b.unkf);
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008291c, 1,
                                  0xff);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                obj->unka += 72;
                if (obj->unka == 0xc18) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828dc, 1,
                                  0xff);
                }
                if (obj->unka >= 0x1000) {
                    obj->unka = 0xfff;
                    obj->unk8 = 2;
                }
                break;
            case 20:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x700, 0xc80, 0xc0) == 0) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828dc, 0,
                                  0xff);
                    obj->unk8 = 21;
                    func_8004482c(obj, def->arg.b.unkf);
                }
                break;
            case 21:
                obj->unka -= 72;
                if ((s16)obj->unka <= 0) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008291c, 0,
                                  0xff);
                    obj->unka = 0;
                    obj->unk8 = 0;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            default:
                obj->unk8++;
                break;
            }
            break;
        case 5:
            switch (obj->unk8) {
            case 0:
                break;
            case 1:
                if (obj->unka == 0) {
                    func_8004482c(obj, def->arg.b.unkf);
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008295c, 1,
                                  0xff);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                obj->unka += 72;
                if (obj->unka == 0xc18) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008293c, 1,
                                  0xff);
                }
                if (obj->unka >= 0x1000) {
                    obj->unka = 0xfff;
                    obj->unk8 = 2;
                }
                break;
            case 20:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x700, 0xc80, 0xc0) == 0) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008293c, 0,
                                  0xff);
                    obj->unk8 = 21;
                    func_8004482c(obj, def->arg.b.unkf);
                }
                break;
            case 21:
                obj->unka -= 72;
                if ((s16)obj->unka <= 0) {
                    func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008295c, 0,
                                  0xff);
                    obj->unka = 0;
                    obj->unk8 = 0;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            default:
                obj->unk8++;
                break;
            }
            break;
        case 2:
            switch (obj->unk8) {
            case 0:
                obj->unk3 |= 2;
                if (obj->arg.b.unk38 >= 150 && obj->arg.b.unk38 <= 198 && (obj->arg.b.unk38 & 1)) {
                    obj->unk8 = 1;
                }
                break;
            case 1:
                if (obj->unka == 0) {
                    obj->unk3 &= ~2;
                    func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd, obj->arg.b.unk3c,
                                  obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                  def->arg.b.unke, obj->angle.vy, 45);
                    func_8004482c(obj, def->arg.b.unkf);
                }
                obj->unka += 72;
                if (obj->unka == 0xc18) {
                    func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                  obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                  def->arg.b.unke, obj->angle.vy, 45);
                }
                if (obj->unka >= 0x1000) {
                    obj->unka = 0xfff;
                    obj->unk8 = 2;
                }
                break;
            case 20:
                if (obj->arg.b.unk38 >= 150 && obj->arg.b.unk38 <= 198 && (obj->arg.b.unk38 & 1)) {
                    break;
                }
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x1130, 0xc80, 0xc0) == 0) {
                    obj->unk8 = 21;
                    func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd, obj->arg.b.unk3c,
                                  obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                  def->arg.b.unke, obj->angle.vy, 45);
                    func_8004482c(obj, def->arg.b.unkf);
                }
                break;
            case 21:
                obj->unka -= 72;
                if ((s16)obj->unka <= 0) {
                    obj->unka = 0;
                    obj->unk8 = 0;
                    func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd * 2,
                                  obj->arg.b.unk3c, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                  def->arg.b.unkd, def->arg.b.unke, obj->angle.vy, 45);
                }
                break;
            default:
                obj->unk8++;
                break;
            }
            break;
        case 27:
            {
                Object44 *other;
                Object44Def *odef;
                s32 t;
                u32 turn;

                if (obj->arg.h.unk3e != 0xffff) {
                    other = &DAT_8018fb3c.objects[obj->arg.h.unk3e];
                    odef = &DAT_8018fb3c.defs[other->unk6];
                }
                if (obj->unk8 == 0) {
                    break;
                }
                if (obj->unk3 & 0x80) {
                    turn = (vector_xz_to_angle(DAT_801b24e4.pos.vx - obj->pos.vx,
                                               DAT_801b24e4.pos.vz - obj->pos.vz) -
                            obj->arg.h.unk42) & 0xfff;
                    if (turn <= 2048) {
                        obj->unke = -48;
                    } else {
                        obj->unke = 0xf0;
                    }
                }
                if (obj->arg.h.unk3e != 0xffff) {
                    other = &DAT_8018fb3c.objects[obj->arg.h.unk3e];
                    if (other->unk3 & 0x80) {
                        turn = (vector_xz_to_angle(DAT_801b24e4.pos.vx - other->pos.vx,
                                                   DAT_801b24e4.pos.vz - other->pos.vz) -
                            obj->arg.h.unk42) & 0xfff;
                    if (turn <= 2048) {
                            other->unke = 0xf0;
                        } else {
                            other->unke = -48;
                        }
                    }
                } else {
                    other = NULL;
                }
                if (obj->unk8 == 1) {
                    obj->unk8 = 2;
                    obj->arg.h.unk40 = 0;
                    DAT_8009c3d4 = DAT_80199154.unk15b08.vy;
                    func_8004482c(obj, def->arg.b.unkf);
                }
                t = obj->arg.sh.unk40++;
                if (t < 32) {
                    if (t < 21 && angle_within_tolerance(DAT_8009c3d4 + 2048, obj->arg.h.unk42, 900)) {
                        if ((other != NULL &&
                             angle_within_tolerance(DAT_8009c3d4,
                                                    vector_xz_to_angle(other->pos.vx - DAT_801b24e4.pos.vx,
                                                                       other->pos.vz - DAT_801b24e4.pos.vz),
                                                    0x400)) ||
                            angle_within_tolerance(DAT_801b24e4.unk124.vy,
                                                   vector_xz_to_angle(obj->pos.vx - DAT_801b24e4.pos.vx,
                                                                      obj->pos.vz - DAT_801b24e4.pos.vz),
                                                   0x400)) {
                            {
                            MATRIX m;
                            SVECTOR v;
                            VECTOR out;

                            v.vx = 0;
                            v.vy = 0;
                            v.vz = -76;
                            matrix_set_rotation_y(obj->arg.h.unk42 + 2048, &m);
                            ApplyMatrix(&m, &v, &out);
                            DAT_801b24e4.unk162 = 0;
                            DAT_801b24e4.unk164 = -76;
                            DAT_801b24e4.pos.vx += out.vx;
                            DAT_801b24e4.pos.vz += out.vz;
                            }
                        }
                    }
                    obj->unk1 = 0x81;
                    obj->angle.vy -= 32;
                    if (other != NULL) {
                        other->angle.vy += 32;
                        other->unk1 = 0x81;
                    }
                    if (t == 24) {
                        func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                      obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2,
                                      obj->arg.sh.unk42, 45);
                    } else if (t == 31) {
                        obj->unk1 = 0x82;
                        obj->arg.h.unk40 = 280;
                        if (other != NULL) {
                            other->unk1 = 0x82;
                        }
                    }
                } else if (t >= 300) {
                    if (t < 332) {
                        if (t == 300) {
                            SVECTOR v;
                            VECTOR out;
                            MATRIX m;

                            v.vx = -1792;
                            v.vy = 0;
                            v.vz = 0;
                            matrix_set_rotation_y(obj->arg.h.unk42, &m);
                            ApplyMatrix(&m, &v, &out);
                            out.vx += obj->pos.vx;
                            out.vz += obj->pos.vz;
                            if (collision_query_world(out.vx, obj->pos.vy, out.vz, 3000, obj->unkc,
                                                      0xc0) != 0) {
                                obj->arg.h.unk40 = 300;
                                break;
                            }
                            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + 3, obj->arg.b.unk3c,
                                          obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2,
                                          obj->arg.sh.unk42, 45);
                            func_8004482c(obj, def->arg.b.unkf);
                        }
                        obj->unk1 = 0x81;
                        obj->angle.vy += 32;
                        if (other != NULL) {
                            other->angle.vy -= 32;
                            other->unk1 = 0x81;
                        }
                    } else {
                        obj->unk8 = 0;
                        obj->unk1 = 0x80;
                        obj->unke = -50;
                        if (other != NULL) {
                            odef->unk0 = 0x24;
                            other->unk1 = 0x80;
                            other->unke = -50;
                        }
                    }
                }
            }
            break;
        case 0:
            {
                Object44 *other;
                Object44Def *odef;
                s32 t;
                u32 turn;

                if (obj->arg.h.unk3e != 0xffff) {
                    other = &DAT_8018fb3c.objects[obj->arg.h.unk3e];
                    odef = &DAT_8018fb3c.defs[other->unk6];
                }
                if (obj->unk8 == 0) {
                    break;
                }
                if (obj->unk3 & 0x80) {
                    turn = (vector_xz_to_angle(DAT_801b24e4.pos.vx - obj->pos.vx,
                                               DAT_801b24e4.pos.vz - obj->pos.vz) -
                            obj->arg.h.unk42) & 0xfff;
                    if (turn <= 2048) {
                        obj->unke = -48;
                    } else {
                        obj->unke = 0xf0;
                    }
                }
                if (obj->arg.h.unk3e != 0xffff) {
                    other = &DAT_8018fb3c.objects[obj->arg.h.unk3e];
                    if (other->unk3 & 0x80) {
                        turn = (vector_xz_to_angle(DAT_801b24e4.pos.vx - other->pos.vx,
                                                   DAT_801b24e4.pos.vz - other->pos.vz) -
                            obj->arg.h.unk42) & 0xfff;
                    if (turn <= 2048) {
                            other->unke = 0xf0;
                        } else {
                            other->unke = -48;
                        }
                    }
                } else {
                    other = NULL;
                }
                if (obj->unk8 == 1) {
                    obj->unk8 = 2;
                    obj->arg.h.unk40 = 0;
                    DAT_8009c3d8 = DAT_80199154.unk15b08.vy;
                    func_8004482c(obj, def->arg.b.unkf);
                }
                t = obj->arg.sh.unk40++;
                if (t < 32) {
                    if (t < 21 && angle_within_tolerance(DAT_8009c3d8 + 2048, obj->arg.h.unk42, 900)) {
                        if ((other != NULL &&
                             angle_within_tolerance(DAT_8009c3d8,
                                                    vector_xz_to_angle(other->pos.vx - DAT_801b24e4.pos.vx,
                                                                       other->pos.vz - DAT_801b24e4.pos.vz),
                                                    0x400)) ||
                            angle_within_tolerance(DAT_801b24e4.unk124.vy,
                                                   vector_xz_to_angle(obj->pos.vx - DAT_801b24e4.pos.vx,
                                                                      obj->pos.vz - DAT_801b24e4.pos.vz),
                                                   0x400)) {
                            {
                            MATRIX m;
                            SVECTOR v;
                            VECTOR out;

                            v.vx = 0;
                            v.vy = 0;
                            v.vz = -76;
                            matrix_set_rotation_y(obj->arg.h.unk42 + 2048, &m);
                            ApplyMatrix(&m, &v, &out);
                            DAT_801b24e4.unk162 = 0;
                            DAT_801b24e4.unk164 = -76;
                            DAT_801b24e4.pos.vx += out.vx;
                            DAT_801b24e4.pos.vz += out.vz;
                            }
                        }
                    }
                    obj->unk1 = 0x81;
                    obj->angle.vy += 32;
                    if (other != NULL) {
                        other->angle.vy -= 32;
                        other->unk1 = 0x81;
                    }
                    if (t == 24) {
                        func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                      obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2,
                                      obj->arg.sh.unk42, 45);
                    } else if (t == 31) {
                        obj->unk1 = 0x82;
                        obj->arg.h.unk40 = 280;
                        if (other != NULL) {
                            other->unk1 = 0x82;
                        }
                    }
                } else if (t >= 300) {
                    if (t < 332) {
                        if (t == 300) {
                            SVECTOR v;
                            VECTOR out;
                            MATRIX m;

                            v.vx = 0x700;
                            v.vy = 0;
                            v.vz = 0;
                            matrix_set_rotation_y(obj->arg.h.unk42, &m);
                            ApplyMatrix(&m, &v, &out);
                            out.vx += obj->pos.vx;
                            out.vz += obj->pos.vz;
                            if (collision_query_world(out.vx, obj->pos.vy, out.vz, 3000, obj->unkc,
                                                      0xc0) != 0) {
                                obj->arg.h.unk40 = 300;
                                break;
                            }
                            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + 3, obj->arg.b.unk3c,
                                          obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2,
                                          obj->arg.sh.unk42, 45);
                            func_8004482c(obj, def->arg.b.unkf);
                        }
                        obj->unk1 = 0x81;
                        obj->angle.vy -= 32;
                        if (other != NULL) {
                            other->angle.vy += 32;
                            other->unk1 = 0x81;
                        }
                    } else {
                        obj->unk8 = 0;
                        obj->unk1 = 0x80;
                        obj->unke = -50;
                        if (other != NULL) {
                            odef->unk0 = 0x24;
                            other->unk1 = 0x80;
                            other->unke = -50;
                        }
                    }
                }
            }
            break;
        case 1:
            {
                s32 t;
                u32 turn;

                if (obj->unk8 == 0) {
                    break;
                }
                if (obj->unk3 & 0x80) {
                    turn = (vector_xz_to_angle(DAT_801b24e4.pos.vx - obj->pos.vx,
                                               DAT_801b24e4.pos.vz - obj->pos.vz) -
                            obj->arg.h.unk42) & 0xfff;
                    if (turn <= 2048) {
                        obj->unke = -48;
                    } else {
                        obj->unke = 0xf0;
                    }
                }
                if (obj->unk8 == 1) {
                    obj->unk8 = 2;
                    obj->arg.h.unk40 = 0;
                    DAT_8009c3dc = DAT_80199154.unk15b08.vy;
                    func_8004482c(obj, def->arg.b.unkf);
                }
                t = obj->arg.sh.unk40++;
                if (t < 32) {
                    if (t < 21 && angle_within_tolerance(DAT_8009c3dc + 2048, obj->arg.h.unk42, 900) &&
                        angle_within_tolerance(DAT_8009c3dc,
                                               vector_xz_to_angle(obj->pos.vx - DAT_801b24e4.pos.vx,
                                                                  obj->pos.vz - DAT_801b24e4.pos.vz),
                                               0x400)) {
                        {
                            MATRIX m;
                            SVECTOR v;
                            VECTOR out;

                            v.vx = 0;
                            v.vy = 0;
                            v.vz = -76;
                            matrix_set_rotation_y(obj->arg.h.unk42 + 2048, &m);
                            ApplyMatrix(&m, &v, &out);
                            DAT_801b24e4.unk162 = 0;
                            DAT_801b24e4.unk164 = -76;
                            DAT_801b24e4.pos.vx += out.vx;
                            DAT_801b24e4.pos.vz += out.vz;
                            }
                    }
                    obj->unk1 = 0x81;
                    obj->angle.vy += 32;
                    if (t == 24) {
                        func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                      obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                      def->arg.b.unke, obj->arg.sh.unk42, 45);
                    } else if (t == 31) {
                        obj->arg.h.unk40 = 280;
                    }
                } else if (t >= 300) {
                    if (t < 332) {
                        if (t == 300) {
                            SVECTOR v;
                            VECTOR out;
                            MATRIX m;

                            v.vx = 0x400;
                            v.vy = 0;
                            v.vz = 0;
                            matrix_set_rotation_y(obj->arg.h.unk42, &m);
                            ApplyMatrix(&m, &v, &out);
                            out.vx += obj->pos.vx;
                            out.vz += obj->pos.vz;
                            if (collision_query_world(out.vx, obj->pos.vy, out.vz, 1500, obj->unkc,
                                                      0xc0) != 0) {
                                obj->arg.h.unk40 = 300;
                                break;
                            }
                            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + 2, obj->arg.b.unk3c,
                                          obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                          def->arg.b.unke, obj->arg.sh.unk42, 45);
                            func_8004482c(obj, def->arg.b.unkf);
                        }
                        obj->angle.vy -= 32;
                    } else {
                        obj->unk1 = 0x80;
                        obj->unk8 = 0;
                        obj->unke = -50;
                    }
                }
            }
            break;
        case 23:
            {
                Object44 *o1;
                Object44 *o2;
                VECTOR a;
                VECTOR b;
                s32 f;
                s32 n;
                static SVECTOR start = {0, -512, 0, 0};
                static SVECTOR end = {0, 0, 0, 0};

                o1 = &DAT_8018fb3c.objects[obj->arg.h.unk3c];
                o2 = &DAT_8018fb3c.objects[obj->arg.h.unk3a];
                switch (obj->unk8) {
                case 0:
                    if (obj->arg.b.unk38 == 0xfe) {
                        obj->unk4 = 0xff;
                        obj->unk0 = 0;
                        obj->unk8 = 8;
                    } else {
                        func_80044900(obj->arg.h.unk3a, 0);
                        func_80044900(obj->arg.h.unk3a, 3, 0);
                        obj->unk8 = 1;
                    }
                    break;
                case 1:
                    if (obj->arg.b.unk38 != 0xfe) {
                        break;
                    }
                    func_80044900(obj->arg.h.unk3a, 1, obj->unk0);
                    obj->arg.b.unk40 = 0;
                    o1->pos.vy -= 30000;
                    o1->arg.b.unk38 = 0;
                    obj->unk8 = 2;
                    o1->unke = -180;
                    break;
                case 2:
                    vector_rotate_yxz(&obj->angle, &start, &a);
                    vector_rotate_yxz(&obj->angle, &end, &b);
                    obj->arg.b.unk40++;
                    n = 40;
                    f = (obj->arg.b.unk40 << 12) / n;
                    o1->pos.vx = fixed_lerp_q12(a.vx, b.vx, f) + obj->pos.vx;
                    o1->pos.vy = fixed_lerp_q12(a.vy, b.vy, f) + obj->pos.vy;
                    o1->pos.vz = fixed_lerp_q12(a.vz, b.vz, f) + obj->pos.vz;
                    o1->angle.vy += 256;
                    if (obj->arg.b.unk40 >= n) {
                        func_8004482c(obj, def->arg.b.unkf);
                        obj->arg.b.unk40 = 0;
                        obj->unk8 = 3;
                    }
                    break;
                case 3:
                    obj->arg.b.unk40++;
                    if (obj->arg.b.unk40 > 20) {
                        obj->unk8 = 4;
                    }
                    break;
                case 4:
                    obj->unk5 = 50;
                    obj->unk10 = 0x400;
                    obj->unk8 = 5;
                    obj->unk2 = (obj->unk2 & 0xf8) | 5;
                    o1->unk5 = 50;
                    o1->unk2 = (o1->unk2 & 0xf8) | 5;
                    break;
                case 5:
                    if (obj->unk10 < 0x1000) {
                        obj->unk10 += 192;
                        break;
                    }
                    o1->unk4 = 0xff;
                    o1->unk0 = 0;
                    o1->unk6 = 0xff;
                    obj->unk10 = 0x1000;
                    obj->unk0 = 0;
                    obj->arg.b.unk40 = 0;
                    obj->unk8 = 6;
                    break;
                case 6:
                    obj->arg.b.unk40++;
                    if (obj->arg.b.unk40 < 32) {
                        o2->pos.vy -= 48;
                        break;
                    }
                    o2->unk4 = 0x10;
                    o2->arg.w.unk40 = o2->pos.vy;
                    obj->unk8 = 7;
                    break;
                case 7:
                    func_80044900(obj->arg.h.unk3a, 2);
                    func_80044900(obj->arg.h.unk3a, 3, 0);
                    obj->unk4 = 0xff;
                    obj->unk8 = 8;
                    break;
                }
            }
            break;
        case 6:
            {
            Object4cTrail spark;

            switch (obj->unk8) {
            case 0:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->angle.vx = 0xa00;
                    obj->unk8 = 3;
                } else {
                    func_80044900(obj->arg.h.unk3a, 0);
                    func_80044900(obj->arg.h.unk3a, 3, 0);
                    obj->unk8 = 1;
                }
                break;
            case 1:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->arg.sh.unk40 = -16;
                    obj->unk8 = 2;
                    func_80044900(obj->arg.h.unk3a, 1, obj->unk0);
                    func_8004482c(obj, def->arg.b.unkf);
                }
                break;
            case 2:
                {
                    if (obj->arg.b.unk3c != 0xff) {
                        spark.pos.vx = obj->pos.vx + (rcos(obj->angle.vy - 1024) * -160 >> 12);
                        spark.pos.vy = obj->pos.vy + 320;
                        spark.pos.vz = obj->pos.vz - (rsin(obj->angle.vy - 1024) * -160 >> 12);
                        func_80046dcc(obj, &DAT_801b24e4.pos, 400, &spark.pos, &spark.angle, -1, 0x400, 1);
                        func_80053c84(10, 0x83, 0x23, &spark.pos, &spark.angle, &obj->angle);
                    }
                    obj->arg.sh.unk40 = angle_velocity_step(0xa00, obj->angle.vx, obj->arg.sh.unk40, 8, 4);
                    obj->angle.vx = (obj->angle.vx + obj->arg.sh.unk40) & 0xfff;
                    if (obj->angle.vx < 3072) {
                        func_80044900(obj->arg.h.unk3a, 2);
                    }
                    if (obj->arg.sh.unk40 == 0 && obj->angle.vx == 0xa00) {
                        obj->unk8 = 3;
                    }
                }
                break;
            }
            }
            break;
        case 7:
            switch (obj->unk8) {
            case 0:
                if (obj->arg.h.unk3a != 0xffff) {
                    Object44 *o;

                    o = &DAT_8018fb3c.objects[obj->arg.h.unk3a];
                    o->unke += 200;
                }
                if (obj->arg.b.unk38 == 0xfe) {
                    if (def->arg.b.unkc != 0xff) {
                        func_80044b40(obj, 1, def->arg.b.unkc);
                    }
                    obj->unka = 0xfff;
                    obj->unk8 = 3;
                } else {
                    func_80044900(obj->arg.h.unk3a, 0);
                    func_80044900(obj->arg.h.unk3a, 3, 0);
                    obj->unk8 = 1;
                }
                break;
            case 1:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->unk8 = 2;
                    func_80044900(obj->arg.h.unk3a, 1, obj->unk0);
                    func_8004482c(obj, def->arg.b.unkf);
                    if (def->arg.b.unkc != 0xff) {
                        func_80044b40(obj, 1, def->arg.b.unkc);
                    }
                }
                break;
            case 2:
                obj->unka += 128;
                if (obj->unka >= 4095) {
                    obj->unka = 0xfff;
                    func_80044900(obj->arg.h.unk3a, 2);
                    obj->unk8 = 3;
                }
                break;
            }
            break;
        case 8:
            switch (obj->unk8) {
            case 0:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->angle.vx = 0xd44;
                    OBJ44_ADVANCE(obj, 160);
                    obj->unk8 = 3;
                } else {
                    func_80044900(obj->arg.h.unk3a, 0);
                    func_80044900(obj->arg.h.unk3a, 3, 0);
                    obj->unk8 = 1;
                }
                break;
            case 1:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->arg.h.unk40 = 16;
                    obj->unk8 = 2;
                    func_80044900(obj->arg.h.unk3a, 1, obj->unk0);
                    func_8004482c(obj, def->arg.b.unkf);
                }
                break;
            case 2:
                {
                    OBJ44_ADVANCE(obj, 10);
                    if (--obj->arg.h.unk40 == 0) {
                        func_80044900(obj->arg.h.unk3a, 2);
                        obj->unk8 = 3;
                    }
                }
                break;
            case 3:
                obj->angle.vx = angle_approach(obj->angle.vx, 0xd44, 50);
                if (obj->angle.vx == 0xd44) {
                    obj->unk8 = 4;
                }
                break;
            }
            break;
        case 9:
            if (obj->arg.h.unk3a != 0xffff) {
                Object44 *o;

                o = &DAT_8018fb3c.objects[obj->arg.h.unk3a];
                if (o->unk6 != 0xff) {
                    o->unk0 = 0;
                    o->arg.b.unk38 = 0;
                }
            }
            obj->unk4 = 0xff;
            break;
        case 83:
            {
                s32 step;
                s32 se;

                step = 0x100;
                if (DAT_8018fad4.area[0] == 13) {
                    se = def->arg.b.unkf + 496;
                } else {
                    se = def->arg.b.unkf;
                }
                if (obj->arg.h.unk3a != 0xffff) {
                    step = obj->arg.h.unk3a;
                }
                switch (obj->unk8) {
                case 1:
                    if (se != 15) {
                        func_8004482c(obj, se);
                    }
                    if (obj->arg.b.unk3c == 1) {
                        func_800463bc(obj->arg.b.unk39);
                    }
                    switch (obj->arg.b.unk38) {
                    case 0:
                    case 1:
                        obj->unk8 = 2;
                        break;
                    case 2:
                        obj->unk8 = 2;
                        obj->arg.b.unk38 = 3;
                        break;
                    case 3:
                        obj->unk8 = 3;
                        obj->arg.b.unk38 = 2;
                        break;
                    }
                    break;
                case 2:
                    obj->unka += step;
                    if (se == 15 && obj->unka > 1024 && obj->unka <= step + 1024) {
                        func_8004482c(obj, 15);
                    }
                    if (obj->unka < 0x1000) {
                        break;
                    }
                    obj->unka = 0xfff;
                arrived:
                    if (obj->arg.b.unk3c != 1) {
                        func_800463bc(obj->arg.b.unk39);
                    }
                    switch (obj->arg.b.unk38) {
                    case 1:
                        func_8004482c(obj, se);
                        obj->unk8 = 4;
                        break;
                    case 0:
                        obj->unk8 = 99;
                        break;
                    case 2:
                    case 3:
                        obj->unk8 = 0;
                        break;
                    }
                    break;
                case 3:
                    if (se == 15 && obj->unka > 3072 && obj->unka <= step + 3072) {
                        func_8004482c(obj, 15);
                    }
                    obj->unka -= step;
                    if ((s16)obj->unka <= 0) {
                        obj->unka = 0;
                        goto arrived;
                    }
                    break;
                case 4:
                    obj->unka -= step;
                    if ((s16)obj->unka <= 0) {
                        obj->unk8 = 99;
                    }
                    break;
                case 9:
                    if (obj->arg.b.unk38 == 3) {
                        obj->unka = 0xfff;
                    }
                    obj->unk8 = 0;
                    break;
                }
            }
            break;
        case 84:
            switch (obj->unk8) {
            case 0:
                if (func_80046884(obj->arg.b.unk39 - ((obj->arg.b.unk3a.bytes[1] - 1) >> 1),
                                  obj->arg.b.unk3a.bytes[0] - ((obj->arg.b.unk3c - 1) >> 1),
                                  obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                  -obj->arg.b.unk3d[0] << 7) != 0 ||
                    obj->arg.b.unk38 == 0xff) {
                    func_800443c8(obj->arg.b.unk40, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                  DAT_8008288c[def->arg.b.unke * 2 + (obj->arg.b.unk3d[1] & 1)], 1, 0x80);
                    obj->unk8 = 1;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                break;
            case 1:
                obj->unka += 256;
                if (obj->unka < 1024) {
                    break;
                }
                if (obj->arg.b.unk3d[1] & 2) {
                    obj->unk8 = 99;
                    obj->arg.b.unk38 = 0xff;
                } else {
                    obj->unk8 = 2;
                }
                break;
            case 32:
                obj->unka += 128;
                if (obj->unka >= 4095) {
                    obj->unk1 = 0;
                    obj->unka = 0;
                    obj->unk8 = 0;
                    func_800443c8(obj->arg.b.unk40, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                  DAT_8008288c[def->arg.b.unke * 2 + (obj->arg.b.unk3d[1] & 1)], 0, 0);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            case 99:
                break;
            default:
                obj->unk8++;
                break;
            }
            break;
        case 95:
            switch (obj->unk8) {
            case 1:
                switch (obj->arg.b.unk3f) {
                case 0:
                    obj->unk8 = 2;
                    break;
                case 1:
                    obj->unk8 = 3;
                    break;
                }
                func_800445b8(2, obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0], obj->arg.b.unk3c,
                              obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0],
                              obj->arg.b.unk3d[1], obj->angle.vy, 45);
                break;
            case 2:
                obj->unk8 = 99;
                func_800445b8(2, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c, obj->arg.b.unk39,
                              obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0], obj->arg.b.unk3d[1],
                              obj->angle.vy, 45);
                break;
            case 3:
                obj->unk8 = 99;
                break;
            }
            break;
        case 81:
            {
                s32 speed;
                s32 hit;
                VECTOR p;
                VECTOR q;
                VECTOR r;

                speed = obj->arg.b.unk3a.bytes[0] << 2;
                if (obj->unk6 == 222) {
                    speed = obj->arg.b.unk3a.bytes[0] << 4;
                }
                switch (obj->unk8) {
                case 0:
                    if (obj->arg.b.unk38 == 0) {
                        obj->unk8 = 2;
                        obj->unk1 = 1;
                        obj->unka = 0;
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xfe) {
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xff ||
                        func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk3a.bytes[1],
                                      obj->arg.b.unk3c, obj->pos.vy) != 0) {
                        obj->unk8 = 1;
                        obj->unk1 = 0;
                        obj->unka = 0;
                        obj->arg.b.unk40 = 0;
                        if (def->arg.b.unk16 != 0xff) {
                            func_8004482c(obj, def->arg.b.unk16 + 496);
                        } else {
                            func_8004482c(obj, 30);
                        }
                    }
                    break;
                case 1:
                    if (obj->arg.b.unk3a.bytes[1] != 0xff && obj->unka == 0) {
                        func_80044b40(obj, 1, def->arg.b.unk17);
                        func_8004482c(obj, 30);
                    }
                    if (obj->arg.b.unk38 == 0 && obj->unka >= 0x1000 - speed) {
                        obj->unk8 = 2;
                        obj->unk1 = 1;
                        obj->unka = 0;
                        break;
                    }
                    obj->unka += speed;
                    if (obj->unka >= 4095) {
                        if (obj->arg.b.unk3a.bytes[1] == 0xff) {
                            if (def->arg.b.unk16 != 0xff) {
                                func_8004482c(obj, def->arg.b.unk16 + 496);
                            } else {
                                func_8004482c(obj, 30);
                            }
                            obj->unka &= 0xfff;
                        } else {
                            obj->unka = 0;
                            obj->unk8 = 0;
                            func_80044b40(obj, 0, def->arg.b.unk17);
                            break;
                        }
                    }
                    if (obj->unka >= 2048 && obj->unka < speed + 2048 && obj->arg.b.unk3a.bytes[1] == 0xff &&
                        def->arg.b.unk16 != 0xff) {
                        func_8004482c(obj, def->arg.b.unk16 + 496);
                    }
                    func_80046760(obj, def->arg.b.unkc, &p);
                    hit = collision_query_world(p.vx, p.vy, p.vz, def->arg.h.unke, def->arg.h.unk10, 0x90);
                    if (hit == 0) {
                        func_80046760(obj, def->arg.b.unkd, &p);
                        hit = collision_query_world(p.vx, p.vy, p.vz, def->arg.h.unke, def->arg.h.unk10,
                                                    0x90);
                    }
                    if (hit != 0) {
                        if (obj->arg.b.unk40 == 0) {
                            obj->arg.b.unk40 = 1;
                            func_800533e8(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39, def->arg.b.unk12[0],
                                          def->arg.b.unk12[1], def->arg.b.unk12[2], def->arg.b.unk12[3], 0, 0,
                                          0, 0, 0, &p);
                        }
                    } else {
                        obj->arg.b.unk40 = 0;
                    }
                    break;
                case 2:
                    {
                        obj->unka += 64;
                        if (obj->unka >= 4095) {
                            obj->unka = 0xfff;
                            if (obj->arg.b.unk38 == 0xff) {
                                obj->unk8 = 3;
                                obj->unk1 = 1;
                                obj->unka = 0xfff;
                            }
                        }
                        func_80046760(obj, def->arg.b.unkc, &q);
                        hit = collision_query_world(q.vx, q.vy, q.vz, def->arg.h.unke, def->arg.h.unk10,
                                                    0x90);
                        if (hit == 0) {
                            func_80046760(obj, def->arg.b.unkd, &q);
                            hit = collision_query_world(q.vx, q.vy, q.vz, def->arg.h.unke, def->arg.h.unk10,
                                                        0x90);
                        }
                        if (hit != 0) {
                            if (obj->arg.b.unk40 == 0) {
                                obj->arg.b.unk40 = 1;
                                func_800533e8(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39, def->arg.b.unk12[0],
                                              def->arg.b.unk12[1], def->arg.b.unk12[2], def->arg.b.unk12[3],
                                              0,
                                              0, 0, 0, 0, &q);
                            }
                        } else {
                            obj->arg.b.unk40 = 0;
                        }
                    }
                    break;
                case 3:
                    {
                        obj->unka -= 64;
                        if (obj->unka >= 0x8000) {
                            obj->unk8 = 0;
                            obj->unka = 0;
                        }
                        func_80046760(obj, def->arg.b.unkc, &r);
                        hit = collision_query_world(r.vx, r.vy, r.vz, def->arg.h.unke, def->arg.h.unk10,
                                                    0x90);
                        if (hit == 0) {
                            func_80046760(obj, def->arg.b.unkd, &r);
                            hit = collision_query_world(r.vx, r.vy, r.vz, def->arg.h.unke, def->arg.h.unk10,
                                                        0x90);
                        }
                        if (hit != 0) {
                            if (obj->arg.b.unk40 == 0) {
                                obj->arg.b.unk40 = 1;
                                func_800533e8(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39, def->arg.b.unk12[0],
                                              def->arg.b.unk12[1], def->arg.b.unk12[2], def->arg.b.unk12[3],
                                              0,
                                              0, 0, 0, 0, &r);
                            }
                        } else {
                            obj->arg.b.unk40 = 0;
                        }
                    }
                    break;
                }
            }
            break;
        case 87:
            switch (obj->unk8) {
            case 1:
                func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c, obj->arg.b.unk39,
                              obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0], obj->arg.b.unk3d[1],
                              obj->angle.vy, 45);
                obj->unk8 = 2;
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                func_8004482c(obj, def->arg.b.unkf);
                break;
            case 2:
                obj->unka += 256;
                if (obj->unka >= 2048) {
                    obj->unk8 = 3;
                }
                break;
            case 3:
                obj->unka += 64;
                if (obj->unka >= 4095) {
                    obj->unka = 0;
                    obj->unk8 = 0;
                    func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0],
                                  obj->arg.b.unk3c,
                                  obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0],
                                  obj->arg.b.unk3d[1], obj->angle.vy, 45);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            case 0:
                break;
            }
            break;
        case 90:
            switch (obj->unk8) {
            case 0:
                if (obj->arg.b.unk38 == 0) {
                    obj->unk8 = 1;
                    obj->unka = 0;
                } else {
                    s32 w;
                    s32 h;

                    switch (obj->angle.vy) {
                    case 0:
                    case 0x800:
                        w = obj->arg.b.unk3d[0];
                        h = obj->arg.b.unk3d[1];
                        break;
                    case 0x400:
                    case 0xc00:
                        w = obj->arg.b.unk3d[1];
                        h = obj->arg.b.unk3d[0];
                        break;
                    }
                    if (func_80046884(obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], w, h, obj->pos.vy) != 0) {
                        obj->arg.b.unk38 = 0;
                        obj->unk8 = 1;
                        obj->unka = 0;
                        func_8004482c(obj, def->arg.b.unkf);
                    }
                }
                break;
            case 1:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x1130, 0xc80, 0xc0) == 0) {
                    obj->unk8 = 2;
                    func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c, obj->arg.b.unk39,
                                  obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0], obj->arg.b.unk3d[1],
                                  obj->angle.vy, 45);
                    func_8004482c(obj, def->arg.b.unkf);
                }
                break;
            case 2:
                if (obj->unka < 2048) {
                    obj->unka += 256;
                } else if (obj->unka < 4095) {
                    obj->unka += 64;
                } else {
                    obj->unka = 0xfff;
                    obj->unk8 = 9;
                }
                break;
            }
            break;
        case 82:
            {
                s32 speed;
                s32 hit;

                speed = obj->arg.b.unk3a.bytes[0] << 2;
                switch (obj->unk8) {
                case 0:
                    if (obj->arg.b.unk38 == 0) {
                        obj->unk8 = 2;
                        obj->unk1 = 1;
                        obj->unka = 0;
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xfe) {
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xff ||
                        func_80046884(obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c, obj->arg.b.unk3d[0],
                                      obj->arg.b.unk3d[1], obj->pos.vy) != 0) {
                        obj->unk8 = 1;
                        obj->unk1 = 0;
                        obj->unka = 0;
                        obj->arg.b.unk40 = 0;
                        func_8004482c(obj, def->arg.b.unk16);
                    }
                    break;
                case 1:
                    {
                    VECTOR p;

                    if (obj->arg.b.unk3d[0] != 0xff) {
                        if (obj->unka == 0) {
                            func_80044b40(obj, 1, def->arg.b.unk17);
                            func_8004482c(obj, def->arg.b.unk16);
                        } else if (obj->unka >= 1500 && obj->unka < speed + 1500) {
                            func_8004482c(obj, 30);
                        }
                    }
                    if (obj->arg.b.unk38 == 0 && obj->unka >= 0x1000 - speed) {
                        obj->unk8 = 2;
                        obj->unk1 = 1;
                        obj->unka = 0;
                        break;
                    }
                    obj->unka += speed;
                    if (obj->unka >= 4095) {
                        if (obj->arg.b.unk3d[0] != 0xff) {
                            obj->unka = 0;
                            obj->unk8 = 0;
                            func_80044b40(obj, 0, def->arg.b.unk17);
                            break;
                        }
                        func_8004482c(obj, def->arg.b.unk16);
                        obj->unka &= 0xfff;
                    }
                    func_80046760(obj, def->arg.h.unkc, &p);
                    hit = collision_query_world(p.vx, p.vy, p.vz, def->arg.h.unke, def->arg.h.unk10, 0x90);
                    if (hit != 0) {
                        if (obj->arg.b.unk40 == 0) {
                            obj->arg.b.unk40 = 1;
                            func_800533e8(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39, def->arg.b.unk12[0],
                                          def->arg.b.unk12[1], def->arg.b.unk12[2], def->arg.b.unk12[3], 0, 0,
                                          0, 0, 0, &p);
                        }
                    } else {
                        obj->arg.b.unk40 = 0;
                    }
                    }
                    break;
                case 2:
                    obj->unka += 64;
                    if (obj->unka >= 4095) {
                        obj->unka = 0xfff;
                        if (obj->arg.b.unk38 == 0xff) {
                            obj->unk8 = 3;
                            obj->unk1 = 1;
                            obj->unka = 0xfff;
                        }
                    }
                    break;
                case 3:
                    obj->unka -= 64;
                    if (obj->unka >= 0x8000) {
                        obj->unk8 = 0;
                        obj->unka = 0;
                    }
                    break;
                }
            }
            break;
        case 85:
            {
                s32 speed;
                s32 hit;
                s32 step;

                speed = obj->arg.b.unk3a.bytes[0] << 3;
                switch (obj->unk8) {
                case 0:
                    if (obj->arg.b.unk38 == 0) {
                        obj->unk8 = 2;
                        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                        obj->unka = 0;
                        break;
                    }
                    obj->unk8 = 1;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                    obj->unka = 0;
                    obj->arg.b.unk40 = 0;
                    func_8004482c(obj, def->arg.b.unk16 + 496);
                    break;
                case 1:
                    {
                    VECTOR p;
                    VECTOR unused; /* never referenced; retail's frame reserves it */

                    if (obj->arg.b.unk38 == 0 && obj->unka >= 0x1000 - speed) {
                        obj->unk8 = 2;
                        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                        obj->unka = 0;
                        break;
                    }
                    obj->unka += speed;
                    if (obj->unka >= 4095) {
                        func_8004482c(obj, def->arg.b.unk16 + 496);
                        obj->unka &= 0xfff;
                    }
                    func_80046760(obj, def->arg.h.unkc, &p);
                    hit = collision_query_world(p.vx, p.vy + 2048, p.vz, def->arg.h.unke, def->arg.h.unk10,
                                                0x90);
                    if (hit != 0) {
                        if (obj->arg.b.unk40 == 0) {
                            func_800533e8(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39, def->arg.b.unk12[0],
                                          def->arg.b.unk12[1], def->arg.b.unk12[2], def->arg.b.unk12[3], 0, 0,
                                          0, 0, 0, &p);
                        }
                    } else {
                        obj->arg.b.unk40 = 0;
                    }
                    if (!(obj->arg.b.unk41[1] & 1)) {
                        step = def->arg.b.unk17 << 4;
                        if (obj->arg.b.unk41[2] != 0) {
                            if (obj->arg.b.unk3a.bytes[1] != obj->pos.vx >> 11 ||
                                obj->arg.b.unk3c != obj->pos.vz >> 11) {
                                obj->arg.b.unk41[2] = 0;
                            } else {
                                step = (step * ((obj->pos.vx - ((obj->arg.b.unk3a.bytes[1] << 11) + 1024)) +
                                                (obj->pos.vz - ((obj->arg.b.unk3c << 11) + 1024))) >> 10) + 8;
                            }
                        } else if (obj->arg.b.unk3d[0] == obj->pos.vx >> 11 &&
                                   obj->arg.b.unk3d[1] == obj->pos.vz >> 11) {
                            s32 dx;
                            s32 dz;

                            dx = ((obj->arg.b.unk3d[0] << 11) + 1024) - obj->pos.vx;
                            dz = ((obj->arg.b.unk3d[1] << 11) + 1024) - obj->pos.vz;
                            if (dx <= 0 && dz <= 0) {
                                obj->arg.b.unk41[2] = 1;
                                if (!(obj->arg.b.unk41[1] & 1)) {
                                    obj->arg.b.unk41[1]++;
                                } else {
                                    obj->arg.b.unk41[1]--;
                                }
                            }
                            step = (step * (dx + dz) >> 10) + 4;
                        }
                    } else {
                        step = -(def->arg.b.unk17 << 4);
                        if (obj->arg.b.unk41[2] != 0) {
                            if (obj->arg.b.unk3d[0] != obj->pos.vx >> 11 ||
                                obj->arg.b.unk3d[1] != obj->pos.vz >> 11) {
                                obj->arg.b.unk41[2] = 0;
                            } else {
                                step = -((def->arg.b.unk17 << 4) *
                                         ((((obj->arg.b.unk3d[0] << 11) + 1024) - obj->pos.vx) +
                                          (((obj->arg.b.unk3d[1] << 11) + 1024) - obj->pos.vz)) >> 10) - 8;
                            }
                        } else if (obj->arg.b.unk3a.bytes[1] == obj->pos.vx >> 11 &&
                                   obj->arg.b.unk3c == obj->pos.vz >> 11) {
                            s32 dx;
                            s32 dz;

                            dx = obj->pos.vx - ((obj->arg.b.unk3a.bytes[1] << 11) + 1024);
                            dz = obj->pos.vz - ((obj->arg.b.unk3c << 11) + 1024);
                            if (dx <= 0 && dz <= 0) {
                                obj->arg.b.unk41[2] = 1;
                                if (!(obj->arg.b.unk41[1] & 1)) {
                                    obj->arg.b.unk41[1]++;
                                } else {
                                    obj->arg.b.unk41[1]--;
                                }
                            }
                            step = -(-step * (dx + dz) >> 10) - 4;
                        }
                    }
                    if (!(obj->arg.b.unk41[1] & 2)) {
                        obj->pos.vz += step;
                    } else {
                        obj->pos.vx += step;
                    }
                    }
                    break;
                case 2:
                    obj->unka += 64;
                    if (obj->unka >= 4095) {
                        obj->unka = 0xfff;
                        if (obj->arg.b.unk38 == 0xff) {
                            obj->unk8 = 3;
                            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                            obj->unka = 0xfff;
                        }
                    }
                    break;
                case 3:
                    obj->unka -= 64;
                    if (obj->unka >= 0x8000) {
                        obj->unk8 = 0;
                        obj->unka = 0;
                    }
                    break;
                }
            }
            break;
        case 86:
            switch (obj->unk8) {
            case 0:
                if (obj->arg.b.unk39 == 0) {
                    obj->unk8 = 2;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                    obj->arg.b.unk38 = 0;
                    obj->unk0 = 0;
                } else {
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                    obj->arg.b.unk38 = 0x80;
                    obj->unk8 = 1;
                    obj->unk0 = 2;
                }
                break;
            case 1:
                if (obj->arg.b.unk39 == 0) {
                    obj->unk8 = 2;
                }
                break;
            case 2:
                obj->scale.vx -= 128;
                obj->scale.vy -= 128;
                obj->scale.vz -= 128;
                if (obj->scale.vx <= 0) {
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                    obj->arg.b.unk38 = 0;
                    obj->unk0 = 0;
                    if (obj->arg.b.unk39 == 0xff) {
                        obj->unk8 = 3;
                    }
                }
                break;
            case 3:
                obj->unk0 = 2;
                obj->scale.vx += 256;
                obj->scale.vy += 256;
                obj->scale.vz += 256;
                if (obj->scale.vx >= 4096) {
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                    obj->unk8 = 0;
                    obj->arg.b.unk38 = 0x80;
                }
                break;
            case 9:
                break;
            }
            break;
        case 231:
            if (func_80046884(obj->arg.b.unk38, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                              obj->arg.b.unk3a.bytes[1], obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    CVECTOR c = {0x80, 0x80, 0x80, 0};
                    s32 n = 0x18c;
                    Object44 *o;

                    func_80046c00(0x82, 0, 0, 0);
                    func_80046c00(0x82, 0, 0x1000, 0x80);
                    o = DAT_8018fb3c.objects;
                    for (; n != 0; n--, o++) {
                        if (o->unk4 == 0xe7) {
                            o->arg.b.unk40 = 1;
                        }
                    }
                    map_cell_add_layer_occupancy(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vz, 800, -1);
                    DAT_801b24e4.pos.vx = (obj->arg.b.unk3c << 11) + 1024;
                    DAT_801b24e4.pos.vz = (obj->arg.b.unk3d[0] << 11) + 1024;
                    DAT_801b24e4.unk160 = obj->arg.b.unk3d[1] != 1 ? 5 : 0;
                    DAT_801b24e4.unk12c.vy = -(obj->arg.b.unk3f << 4);
                    player_sync_position_to_map();
                    DAT_801b24e4.unk124.vy = DAT_801b24e4.unk12c.vy + DAT_801b24e4.unk136 +
                                             DAT_801b24e4.unk13e + DAT_801b24e4.unk146;
                    func_80046c00(0x82, 0x1000, 0x1000, 0);
                    cd_request_wait_idle();
                    func_80046c00(0x82, 0x1000, 0, -128);
                    func_8003d624(0xff, 0, 0, 0);
                }
            } else {
                obj->arg.b.unk40 = 0;
            }
            obj->unka = (obj->unka + 64) & 0xfff;
            break;
        case 235:
            if (obj->arg.b.unk3f == 0) {
                break;
            }
            if (obj->arg.b.unk38 != 0xff) {
                if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, 1, 1, obj->pos.vy) != 0) {
                    if (obj->arg.b.unk40 == 0) {
                        Object44 *o;
                        s32 n = 0x18c;
                        u8 x;
                        u8 z;
                        u8 y;
                        u8 yaw;
                        u8 area;
                        u8 area3;
                        u8 area4;

                        o = DAT_8018fb3c.objects;
                        for (; n != 0; n--, o++) {
                            if (o->unk4 == 0xeb) {
                                o->arg.b.unk40 = 1;
                            }
                        }
                        x = obj->arg.b.unk3a.bytes[1];
                        z = obj->arg.b.unk3c;
                        y = obj->arg.b.unk3d[0];
                        yaw = obj->arg.b.unk3d[1];
                        func_80046c00(1, 0, 0x1000, 0x100);
                        func_8004b794();
                        area = obj->arg.b.unk38;
                        area3 = obj->arg.b.unk39;
                        area4 = obj->arg.b.unk3a.bytes[0];
                        do {
                            cd_request_yield();
                            resource_advance_transition();
                        } while ((s16)DAT_8018fad4.loading != 0);
                        if (area != DAT_8018fad4.area[0]) {
                            resource_request_transition(area, area, area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
                        } else {
                            resource_request_transition(0xff, 0xff, area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
                            DAT_801b24e4.unke = 1;
                        }
                        do {
                            cd_request_yield();
                            resource_advance_transition();
                        } while ((s16)DAT_8018fad4.loading != 0);
                        cd_request_wait_idle();
                        DAT_801b24e4.unk101 = 0;
                        DAT_801b24e4.unk140 = 0;
                        DAT_801b24e4.unk13e = 0;
                        DAT_801b24e4.unk13c = 0;
                        DAT_801b24e4.unk124 = DAT_801b24e4.unk12c;
                        DAT_801b24e4.pos.vx = (x << 11) + 1024;
                        DAT_801b24e4.pos.vz = (z << 11) + 1024;
                        DAT_801b24e4.pos.vy = -(y << 7);
                        DAT_801b24e4.unk124.vy = DAT_801b24e4.unk12c.vy = -(yaw << 4);
                        func_80046c00(1, 0x1000, 0x1000, 0);
                        cd_request_wait_idle();
                        func_80046c00(1, 0x1000, 0, -256);
                        resource_request_transition(0xff, 0xff, 0xff, area3, area4, 0x7f, 0x7f, 0x7f);
                    }
                } else {
                    obj->arg.b.unk40 = 0;
                }
            }
            if (DAT_8018fad4.area[0] == 24) {
                if (obj->unk6 == 0x144) {
                    obj->angle.vy += 128;
                }
                if (obj->unk6 == 0x145) {
                    obj->angle.vy += 256;
                }
            }
            break;
        case 234:
            if (func_80046884(obj->arg.b.unk38, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                              obj->arg.b.unk3a.bytes[1], obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    Object44 *o;
                    s32 n = 0x18c;

                    o = DAT_8018fb3c.objects;
                    for (; n != 0; n--, o++) {
                        if (o->unk4 == 0xe7) {
                            o->arg.b.unk40 = 1;
                        }
                    }
                    map_cell_add_layer_occupancy(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vz, 800, -1);
                    func_80046cb0(obj->arg.sb.unk3c << 11, 0, obj->arg.sb.unk3d << 11);
                    map_cell_add_layer_occupancy(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vz, 800, 1);
                }
            } else {
                obj->arg.b.unk40 = 0;
            }
            break;
        case 232:
            if (func_80046884(obj->arg.b.unk38, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                              obj->arg.b.unk3a.bytes[1], obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    CVECTOR c = {0x80, 0x80, 0x80, 0};
                    s32 n = 0x18c;
                    Object44 *o;

                    func_8003daec(&c.r, 320, 240, 1);
                    o = DAT_8018fb3c.objects;
                    for (; n != 0; n--, o++) {
                        if (o->unk4 == 0xe8) {
                            o->arg.b.unk40 = 1;
                        }
                    }
                    map_cell_add_layer_occupancy(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vz, 800, -1);
                    DAT_801b24e4.pos.vx = (obj->arg.b.unk3c << 11) + 1024;
                    DAT_801b24e4.pos.vz = (obj->arg.b.unk3d[0] << 11) + 1024;
                    DAT_801b24e4.unk160 = obj->arg.b.unk3d[1] != 1 ? 5 : 0;
                    DAT_801b24e4.unk12c.vy = -(obj->arg.b.unk3f << 4);
                    player_sync_position_to_map();
                    DAT_801b24e4.unk124.vy = DAT_801b24e4.unk12c.vy + DAT_801b24e4.unk136 +
                                             DAT_801b24e4.unk13e + DAT_801b24e4.unk146;
                    func_8003daec(&c.r, 320, 240, -1);
                }
            } else if (obj->arg.b.unk40 != 0xff) {
                obj->arg.b.unk40 = 0;
            }
            obj->unka = (obj->unka + 64) & 0xfff;
            break;
        case 224:
            if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38, obj->arg.b.unk39,
                              obj->pos.vy) != 0) {
                resource_request_transition(obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3a.bytes[1],
                                            obj->arg.b.unk3c, obj->arg.b.unk3d[0], obj->arg.b.unk3d[1],
                                            obj->arg.sb.unk40, obj->arg.sb.unk41, obj->arg.sb.unk42);
            }
            break;
        case 225:
            if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38, obj->arg.b.unk39,
                              obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    if (!(obj->arg.b.unk3a.bytes[0] & 0x80)) {
                        obj->arg.b.unk40 = 1;
                    }
                    switch (obj->arg.b.unk3a.bytes[0] & 0xf) {
                    case 0:
                        ((void (*)(Object44 *))DAT_8018fad4.handlers[3])(obj);
                        break;
                    case 1:
                        func_800463bc(obj->arg.b.unk3a.bytes[1]);
                        break;
                    case 2:
                        DAT_801ba984.unkab[obj->arg.b.unk3a.bytes[1]] = obj->arg.b.unk3c;
                        break;
                    }
                }
            } else {
                obj->arg.b.unk40 = 0;
            }
            break;
        case 227:
            if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38, obj->arg.b.unk39,
                              0x8000) != 0) {
                DAT_80199154.unk15995 = obj->arg.b.unk3a.bytes[0];
                DAT_80199154.unk15996 = obj->arg.b.unk3a.bytes[1];
            }
            break;
        case 228:
            {
                u8 *movie;

                movie = &DAT_801e7edc.unk384;
                if (*movie == 0xff) {
                    if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                      obj->arg.b.unk39, 0x8000) != 0) {
                        DAT_801e7edc.unk370 = 1;
                        *movie = obj->arg.b.unk3a.bytes[0];
                    }
                } else if (*movie == obj->arg.b.unk3a.bytes[0]) {
                    if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                      obj->arg.b.unk39, 0x8000) == 0) {
                        if (DAT_801e7edc.unk370 == 0) {
                            *movie = 0xff;
                        }
                        if (DAT_801e7edc.unk370 == 16) {
                            *movie = 0xff;
                        }
                    }
                }
            }
            break;
        case 230:
            {
                Object44 *o;
                s32 n;

                if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38, obj->arg.b.unk39,
                                  obj->pos.vy) != 0) {
                    if (obj->arg.b.unk40 == 0) {
                        n = obj->arg.b.unk3c;
                        obj->arg.b.unk40 = 1;
                        o = &DAT_8018fb3c.objects[obj->arg.h.unk3a];
                        do {
                            o->unke += obj->arg.b.unk3d[0];
                            n--;
                            o++;
                        } while (n != 0);
                    }
                } else {
                    if (obj->arg.b.unk40 == 1) {
                        o = &DAT_8018fb3c.objects[obj->arg.h.unk3a];
                        n = obj->arg.b.unk3c;
                        do {
                            o->unke -= obj->arg.b.unk3d[0];
                            n--;
                            o++;
                        } while (n != 0);
                    }
                    obj->arg.b.unk40 = 0;
                }
            }
            break;
        case 88:
            switch (obj->unk8) {
            case 1:
                switch (obj->arg.b.unk38) {
                case 0:
                    obj->unk8 = 2;
                    func_8004482c(obj, 20);
                    break;
                case 1:
                    obj->unk8 = 3;
                    func_8004482c(obj, 20);
                    break;
                }
                func_800445b8(obj->unk0, obj->arg.b.unk3c + obj->arg.b.unk3d[1], obj->arg.b.unk3d[0],
                              obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3d[1],
                              obj->arg.b.unk3f, obj->angle.vy, 45);
                break;
            case 2:
                obj->unka -= 64;
                if ((s16)obj->unka <= 0) {
                    obj->unk8 = 99;
                    obj->unka = 0;
                    func_800445b8(obj->unk0, obj->arg.b.unk3c, obj->arg.b.unk3d[0], obj->arg.b.unk3a.bytes[0],
                                  obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3d[1], obj->arg.b.unk3f,
                                  obj->angle.vy, 45);
                }
                break;
            case 3:
                obj->unka += 64;
                if (obj->unka >= 4096) {
                    obj->unka = 0xfff;
                    obj->unk8 = 99;
                }
                break;
            }
            break;
        case 89:
            {
                MapLayer *layer;

                switch (obj->unk8) {
                case 1:
                    obj->unk8 = 2;
                    obj->arg.h.unk40 = obj->arg.h.unk3a;
                    obj->unk0 = obj->arg.b.unk41[1];
                    break;
                case 2:
                    if (obj->arg.h.unk40-- == 0) {
                        obj->unk8 = 3;
                        {
                            MapCell *row = DAT_801d4464.cells[obj->pos.vz >> 11];

                            layer = row[obj->pos.vx >> 11].layer;
                        }
                        if (obj->unk0 != 1) {
                            layer++;
                        }
                        layer->unk3 = 0x74;
                    }
                    break;
                case 3:
                    obj->unk10 -= 256;
                    if ((s16)obj->unk10 <= 0) {
                        obj->unk10 = 0;
                        obj->unk8 = 4;
                        obj->arg.h.unk40 = obj->arg.h.unk3c;
                    }
                    break;
                case 4:
                    if (obj->arg.h.unk40-- == 0) {
                        obj->unk8 = 5;
                    }
                    break;
                case 5:
                    obj->unk10 += 256;
                    if (obj->unk10 >= 4096) {
                        obj->unk10 = 0x1000;
                        obj->unk8 = 0;
                        {
                            MapCell *row = DAT_801d4464.cells[obj->pos.vz >> 11];

                            layer = row[obj->pos.vx >> 11].layer;
                        }
                        if (obj->unk0 != 1) {
                            layer++;
                        }
                        layer->unk3 = 0x75;
                        obj->unk0 = 0;
                    }
                    break;
                }
            }
            break;
        case 17:
            {
                Object44 *o;
                static SVECTOR from = {0, -100, 300, 0};
                static SVECTOR to = {0, 0, 64, 0};

                o = &DAT_8018fb3c.objects[obj->arg.b.unk39 + 380];
                if (obj->unk8 != 0 && o->unk6 == 0xff) {
                    obj->arg.b.unk38 = 0xff;
                    obj->unk8 = 0;
                    (DAT_8018fb3c.objects + obj->arg.b.unk3a.bytes[1])->arg.b.unk38 &=
                        ~obj->arg.b.unk3a.bytes[0];
                }
                if (func_80046914(obj, o, &from, &to, 0, 20) != 0) {
                    (DAT_8018fb3c.objects + obj->arg.b.unk3a.bytes[1])->arg.b.unk38 |=
                        obj->arg.b.unk3a.bytes[0];
                }
            }
            break;
        case 24:
            {
                Object44 *o;
                static SVECTOR from = {0, -1536, 400, 0};
                static SVECTOR to = {0, -1280, 0, 0};
                Object44 *sw;

                o = &DAT_8018fb3c.objects[obj->arg.b.unk39 + 380];
                if (o->unk6 == 0x86 &&
                    (DAT_8018fad4.area[0] == 23 ||
                     ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.previous[0] == 23))) {
                    to.vy = -1152;
                }
                if (obj->unk8 != 0 && o->unk6 == 0xff) {
                    obj->arg.b.unk38 = 0xff;
                    obj->unk8 = 0;
                    sw = DAT_8018fb3c.objects + obj->arg.b.unk3a.bytes[1];
                    if (obj->arg.b.unk3d[0] != 0xff) {
                        sw->arg.b.unk38 &= ~obj->arg.b.unk3d[0];
                    } else {
                        sw->arg.b.unk38 &= ~obj->arg.b.unk3a.bytes[0];
                    }
                }
                if (func_80046914(obj, o, &from, &to, 0, 20) != 0) {
                    (DAT_8018fb3c.objects + obj->arg.b.unk3a.bytes[1])->arg.b.unk38 |=
                        obj->arg.b.unk3a.bytes[0];
                }
            }
            break;
        case 26:
            {
                Object44 *o;
                static SVECTOR from = {0, -100, 300, 0};
                static SVECTOR to = {0, 0, 64, 0};
                Object44 *sw;

                o = &DAT_8018fb3c.objects[obj->arg.b.unk39 + 350];
                if (func_80046914(obj, o, &from, &to, 0, 20) != 0) {
                    sw = DAT_8018fb3c.objects + obj->arg.b.unk3a.bytes[1];
                    o->arg.b.unk38 = 0;
                    sw->arg.b.unk38 |= obj->arg.b.unk3a.bytes[0];
                    obj->unk8 = 3;
                }
                switch (obj->unk8) {
                case 3:
                    obj->unk5 = 50;
                    obj->unk2 = (obj->unk2 & 0xf8) | 5;
                    o->unk5 = 50;
                    o->unk2 = (o->unk2 & 0xf8) | 5;
                    obj->unk8 = 4;
                    break;
                case 4:
                    obj->unk10 += 256;
                    o->unk10 += 256;
                    if (obj->unk10 >= 4095) {
                        obj->unk10 = 0xfff;
                        o->unk10 = 0xfff;
                        obj->unk8 = 5;
                    }
                    break;
                case 5:
                    obj->unk6 = 0xff;
                    o->unk6 = 0xff;
                    obj->unk8 = 99;
                    break;
                }
            }
            break;
        case 49:
            obj->angle.vz -= 32;
            break;
        case 50:
            {
                Object88 *e;

                e = NULL;
                if (obj->arg.h.unk3a != 0xffff) {
                    e = &DAT_80185da8.objects[obj->arg.h.unk3a];
                }
                if (obj->unk8 == 0) {
                    if (e != NULL) {
                        obj->unka = (e->unk18 + obj->arg.h.unk3c) & 0xfff;
                    } else {
                        obj->unka = (obj->unka + obj->arg.h.unk38) & 0xfff;
                    }
                }
            }
            break;
        case 16:
            obj->angle.vy += 128;
            obj->pos.vy = (rsin(obj->angle.vy) >> 6) + obj->arg.w.unk40;
            break;
        case 18:
        case 22:
            if (obj->arg.w.unk40 - (s32)DAT_801c12e4.frames < 0) {
                obj->arg.w.unk40 = DAT_801c12e4.frames + 30;
            }
            break;
        case 96:
            switch (obj->unk8) {
            case 0:
                {
                    s32 floor;

                    floor = collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz, def->unk4,
                                                         def->unk8);
                    obj->unk0 = DAT_801d4464.unk1200a == 0 ? 1 : 2;
                    obj->pos.vy += obj->arg.sh.unk3e;
                    obj->arg.sh.unk3e += 20;
                    if (obj->pos.vy >= floor) {
                        obj->pos.vy = floor;
                        obj->arg.sh.unk3e = 16;
                        obj->unk8 = 1;
                    }
                }
                break;
            case 1:
                obj->angle.vx += obj->arg.sh.unk3e;
                obj->arg.sh.unk3e += 16;
                if (obj->angle.vx >= 1024) {
                    obj->angle.vx = 0x400;
                    obj->unk8 = 99;
                }
                break;
            }
            break;
        case 97:
            if (obj->unk8 == 0) {
                s32 floor;

                floor = collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz, def->unk4,
                                                     def->unk8);
                obj->unk0 = DAT_801d4464.unk1200a == 0 ? 1 : 2;
                obj->pos.vy += 20;
                obj->angle.vy = (obj->angle.vy + 256) & 0xfff;
                if (obj->pos.vy >= floor) {
                    obj->pos.vy = floor;
                    obj->unk8 = 99;
                }
            }
            break;
        case 98:
            if (obj->unk8 < 2) {
                s32 floor;
                s32 ang;

                floor = collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz, def->unk4,
                                                     def->unk8);
                obj->unk0 = DAT_801d4464.unk1200a == 0 ? 1 : 2;
                obj->pos.vy += obj->arg.sh.unk3e;
                func_80056268(&obj->pos, 0x1000, 6000, 300);
                ang = obj->angle.vx;
                obj->angle.vx = (obj->unk8 == 0 ? ang + 160 : ang - 160) & 0xfff;
                obj->arg.sh.unk3e += 30;
                if (obj->arg.sh.unk3e >= 0 && obj->pos.vy >= floor) {
                    if (obj->arg.sh.unk3e < 80) {
                        obj->pos.vy = floor;
                        obj->unk8 = 2;
                    } else {
                        obj->arg.sh.unk3e = -(obj->arg.sh.unk3e >> 1);
                        obj->unk8 = obj->unk8 == 0;
                    }
                }
            } else if (obj->unk8 == 2) {
                obj->angle.vx = angle_approach(obj->angle.vx, obj->angle.pad, 160);
                if (obj->angle.vx == obj->angle.pad) {
                    obj->unk8 = 99;
                }
            }
            break;
        default:
            DAT_8018fad4.handlers[9]();
            break;
        case 229:
        case 233:
            break;
        }
    } while (obj++, --i != 0);
    DAT_8018fb3c.unk8858 = NULL;
}
