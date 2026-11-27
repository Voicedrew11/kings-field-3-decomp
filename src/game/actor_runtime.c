/*
 * GAME.EXE: Actor records, KF2's actor_runtime.c (the same functions in the
 * same order, plus two uncalled angle helpers): allocation, placement on the
 * map from the record's cell and offsets, set-up from its ActorGroup record,
 * target record scoring and selection, wake and sleep, magic hits, the
 * searches over all 200 records, animation time, movement and steering,
 * attacks and sound, the per-frame script interpreter, the per-frame update
 * and the records made from an area's placement records.
 * One TU: actor_score_target_candidate's and actor_dispatch_group_effect's
 * jump tables share one .rdata (the 4-byte pad at 0x800126e4 aligns the
 * second table inside it).
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
#include <game/actor.h>
#include <game/sound.h>
#include <game/map_object.h>
#include <game/effect.h>

RODATA(0x800124d0, 0x7e4)

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

/* Starts the best-scoring script record of the current record. */
ADDRESS(0x8004bf1c, 0x100)
void actor_select_best_target(s32 dist)
{
    Actor *obj;
    ActorGroup *def;
    u8 **scripts;
    u8 *script;
    u8 *best;
    s32 best_score;
    s32 score;
    s32 i;

    obj = actor_status.current;
    def = actor_status.current_group;
    if (obj->target_action_state == 0xf0 || obj->target_action_state == 0) {
        return;
    }
    scripts = def->targets;
    best_score = -2;
    best = NULL;
    for (i = 15; i != -1; i--) {
        script = *scripts++;
        if (script == NULL) {
            break;
        }
        score = actor_score_target_candidate(script, dist);
        if (best_score < score) {
            best_score = score;
            best = script;
        }
    }
    if (best == NULL) {
        return;
    }
    if (best != obj->target || obj->target_action_state == 0xff) {
        actor_set_target(obj, best);
    }
}

ADDRESS(0x8004c01c, 0x4c)
void actor_select_target_for_player_distance(void)
{
    actor_select_best_target(fixed_vector2_length(actor_status.current->pos.vx - cur_avatar_status.pos.vx,
                                                  actor_status.current->pos.vz - cur_avatar_status.pos.vz));
}

/* The script record of the given id, or 0. */
ADDRESS(0x8004c068, 0x48)
u8 *actor_find_target_of_type(ActorGroup *def, u8 id)
{
    u8 **scripts;
    u8 *script;
    s32 i;

    scripts = def->targets;
    for (i = 15; i != -1; i--) {
        script = *scripts++;
        if (script == NULL) {
            break;
        }
        if (script[0] == id) {
            return script;
        }
    }
    return NULL;
}

ADDRESS(0x8004c0b0, 0x54)
void actor_select_target_type_in_own_group(Actor *obj, u8 id)
{
    actor_set_target(obj, actor_find_target_of_type(&actor_status.groups[obj->group_index], id));
}

ADDRESS(0x8004c104, 0x2c)
void actor_reset_target_and_reselect(void)
{
    actor_status.current->target_action_state = 0xff;
    actor_select_target_for_player_distance();
}

ADDRESS(0x8004c130, 0x54)
void actor_set_animation(u8 anim)
{
    Actor *obj;

    if (anim == 0xff) {
        return;
    }
    obj = actor_status.current;
    if (actor_animation_shift_enabled != 0 && actor_animation_shift_target == obj) {
        anim += 2;
    }
    obj->animation_id = anim;
    obj->animation_phase = 0;
}

ADDRESS(0x8004c184, 0x6c)
void actor_set_animation_if_changed(u8 anim)
{
    Actor *obj;

    if (anim == 0xff) {
        return;
    }
    obj = actor_status.current;
    if (obj->animation_id == anim) {
        return;
    }
    if (actor_animation_shift_enabled != 0 && actor_animation_shift_target == obj) {
        anim += 2;
    }
    obj->animation_id = anim;
    obj->animation_phase = 0;
}

/* Wakes, sleeps and retargets the current record by its distance to the player. */
ADDRESS(0x8004c1f0, 0x33c)
void actor_update_lifecycle_for_player_range(void)
{
    Actor *obj = actor_status.current;
    ActorGroup *def = actor_status.current_group;
    u8 kind = obj->slot_state;
    s32 dist;

    switch (obj->lifecycle) {
    case 0:
        dist = vector_distance_to_point(&obj->pos, cur_avatar_status.pos.vx, 0xffff, cur_avatar_status.pos.vz,
                                        (def->activation_range_cells + 1) << 11, 0, 0);
        if (dist == -1) {
            return;
        }
        if (kind == 3 || kind == 4) {
            if (actor_status.other_actor->lifecycle != 1) {
                return;
            }
            actor_prepare_and_initialize(actor_status.current);
            actor_select_best_target(dist);
            return;
        }
        if (kind == 2) {
            u8 chance = obj->spawn_chance;
            if (chance != 0xff && chance < rand() >> 4) {
                return;
            }
        } else {
            if (dist < def->activation_range_cells << 11
                && cur_avatar_status.force_actor_lifecycle_refresh == 0) {
                goto wake;
            }
            if (kind != 1) {
                if (kind != 0) {
                    goto wake;
                }
                if (obj->spawn_chance == 0 || obj->spawn_chance < rand() >> 7) {
                    goto wake;
                }
            }
        }
        if (actor_find_overlap_excluding_target_type3(obj->pos.vx, obj->pos.vy, obj->pos.vz,
                                                      def->collision_radius, def->collision_height) != -1) {
            if (kind != 2) {
                obj->lifecycle = 2;
            }
            return;
        }
        actor_prepare_and_initialize(actor_status.current);
        {
            u8 *script = actor_find_target_of_type(def, 0x15);
            if (script == NULL) {
                script = actor_find_target_of_type(def, 6);
            }
            if (script == NULL) {
                script = actor_find_target_of_type(def, 0x1a);
            }
            if (script != NULL) {
                actor_set_target(obj, script);
                return;
            }
        }
        actor_select_best_target(dist);
        return;
    wake:
        if (kind != 2) {
            obj->lifecycle = 2;
        }
        return;
    case 1:
        dist = vector_distance_to_point(&obj->pos, cur_avatar_status.pos.vx, 0xffff, cur_avatar_status.pos.vz,
                                        def->deactivation_range_cells << 11, 0, 0);
        if (dist != -1) {
            return;
        }
        map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->collision_radius, -1);
        obj->lifecycle = 0;
        actor_set_home_position(obj);
        return;
    case 2:
        if (kind == 3 || kind == 4) {
            if (actor_status.other_actor->lifecycle == 1) {
                return;
            }
        } else {
            dist = vector_distance_to_point(&obj->pos, cur_avatar_status.pos.vx, 0xffff,
                                            cur_avatar_status.pos.vz, def->deactivation_range_cells << 11, 0,
                                            0);
            if (dist != -1) {
                return;
            }
        }
        obj->lifecycle = 0;
        actor_set_home_position(obj);
        return;
    }
}

/* Restarts or puts back every live record made from record kind. */
ADDRESS(0x8004c52c, 0xc0)
void actor_retarget_or_disable_group_members(u16 kind)
{
    Actor *obj;
    s16 i;

    obj = actor_status.objects;
    for (i = 199; i != -1; i--) {
        if (obj->slot_state != ACTOR_FREE && obj->group_index == kind) {
            if (obj->lifecycle == 1) {
                actor_select_target_type_in_own_group(obj, 3);
            } else {
                actor_set_lifecycle_and_home_position(obj);
            }
        }
        obj++;
    }
}

ADDRESS(0x8004c5ec, 0x7c)
s32 actor_magic_component_curve(s32 a, s32 b, s32 c)
{
    s32 d;

    a <<= 4;
    b <<= 4;
    c <<= 4;
    if (b == 0) {
        return 0;
    }
    b += a;
    d = b - c;
    if (d < 0) {
        d = 0;
    }
    if (c == 0) {
        c = 16;
    }
    return d + (b * b) / (c * 2);
}

/*
 * Hits record index: attack stats a2..a9 of power a1 against the record's
 * resistances (actor_magic_component_curve each), strength a10 and flags a11 (bits 6-7
 * the attacker kind, bits 4-5 the hit kind); FDAT slot 18 adjusts the
 * damage. Starts the record's hurt or death script and pushes it away from
 * target (NULL: no push).
 */
