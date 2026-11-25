/*
 * GAME.EXE: MapObject searches and spawns (WIP module): the collision search
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
s32 map_object_find_collision_at_point(s32 x, s32 y, s32 z, s32 radius, s32 shape)
{
    MapObject *obj;
    MapObjectDef *def;
    s32 i;
    s32 r;

    obj = map_objects_slots.objects;
    for (i = 0; i < 396; i++, obj++) {
        if (obj->object_id == 0xff) {
            continue;
        }
        if (obj->layer_mask == 0) {
            continue;
        }
        if (obj == map_objects_slots.current) {
            continue;
        }
        def = &map_objects_slots.defs[obj->object_id];
        if (def->flags & 0x10) {
            r = def->collision_radius * obj->arg.b.unk38 >> 7;
        } else {
            r = def->collision_radius;
        }
        if (r != 0) {
            if (vector_distance_to_point(&obj->pos, x, y, z, r + radius, obj->collision_height,
                                         shape) != -1) {
                return i;
            }
        } else if (obj->flags & 4) {
            if (vector_distance_to_point_in_rotated_box(&obj->angle, &obj->pos, x, y, z,
                                                        def->arg.h.unke + radius, def->arg.h.unk10 + radius,
                                                        obj->collision_height, shape) != -1) {
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
s32 map_object_find_interaction_target(s32 start, VECTOR *pos, s32 height, s32 reach_up, int angle_in,
                                       int range_in)
{
    s16 angle = angle_in;
    s16 range = range_in;
    SVECTOR offset;
    VECTOR at;
    MATRIX m;
    MapObject *obj;
    MapObjectDef *def;
    s16 i;

    obj = &map_objects_slots.objects[start];
    for (i = start; i < 396; i++, obj++) {
        if (obj->object_id == 0xff) {
            continue;
        }
        def = &map_objects_slots.defs[obj->object_id];
        if (def->action == 0x1b) {
            offset.vx = 0x700;
            offset.vy = 0;
            offset.vz = 0;
            matrix_set_rotation_y(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (vector_distance_to_point(&obj->pos, at.vx, pos->vy, at.vz, def->interaction_radius + height,
                                         obj->collision_height, reach_up) == -1) {
                continue;
            }
            if (angle_within_tolerance(angle, vector_xz_to_angle(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                                       range)) {
                return i;
            }
        } else if (def->action == 0) {
            offset.vx = -0x700;
            offset.vy = 0;
            offset.vz = 0;
            matrix_set_rotation_y(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (vector_distance_to_point(&obj->pos, at.vx, pos->vy, at.vz, def->interaction_radius + height,
                                         obj->collision_height, reach_up) == -1) {
                continue;
            }
            if (angle_within_tolerance(angle, vector_xz_to_angle(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                                       range)) {
                return i;
            }
        } else if (def->action == 1) {
            offset.vx = -0x400;
            offset.vy = 0;
            offset.vz = 0;
            matrix_set_rotation_y(obj->angle.vy, &m);
            ApplyMatrix(&m, &offset, &at);
            at.vx += pos->vx;
            at.vz += pos->vz;
            if (vector_distance_to_point(&obj->pos, at.vx, pos->vy, at.vz, def->interaction_radius + height,
                                         obj->collision_height, reach_up) == -1) {
                continue;
            }
            if (angle_within_tolerance(angle, vector_xz_to_angle(obj->pos.vx - at.vx, obj->pos.vz - at.vz),
                                       range)) {
                return i;
            }
        } else {
            if (vector_distance_to_point(&obj->pos, pos->vx, pos->vy, pos->vz,
                                         def->interaction_radius + height, def->collision_height,
                                         reach_up) == -1) {
                continue;
            }
            if (obj->object_id == 0x95
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
void map_object_start_action_if_idle(MapObject *obj, u8 kind)
{
    if (obj->action == 0xff) {
        obj->action = kind;
        obj->action_state = 0;
    }
}

/*
 * A free record among the count from first, else (stamp not -1) the one
 * whose arg.h.unk3c stamp is oldest, else NULL.
 */
