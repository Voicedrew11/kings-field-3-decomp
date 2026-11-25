/*
 * GAME.EXE: Actor movement (WIP module): bouncing steps, jumps and
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
s32 actor_move_with_collision(SVECTOR *vel)
{
    Actor *obj;
    VECTOR pos;
    s32 hit;

    obj = actor_status.current;
    pos.vx = vel->vx + obj->pos.vx;
    pos.vy = vel->vy + obj->pos.vy;
    pos.vz = vel->vz + obj->pos.vz;
    hit = collision_query_world(pos.vx, pos.vy, pos.vz, obj->collision_radius, ACTOR_SHAPE(obj),
                                actor_status.actor_collision_query_flags);
    if (hit == 0) {
        obj->pos.vx = pos.vx;
        obj->pos.vy = pos.vy;
        obj->pos.vz = pos.vz;
    } else if (collision_query_world(pos.vx, obj->pos.vy, obj->pos.vz, obj->collision_radius,
                                     ACTOR_SHAPE(obj), actor_status.actor_collision_query_flags)) {
        vel->vx = -vel->vx;
    } else if (collision_query_world(obj->pos.vx, pos.vy, obj->pos.vz, obj->collision_radius,
                                     ACTOR_SHAPE(obj), actor_status.actor_collision_query_flags)) {
        vel->vy = -vel->vy;
    } else if (collision_query_world(obj->pos.vx, obj->pos.vy, pos.vz, obj->collision_radius,
                                     ACTOR_SHAPE(obj), actor_status.actor_collision_query_flags)) {
        vel->vz = -vel->vz;
    }
    return hit;
}

/* Starts a jump of the current record to (x, y, z); returns its time or -1. */
ADDRESS(0x8004e27c, 0xa0)
s32 actor_start_ballistic_motion(s32 high, s32 x, s32 y, s32 z, s32 gravity, s32 speed)
{
    Actor *obj;
    s16 time;

    obj = actor_status.current;
    if (trajectory_solve_motion_between_points(high, obj->pos.vx, obj->pos.vy, obj->pos.vz, x, y, z, gravity,
                                               speed, &time, &obj->ballistic_horizontal_speed,
                                               &obj->ballistic_launch_speed_y) != 0) {
        return -1;
    }
    obj->vertical_motion_state = 0x30;
    obj->vel.vy = 1;
    obj->ballistic_acceleration = gravity;
    obj->ballistic_origin_y = obj->pos.vy;
    return time;
}

ADDRESS(0x8004e31c, 0x14)
void actor_suspend_vertical_motion(void)
{
    actor_status.current->vertical_motion_state = 0x60;
}

