/*
 * GAME.EXE: Object88 movement (WIP module): bouncing steps, jumps and
 * falls, steering toward an angle, a vertex of the record's model, its aim
 * point, the walk animation pick and the aim at the player.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>

/* Moves the current record by vel; on a hit, reverses the blocked components. */
ADDRESS(0x8004e094, 0x1e8)
s32 func_8004e094(SVECTOR *vel)
{
    Object88 *obj;
    VECTOR pos;
    s32 hit;

    obj = DAT_80185da8.current;
    pos.vx = vel->vx + obj->pos.vx;
    pos.vy = vel->vy + obj->pos.vy;
    pos.vz = vel->vz + obj->pos.vz;
    hit = func_80033f38(pos.vx, pos.vy, pos.vz, obj->unk1c, OBJECT88_SHAPE(obj), DAT_80185da8.unk9d04);
    if (hit == 0) {
        obj->pos.vx = pos.vx;
        obj->pos.vy = pos.vy;
        obj->pos.vz = pos.vz;
    } else if (func_80033f38(pos.vx, obj->pos.vy, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                             DAT_80185da8.unk9d04)) {
        vel->vx = -vel->vx;
    } else if (func_80033f38(obj->pos.vx, pos.vy, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                             DAT_80185da8.unk9d04)) {
        vel->vy = -vel->vy;
    } else if (func_80033f38(obj->pos.vx, obj->pos.vy, pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                             DAT_80185da8.unk9d04)) {
        vel->vz = -vel->vz;
    }
    return hit;
}

/* Starts a jump of the current record to (x, y, z); returns its time or -1. */
ADDRESS(0x8004e27c, 0xa0)
s32 func_8004e27c(s32 high, s32 x, s32 y, s32 z, s32 gravity, s32 speed)
{
    Object88 *obj;
    s16 time;

    obj = DAT_80185da8.current;
    if (func_8001749c(high, obj->pos.vx, obj->pos.vy, obj->pos.vz, x, y, z, gravity, speed, &time, &obj->unk68,
                      &obj->unk6a) != 0) {
        return -1;
    }
    obj->unkd = 0x30;
    obj->vel.vy = 1;
    obj->unk6c = gravity;
    obj->unk3c = obj->pos.vy;
    return time;
}

ADDRESS(0x8004e31c, 0x14)
void func_8004e31c(void)
{
    DAT_80185da8.current->unkd = 0x60;
}

/* Falls, lands and jumps the current record (unkd 0, 0x10 falling, 0x20 rising, 0x30 jumping). */
ADDRESS(0x8004e330, 0x3c8)
void func_8004e330(void)
{
    Object88 *obj;
    Object78 *def;
    u16 *wet;
    s32 y;
    s32 hit;

    obj = DAT_80185da8.current;
    def = DAT_80185da8.def;
    func_80033b10(obj->pos.vx, obj->pos.vy, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj));
    wet = &DAT_801d4464.unk1200a;
    obj->unk3 = *wet == 0 ? 1 : 2;
    if (obj->unk28 & 0x400) {
        DAT_801d4464.unk12010 = DAT_801d4464.unk1200c;
    }
    switch (obj->unkd) {
    case 0: {
        s32 dy;

        dy = DAT_801d4464.unk12010 - obj->pos.vy;
        if (dy < 0) {
            obj->unkd = 0x20;
            obj->vel.vy = -100;
        } else if (dy > 0) {
            obj->unkd = 0x10;
            obj->vel.vy = 0;
        }
        break;
    }
    case 0x10:
        y = obj->vel.vy + obj->pos.vy;
        hit = func_80033f38(obj->pos.vx, y, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj), DAT_80185da8.unk9d04);
        if (hit == 0) {
        fall:
            obj->pos.vy = y;
            obj->vel.vy += def->unk5;
            break;
        }
        if (hit == 0x80 && obj->vel.vy > 40) {
            func_8002ab18(0, def->unk6, 0, 0, 0, 0, 0, 0, 0, 0x1000, 10, &obj->pos);
        }
        if (hit & 4) {
            if (obj->unk28 & 0x400) {
                if (DAT_801d4464.unk1200c > obj->pos.vy) {
                    goto fall;
                }
                obj->pos.vy = DAT_801d4464.unk1200c;
            } else {
                obj->pos.vy = DAT_801d4464.unk12010;
            }
            obj->vel.vy = 0;
            obj->unkd = 0;
            break;
        }
        if (obj->unk28 & 0x400) {
            goto fall;
        }
        obj->unkd = 0;
        break;
    case 0x20:
        obj->pos.vy += obj->vel.vy;
        obj->vel.vy += 5;
        if (DAT_801d4464.unk12010 < obj->pos.vy && obj->vel.vy < 0) {
            break;
        }
        obj->pos.vy = DAT_801d4464.unk12010;
        obj->unkd = 0;
        break;
    case 0x30: {
        s32 jump_y;
        s32 jump_hit;

        jump_y = obj->unk3c - obj->unk6a * obj->vel.vy + ((obj->unk6c * obj->vel.vy * obj->vel.vy) >> 1);
        jump_hit = func_80033f38(obj->pos.vx, jump_y, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                                 DAT_80185da8.unk9d04);
        if (jump_hit == 0) {
            obj->pos.vy = jump_y;
            obj->vel.vy++;
            obj->unk3 = *wet == 0 ? 1 : 2;
            break;
        }
        if (jump_hit == 0x80) {
            func_8002ab18(0, def->unk6, 0, 0, 0, 0, 0, 0, 0, 0x1000, 10, &obj->pos);
        }
        obj->unkd = 0x10;
        obj->vel.vy = 0;
        break;
    }
    case 0x60:
        break;
    }
}

