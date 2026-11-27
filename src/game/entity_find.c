/*
 * GAME.EXE: Actor searches and animation time (WIP module): the
 * player-in-reach test, the best record in a cone, the first record a point
 * touches, the current-record pointers, and the animation time steps.
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

/*
 * Hits every active record within reach of pos (mode: 0x8000 sphere, 0x8001
 * sphere above its base, 0x8002 sphere outside the cone either side of the
 * yaw kept in pos->pad, otherwise vector_distance_to_point's own mode) with
 * actor_apply_magic_to_actor, its strength falling off to falloff at the edge.
 */
ADDRESS(0x8004ceac, 0x3a8)
void actor_apply_area_magic(VECTOR *pos, s32 near, s32 reach, s32 mode, u16 falloff, u16 a5, u16 a6, u16 a7,
                            u16 a8, u16 a9, u16 a10, u16 a11, u16 a12, u16 a13, u16 flags, u16 a15,
                            MagicRecord *rec)
{
    Actor *obj;
    VECTOR *target;
    s32 dist;
    s32 angle;
    s32 side;
    u16 scale;
    s16 i;

    target = pos;
    if (flags & 0x8000) {
        target = NULL;
    }
    flags &= 0x7fff;
    obj = actor_status.objects;
    for (i = 0; i < 200; i++, obj++) {
        if (obj->lifecycle != 1 || obj == actor_status.current) {
            continue;
        }
        if (mode == 0x8000) {
            dist = vector_distance_between_with_reach(pos, reach, &obj->pos, obj->collision_radius,
                                                      obj->collision_height);
        } else if (mode == 0x8001) {
            if (pos->vy < obj->pos.vy - obj->collision_height) {
                dist = -9999999;
            } else {
                dist = vector_distance_between_with_reach(pos, reach, &obj->pos, obj->collision_radius,
                                                          obj->collision_height);
            }
        } else if (mode == 0x8002) {
            dist = vector_distance_between_with_reach(pos, reach, &obj->pos, obj->collision_radius,
                                                      obj->collision_height);
            angle = (s16)ratan2(obj->pos.vz - pos->vz, obj->pos.vx - pos->vx);
            side = angle_within_tolerance(angle, pos->pad + 1024, 0x40);
            if (side == 1 && angle_within_tolerance(angle, pos->pad - 1024, 0x40) == side) {
                dist = -9999999;
            }
        } else {
            dist = vector_distance_to_point(&obj->pos, pos->vx, pos->vy, pos->vz, reach,
                                            obj->collision_height, mode);
            if (dist == -1) {
                dist = -9999999;
            }
        }
        if (dist < near) {
            continue;
        }
        if (falloff != 0x1000) {
            scale = (dist << 12) / reach;
            scale = (u32)(scale * (0x1000 - falloff)) >> 12;
            scale = 0x1000 - scale;
            scale = flags * scale >> 12;
        } else {
            scale = flags;
        }
        actor_apply_magic_to_actor(i, a5, a6, a7, a8, a9, a10, a11, a12, a13, scale, a15, rec, target);
    }
}

/* Fires (player_apply_damage) at the player when the player is within reach and the cone. */
ADDRESS(0x8004d254, 0x170)
s32 actor_try_damage_player_in_cone(s32 near, s32 range, s32 height, s32 cone, u16 a4, u16 a5, u16 a6, u16 a7)
{
    s32 offset = height << 5;
    Actor *obj = actor_status.current;
    VECTOR *target;
    VECTOR pos;
    s32 dist;
    s32 angle;

    near <<= 6;
    range <<= 6;
    cone <<= 4;
    pos.vx = obj->pos.vx;
    pos.vy = obj->pos.vy - offset;
    pos.vz = obj->pos.vz;
    dist = vector_distance_to_point(&pos, cur_avatar_status.pos.vx, cur_avatar_status.pos.vy,
                                    cur_avatar_status.pos.vz, range, 0, 0x6a4);
    if (dist == -1 || dist < near) {
        return 0;
    }

    target = &cur_avatar_status.pos;
    angle = vector_xz_to_angle(target->vx - pos.vx, target->vz - pos.vz);
    if (!angle_within_tolerance(obj->angle.vy, angle, cone)) {
        return 0;
    }

    player_apply_damage(a4, a5, a6, a7, 0, 0, 0, 0, 0, 0x1000, 10, &pos);
    return 1;
}