/* Falls, lands and jumps the current record (unkd 0, 0x10 falling, 0x20 rising, 0x30 jumping). */
ADDRESS(0x8004e330, 0x3c8)
void actor_update_vertical_motion(void)
{
    Actor *obj = actor_status.current;
    ActorGroup *def = actor_status.current_group;
    u16 *wet = &map_world.unk1200a;

    collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz, obj->collision_radius,
                                 ACTOR_SHAPE(obj));
    obj->layer_mask = *wet == 0 ? 1 : 2;
    if (obj->flags & 0x400) {
        map_world.unk12010 = map_world.unk1200c;
    }
    switch (obj->vertical_motion_state) {
    case 0: {
        s32 dy;

        dy = map_world.unk12010 - obj->pos.vy;
        if (dy < 0) {
            obj->vertical_motion_state = 0x20;
            obj->vel.vy = -100;
        } else if (dy > 0) {
            obj->vertical_motion_state = 0x10;
            obj->vel.vy = 0;
        }
        return;
    }
    case 0x10: {
        s32 y;
        s32 hit;

        y = obj->pos.vy + obj->vel.vy;
        hit = collision_query_world(obj->pos.vx, y, obj->pos.vz, obj->collision_radius, ACTOR_SHAPE(obj),
                                    actor_status.actor_collision_query_flags);
        if (hit == 0) {
        fall:
            obj->pos.vy = y;
            obj->vel.vy += def->vertical_acceleration;
            return;
        }
        if (hit == 0x80 && obj->vel.vy > 40) {
            player_apply_damage(0, def->landing_damage, 0, 0, 0, 0, 0, 0, 0, 0x1000, 10, &obj->pos);
        }
        if (hit & 4) {
            if (obj->flags & 0x400) {
                s32 floor = map_world.unk1200c;

                if (obj->pos.vy < floor) {
                    goto fall;
                }
                obj->pos.vy = floor;
            } else {
                obj->pos.vy = map_world.unk12010;
            }
            obj->vel.vy = 0;
            goto idle;
        }
        if (obj->flags & 0x400) {
            goto fall;
        }
        goto idle;
    }
    case 0x20:
        obj->pos.vy += obj->vel.vy;
        obj->vel.vy += 5;
        if (map_world.unk12010 < obj->pos.vy && obj->vel.vy < 0) {
            return;
        }
        obj->pos.vy = map_world.unk12010;
    idle:
        obj->vertical_motion_state = 0;
        return;
    case 0x30: {
        s32 jump_y;
        s32 jump_hit;

        jump_y = obj->ballistic_origin_y - obj->ballistic_launch_speed_y * obj->vel.vy + ((obj->ballistic_acceleration * obj->vel.vy * obj->vel.vy) >> 1);
        jump_hit = collision_query_world(obj->pos.vx, jump_y, obj->pos.vz, obj->collision_radius,
                                         ACTOR_SHAPE(obj), actor_status.actor_collision_query_flags);
        if (jump_hit == 0) {
            obj->pos.vy = jump_y;
            obj->vel.vy++;
            obj->layer_mask = *wet == 0 ? 1 : 2;
            return;
        }
        if (jump_hit == 0x80) {
            player_apply_damage(0, def->landing_damage, 0, 0, 0, 0, 0, 0, 0, 0x1000, 10, &obj->pos);
        }
        obj->vertical_motion_state = 0x10;
        obj->vel.vy = 0;
        return;
    }
    case 0x60:
        break;
    }
}

/* Scales the current record's horizontal velocity toward speed, then moves it. */
ADDRESS(0x8004e6f8, 0x12c)
s32 actor_damp_horizontal_motion(s32 speed, s32 mode)
{
    Actor *obj;
    s32 length;

    obj = actor_status.current;
    if (obj->vertical_motion_state == 0) {
        length = SquareRoot0(obj->vel.vx * obj->vel.vx + obj->vel.vz * obj->vel.vz);
        if (length == 0) {
            return 0;
        }
        obj->vel.vx = value_approach(obj->vel.vx, 0, obj->vel.vx * speed * 2 / length);
        obj->vel.vz = value_approach(obj->vel.vz, 0, obj->vel.vz * speed * 2 / length);
    }
    return actor_move_horizontal_with_collision(&obj->vel, mode);
}