ADDRESS(0x8004c668, 0x844)
void actor_apply_magic_to_actor(u16 index, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6, u16 a7, u16 a8,
                                u16 a9, u16 a10, u16 a11, MagicRecord *rec, VECTOR *target)
{
    Actor *obj;
    Actor *link;
    ActorGroup *def;
    u8 **scripts;
    u8 *script;
    u8 *dodge;
    SVECTOR dir;
    s32 dmg;
    s32 hp;
    s32 mass;
    s32 kind;
    s32 mode;
    s32 i;
    u8 chance;

    obj = &actor_status.objects[index];
    if (obj->definition_id == 0x6e) {
        return;
    }
    if (obj->definition_id == 1) {
        return;
    }
    if (obj->definition_id == 2) {
        return;
    }
    if (obj->definition_id == 0x1d) {
        return;
    }
    if (obj->definition_id == 0x16) {
        return;
    }
    if (obj->slot_state == 3) {
        obj = &actor_status.objects[obj->unk22];
    }
    def = &actor_status.groups[obj->group_index];
    if (obj->target_type == 3 && obj->animation_phase >= 1548) {
        return;
    }
    if (obj->target_type == 0x15) {
        if (obj->target_phase == 0) {
            obj->target_phase = 2;
        }
        return;
    }
    if (obj->target_type == 6 && obj->target_phase == 0) {
        obj->target_phase = 2;
    }
    if (obj->target_type == 0x1a) {
        return;
    }
    kind = a11 & 0xc0;
    mode = a11 & 0x30;
    if (kind == 0x80 && (obj->flags & 0x100000)) {
        return;
    }
    dmg = actor_magic_component_curve(a1, a2, def->defense_components[0]);
    dmg += actor_magic_component_curve(a1, a3, def->defense_components[1]);
    dmg += actor_magic_component_curve(a1, a4, def->defense_components[2]);
    dmg += actor_magic_component_curve(a1, a5, def->defense_components[3]);
    dmg += actor_magic_component_curve(a1, a6, def->defense_components[4]);
    dmg += actor_magic_component_curve(a1, a7, def->defense_components[5]);
    dmg += actor_magic_component_curve(a1, a8, def->defense_components[6]);
    dmg += actor_magic_component_curve(a1, a9, def->defense_components[7]);
    if (dmg > 0x68db7) {
        dmg = 0x68db7;
    }
    dmg = (dmg * a10 / 5000 + 128) >> 4;
    asset_status.handlers->adjust_actor_magic_damage(obj, &dmg, a2, a3, a4, a5, a6, a7, a8, a9);
    if (dmg == 0) {
        return;
    }
    if (obj->health != 0 && (kind & 0x40)) {
        if (mode & 0x20) {
            player_increment_magic_training(rec);
        } else if ((mode & 0x10) && a10 >= 2500) {
            player_increment_physical_power_training();
        }
    }
    if (mode & 0x10) {
        if (obj->target_type == 0x13 && obj->target_phase == 0x10) {
            goto push;
        }
    } else if (mode & 0x20) {
        if (obj->flags & 0x40000) {
            goto push;
        }
        dodge = actor_find_target_of_type(def, 0x16);
        if (dodge != NULL &&
            angle_within_tolerance(obj->angle.vy, vector_xz_to_angle(obj->pos.vx - target->vx,
                                                                     obj->pos.vz - target->vz), 0x200)) {
            actor_set_target(obj, dodge);
            goto push;
        }
    }
    if ((kind & 0x40) && (mode & 0x10)) {
        switch (cur_avatar_status.equipped_weapon_id) {
        case 0x1f:
            player_adjust_hp_on_weapon_hit((rand() * 9 >> 15) + 1);
            break;
        case 0x20:
            if (obj->definition_id == 59 || obj->definition_id == 60 || obj->definition_id == 61
                || obj->definition_id == 63 || obj->definition_id == 66 ||
                obj->definition_id == 67 || obj->definition_id == 79 || obj->definition_id == 88
                || obj->definition_id == 89 || obj->definition_id == 91) {
                if (obj->health != 0) {
                    dmg = obj->health;
                }
            }
            break;
        }
    }
    hp = obj->health - dmg;
    scripts = def->targets;
    if (hp <= 0) {
        if (obj->health != 0 && kind == 0x40) {
            player_add_experience(def->experience_reward);
        }
        if (obj->definition_id < 43) {
            if (cur_evt_state.first_kill_flag == 0) {
                cur_evt_state.first_kill_flag = 1;
            }
            cur_evt_state.kill_count++;
        }
        hp = 0;
        actor_select_target_type_in_own_group(obj, 3);
    } else {
        for (i = 15; i != -1; i--) {
            script = *scripts++;
            if (script == NULL) {
                break;
            }
            if (script[0] == 2) {
                if (dmg >= SCRIPT_U16(script, 12)) {
                    chance = script[2];
                    if (chance == 0xff || rand() >> 7 < chance) {
                        actor_set_target(obj, script);
                        break;
                    }
                } else {
                    obj->flags |= 0x200000;
                    obj->hit_flash_phase = 0;
                }
            }
        }
    }
    obj->health = hp;
push:
    if (obj->flags & 0x10) {
        link = &actor_status.objects[obj->unk22];
        mass = (actor_status.groups + link->group_index)->unk1[1];
    } else {
        mass = def->unk1[1];
    }
    if (target != NULL && mass < 240) {
        vector_displacement_to_pitch_yaw(obj->pos.vx - target->vx,
                                         obj->pos.vy - (obj->collision_height >> 1) - target->vy,
                                         obj->pos.vz - target->vz, &dir);
        pitch_yaw_to_forward_vector(&dir, &obj->vel);
        dmg = (SquareRoot0(SquareRoot0(dmg << 11)) << 10) / mass;
        dmg = (dmg << 5) / mass;
        if (dmg > 512) {
            dmg = 512;
        }
        vector3s_scale_shift12(dmg, &obj->vel);
        obj->vel.vx >>= 3;
        obj->vel.vz >>= 3;
        obj->vel.vy >>= 6;
        if (obj->flags & 0x10) {
            link->vel.vx = obj->vel.vx;
            link->vel.vy = obj->vel.vy;
            link->vel.vz = obj->vel.vz;
            if (link->target_type != 3) {
                actor_select_target_type_in_own_group(link, 2);
            }
        }
    }
    obj->vertical_motion_state = 0x10;
}

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

/* Falls, lands and jumps the current record (vertical_motion_state 0, 0x10 falling, 0x20 rising, 0x30
 * jumping). */
