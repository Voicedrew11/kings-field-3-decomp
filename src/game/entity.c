/*
 * GAME.EXE: Actor records (WIP module): allocation, placement on the
 * map from the record's cell and offsets, set-up from its ActorGroup record,
 * script record selection and scoring, and the searches over all 200 records.
 * The RODATA claim holds actor_score_target_candidate's jump table.
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

RODATA(0x800124d0, 0x214)

/* A free record among the last ten. */
ADDRESS(0x8004b524, 0x3c)
Actor *actor_pool_find_free(void)
{
    Actor *obj;
    s32 i;

    obj = &actor_status.objects[190];
    for (i = 9; i != -1; i--) {
        if (obj->slot_state == ACTOR_FREE) {
            return obj;
        }
        obj++;
    }
    return NULL;
}

/* Places the record in its map cell and on the floor. */
ADDRESS(0x8004b560, 0xc4)
void actor_set_home_position(Actor *obj)
{
    obj->pos.vx = (obj->home_cell_x << 11) + obj->unk24;
    obj->pos.vz = (obj->home_cell_z << 11) + obj->unk22;
    obj->pos.vy = collision_sample_map_layer_height(obj->home_layer_mask, obj->pos.vx, obj->pos.vz,
                                                    obj->collision_radius, obj->collision_height);
    if (obj->pos.vy >= 0) {
        obj->pos.vy = 0;
    }
    if (obj->flags & 0x400) {
        obj->pos.vy = map_world.collision_layer_height;
    }
    if (!(obj->flags & 0x10)) {
        obj->pos.vy += obj->vertical_anchor_offset;
    }
}

ADDRESS(0x8004b624, 0x74)
void actor_copy_group_defaults(Actor *obj)
{
    ActorGroup *def;

    def = &actor_status.groups[obj->group_index];
    obj->definition_id = def->definition_id;
    obj->health = def->unk1a;
    obj->collision_radius = def->collision_radius;
    obj->collision_height = def->collision_height;
    obj->flags = def->initial_actor_flags;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = def->initial_model_scale_q12;
}

ADDRESS(0x8004b698, 0xd8)
void actor_initialize_from_group(Actor *obj)
{
    actor_copy_group_defaults(obj);
    obj->lifecycle = 1;
    obj->animation_id = 0;
    obj->animation_phase = 0;
    obj->unk11 = 0;
    obj->vertical_motion_state = 0;
    obj->target_type = 0;
    obj->target_action_state = 0xff;
    obj->target = NULL;
    if (!(obj->placement_flags & 1)) {
        obj->angle.vy = rand() >> 3;
    }
    obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
    obj->turn_rate = 0;
    if (obj->definition_id < 50) {
        obj->lighting_override_index = 0x3e;
    } else {
        obj->lighting_override_index = 0x37;
    }
    obj->lighting_blend = 0x800;
    if (obj->flags & 0x80) {
        obj->render_mode |= 7;
    } else {
        obj->render_mode = 0;
    }
    map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->collision_radius, 1);
}

ADDRESS(0x8004b770, 0x24)
void actor_set_lifecycle_and_home_position(Actor *obj)
{
    obj->lifecycle = 3;
    actor_set_home_position(obj);
}

ADDRESS(0x8004b794, 0xd4)
void actor_disable_type3_transition_actors(void)
{
    Actor *obj;
    s32 i;

    obj = actor_status.objects;
    for (i = 199; i != -1; i--) {
        if (obj->slot_state == 1 && obj->lifecycle == 1 && obj->target_type == 3
            && obj->target_action_state == 0xf0) {
            if (obj->target_phase != 0 || obj->animation_phase > 0x800) {
                asset_status.handlers->on_actor_removed(obj);
                actor_set_lifecycle_and_home_position(obj);
            }
        }
        obj++;
    }
}

ADDRESS(0x8004b868, 0x5c)
void actor_prepare_and_initialize(Actor *obj)
{
    obj->angle.vx = obj->angle.vz = 0;
    obj->layer_mask = obj->home_layer_mask;
    obj->angle.vy = obj->unk20;
    actor_set_home_position(obj);
    actor_initialize_from_group(obj);
    if (obj->slot_state == 3) {
        obj->layer_mask = 0;
    }
}

ADDRESS(0x8004b8c4, 0x38)
void actor_prepare_and_initialize_by_index(u16 index)
{
    actor_prepare_and_initialize(&actor_status.objects[index]);
}

ADDRESS(0x8004b8fc, 0x50)
void actor_pool_clear(void)
{
    Actor *obj;
    u16 i;

    actor_status.actor_update_frame_count = 0;
    obj = actor_status.objects;
    i = 0;
    while (i < 200) {
        i++;
        obj->slot_state = ACTOR_FREE;
        obj->lifecycle = 0;
        obj->animation_cache = 0;
        obj++;
    }
    actor_status.actor_overlap_exclusion_flags = 0;
}

/* Starts a script record (0 stops the script). */
ADDRESS(0x8004b94c, 0x38)
void actor_set_target(Actor *obj, u8 *script)
{
    obj->previous_target_type = obj->target_type;
    if (script != NULL) {
        obj->target = script;
        obj->target_type = script[0];
        obj->target_action_state = 0;
    } else {
        obj->target = NULL;
        obj->target_type = 0xff;
        obj->target_action_state = 0xff;
    }
}