/* Accelerates the current record along angle, then moves it. */
ADDRESS(0x8004e824, 0xc8)
s32 actor_move_along_heading(s32 angle, s32 scale_x, s32 scale_z, s32 mode)
{
    Actor *obj;
    struct {
        s32 v[2];
    } target, step;

    obj = actor_status.current;
    angle_to_forward_xz(angle, target.v);
    step = target;
    vector2i_scale_shift11(scale_x, target.v);
    vector2i_scale_shift11(scale_z, step.v);
    obj->vel.vx = value_approach(obj->vel.vx, target.v[0], step.v[0]);
    obj->vel.vz = value_approach(obj->vel.vz, target.v[1], step.v[1]);
    return actor_move_horizontal_with_collision(&obj->vel, mode);
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
 * target arrives as an int (actor_update_behavior passes vector_xz_to_angle's result
 * and Actor.unk20 unconverted); retail narrows it to 16 bits for each
 * angle helper call.
 */
ADDRESS(0x8004e928, 0x154)
void actor_turn_toward_angle(Actor *obj, int target, s32 max, s32 accel)
{
    s32 yaw;

    if (angle_shortest_delta((s16)target, obj->angle.vy) != 0) {
        if (angle_mod_delta_le_half_turn(target, obj->angle.vy)) {
            obj->turn_rate += accel;
            if (obj->turn_rate > max) {
                obj->turn_rate = max;
            }
        } else {
            obj->turn_rate -= accel;
            if (obj->turn_rate < -max) {
                obj->turn_rate = -max;
            }
        }
        yaw = obj->angle.vy;
        obj->angle.vy += obj->turn_rate;
        if (obj->turn_rate > 0) {
            if (angle_signed_greater_than((s16)target, yaw)
                && !angle_signed_greater_than((s16)target, obj->angle.vy)) {
                obj->angle.vy = target;
            }
        } else {
            if (angle_signed_greater_than(yaw, (s16)target)
                && !angle_signed_greater_than(obj->angle.vy, (s16)target)) {
                obj->angle.vy = target;
            }
        }
    } else {
        obj->turn_rate = 0;
    }
}

ADDRESS(0x8004ea7c, 0x74)
/* Returns actor_move_along_heading's result (FDAT module slot 17 handlers test it). */
s32 actor_turn_and_move_along_heading(s16 target, s32 scale_x, s32 max, s32 scale_z, s32 accel, s32 mode)
{
    Actor *obj;

    obj = actor_status.current;
    actor_turn_toward_angle(obj, target, max, accel);
    return actor_move_along_heading(obj->angle.vy, scale_x, scale_z, mode);
}

/* Walks the current record toward (x, z); -1 when face is set, it is close and not facing. */
ADDRESS(0x8004eaf0, 0x118)
s32 actor_turn_and_move_toward_point(s32 x, s32 z, s32 scale_x, s32 max, s16 face, s32 scale_z, s32 accel,
                                     s32 mode)
{
    Actor *obj = actor_status.current;
    s32 dx = x - obj->pos.vx;
    s32 dz = z - obj->pos.vz;
    s32 angle = vector_xz_to_angle(dx, dz);
    s32 distance = abs(dx) + abs(dz);

    if (face != -1 && distance <= 600 && !angle_within_tolerance(angle, face, 0x320)) {
        return -1;
    }
    actor_turn_and_move_along_heading(angle, scale_x, max, scale_z, accel, mode);
    return angle & 0xfff;
}

/* Accelerates the current record along angle in 3D, then moves it and applies its fall. */
ADDRESS(0x8004ec08, 0x148)
s32 actor_move_along_euler_angles(SVECTOR *angle, s32 scale_a, s32 scale_b, s32 mode)
{
    Actor *obj;
    SVECTOR target;
    SVECTOR step;
    s32 r;
    s32 y;

    obj = actor_status.current;
    pitch_yaw_to_forward_vector(angle, &target);
    step = target;
    vector3s_scale_shift12(scale_a, &target);
    vector3s_scale_shift12(scale_b, &step);
    obj->vel.vx = value_approach(obj->vel.vx, target.vx, step.vx);
    obj->vel.vy = value_approach(obj->vel.vy, target.vy, step.vy);
    obj->vel.vz = value_approach(obj->vel.vz, target.vz, step.vz);
    r = actor_move_horizontal_with_collision(&obj->vel, mode) != 0;
    y = obj->vel.vy + obj->pos.vy;
    if (collision_query_world(obj->pos.vx, y, obj->pos.vz, obj->collision_radius, ACTOR_SHAPE(obj),
                              actor_status.actor_collision_query_flags) == 0) {
        obj->pos.vy = y;
    } else {
        r |= 2;
    }
    return r;
}

/* Returns actor_move_along_euler_angles's result (actor_update_behavior tests it). */
ADDRESS(0x8004ed50, 0x90)
s32 actor_turn_and_move_along_euler_angles(SVECTOR *angle, s32 scale_a, s32 max, s32 scale_b, s32 accel,
                                           s32 mode)
{
    Actor *obj;

    obj = actor_status.current;
    actor_turn_toward_angle(obj, angle->vy, max, accel);
    obj->angle.vx = angle_approach(obj->angle.vx, angle->vx, 8);
    return actor_move_along_euler_angles(&obj->angle, scale_a, scale_b, mode);
}

/* A vertex of obj's posed model relative to its position. */
ADDRESS(0x8004ede0, 0x100)
s32 actor_sample_rotated_animation_vertex(Actor *obj, s32 vertex, VECTOR *out)
{
    SVECTOR rotation;
    SVECTOR v;

    if (animation_sample_vertex(obj->definition_id + 128, obj->animation_id, obj->animation_phase, vertex,
                                &v)) {
        v.vx = 0;
        v.vy = -obj->collision_height >> 1;
        v.vz = -obj->collision_radius;
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

/* obj's aim point (Actor.unk28 bits 0-1: position, offset, rotated offset). */
ADDRESS(0x8004eee0, 0x128)
VECTOR *actor_resolve_group_position(Actor *obj, VECTOR *out)
{
    ActorGroup *def;

    switch (obj->flags & 3) {
    case 0:
        return &obj->pos;
    case 1:
        def = &actor_status.groups[obj->group_index];
        out->vx = obj->pos.vx + def->position_offset_x;
        out->vy = obj->pos.vy + def->position_offset_y;
        out->vz = obj->pos.vz + def->position_offset_z;
        return out;
    case 2:
        def = &actor_status.groups[obj->group_index];
        vector_rotate_yxz(&obj->angle, (SVECTOR *)&def->position_offset_x, out);
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
void actor_update_motion_animation(s32 idle, s32 left, s32 right, s32 up, s32 down, s32 speed)
{
    Actor *obj;
    s32 anim;
    s32 turn;

    obj = actor_status.current;
    anim = idle;
    turn = 0;
    if (obj->turn_rate > 0) {
        anim = right;
        turn = obj->turn_rate;
    } else if (obj->turn_rate < 0) {
        anim = left;
        turn = -obj->turn_rate;
    }
    if (obj->vel.vy > 10) {
        if (turn < obj->vel.vy) {
            anim = up;
        }
    } else if (obj->vel.vy < 10 && turn < -obj->vel.vy) {
        anim = down;
    }
    if (obj->animation_id == idle) {
        actor_advance_animation_wrapped(obj, speed);
        if (actor_animation_crossed_phase(obj, 0)) {
            actor_set_animation_if_changed(anim);
        }
    } else if (obj->animation_id == left) {
        if (obj->animation_phase < 2048) {
            actor_advance_animation_clamped(obj, speed);
        } else if (anim != left) {
            actor_advance_animation_clamped(obj, speed);
            if (obj->animation_phase >= 4095) {
                actor_set_animation(anim);
            }
        }
    } else if (obj->animation_id == right) {
        if (obj->animation_phase < 2048) {
            actor_advance_animation_clamped(obj, speed);
        } else if (anim != right) {
            actor_advance_animation_clamped(obj, speed);
            if (obj->animation_phase >= 4095) {
                actor_set_animation(anim);
            }
        }
    } else if (obj->animation_id == up) {
        if (obj->animation_phase < 2048) {
            actor_advance_animation_clamped(obj, speed);
        } else if (anim != up) {
            actor_advance_animation_clamped(obj, speed);
            if (obj->animation_phase >= 4095) {
                actor_set_animation(anim);
            }
        }
    } else if (obj->animation_id == down) {
        if (obj->animation_phase < 2048) {
            actor_advance_animation_clamped(obj, speed);
        } else if (anim != down) {
            actor_advance_animation_clamped(obj, speed);
            if (obj->animation_phase >= 4095) {
                actor_set_animation(anim);
            }
        }
    }
}

/* Aims from `from` at target, leading the player's motion over steps refinements. */
ADDRESS(0x8004f1d0, 0x244)
s32 actor_compute_target_direction(Actor *obj, VECTOR *from, s32 speed, VECTOR *target, SVECTOR *out,
                                   s32 pitch, s16 turn, s32 steps)
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
        vector_add_scaled_delta(from, &cur_avatar_status.unk11c,
                                fixed_vector3_length(target->vx - at.vx, target->vy - at.vy,
                                                     target->vz - at.vz) / speed, &at);
    }
    return angle.vy;
}