ADDRESS(0x8004e330, 0x3c8)
void actor_update_vertical_motion(void)
{
    Actor *obj = actor_status.current;
    ActorGroup *def = actor_status.current_group;
    u16 *wet = &map_world.collision_layer_offset;

    collision_probe_floor_height(obj->pos.vx, obj->pos.vy, obj->pos.vz, obj->collision_radius,
                                 ACTOR_SHAPE(obj));
    obj->layer_mask = *wet == 0 ? 1 : 2;
    if (obj->flags & 0x400) {
        map_world.collision_floor_height = map_world.collision_layer_height;
    }
    switch (obj->vertical_motion_state) {
    case 0: {
        s32 dy;

        dy = map_world.collision_floor_height - obj->pos.vy;
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
                s32 floor = map_world.collision_layer_height;

                if (obj->pos.vy < floor) {
                    goto fall;
                }
                obj->pos.vy = floor;
            } else {
                obj->pos.vy = map_world.collision_floor_height;
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
        if (map_world.collision_floor_height < obj->pos.vy && obj->vel.vy < 0) {
            return;
        }
        obj->pos.vy = map_world.collision_floor_height;
    idle:
        obj->vertical_motion_state = 0;
        return;
    case 0x30: {
        s32 jump_y;
        s32 jump_hit;

        jump_y = obj->ballistic_origin_y - obj->ballistic_launch_speed_y * obj->vel.vy
            + ((obj->ballistic_acceleration * obj->vel.vy * obj->vel.vy) >> 1);
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

/* obj's aim point (Actor.flags bits 0-1: position, offset, rotated offset). */
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
        vector_add_scaled_delta(from, &cur_avatar_status.frame_displacement,
                                fixed_vector3_length(target->vx - at.vx, target->vy - at.vy,
                                                     target->vz - at.vz) / speed, &at);
    }
    return angle.vy;
}

/* Aims a shot of speed from pos at target. */
#define AIM(speed) actor_compute_target_direction(obj, target, speed, &pos, &dir, -1, 0x400, 1)

/*
 * Fires effect kind of the current record (damage_multiplier_tenths goes to
 * effect_construct_record with it) from its model vertex (-1: the
 * offset in the next three arguments rotated by its angle; -2: between two
 * vertices, aimed at a point eased toward the player): Effect records
 * through effect_construct_record, aimed at the player. The arguments past vertex
 * are read through its address: retail homes a2/a3 like a `...` function
 * and stores vertex to its slot.
 */
ADDRESS(0x8004f414, 0xb8c)
void actor_dispatch_group_effect(s32 kind, s32 damage_multiplier_tenths, s32 vertex, ...)
{
    Actor *obj;
    Actor *o;
    Effect *shot;
    ActorGroup *def;
    VECTOR *target;
    s32 *args;
    VECTOR pos;
    VECTOR off;
    VECTOR from;
    VECTOR aim;
    SVECTOR dir;
    SVECTOR sv;
    SVECTOR rot;
    VECTOR p;
    s32 time;
    s32 pitch;
    s32 speed;
    s32 i;
    s32 delay;
    u16 n;

    args = &vertex;
    target = &cur_avatar_status.pos;
    obj = actor_status.current;
    if (vertex == -1) {
        rot.vx = args[1];
        rot.vy = args[2];
        rot.vz = args[3];
        args += 3;
        vector_rotate_yxz(&obj->angle, &rot, &off);
    } else if (vertex == -2) {
        actor_sample_rotated_animation_vertex(obj, args[1], &from);
        actor_sample_rotated_animation_vertex(obj, args[2], &off);
        args += 3;
        speed = args[0];
        aim.vx = fixed_lerp_q12(cur_avatar_status.pos.vx, ((off.vx - from.vx) << 8) + obj->pos.vx, speed);
        aim.vy = fixed_lerp_q12(cur_avatar_status.pos.vy, ((off.vy - from.vy) << 8) + obj->pos.vy, speed);
        aim.vz = fixed_lerp_q12(cur_avatar_status.pos.vz, ((off.vz - from.vz) << 8) + obj->pos.vz, speed);
        target = &aim;
    } else {
        actor_sample_rotated_animation_vertex(obj, vertex, &off);
    }
    pos.vx = obj->pos.vx + off.vx;
    pos.vy = obj->pos.vy + off.vy;
    pos.vz = obj->pos.vz + off.vz;
    switch (kind) {
    case 0:
    case 0x56:
    case 0x66:
        audio_play_spatial_default_range(0x31, &pos, 0x6e, 0);
        AIM(500);
        shot = effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        if (shot != NULL) {
            shot->cooldown = 3;
        }
        break;
    case 0xf:
        actor_compute_target_direction(obj, target, 0x200, &pos, &dir, 0, 0x400, 1);
        effect_construct_record((u8)damage_multiplier_tenths, 0x81, (u8)kind, &pos, &dir, 0, 0);
        effect_construct_record((u8)damage_multiplier_tenths, 0x81, (u8)kind, &pos, &dir, 0, 1);
        break;
    case 0x27:
        if (cur_avatar_status.paralysis_timer != 0) {
            obj->flags |= 0x100;
        }
    case 0x26:
        actor_compute_target_direction(obj, target, 0, &pos, &dir, -1, 0x400, 1);
        shot = effect_construct_record((u8)damage_multiplier_tenths, 0x81, (u8)kind, &pos, &dir);
        if (shot != NULL) {
            shot->cooldown = 0;
        }
        break;
    case 0x13:
        kind = 0x71;
        goto lob;
    case 0x29:
    case 0x7d:
        kind = 0x7d;
        goto lob;
    case 0x2d:
    case 0x81:
        kind = 0x81;
    lob:
        AIM(600);
        delay = (fixed_vector3_length(pos.vx - target->vx, pos.vy - target->vy,
                                      pos.vz - target->vz) - 2000) / 600;
        if (delay < 0) {
            delay = 0;
        }
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, delay);
        break;
    case 0xd:
        AIM(0x2a8);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        break;
    case 0xb:
        for (i = 0; i < 4; i++) {
            AIM(200);
            effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, i);
        }
        break;
    case 0x32:
        for (i = 0; i < 4; i++) {
            AIM(200);
            effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        }
        break;
    case 0x2a:
        AIM(600);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, &obj->angle);
        break;
    case 0x2c:
        AIM(0x1a4);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        break;
    case 0x35:
        AIM(300);
        effect_construct_record((u8)damage_multiplier_tenths, 0x81, (u8)kind, &pos, &dir);
        break;
    case 0x31:
        AIM(600);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        break;
    case 0x44:
        AIM(0x226);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, 0x7c, &pos, &dir);
        break;
    case 0x45:
        AIM(700);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, 0x7b, &pos, &dir);
        break;
    case 0x42:
        vector_displacement_to_pitch_yaw(target->vx - pos.vx, -(pos.vy + 1600) + target->vy,
                                         target->vz - pos.vz, &sv);
        pitch_yaw_to_forward_vector(&sv, &dir);
        vector3s_scale_shift12(0x400, &dir);
        pos.vx += dir.vx;
        pos.vy += dir.vy;
        pos.vz += dir.vz;
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, &sv);
        break;
    case 2:
    case 0x57:
        AIM(400);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, 0xfe, 2);
        break;
    case 0x2e:
        kind = 100;
    case 0x64:
        AIM(0x3c0);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        break;
    case 0x2f:
        AIM(0x1a4);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        break;
    case 0x30:
        AIM(0x1e0);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        break;
    case 7:
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, 0x1000, 0x100,
                                0x1000, 1);
        break;
    case 0x55:
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, NULL,
                                actor_status.current_index, vertex,
                                SCRIPT_U16((u8 *)args[1], 4));
        break;
    case 0x28:
        pitch_yaw_to_forward_vector(&obj->angle, &dir);
        vector3s_scale_shift12(0x226, &dir);
        shot = effect_construct_record((u8)damage_multiplier_tenths, 0x83, 0x28, &pos, &dir);
        if (shot != NULL) {
            shot->cooldown = 3;
        }
        break;
    case 6:
    case 0x33:
    case 0x34:
        AIM(300);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir);
        break;
    case 0x1f:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
        AIM(300);
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, &obj->angle);
        break;
    case 0x12:
    case 0x14:
        vector_displacement_to_pitch_yaw(target->vx - pos.vx, -(pos.vy + 1600) + target->vy,
                                         target->vz - pos.vz, &sv);
        pitch_yaw_to_forward_vector(&sv, &dir);
        if (kind == 0x12) {
            vector3s_scale_shift12(200, &dir);
        } else {
            vector3s_scale_shift12(20, &dir);
        }
        effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, &obj->angle, 600,
                                60, 0x80, 0x8c, 0xa0);
        break;
    case 0x76:
        n = SCRIPT_U16((u8 *)args[1], 4);
        o = actor_pool_find_free();
        if (o == NULL) {
            break;
        }
        AIM(400);
        o->slot_state = 5;
        o->group_index = n;
        o->unk4 = 0;
        o->placement_flags = 0;
        o->layer_mask = obj->home_layer_mask;
        o->lifecycle = 1;
        def = &actor_status.groups[n];
        o->flags = def->initial_actor_flags;
        o->render_depth_offset = def->render_depth_offset;
        o->pos.vx = pos.vx;
        o->pos.vy = pos.vy + 4096;
        o->pos.vz = pos.vz;
        actor_initialize_from_group(o);
        o->vel = dir;
        o->angle = obj->angle;
        actor_select_target_type_in_own_group(o, 0x1d);
        break;
    case 0x50:
    case 0x53:
        p = *target;
        i = 6;
        for (;;) {
            if (trajectory_solve_time_angle(0, fixed_vector2_length(p.vx - pos.vx, p.vz - pos.vz),
                                            pos.vy + (1400 - p.vy), 10, 800, &time, &pitch)) {
                pitch = 0x100;
            }
            if (--i == 0) {
                break;
            }
            vector_add_scaled_delta(target, &cur_avatar_status.frame_displacement, time >> 6, &p);
        }
        sv.vy = actor_compute_target_direction(obj, &p, 800, &pos, &dir, (s16)pitch, 0xc00, 1);
        sv.vx = pitch;
        sv.vz = 0;
        shot = effect_construct_record((u8)damage_multiplier_tenths, 0x83, (u8)kind, &pos, &dir, &sv);
        if (shot != NULL) {
            shot->arg.h.unk44 = pos.vy;
            shot->updates_remaining = 0x32;
            shot->phase = 0;
        }
        break;
    }
}

/* Note offset for obj's sound: lower for a larger record, plus a little noise. */
ADDRESS(0x8004ffa0, 0x70)
s32 actor_sound_note_offset(Actor *obj)
{
    s32 note = 16 - (s8)(obj->scale.vy >> 8);

    if (note > 12) {
        note = 12;
    } else if (note < -12) {
        note = -12;
    }
    return note + (((rand() * 5) >> 15) - 2);
}

/* Plays the sound effect of obj's running script record at obj. */
ADDRESS(0x80050010, 0x98)
void actor_play_target_sound(Actor *obj)
{
    u8 *script;

    script = obj->target;
    if (obj->flags & 0x800000) {
        audio_play_spatial_range(script[4] + 240, &obj->pos, 0x7f, 0x6000, 0x7800,
                                 actor_sound_note_offset(obj));
    } else {
        audio_play_spatial_default_range(script[4] + 240, &obj->pos, 0x6e, actor_sound_note_offset(obj));
    }
}

/*
 * Runs one frame of the current record's script record (actor_status.current
 * and .def): its sound and light fade first, then the record's behaviour by
 * target type (Actor.target_type; target_action_state 0 starts it), FDAT slot 17 for the ids this
 * module does not know, and last the ride on its target record (flag 0x10).
 * The angles passed to actor_move_along_heading are narrowed to s16 here; the FDAT
 * callers pass theirs as int.
 */