/* The active record best inside the cone from pos (yaw, pitch); its distance in *dist. */
ADDRESS(0x8004d3c4, 0x280)
Actor *actor_find_best_in_cone(VECTOR *pos, s16 yaw, s16 pitch, s32 range, s32 max_yaw, s32 max_pitch,
                               s32 *dist, s32 jitter)
{
    Actor *obj;
    Actor *best;
    SVECTOR angle;
    s32 best_score;
    s32 best_dist;
    s32 d;
    s32 r;
    s32 score;
    u16 i;

    best_score = 30000;
    best_dist = -1;
    best = NULL;
    obj = actor_status.objects;
    i = 200;
    while (i--) {
        if (obj->lifecycle == 1 && obj->target_type != 3 && obj != actor_status.current) {
            r = range;
            if (obj->flags & 0x20000) {
                r <<= 1;
            }
            d = vector_distance_to_point(&obj->pos, pos->vx, 0xffff, pos->vz, r, 0, 0);
            if (d != -1) {
                vector_displacement_to_pitch_yaw(obj->pos.vx - pos->vx, obj->pos.vy - pos->vy,
                                                 obj->pos.vz - pos->vz, &angle);
                angle.vy = (angle.vy - yaw) & 0xfff;
                if (angle.vy >= 2048) {
                    angle.vy = 0x1000 - angle.vy;
                }
                angle.vx = (angle.vx - pitch) & 0xfff;
                if (angle.vx >= 2048) {
                    angle.vx = 0x1000 - angle.vx;
                }
                if ((obj->flags & 0x20000) && jitter >= 0) {
                    d >>= 2;
                    angle.vx -= 512;
                    angle.vy -= 512;
                } else if (angle.vy > max_yaw || angle.vx > max_pitch) {
                    goto next;
                }
                score = angle.vy + angle.vx + (d >> 7);
                if (jitter > 0) {
                    score += (rand() * jitter) >> 15;
                }
                if (score < best_score) {
                    best_score = score;
                    best = obj;
                    best_dist = d;
                }
            }
        }
    next:
        obj++;
    }
    *dist = best_dist;
    return best;
}

/* The first active record (not current) whose reach holds the point; -1 for none. */
ADDRESS(0x8004d644, 0x1f4)
s32 actor_find_overlap_excluding_target_type3(s32 x, s32 y, s32 z, s32 radius, s32 reach_up)
{
    Actor *obj;
    ActorGroup *def;
    VECTOR pos;
    s32 i;

    obj = actor_status.objects;
    for (i = 0; i < 200; i++, obj++) {
        if (obj->lifecycle != 1) {
            continue;
        }
        if (obj->target_type == 3) {
            continue;
        }
        if (actor_status.actor_overlap_exclusion_flags & obj->flags) {
            continue;
        }
        if (obj == actor_status.current) {
            continue;
        }
        if (obj->flags & 0x10) {
            if (obj->unk22 == actor_status.current_index) {
                continue;
            }
            pos.vx = obj->pos.vx;
            pos.vz = obj->pos.vz;
            pos.vy = obj->vertical_anchor_offset + obj->pos.vy;
            if (vector_distance_to_point(&pos, x, y, z, obj->collision_radius + radius, obj->collision_height,
                                         reach_up) != -1) {
                return i;
            }
        } else if (obj->flags & 0x400000) {
            def = &actor_status.groups[obj->group_index];
            actor_sample_rotated_animation_vertex(obj, def->definition_id == 0x6a ? 0x46 : 0, &pos);
            pos.vx += obj->pos.vx;
            pos.vz += obj->pos.vz;
            pos.vy += obj->pos.vy;
            if (vector_distance_to_point(&pos, x, y, z, obj->collision_radius + radius, obj->collision_height,
                                         reach_up) != -1) {
                return i;
            }
        } else if (vector_distance_to_point(&obj->pos, x, y, z, obj->collision_radius + radius,
                                            obj->collision_height, reach_up) != -1) {
            return i;
        }
    }
    return -1;
}