/* Scales the current record's horizontal velocity toward speed, then moves it. */
ADDRESS(0x8004e6f8, 0x12c)
s32 func_8004e6f8(s32 speed, s32 mode)
{
    Object88 *obj;
    s32 length;

    obj = DAT_80185da8.current;
    if (obj->unkd == 0) {
        length = SquareRoot0(obj->vel.vx * obj->vel.vx + obj->vel.vz * obj->vel.vz);
        if (length == 0) {
            return 0;
        }
        obj->vel.vx = func_80015fdc(obj->vel.vx, 0, obj->vel.vx * speed * 2 / length);
        obj->vel.vz = func_80015fdc(obj->vel.vz, 0, obj->vel.vz * speed * 2 / length);
    }
    return func_8004dbc8(&obj->vel, mode);
}

/* Accelerates the current record along angle, then moves it. */
ADDRESS(0x8004e824, 0xc8)
s32 func_8004e824(s32 angle, s32 scale_x, s32 scale_z, s32 mode)
{
    Object88 *obj;
    struct {
        s32 v[2];
    } target, step;

    obj = DAT_80185da8.current;
    func_80016240(angle, target.v);
    step = target;
    func_800168f0(scale_x, target.v);
    func_800168f0(scale_z, step.v);
    obj->vel.vx = func_80015fdc(obj->vel.vx, target.v[0], step.v[0]);
    obj->vel.vz = func_80015fdc(obj->vel.vz, target.v[1], step.v[1]);
    return func_8004dbc8(&obj->vel, mode);
}

ADDRESS(0x8004e8ec, 0x28)
s32 func_8004e8ec(s32 angle, s32 target)
{
    s16 difference = target - angle;

    if (difference >= 2048) {
        return 0x1000 - difference;
    }
    return difference & 0xfff;
}

ADDRESS(0x8004e914, 0x14)
s32 func_8004e914(s32 angle, s32 target)
{
    s16 difference = angle - target;

    return difference <= 2048;
}

/*
 * Turns obj toward target: the turn rate obj->unk58 grows by accel up to max.
 * K&R definition: target arrives as int (func_800500a8 passes
 * func_80016ab8's result and Object88.unk20 unconverted).
 */
ADDRESS(0x8004e928, 0x154)
void func_8004e928(obj, target, max, accel)
    Object88 *obj;
    s16 target;
    s32 max;
    s32 accel;
{
    s32 yaw;

    if (func_80016a98(target, obj->angle.vy) != 0) {
        if (func_80016a68(target, obj->angle.vy)) {
            obj->unk58 += accel;
            if (obj->unk58 > max) {
                obj->unk58 = max;
            }
        } else {
            obj->unk58 -= accel;
            if (obj->unk58 < -max) {
                obj->unk58 = -max;
            }
        }
        yaw = obj->angle.vy;
        obj->angle.vy += obj->unk58;
        if (obj->unk58 > 0) {
            if (func_80016a78(target, yaw) && !func_80016a78(target, obj->angle.vy)) {
                obj->angle.vy = target;
            }
        } else {
            if (func_80016a78(yaw, target) && !func_80016a78(obj->angle.vy, target)) {
                obj->angle.vy = target;
            }
        }
    } else {
        obj->unk58 = 0;
    }
}