ADDRESS(0x800500a8, 0x2db4)
void actor_update_behavior(void)
{
    Actor *obj;
    ActorGroup *def;
    u8 *script;
    s32 phase;
    SVECTOR dir;
    VECTOR at;
    SVECTOR target;
    SVECTOR step;

    obj = actor_status.current;
    def = actor_status.current_group;
    script = obj->target;
    if (obj->flags & 4) {
        actor_status.actor_collision_query_flags = 3;
    } else {
        actor_status.actor_collision_query_flags = 0x93;
    }
    map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->collision_radius, -1);
    if (script[4] != 0xff) {
        s32 n;
        s32 slot;

        n = SCRIPT_U16(script, 10) % 0x4000;
        if (script[0] != 0x15 || obj->target_phase == 2) {
            switch (SCRIPT_U16(script, 10) & 0xc000) {
            case 0:
                if (actor_animation_crossed_phase(obj, n)) {
                    actor_play_target_sound(obj);
                }
                break;
            case 0x8000:
                if ((rand() >> 3) < n) {
                    actor_play_target_sound(obj);
                }
                break;
            case 0x4000:
                slot = n * actor_status.current_index / 3 % n;
                if (actor_status.actor_update_frame_count % n == slot) {
                    actor_play_target_sound(obj);
                }
                break;
            }
        }
    }
    if (obj->flags & 0x200000) {
        u8 *light;

        light = actor_find_target_of_type(def, 2);
        if (light != NULL) {
            switch (obj->hit_flash_phase) {
            case 0:
                obj->lighting_override_index = 0x3b;
                obj->lighting_blend = 0;
                obj->render_mode &= 0xf8;
                obj->render_mode |= light[14];
                obj->hit_flash_phase++;
                break;
            case 1:
                obj->lighting_blend += 512;
                if (obj->lighting_blend >= 4096) {
                    obj->lighting_blend = 0xfff;
                    obj->hit_flash_phase++;
                }
                break;
            case 2:
                obj->lighting_blend -= 512;
                if (obj->lighting_blend < 0) {
                    obj->lighting_blend = 0;
                    obj->hit_flash_phase++;
                }
                break;
            case 3:
                if (obj->definition_id < 50) {
                    obj->lighting_override_index = 0x3e;
                } else {
                    obj->lighting_override_index = 0x37;
                }
                if (obj->flags & 0x80) {
                    obj->render_mode |= 7;
                } else {
                    obj->render_mode &= 0xfb;
                }
                obj->lighting_blend = 0x800;
                obj->flags &= ~0x200000;
                break;
            }
        }
    }
    switch (obj->target_type) {
    case 2:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = 0;
            actor_set_animation(script[1]);
        }
        phase = obj->target_phase;
        if (phase < 2 || !(obj->flags & 0x800)) {
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        }
        switch (phase) {
        case 0:
            obj->lighting_override_index = 0x3b;
            obj->lighting_blend = 0;
            obj->render_mode &= 0xf8;
            obj->render_mode |= script[14];
            obj->target_phase++;
            break;
        case 1:
            if (obj->lighting_blend < 4096) {
                obj->lighting_blend += 512;
            }
            if (obj->animation_phase >= 2047) {
                obj->target_phase++;
            }
            break;
        case 2:
            if (obj->lighting_blend > 0) {
                obj->lighting_blend -= 512;
            }
            if (obj->animation_phase >= 4095) {
                obj->target_phase++;
            }
            break;
        case 3:
            if (obj->definition_id < 50) {
                obj->lighting_override_index = 0x3e;
            } else {
                obj->lighting_override_index = 0x37;
            }
            if (obj->flags & 0x80) {
                obj->render_mode |= 7;
            } else {
                obj->render_mode &= 0xfb;
            }
            obj->lighting_blend = 0x800;
            break;
        }
        if (phase == 3) {
            actor_reset_target_and_reselect();
        }
        actor_damp_horizontal_motion(def->unk1[2] * 2, 10);
        break;
    case 3: {
        u8 *light;

        phase = obj->target_phase;
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
            obj->flags &= ~0x10000;
            obj->target_phase = 0;
            light = actor_find_target_of_type(def, 2);
            if (light != NULL) {
                obj->lighting_override_index = 0x3b;
                obj->lighting_blend = 0;
                obj->render_mode &= 0xf8;
                obj->render_mode |= light[14];
            }
        }
        if (obj->target_phase == 0) {
            if (obj->animation_phase >= 1024 && (obj->flags & 0x800)) {
                break;
            }
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
            if (actor_find_target_of_type(def, 2) != NULL) {
                if (obj->animation_phase < 2048) {
                    if (obj->lighting_blend < 4096) {
                        obj->lighting_blend += 512;
                    }
                } else if (obj->lighting_blend > 0) {
                    obj->lighting_blend -= 512;
                }
            }
            if (actor_animation_crossed_phase(obj, 0x800)) {
                u16 drop;

                drop = def->gold_drop_center + random_centered_triangular_scaled(def->gold_drop_center);
                if (drop != 0) {
                    map_object_spawn_gold_drop(drop, &obj->pos, -(obj->collision_height >> 1));
                }
                if (obj->slot_state == 0 || obj->slot_state == 4) {
                    if (script[12] != 0xff && (rand() >> 7) < script[13]) {
                        map_object_spawn_effect(1, script[12], &obj->pos, -(obj->collision_height >> 1));
                    }
                } else if (obj->slot_state == 1) {
                    if (obj->death_drop_object_id != 0xff) {
                        map_object_spawn_effect(0, obj->death_drop_object_id, &obj->pos,
                                                -(obj->collision_height >> 1));
                    }
                }
            }
            if (obj->animation_phase >= 4095 || SCRIPT_U16(script, 8) == 0) {
                obj->target_phase = 15;
                if (actor_find_target_of_type(def, 2) != NULL) {
                    if (obj->definition_id < 50) {
                        obj->lighting_override_index = 0x3e;
                    } else {
                        obj->lighting_override_index = 0x37;
                    }
                    if (obj->flags & 0x80) {
                        obj->render_mode |= 7;
                    } else {
                        obj->render_mode &= 0xfb;
                    }
                    obj->lighting_blend = 0x800;
                }
            }
        } else {
            if (obj->target_phase == 99) {
                break;
            }
            obj->target_phase++;
            if (phase >= 20) {
                if (phase == 20) {
                    obj->lighting_override_index = 0x32;
                    obj->lighting_blend = 0x400;
                    obj->render_mode = (obj->render_mode & 7) | 5;
                } else if (phase < 38) {
                    if (obj->lighting_blend < 4096) {
                        obj->lighting_blend += 192;
                    } else {
                        obj->lighting_blend = 0x1000;
                    }
                } else {
                    asset_status.handlers->on_actor_removed(obj);
                    if (obj->slot_state == 1) {
                        actor_set_lifecycle_and_home_position(obj);
                    } else if (obj->slot_state == 2) {
                        obj->lifecycle = 0;
                        actor_set_home_position(obj);
                    } else if (obj->slot_state == 5) {
                        obj->slot_state = 0xff;
                        obj->lifecycle = 0;
                    } else {
                        obj->lifecycle = 2;
                        actor_set_home_position(obj);
                    }
                }
            }
        }
        actor_damp_horizontal_motion(def->unk1[2] * 2, 10);
        break;
    }
    case 0:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        actor_damp_horizontal_motion(def->unk1[2], 10);
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        break;
    case 1:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
            obj->movement_yaw = rand() >> 3;
        } else if (actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 12),
                                                     SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3], 5) ||
                   (rand() >> 5) < script[20]) {
            obj->movement_yaw = rand() >> 3;
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        break;
    case 12:
    case 16: {
        s32 flags;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf1;
            obj->unk72.vz = 0;
            obj->unk72.vy = 0;
            obj->unk72.vx = 0;
            obj->unk72.pad = collision_sample_map_layer_height(obj->home_layer_mask,
                                                               (obj->home_cell_x << 11) + obj->unk24,
                                                               (obj->home_cell_z << 11) + obj->unk22,
                                                               obj->collision_radius, obj->collision_height) +
                             obj->vertical_anchor_offset;
            actor_set_animation_if_changed(script[1]);
            flags = 3;
            actor_suspend_vertical_motion();
        } else {
            flags = actor_turn_and_move_along_euler_angles(&obj->unk72, SCRIPT_U16(script, 12),
                                                           SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3],
                                                           0x11);
            if ((rand() >> 5) < script[19]) {
                flags |= 1;
            }
            if ((rand() >> 5) < script[19]) {
                flags |= 2;
            }
            if (obj->unk72.pad + SCRIPT_U16(script, 16) < obj->pos.vy) {
                obj->vel.vy -= script[18];
            } else if (obj->pos.vy < obj->unk72.pad - SCRIPT_U16(script, 16)) {
                obj->vel.vy += script[18];
            }
        }
        if (flags & 1) {
            obj->unk72.vy = rand() >> 3;
        }
        if (flags & 2) {
            obj->unk72.vx = (rand() >> 5) - 512;
        }
        if (obj->target_type == 16) {
            actor_update_motion_animation(script[1], script[20], script[21], script[22], script[23],
                                          SCRIPT_U16(script, 8));
        } else {
            actor_advance_animation_wrapped(obj, SCRIPT_U16(script, 8));
        }
        break;
    }
    case 5: {
        s32 r;
        s32 mode;
        s32 dist;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
            switch (obj->previous_target_type) {
            case 4:
            case 18:
            case 23:
            case 24:
                ACTOR_B70(obj) = 1;
                break;
            default:
                ACTOR_B70(obj) = 0;
                break;
            }
            ACTOR_B71(obj) = 0;
        }
        if (rand() < 6000) {
            dist = fixed_vector2_length(cur_avatar_status.pos.vx - obj->pos.vx,
                                        cur_avatar_status.pos.vz - obj->pos.vz);
            if (dist >= SCRIPT_U16(script, 22)) {
                ACTOR_B70(obj) = 0;
            } else if (dist <= SCRIPT_U16(script, 20)) {
                ACTOR_B70(obj) = 1;
            }
        }
        obj->movement_yaw = vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                               cur_avatar_status.pos.vz - obj->pos.vz);
        mode = 0x21;
        if (ACTOR_B71(obj) == 0) {
            mode = 5;
        }
        if (ACTOR_B70(obj) == 0) {
            r = actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 12),
                                                  SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3], mode);
        } else {
            r = actor_move_along_heading((s16)(obj->movement_yaw + 2048), SCRIPT_U16(script, 12),
                                         def->unk1[2], mode);
        }
        if (r & 0x100) {
            if (ACTOR_B71(obj) == 0) {
                if (rand() < SCRIPT_U16(script, 24)) {
                    ACTOR_B71(obj) = 1;
                } else if (r != 0) {
                    ACTOR_B70(obj) = ACTOR_B70(obj) == 0;
                }
            }
        } else {
            ACTOR_B71(obj) = 0;
            if (r != 0) {
                ACTOR_B70(obj) = ACTOR_B70(obj) == 0;
            }
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        break;
    }
    case 13:
    case 17: {
        s32 dx;
        s32 dy;
        s32 dz;
        s32 dist;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation_if_changed(script[1]);
            obj->target_phase = 0;
        }
        dx = cur_avatar_status.pos.vx - obj->pos.vx;
        dy = cur_avatar_status.pos.vy - obj->pos.vy - 1600;
        dz = cur_avatar_status.pos.vz - obj->pos.vz;
        dist = fixed_vector3_length(dx, dy, dz);
        vector_displacement_to_pitch_yaw(dx, dy, dz, &obj->unk72);
        if (obj->unk72.vx > 0xe00) {
            obj->unk72.vx = 0xe00;
        } else if (obj->unk72.vx > 0x200) {
            obj->unk72.vx = 0x200;
        }
        if (dist >= SCRIPT_U16(script, 22)) {
            obj->target_phase = 0;
            obj->target_action_state = 0xf1;
        } else if (dist <= SCRIPT_U16(script, 20)) {
            obj->target_phase = 1;
            obj->target_action_state = 0xf0;
        }
        if (obj->target_phase == 0) {
            actor_turn_and_move_along_euler_angles(&obj->unk72, SCRIPT_U16(script, 12),
                                                   SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3], 0x11);
        } else {
            dir.vx = -512;
            dir.vy = obj->unk72.vy + 2048;
            dir.vz = 0;
            actor_turn_and_move_along_euler_angles(&dir, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14),
                                                   def->unk1[2], def->unk1[3], 0x11);
        }
        if (obj->target_type == 17) {
            actor_update_motion_animation(script[1], script[24], script[25], script[26], script[27],
                                          SCRIPT_U16(script, 8));
        } else {
            actor_advance_animation_wrapped(obj, SCRIPT_U16(script, 8));
        }
        break;
    }
    case 9:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = 0;
            actor_set_animation(script[20]);
        }
        switch (obj->target_phase) {
        case 0:
            if (actor_move_along_heading((s16)(obj->angle.vy + 2048), SCRIPT_U16(script, 18), def->unk1[2],
                                         5) == 0) {
                actor_advance_animation_wrapped(obj, SCRIPT_U16(script, 8));
                if (fixed_vector2_length(cur_avatar_status.pos.vx - obj->pos.vx,
                                         cur_avatar_status.pos.vz - obj->pos.vz) <
                    SCRIPT_U16(script, 14)) {
                    break;
                }
            }
            obj->target_phase = 1;
            actor_set_animation(script[1]);
            break;
        case 1:
            actor_advance_animation_clamped(obj, script[22]);
            if (obj->animation_phase < 4095) {
                actor_damp_horizontal_motion(def->unk1[2] * 2, 10);
                break;
            }
            if ((s16)actor_start_ballistic_motion(1, cur_avatar_status.pos.vx, cur_avatar_status.pos.vy - 500,
                                                  cur_avatar_status.pos.vz, script[23],
                                                  SCRIPT_U16(script, 16)) > 0) {
                obj->target_phase = 2;
                actor_damp_horizontal_motion(def->unk1[2] * 2, 10);
                break;
            }
            obj->target_phase = 3;
            actor_set_animation(script[21]);
            break;
        case 2:
            if (obj->vertical_motion_state == 0) {
                obj->target_phase = 3;
                actor_set_animation(script[21]);
                break;
            }
            actor_turn_and_move_along_heading(obj->angle.vy, obj->ballistic_horizontal_speed, 0,
                                              obj->ballistic_horizontal_speed, def->unk1[3], 10);
            break;
        case 3:
            actor_advance_animation_clamped(obj, script[22]);
            if (obj->animation_phase >= 4095) {
                actor_reset_target_and_reselect();
            }
            actor_damp_horizontal_motion(obj->ballistic_horizontal_speed >> 4, 10);
            break;
        }
        break;
    case 10: {
        s32 r;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf1;
            actor_set_animation_if_changed(script[1]);
            actor_suspend_vertical_motion();
        }
        r = rand();
        if (r < 2048) {
            obj->vel.vx += SCRIPT_U16(script, 16);
            if (obj->vel.vx > SCRIPT_U16(script, 12)) {
                obj->vel.vx = SCRIPT_U16(script, 12);
            }
        } else if (r < 4096) {
            obj->vel.vx -= SCRIPT_U16(script, 16);
            if (obj->vel.vx < -SCRIPT_U16(script, 12)) {
                obj->vel.vx = -SCRIPT_U16(script, 12);
            }
        }
        r = rand();
        if (r < 2048) {
            obj->vel.vz += SCRIPT_U16(script, 16);
            if (obj->vel.vz > SCRIPT_U16(script, 12)) {
                obj->vel.vz = SCRIPT_U16(script, 12);
            }
        } else if (r < 4096) {
            obj->vel.vz -= SCRIPT_U16(script, 16);
            if (obj->vel.vz < -SCRIPT_U16(script, 12)) {
                obj->vel.vz = -SCRIPT_U16(script, 12);
            }
        }
        r = rand();
        if (r < 2048) {
            obj->vel.vy += SCRIPT_U16(script, 16);
            if (obj->vel.vy > SCRIPT_U16(script, 12)) {
                obj->vel.vy = SCRIPT_U16(script, 12);
            }
        } else if (r < 4096) {
            obj->vel.vy -= SCRIPT_U16(script, 16);
            if (obj->vel.vy < -SCRIPT_U16(script, 12)) {
                obj->vel.vy = -SCRIPT_U16(script, 12);
            }
        }
        actor_move_with_collision(&obj->vel);
        actor_advance_animation_wrapped(obj, SCRIPT_U16(script, 8));
        actor_turn_toward_angle(obj, vector_xz_to_angle(obj->vel.vx, obj->vel.vz), SCRIPT_U16(script, 14),
                                def->unk1[3]);
        break;
    }
    case 132: {
        s16 *p;
        s32 c;
        s32 tries;
        u16 time;
        VECTOR *player;

        tries = 1;
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = SCRIPT_U16(script, 24);
            obj->unk72.vx = 0;
            obj->unk72.vy = 0;
            actor_set_animation(script[1]);
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        p = (s16 *)(script + 28) + obj->unk72.vx;
        for (;;) {
            c = *p;
            obj->unk72.vx++;
            p++;
            if (c == 0x8000) {
                p = (s16 *)(script + 28);
                obj->unk72.vx = 0;
                continue;
            }
            if (c == 0x8001) {
                tries = *p++;
                obj->unk72.vx++;
                continue;
            }
            if (c == 0x8003) {
                p += *p + 1;
                continue;
            }
            if (c == 0x8002) {
                at.vx = *p++;
                obj->unk72.vx++;
                at.vy = *p++;
                obj->unk72.vx++;
                at.vz = *p++;
                obj->unk72.vx++;
            } else {
                actor_sample_rotated_animation_vertex(obj, c, &at);
            }
            at.vx += obj->pos.vx;
            at.vy += obj->pos.vy;
            at.vz += obj->pos.vz;
            if (vector_distance_to_point(&at, cur_avatar_status.pos.vx, cur_avatar_status.pos.vy,
                                         cur_avatar_status.pos.vz, script[14], script[15], 0x6a4) != -1) {
                time = obj->animation_phase;
                obj->animation_phase -= obj->animation_step << 2;
                if ((s16)obj->animation_phase < 0) {
                    obj->animation_phase = 0;
                }
                actor_sample_rotated_animation_vertex(obj, c, &at);
                at.vx += obj->pos.vx;
                at.vy += (script[15] << 4) + obj->pos.vy;
                at.vz += obj->pos.vz;
                player_apply_damage(SCRIPT_U16(script, 18), SCRIPT_U16(script, 20), SCRIPT_U16(script, 22),
                                    script[17], 0, 0, 0, 0, 0, 0x1000, 10, &at);
                obj->animation_phase = time;
                break;
            }
            if (--tries == 0) {
                break;
            }
        }
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        actor_damp_horizontal_motion(script[13], 10);
        break;
    }
    case 4:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (actor_animation_crossed_phase(obj, SCRIPT_U16(script, 24))) {
            actor_try_damage_player_in_cone(0, script[14], script[15], script[16], SCRIPT_U16(script, 18),
                                            SCRIPT_U16(script, 20), SCRIPT_U16(script, 22), script[17]);
        }
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        actor_damp_horizontal_motion(script[13], 10);
        break;
    case 23:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = SCRIPT_U16(script, 24);
            actor_set_animation(script[1]);
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (actor_animation_crossed_phase(obj, obj->target_phase)) {
            obj->target_phase += SCRIPT_U16(script, 40);
            if (obj->target_phase > SCRIPT_U16(script, 38)) {
                obj->target_phase = 0;
            }
            actor_try_damage_player_in_cone(0, script[14], script[15], script[16], SCRIPT_U16(script, 18),
                                            SCRIPT_U16(script, 20), SCRIPT_U16(script, 22),
                                            script[17] | 0x80);
        }
        if (SCRIPT_U16(script, 42) != 0 && actor_animation_crossed_phase(obj, SCRIPT_U16(script, 42))) {
            actor_try_damage_player_in_cone(0, script[28], script[29], script[30], SCRIPT_U16(script, 32),
                                            SCRIPT_U16(script, 34), SCRIPT_U16(script, 36), script[31]);
        }
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        actor_damp_horizontal_motion(script[13], 10);
        break;
    case 24: {
        s32 speed;
        s32 turn;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (obj->animation_phase >= SCRIPT_U16(script, 34)) {
            speed = 0;
            turn = SCRIPT_S16(script, 38);
        } else if (obj->animation_phase >= SCRIPT_U16(script, 32)) {
            speed = SCRIPT_U16(script, 28);
            turn = SCRIPT_S16(script, 36);
        } else {
            speed = SCRIPT_U16(script, 28);
            turn = script[13];
        }
        obj->movement_yaw = vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                               cur_avatar_status.pos.vz - obj->pos.vz);
        actor_turn_and_move_along_heading(obj->movement_yaw, speed, SCRIPT_U16(script, 30), turn,
                                          def->unk1[3], 4);
        if (actor_animation_crossed_phase(obj, SCRIPT_U16(script, 24))) {
            actor_try_damage_player_in_cone(0, script[14], script[15], script[16], SCRIPT_U16(script, 18),
                                            SCRIPT_U16(script, 20), SCRIPT_U16(script, 22), script[17]);
        }
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        break;
    }
    case 11: {
        s32 hit;
        u16 spin;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation_if_changed(script[1]);
            obj->target_phase = 0;
            actor_suspend_vertical_motion();
        }
        switch (obj->target_phase) {
        case 0:
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
            if (obj->animation_phase >= 4095) {
                vector_displacement_to_pitch_yaw(cur_avatar_status.pos.vx - obj->pos.vx,
                                                 cur_avatar_status.pos.vy - obj->pos.vy,
                                                 cur_avatar_status.pos.vz - obj->pos.vz, &dir);
                pitch_yaw_to_forward_vector(&dir, &obj->unk72);
                obj->target_phase = 1;
            }
            actor_move_with_collision(&obj->vel);
            break;
        case 1:
            target = step = obj->unk72;
            vector3s_scale_shift12(SCRIPT_U16(script, 20), &target);
            vector3s_scale_shift12(SCRIPT_U16(script, 24), &step);
            obj->vel.vx = value_approach(obj->vel.vx, target.vx, step.vx);
            obj->vel.vy = value_approach(obj->vel.vy, target.vy, step.vy);
            obj->vel.vz = value_approach(obj->vel.vz, target.vz, step.vz);
            hit = actor_move_with_collision(&obj->vel);
            if (hit != 0) {
                if (hit & 0x80) {
                    player_apply_damage(SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), SCRIPT_U16(script, 18),
                                        script[13], 0, 0, 0, 0, 0, 0x1000, 10, &obj->pos);
                }
                obj->target_phase = 2;
            }
            obj->angle.vx = (obj->angle.vx + 256) & 0xfff;
            break;
        case 2:
            spin = (obj->angle.vx + 256) & 0xfff;
            obj->angle.vx = spin;
            if (spin < 256) {
                obj->angle.vx = 0;
                obj->target_phase = 3;
            }
            actor_move_with_collision(&obj->vel);
            break;
        case 3:
            actor_advance_animation_clamped(obj, -SCRIPT_U16(script, 8));
            if (obj->animation_phase == 0) {
                actor_reset_target_and_reselect();
            }
            actor_move_with_collision(&obj->vel);
            break;
        }
        actor_turn_toward_angle(obj, vector_xz_to_angle(obj->vel.vx, obj->vel.vz), SCRIPT_U16(script, 22),
                                def->unk1[3]);
        break;
    }
    case 18: {
        s32 dx;
        s32 dy;
        s32 dz;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation_if_changed(script[1]);
        }
        dx = cur_avatar_status.pos.vx - obj->pos.vx;
        dy = cur_avatar_status.pos.vy - obj->pos.vy - 1600;
        dz = cur_avatar_status.pos.vz - obj->pos.vz;
        fixed_vector3_length(dx, dy, dz);
        vector_displacement_to_pitch_yaw(dx, dy, dz, &obj->unk72);
        actor_turn_and_move_along_euler_angles(&obj->unk72, SCRIPT_U16(script, 28), SCRIPT_U16(script, 30),
                                               script[13], def->unk1[3], 6);
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (actor_animation_crossed_phase(obj, SCRIPT_U16(script, 24))) {
            actor_try_damage_player_in_cone(0, script[14], script[15], script[16], SCRIPT_U16(script, 18),
                                            SCRIPT_U16(script, 20), SCRIPT_U16(script, 22), script[17]);
        }
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        break;
    }
    case 14: {
        Actor *target;

        target = actor_status.other_actor;
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf1;
            obj->target_phase = 0;
        }
        switch (obj->target_phase) {
        case 0:
            if (target->turn_rate > 0) {
                actor_set_animation(script[13]);
                obj->target_phase = 1;
            } else if (target->turn_rate < 0) {
                actor_set_animation(script[12]);
                obj->target_phase = 2;
            }
            break;
        case 1:
            if (obj->animation_phase < 2048) {
                obj->animation_phase += 128;
            } else if (target->turn_rate <= 0) {
                obj->target_phase = 3;
            }
            break;
        case 2:
            if (obj->animation_phase < 2048) {
                obj->animation_phase += 128;
            } else if (target->turn_rate >= 0) {
                obj->target_phase = 3;
            }
            break;
        case 3:
            obj->animation_phase -= 128;
            if (obj->animation_phase == 0) {
                obj->target_phase = 0;
            }
            break;
        }
        break;
    }
    case 15:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation_if_changed(script[1]);
        }
        actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14),
                                          def->unk1[2], def->unk1[3], 5);
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        break;
    case 19:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = 0;
            actor_set_animation_if_changed(script[1]);
        }
        switch (obj->target_phase) {
        case 0:
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
            if (obj->animation_phase >= SCRIPT_U16(script, 16)) {
                obj->target_phase = 16;
            }
            break;
        case 16:
            if (cur_avatar_status.weapon_attack_phase == -1) {
                obj->target_phase = 32;
            }
            break;
        case 32:
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
            if (obj->animation_phase >= 4095) {
                actor_reset_target_and_reselect();
            }
            break;
        }
        actor_damp_horizontal_motion(def->unk1[2], 10);
        break;
    case 20:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = SCRIPT_U16(script, 18);
            actor_set_animation_if_changed(script[1]);
        }
        if (actor_move_along_heading((s16)(obj->angle.vy + 2048), SCRIPT_U16(script, 16), 1000, 5)) {
            obj->target_phase = 1;
        }
        actor_advance_animation_wrapped(obj, SCRIPT_U16(script, 8));
        if (obj->animation_phase >= 0xfff - SCRIPT_U16(script, 8) && --obj->target_phase == 0) {
            obj->animation_phase = 0xfff;
            actor_reset_target_and_reselect();
        }
        break;
    case 22:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        actor_damp_horizontal_motion(def->unk1[2], 10);
        break;
    case 21: {
        s32 mask;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = 0;
            actor_set_animation(script[1]);
            obj->collision_radius = SCRIPT_U16(script, 20);
            obj->collision_height = SCRIPT_U16(script, 22);
        }
        switch (obj->target_phase) {
        case 0:
            if (fixed_vector2_length(cur_avatar_status.pos.vx - obj->pos.vx,
                                     cur_avatar_status.pos.vz - obj->pos.vz) <
                SCRIPT_U16(script, 12)) {
                obj->target_phase = 2;
            }
            break;
        case 2:
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
            obj->collision_radius = fixed_lerp_q12(SCRIPT_U16(script, 20), def->collision_radius,
                                                   obj->animation_phase);
            obj->collision_height = fixed_lerp_q12(SCRIPT_U16(script, 22), def->collision_height,
                                                   obj->animation_phase);
            if (obj->animation_phase >= 4095) {
                obj->collision_radius = def->collision_radius;
                obj->collision_height = def->collision_height;
                obj->target_phase = 3;
                obj->unk72.vx = SCRIPT_U16(script, 16);
                actor_set_animation(script[24]);
            }
            if (obj->animation_phase >= SCRIPT_U16(script, 18)) {
                mask = actor_status.actor_collision_query_flags;
                actor_status.actor_collision_query_flags = mask & ~0x20;
                actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
                actor_status.actor_collision_query_flags = mask;
            }
            break;
        case 3:
            mask = actor_status.actor_collision_query_flags;
            actor_status.actor_collision_query_flags = mask & ~0x20;
            actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
            if (--obj->unk72.vx == -1) {
                actor_reset_target_and_reselect();
            }
            actor_status.actor_collision_query_flags = mask;
            actor_advance_animation_wrapped(obj, script[25]);
            break;
        }
        break;
    }
    case 6: {
        s32 mask;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = 0;
            actor_set_animation(script[1]);
            obj->collision_radius = SCRIPT_U16(script, 22);
            obj->collision_height = SCRIPT_U16(script, 24);
            if (SCRIPT_U16(script, 14) != 0) {
                obj->movement_yaw = rand() >> 3;
            }
        }
        switch (obj->target_phase) {
        case 0:
            actor_advance_animation_wrapped(obj, SCRIPT_U16(script, 8));
            if (fixed_vector2_length(cur_avatar_status.pos.vx - obj->pos.vx,
                                     cur_avatar_status.pos.vz - obj->pos.vz) <
                    SCRIPT_U16(script, 12) &&
                obj->animation_phase >= 0xfff - SCRIPT_U16(script, 8)) {
                obj->target_phase = 2;
                obj->animation_phase = 0;
                actor_set_animation(script[26]);
            }
            if (actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 14),
                                                  SCRIPT_U16(script, 16),
                                                  def->unk1[2], def->unk1[3],
                                                  5) ||
                (rand() >> 5) < script[28]) {
                obj->movement_yaw = rand() >> 3;
            }
            break;
        case 2:
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
            obj->collision_radius = fixed_lerp_q12(SCRIPT_U16(script, 22), def->collision_radius,
                                                   obj->animation_phase);
            obj->collision_height = fixed_lerp_q12(SCRIPT_U16(script, 24), def->collision_height,
                                                   obj->animation_phase);
            if (obj->animation_phase >= 4095) {
                obj->collision_radius = def->collision_radius;
                obj->collision_height = def->collision_height;
                obj->target_phase = 3;
                obj->unk72.vx = SCRIPT_U16(script, 18);
            } else if (obj->animation_phase >= SCRIPT_U16(script, 20)) {
                mask = actor_status.actor_collision_query_flags;
                actor_status.actor_collision_query_flags = mask & ~0x20;
                actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
                actor_status.actor_collision_query_flags = mask;
            }
            break;
        case 3:
            mask = actor_status.actor_collision_query_flags;
            actor_status.actor_collision_query_flags = mask & ~0x20;
            actor_turn_and_move_along_heading(obj->movement_yaw, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
            if (--obj->unk72.vx == -1) {
                actor_reset_target_and_reselect();
            }
            actor_status.actor_collision_query_flags = mask;
            actor_advance_animation_clamped(obj, script[27]);
            break;
        }
        break;
    }
    case 26: {
        MapObject *door;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            obj->target_phase = 0;
            actor_set_animation(script[1]);
            obj->scale.vz = 0;
            obj->scale.vy = 0;
            obj->scale.vx = 0;
        }
        switch (obj->target_phase) {
        case 0:
            door = &map_objects_slots.objects[obj->unk20];
            if (door->action_state < 2 || door->angle.vx == 0 || door->angle.vx >= 3072) {
                break;
            }
            obj->target_phase = 2;
            obj->unk72.vx = 8;
            obj->collision_radius = def->collision_radius;
            obj->collision_height = def->collision_height;
            obj->pos.vy += 2048;
            obj->scale.vx = obj->scale.vy = obj->scale.vz = def->initial_model_scale_q12;
            obj->angle.vy = door->angle.vy;
        case 2:
            obj->vertical_motion_state = 0;
            obj->pos.vy -= 256;
            if (--obj->unk72.vx == 0) {
                actor_reset_target_and_reselect();
            }
            break;
        }
        break;
    }
    case 25: {
        u16 *p;
        s32 c;
        s32 tries;
        s16 x;
        s16 y;
        s16 z;
        s32 a;
        s32 b;
        s32 d;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
            obj->target_phase = SCRIPT_U16(script, 14);
            obj->unk72.vx = 0;
            obj->unk72.vy = 0;
            if (script[7] == 1) {
                obj->vertical_motion_state = 0x10;
            }
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (actor_animation_crossed_phase(obj, obj->target_phase)) {
            obj->target_phase += SCRIPT_U16(script, 18);
            if (obj->target_phase > SCRIPT_U16(script, 16)) {
                obj->target_phase = 0;
            }
            tries = 1;
            p = (u16 *)(script + 26) + obj->unk72.vx;
            for (;;) {
                c = *p;
                obj->unk72.vx++;
                p++;
                if (c == 0x8000) {
                    p = (u16 *)(script + 26);
                    obj->unk72.vx = 0;
                    continue;
                }
                if (c == 0x8001) {
                    tries = *p++;
                    obj->unk72.vx++;
                    continue;
                }
                if (c == 0x8003) {
                    p += *p + 1;
                    continue;
                }
                if (c == 0x8002) {
                    x = *p++;
                    obj->unk72.vx++;
                    y = *p++;
                    obj->unk72.vx++;
                    z = *p++;
                    obj->unk72.vx++;
                    actor_dispatch_group_effect(script[12], SCRIPT_U16(script, 24), -1, x, y, z, p);
                } else if (c == 0x8004) {
                    a = *p++;
                    b = *p++;
                    d = *p++;
                    obj->unk72.vx += 3;
                    actor_dispatch_group_effect(script[12], SCRIPT_U16(script, 24), -2, a, b, d);
                } else {
                    actor_dispatch_group_effect(script[12], SCRIPT_U16(script, 24), c, p + tries - 1,
                                                obj->unk72.vy);
                }
                if (--tries == 0) {
                    break;
                }
            }
            obj->unk72.vy++;
        }
        if (obj->animation_phase >= 4095) {
            actor_reset_target_and_reselect();
        }
        actor_damp_horizontal_motion(def->unk1[2], 10);
        break;
    }
    case 27: {
        u8 *next;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf1;
            obj->target_phase = 0;
            obj->unk72.vx = -1;
            actor_set_animation(script[22]);
        }
        switch (obj->target_phase) {
        case 0:
            if (SCRIPT_U16(script, 14) == 0) {
                obj->target_phase = 1;
            } else {
                obj->unk72.vx = actor_turn_and_move_toward_point((obj->home_cell_x << 11) + obj->unk24,
                                                                 (obj->home_cell_z << 11) + obj->unk22,
                                                                 SCRIPT_U16(script, 14), SCRIPT_U16(script, 16),
                                                                 obj->unk72.vx, def->unk1[2], def->unk1[3], 5);
                if (obj->unk72.vx == -1) {
                    obj->target_phase = 1;
                }
            }
            actor_advance_animation_wrapped(obj, script[23]);
            break;
        case 1:
            actor_turn_toward_angle(obj, obj->unk20, SCRIPT_U16(script, 16), def->unk1[3]);
            if (obj->angle.vy == obj->unk20) {
                actor_set_animation(script[1]);
                obj->target_phase = 2;
            }
            break;
        case 2:
            actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
            obj->collision_radius = fixed_lerp_q12(def->collision_radius, SCRIPT_U16(script, 18),
                                                   obj->animation_phase);
            obj->collision_height = fixed_lerp_q12(def->collision_height, SCRIPT_U16(script, 20),
                                                   obj->animation_phase);
            if (obj->animation_phase >= 4095) {
                obj->collision_radius = SCRIPT_U16(script, 18);
                obj->collision_height = SCRIPT_U16(script, 20);
                next = actor_find_target_of_type(def, 0x15);
                if (next != NULL) {
                    actor_set_target(obj, next);
                } else {
                    actor_reset_target_and_reselect();
                }
            }
            break;
        }
        break;
    }
    case 29:
        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
            actor_suspend_vertical_motion();
            obj->unk72.vx = 16;
        }
        obj->angle.vx += 128;
        obj->angle.vy += 64;
        obj->pos.vy -= 256;
        if (--obj->unk72.vx == 0) {
            actor_reset_target_and_reselect();
        }
        break;
    case 28: {
        s32 hit;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
            actor_suspend_vertical_motion();
            audio_play_spatial_default_range(0x32, &obj->pos, 0x78, 0);
        }
        obj->angle.vx += 128;
        obj->angle.vy += 64;
        hit = actor_move_with_collision(&obj->vel);
        if (hit != 0) {
            if (hit & 0x80) {
                player_apply_damage(SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), SCRIPT_U16(script, 18),
                                    SCRIPT_U16(script, 12), SCRIPT_U16(script, 20), SCRIPT_U16(script, 22),
                                    SCRIPT_U16(script, 24), SCRIPT_U16(script, 26), SCRIPT_U16(script, 28),
                                    0x1000, 10, &obj->pos);
            }
            actor_select_target_type_in_own_group(obj, 3);
            obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
            obj->vertical_motion_state = 0x10;
        }
        break;
    }
    case 30: {
        VECTOR next;
        s32 hit;

        if (obj->target_action_state == 0) {
            obj->target_action_state = 0xf0;
            actor_set_animation(script[1]);
            actor_suspend_vertical_motion();
        }
        next.vx = obj->vel.vx + obj->pos.vx;
        next.vy = obj->vel.vy + obj->pos.vy;
        next.vz = obj->vel.vz + obj->pos.vz;
        hit = collision_query_world(next.vx, next.vy, next.vz, obj->collision_radius, ACTOR_SHAPE(obj),
                                    actor_status.actor_collision_query_flags);
        if (hit == 0) {
        move:
            obj->pos.vx = next.vx;
            obj->pos.vy = next.vy;
            obj->pos.vz = next.vz;
        } else if (hit & 0x80) {
            player_apply_damage(SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), SCRIPT_U16(script, 18),
                                SCRIPT_U16(script, 12), SCRIPT_U16(script, 20), SCRIPT_U16(script, 22),
                                SCRIPT_U16(script, 24), SCRIPT_U16(script, 26), SCRIPT_U16(script, 28),
                                0x1000, 10, &obj->pos);
            goto move;
        } else {
            obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
        }
        actor_advance_animation_clamped(obj, SCRIPT_U16(script, 8));
        if (obj->animation_phase >= 4095) {
            actor_select_target_type_in_own_group(obj, 3);
            obj->vertical_motion_state = 0x10;
        }
        break;
    }
    case 31:
        phase = obj->fade_out_phase;
        if (phase == 0 && SCRIPT_U16(script, 14) != 0) {
            obj->fade_out_phase++;
            obj->angle.vy = cur_avatar_status.camera_rotation.vy + 2048;
            actor_select_target_type_in_own_group(obj, 0x19);
        } else {
            obj->flags &= ~0x1000000;
            obj->fade_out_phase++;
            if (phase > 0) {
                if (phase == 1) {
                    obj->lighting_override_index = 0x32;
                    obj->lighting_blend = 0x400;
                    obj->render_mode = (obj->render_mode & 7) | 5;
                } else if (phase < 38) {
                    if (obj->lighting_blend < 4096) {
                        obj->lighting_blend += 192;
                    } else {
                        obj->lighting_blend = 0x1000;
                    }
                } else if (obj->slot_state == 1) {
                    actor_set_lifecycle_and_home_position(obj);
                } else if (obj->slot_state == 2) {
                    obj->lifecycle = 0;
                    actor_set_home_position(obj);
                } else if (obj->slot_state == 5) {
                    obj->slot_state = 0xff;
                    obj->lifecycle = 0;
                } else {
                    obj->lifecycle = 2;
                    actor_set_home_position(obj);
                }
            }
        }
        actor_damp_horizontal_motion(def->unk1[2] * 2, 10);
        break;
    case 240:
        actor_suspend_vertical_motion();
        break;
    default:
        asset_status.handlers->update_actor_target();
        break;
    }
    if (obj->flags & 0x10) {
        Actor *target;
        SVECTOR rot;
        VECTOR from;
        VECTOR to;
        s32 hit;

        target = actor_status.other_actor;
        if (obj->slot_state == 3) {
            obj->layer_mask = 0;
            if (target->lifecycle != 1) {
                obj->lifecycle = 0;
                goto done;
            }
            if (target->target_type == 3) {
                obj->target_type = 3;
                obj->target_action_state = 0xf0;
                obj->target_phase = 99;
                goto done;
            }
        } else {
            obj->layer_mask = target->layer_mask;
            if (target->lifecycle != 1) {
                obj->flags = (obj->flags & ~0x10) | 0x100;
                hit = collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, obj->collision_radius,
                                            obj->collision_height, 0x81);
                if (hit & 0x80) {
                    obj->pos.vx += 800 + obj->collision_radius;
                }
                if (hit & 0xf) {
                    obj->flags |= 0x400;
                }
                if (obj->flags & 0x200) {
                    actor_select_target_type_in_own_group(obj, 3);
                }
                obj->vel.vx = target->vel.vx;
                obj->vel.vy = target->vel.vy;
                obj->vel.vz = target->vel.vz;
                goto done;
            }
        }
        obj->angle.vx = target->angle.vx;
        obj->angle.vz = target->angle.vz;
        rot.vx = target->angle.vx;
        rot.vy = target->angle.vy;
        rot.vz = target->angle.vz;
        vector_rotate_yxz(&rot, (SVECTOR *)&def->position_offset_x, &from);
        actor_sample_rotated_animation_vertex(target, obj->unk24, &to);
        obj->pos.vx = target->pos.vx + to.vx - from.vx;
        obj->pos.vy = target->pos.vy + to.vy - from.vy;
        obj->pos.vz = target->pos.vz + to.vz - from.vz;
    } else if (!(obj->flags & 0x10000)) {
        actor_update_vertical_motion();
    }