/* As actor_find_overlap_excluding_target_type3, for every active record that is not of kind 0x6e. */
ADDRESS(0x8004d838, 0x1f4)
s32 actor_find_overlap(s32 x, s32 y, s32 z, s32 radius, s32 reach_up)
{
    Actor *obj;
    ActorGroup *def;
    VECTOR pos;
    s32 i;

    obj = actor_status.objects;
    for (i = 0; i < 200; i++, obj++) {
        if (obj->lifecycle != 1) {
            continue;
        }
        if (actor_status.actor_overlap_exclusion_flags & obj->flags) {
            continue;
        }
        if (obj->definition_id == 0x6e) {
            continue;
        }
        if (obj == actor_status.current) {
            continue;
        }
        if (obj->flags & 0x10) {
            if (obj->unk22 == actor_status.current_index) {
                continue;
            }
            pos.vx = obj->pos.vx;
            pos.vz = obj->pos.vz;
            pos.vy = obj->vertical_anchor_offset + obj->pos.vy;
            if (vector_distance_to_point(&pos, x, y, z, obj->collision_radius + radius, obj->collision_height,
                                         reach_up) != -1) {
                return i;
            }
        } else if (obj->flags & 0x400000) {
            def = &actor_status.groups[obj->group_index];
            actor_sample_rotated_animation_vertex(obj, def->definition_id == 0x6a ? 0x46 : 0, &pos);
            pos.vx += obj->pos.vx;
            pos.vz += obj->pos.vz;
            pos.vy += obj->pos.vy;
            if (vector_distance_to_point(&pos, x, y, z, obj->collision_radius + radius, obj->collision_height,
                                         reach_up) != -1) {
                return i;
            }
        } else if (vector_distance_to_point(&obj->pos, x, y, z, obj->collision_radius + radius,
                                            obj->collision_height, reach_up) != -1) {
            return i;
        }
    }
    return -1;
}

/* Makes obj the current record (0 clears the pointers). */
ADDRESS(0x8004da2c, 0xdc)
void actor_bind_current(Actor *obj)
{
    if (obj != NULL) {
        actor_status.current = obj;
        actor_status.current_group = &actor_status.groups[obj->group_index];
        actor_status.current_group_index = obj->group_index;
        if (actor_status.current_group->initial_actor_flags & 0x10) {
            actor_status.other_actor = &actor_status.objects[obj->unk22];
            actor_status.other_group = &actor_status.groups[actor_status.other_actor->group_index];
        }
    } else {
        actor_status.current = NULL;
        actor_status.current_group = NULL;
        actor_status.current_group_index = 0;
        actor_status.other_actor = NULL;
        actor_status.other_group = NULL;
    }
}

/* Steps the animation time, wrapping. */
ADDRESS(0x8004db08, 0x34)
void actor_advance_animation_wrapped(Actor *obj, s16 step)
{
    if (step < 0) {
        obj->animation_step = -step;
    } else {
        obj->animation_step = step;
    }
    obj->animation_phase = (step + obj->animation_phase) & 0xfff;
}

/* Steps the animation time, holding at either end. */
ADDRESS(0x8004db3c, 0x5c)
void actor_advance_animation_clamped(Actor *obj, s16 step)
{
    s16 time;

    if (step < 0) {
        obj->animation_step = -step;
    } else {
        obj->animation_step = step;
    }
    obj->animation_phase = time = step + obj->animation_phase;
    if (time >= 0x1000) {
        obj->animation_phase = 0xfff;
    } else if (time < 0) {
        obj->animation_phase = 0;
    }
}

/* Whether the last time step passed time. */
ADDRESS(0x8004db98, 0x30)
s32 actor_animation_crossed_phase(Actor *obj, u16 time)
{
    s32 passed;

    return time < obj->animation_phase && time >= obj->animation_phase - obj->animation_step;
}

/*
 * Moves the current record horizontally by vel, sliding along walls and
 * turning off slopes as mode allows (bits 0-1 slide, 2 stop at a drop, 3
 * write the final step back, 4 and 5 more wall rules). Returns the hit bits.
 */