ADDRESS(0x8004ea7c, 0x74)
/* Returns func_8004e824's result (FDAT module slot 17 handlers test it). */
s32 func_8004ea7c(s16 target, s32 scale_x, s32 max, s32 scale_z, s32 accel, s32 mode)
{
    Object88 *obj;

    obj = DAT_80185da8.current;
    func_8004e928(obj, target, max, accel);
    return func_8004e824(obj->angle.vy, scale_x, scale_z, mode);
}

/* Walks the current record toward (x, z); -1 when face is set, it is close and not facing. */
ADDRESS(0x8004eaf0, 0x118)
s32 func_8004eaf0(s32 x, s32 z, s32 scale_x, s32 max, s16 face, s32 scale_z, s32 accel, s32 mode)
{
    Object88 *obj;
    s32 dx;
    s32 dz;
    s32 angle;

    obj = DAT_80185da8.current;
    dx = x - obj->pos.vx;
    dz = z - obj->pos.vz;
    angle = func_80016ab8(dx, dz);
    if (dx < 0) {
        dx = -dx;
    }
    if (dz < 0) {
        dz = -dz;
    }
    dx += dz;
    if (face != -1 && dx < 601) {
        if (!func_80016a2c(angle, face, 0x320)) {
            return -1;
        }
    }
    func_8004ea7c(angle, scale_x, max, scale_z, accel, mode);
    return angle & 0xfff;
}

/* Accelerates the current record along angle in 3D, then moves it and applies its fall. */
ADDRESS(0x8004ec08, 0x148)
s32 func_8004ec08(SVECTOR *angle, s32 scale_a, s32 scale_b, s32 mode)
{
    Object88 *obj;
    SVECTOR target;
    SVECTOR step;
    s32 r;
    s32 y;

    obj = DAT_80185da8.current;
    func_800167cc(angle, &target);
    step = target;
    func_80016928(scale_a, &target);
    func_80016928(scale_b, &step);
    obj->vel.vx = func_80015fdc(obj->vel.vx, target.vx, step.vx);
    obj->vel.vy = func_80015fdc(obj->vel.vy, target.vy, step.vy);
    obj->vel.vz = func_80015fdc(obj->vel.vz, target.vz, step.vz);
    r = func_8004dbc8(&obj->vel, mode) != 0;
    y = obj->vel.vy + obj->pos.vy;
    if (func_80033f38(obj->pos.vx, y, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj), DAT_80185da8.unk9d04) == 0) {
        obj->pos.vy = y;
    } else {
        r |= 2;
    }
    return r;
}

/* Returns func_8004ec08's result (func_800500a8 tests it). */
ADDRESS(0x8004ed50, 0x90)
s32 func_8004ed50(SVECTOR *angle, s32 scale_a, s32 max, s32 scale_b, s32 accel, s32 mode)
{
    Object88 *obj;

    obj = DAT_80185da8.current;
    func_8004e928(obj, angle->vy, max, accel);
    obj->angle.vx = func_80015f18(obj->angle.vx, angle->vx, 8);
    return func_8004ec08(&obj->angle, scale_a, scale_b, mode);
}

/* A vertex of obj's posed model relative to its position. */
ADDRESS(0x8004ede0, 0x100)
s32 func_8004ede0(Object88 *obj, s32 vertex, VECTOR *out)
{
    SVECTOR rotation;
    SVECTOR v;

    if (func_8004352c(obj->unk1 + 128, obj->unkc, obj->unk18, vertex, &v)) {
        v.vx = 0;
        v.vy = -obj->unk1e >> 1;
        v.vz = -obj->unk1c;
    } else {
        v.vx = v.vx * obj->scale.vx >> 12;
        v.vy = v.vy * obj->scale.vy >> 12;
        v.vz = v.vz * obj->scale.vz >> 12;
    }
    rotation.vx = obj->angle.vx;
    rotation.vy = obj->angle.vy + 2048;
    rotation.vz = obj->angle.vz;
    func_800168a8(&rotation, &v, out);
    return 1;
}