done:
    if (obj->lifecycle == 1) {
        map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->collision_radius, 1);
    }
}

/* Updates every record: wake/sleep, script pick, its script, attachment. */
ADDRESS(0x80052e5c, 0x228)
void actor_update_frame(void)
{
    Actor *obj;
    MapObject *carrier;

    obj = actor_status.objects;
    actor_status.active_actor_count = 0;
    actor_status.current_index = 0;
    do {
        if (obj->slot_state != ACTOR_FREE) {
            actor_bind_current(obj);
            if ((actor_status.actor_update_frame_count & 3) == (actor_status.current_index & 3)
                || cur_avatar_status.force_actor_lifecycle_refresh != 0 ||
                cur_avatar_status.reaction_mode == 1) {
                actor_update_lifecycle_for_player_range();
            }
            if (obj->lifecycle == 1) {
                if ((actor_status.actor_update_frame_count & 3) == (actor_status.active_actor_count & 3)) {
                    actor_select_target_for_player_distance();
                }
                actor_update_behavior();
                actor_status.active_actor_count++;
            }
            if (actor_status.current_group->initial_actor_flags & 0x10000) {
                carrier = &map_objects_slots.objects[obj->unk22];
                obj->pos.vx = carrier->pos.vx;
                obj->pos.vy = carrier->pos.vy - 500;
                obj->pos.vz = carrier->pos.vz;
                if (obj->lifecycle == 1) {
                    obj->angle.vy = vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                       cur_avatar_status.pos.vz - obj->pos.vz);
                }
            }
            if (obj->definition_id == 0x6e) {
                if (cur_avatar_status.equipped_accessory_id != 0x5c
                    && cur_avatar_status.equipped_extra_id != 0x5c) {
                    obj->layer_mask = 0;
                } else if (cur_avatar_status.mp == 0) {
                    obj->layer_mask = 0;
                }
            }
        }
        obj++;
    } while (++actor_status.current_index < 200);
    actor_status.actor_update_frame_count++;
    actor_bind_current(NULL);
}