ADDRESS(0x8004dbc8, 0x4cc)
s32 actor_move_horizontal_with_collision(SVECTOR *vel, s32 mode)
{
    Actor *obj = actor_status.current;
    s32 dx;
    s32 dz;
    s32 dx0;
    s32 dz0;
    VECTOR p;
    s32 hit;
    s32 slid;
    s32 cornered;
    s32 turns;
    s32 hits = 0;

    if (vel->vx == 0 && vel->vz == 0) {
        return 0;
    }
    turns = 0;
    cornered = 0;
    slid = 0;
    dx0 = dx = vel->vx;
    dz0 = dz = vel->vz;

retry:
    p.vx = obj->pos.vx + dx;
    p.vz = obj->pos.vz + dz;
    hit = collision_query_world(p.vx, obj->pos.vy, p.vz, obj->collision_radius, ACTOR_SHAPE(obj),
                                actor_status.actor_collision_query_flags);
    if (hit == 0) {
    clear:
        if (obj->vertical_motion_state == 0 && (mode & 0x24)) {
            s32 floor = map_world.collision_floor_height;
            s32 step_x;
            s32 step_z;

            if (dx == 0) {
                step_x = 0;
            } else if (dx > 0) {
                step_x = obj->collision_radius * 2;
            } else {
                step_x = -obj->collision_radius * 2;
            }
            if (dz == 0) {
                step_z = 0;
            } else if (dz > 0) {
                step_z = obj->collision_radius * 2;
            } else {
                step_z = -obj->collision_radius * 2;
            }
            collision_query_shapes_with_layer_sample(obj->pos.vx, obj->pos.vy, obj->pos.vz + step_z,
                                                     obj->collision_radius, ACTOR_SHAPE(obj));
            if (floor < map_world.collision_floor_height) {
                floor = map_world.collision_floor_height;
            }
            collision_query_shapes_with_layer_sample(obj->pos.vx + step_x, obj->pos.vy, obj->pos.vz,
                                                     obj->collision_radius, ACTOR_SHAPE(obj));
            if (floor < map_world.collision_floor_height) {
                floor = map_world.collision_floor_height;
            }
            if (obj->pos.vy < floor - 1100) {
                hits |= 0x100;
                if (mode & 4) {
                    goto blocked;
                }
            }
        }
        obj->pos.vx = p.vx;
        obj->pos.vz = p.vz;
        goto finish;
    }

    hits |= hit;
    if (!(mode & 3)) {
        goto finish;
    }
    if (hit & 0xb0) {
        s32 angle;
        s32 heading;
        s32 len;

        turns++;
        if (turns == 2) {
            goto finish;
        }
        collision_cache_load_hit_bounds();
        angle = vector_xz_to_angle(map_world.collision_hit_position.vx - obj->pos.vx,
                                   map_world.collision_hit_position.vz - obj->pos.vz);
        heading = vector_xz_to_angle(dx, dz);
        if (angle_mod_delta_le_half_turn(heading, angle)) {
            angle = (angle + 1024) & 0xfff;
        } else {
            angle = (angle - 1024) & 0xfff;
        }
        len = SquareRoot0(dx * dx + dz * dz);
        dx = -(rsin(angle) * len) >> 13;
        dz = (rcos(angle) * len) >> 13;
        goto retry;
    }
    if (!(mode & 0x10)) {
        if (obj->flags & 0x4000) {
            goto blocked;
        }
        if (!(hit & ~0x205)) {
            if (obj->pos.vy <= map_world.collision_floor_height + 800) {
                goto clear;
            }
            goto blocked;
        }
    }
    if (!(hit & 1)) {
        goto diagonal;
    }

blocked:
    if (slid) {
        dz = 0;
        dx = 0;
        goto finish;
    }
    if (dx != 0) {
        dx = 0;
        if (mode & 1) {
            s32 len = SquareRoot0(dx0 * dx0 + dz0 * dz0);

            dz = len;
            if (dz0 <= 0) {
                dz = -dz;
            }
        } else {
            dz = dz0;
        }
        goto retry;
    }
    if (dz != 0) {
        dz = 0;
        if (mode & 1) {
            s32 len = SquareRoot0(dx0 * dx0 + dz0 * dz0);

            dx = len;
            if (dx0 <= 0) {
                dx = -dx;
            }
        } else {
            dz = dz0;
        }
        slid = 1;
        goto retry;
    }
diagonal:
    if (!(hit & 2)) {
        goto finish;
    }
    if (cornered) {
        goto blocked;
    }
    switch (map_world.collision_layer->quarter_turns & 3) {
    case 0:
    case 2:
        dx = (dx0 + dz0) >> 1;
        dz = dx;
        break;
    default:
        dx = (dx0 - dz0) >> 1;
        dz = -dx;
        break;
    }
    cornered = 1;
    goto retry;

finish:
    if (mode & 8) {
        vel->vx = dx;
        vel->vz = dz;
    }
    return hits;
}