/* obj's aim point (Object88.unk28 bits 0-1: position, offset, rotated offset). */
ADDRESS(0x8004eee0, 0x128)
VECTOR *func_8004eee0(Object88 *obj, VECTOR *out)
{
    Object78 *def;

    switch (obj->unk28 & 3) {
    case 0:
        return &obj->pos;
    case 1:
        def = &DAT_80185da8.defs[obj->unk2];
        out->vx = def->unkc + obj->pos.vx;
        out->vy = def->unke + obj->pos.vy;
        out->vz = def->unk10 + obj->pos.vz;
        return out;
    case 2:
        func_800168a8(&obj->angle, (SVECTOR *)&DAT_80185da8.defs[obj->unk2].unkc, out);
        out->vx += obj->pos.vx;
        out->vy += obj->pos.vy;
        out->vz += obj->pos.vz;
        return out;
    }
}

/* Picks the current record's walk animation from its turn rate and climb. */
ADDRESS(0x8004f008, 0x1c8)
void func_8004f008(s32 idle, s32 left, s32 right, s32 up, s32 down, s32 speed)
{
    Object88 *obj;
    s32 anim;
    s32 turn;

    obj = DAT_80185da8.current;
    anim = idle;
    turn = 0;
    if (obj->unk58 > 0) {
        anim = right;
        turn = obj->unk58;
    } else if (obj->unk58 < 0) {
        anim = left;
        turn = -obj->unk58;
    }
    if (obj->vel.vy > 10) {
        if (turn < obj->vel.vy) {
            anim = up;
        }
    } else if (obj->vel.vy < 10 && turn < -obj->vel.vy) {
        anim = down;
    }
    if (obj->unkc == idle) {
        func_8004db08(obj, speed);
        if (func_8004db98(obj, 0)) {
            func_8004c184(anim);
        }
    } else if (obj->unkc == left) {
        if (obj->unk18 < 2048) {
            func_8004db3c(obj, speed);
        } else if (anim != left) {
            func_8004db3c(obj, speed);
            if (obj->unk18 >= 4095) {
                func_8004c130(anim);
            }
        }
    } else if (obj->unkc == right) {
        if (obj->unk18 < 2048) {
            func_8004db3c(obj, speed);
        } else if (anim != right) {
            func_8004db3c(obj, speed);
            if (obj->unk18 >= 4095) {
                func_8004c130(anim);
            }
        }
    } else if (obj->unkc == up) {
        if (obj->unk18 < 2048) {
            func_8004db3c(obj, speed);
        } else if (anim != up) {
            func_8004db3c(obj, speed);
            if (obj->unk18 >= 4095) {
                func_8004c130(anim);
            }
        }
    } else if (obj->unkc == down) {
        if (obj->unk18 < 2048) {
            func_8004db3c(obj, speed);
        } else if (anim != down) {
            func_8004db3c(obj, speed);
            if (obj->unk18 >= 4095) {
                func_8004c130(anim);
            }
        }
    }
}

/* Aims from `from` at target, leading the player's motion over steps refinements. */
ADDRESS(0x8004f1d0, 0x244)
s32 func_8004f1d0(Object88 *obj, VECTOR *from, s32 speed, VECTOR *target, SVECTOR *out, s32 pitch,
                  s16 turn, s32 steps)
{
    VECTOR at;
    SVECTOR angle;
    s32 d;

    at = *from;
    for (;;) {
        func_80016c90(at.vx - target->vx, -(target->vy + 1600) + at.vy, at.vz - target->vz, &angle);
        d = (angle.vy - obj->angle.vy) & 0xfff;
        if (d >= 2048) {
            d = 0x1000 - d;
        }
        d = (d << 12) / turn;
        if (d > 0x1000) {
            d = 0x1000;
        }
        angle.vy = func_80017170(angle.vy, obj->angle.vy, d);
        if ((s16)pitch == -1) {
            d = (angle.vx - obj->angle.vx) & 0xfff;
            if (d >= 2048) {
                d = 0x1000 - d;
            }
            angle.vx = func_80017170(angle.vx, obj->angle.vx, d);
        } else {
            angle.vx = pitch;
        }
        func_800167cc(&angle, out);
        func_80016928(speed, out);
        if (--steps == 0) {
            break;
        }
        func_800175a8(from, &DAT_801b24e4.unk11c,
                      func_80016c44(target->vx - at.vx, target->vy - at.vy, target->vz - at.vz) / speed, &at);
    }
    return angle.vy;
}