/* Turns the loaded ActorGroup records' script offsets into pointers (-1: none). */
ADDRESS(0x80053084, 0x74)
void actor_fixup_group_targets(void)
{
    ActorGroup *def;
    u8 **scripts;
    s32 i;
    s32 j;

    def = actor_status.groups;
    for (i = 0; i < 40; i++, def++) {
        if (def->definition_id == 0xff) {
            return;
        }
        scripts = def->targets;
        for (j = 0; j < 16; j++, scripts++) {
            if ((s32)*scripts == -1) {
                *scripts = NULL;
            } else {
                *scripts = actor_status.target_candidate_blob + (s32)*scripts;
            }
        }
    }
}

/* Makes the 200 records from an area's placement records. */
ADDRESS(0x800530f8, 0x1dc)
void actor_load_records(ActorPlacement *place)
{
    Actor *obj;
    ActorGroup *def;
    u16 i;

    obj = actor_status.objects;
    i = 200;
    while (i--) {
        obj->slot_state = place->slot_state;
        if (obj->slot_state != ACTOR_FREE) {
            obj->group_index = place->group_index;
            obj->unk4 = 0;
            obj->placement_flags = place->placement_flags;
            obj->home_layer_mask = place->home_layer_mask;
            obj->home_cell_z = place->cell_z;
            obj->home_cell_x = place->cell_x;
            obj->spawn_chance = place->spawn_chance;
            obj->death_drop_object_id = place->death_drop_object_id;
            obj->unk20 = -place->unk8 & 0xfff;
            obj->unk22 = place->unka;
            obj->unk24 = place->unkc;
            obj->vertical_anchor_offset = place->vertical_anchor_offset;
            obj->lifecycle = 0;
            obj->target_type = 0;
            obj->target_action_state = 0xff;
            obj->target = NULL;
            def = &actor_status.groups[obj->group_index];
            actor_copy_group_defaults(obj);
            actor_set_home_position(obj);
            obj->render_depth_offset = def->render_depth_offset;
            if (obj->flags & 0x10) {
                if (obj->slot_state == 3) {
                    if (obj->unk24 == -1) {
                        obj->unk24 = def->unk1a;
                    }
                    if (obj->vertical_anchor_offset == -1) {
                        obj->vertical_anchor_offset = def->default_vertical_anchor_offset;
                    }
                } else {
                    obj->slot_state = 4;
                    obj->vertical_anchor_offset = 0;
                }
            }
        } else {
            obj->lifecycle = 0;
        }
        place++;
        obj++;
    }
}