/*
 * Scores target record script for the current Actor at player distance dist;
 * actor_select_best_target starts the best one (0: not now, -1: never by
 * score). Past the type and the two chances (script[2] to start the record,
 * script[3] while it runs), the fields depend on the type: ranges are u16
 * words, cone half-widths are bytes in units of 16 or 32. Types from 128
 * ask the area module first.
 */
ADDRESS(0x8004b984, 0x598)
s32 actor_score_target_candidate(u8 *script, s32 dist)
{
    Actor *obj;
    s32 score; /* unassigned on some rejection paths (target types 9 and 128+) */
    s32 near;

    obj = actor_status.current;
    if (script[0] == 0xff) {
        return 0;
    }
    switch (script[0]) {
    case 0:
        if (script == obj->target) {
            if (SCRIPT_U16(script, 12) < dist) {
                score = random_triangular_scaled(script[3]);
            } else {
                score = 0;
            }
        } else {
            if (SCRIPT_U16(script, 12) < dist) {
                score = random_triangular_scaled(script[2]);
            } else {
                score = 0;
            }
        }
        break;
    case 1:
        if (script == obj->target) {
            score = 0;
            if (SCRIPT_U16(script, 16) < dist && SCRIPT_U16(script, 18) >= dist) {
                score = random_triangular_scaled(script[3]);
            }
        } else {
            if (SCRIPT_U16(script, 16) < dist && SCRIPT_U16(script, 18) >= dist) {
                score = random_triangular_scaled(script[2]);
            } else {
                score = 0;
            }
        }
        break;
    case 5:
    case 13:
        if (script == obj->target) {
            score = 0;
            if (SCRIPT_U16(script, 18) >= dist) {
                score = random_triangular_scaled(script[3]);
            }
        } else {
            score = 0;
            if (SCRIPT_U16(script, 16) >= dist) {
                score = random_triangular_scaled(script[2]);
            }
            switch (obj->target_type) {
            case 4:
            case 18:
            case 23:
            case 24:
            case 132:
                score <<= 1;
                break;
            }
        }
        break;
    case 9:
        if (SCRIPT_U16(script, 12) < dist) {
            score = 0;
            break;
        }
        if (cur_avatar_status.pos.vy - obj->pos.vy > -1024 && cur_avatar_status.pos.vy - obj->pos.vy < 1024
            && rand() >= 4096) {
            break;
        }
        if (angle_within_tolerance(obj->angle.vy,
                                   vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                      cur_avatar_status.pos.vz - obj->pos.vz),
                                   0x140)) {
            goto score_target;
        }
        break;
    case 4:
    case 18:
    case 23:
    case 24:
    case 132:
        if ((obj->flags & 0x100) || SCRIPT_U16(script, 26) < dist) {
            score = 0;
            break;
        }
        if (!directed_intervals_overlap(obj->pos.vy, obj->collision_height, cur_avatar_status.pos.vy + 200,
                                        0x834)) {
            score = 0;
            break;
        }
        if (!angle_within_tolerance(obj->angle.vy,
                                    vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                       cur_avatar_status.pos.vz - obj->pos.vz),
                                    script[16] << 4)) {
            score = 0;
            break;
        }
        if (script == obj->target) {
            score = random_triangular_scaled(script[3]);
            break;
        }
        score = script[2];
        near = script[14] << 6;
        if (dist >= (near + SCRIPT_U16(script, 26)) >> 1) {
            score -= score / 3;
        } else if (dist >= near) {
            score -= score / 6;
        }
        score = random_triangular_scaled(score);
        break;
    case 25:
        if (!(obj->flags & 0x100)) {
            if (SCRIPT_U16(script, 22) >= dist && dist >= SCRIPT_U16(script, 20)
                && angle_within_tolerance(obj->angle.vy,
                                          vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                             cur_avatar_status.pos.vz - obj->pos.vz),
                                          script[13] << 4)) {
                goto score_target;
            }
        } else if (script[12] == 0x27 && cur_avatar_status.paralysis_timer == 0) {
            obj->flags &= ~0x100;
        }
        score = 0;
        break;
    case 31:
        if (dist < SCRIPT_U16(script, 12)) {
            goto score_target;
        }
        score = 0;
        break;
    case 11:
        if (SCRIPT_U16(script, 26) >= dist
            && angle_within_tolerance(obj->angle.vy,
                                      vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                         cur_avatar_status.pos.vz - obj->pos.vz),
                                      0x140)) {
            goto score_target;
        }
        score = 0;
        break;
    case 19:
    case 20:
        if (SCRIPT_U16(script, 14) >= dist && cur_avatar_status.weapon_attack_phase != -1
            && angle_within_tolerance(obj->angle.vy,
                                      vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                         cur_avatar_status.pos.vz - obj->pos.vz),
                                      script[12] << 5)) {
            goto score_target;
        }
        score = 0;
        break;
    case 112:
        score = -1;
        break;
    case 2:
    case 3:
    case 22:
        score = 0;
        break;
    case 27:
        if (dist >= SCRIPT_U16(script, 12)) {
            goto score_target;
        }
        score = 0;
        break;
    default:
        if (script[0] < 128
            || asset_status.handlers->actor_target_check(script, dist)) {
        score_target:
            if (script == obj->target) {
                score = random_triangular_scaled(script[3]);
            } else {
                score = random_triangular_scaled(script[2]);
            }
        }
        break;
    }
    return score;
}
