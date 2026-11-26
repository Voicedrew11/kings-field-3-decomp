/*
 * GAME.EXE: Effect movement (WIP module): steering and speed limits of
 * the current record, spawning further records from it, its step and
 * collision test, and effect_update_dispatch, the per-frame update of the current
 * record by its kind (Effect.kind).
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

/* Zero velocity for the records effect_spawn_zero_direction spawns. */
DATA(0x8009c33c, 0x8, ".sdata")
SVECTOR effect_zero_direction = {0, 0, 0, 0};

/* dir's x and z scaled by scale and turned by yaw into out. */
ADDRESS(0x80055b4c, 0x8c)
void effect_rotate_scale_offset_y(SVECTOR *dir, VECTOR *out, s32 yaw, s32 scale)
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
s32 effect_move_probe(s32 scale, s32 max, s32 radius, s32 height, SVECTOR *accel)
{
    Effect *obj;
    s32 speed;

    obj = effect_status.current;
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
    if (effect_probe_collision_by_type(&obj->pos, radius, height) != 0) {
        return -1;
    }
    return 0;
}

/* Homing: turns the current record toward an Actor record (type bit 6) or the player, then
 * effect_move_probe. */
ADDRESS(0x80055db0, 0x1c4)
s32 effect_aim_and_move(s32 max, s32 scale, s32 turn, s32 radius, s32 height, s32 range, s32 near_scale,
                        s32 cone)
{
    Effect *obj;
    VECTOR target;
    SVECTOR dir;
    SVECTOR accel;
    s32 dist;
    Actor *found;

    obj = effect_status.current;
    if (obj->type & 0x40) {
        found = actor_find_best_in_cone(&obj->pos, obj->angle.vy, obj->angle.vx, 24000, cone, cone, &dist, 0);
        if (found != NULL) {
            target.vx = found->pos.vx;
            target.vy = found->pos.vy - (found->collision_height >> 1);
            target.vz = found->pos.vz;
        aim:
            vector_displacement_to_pitch_yaw(target.vx - obj->pos.vx, target.vy - obj->pos.vy,
                                             target.vz - obj->pos.vz, &dir);
            obj->angle.vx = angle_approach(obj->angle.vx, dir.vx, turn);
            obj->angle.vy = angle_approach(obj->angle.vy, dir.vy, turn);
        }
    } else {
        dist = player_distance_to_point_in_cone(&obj->pos, obj->angle.vy, 24000, 0x1000);
        if (dist != -1) {
            target.vx = cur_avatar_status.pos.vx;
            target.vy = cur_avatar_status.pos.vy - 1600;
            target.vz = cur_avatar_status.pos.vz;
            goto aim;
        }
    }
    pitch_yaw_to_forward_vector(&obj->angle, &accel);
    if (dist >= 0 && dist <= range) {
        scale = near_scale;
    }
    return effect_move_probe(scale, max, radius, height, &accel);
}

/* Steers the current record straight at target; -2 once within near, braking within slow. */
ADDRESS(0x80055f74, 0x1c0)
s32 effect_target_motion(VECTOR *target, s32 max, s32 scale, s32 slow, s32 near, s32 radius, s32 height)
{
    Effect *obj;
    VECTOR d;
    SVECTOR accel;
    s32 dist;

    obj = effect_status.current;
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
    return effect_move_probe(scale, max, radius, height, &accel);
}

/* Grows the current record by grow, hits around it, frees it at limit. */
ADDRESS(0x80056134, 0xa4)
void effect_scale_step(s32 rate, s32 limit, s32 grow, s32 falloff, s32 mode)
{
    Effect *obj;
    s32 r;

    obj = effect_status.current;
    obj->scale.vy = obj->scale.vz = obj->scale.vx += grow;
    r = rate * obj->scale.vx >> 12;
    effect_apply_radial_magic_damage(&obj->pos, r - (rate * grow >> 12), r - 1, mode, falloff, 0x1000);
    if (obj->scale.vx >= limit) {
        obj->type = EFFECT_FREE;
    }
}

/* Spawns record 0x6e at vertex of obj, yaw jittered. */
ADDRESS(0x800561d8, 0x90)
void effect_spawn_zero_direction(Effect *obj, s32 vertex)
{
    VECTOR pos;
    SVECTOR angle;
    Effect *made;

    effect_sample_world_vertex(obj, vertex, &pos, &obj->scale);
    angle.vx = obj->angle.vx;
    angle.vy = obj->angle.vy + (rand() >> 7) - 128;
    angle.vz = obj->angle.vz;
    made = effect_construct_record(10, obj->type, 0x6e, &pos, &effect_zero_direction, &angle);
    if (made != NULL) {
        made->phase = 2;
    }
}

/* Spawns record 0x70 on the floor below pos if pos is no more than height above it. */
ADDRESS(0x80056268, 0x88)
s32 effect_spawn_at_lower_bound(VECTOR *pos, s32 a1, s32 a2, s32 height)
{
    VECTOR p;
    SVECTOR v;
    s32 floor;

    floor = map_world.collision_bounds[0];
    if (pos->vy < floor) {
        return 0;
    }
    if (height + floor < pos->vy) {
        return 0;
    }
    p.vx = pos->vx;
    p.vz = pos->vz;
    p.vy = floor;
    effect_construct_record(10, 0, 0x70, &p, &v, a1, a2);
    return 1;
}

/*
 * Spawns record 0x6f at vertex of obj (-1: its position). mode picks the
 * velocity and how many of the arguments after a7 it reads: -2 three random
 * ranges and bases, -1 a speed and a vertex and scale, -3 a speed, other
 * values are a speed backwards along the record's angle.
 */