ADDRESS(0x80046034, 0x88)
MapObject *map_object_effect_pool_acquire(s32 first, s32 count, s32 stamp)
{
    MapObject *obj;
    MapObject *oldest;
    s32 best;
    s32 age;

    obj = &map_objects_slots.objects[first];
    oldest = NULL;
    best = 0;
    do {
        if (obj->object_id == 0xff) {
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
 * Spawns a record of MapObjectDef def_id at pos raised by dy in one of the
 * two ten-record pools (which 0: from 370, else from 360), replacing the
 * oldest when the pool is full.
 */
ADDRESS(0x800460bc, 0x1d8)
void map_object_spawn_effect(u8 which, u8 def_id, VECTOR *pos, s32 dy)
{
    MapObject *obj;
    MapObjectDef *def;

    if (which == 0) {
        obj = map_object_effect_pool_acquire(370, 10, map_objects_slots.placement_drop_sequence);
        obj->arg.h.unk3c = map_objects_slots.placement_drop_sequence++;
        obj->object_id = def_id;
    } else {
        obj = map_object_effect_pool_acquire(360, 10, map_objects_slots.definition_drop_sequence);
        obj->arg.h.unk3c = map_objects_slots.definition_drop_sequence++;
        obj->object_id = def_id;
    }
    map_object_reset(obj);
    def = &map_objects_slots.defs[obj->object_id];
    if (def->flags & 0x10) {
        obj->scale.vx <<= 1;
        obj->scale.vy <<= 1;
        obj->scale.vz <<= 1;
    }
    obj->pos.vx = pos->vx;
    obj->pos.vy = dy + pos->vy;
    obj->pos.vz = pos->vz;
    obj->angle.vy = rand() >> 3;
    switch (def->kind) {
    case 16:
    case 19:
    case 22:
        map_object_start_action_if_idle(obj, 0x60);
        break;
    case 23:
        map_object_start_action_if_idle(obj, 0x61);
        break;
    case 17:
    case 18:
    case 20:
    case 21:
    case 24:
    case 25:
    case 32:
        map_object_start_action_if_idle(obj, 0x62);
        if (def_id == 0x5c || def_id == 0x86 || def_id == 0x88 || def_id == 0x8d || def_id == 0x8f) {
            obj->angle.pad = 0;
        } else {
            obj->angle.pad = 0x400;
        }
        break;
    }
    obj->arg.sh.motion_velocity = 0;
    obj->arg.b.unk38 = 0xff;
}

/*
 * Spawns an MapObjectDef 0x95 record (kind 0x62) for MapObject target in the
 * ten-record pool from 350, at a random point 600 from pos raised by dy.
 */
ADDRESS(0x80046294, 0x128)
void map_object_spawn_scattered_effect(u16 target, VECTOR *pos, s32 dy)
{
    MapObject *obj;
    u16 angle;

    obj = map_object_effect_pool_acquire(350, 10, map_objects_slots.scatter_pool_sequence);
    map_object_reset(obj);
    obj->arg.h.unk3c = map_objects_slots.scatter_pool_sequence++;
    obj->object_id = 0x95;
    obj->arg.h.unk3a = target;
    angle = rand() >> 3;
    obj->pos.vx = (rsin(angle) * 600 >> 12) + pos->vx;
    obj->pos.vy = dy + pos->vy;
    obj->pos.vz = (rcos(angle) * 600 >> 12) + pos->vz;
    obj->angle.vy = rand() >> 3;
    obj->arg.b.unk38 = 0xff;
    map_object_start_action_if_idle(obj, 0x62);
    obj->arg.sh.motion_velocity = -120;
}

/* Passes the switch event id to every record whose kind listens for it. */
ADDRESS(0x800463bc, 0x24c)
void map_object_apply_marker_signal(u8 id)
{
    MapObject *obj;
    u16 i;
    u8 v;

    obj = map_objects_slots.objects;
    if (id == 0xff) {
        return;
    }
    i = 396;
    while (i--) {
        switch (obj->action) {
        case 0x54:
        case 162:
        case 163:
            if (obj->arg.b.unk38 == id) {
                obj->arg.b.unk38 = 0xff;
            }
            break;
        case 0x5f:
            if (obj->arg.b.unk38 == id) {
                obj->action_state = 1;
                obj->arg.b.unk3f = obj->arg.b.unk3f == 0;
            }
            break;
        case 0x58:
            if (obj->arg.b.unk39 == id) {
                obj->action_state = 1;
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
                obj->action_state = 1;
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
                        if (obj->action_state != 0) {
                            break;
                        }
                        obj->action_state = 1;
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
s32 map_object_check_and_consume_marker(MapObject *obj, s32 id)
{
    switch (obj->action) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 0x1b:
        if (obj->action_state != 0) {
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

/* map_object_check_and_consume_marker for every record. */
ADDRESS(0x800466e8, 0x78)
void map_object_process_marker_all(u8 id)
{
    MapObject *obj;
    u16 i;

    obj = map_objects_slots.objects;
    if (id == 0xff) {
        return;
    }
    i = 396;
    while (i--) {
        map_object_check_and_consume_marker(obj, id);
        obj++;
    }
}

/* World position of vertex of the record's animated model. */
ADDRESS(0x80046760, 0x124)
void map_object_sample_world_vertex(MapObject *obj, s32 vertex, VECTOR *out)
{
    SVECTOR rot;
    SVECTOR v;

    animation_sample_vertex(map_object_asset_index(obj->object_id), obj->animation_id, obj->animation_phase,
                            vertex, &v);
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
s32 player_camera_within_map_region(s32 x, s32 z, s32 w, s32 h, s32 y)
{
    s32 px;
    s32 pz;

    px = cur_avatar_status.pos.vx >> 11;
    pz = cur_avatar_status.pos.vz >> 11;
    if (x <= px && px < x + w && z <= pz && pz < z + h) {
        if (y != 0x8000) {
            if (y + 2048 < cur_avatar_status.pos.vy) {
                return 0;
            }
            if (cur_avatar_status.pos.vy < y - 3200) {
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
s32 map_object_step_offset_motion(MapObject *src, MapObject *dst, SVECTOR *from, SVECTOR *to, s32 fade,
                                  s32 steps)
{
    VECTOR a;
    VECTOR b;
    s32 t;

    switch (src->action_state) {
    case 0:
        if (src->arg.b.unk38 == 0xff) {
            return 0;
        }
        if (dst->arg.b.unk38 == 0) {
            src->arg.b.unk40 = 0;
        } else {
            src->arg.b.unk40 = steps - 1;
        }
        src->action_state = 1;
        map_object_reset(dst);
        dst->action = 0x70;
        dst->layer_mask = src->layer_mask;
        switch (dst->object_id) {
        case 0x88:
            dst->angle.vx = 0x400;
            dst->render_depth_offset = 100;
            break;
        case 0x85:
            dst->render_depth_offset = -220;
            break;
        }
        if (fade) {
            dst->animation_id = 0;
        } else {
            dst->animation_id = 0x80;
        }
        vector_rotate_yxz(&src->angle, from, &dst->pos);
        dst->pos.vx += src->pos.vx;
        dst->pos.vy += src->pos.vy;
        dst->pos.vz += src->pos.vz;
        dst->angle.vy = src->angle.vy;
        dst->arg.h.unk40 = src - map_objects_slots.objects;
        /* fall through */
    case 1:
        if (fade) {
            dst->animation_phase += 204;
            if (dst->animation_phase >= 4096) {
                dst->animation_phase = 0xfff;
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
            dst->animation_phase = 0xfff;
        }
        if ((s16)asset_status.loading == 0) {
            map_object_play_spatial_sound(dst, 0x12);
        }
        dst->arg.b.unk38 = 0xff;
        src->action_state = 0x63;
        return 1;
    }
    return 0;
}

/* Runs frames while the screen tint (mode) steps its level from phase to last. */
ADDRESS(0x80046c00, 0xb0)
void render_frames_with_color_overlay(int mode, s32 phase, s32 last, s32 step)
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
        render_set_color_overlay(mode, v, v, v);
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
        if (level == end) {
            break;
        }
        level += increment;
    }
}

/* Moves the player, the Object4c and the Actor records by (dx, dy, dz). */
ADDRESS(0x80046cb0, 0x11c)
void translate_player_effects_and_actors(s32 dx, s32 dy, s32 dz)
{
    Object4c *o4c;
    Actor *o88;
    u16 i;

    cur_avatar_status.pos.vx += dx;
    cur_avatar_status.pos.vz += dz;
    cur_avatar_status.pos.vy += dy;
    o4c = effect_status.objects;
    i = 128;
    while (i--) {
        if (o4c->unk0 != 0xff && (o4c->unk8 & 0xc) != 0xc) {
            o4c->pos.vx += dx;
            o4c->pos.vz += dz;
            o4c->pos.vy += dy;
        }
        o4c++;
    }
    o88 = actor_status.objects;
    i = 200;
    while (i--) {
        if (o88->slot_state != 0xff) {
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
s16 map_object_compute_aim_vector(MapObject *obj, VECTOR *from, s32 dist, VECTOR *to, SVECTOR *out, s16 pitch,
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
        vector_add_scaled_delta(from, &cur_avatar_status.unk11c,
                                fixed_vector3_length(to->vx - pos.vx, to->vy - pos.vy,
                                                     to->vz - pos.vz) / dist, &pos);
    }
    return rot.vy;
}

/* Frame-time reference yaws of the door kinds 0x1b, 0 and 1 while they swing (map_object_update_actions). */
DATA(0x8009c3d4, 0x2, ".sbss")
static s16 map_object_door1b_view_yaw;
DATA(0x8009c3d8, 0x2, ".sbss")
static s16 map_object_door0_view_yaw;
DATA(0x8009c3dc, 0x2, ".sbss")
static s16 map_object_door1_view_yaw;

/* Moves an MapObject forward along its yaw by DIST (vector2i_scale_shift11 scale). */
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
 * Steps every live MapObject record by its kind (doors, lifts, levers, switches,
 * warps, falling items); kinds without a case go to FDAT handler 9 with the
 * record and its def in map_objects_slots.unk8858 / unk8854.
 */
ADDRESS(0x80047010, 0x4514)
void map_object_update_actions(void)
{
    MapObject *obj;
    MapObjectDef *def;
    s32 i;

    obj = map_objects_slots.objects;
    i = 0x18c;
    do {
        if (obj->action == 0xff) {
            continue;
        }
        map_objects_slots.current = obj;
        def = &map_objects_slots.defs[obj->object_id];
        map_objects_slots.current_def = def;
        switch (obj->action) {
        case 3:
            switch (obj->action_state) {
            case 0:
                break;
            case 1:
                if (obj->animation_phase == 0) {
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door3_object_pattern, 1, 0xff);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                obj->animation_phase += 72;
                if (obj->animation_phase == 0xc18) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door_shape_pattern, 1, 0xff);
                }
                if (obj->animation_phase >= 0x1000) {
                    obj->animation_phase = 0xfff;
                    obj->action_state = 2;
                }
                break;
            case 20:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x700, 0xc80, 0xc0) == 0) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door_shape_pattern, 0, 0xff);
                    obj->action_state = 21;
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                break;
            case 21:
                obj->animation_phase -= 72;
                if ((s16)obj->animation_phase <= 0) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door3_object_pattern, 0, 0xff);
                    obj->animation_phase = 0;
                    obj->action_state = 0;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            default:
                obj->action_state++;
                break;
            }
            break;
        case 4:
            switch (obj->action_state) {
            case 0:
                break;
            case 1:
                if (obj->animation_phase == 0) {
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door4_object_pattern, 1, 0xff);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                obj->animation_phase += 72;
                if (obj->animation_phase == 0xc18) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door_shape_pattern, 1, 0xff);
                }
                if (obj->animation_phase >= 0x1000) {
                    obj->animation_phase = 0xfff;
                    obj->action_state = 2;
                }
                break;
            case 20:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x700, 0xc80, 0xc0) == 0) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door_shape_pattern, 0, 0xff);
                    obj->action_state = 21;
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                break;
            case 21:
                obj->animation_phase -= 72;
                if ((s16)obj->animation_phase <= 0) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door4_object_pattern, 0, 0xff);
                    obj->animation_phase = 0;
                    obj->action_state = 0;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            default:
                obj->action_state++;
                break;
            }
            break;
        case 5:
            switch (obj->action_state) {
            case 0:
                break;
            case 1:
                if (obj->animation_phase == 0) {
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door5_object_pattern, 1, 0xff);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                obj->animation_phase += 72;
                if (obj->animation_phase == 0xc18) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door5_shape_pattern, 1, 0xff);
                }
                if (obj->animation_phase >= 0x1000) {
                    obj->animation_phase = 0xfff;
                    obj->action_state = 2;
                }
                break;
            case 20:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x700, 0xc80, 0xc0) == 0) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door5_shape_pattern, 0, 0xff);
                    obj->action_state = 21;
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                break;
            case 21:
                obj->animation_phase -= 72;
                if ((s16)obj->animation_phase <= 0) {
                    map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                                   map_object_door5_object_pattern, 0, 0xff);
                    obj->animation_phase = 0;
                    obj->action_state = 0;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            default:
                obj->action_state++;
                break;
            }
            break;
        case 2:
            switch (obj->action_state) {
            case 0:
                obj->flags |= 2;
                if (obj->arg.b.unk38 >= 150 && obj->arg.b.unk38 <= 198 && (obj->arg.b.unk38 & 1)) {
                    obj->action_state = 1;
                }
                break;
            case 1:
                if (obj->animation_phase == 0) {
                    obj->flags &= ~2;
                    map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd,
                                                 obj->arg.b.unk3c, obj->arg.b.unk39,
                                                 obj->arg.b.unk3a.bytes[0], def->arg.b.unkd, def->arg.b.unke,
                                                 obj->angle.vy, 45);
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                obj->animation_phase += 72;
                if (obj->animation_phase == 0xc18) {
                    map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                                 obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                                 def->arg.b.unke, obj->angle.vy, 45);
                }
                if (obj->animation_phase >= 0x1000) {
                    obj->animation_phase = 0xfff;
                    obj->action_state = 2;
                }
                break;
            case 20:
                if (obj->arg.b.unk38 >= 150 && obj->arg.b.unk38 <= 198 && (obj->arg.b.unk38 & 1)) {
                    break;
                }
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x1130, 0xc80, 0xc0) == 0) {
                    obj->action_state = 21;
                    map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd,
                                                 obj->arg.b.unk3c, obj->arg.b.unk39,
                                                 obj->arg.b.unk3a.bytes[0], def->arg.b.unkd, def->arg.b.unke,
                                                 obj->angle.vy, 45);
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                break;
            case 21:
                obj->animation_phase -= 72;
                if ((s16)obj->animation_phase <= 0) {
                    obj->animation_phase = 0;
                    obj->action_state = 0;
                    map_cell_copy_rotated_fields(obj->layer_mask,
                                                 obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd * 2,
                                                 obj->arg.b.unk3c, obj->arg.b.unk39,
                                                 obj->arg.b.unk3a.bytes[0], def->arg.b.unkd, def->arg.b.unke,
                                                 obj->angle.vy, 45);
                }
                break;
            default:
                obj->action_state++;
                break;
            }
            break;
        case 27:
            {
                MapObject *other;
                MapObjectDef *odef;
                s32 t;
                u32 turn;

                if (obj->arg.h.unk3e != 0xffff) {
                    other = &map_objects_slots.objects[obj->arg.h.unk3e];
                    odef = &map_objects_slots.defs[other->object_id];
                }
                if (obj->action_state == 0) {
                    break;
                }
                if (obj->flags & 0x80) {
                    turn = (vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                               cur_avatar_status.pos.vz - obj->pos.vz) -
                            obj->arg.h.base_yaw) & 0xfff;
                    if (turn <= 2048) {
                        obj->render_depth_offset = -48;
                    } else {
                        obj->render_depth_offset = 0xf0;
                    }
                }
                if (obj->arg.h.unk3e != 0xffff) {
                    other = &map_objects_slots.objects[obj->arg.h.unk3e];
                    if (other->flags & 0x80) {
                        turn = (vector_xz_to_angle(cur_avatar_status.pos.vx - other->pos.vx,
                                                   cur_avatar_status.pos.vz - other->pos.vz) -
                            obj->arg.h.base_yaw) & 0xfff;
                    if (turn <= 2048) {
                            other->render_depth_offset = 0xf0;
                        } else {
                            other->render_depth_offset = -48;
                        }
                    }
                } else {
                    other = NULL;
                }
                if (obj->action_state == 1) {
                    obj->action_state = 2;
                    obj->arg.h.unk40 = 0;
                    map_object_door1b_view_yaw = scene_display_sys.unk15b08.vy;
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                t = obj->arg.sh.unk40++;
                if (t < 32) {
                    if (t < 21 && angle_within_tolerance(map_object_door1b_view_yaw + 2048,
                                                         obj->arg.h.base_yaw, 900)) {
                        if ((other != NULL &&
                             angle_within_tolerance(map_object_door1b_view_yaw,
                                                    vector_xz_to_angle(other->pos.vx - cur_avatar_status.pos.vx,
                                                                       other->pos.vz - cur_avatar_status.pos.vz),
                                                    0x400)) ||
                            angle_within_tolerance(cur_avatar_status.unk124.vy,
                                                   vector_xz_to_angle(obj->pos.vx - cur_avatar_status.pos.vx,
                                                                      obj->pos.vz - cur_avatar_status.pos.vz),
                                                   0x400)) {
                            {
                            MATRIX m;
                            SVECTOR v;
                            VECTOR out;

                            v.vx = 0;
                            v.vy = 0;
                            v.vz = -76;
                            matrix_set_rotation_y(obj->arg.h.base_yaw + 2048, &m);
                            ApplyMatrix(&m, &v, &out);
                            cur_avatar_status.unk162 = 0;
                            cur_avatar_status.unk164 = -76;
                            cur_avatar_status.pos.vx += out.vx;
                            cur_avatar_status.pos.vz += out.vz;
                            }
                        }
                    }
                    obj->animation_id = 0x81;
                    obj->angle.vy -= 32;
                    if (other != NULL) {
                        other->angle.vy += 32;
                        other->animation_id = 0x81;
                    }
                    if (t == 24) {
                        map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1],
                                                     obj->arg.b.unk3c, obj->arg.b.unk39,
                                                     obj->arg.b.unk3a.bytes[0], 3, 2, obj->arg.sh.base_yaw,
                                                     45);
                    } else if (t == 31) {
                        obj->animation_id = 0x82;
                        obj->arg.h.unk40 = 280;
                        if (other != NULL) {
                            other->animation_id = 0x82;
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
                            matrix_set_rotation_y(obj->arg.h.base_yaw, &m);
                            ApplyMatrix(&m, &v, &out);
                            out.vx += obj->pos.vx;
                            out.vz += obj->pos.vz;
                            if (collision_query_world(out.vx, obj->pos.vy, out.vz, 3000,
                                                      obj->collision_height, 0xc0) != 0) {
                                obj->arg.h.unk40 = 300;
                                break;
                            }
                            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + 3,
                                                         obj->arg.b.unk3c, obj->arg.b.unk39,
                                                         obj->arg.b.unk3a.bytes[0], 3, 2,
                                                         obj->arg.sh.base_yaw, 45);
                            map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                        }
                        obj->animation_id = 0x81;
                        obj->angle.vy += 32;
                        if (other != NULL) {
                            other->angle.vy -= 32;
                            other->animation_id = 0x81;
                        }
                    } else {
                        obj->action_state = 0;
                        obj->animation_id = 0x80;
                        obj->render_depth_offset = -50;
                        if (other != NULL) {
                            odef->action = 0x24;
                            other->animation_id = 0x80;
                            other->render_depth_offset = -50;
                        }
                    }
                }
            }
            break;
        case 0:
            {
                MapObject *other;
                MapObjectDef *odef;
                s32 t;
                u32 turn;

                if (obj->arg.h.unk3e != 0xffff) {
                    other = &map_objects_slots.objects[obj->arg.h.unk3e];
                    odef = &map_objects_slots.defs[other->object_id];
                }
                if (obj->action_state == 0) {
                    break;
                }
                if (obj->flags & 0x80) {
                    turn = (vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                               cur_avatar_status.pos.vz - obj->pos.vz) -
                            obj->arg.h.base_yaw) & 0xfff;
                    if (turn <= 2048) {
                        obj->render_depth_offset = -48;
                    } else {
                        obj->render_depth_offset = 0xf0;
                    }
                }
                if (obj->arg.h.unk3e != 0xffff) {
                    other = &map_objects_slots.objects[obj->arg.h.unk3e];
                    if (other->flags & 0x80) {
                        turn = (vector_xz_to_angle(cur_avatar_status.pos.vx - other->pos.vx,
                                                   cur_avatar_status.pos.vz - other->pos.vz) -
                            obj->arg.h.base_yaw) & 0xfff;
                    if (turn <= 2048) {
                            other->render_depth_offset = 0xf0;
                        } else {
                            other->render_depth_offset = -48;
                        }
                    }
                } else {
                    other = NULL;
                }
                if (obj->action_state == 1) {
                    obj->action_state = 2;
                    obj->arg.h.unk40 = 0;
                    map_object_door0_view_yaw = scene_display_sys.unk15b08.vy;
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                t = obj->arg.sh.unk40++;
                if (t < 32) {
                    if (t < 21 && angle_within_tolerance(map_object_door0_view_yaw + 2048,
                                                         obj->arg.h.base_yaw, 900)) {
                        if ((other != NULL &&
                             angle_within_tolerance(map_object_door0_view_yaw,
                                                    vector_xz_to_angle(other->pos.vx - cur_avatar_status.pos.vx,
                                                                       other->pos.vz - cur_avatar_status.pos.vz),
                                                    0x400)) ||
                            angle_within_tolerance(cur_avatar_status.unk124.vy,
                                                   vector_xz_to_angle(obj->pos.vx - cur_avatar_status.pos.vx,
                                                                      obj->pos.vz - cur_avatar_status.pos.vz),
                                                   0x400)) {
                            {
                            MATRIX m;
                            SVECTOR v;
                            VECTOR out;

                            v.vx = 0;
                            v.vy = 0;
                            v.vz = -76;
                            matrix_set_rotation_y(obj->arg.h.base_yaw + 2048, &m);
                            ApplyMatrix(&m, &v, &out);
                            cur_avatar_status.unk162 = 0;
                            cur_avatar_status.unk164 = -76;
                            cur_avatar_status.pos.vx += out.vx;
                            cur_avatar_status.pos.vz += out.vz;
                            }
                        }
                    }
                    obj->animation_id = 0x81;
                    obj->angle.vy += 32;
                    if (other != NULL) {
                        other->angle.vy -= 32;
                        other->animation_id = 0x81;
                    }
                    if (t == 24) {
                        map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1],
                                                     obj->arg.b.unk3c, obj->arg.b.unk39,
                                                     obj->arg.b.unk3a.bytes[0], 3, 2, obj->arg.sh.base_yaw,
                                                     45);
                    } else if (t == 31) {
                        obj->animation_id = 0x82;
                        obj->arg.h.unk40 = 280;
                        if (other != NULL) {
                            other->animation_id = 0x82;
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
                            matrix_set_rotation_y(obj->arg.h.base_yaw, &m);
                            ApplyMatrix(&m, &v, &out);
                            out.vx += obj->pos.vx;
                            out.vz += obj->pos.vz;
                            if (collision_query_world(out.vx, obj->pos.vy, out.vz, 3000,
                                                      obj->collision_height, 0xc0) != 0) {
                                obj->arg.h.unk40 = 300;
                                break;
                            }
                            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + 3,
                                                         obj->arg.b.unk3c, obj->arg.b.unk39,
                                                         obj->arg.b.unk3a.bytes[0], 3, 2,
                                                         obj->arg.sh.base_yaw, 45);
                            map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                        }
                        obj->animation_id = 0x81;
                        obj->angle.vy -= 32;
                        if (other != NULL) {
                            other->angle.vy += 32;
                            other->animation_id = 0x81;
                        }
                    } else {
                        obj->action_state = 0;
                        obj->animation_id = 0x80;
                        obj->render_depth_offset = -50;
                        if (other != NULL) {
                            odef->action = 0x24;
                            other->animation_id = 0x80;
                            other->render_depth_offset = -50;
                        }
                    }
                }
            }
            break;
        case 1:
            {
                s32 t;
                u32 turn;

                if (obj->action_state == 0) {
                    break;
                }
                if (obj->flags & 0x80) {
                    turn = (vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                               cur_avatar_status.pos.vz - obj->pos.vz) -
                            obj->arg.h.base_yaw) & 0xfff;
                    if (turn <= 2048) {
                        obj->render_depth_offset = -48;
                    } else {
                        obj->render_depth_offset = 0xf0;
                    }
                }
                if (obj->action_state == 1) {
                    obj->action_state = 2;
                    obj->arg.h.unk40 = 0;
                    map_object_door1_view_yaw = scene_display_sys.unk15b08.vy;
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                t = obj->arg.sh.unk40++;
                if (t < 32) {
                    if (t < 21 && angle_within_tolerance(map_object_door1_view_yaw + 2048,
                                                         obj->arg.h.base_yaw,
                                                         900) &&
                        angle_within_tolerance(map_object_door1_view_yaw,
                                               vector_xz_to_angle(obj->pos.vx - cur_avatar_status.pos.vx,
                                                                  obj->pos.vz - cur_avatar_status.pos.vz),
                                               0x400)) {
                        {
                            MATRIX m;
                            SVECTOR v;
                            VECTOR out;

                            v.vx = 0;
                            v.vy = 0;
                            v.vz = -76;
                            matrix_set_rotation_y(obj->arg.h.base_yaw + 2048, &m);
                            ApplyMatrix(&m, &v, &out);
                            cur_avatar_status.unk162 = 0;
                            cur_avatar_status.unk164 = -76;
                            cur_avatar_status.pos.vx += out.vx;
                            cur_avatar_status.pos.vz += out.vz;
                            }
                    }
                    obj->animation_id = 0x81;
                    obj->angle.vy += 32;
                    if (t == 24) {
                        map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1],
                                                     obj->arg.b.unk3c, obj->arg.b.unk39,
                                                     obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                                     def->arg.b.unke, obj->arg.sh.base_yaw, 45);
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
                            matrix_set_rotation_y(obj->arg.h.base_yaw, &m);
                            ApplyMatrix(&m, &v, &out);
                            out.vx += obj->pos.vx;
                            out.vz += obj->pos.vz;
                            if (collision_query_world(out.vx, obj->pos.vy, out.vz, 1500,
                                                      obj->collision_height, 0xc0) != 0) {
                                obj->arg.h.unk40 = 300;
                                break;
                            }
                            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + 2,
                                                         obj->arg.b.unk3c, obj->arg.b.unk39,
                                                         obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                                         def->arg.b.unke, obj->arg.sh.base_yaw, 45);
                            map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                        }
                        obj->angle.vy -= 32;
                    } else {
                        obj->animation_id = 0x80;
                        obj->action_state = 0;
                        obj->render_depth_offset = -50;
                    }
                }
            }
            break;
        case 23:
            {
                MapObject *o1;
                MapObject *o2;
                VECTOR a;
                VECTOR b;
                s32 f;
                s32 n;
                static SVECTOR start = {0, -512, 0, 0};
                static SVECTOR end = {0, 0, 0, 0};

                o1 = &map_objects_slots.objects[obj->arg.h.unk3c];
                o2 = &map_objects_slots.objects[obj->arg.h.unk3a];
                switch (obj->action_state) {
                case 0:
                    if (obj->arg.b.unk38 == 0xfe) {
                        obj->action = 0xff;
                        obj->layer_mask = 0;
                        obj->action_state = 8;
                    } else {
                        map_object_set_property(obj->arg.h.unk3a, 0);
                        map_object_set_property(obj->arg.h.unk3a, 3, 0);
                        obj->action_state = 1;
                    }
                    break;
                case 1:
                    if (obj->arg.b.unk38 != 0xfe) {
                        break;
                    }
                    map_object_set_property(obj->arg.h.unk3a, 1, obj->layer_mask);
                    obj->arg.b.unk40 = 0;
                    o1->pos.vy -= 30000;
                    o1->arg.b.unk38 = 0;
                    obj->action_state = 2;
                    o1->render_depth_offset = -180;
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
                        map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                        obj->arg.b.unk40 = 0;
                        obj->action_state = 3;
                    }
                    break;
                case 3:
                    obj->arg.b.unk40++;
                    if (obj->arg.b.unk40 > 20) {
                        obj->action_state = 4;
                    }
                    break;
                case 4:
                    obj->lighting_override_index = 50;
                    obj->lighting_blend = 0x400;
                    obj->action_state = 5;
                    obj->render_mode = (obj->render_mode & 0xf8) | 5;
                    o1->lighting_override_index = 50;
                    o1->render_mode = (o1->render_mode & 0xf8) | 5;
                    break;
                case 5:
                    if (obj->lighting_blend < 0x1000) {
                        obj->lighting_blend += 192;
                        break;
                    }
                    o1->action = 0xff;
                    o1->layer_mask = 0;
                    o1->object_id = 0xff;
                    obj->lighting_blend = 0x1000;
                    obj->layer_mask = 0;
                    obj->arg.b.unk40 = 0;
                    obj->action_state = 6;
                    break;
                case 6:
                    obj->arg.b.unk40++;
                    if (obj->arg.b.unk40 < 32) {
                        o2->pos.vy -= 48;
                        break;
                    }
                    o2->action = 0x10;
                    o2->arg.w.unk40 = o2->pos.vy;
                    obj->action_state = 7;
                    break;
                case 7:
                    map_object_set_property(obj->arg.h.unk3a, 2);
                    map_object_set_property(obj->arg.h.unk3a, 3, 0);
                    obj->action = 0xff;
                    obj->action_state = 8;
                    break;
                }
            }
            break;
        case 6:
            {
            Object4cTrail spark;

            switch (obj->action_state) {
            case 0:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->angle.vx = 0xa00;
                    obj->action_state = 3;
                } else {
                    map_object_set_property(obj->arg.h.unk3a, 0);
                    map_object_set_property(obj->arg.h.unk3a, 3, 0);
                    obj->action_state = 1;
                }
                break;
            case 1:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->arg.sh.unk40 = -16;
                    obj->action_state = 2;
                    map_object_set_property(obj->arg.h.unk3a, 1, obj->layer_mask);
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                break;
            case 2:
                {
                    if (obj->arg.b.unk3c != 0xff) {
                        spark.pos.vx = obj->pos.vx + (rcos(obj->angle.vy - 1024) * -160 >> 12);
                        spark.pos.vy = obj->pos.vy + 320;
                        spark.pos.vz = obj->pos.vz - (rsin(obj->angle.vy - 1024) * -160 >> 12);
                        map_object_compute_aim_vector(obj, &cur_avatar_status.pos, 400, &spark.pos,
                                                      &spark.angle, -1, 0x400, 1);
                        effect_construct_record(10, 0x83, 0x23, &spark.pos, &spark.angle, &obj->angle);
                    }
                    obj->arg.sh.unk40 = angle_velocity_step(0xa00, obj->angle.vx, obj->arg.sh.unk40, 8, 4);
                    obj->angle.vx = (obj->angle.vx + obj->arg.sh.unk40) & 0xfff;
                    if (obj->angle.vx < 3072) {
                        map_object_set_property(obj->arg.h.unk3a, 2);
                    }
                    if (obj->arg.sh.unk40 == 0 && obj->angle.vx == 0xa00) {
                        obj->action_state = 3;
                    }
                }
                break;
            }
            }
            break;
        case 7:
            switch (obj->action_state) {
            case 0:
                if (obj->arg.h.unk3a != 0xffff) {
                    MapObject *o;

                    o = &map_objects_slots.objects[obj->arg.h.unk3a];
                    o->render_depth_offset += 200;
                }
                if (obj->arg.b.unk38 == 0xfe) {
                    if (def->arg.b.unkc != 0xff) {
                        map_object_set_cell_marker(obj, 1, def->arg.b.unkc);
                    }
                    obj->animation_phase = 0xfff;
                    obj->action_state = 3;
                } else {
                    map_object_set_property(obj->arg.h.unk3a, 0);
                    map_object_set_property(obj->arg.h.unk3a, 3, 0);
                    obj->action_state = 1;
                }
                break;
            case 1:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->action_state = 2;
                    map_object_set_property(obj->arg.h.unk3a, 1, obj->layer_mask);
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                    if (def->arg.b.unkc != 0xff) {
                        map_object_set_cell_marker(obj, 1, def->arg.b.unkc);
                    }
                }
                break;
            case 2:
                obj->animation_phase += 128;
                if (obj->animation_phase >= 4095) {
                    obj->animation_phase = 0xfff;
                    map_object_set_property(obj->arg.h.unk3a, 2);
                    obj->action_state = 3;
                }
                break;
            }
            break;
        case 8:
            switch (obj->action_state) {
            case 0:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->angle.vx = 0xd44;
                    OBJ44_ADVANCE(obj, 160);
                    obj->action_state = 3;
                } else {
                    map_object_set_property(obj->arg.h.unk3a, 0);
                    map_object_set_property(obj->arg.h.unk3a, 3, 0);
                    obj->action_state = 1;
                }
                break;
            case 1:
                if (obj->arg.b.unk38 == 0xfe) {
                    obj->arg.h.unk40 = 16;
                    obj->action_state = 2;
                    map_object_set_property(obj->arg.h.unk3a, 1, obj->layer_mask);
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                break;
            case 2:
                {
                    OBJ44_ADVANCE(obj, 10);
                    if (--obj->arg.h.unk40 == 0) {
                        map_object_set_property(obj->arg.h.unk3a, 2);
                        obj->action_state = 3;
                    }
                }
                break;
            case 3:
                obj->angle.vx = angle_approach(obj->angle.vx, 0xd44, 50);
                if (obj->angle.vx == 0xd44) {
                    obj->action_state = 4;
                }
                break;
            }
            break;
        case 9:
            if (obj->arg.h.unk3a != 0xffff) {
                MapObject *o;

                o = &map_objects_slots.objects[obj->arg.h.unk3a];
                if (o->object_id != 0xff) {
                    o->layer_mask = 0;
                    o->arg.b.unk38 = 0;
                }
            }
            obj->action = 0xff;
            break;
        case 83:
            {
                s32 step;
                s32 se;

                step = 0x100;
                if (asset_status.area[0] == 13) {
                    se = def->arg.b.sound_id + 496;
                } else {
                    se = def->arg.b.sound_id;
                }
                if (obj->arg.h.unk3a != 0xffff) {
                    step = obj->arg.h.unk3a;
                }
                switch (obj->action_state) {
                case 1:
                    if (se != 15) {
                        map_object_play_spatial_sound(obj, se);
                    }
                    if (obj->arg.b.unk3c == 1) {
                        map_object_apply_marker_signal(obj->arg.b.unk39);
                    }
                    switch (obj->arg.b.unk38) {
                    case 0:
                    case 1:
                        obj->action_state = 2;
                        break;
                    case 2:
                        obj->action_state = 2;
                        obj->arg.b.unk38 = 3;
                        break;
                    case 3:
                        obj->action_state = 3;
                        obj->arg.b.unk38 = 2;
                        break;
                    }
                    break;
                case 2:
                    obj->animation_phase += step;
                    if (se == 15 && obj->animation_phase > 1024 && obj->animation_phase <= step + 1024) {
                        map_object_play_spatial_sound(obj, 15);
                    }
                    if (obj->animation_phase < 0x1000) {
                        break;
                    }
                    obj->animation_phase = 0xfff;
                arrived:
                    if (obj->arg.b.unk3c != 1) {
                        map_object_apply_marker_signal(obj->arg.b.unk39);
                    }
                    switch (obj->arg.b.unk38) {
                    case 1:
                        map_object_play_spatial_sound(obj, se);
                        obj->action_state = 4;
                        break;
                    case 0:
                        obj->action_state = 99;
                        break;
                    case 2:
                    case 3:
                        obj->action_state = 0;
                        break;
                    }
                    break;
                case 3:
                    if (se == 15 && obj->animation_phase > 3072 && obj->animation_phase <= step + 3072) {
                        map_object_play_spatial_sound(obj, 15);
                    }
                    obj->animation_phase -= step;
                    if ((s16)obj->animation_phase <= 0) {
                        obj->animation_phase = 0;
                        goto arrived;
                    }
                    break;
                case 4:
                    obj->animation_phase -= step;
                    if ((s16)obj->animation_phase <= 0) {
                        obj->action_state = 99;
                    }
                    break;
                case 9:
                    if (obj->arg.b.unk38 == 3) {
                        obj->animation_phase = 0xfff;
                    }
                    obj->action_state = 0;
                    break;
                }
            }
            break;
        case 84:
            switch (obj->action_state) {
            case 0:
                if (player_camera_within_map_region(obj->arg.b.unk39 - ((obj->arg.b.unk3a.bytes[1] - 1) >> 1),
                                                    obj->arg.b.unk3a.bytes[0] - ((obj->arg.b.unk3c - 1) >> 1),
                                                    obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                                    -obj->arg.b.unk3d[0] << 7) != 0 ||
                    obj->arg.b.unk38 == 0xff) {
                    map_cell_apply_rotated_pattern(
                        obj->arg.b.unk40, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                        map_object_cell_patterns[def->arg.b.unke * 2 + (obj->arg.b.unk3d[1] & 1)], 1, 0x80);
                    obj->action_state = 1;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                }
                break;
            case 1:
                obj->animation_phase += 256;
                if (obj->animation_phase < 1024) {
                    break;
                }
                if (obj->arg.b.unk3d[1] & 2) {
                    obj->action_state = 99;
                    obj->arg.b.unk38 = 0xff;
                } else {
                    obj->action_state = 2;
                }
                break;
            case 32:
                obj->animation_phase += 128;
                if (obj->animation_phase >= 4095) {
                    obj->animation_id = 0;
                    obj->animation_phase = 0;
                    obj->action_state = 0;
                    map_cell_apply_rotated_pattern(
                        obj->arg.b.unk40, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                        map_object_cell_patterns[def->arg.b.unke * 2 + (obj->arg.b.unk3d[1] & 1)], 0, 0);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            case 99:
                break;
            default:
                obj->action_state++;
                break;
            }
            break;
        case 95:
            switch (obj->action_state) {
            case 1:
                switch (obj->arg.b.unk3f) {
                case 0:
                    obj->action_state = 2;
                    break;
                case 1:
                    obj->action_state = 3;
                    break;
                }
                map_cell_copy_rotated_fields(2, obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0],
                                             obj->arg.b.unk3c, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                             obj->arg.b.unk3d[0], obj->arg.b.unk3d[1], obj->angle.vy, 45);
                break;
            case 2:
                obj->action_state = 99;
                map_cell_copy_rotated_fields(2, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c, obj->arg.b.unk39,
                                             obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0],
                                             obj->arg.b.unk3d[1], obj->angle.vy, 45);
                break;
            case 3:
                obj->action_state = 99;
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
                if (obj->object_id == 222) {
                    speed = obj->arg.b.unk3a.bytes[0] << 4;
                }
                switch (obj->action_state) {
                case 0:
                    if (obj->arg.b.unk38 == 0) {
                        obj->action_state = 2;
                        obj->animation_id = 1;
                        obj->animation_phase = 0;
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xfe) {
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xff ||
                        player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11,
                                                        obj->arg.b.unk3a.bytes[1],
                                                        obj->arg.b.unk3c, obj->pos.vy) != 0) {
                        obj->action_state = 1;
                        obj->animation_id = 0;
                        obj->animation_phase = 0;
                        obj->arg.b.unk40 = 0;
                        if (def->arg.b.collision_sound_id != 0xff) {
                            map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id + 496);
                        } else {
                            map_object_play_spatial_sound(obj, 30);
                        }
                    }
                    break;
                case 1:
                    if (obj->arg.b.unk3a.bytes[1] != 0xff && obj->animation_phase == 0) {
                        map_object_set_cell_marker(obj, 1, def->arg.b.unk17);
                        map_object_play_spatial_sound(obj, 30);
                    }
                    if (obj->arg.b.unk38 == 0 && obj->animation_phase >= 0x1000 - speed) {
                        obj->action_state = 2;
                        obj->animation_id = 1;
                        obj->animation_phase = 0;
                        break;
                    }
                    obj->animation_phase += speed;
                    if (obj->animation_phase >= 4095) {
                        if (obj->arg.b.unk3a.bytes[1] == 0xff) {
                            if (def->arg.b.collision_sound_id != 0xff) {
                                map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id + 496);
                            } else {
                                map_object_play_spatial_sound(obj, 30);
                            }
                            obj->animation_phase &= 0xfff;
                        } else {
                            obj->animation_phase = 0;
                            obj->action_state = 0;
                            map_object_set_cell_marker(obj, 0, def->arg.b.unk17);
                            break;
                        }
                    }
                    if (obj->animation_phase >= 2048 && obj->animation_phase < speed + 2048 && obj->arg.b.unk3a.bytes[1] == 0xff &&
                        def->arg.b.collision_sound_id != 0xff) {
                        map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id + 496);
                    }
                    map_object_sample_world_vertex(obj, def->arg.b.unkc, &p);
                    hit = collision_query_world(p.vx, p.vy, p.vz, def->arg.h.unke, def->arg.h.unk10, 0x90);
                    if (hit == 0) {
                        map_object_sample_world_vertex(obj, def->arg.b.unkd, &p);
                        hit = collision_query_world(p.vx, p.vy, p.vz, def->arg.h.unke, def->arg.h.unk10,
                                                    0x90);
                    }
                    if (hit != 0) {
                        if (obj->arg.b.unk40 == 0) {
                            obj->arg.b.unk40 = 1;
                            effect_dispatch_magic_impact(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39,
                                                         def->arg.b.impact_magic_values[0],
                                                         def->arg.b.impact_magic_values[1],
                                                         def->arg.b.impact_magic_values[2],
                                                         def->arg.b.impact_magic_values[3], 0, 0, 0, 0, 0,
                                                         &p);
                        }
                    } else {
                        obj->arg.b.unk40 = 0;
                    }
                    break;
                case 2:
                    {
                        obj->animation_phase += 64;
                        if (obj->animation_phase >= 4095) {
                            obj->animation_phase = 0xfff;
                            if (obj->arg.b.unk38 == 0xff) {
                                obj->action_state = 3;
                                obj->animation_id = 1;
                                obj->animation_phase = 0xfff;
                            }
                        }
                        map_object_sample_world_vertex(obj, def->arg.b.unkc, &q);
                        hit = collision_query_world(q.vx, q.vy, q.vz, def->arg.h.unke, def->arg.h.unk10,
                                                    0x90);
                        if (hit == 0) {
                            map_object_sample_world_vertex(obj, def->arg.b.unkd, &q);
                            hit = collision_query_world(q.vx, q.vy, q.vz, def->arg.h.unke, def->arg.h.unk10,
                                                        0x90);
                        }
                        if (hit != 0) {
                            if (obj->arg.b.unk40 == 0) {
                                obj->arg.b.unk40 = 1;
                                effect_dispatch_magic_impact(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39,
                                                             def->arg.b.impact_magic_values[0],
                                                             def->arg.b.impact_magic_values[1],
                                                             def->arg.b.impact_magic_values[2],
                                                             def->arg.b.impact_magic_values[3], 0, 0, 0, 0, 0,
                                                             &q);
                            }
                        } else {
                            obj->arg.b.unk40 = 0;
                        }
                    }
                    break;
                case 3:
                    {
                        obj->animation_phase -= 64;
                        if (obj->animation_phase >= 0x8000) {
                            obj->action_state = 0;
                            obj->animation_phase = 0;
                        }
                        map_object_sample_world_vertex(obj, def->arg.b.unkc, &r);
                        hit = collision_query_world(r.vx, r.vy, r.vz, def->arg.h.unke, def->arg.h.unk10,
                                                    0x90);
                        if (hit == 0) {
                            map_object_sample_world_vertex(obj, def->arg.b.unkd, &r);
                            hit = collision_query_world(r.vx, r.vy, r.vz, def->arg.h.unke, def->arg.h.unk10,
                                                        0x90);
                        }
                        if (hit != 0) {
                            if (obj->arg.b.unk40 == 0) {
                                obj->arg.b.unk40 = 1;
                                effect_dispatch_magic_impact(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39,
                                                             def->arg.b.impact_magic_values[0],
                                                             def->arg.b.impact_magic_values[1],
                                                             def->arg.b.impact_magic_values[2],
                                                             def->arg.b.impact_magic_values[3], 0, 0, 0, 0, 0,
                                                             &r);
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
            switch (obj->action_state) {
            case 1:
                map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                             obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0],
                                             obj->arg.b.unk3d[1], obj->angle.vy, 45);
                obj->action_state = 2;
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                break;
            case 2:
                obj->animation_phase += 256;
                if (obj->animation_phase >= 2048) {
                    obj->action_state = 3;
                }
                break;
            case 3:
                obj->animation_phase += 64;
                if (obj->animation_phase >= 4095) {
                    obj->animation_phase = 0;
                    obj->action_state = 0;
                    map_cell_copy_rotated_fields(obj->layer_mask,
                                                 obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0],
                                                 obj->arg.b.unk3c, obj->arg.b.unk39,
                                                 obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3d[0],
                                                 obj->arg.b.unk3d[1], obj->angle.vy, 45);
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                }
                break;
            case 0:
                break;
            }
            break;
        case 90:
            switch (obj->action_state) {
            case 0:
                if (obj->arg.b.unk38 == 0) {
                    obj->action_state = 1;
                    obj->animation_phase = 0;
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
                    if (player_camera_within_map_region(obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], w, h,
                                                        obj->pos.vy) != 0) {
                        obj->arg.b.unk38 = 0;
                        obj->action_state = 1;
                        obj->animation_phase = 0;
                        map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                    }
                }
                break;
            case 1:
                if (collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, 0x1130, 0xc80, 0xc0) == 0) {
                    obj->action_state = 2;
                    map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                                 obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                                 obj->arg.b.unk3d[0], obj->arg.b.unk3d[1], obj->angle.vy, 45);
                    map_object_play_spatial_sound(obj, def->arg.b.sound_id);
                }
                break;
            case 2:
                if (obj->animation_phase < 2048) {
                    obj->animation_phase += 256;
                } else if (obj->animation_phase < 4095) {
                    obj->animation_phase += 64;
                } else {
                    obj->animation_phase = 0xfff;
                    obj->action_state = 9;
                }
                break;
            }
            break;
        case 82:
            {
                s32 speed;
                s32 hit;

                speed = obj->arg.b.unk3a.bytes[0] << 2;
                switch (obj->action_state) {
                case 0:
                    if (obj->arg.b.unk38 == 0) {
                        obj->action_state = 2;
                        obj->animation_id = 1;
                        obj->animation_phase = 0;
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xfe) {
                        break;
                    }
                    if (obj->arg.b.unk3a.bytes[1] == 0xff ||
                        player_camera_within_map_region(obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3c,
                                                        obj->arg.b.unk3d[0],
                                                        obj->arg.b.unk3d[1], obj->pos.vy) != 0) {
                        obj->action_state = 1;
                        obj->animation_id = 0;
                        obj->animation_phase = 0;
                        obj->arg.b.unk40 = 0;
                        map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id);
                    }
                    break;
                case 1:
                    {
                    VECTOR p;

                    if (obj->arg.b.unk3d[0] != 0xff) {
                        if (obj->animation_phase == 0) {
                            map_object_set_cell_marker(obj, 1, def->arg.b.unk17);
                            map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id);
                        } else if (obj->animation_phase >= 1500 && obj->animation_phase < speed + 1500) {
                            map_object_play_spatial_sound(obj, 30);
                        }
                    }
                    if (obj->arg.b.unk38 == 0 && obj->animation_phase >= 0x1000 - speed) {
                        obj->action_state = 2;
                        obj->animation_id = 1;
                        obj->animation_phase = 0;
                        break;
                    }
                    obj->animation_phase += speed;
                    if (obj->animation_phase >= 4095) {
                        if (obj->arg.b.unk3d[0] != 0xff) {
                            obj->animation_phase = 0;
                            obj->action_state = 0;
                            map_object_set_cell_marker(obj, 0, def->arg.b.unk17);
                            break;
                        }
                        map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id);
                        obj->animation_phase &= 0xfff;
                    }
                    map_object_sample_world_vertex(obj, def->arg.h.vertex_index, &p);
                    hit = collision_query_world(p.vx, p.vy, p.vz, def->arg.h.unke, def->arg.h.unk10, 0x90);
                    if (hit != 0) {
                        if (obj->arg.b.unk40 == 0) {
                            obj->arg.b.unk40 = 1;
                            effect_dispatch_magic_impact(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39,
                                                         def->arg.b.impact_magic_values[0],
                                                         def->arg.b.impact_magic_values[1],
                                                         def->arg.b.impact_magic_values[2],
                                                         def->arg.b.impact_magic_values[3], 0, 0, 0, 0, 0,
                                                         &p);
                        }
                    } else {
                        obj->arg.b.unk40 = 0;
                    }
                    }
                    break;
                case 2:
                    obj->animation_phase += 64;
                    if (obj->animation_phase >= 4095) {
                        obj->animation_phase = 0xfff;
                        if (obj->arg.b.unk38 == 0xff) {
                            obj->action_state = 3;
                            obj->animation_id = 1;
                            obj->animation_phase = 0xfff;
                        }
                    }
                    break;
                case 3:
                    obj->animation_phase -= 64;
                    if (obj->animation_phase >= 0x8000) {
                        obj->action_state = 0;
                        obj->animation_phase = 0;
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
                switch (obj->action_state) {
                case 0:
                    if (obj->arg.b.unk38 == 0) {
                        obj->action_state = 2;
                        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                        obj->animation_phase = 0;
                        break;
                    }
                    obj->action_state = 1;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                    obj->animation_phase = 0;
                    obj->arg.b.unk40 = 0;
                    map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id + 496);
                    break;
                case 1:
                    {
                    VECTOR p;
                    VECTOR unused; /* never referenced; retail's frame reserves it */

                    if (obj->arg.b.unk38 == 0 && obj->animation_phase >= 0x1000 - speed) {
                        obj->action_state = 2;
                        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                        obj->animation_phase = 0;
                        break;
                    }
                    obj->animation_phase += speed;
                    if (obj->animation_phase >= 4095) {
                        map_object_play_spatial_sound(obj, def->arg.b.collision_sound_id + 496);
                        obj->animation_phase &= 0xfff;
                    }
                    map_object_sample_world_vertex(obj, def->arg.h.vertex_index, &p);
                    hit = collision_query_world(p.vx, p.vy + 2048, p.vz, def->arg.h.unke, def->arg.h.unk10,
                                                0x90);
                    if (hit != 0) {
                        if (obj->arg.b.unk40 == 0) {
                            effect_dispatch_magic_impact(hit, 0x80, NULL, 5000, 5, obj->arg.b.unk39,
                                                         def->arg.b.impact_magic_values[0],
                                                         def->arg.b.impact_magic_values[1],
                                                         def->arg.b.impact_magic_values[2],
                                                         def->arg.b.impact_magic_values[3], 0, 0, 0, 0, 0,
                                                         &p);
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
                    obj->animation_phase += 64;
                    if (obj->animation_phase >= 4095) {
                        obj->animation_phase = 0xfff;
                        if (obj->arg.b.unk38 == 0xff) {
                            obj->action_state = 3;
                            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                            obj->animation_phase = 0xfff;
                        }
                    }
                    break;
                case 3:
                    obj->animation_phase -= 64;
                    if (obj->animation_phase >= 0x8000) {
                        obj->action_state = 0;
                        obj->animation_phase = 0;
                    }
                    break;
                }
            }
            break;
        case 86:
            switch (obj->action_state) {
            case 0:
                if (obj->arg.b.unk39 == 0) {
                    obj->action_state = 2;
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                    obj->arg.b.unk38 = 0;
                    obj->layer_mask = 0;
                } else {
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                    obj->arg.b.unk38 = 0x80;
                    obj->action_state = 1;
                    obj->layer_mask = 2;
                }
                break;
            case 1:
                if (obj->arg.b.unk39 == 0) {
                    obj->action_state = 2;
                }
                break;
            case 2:
                obj->scale.vx -= 128;
                obj->scale.vy -= 128;
                obj->scale.vz -= 128;
                if (obj->scale.vx <= 0) {
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                    obj->arg.b.unk38 = 0;
                    obj->layer_mask = 0;
                    if (obj->arg.b.unk39 == 0xff) {
                        obj->action_state = 3;
                    }
                }
                break;
            case 3:
                obj->layer_mask = 2;
                obj->scale.vx += 256;
                obj->scale.vy += 256;
                obj->scale.vz += 256;
                if (obj->scale.vx >= 4096) {
                    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
                    obj->action_state = 0;
                    obj->arg.b.unk38 = 0x80;
                }
                break;
            case 9:
                break;
            }
            break;
        case 231:
            if (player_camera_within_map_region(obj->arg.b.unk38, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                                obj->arg.b.unk3a.bytes[1], obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    CVECTOR c = {0x80, 0x80, 0x80, 0};
                    s32 n = 0x18c;
                    MapObject *o;

                    render_frames_with_color_overlay(0x82, 0, 0, 0);
                    render_frames_with_color_overlay(0x82, 0, 0x1000, 0x80);
                    o = map_objects_slots.objects;
                    for (; n != 0; n--, o++) {
                        if (o->action == 0xe7) {
                            o->arg.b.unk40 = 1;
                        }
                    }
                    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, -1);
                    cur_avatar_status.pos.vx = (obj->arg.b.unk3c << 11) + 1024;
                    cur_avatar_status.pos.vz = (obj->arg.b.unk3d[0] << 11) + 1024;
                    cur_avatar_status.unk160 = obj->arg.b.unk3d[1] != 1 ? 5 : 0;
                    cur_avatar_status.unk12c.vy = -(obj->arg.b.unk3f << 4);
                    player_sync_position_to_map();
                    cur_avatar_status.unk124.vy = cur_avatar_status.unk12c.vy + cur_avatar_status.unk136 +
                                             cur_avatar_status.unk13e + cur_avatar_status.unk146;
                    render_frames_with_color_overlay(0x82, 0x1000, 0x1000, 0);
                    cd_request_wait_idle();
                    render_frames_with_color_overlay(0x82, 0x1000, 0, -128);
                    render_set_color_overlay(0xff, 0, 0, 0);
                }
            } else {
                obj->arg.b.unk40 = 0;
            }
            obj->animation_phase = (obj->animation_phase + 64) & 0xfff;
            break;
        case 235:
            if (obj->arg.b.unk3f == 0) {
                break;
            }
            if (obj->arg.b.unk38 != 0xff) {
                if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, 1, 1,
                                                    obj->pos.vy) != 0) {
                    if (obj->arg.b.unk40 == 0) {
                        MapObject *o;
                        s32 n = 0x18c;
                        u8 x;
                        u8 z;
                        u8 y;
                        u8 yaw;
                        u8 area;
                        u8 area3;
                        u8 area4;

                        o = map_objects_slots.objects;
                        for (; n != 0; n--, o++) {
                            if (o->action == 0xeb) {
                                o->arg.b.unk40 = 1;
                            }
                        }
                        x = obj->arg.b.unk3a.bytes[1];
                        z = obj->arg.b.unk3c;
                        y = obj->arg.b.unk3d[0];
                        yaw = obj->arg.b.unk3d[1];
                        render_frames_with_color_overlay(1, 0, 0x1000, 0x100);
                        actor_disable_type3_transition_actors();
                        area = obj->arg.b.unk38;
                        area3 = obj->arg.b.unk39;
                        area4 = obj->arg.b.unk3a.bytes[0];
                        do {
                            cd_request_yield();
                            resource_advance_transition();
                        } while ((s16)asset_status.loading != 0);
                        if (area != asset_status.area[0]) {
                            resource_request_transition(area, area, area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
                        } else {
                            resource_request_transition(0xff, 0xff, area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
                            cur_avatar_status.unke = 1;
                        }
                        do {
                            cd_request_yield();
                            resource_advance_transition();
                        } while ((s16)asset_status.loading != 0);
                        cd_request_wait_idle();
                        cur_avatar_status.unk101 = 0;
                        cur_avatar_status.unk140 = 0;
                        cur_avatar_status.unk13e = 0;
                        cur_avatar_status.unk13c = 0;
                        cur_avatar_status.unk124 = cur_avatar_status.unk12c;
                        cur_avatar_status.pos.vx = (x << 11) + 1024;
                        cur_avatar_status.pos.vz = (z << 11) + 1024;
                        cur_avatar_status.pos.vy = -(y << 7);
                        cur_avatar_status.unk124.vy = cur_avatar_status.unk12c.vy = -(yaw << 4);
                        render_frames_with_color_overlay(1, 0x1000, 0x1000, 0);
                        cd_request_wait_idle();
                        render_frames_with_color_overlay(1, 0x1000, 0, -256);
                        resource_request_transition(0xff, 0xff, 0xff, area3, area4, 0x7f, 0x7f, 0x7f);
                    }
                } else {
                    obj->arg.b.unk40 = 0;
                }
            }
            if (asset_status.area[0] == 24) {
                if (obj->object_id == 0x144) {
                    obj->angle.vy += 128;
                }
                if (obj->object_id == 0x145) {
                    obj->angle.vy += 256;
                }
            }
            break;
        case 234:
            if (player_camera_within_map_region(obj->arg.b.unk38, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                                obj->arg.b.unk3a.bytes[1], obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    MapObject *o;
                    s32 n = 0x18c;

                    o = map_objects_slots.objects;
                    for (; n != 0; n--, o++) {
                        if (o->action == 0xe7) {
                            o->arg.b.unk40 = 1;
                        }
                    }
                    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, -1);
                    translate_player_effects_and_actors(obj->arg.sb.cell_dx << 11, 0,
                                                        obj->arg.sb.cell_dz << 11);
                    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, 1);
                }
            } else {
                obj->arg.b.unk40 = 0;
            }
            break;
        case 232:
            if (player_camera_within_map_region(obj->arg.b.unk38, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                                obj->arg.b.unk3a.bytes[1], obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    CVECTOR c = {0x80, 0x80, 0x80, 0};
                    s32 n = 0x18c;
                    MapObject *o;

                    render_zoom_screen_transition(&c.r, 320, 240, 1);
                    o = map_objects_slots.objects;
                    for (; n != 0; n--, o++) {
                        if (o->action == 0xe8) {
                            o->arg.b.unk40 = 1;
                        }
                    }
                    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, -1);
                    cur_avatar_status.pos.vx = (obj->arg.b.unk3c << 11) + 1024;
                    cur_avatar_status.pos.vz = (obj->arg.b.unk3d[0] << 11) + 1024;
                    cur_avatar_status.unk160 = obj->arg.b.unk3d[1] != 1 ? 5 : 0;
                    cur_avatar_status.unk12c.vy = -(obj->arg.b.unk3f << 4);
                    player_sync_position_to_map();
                    cur_avatar_status.unk124.vy = cur_avatar_status.unk12c.vy + cur_avatar_status.unk136 +
                                             cur_avatar_status.unk13e + cur_avatar_status.unk146;
                    render_zoom_screen_transition(&c.r, 320, 240, -1);
                }
            } else if (obj->arg.b.unk40 != 0xff) {
                obj->arg.b.unk40 = 0;
            }
            obj->animation_phase = (obj->animation_phase + 64) & 0xfff;
            break;
        case 224:
            if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                                obj->arg.b.unk39, obj->pos.vy) != 0) {
                resource_request_transition(obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3a.bytes[1],
                                            obj->arg.b.unk3c, obj->arg.b.unk3d[0], obj->arg.b.unk3d[1],
                                            obj->arg.sb.shift_x, obj->arg.sb.shift_z, obj->arg.sb.shift_y);
            }
            break;
        case 225:
            if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                                obj->arg.b.unk39, obj->pos.vy) != 0) {
                if (obj->arg.b.unk40 == 0) {
                    if (!(obj->arg.b.unk3a.bytes[0] & 0x80)) {
                        obj->arg.b.unk40 = 1;
                    }
                    switch (obj->arg.b.unk3a.bytes[0] & 0xf) {
                    case 0:
                        ((void (*)(MapObject *))asset_status.handlers[3])(obj);
                        break;
                    case 1:
                        map_object_apply_marker_signal(obj->arg.b.unk3a.bytes[1]);
                        break;
                    case 2:
                        cur_evt_state.unkab[obj->arg.b.unk3a.bytes[1]] = obj->arg.b.unk3c;
                        break;
                    }
                }
            } else {
                obj->arg.b.unk40 = 0;
            }
            break;
        case 227:
            if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                                obj->arg.b.unk39, 0x8000) != 0) {
                scene_display_sys.unk15995 = obj->arg.b.unk3a.bytes[0];
                scene_display_sys.unk15996 = obj->arg.b.unk3a.bytes[1];
            }
            break;
        case 228:
            {
                u8 *movie;

                movie = &movie_state.unk384;
                if (*movie == 0xff) {
                    if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11,
                                                        obj->arg.b.unk38, obj->arg.b.unk39, 0x8000) != 0) {
                        movie_state.unk370 = 1;
                        *movie = obj->arg.b.unk3a.bytes[0];
                    }
                } else if (*movie == obj->arg.b.unk3a.bytes[0]) {
                    if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11,
                                                        obj->arg.b.unk38, obj->arg.b.unk39, 0x8000) == 0) {
                        if (movie_state.unk370 == 0) {
                            *movie = 0xff;
                        }
                        if (movie_state.unk370 == 16) {
                            *movie = 0xff;
                        }
                    }
                }
            }
            break;
        case 230:
            {
                MapObject *o;
                s32 n;

                if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                                    obj->arg.b.unk39, obj->pos.vy) != 0) {
                    if (obj->arg.b.unk40 == 0) {
                        n = obj->arg.b.unk3c;
                        obj->arg.b.unk40 = 1;
                        o = &map_objects_slots.objects[obj->arg.h.unk3a];
                        do {
                            o->render_depth_offset += obj->arg.b.unk3d[0];
                            n--;
                            o++;
                        } while (n != 0);
                    }
                } else {
                    if (obj->arg.b.unk40 == 1) {
                        o = &map_objects_slots.objects[obj->arg.h.unk3a];
                        n = obj->arg.b.unk3c;
                        do {
                            o->render_depth_offset -= obj->arg.b.unk3d[0];
                            n--;
                            o++;
                        } while (n != 0);
                    }
                    obj->arg.b.unk40 = 0;
                }
            }
            break;
        case 88:
            switch (obj->action_state) {
            case 1:
                switch (obj->arg.b.unk38) {
                case 0:
                    obj->action_state = 2;
                    map_object_play_spatial_sound(obj, 20);
                    break;
                case 1:
                    obj->action_state = 3;
                    map_object_play_spatial_sound(obj, 20);
                    break;
                }
                map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3c + obj->arg.b.unk3d[1],
                                             obj->arg.b.unk3d[0], obj->arg.b.unk3a.bytes[0],
                                             obj->arg.b.unk3a.bytes[1], obj->arg.b.unk3d[1], obj->arg.b.unk3f,
                                             obj->angle.vy, 45);
                break;
            case 2:
                obj->animation_phase -= 64;
                if ((s16)obj->animation_phase <= 0) {
                    obj->action_state = 99;
                    obj->animation_phase = 0;
                    map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3c, obj->arg.b.unk3d[0],
                                                 obj->arg.b.unk3a.bytes[0], obj->arg.b.unk3a.bytes[1],
                                                 obj->arg.b.unk3d[1], obj->arg.b.unk3f, obj->angle.vy, 45);
                }
                break;
            case 3:
                obj->animation_phase += 64;
                if (obj->animation_phase >= 4096) {
                    obj->animation_phase = 0xfff;
                    obj->action_state = 99;
                }
                break;
            }
            break;
        case 89:
            {
                MapLayer *layer;

                switch (obj->action_state) {
                case 1:
                    obj->action_state = 2;
                    obj->arg.h.unk40 = obj->arg.h.unk3a;
                    obj->layer_mask = obj->arg.b.unk41[1];
                    break;
                case 2:
                    if (obj->arg.h.unk40-- == 0) {
                        obj->action_state = 3;
                        {
                            MapCell *row = map_world.cells[obj->pos.vz >> 11];

                            layer = row[obj->pos.vx >> 11].layer;
                        }
                        if (obj->layer_mask != 1) {
                            layer++;
                        }
                        layer->collision_shape_id = 0x74;
                    }
                    break;
                case 3:
                    obj->lighting_blend -= 256;
                    if ((s16)obj->lighting_blend <= 0) {
                        obj->lighting_blend = 0;
                        obj->action_state = 4;
                        obj->arg.h.unk40 = obj->arg.h.unk3c;
                    }
                    break;
                case 4:
                    if (obj->arg.h.unk40-- == 0) {
                        obj->action_state = 5;
                    }
                    break;
                case 5:
                    obj->lighting_blend += 256;
                    if (obj->lighting_blend >= 4096) {
                        obj->lighting_blend = 0x1000;
                        obj->action_state = 0;
                        {
                            MapCell *row = map_world.cells[obj->pos.vz >> 11];

                            layer = row[obj->pos.vx >> 11].layer;
                        }
                        if (obj->layer_mask != 1) {
                            layer++;
                        }
                        layer->collision_shape_id = 0x75;
                        obj->layer_mask = 0;
                    }
                    break;
                }
            }
            break;
        case 17:
            {
                MapObject *o;
                static SVECTOR from = {0, -100, 300, 0};
                static SVECTOR to = {0, 0, 64, 0};

                o = &map_objects_slots.objects[obj->arg.b.unk39 + 380];
                if (obj->action_state != 0 && o->object_id == 0xff) {
                    obj->arg.b.unk38 = 0xff;
                    obj->action_state = 0;
                    (map_objects_slots.objects + obj->arg.b.unk3a.bytes[1])->arg.b.unk38 &=
                        ~obj->arg.b.unk3a.bytes[0];
                }
                if (map_object_step_offset_motion(obj, o, &from, &to, 0, 20) != 0) {
                    (map_objects_slots.objects + obj->arg.b.unk3a.bytes[1])->arg.b.unk38 |=
                        obj->arg.b.unk3a.bytes[0];
                }
            }
            break;
        case 24:
            {
                MapObject *o;
                static SVECTOR from = {0, -1536, 400, 0};
                static SVECTOR to = {0, -1280, 0, 0};
                MapObject *sw;

                o = &map_objects_slots.objects[obj->arg.b.unk39 + 380];
                if (o->object_id == 0x86 &&
                    (asset_status.area[0] == 23 ||
                     ((s16)asset_status.loading == 1 && asset_status.previous[0] == 23))) {
                    to.vy = -1152;
                }
                if (obj->action_state != 0 && o->object_id == 0xff) {
                    obj->arg.b.unk38 = 0xff;
                    obj->action_state = 0;
                    sw = map_objects_slots.objects + obj->arg.b.unk3a.bytes[1];
                    if (obj->arg.b.unk3d[0] != 0xff) {
                        sw->arg.b.unk38 &= ~obj->arg.b.unk3d[0];
                    } else {
                        sw->arg.b.unk38 &= ~obj->arg.b.unk3a.bytes[0];
                    }
                }
                if (map_object_step_offset_motion(obj, o, &from, &to, 0, 20) != 0) {
                    (map_objects_slots.objects + obj->arg.b.unk3a.bytes[1])->arg.b.unk38 |=
                        obj->arg.b.unk3a.bytes[0];
                }
            }
            break;
        case 26:
            {
                MapObject *o;
                static SVECTOR from = {0, -100, 300, 0};
                static SVECTOR to = {0, 0, 64, 0};
                MapObject *sw;

                o = &map_objects_slots.objects[obj->arg.b.unk39 + 350];
                if (map_object_step_offset_motion(obj, o, &from, &to, 0, 20) != 0) {
                    sw = map_objects_slots.objects + obj->arg.b.unk3a.bytes[1];
                    o->arg.b.unk38 = 0;
                    sw->arg.b.unk38 |= obj->arg.b.unk3a.bytes[0];
                    obj->action_state = 3;
                }
                switch (obj->action_state) {
                case 3:
                    obj->lighting_override_index = 50;
                    obj->render_mode = (obj->render_mode & 0xf8) | 5;
                    o->lighting_override_index = 50;
                    o->render_mode = (o->render_mode & 0xf8) | 5;
                    obj->action_state = 4;
                    break;
                case 4:
                    obj->lighting_blend += 256;
                    o->lighting_blend += 256;
                    if (obj->lighting_blend >= 4095) {
                        obj->lighting_blend = 0xfff;
                        o->lighting_blend = 0xfff;
                        obj->action_state = 5;
                    }
                    break;
                case 5:
                    obj->object_id = 0xff;
                    o->object_id = 0xff;
                    obj->action_state = 99;
                    break;
                }
            }
            break;
        case 49:
            obj->angle.vz -= 32;
            break;
        case 50:
            {
                Actor *e;

                e = NULL;
                if (obj->arg.h.unk3a != 0xffff) {
                    e = &actor_status.objects[obj->arg.h.unk3a];
                }
                if (obj->action_state == 0) {
                    if (e != NULL) {
                        obj->animation_phase = (e->animation_phase + obj->arg.h.unk3c) & 0xfff;
                    } else {
                        obj->animation_phase = (obj->animation_phase + obj->arg.h.unk38) & 0xfff;
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
            if (obj->arg.w.unk40 - (s32)cdrom_ctl.frames < 0) {
                obj->arg.w.unk40 = cdrom_ctl.frames + 30;
            }
            break;
        case 96:
            switch (obj->action_state) {
            case 0:
                {
                    s32 floor;

                    floor = collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz,
                                                         def->collision_radius, def->collision_height);
                    obj->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
                    obj->pos.vy += obj->arg.sh.motion_velocity;
                    obj->arg.sh.motion_velocity += 20;
                    if (obj->pos.vy >= floor) {
                        obj->pos.vy = floor;
                        obj->arg.sh.motion_velocity = 16;
                        obj->action_state = 1;
                    }
                }
                break;
            case 1:
                obj->angle.vx += obj->arg.sh.motion_velocity;
                obj->arg.sh.motion_velocity += 16;
                if (obj->angle.vx >= 1024) {
                    obj->angle.vx = 0x400;
                    obj->action_state = 99;
                }
                break;
            }
            break;
        case 97:
            if (obj->action_state == 0) {
                s32 floor;

                floor = collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz,
                                                     def->collision_radius, def->collision_height);
                obj->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
                obj->pos.vy += 20;
                obj->angle.vy = (obj->angle.vy + 256) & 0xfff;
                if (obj->pos.vy >= floor) {
                    obj->pos.vy = floor;
                    obj->action_state = 99;
                }
            }
            break;
        case 98:
            if (obj->action_state < 2) {
                s32 floor;
                s32 ang;

                floor = collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz,
                                                     def->collision_radius, def->collision_height);
                obj->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
                obj->pos.vy += obj->arg.sh.motion_velocity;
                effect_spawn_at_lower_bound(&obj->pos, 0x1000, 6000, 300);
                ang = obj->angle.vx;
                obj->angle.vx = (obj->action_state == 0 ? ang + 160 : ang - 160) & 0xfff;
                obj->arg.sh.motion_velocity += 30;
                if (obj->arg.sh.motion_velocity >= 0 && obj->pos.vy >= floor) {
                    if (obj->arg.sh.motion_velocity < 80) {
                        obj->pos.vy = floor;
                        obj->action_state = 2;
                    } else {
                        obj->arg.sh.motion_velocity = -(obj->arg.sh.motion_velocity >> 1);
                        obj->action_state = obj->action_state == 0;
                    }
                }
            } else if (obj->action_state == 2) {
                obj->angle.vx = angle_approach(obj->angle.vx, obj->angle.pad, 160);
                if (obj->angle.vx == obj->angle.pad) {
                    obj->action_state = 99;
                }
            }
            break;
        default:
            asset_status.handlers[9]();
            break;
        case 229:
        case 233:
            break;
        }
    } while (obj++, --i != 0);
    map_objects_slots.current = NULL;
}
