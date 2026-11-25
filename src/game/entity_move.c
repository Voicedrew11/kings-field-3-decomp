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
    hit = collision_query_world(pos.vx, pos.vy, pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                                DAT_80185da8.unk9d04);
    if (hit == 0) {
        obj->pos.vx = pos.vx;
        obj->pos.vy = pos.vy;
        obj->pos.vz = pos.vz;
    } else if (collision_query_world(pos.vx, obj->pos.vy, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                                     DAT_80185da8.unk9d04)) {
        vel->vx = -vel->vx;
    } else if (collision_query_world(obj->pos.vx, pos.vy, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                                     DAT_80185da8.unk9d04)) {
        vel->vy = -vel->vy;
    } else if (collision_query_world(obj->pos.vx, obj->pos.vy, pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
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
    if (trajectory_solve_motion_between_points(high, obj->pos.vx, obj->pos.vy, obj->pos.vz, x, y, z, gravity,
                                               speed, &time, &obj->unk68, &obj->unk6a) != 0) {
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
    Object88 *obj = DAT_80185da8.current;
    Object78 *def = DAT_80185da8.def;
    u16 *wet = &DAT_801d4464.unk1200a;

    collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj));
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
        return;
    }
    case 0x10: {
        s32 y;
        s32 hit;

        y = obj->pos.vy + obj->vel.vy;
        hit = collision_query_world(obj->pos.vx, y, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                                    DAT_80185da8.unk9d04);
        if (hit == 0) {
        fall:
            obj->pos.vy = y;
            obj->vel.vy += def->unk5;
            return;
        }
        if (hit == 0x80 && obj->vel.vy > 40) {
            player_apply_damage(0, def->unk6, 0, 0, 0, 0, 0, 0, 0, 0x1000, 10, &obj->pos);
        }
        if (hit & 4) {
            if (obj->unk28 & 0x400) {
                s32 floor = DAT_801d4464.unk1200c;

                if (obj->pos.vy < floor) {
                    goto fall;
                }
                obj->pos.vy = floor;
            } else {
                obj->pos.vy = DAT_801d4464.unk12010;
            }
            obj->vel.vy = 0;
            goto idle;
        }
        if (obj->unk28 & 0x400) {
            goto fall;
        }
        goto idle;
    }
    case 0x20:
        obj->pos.vy += obj->vel.vy;
        obj->vel.vy += 5;
        if (DAT_801d4464.unk12010 < obj->pos.vy && obj->vel.vy < 0) {
            return;
        }
        obj->pos.vy = DAT_801d4464.unk12010;
    idle:
        obj->unkd = 0;
        return;
    case 0x30: {
        s32 jump_y;
        s32 jump_hit;

        jump_y = obj->unk3c - obj->unk6a * obj->vel.vy + ((obj->unk6c * obj->vel.vy * obj->vel.vy) >> 1);
        jump_hit = collision_query_world(obj->pos.vx, jump_y, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                                         DAT_80185da8.unk9d04);
        if (jump_hit == 0) {
            obj->pos.vy = jump_y;
            obj->vel.vy++;
            obj->unk3 = *wet == 0 ? 1 : 2;
            return;
        }
        if (jump_hit == 0x80) {
            player_apply_damage(0, def->unk6, 0, 0, 0, 0, 0, 0, 0, 0x1000, 10, &obj->pos);
        }
        obj->unkd = 0x10;
        obj->vel.vy = 0;
        return;
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
        obj->vel.vx = value_approach(obj->vel.vx, 0, obj->vel.vx * speed * 2 / length);
        obj->vel.vz = value_approach(obj->vel.vz, 0, obj->vel.vz * speed * 2 / length);
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
    angle_to_forward_xz(angle, target.v);
    step = target;
    vector2i_scale_shift11(scale_x, target.v);
    vector2i_scale_shift11(scale_z, step.v);
    obj->vel.vx = value_approach(obj->vel.vx, target.v[0], step.v[0]);
    obj->vel.vz = value_approach(obj->vel.vz, target.v[1], step.v[1]);
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
 * target arrives as an int (func_800500a8 passes vector_xz_to_angle's result
 * and Object88.unk20 unconverted); retail narrows it to 16 bits for each
 * angle helper call.
 */
ADDRESS(0x8004e928, 0x154)
void func_8004e928(Object88 *obj, int target, s32 max, s32 accel)
{
    s32 yaw;

    if (angle_shortest_delta((s16)target, obj->angle.vy) != 0) {
        if (angle_mod_delta_le_half_turn(target, obj->angle.vy)) {
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
            if (angle_signed_greater_than((s16)target, yaw) && !angle_signed_greater_than((s16)target, obj->angle.vy)) {
                obj->angle.vy = target;
            }
        } else {
            if (angle_signed_greater_than(yaw, (s16)target) && !angle_signed_greater_than(obj->angle.vy, (s16)target)) {
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
    Object88 *obj = DAT_80185da8.current;
    s32 dx = x - obj->pos.vx;
    s32 dz = z - obj->pos.vz;
    s32 angle = vector_xz_to_angle(dx, dz);
    s32 distance = abs(dx) + abs(dz);

    if (face != -1 && distance <= 600 && !angle_within_tolerance(angle, face, 0x320)) {
        return -1;
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
    pitch_yaw_to_forward_vector(angle, &target);
    step = target;
    vector3s_scale_shift12(scale_a, &target);
    vector3s_scale_shift12(scale_b, &step);
    obj->vel.vx = value_approach(obj->vel.vx, target.vx, step.vx);
    obj->vel.vy = value_approach(obj->vel.vy, target.vy, step.vy);
    obj->vel.vz = value_approach(obj->vel.vz, target.vz, step.vz);
    r = func_8004dbc8(&obj->vel, mode) != 0;
    y = obj->vel.vy + obj->pos.vy;
    if (collision_query_world(obj->pos.vx, y, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                              DAT_80185da8.unk9d04) == 0) {
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
    obj->angle.vx = angle_approach(obj->angle.vx, angle->vx, 8);
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
    vector_rotate_yxz(&rotation, &v, out);
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
        out->vx = obj->pos.vx + def->unkc;
        out->vy = obj->pos.vy + def->unke;
        out->vz = obj->pos.vz + def->unk10;
        return out;
    case 2:
        def = &DAT_80185da8.defs[obj->unk2];
        vector_rotate_yxz(&obj->angle, (SVECTOR *)&def->unkc, out);
        out->vx += obj->pos.vx;
        out->vy += obj->pos.vy;
        out->vz += obj->pos.vz;
        return out;
    default:
        break;
    }
    /* Mode 3 returns whatever is left in the return register. */
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
        vector_displacement_to_pitch_yaw(at.vx - target->vx, -(target->vy + 1600) + at.vy, at.vz - target->vz,
                                         &angle);
        d = (angle.vy - obj->angle.vy) & 0xfff;
        if (d >= 2048) {
            d = 0x1000 - d;
        }
        d = (d << 12) / turn;
        if (d > 0x1000) {
            d = 0x1000;
        }
        angle.vy = angle_lerp_shortest_q12(angle.vy, obj->angle.vy, d);
        if ((s16)pitch == -1) {
            d = (angle.vx - obj->angle.vx) & 0xfff;
            if (d >= 2048) {
                d = 0x1000 - d;
            }
            angle.vx = angle_lerp_shortest_q12(angle.vx, obj->angle.vx, d);
        } else {
            angle.vx = pitch;
        }
        pitch_yaw_to_forward_vector(&angle, out);
        vector3s_scale_shift12(speed, out);
        if (--steps == 0) {
            break;
        }
        vector_add_scaled_delta(from, &DAT_801b24e4.unk11c,
                                fixed_vector3_length(target->vx - at.vx, target->vy - at.vy,
                                                     target->vz - at.vz) / speed, &at);
    }
    return angle.vy;
}