ADDRESS(0x800562f0, 0x290)
void effect_spawn_motion(Effect *obj, s32 vertex, s32 mode, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7)
{
    s32 *args;
    SVECTOR dir;
    VECTOR off;
    VECTOR p;

    args = &a7;
    if (vertex == -1) {
        off.vx = off.vy = off.vz = 0;
    } else {
        effect_sample_rotated_vertex(obj, vertex, &off, &obj->scale);
    }
    switch (mode) {
    case -1:
        effect_sample_rotated_vertex(obj, args[2], &p, (SVECTOR *)args[3]);
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
    effect_construct_record(10, 0, 0x6f, &off, &dir, a3, a4, a5, a6, a7, 1, 0);
}

/* Spawns count records 0x70 on the floor around pos, scattered by spread. */
ADDRESS(0x80056580, 0x154)
s32 effect_scatter_lower_bound(VECTOR *pos, s32 count, s32 spread, s32 a3, s32 a4, s32 range)
{
    VECTOR p;
    SVECTOR v;
    s32 dx;
    s32 dz;
    s32 r;
    s32 floor;

    dx = 0;
    dz = 0;
    floor = map_world.collision_bounds[0];
    if (pos->vy >= floor && floor + 500 >= pos->vy) {
        while (--count != -1) {
            p.vx = dx + pos->vx;
            p.vz = dz + pos->vz;
            p.vy = map_world.collision_bounds[0];
            r = random_centered_triangular_scaled(range) + 4096;
            effect_construct_record(10, 0, 0x70, &p, &v, a3 * r >> 12, a4 * r >> 12);
            dx = (rand() * spread >> 14) - spread;
            dz = (rand() * spread >> 14) - spread;
        }
        return 1;
    }
    return 0;
}

/* Moves the current record one step, tests it (also half a step back when midpoint_collision_enabled), spins
 * it. */
ADDRESS(0x800566d4, 0x12c)
s32 effect_collision_step(s32 radius, s32 height, s32 spin)
{
    Effect *obj;
    VECTOR p;
    s32 hit;

    obj = effect_status.current;
    obj->pos.vx += obj->vel.vx;
    obj->pos.vy += obj->vel.vy;
    obj->pos.vz += obj->vel.vz;
    hit = effect_probe_collision_by_type(&obj->pos, radius, height);
    if (obj->midpoint_collision_enabled != 0 && hit == 0) {
        p.vx = obj->pos.vx - (obj->vel.vx >> 1);
        p.vy = obj->pos.vy - (obj->vel.vy >> 1);
        p.vz = obj->pos.vz - (obj->vel.vz >> 1);
        hit = effect_probe_collision_by_type(&p, radius, height);
    }
    obj->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
    obj->angle.vz = (obj->angle.vz - spin) & 0xfff;
    return hit;
}

/* Moves the current record one step back. */
ADDRESS(0x80056800, 0x40)
void effect_collision_backtrack(void)
{
    Effect *obj;

    obj = effect_status.current;
    obj->pos.vx -= obj->vel.vx;
    obj->pos.vy -= obj->vel.vy;
    obj->pos.vz -= obj->vel.vz;
}

/* Moves the current record half a step back. */
ADDRESS(0x80056840, 0x7c)
void effect_collision_backtrack_half(void)
{
    Effect *obj;

    obj = effect_status.current;
    obj->pos.vx -= obj->vel.vx / 2;
    obj->pos.vy -= obj->vel.vy / 2;
    obj->pos.vz -= obj->vel.vz / 2;
}

/*
 * Per-frame update of the current Effect record (effect_status.current),
 * switched on its kind; obj->phase is the phase of the multi-step kinds. Most
 * kinds move the record, test it for hits and leave a particle trail through
 * effect_construct_record; a record is freed by setting type to EFFECT_FREE.
 *
 * Stack frame: a case block opened at a case label stays open to the end
 * of the switch (the closing braces at the bottom), so its locals keep
 * their slots for the rest of the switch while locals of inner blocks are
 * freed and reused (gcc temp-slot levels; locals only passed as stack
 * arguments are otherwise freed with their block). The spare* locals are
 * never referenced; they keep the retail slot layout.
 */
ADDRESS(0x800568bc, 0x5338)
void effect_update_dispatch(void)
{
    Effect *obj;
    s32 id;
    s32 phase;
    s32 hit;
    s32 i;
    VECTOR p;
    VECTOR spare;

    obj = effect_status.current;
    id = obj->kind;
    phase = obj->phase;
    switch (id) {
    case 0:
    case 40:
    case 62:
    case0:
        if (phase == 0) {
            hit = effect_collision_step(180, 0x80000000, -300);
            if (hit == 0) {
                break;
            }
            effect_scatter_lower_bound(&obj->pos, 3, 400, 0x2000, 0x2000, 0x400);
            /* XOR, not a square: the retail code takes the root of dx ^ 2. */
            if (SquareRoot0((obj->pos.vx - cur_avatar_status.pos.vx) ^ 2) < 1024U) {
                effect_collision_backtrack_half();
                goto hit86b;
            }
            goto hit86;
        }
        if (phase < 3) {
            goto grow86;
        }
        obj->type = EFFECT_FREE;
        break;
    case 3:
        obj->scale.vy += 1200;
        obj->animation_id = (obj->updates_remaining & 1) - 128;
        obj->clut_offset = (obj->updates_remaining % 2) << 6;
        break;
    case 6:
    case 51:
        if (obj->arg.b.unk44 == 2) {
            obj->lighting_blend += 256;
            if (obj->lighting_blend >= 0x1000) {
                obj->type = EFFECT_FREE;
            }
            break;
        }
        if (obj->arg.b.unk44 == 1) {
            obj->vel.vy += 13;
        }
        obj->angle.vx += 200;
        hit = effect_collision_step(500, 500, 250);
        if (hit != 0) {
            effect_apply_current_magic_backstep(hit);
            if (obj->arg.b.unk44 == 1) {
                obj->arg.b.unk44 = 2;
                obj->render_mode = 5;
                obj->lighting_override_index = 0x32;
                obj->lighting_blend = 0x400;
                break;
            }
            effect_collision_backtrack();
            obj->arg.b.unk44 = 1;
            obj->vel.vz = 0;
            obj->vel.vx = 0;
            obj->vel.vy = -100;
        }
        effect_spawn_at_lower_bound(&obj->pos, 0x4000, 0x4000, 500);
        break;
    case 15:
        {
            SVECTOR v;
            VECTOR at;

            hit = effect_collision_step(180, 0, 0);
            if (hit != 0) {
                if (hit & 3) {
                    effect_collision_backtrack();
                    obj->type = EFFECT_FREE;
                }
                if (obj->arg.b.unk44 == 0) {
                    obj->arg.b.unk44 = 1;
                    effect_apply_current_magic_backstep(hit);
                }
            } else {
                obj->arg.b.unk44 = 0;
            }
            at.vx = v.vx + obj->pos.vx;
            at.vy = v.vy + obj->pos.vy;
            at.vz = v.vz + obj->pos.vz;
            effect_apply_radial_magic_damage(&at, 0, obj->scale.vx * 512 >> 12, obj->scale.vy * 2048 >> 12,
                                             0x400, 0x1000);
            v.vx = rand() * 512 / 32767 - 256;
            v.vy = rand() * -256 / 32767;
            v.vz = rand() * 512 / 32767 - 256;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 9999, -128, 5, 22, 0, 1, 1);
            obj->angle.vy += 350;
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            break;
    case 75:
        {
            obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
            obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
            obj->angle.vz += 200;
            hit = effect_collision_step(180, 0, 0);
            if (hit != 0) {
                if (hit & 0xf) {
                    obj->type = EFFECT_FREE;
                } else {
                    effect_apply_current_magic_backstep(hit);
                    obj->type = EFFECT_FREE;
                }
                effect_play_spatial_sound(obj, 0x27);
                effect_construct_record(10, 3, 0x58, &obj->pos, NULL, 0, 2);
                effect_construct_record(10, 3, 0x58, &obj->pos, NULL, 0x40, 2);
                effect_construct_record(10, 3, 0x58, &obj->pos, NULL, 0x80, 2);
            }
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            {
                SVECTOR v;

                v.vx = v.vy = v.vz = 0;
                if (obj->updates_remaining % 3 == 0) {
                    effect_construct_record(10, 0, 3, &obj->pos, &v);
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
        if (obj->updates_remaining > 32) {
            obj->pos.vy -= 40;
        } else {
            obj->pos.vy += 85;
        }
        obj->type = 0x81;
        effect_apply_radial_magic_damage(&obj->pos, 0, obj->scale.vx, 0x8000, 0x400, 0x1000);
        obj->type = 0x84;
        if (effect_collision_step(100, 200, 0) != 0) {
            obj->type = EFFECT_FREE;
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
            hit = effect_probe_collision_by_type(&p, 20, 20);
            if (hit == 0) {
                q.vx = (obj->vel.vx >> 1) + obj->pos.vx;
                q.vy = obj->pos.vy;
                q.vz = (obj->vel.vz >> 1) + obj->pos.vz;
                hit = effect_probe_collision_by_type(&q, 20, 20);
                if (hit == 0) {
                    q.vx = (obj->vel.vx >> 2) + obj->pos.vx;
                    q.vy = obj->pos.vy;
                    q.vz = (obj->vel.vz >> 2) + obj->pos.vz;
                    hit = effect_probe_collision_by_type(&q, 20, 20);
                }
            }
            obj->pos.vx = p.vx;
            obj->pos.vy = p.vy;
            obj->pos.vz = p.vz;
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            if (hit != 0) {
                effect_apply_current_magic_backstep(hit | 0x20000);
                obj->type = EFFECT_FREE;
                break;
            }
            obj->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
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
                effect_aim_and_move(300, 10, 0x100, 20, 360, 0x2000, 0x30, 0x800);
            }
            if (obj->arg.s.unk46 == 0) {
                audio_play_spatial_range(4, &obj->pos, 0x6e, 0x6d60, 0x7148, 0);
            }
            t = ++obj->arg.s.unk46;
            y = obj->pos.vy;
            np.vy = obj->vel.vy + y + g * (t >> 1);
            np.vx = obj->vel.vx + obj->pos.vx;
            np.vz = obj->vel.vz + obj->pos.vz;
            hit = effect_probe_collision_by_type(&np, 20, 20);
            if (hit == 0) {
                q.vx = (np.vx + obj->pos.vx) >> 1;
                q.vy = (np.vy + obj->pos.vy) >> 1;
                q.vz = (np.vz + obj->pos.vz) >> 1;
                hit = effect_probe_collision_by_type(&q, 20, 20);
            }
            obj->pos.vx = np.vx;
            obj->pos.vy = np.vy;
            obj->pos.vz = np.vz;
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            if (hit != 0) {
                effect_apply_current_magic_backstep(hit | 0x20000);
                obj->type = EFFECT_FREE;
                break;
            }
            obj->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
            vector_displacement_to_pitch_yaw(obj->vel.vx, obj->pos.vy - y, obj->vel.vz, &obj->angle);
            break;
    case 86:
        if (phase == 0) {
            hit = effect_collision_step(180, 0, -300);
            if (hit == 0) {
                effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
                break;
            }
        hit86:
            effect_collision_backtrack();
        hit86b:
            effect_apply_current_magic_backstep(hit);
            obj->phase = 1;
        } else if (phase >= 3) {
            obj->type = EFFECT_FREE;
            break;
        }
    grow86:
        obj->animation_id = phase - 128;
        obj->scale.vz = obj->scale.vy = obj->scale.vx += 2048;
        obj->phase++;
        break;
    case 85:
        if (phase == 9) {
            EffectTrailRow spare2;
            VECTOR a;
            VECTOR c;
            VECTOR old;
            Actor *o;
            VECTOR *b;
            VECTOR *pos;

            o = &actor_status.objects[obj->arg.s.unk44];
            if (o->lifecycle != 1) {
                obj->type = EFFECT_FREE;
                break;
            }
            if (o->target_type != 0x19) {
                obj->type = EFFECT_FREE;
                break;
            }
            actor_sample_rotated_animation_vertex(o, obj->arg.s.unk46, &a);
            b = actor_resolve_group_position(o, &c);
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
            effect_spawn_motion(obj, -1, -700, obj->scale.vx, -300, 3, phase, 0);
            if (o->animation_phase < obj->arg.h.unk48) {
                break;
            }
            effect_play_spatial_sound(obj, 0x33);
            actor_compute_target_direction(o, &cur_avatar_status.pos, 650, pos, &obj->vel, -1, 0x400, 5);
            obj->phase = 0;
            obj->updates_remaining = 50;
            break;
        }
        effect_spawn_motion(obj, -1, 0x100, obj->scale.vx, -300, 2, 9, 0);
        goto case0;
    case 13:
    case 56:
        hit = effect_collision_step(180, 0x80000000, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->type = EFFECT_FREE;
            }
            if (obj->arg.b.unk44 == 0) {
                obj->arg.b.unk44 = 1;
                effect_apply_current_magic_backstep(hit);
            }
        } else {
            obj->arg.b.unk44 = 0;
        }
        effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
        obj->angle.vy += 700;
        break;
    case 52:
        hit = effect_collision_step(180, 0x80000000, 0);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                effect_apply_current_magic_backstep(hit);
            }
            obj->type = EFFECT_FREE;
        }
        effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
        obj->angle.vy += 300;
        break;
    case 76:
        {
            SVECTOR v;

            obj->angle.vy += 200;
            obj->angle.vx += 200;
            obj->vel.vy += 5;
            v.vx = v.vy = v.vz = 0;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0xc00, -256, 8, 0x1c, 0, 1, 2);
            hit = effect_collision_step(180, 0x80000000, 0);
            if (hit != 0) {
                obj->type = EFFECT_FREE;
                effect_apply_current_magic_backstep(hit);
            }
            break;
    case 64:
        hit = effect_collision_step(180, 0x80000000, -500);
        if (hit != 0) {
            SVECTOR v;

            obj->type = EFFECT_FREE;
            effect_apply_current_magic_backstep(hit);
            for (i = 15; i != -1; i--) {
                v.vx = (rand() * 500 >> 15) - 250;
                v.vy = (rand() * 500 >> 15) - 250;
                v.vz = (rand() * 500 >> 15) - 250;
                effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1400, -24, 10, 0x26, 0, 1, 2);
            }
        }
        break;
    case 70:
        {
            SVECTOR v;

            obj->angle.vz += 400;
            obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
            obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
            if (obj->updates_remaining == 45 && obj->arg.s.unk46 == 0) {
                v.vx = (-obj->vel.vz + obj->vel.vx + obj->vel.vx) >> 1;
                v.vz = (obj->vel.vx + obj->vel.vz + obj->vel.vz) >> 1;
                v.vy = obj->vel.vy;
                effect_construct_record(10, 0x43, 0x46, &obj->pos, &v, 1);
                v.vx = (obj->vel.vz + obj->vel.vx + obj->vel.vx) >> 1;
                v.vz = (-obj->vel.vx + obj->vel.vz + obj->vel.vz) >> 1;
                v.vy = obj->vel.vy;
                effect_construct_record(10, 0x43, 0x46, &obj->pos, &v, 1);
            }
            hit = effect_collision_step(180, 0x80000000, 0);
            if (hit != 0) {
                effect_apply_current_magic_backstep(hit);
                obj->kind = 0x58;
                obj->render_flags = 1;
                obj->render_mode = 5;
                obj->base_render_id = 0x15;
                obj->render_id = 0x15;
                obj->lighting_override_index = 0x34;
                obj->lighting_blend = 0x1000;
                obj->updates_remaining = 40;
                obj->animation_id = 0;
                obj->scale.vx = 0;
                obj->scale.vy = 0;
                obj->scale.vz = 0;
                obj->angle.vx = 0;
                obj->angle.vy = 0;
                obj->angle.vz = 0;
                obj->clut_offset = 0x40;
                effect_play_spatial_sound(obj, 0x28);
                effect_construct_record(10, 3, 0x58, &obj->pos, NULL, 0x80, 1);
                break;
            }
            v.vx = (rand() * 20 >> 15) - 10;
            v.vy = (rand() * 20 >> 15) - 10;
            v.vz = (rand() * 20 >> 15) - 10;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1000, -640, 5, 12, 0, 1, 0);
            break;
    case 44:
        {
            SVECTOR v;

            obj->angle.vz += 400;
            obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
            obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
            hit = effect_collision_step(180, 0x80000000, 0);
            if (hit != 0) {
                effect_apply_current_magic_backstep(hit);
                obj->type = EFFECT_FREE;
            }
            v.vx = (rand() * 20 >> 15) - 10;
            v.vy = (rand() * 20 >> 15) - 10;
            v.vz = (rand() * 20 >> 15) - 10;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1400, -256, 4, 0x16, 0, 2, 5);
            break;
    case 49:
    case 54:
    case 55:
        {
            SVECTOR v;

            hit = effect_collision_step(180, 0x80000000, 0);
            if (hit != 0) {
                if (hit & 0xf) {
                    obj->type = EFFECT_FREE;
                }
                if (obj->arg.b.unk44 == 0) {
                    obj->arg.b.unk44 = 1;
                    effect_apply_current_magic_backstep(hit);
                    for (i = 6; i != -1; i--) {
                        v.vx = (rand() * 240 >> 15) - 120;
                        v.vy = (rand() * 240 >> 15) - 120;
                        v.vz = (rand() * 240 >> 15) - 120;
                        effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1400, 0, 7, 12, 0, 1, 1);
                    }
                }
            } else {
                obj->arg.b.unk44 = 0;
            }
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            break;
    case 60:
        switch (obj->arg.s.unk44) {
        case 0:
            if (effect_collision_step(180, 0x80000000, 0) != 0) {
                obj->arg.s.unk44++;
            }
            obj->vel.vy += 5;
            break;
        case 1:
            obj->scale.vx += 256;
            obj->scale.vz += 256;
            if (obj->scale.vx > 0x1000) {
                obj->type = EFFECT_FREE;
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
            effect_apply_radial_magic_damage(&p, (obj->scale.vx - obj->arg.s.unk46) * 4,
                                             obj->scale.vx * 4 - 1, 2000, 0x1000, obj->arg.s.unk48);
        }
        if (obj->scale.vx > obj->arg.s.unk44) {
            obj->type = EFFECT_FREE;
        }
        break;
    case 12:
        if (obj->arg.s.unk44 == 0) {
            collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x400, obj->scale.vx, 0x30);
            if (map_world.collision_hit_flags == 0x10) {
                effect_apply_current_magic(0x10010, 5000, NULL);
            }
            obj->arg.s.unk44++;
            effect_construct_record(10, 0, 0x65, &obj->pos, NULL, map_world.collision_actor_index);
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
                    Actor *t;

                    t = &actor_status.objects[obj->arg.s.unk46];
                    t->flags &= ~0x800;
                }
            }
            if (obj->arg.s.unk44 >= 12 && obj->arg.s.unk44 < 48) {
                v.vx = rand() * 512 / 32767 - 256;
                v.vy = rand() * -256 / 32767;
                v.vz = rand() * 512 / 32767 - 256;
                effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1000, -128, 5, 12, 0, 1, 0);
            }
            obj->angle.vy -= 400;
            obj->arg.s.unk44++;
            break;
    case 102:
        hit = effect_collision_step(180, 0, -600);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                effect_apply_current_magic_backstep(hit);
            }
            obj->type = EFFECT_FREE;
        }
        break;
    case 50:
        obj->scale.vx += 32;
        obj->scale.vy += 32;
        obj->scale.vz += 32;
        hit = effect_collision_step(180, 0, -300);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                effect_apply_current_magic_backstep(hit);
            }
            obj->type = EFFECT_FREE;
        }
        break;
    case 58:
        obj->clut_offset = (obj->clut_offset + 64) & 0x40;
        hit = effect_collision_step(180, 0, -600);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->type = EFFECT_FREE;
            }
            effect_apply_current_magic_backstep(hit);
            obj->type = EFFECT_FREE;
        }
        break;
    case 21:
        {
            SVECTOR v;
            VECTOR ring;

            v.vx = (rand() * 32 >> 15) - 16;
            v.vy = (rand() * 32 >> 15) - 16;
            v.vz = (rand() * 32 >> 15) - 16;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1a90, -1000, 4, 0x25, 0, 1, 1);
            hit = effect_collision_step(180, 0, 0);
            if (hit == 0) {
                break;
            }
            if (hit & 0xf) {
                obj->type = EFFECT_FREE;
            }
            if (!(hit & 0x10)) {
                break;
            }
            effect_construct_record(10, 0x43, 0x58, &obj->pos, NULL, 0, 1);
            effect_construct_record(10, 0x43, 0x58, &obj->pos, NULL, 0x80, 1);
            effect_play_spatial_sound(obj, 0x25);
            for (i = 0; i < 4096; i += 256) {
                v.vx = rsin(i) * 360 >> 12;
                v.vz = rcos(i) * 360 >> 12;
                v.vy = 0;
                ring.vx = obj->pos.vx;
                ring.vy = obj->pos.vy + 1200;
                ring.vz = obj->pos.vz;
                effect_construct_record(10, 0, 0x5b, &ring, &v);
            }
            break;
    case 11:
        hit = effect_collision_step(180, 0, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->type = EFFECT_FREE;
            }
            if (obj->arg.b.unk44 == 0) {
                effect_apply_current_magic_backstep(hit);
                obj->arg.b.unk44 = 1;
            }
        } else {
            obj->arg.b.unk44 = 0;
        }
        break;
    case 9:
        hit = effect_collision_step(250, 0x80000000, 0);
        if (hit != 0 && (hit & 0xf)) {
            obj->type = EFFECT_FREE;
        }
        if (obj->arg.s.unk44 >= 11 && obj->arg.s.unk44 % 6 == 0) {
            effect_construct_record(10, 0x42, 0x5e, &obj->pos, NULL, obj->arg.s.unk46);
        }
        obj->animation_phase = (obj->animation_phase + 128) & 0xfff;
        obj->arg.s.unk44++;
        break;
    case 17:
        {
            SVECTOR v;

            hit = effect_collision_step(250, 0x80000000, 0);
            if (hit != 0) {
                if (hit & 0xf) {
                    obj->type = EFFECT_FREE;
                }
                effect_apply_current_magic_backstep(hit);
            }
            if (obj->arg.s.unk44 >= 16 && obj->arg.s.unk44 % 10 == 0) {
                v.vy = 84;
                v.vx = 0;
                v.vz = 0;
                effect_construct_record(10, 0x42, 0x59, &obj->pos, &v);
            }
            obj->animation_phase = (obj->animation_phase + 128) & 0xfff;
            obj->arg.s.unk44++;
            break;
    case 89:
        obj->angle.vy += 320;
        switch (obj->arg.s.unk44) {
        case 0:
            hit = effect_collision_step(180, 0x80000000, 0);
            if (hit != 0 && (hit & 0xf)) {
                obj->arg.s.unk44++;
                effect_play_spatial_sound(obj, 0x27);
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
                obj->type = EFFECT_FREE;
            }
            if (obj->scale.vx >= 768) {
                VECTOR p;

                p.vx = obj->pos.vx;
                p.vy = obj->pos.vy + 1000;
                p.vz = obj->pos.vz;
                effect_apply_radial_magic_damage(&p, obj->scale.vx * 29 >> 5, obj->scale.vx * 35 >> 5, 0x800,
                                                 0x400, 0x1000);
            }
            break;
        }
        break;
    case 72:
        if (obj->arg.c.unk44 == 0) {
            effect_scale_step(0x3800, 504, 70, 0x800, 0x8000);
        } else {
            obj->arg.c.unk44--;
        }
        break;
    case 122:
        if (effect_aim_and_move(600, 40, 36, 180, 360, 0x1000, 0x104,
                                0x800) != -1 && obj->updates_remaining >= 2) {
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            break;
        }
        effect_collision_backtrack();
        effect_construct_record(10, obj->type | 3, 0x48, &obj->pos, NULL, 0);
        effect_construct_record(10, obj->type | 3, 0x48, &obj->pos, NULL, 1);
        effect_construct_record(10, obj->type | 3, 0x48, &obj->pos, NULL, 2);
        effect_play_spatial_sound(obj, 0x27);
        obj->type = EFFECT_FREE;
        break;
    case 120:
        if (effect_collision_step(180, 360, 0) == 0 && obj->updates_remaining >= 2) {
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            break;
        }
        effect_construct_record(10, obj->type | 3, 0x48, &obj->pos, NULL, 0);
        effect_construct_record(10, obj->type | 3, 0x48, &obj->pos, NULL, 1);
        effect_construct_record(10, obj->type | 3, 0x48, &obj->pos, NULL, 2);
        effect_play_spatial_sound(obj, 0x27);
        obj->type = EFFECT_FREE;
        break;
    case 92:
    case 93:
        {
            Effect *link;
            s32 d;

            obj->pos.vx += obj->vel.vx;
            obj->pos.vz += obj->vel.vz;
            link = &effect_status.objects[obj->arg.c.unk45];
            switch (obj->arg.c.unk44) {
            case 0:
                effect_apply_current_magic_backstep(
                    collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 10, obj->scale.vy, 0x30));
                collision_probe_floor_height(obj->pos.vx, link->pos.vy, obj->pos.vz, 0, 0);
                obj->pos.vy = map_world.collision_floor_height;
                d = map_world.collision_floor_height - map_world.collision_height_limit;
                obj->scale.vy = d > 0x7fff ? 0x4000 : d >> 1;
                if (link->type == EFFECT_FREE) {
                    obj->arg.c.unk44 = 1;
                    obj->arg.s.unk46 = 0;
                    obj->scale.pad = obj->scale.vy;
                }
                break;
            case 1:
                obj->scale.vy = fixed_lerp_q12(obj->scale.pad, 0, obj->arg.s.unk46);
                obj->arg.s.unk46 += 512;
                if (obj->arg.s.unk46 >= 0x1000) {
                    obj->type = EFFECT_FREE;
                }
                break;
            }
            break;
    case 68:
    case 69:
        effect_scale_step(0x4000, obj->arg.s.unk44, 70, 0x800, 0x8000);
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
            if (collision_query_shapes_with_layer_sample(mid.vx, mid.vy, mid.vz, 5, 10) != 0 ||
                collision_query_shapes_with_layer_sample(obj->pos.vx, obj->pos.vy, obj->pos.vz, 5, 10) != 0) {
                obj->type = EFFECT_FREE;
            }
            if (obj->type & 0x40) {
                effect_construct_record(10, obj->type, 0x45, &obj->pos, NULL, 420);
            } else {
                effect_construct_record(obj->damage_multiplier_tenths, obj->type, 0x45, &obj->pos, NULL, 420);
            }
            break;
    case 124:
        {
            Actor *o;
            s32 n;

            obj->angle.vy += 400;
            obj->pos.vx += obj->vel.vx;
            obj->pos.vy += obj->vel.vy;
            obj->pos.vz += obj->vel.vz;
            p.vx = obj->pos.vx;
            p.vy = obj->pos.vy + 5000;
            p.vz = obj->pos.vz;
            n = actor_find_overlap_excluding_target_type3(p.vx, p.vy, p.vz, 100, 10000);
            if (n != -1) {
                o = &actor_status.objects[n];
                effect_play_spatial_sound(obj, 0x27);
                effect_construct_record(10, obj->type, 0x44, &o->pos, NULL, 1260);
            } else {
                collision_query_world(p.vx, p.vy, p.vz, 100, 10000, 0xb0);
                if (map_world.collision_hit_flags == 0x80) {
                    VECTOR q;

                    q = cur_avatar_status.pos;
                    q.vy -= 1700;
                    effect_play_spatial_sound(obj, 0x27);
                    effect_construct_record(obj->damage_multiplier_tenths, obj->type, 0x44,
                                            &cur_avatar_status.pos, NULL, 1260);
                }
            }
            effect_spawn_at_lower_bound(&p, 0x2000, 0x7fff, 10000);
            break;
    case 66:
        hit = effect_collision_step(100, 200, 0);
        if (hit == 0) {
            effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
            break;
        }
        effect_apply_current_magic_backstep(hit);
        obj->type = EFFECT_FREE;
        break;
    case 31:
        {
            SVECTOR v;

            obj->type = 0x81;
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
            effect_apply_radial_magic_damage(&obj->pos, 0, obj->scale.vx >> 4, 0x8000, 0x400, 0x1000);
            obj->type = 0x84;
            if (effect_collision_step(100, 200, 0) != 0) {
                obj->type = EFFECT_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 2, 5);
            break;
    case 34:
        {
            SVECTOR v;

            obj->type = 0x81;
            effect_apply_radial_magic_damage(&obj->pos, 0, obj->scale.vx >> 4, 0x8000, 0x400, 0x1000);
            obj->type = 0x84;
            if (effect_collision_step(100, 200, 0) != 0) {
                effect_collision_backtrack();
                obj->type = EFFECT_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 4);
            break;
    case 32:
        {
            SVECTOR v;

            obj->type = 0x81;
            effect_apply_radial_magic_damage(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->type = 0x84;
            if (effect_collision_step(100, 200, 0) != 0) {
                effect_collision_backtrack();
                obj->type = EFFECT_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 5);
            obj->angle.vz += 300;
            break;
    case 37:
        {
            SVECTOR v;

            obj->type = 0x81;
            effect_apply_radial_magic_damage(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->type = 0x84;
            if (effect_collision_step(100, 200, 0) != 0) {
                effect_collision_backtrack();
                obj->type = EFFECT_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 7);
            obj->angle.vz += 300;
            break;
    case 35:
    case 36:
        {
            SVECTOR v;

            obj->type = 0x81;
            effect_apply_radial_magic_damage(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->type = 0x81;
            if (effect_collision_step(100, 200, 0) != 0) {
                effect_collision_backtrack();
                obj->type = EFFECT_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 3);
            break;
    case 33:
        {
            SVECTOR v;

            obj->type = 0x81;
            effect_apply_radial_magic_damage(&obj->pos, 0, obj->scale.vx >> 3, 0x8000, 0x400, 0x1000);
            obj->type = 0x81;
            if (effect_collision_step(100, 200, 0) != 0) {
                effect_collision_backtrack();
                obj->type = EFFECT_FREE;
            }
            v.vx = obj->vel.vx * 2 / 5;
            v.vy = obj->vel.vy * 2 / 5;
            v.vz = obj->vel.vz * 2 / 5;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 5, 22, 0, 1, 8);
            break;
    case 113:
    case 125:
    case 129:
        {
            VECTOR at;
            SVECTOR v;
            s32 d;

            if (obj->phase == 2) {
                obj->scale.vy = obj->scale.vx -= 128;
                if (obj->scale.vx <= 0) {
                    obj->type = EFFECT_FREE;
                }
                break;
            }
            hit = effect_collision_step(50, 0x80000000, -300);
            if (hit != 0) {
                if (hit & 0xf) {
                    if (phase == 0) {
                        effect_collision_backtrack();
                        obj->vel.vx = obj->vel.vz = 0;
                    } else {
                        effect_collision_backtrack();
                        collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz, 50, 0);
                        if (map_world.collision_floor_height < map_world.collision_bounds[0]) {
                            at.vy = map_world.collision_floor_height;
                        } else {
                            at.vy = map_world.collision_bounds[0];
                        }
                        d = at.vy - obj->pos.vy;
                        goto burst;
                    }
                }
                obj->phase = 1;
            }
            if (phase == 1) {
                obj->vel.vy -= 70;
                obj->vel.vx = fixed_lerp_q12(0, obj->vel.vx, 3000);
                obj->vel.vz = fixed_lerp_q12(0, obj->vel.vz, 3000);
                if (map_world.collision_floor_height < map_world.collision_bounds[0]) {
                    at.vy = map_world.collision_floor_height;
                } else {
                    at.vy = map_world.collision_bounds[0];
                }
                d = at.vy - obj->pos.vy;
                if (d >= 7000) {
                burst:
                    obj->phase = 2;
                    at.vx = obj->pos.vx;
                    at.vz = obj->pos.vz;
                    switch (id) {
                    case 113:
                        effect_construct_record(10, obj->type | 3, 0x72, &at, &v, d);
                        effect_construct_record(10, obj->type, 0x16, &at, &v, d >> 1, d >> 4, 0x800, 1);
                        break;
                    case 125:
                        effect_construct_record(10, obj->type | 3, 0x7e, &at, &v, d);
                        effect_construct_record(10, obj->type, 0x16, &at, &v, d >> 1, d >> 4, 0x800, 3);
                        break;
                    case 129:
                        effect_construct_record(10, obj->type | 3, 0x80, &at, &v, d);
                        effect_construct_record(10, obj->type, 0x16, &at, &v, d >> 1, d >> 4, 0x800, 0);
                        break;
                    }
                    effect_play_spatial_sound(obj, 0x27);
                }
            } else if (phase == 0) {
                if (obj->arg.s.unk44-- <= 0) {
                    obj->phase = 1;
                }
            }
            for (i = 3; i != -1; i--) {
                v.vx = (rand() >> 7) - 128;
                v.vy = (rand() >> 7) - 128;
                v.vz = (rand() >> 7) - 128;
                effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x800, -128, 5, 12, 0, 1, 1);
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
                    effect_construct_record(10, obj->type | 3, 0x13, &obj->pos, &v, r);
                    break;
                case 126:
                    effect_construct_record(10, obj->type | 3, 0x29, &obj->pos, &v, r);
                    break;
                case 128:
                    effect_construct_record(10, obj->type | 3, 0x2d, &obj->pos, &v, r);
                    break;
                }
            }
            obj->animation_id = (phase & 1) - 128;
            if (obj->phase < 9) {
                d = fixed_vector3_length(cur_avatar_status.pos.vx - obj->pos.vx,
                                         cur_avatar_status.pos.vy - obj->pos.vy,
                                         cur_avatar_status.pos.vz - obj->pos.vz);
                if (d <= 32000) {
                    d = ((rsin(obj->phase << 8) >> 1) + 1024) * (32000 - d) / 32000;
                    light_interpolate_color_and_fog(200, 180, 160, 0, 32000, d);
                }
            }
            obj->phase++;
            break;
    case 19:
    case 41:
    case 45:
        {
            SVECTOR v;

            effect_scale_step(0x3800, obj->arg.s.unk44, 0x80, 0x400, 0x8000);
            v.vx = (rand() >> 6) - 256;
            v.vz = (rand() >> 6) - 256;
            v.vy = -(rand() >> 7) - 128;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x2000, -128, 15, 12, 10, 1, 1);
            obj->angle.vy += 64;
            break;
    case 42:
        hit = effect_collision_step(180, 0, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->type = EFFECT_FREE;
            }
            if (obj->arg.b.unk44 == 0) {
                obj->arg.b.unk44 = 1;
                effect_apply_current_magic_backstep(hit);
            }
        } else {
            obj->arg.b.unk44 = 0;
        }
        effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
        break;
    case 48:
    case 53:
        obj->scale.vx += 32;
        obj->scale.vy += 32;
        obj->scale.vz += 32;
        obj->angle.vz += 200;
        hit = effect_collision_step(180, 0, 0);
        if (hit != 0) {
            if (!(hit & 0xf)) {
                effect_apply_current_magic_backstep(hit);
            }
            obj->type = EFFECT_FREE;
        }
        effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
        break;
    case 46:
        obj->angle.vx = ratan2(obj->vel.vy, fixed_vector2_length(obj->vel.vx, obj->vel.vz));
        obj->angle.vy = ratan2(obj->vel.vz, obj->vel.vx) - 1024;
        hit = effect_collision_step(180, 0, 0);
        if (hit != 0) {
            effect_apply_current_magic_backstep(hit);
            obj->kind = 0x58;
            obj->render_flags = 1;
            obj->render_mode = 5;
            obj->base_render_id = 0x15;
            obj->render_id = 0x15;
            obj->lighting_override_index = 0x34;
            obj->lighting_blend = 0x1000;
            obj->updates_remaining = 40;
            obj->animation_id = 0;
            obj->scale.vx = 0;
            obj->scale.vy = 0;
            obj->scale.vz = 0;
            obj->clut_offset = 0x40;
            break;
        }
        effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
        if ((obj->type & 0x40) && obj->vel.vy < 800) {
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
                effect_construct_record(10, 0x81, 0x2e, &obj->pos, &obj->vel);
            }
            if (++obj->arg.s.unk46 >= 7) {
                obj->type = EFFECT_FREE;
            }
            break;
        }
        break;
    case 47:
        hit = effect_collision_step(180, 0x80000000, 0);
        if (hit != 0) {
            if (hit & 0xf) {
                obj->type = EFFECT_FREE;
            } else {
                effect_apply_current_magic_backstep(hit);
            }
        }
        effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
        if (++obj->arg.s.unk44 >= 6) {
            obj->arg.s.unk44 = 0;
        }
        obj->clut_offset = obj->arg.s.unk44 << 6;
        break;
    case 90:
        obj->scale.vz += obj->arg.s.unk46;
        obj->scale.vx += obj->arg.s.unk46;
        if (obj->scale.vx >= 768) {
            VECTOR p;

            p.vx = obj->pos.vx;
            p.vy = obj->pos.vy + 1000;
            p.vz = obj->pos.vz;
            effect_apply_radial_magic_damage(&p, (obj->scale.vx - obj->arg.s.unk46) * 4,
                                             obj->scale.vx * 4 - 1, 2000, 0x1000, obj->arg.s.unk48);
        }
        if (obj->scale.vx > obj->arg.s.unk44) {
            obj->type = EFFECT_FREE;
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
            effect_apply_radial_magic_damage(&p, obj->scale.vx * 15 >> 4, obj->scale.vx * 9 >> 3, 0x8002,
                                             0x400, 0x1000);
        }
        if (obj->scale.vx > 3200) {
            obj->type = EFFECT_FREE;
        }
        break;
    case 88:
        effect_scale_step(0x4000, 0x600, 0x80, 0x400, 0x8000);
        obj->angle.vy += 64;
        break;
    case 18:
    case 20:
    case 71:
        switch (phase) {
        case 101:
            obj->phase++;
            break;
        case 100:
            goto spawn18;
        case 102:
            goto become18;
        case 110:
        shrink18:
            if (id == 18) {
                obj->clut_offset = 0;
            }
            effect_scale_step(0x2000, 799, 70, 0xc00, 0x8000);
            obj->angle.vy += 64;
        break;
        default:
            if (obj->phase == 0) {
                audio_play_spatial_range(0x2c, &obj->pos, 0x6e, 0x6d60, 0x7148, 0);
            }
            obj->phase++;
            if (effect_aim_and_move(obj->arg.s.unk44, obj->arg.s.unk46, obj->arg.s.unk48, 160, 0, 6000,
                                    obj->arg.s.unk4a, 0x800) == -1) {
                Effect *made;

            spawn18:
                made = effect_construct_record(10, obj->type | 3, id, &obj->pos, NULL, &obj->angle);
                made->phase = 101;
            become18:
                obj->render_id = 0x15;
                obj->phase = 110;
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->clut_offset = 0x40;
                obj->type |= 3;
                effect_play_spatial_sound(obj, 0x28);
                goto shrink18;
            } else {
                SVECTOR v;

                effect_spawn_at_lower_bound(&obj->pos, 0x2000, 0x2000, 500);
                obj->angle.vz += 128;
                v.vx = v.vy = v.vz = 0;
                if (id == 18) {
                    effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 3, 0x25, 0, 1, 0);
                } else {
                    effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 3600, -128, 3, 0x25, 0, 1, 1);
                }
            }
            break;
        }
        break;
    case 2:
        {
            SVECTOR v;
            VECTOR p;
            Actor *o;

            if (obj->arg.b.unk44 == 0xfe) {
                p.vx = cur_avatar_status.pos.vx;
                p.vy = cur_avatar_status.pos.vy - 1600;
                p.vz = cur_avatar_status.pos.vz;
                hit = effect_target_motion(&p, 500, 60, 3000, 0, 10, 0x80000000);
            } else if (obj->arg.b.unk44 == 0xff) {
                goto fall2;
            } else {
                o = &actor_status.objects[obj->arg.b.unk44];
                p.vx = o->pos.vx;
                p.vy = o->pos.vy - (o->collision_height >> 1);
                p.vz = o->pos.vz;
                hit = effect_target_motion(&p, 450, 60, 100, 0, 10, 0x80000000);
            }
            if (hit != -1) {
                goto trail2;
            }
            {
            boom2:
                effect_apply_current_magic_backstep(map_world.collision_hit_flags);
                for (i = 5; i != -1; i--) {
                    v.vx = (rand() * 400 >> 15) - 200;
                    v.vz = (rand() * 400 >> 15) - 200;
                    v.vy = (rand() * 400 >> 15) - 200;
                    effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 7500, -750, 8, 22, 10, 1, 2);
                }
                effect_scatter_lower_bound(&obj->pos, 5, 0x400, 0x2000, 0x4000, 0x1000);
                obj->type = EFFECT_FREE;
                break;
            }
        fall2:
            obj->vel.vy += 10;
            if (effect_collision_step(10, 0x80000000, 0) != 0) {
                goto boom2;
            }
        trail2:
            if (obj->updates_remaining < 74) {
                v.vx = (rand() * 60 >> 15) - 30;
                v.vy = (rand() * 60 >> 15) - 30;
                v.vz = (rand() * 60 >> 15) - 30;
                effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1000, -256, 13, 0x1c, 10, 1, 2);
            }
        }
        break;
    case 1:
        {
            SVECTOR v;
            VECTOR p;

            hit = effect_collision_step(180, 0x80000000, 0);
            if (hit != 0 && (hit & 0x10)) {
                effect_apply_current_magic_backstep(hit);
            }
            p.vx = v.vx + obj->pos.vx;
            p.vy = v.vy + obj->pos.vy;
            p.vz = v.vz + obj->pos.vz;
            effect_apply_radial_magic_damage(&p, 0, obj->scale.vx * 1024 >> 12, obj->scale.vy, 0x400, 0x1000);
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
                    obj->type = EFFECT_FREE;
                }
                break;
            }
            v.vx = 0;
            v.vy = -(rand() * 100 >> 15) - 100;
            v.vz = 0;
            effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1000, 0, 8, 0x1c, 2, 1, 2);
            break;
    case 108:
        hit = effect_collision_step(180, 0, 0);
        if (hit != 0 && (hit & 0x10)) {
            effect_apply_current_magic_backstep(hit);
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
                obj->type = EFFECT_FREE;
            }
            break;
        }
        break;
    case 4:
        switch (phase) {
        case 0:
            if (effect_collision_step(250, 0x80000000, 0) != 0 || obj->updates_remaining < 2) {
                Effect *made;

                effect_scatter_lower_bound(&obj->pos, 8, 400, 0x2000, 0x8000, 0x400);
                made = effect_construct_record(10, obj->type | 3, 4, &obj->pos, NULL, &obj->angle);
                made->phase = 2;
                effect_play_spatial_sound(obj, 0x27);
            case 5:
                obj->phase = 1;
                obj->render_id = 0x15;
                obj->lighting_override_index = 0x34;
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->updates_remaining = -1;
                obj->type |= 3;
                goto shrink4;
            } else {
                SVECTOR dir;
                VECTOR t;
                VECTOR a;
                Actor *o;
                s32 dist;

                obj->animation_phase = (obj->animation_phase + 128) & 0xfff;
                if (obj->arg.b.unk45 != 0) {
                    effect_sample_world_vertex(obj, 0, &a, &obj->scale);
                    o = &actor_status.objects[obj->arg.b.unk44];
                    t.vx = o->pos.vx;
                    t.vy = o->pos.vy - (o->collision_height >> 1);
                    t.vz = o->pos.vz;
                    vector_direction_scaled(&a, &t, 800, &dir);
                    effect_construct_record(10, obj->type, 0, &a, &dir);
                    obj->arg.b.unk45--;
                } else if (rand() < 2048) {
                    o = actor_find_best_in_cone(&obj->pos, obj->angle.vy, obj->angle.vx, 25000, 800, 800,
                                                &dist, 0x200);
                    if (o != NULL) {
                        obj->arg.b.unk44 = o - actor_status.objects;
                        obj->arg.b.unk45 = 5;
                    }
                }
                if (rand() < 1024 &&
                    actor_find_best_in_cone(&obj->pos, obj->angle.vy, obj->angle.vx, 30000, 800, 800, &dist,
                                            0x200) != NULL) {
                    effect_spawn_zero_direction(obj, 0x2f);
                    effect_spawn_zero_direction(obj, 0x32);
                }
            }
            break;
        case 1:
        shrink4:
            effect_scale_step(0x4000, 0x800, 0x4b, 0x400, 0x8000);
            obj->angle.vy += 64;
            break;
        default:
            obj->phase++;
            break;
        }
        break;
    case 16:
        {
            Actor *o;
            EffectTrailRow *trail;
            s32 a;
            VECTOR tmp;

            switch (phase) {
            case 0:
                {
                    Effect *made;

                    for (i = 0; i < 8; i++) {
                        made = effect_construct_record(10, 0, 0x74, &obj->pos, &obj->vel, &obj->angle, i);
                        if (i == 0) {
                            made->render_id = 0x10;
                        }
                        made->arg.b.unk44 = effect_status.current_index;
                        made->arg.b.unk45 = i;
                    }
                    made->render_id = 0x12;
                    obj->phase = 1;
                }
            case 1:
                if (obj->updates_remaining < 3) {
                fail16:
                    obj->phase = 3;
                    obj->updates_remaining = -1;
                    obj->arg.b.trail_phase_counter = 24;
                    break;
                }
                if (effect_aim_and_move(250, 25, 32, 200, 0, 0x400, 100, 0x800) == -1) {
                    obj->updates_remaining = -1;
                    if (map_world.collision_hit_flags != 0x10) {
                        goto fail16;
                    }
                    effect_apply_current_magic(0x10010, 5000, NULL);
                    obj->arg.b.trail_actor_index = map_world.collision_actor_index;
                    o = &actor_status.objects[obj->arg.b.trail_actor_index];
                    if (o->target_type != 2 && o->target_type != 3) {
                        goto fail16;
                    }
                    obj->render_flags = 1;
                    obj->render_id = 0x13;
                    obj->phase = 2;
                    obj->arg.b.trail_phase_counter = 0;
                    obj->angle.vz = 0;
                    obj->angle.vx = 0;
                    obj->angle.pad = -vector_xz_to_angle(obj->pos.vx - o->pos.vx, obj->pos.vz - o->pos.vz);
                    goto case16_2;
                } else {
                    if (++obj->arg.b.trail_frame_index >= 24) {
                        obj->arg.b.trail_frame_index = 0;
                    }
                    trail = &obj->arg.w.trail_rows[obj->arg.b.trail_frame_index];
                    a = obj->arg.b.trail_phase_counter << 4;
                    obj->arg.b.trail_phase_counter += 8;
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

                    o = &actor_status.objects[obj->arg.b.trail_actor_index];
                    o->flags |= 0x800;
                    n = o->collision_radius;
                    if (++obj->arg.b.trail_frame_index >= 24) {
                        obj->arg.b.trail_frame_index = 0;
                    }
                    obj->pos = *actor_resolve_group_position(o, &tmp);
                    obj->pos.vy -= o->collision_height >> 1;
                    k = obj->arg.b.trail_phase_counter;
                    if (k >= 17) {
                        if (k >= 60) {
                            obj->phase = 4;
                            break;
                        }
                    } else {
                        obj->scale.vx = obj->scale.vz = fixed_lerp_q12(0, n, k << 9);
                        obj->scale.vy = fixed_lerp_q12(0, o->collision_height,
                                                       obj->arg.b.trail_phase_counter * 350);
                    }
                    d = 100000 / n;
                    n = n * 6400 >> 12;
                    a = obj->angle.pad;
                    obj->arg.b.trail_phase_counter++;
                    trail = &obj->arg.w.trail_rows[obj->arg.b.trail_frame_index];
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
                if (++obj->arg.b.trail_frame_index >= 24) {
                    obj->arg.b.trail_frame_index = 0;
                }
                if (obj->arg.b.trail_phase_counter == 0) {
                    goto release16;
                }
                obj->arg.b.trail_phase_counter--;
                break;
            case 4:
                {
                    SVECTOR v;

                    for (i = 23; i != -1; i--) {
                        if (!(i & 7)) {
                            effect_play_spatial_sound(obj, 0x27);
                        }
                        v.vx = rand() * 600 / 32767 - 300;
                        v.vy = rand() * -300 / 32767;
                        v.vz = rand() * 600 / 32767 - 300;
                        effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 1500, -128, 9, 0x30, 0, 1, 0);
                    }
                release16:
                    o = &actor_status.objects[obj->arg.b.trail_actor_index];
                    o->flags &= ~0x800;
                    obj->type = EFFECT_FREE;
                }
                break;
            }
            break;
    case 5:
        {
            Actor *o;
            EffectTrailRow *trail;
            s32 a;
            SVECTOR v;
            VECTOR tmp;

            switch (phase) {
            case 0:
                {
                    Effect *made;

                    for (i = 0; i < 8; i++) {
                        made = effect_construct_record(10, 0, 0x67, &obj->pos, &obj->vel, &obj->angle, i);
                        if (i == 0) {
                            made->render_id = 0x10;
                        }
                        made->arg.b.unk44 = effect_status.current_index;
                        made->arg.b.unk45 = i;
                    }
                    made->render_id = 0x12;
                    obj->phase = 1;
                }
            case 1:
                if (obj->updates_remaining < 3) {
                fail5:
                    obj->phase = 3;
                    obj->updates_remaining = -1;
                    obj->arg.b.trail_phase_counter = 24;
                    break;
                }
                if (effect_aim_and_move(250, 25, 32, 200, 0, 0x400, 100, 0x800) == -1) {
                    obj->updates_remaining = -1;
                    if (map_world.collision_hit_flags != 0x10) {
                        goto fail5;
                    }
                    effect_apply_current_magic(0x10010, 5000, NULL);
                    obj->arg.b.trail_actor_index = map_world.collision_actor_index;
                    o = &actor_status.objects[obj->arg.b.trail_actor_index];
                    if (o->target_type != 2 && o->target_type != 3) {
                        goto fail5;
                    }
                    obj->render_flags = 1;
                    obj->render_id = 0x13;
                    obj->phase = 2;
                    obj->arg.b.trail_phase_counter = 0;
                    obj->angle.vz = 0;
                    obj->angle.vx = 0;
                    obj->angle.pad = -vector_xz_to_angle(obj->pos.vx - o->pos.vx, obj->pos.vz - o->pos.vz);
                    for (i = 0; i < 8; i++) {
                        effect_construct_record(10, 0, 0x6c, &o->pos, NULL, i << 9);
                    }
                    goto case5_2;
                } else {
                    if (++obj->arg.b.trail_frame_index >= 24) {
                        obj->arg.b.trail_frame_index = 0;
                    }
                    trail = &obj->arg.w.trail_rows[obj->arg.b.trail_frame_index];
                    a = obj->arg.b.trail_phase_counter << 4;
                    obj->arg.b.trail_phase_counter += 8;
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

                    o = &actor_status.objects[obj->arg.b.trail_actor_index];
                    o->flags |= 0x800;
                    v.vx = rand() * 100 / 32767 - 50;
                    v.vy = -(rand() * 160 / 32767) - 200;
                    v.vz = rand() * 100 / 32767 - 50;
                    effect_construct_record(10, 0, 0x6f, &o->pos, &v, 0x1000, 0x200, 6, 0x1c, 2, 1, 2);
                    n = o->collision_radius;
                    if (++obj->arg.b.trail_frame_index >= 24) {
                        obj->arg.b.trail_frame_index = 0;
                    }
                    obj->pos = *actor_resolve_group_position(o, &tmp);
                    obj->pos.vy -= o->collision_height >> 1;
                    k = obj->arg.b.trail_phase_counter;
                    if (k >= 17) {
                        if (k >= 60) {
                            obj->phase = 4;
                            break;
                        }
                    } else {
                        obj->scale.vx = obj->scale.vz = fixed_lerp_q12(0, n, k << 9);
                        obj->scale.vy = fixed_lerp_q12(0, o->collision_height,
                                                       obj->arg.b.trail_phase_counter * 350);
                    }
                    d = 100000 / n;
                    n = n * 6400 >> 12;
                    a = obj->angle.pad;
                    obj->arg.b.trail_phase_counter++;
                    trail = &obj->arg.w.trail_rows[obj->arg.b.trail_frame_index];
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
                if (++obj->arg.b.trail_frame_index >= 24) {
                    obj->arg.b.trail_frame_index = 0;
                }
                if (obj->arg.b.trail_phase_counter == 0) {
                    goto release5;
                }
                obj->arg.b.trail_phase_counter--;
                break;
            case 4:
                {
                    for (i = 1; i != -1; i--) {
                        v.vx = rand() * 200 / 32767 - 100;
                        v.vy = -(rand() * 200 / 32767) - 200;
                        v.vz = rand() * 200 / 32767 - 100;
                        effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1000, 0, 5, 0x1c, 10, 1, 1);
                    }
                release5:
                    o = &actor_status.objects[obj->arg.b.trail_actor_index];
                    o->flags &= ~0x800;
                    obj->type = EFFECT_FREE;
                }
                break;
            }
            break;
    case 116:
        {
            Effect *link;
            EffectTrailRow *t;
            s32 k;

            link = &effect_status.objects[obj->arg.b.unk44];
            if (phase == 0 && link->phase >= 3) {
                obj->phase = 1;
                obj->updates_remaining = obj->arg.b.unk45 * 3;
            }
            if (link->phase == 4) {
                obj->type = EFFECT_FREE;
                break;
            }
            k = link->arg.b.trail_frame_index - obj->arg.b.unk45 * 3;
            if (k < 0) {
                k += 24;
            }
            t = &link->arg.w.trail_rows[k];
            obj->pos = t->pos;
            obj->angle = t->angle;
            break;
    case 103:
        {
            Effect *link;
            EffectTrailRow *t;
            SVECTOR v;
            s32 k;

            link = &effect_status.objects[obj->arg.b.unk44];
            if (obj->updates_remaining < 188) {
                v.vx = (rand() * 100 >> 15) - 50;
                v.vz = (rand() * 100 >> 15) - 50;
                v.vy = -120;
                effect_construct_record(10, 0, 0x6f, &obj->pos, &v, 0x1000, -256, 4, 0x16, 0, 2, 5);
            }
            if (phase == 0 && link->phase >= 3) {
                obj->phase = 1;
                obj->updates_remaining = obj->arg.b.unk45 * 3;
            }
            if (link->phase == 4) {
                obj->type = EFFECT_FREE;
                break;
            }
            k = link->arg.b.trail_frame_index - obj->arg.b.unk45 * 3;
            if (k < 0) {
                k += 24;
            }
            t = &link->arg.w.trail_rows[k];
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

            if (obj->updates_remaining >= 10) {
                obj->pos.vx += obj->vel.vx;
                obj->pos.vz += obj->vel.vz;
                obj->animation_id = (obj->updates_remaining & 1) - 128;
            } else {
                obj->pos.vx -= obj->vel.vx;
                obj->pos.vz -= obj->vel.vz;
                obj->scale.vy += 512;
                obj->animation_id = (obj->updates_remaining & 1) - 128;
            }
            p.vx = obj->vel.vx + obj->pos.vx;
            p.vy = obj->vel.vy + obj->pos.vy;
            p.vz = obj->vel.vz + obj->pos.vz;
            effect_apply_radial_magic_damage(&p, 0, obj->scale.vx, 0x8000, 0x400, 0x1000);
            break;
    case 112:
        obj->phase++;
        obj->scale.vy = rsin(obj->phase << 8) * obj->arg.s.unk44 >> 12;
        if (obj->phase >= 8) {
            obj->type = EFFECT_FREE;
        }
        break;
    case 26:
        if (cur_avatar_status.fire_defense_boost_timer == 0) {
            obj->type = EFFECT_FREE;
        }
        obj->angle.vy += 64;
        break;
    case 27:
        if (cur_avatar_status.damage_immunity_timer == 0) {
            obj->type = EFFECT_FREE;
        }
        obj->angle.vy -= 256;
        break;
    case 28:
        if (cur_avatar_status.physical_defense_boost_timer == 0) {
            obj->type = EFFECT_FREE;
        }
        obj->angle.vy -= 64;
        break;
    case 23:
        if (obj->updates_remaining == 4) {
            cur_avatar_status.hp += 80;
            if (cur_avatar_status.hp > cur_avatar_status.hp_max) {
                cur_avatar_status.hp = cur_avatar_status.hp_max;
            }
        }
        light_interpolate_color_and_fog(230, 200, 160, 0, 23000, rsin(obj->updates_remaining << 8));
        break;
    case 25:
        {
            VECTOR p;
            SVECTOR v;
            Effect *made;

            if (obj->updates_remaining == 12) {
                cur_avatar_status.poison_timer = 0;
            }
            p.vx = (rand() >> 5) - 512;
            p.vy = (rand() >> 8) + 200;
            p.vz = 0x400;
            v.vz = 0;
            v.vx = 0;
            v.vy = 0;
            made = effect_construct_record(10, 0, 0x6f, &p, &v, 700, -30, 10, 0x26, -10, 1, 2);
            made->layer_mask = 1;
            made->render_flags = 14;
            light_interpolate_color_and_fog(160, 180, 220, 0, 18000, rsin(obj->updates_remaining << 7));
            break;
    case 30:
        {
            VECTOR p;
            SVECTOR v;
            Effect *made;

            if (obj->updates_remaining == 12) {
                cur_avatar_status.hp += 200;
                if (cur_avatar_status.hp > cur_avatar_status.hp_max) {
                    cur_avatar_status.hp = cur_avatar_status.hp_max;
                }
                player_cap_status_components(7);
            }
            v.vz = 0;
            v.vx = 0;
            v.vy = 0;
            for (i = 3; i != 0; i--) {
                p.vx = (rand() >> 5) - 512;
                p.vy = (rand() >> 8) + 200;
                p.vz = 0x400;
                made = effect_construct_record(10, 0, 0x6f, &p, &v, 700, -30, 10, 0x26, -10, 1, 1);
                made->layer_mask = 1;
                made->render_flags = 14;
            }
            light_interpolate_color_and_fog(240, 240, 160, 0, 18000, rsin(obj->updates_remaining << 7));
            break;
    case 8:
        effect_play_spatial_sound(obj, 0x23);
        effect_scale_step(0x4000, 480, 80, 0xc00, 0x8000);
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
            hit = collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 10, 10, 0xb0);
            if (hit == 0) {
                collision_probe_floor_height(obj->pos.vx, obj->arg.w.origin_y, obj->pos.vz, 0, 0);
                if (obj->pos.vy < map_world.collision_floor_height) {
                    goto fly95;
                }
                obj->pos.vy = map_world.collision_floor_height;
            }
            p = obj->pos;
            v.vx = 0;
            v.vy = -32;
            v.vz = 0;
            effect_apply_current_magic_backstep(hit);
            effect_construct_record(10, obj->type, 0x5a, &p, &v, 1600, 100, 0x800);
            for (i = 3; i != -1; i--) {
                v.vx = 0;
                v.vy = -360;
                v.vz = 0;
                p = obj->pos;
                p.vx += (rand() * 1000 >> 15) - 500;
                p.vz += (rand() * 1000 >> 15) - 500;
                effect_construct_record(10, 0, 0x6f, &p, &v, 0x4000, -128, 5, 0x1d, 0, 1, 2);
            }
            obj->type = EFFECT_FREE;
            break;
        fly95:
            for (i = 5; i != -1; i--) {
                v = obj->vel;
                v.vx = (v.vx >> 3) * (i + 1);
                v.vy = (v.vy >> 3) * (i + 1);
                v.vz = (v.vz >> 3) * (i + 1);
                effect_construct_record(10, 0, 0x6f, &obj->pos, &v, i * 350 + 1200, -128, 5, 0x1c, 0, 1, 2);
            }
            break;
    default:
        break;
        /* end of the case scopes left open above */
        }}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
    }
}
