/*
 * GAME.EXE: player state, KF2's player_collision_bounds.c and
 * player_runtime.c in their order, closed by collision_sample_map_cell_layer
 * (KF2's collision_grid_sample.c): the collision bounds and view reset, fall
 * and death heights, the per-area reset, equipment and the derived stats,
 * experience and levels, damage, the camera the renderer takes from
 * cur_avatar_status, the weapon tip and swing, the magic checks and casts,
 * the wall-sliding mover, the reaction modes and their parameters, hp/mp
 * changes, the per-frame effect timers, the player frame and the map cell
 * pick. One TU: the jump tables of player_dispatch_magic_effect,
 * player_select_magic_action, item_id_is_consumable and player_update_frame
 * share one .rdata (the 4-byte pads at 0x8001191c, 0x80011a74 and 0x80011abc
 * align them inside it); collision_evaluate_shape_records' table at
 * 0x80011b0c (4 mod 8, no pad) starts the next TU.
 */
/* player_update_frame polls asset_status.step, which the CD callbacks set
 * asynchronously (game/data.h SCENE_STEP_SHARED). */
#define GAME_FLOOR_SELECT_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <stdarg.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/render.h>
#include <game/data.h>
#include <game/menu.h>
#include <game/effect.h>
#include <game/misc.h>
#include <game/map_object.h>
#include <game/funcs.h>
#include <game/player.h>
#include <game/actor.h>
#include <game/scene.h>
#include <kf/lib/null.h>
#include <game/collision.h>
#include <psyq/audio.h>
#include <game/memory.h>

RODATA(0x80011760, 0x3ac)

/* Pad bit per action (WIP); player_clear_motion keeps four of them held. */
DATA(0x80081868, 0x1c, ".data")
u16 action_button_masks[14] = {
    0x1000, 0x4000, 0x8000, 0x2000, 0x10, 0x40, 0x80, 0x20, 4, 1, 8, 2, 0x100, 0x800,
};

/* Per magic element: the levels at which player_learn_element_magic teaches spells. */
DATA(0x80081884, 0x3c, ".data")
s16 magic_element_learn_levels[5][6] = {
    { 20, 35, 50, 65, 80, 0 },
    { 20, 35, 50, 65, 80, 0 },
    { 20, 35, 50, 65, 80, 0 },
    { 20, 35, 50, 65, 80, 0 },
    { 10, 35, 50, 85, 115, 0 },
};

/* Per magic element: the MagicRecord indices of its spells, 0xff ended. */
DATA(0x800818c0, 0x23, ".data")
u8 magic_element_learn_ids[5][7] = {
    { 0x00, 0x01, 0x02, 0x04, 0x05, 0xff, 0x00 },
    { 0x17, 0x06, 0x07, 0x08, 0x09, 0xff, 0x00 },
    { 0x19, 0x1a, 0x0b, 0x0c, 0x1b, 0xff, 0x00 },
    { 0x0d, 0x1c, 0x0f, 0x10, 0x11, 0xff, 0x00 },
    { 0x1d, 0x1e, 0x13, 0x14, 0x15, 0xff, 0x00 },
};

/* Offsets (magic_origin_offset) of the five shots of spells 72..74 (player_dispatch_magic_effect). */
DATA(0x800818e4, 0x28, ".data")
static SVECTOR player_magic_spawn_offsets[5] = {
    { 0, 0, 100, 0 }, { -1500, 0, 0, 0 }, { -3000, 0, -2000, 0 }, { 1500, 0, 0, 0 }, { 3000, 0, -2000, 0 }
};

/* The map cell outside the 0x28000 square map. */
DATA(0x8008190c, 0xa, ".data")
static MapCell collision_default_cell =
    { { { 0xff, 0x00, 0x00, 0x3f, 0x00 }, { 0xff, 0x00, 0x00, 0x3f, 0x00 } } };

ADDRESS(0x80028d54, 0xf4)
void player_update_collision_bounds(void)
{
    s32 height0 = map_world.collision_bounds[0] + 1600;
    s32 height1;
    s32 height2;

    if ((cur_avatar_status.collision_lower_clearance = height0 - (cur_avatar_status.camera_vertical_offset + cur_avatar_status.pos.vy + cur_avatar_status.landing_vertical_offset))
        < -1000) {
        player_death_begin(0);
    }
    height1 = map_world.collision_bounds[1] + 1600;
    if ((cur_avatar_status.collision_upper_clearance = height1 - (cur_avatar_status.camera_vertical_offset + cur_avatar_status.pos.vy + cur_avatar_status.landing_vertical_offset))
        <= 0) {
        player_death_begin(0);
    }
    height2 = map_world.collision_bounds[2] + 1600;
    if ((cur_avatar_status.collision_second_upper_clearance = height2 - (cur_avatar_status.camera_vertical_offset + cur_avatar_status.pos.vy + cur_avatar_status.landing_vertical_offset))
        <= 0) {
        player_death_begin(0);
    }
}

ADDRESS(0x80028e48, 0x54)
s32 player_distance_to_point_with_margin(s32 x, s32 y, s32 z, s32 radius, s32 reach_up)
{
    return vector_distance_to_point(&cur_avatar_status.pos, x, y, z, radius + 800, 1700, reach_up);
}

ADDRESS(0x80028e9c, 0xf4)
void player_reset_view(void)
{
    cur_avatar_status.camera_vertical_offset = 0;
    cur_avatar_status.walking_bob_phase = 0;
    cur_avatar_status.landing_vertical_offset = 0;
    cur_avatar_status.vertical_motion_state = 0;
    cur_avatar_status.vertical_velocity = 0;
    cur_avatar_status.reaction_mode = 0;
    cur_avatar_status.camera_roll_offset = 0;
    cur_avatar_status.camera_yaw_offset = 0;
    cur_avatar_status.vertical_motion_pitch_offset = 0;
    cur_avatar_status.view_rotation_offset_z = 0;
    cur_avatar_status.view_rotation_offset_y = 0;
    cur_avatar_status.view_rotation_offset_x = 0;
    cur_avatar_status.reaction_rotation_offset_z = 0;
    cur_avatar_status.reaction_rotation_offset_y = 0;
    cur_avatar_status.reaction_rotation_offset_x = 0;
    cur_avatar_status.camera_rotation = cur_avatar_status.camera_rotation_target;
    cur_avatar_status.damage_scale = 0x1000;
    cur_avatar_status.queued_magic_id = 0xff;
    cur_avatar_status.movement_speed_adjustment_q12 = 0;
    cur_avatar_status.movement_speed_adjustment_decay_latch = 0;
    cur_avatar_status.collision_lower_clearance = 10000;
    cur_avatar_status.collision_upper_clearance = 10000;
    cur_avatar_status.collision_second_upper_clearance = 10000;
}

ADDRESS(0x80028f90, 0x148)
void player_restore_equipment_effects(void)
{
    player_set_equipment_slot(0xff, 0xff);
    player_equip_weapon(cur_avatar_status.equipped_weapon_id);
    player_set_primary_magic_shortcut_id(cur_avatar_status.primary_magic_shortcut_id);
    player_set_secondary_magic_shortcut_id(cur_avatar_status.secondary_magic_shortcut_id);
    player_reset_view();
    if (cur_avatar_status.fire_defense_boost_timer != 0) {
        effect_construct_record(10, 64, 26, &cur_avatar_status.pos, 0, 0);
        effect_construct_record(10, 64, 26, &cur_avatar_status.pos, 0, 1);
    }
    if (cur_avatar_status.damage_immunity_timer != 0) {
        effect_construct_record(10, 64, 27, &cur_avatar_status.pos, 0);
    }
    if (cur_avatar_status.physical_defense_boost_timer != 0) {
        effect_construct_record(10, 64, 28, &cur_avatar_status.pos, 0, 0);
        effect_construct_record(10, 64, 28, &cur_avatar_status.pos, 0, 1);
    }
    player_recalculate_combat_stats();
    menu_apply_button_config();
}

/*
 * Marks the 32 arena blocks of part as cached unless they are still loading.
 * int with no return value: a void return changes the loop's delay slot.
 */
ADDRESS(0x800290d8, 0xb0)
int resource_mark_area_models_reclaimable(int part_in)
{
    u8 part = part_in;
    u16 i;
    u8 *block;

    for (i = part * 32 + 556; i < part * 32 + 588; i++) {
        block = scene_display_sys.models[i];
        if (block != NULL && memory_block_kind(block) != 3) {
            memory_block_set_kind(block, 1);
        }
    }
}

ADDRESS(0x80029188, 0x208)
void player_reload_map_resources(s32 area0, s32 area1, s32 area2, s32 area3, s32 area4, s32 part)
{
    cd_request_wait_idle();
    do {
        cd_request_yield();
        resource_advance_transition();
    } while ((s16)asset_status.loading != 0);
    resource_mark_area_models_reclaimable(asset_status.active_resource_ids[0]);
    asset_status.active_resource_ids[0] = 99;
    asset_status.active_resource_ids[1] = 99;
    asset_status.active_resource_ids[2] = 99;
    asset_status.active_resource_ids[3] = 99;
    asset_status.active_resource_ids[4] = 99;
    resource_request_transition(area0, area1, area2, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
    do {
        cd_request_yield();
        resource_advance_transition();
    } while ((s16)asset_status.loading != 0);
    cd_request_wait_idle();
    VSync(2);
    if (part != 0xff) {
        resource_request_transition(0xff, 0xff, part, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
        do {
            cd_request_yield();
            resource_advance_transition();
        } while ((s16)asset_status.loading != 0);
        cd_request_wait_idle();
        VSync(2);
    }
    player_sync_position_to_map();
    light_reset_records();
    render_reset_color_overlays();
    light_refresh_record_rotations();
    player_update_status_visuals();
    render_frames_with_color_overlay(0x82, 0x1000, 0x1000, 0);
    cd_request_wait_idle();
    render_frames_with_color_overlay(0x82, 0x1000, 0, -128);
    resource_request_transition(0xff, 0xff, 0xff, area3, area4, 0x7f, 0x7f, 0x7f);
}

ADDRESS(0x80029390, 0x54)
s32 player_charge_gain_for_rank(s32 magic, s32 weight)
{
    magic = (magic << 6) / (weight + 1) + 1;
    if (magic > 4999) {
        return 5000;
    }
    return magic;
}

ADDRESS(0x800293e4, 0x11c)
void player_add_equipment_bonuses(s32 id)
{
    EquipmentRecord *r;

    if (id == 0xff) {
        return;
    }
    r = &item_gear_recs[id - EQUIPMENT_RECORD_FIRST_ID];
    cur_avatar_status.defense_slash += r->defense_slash;
    cur_avatar_status.defense_blow += r->defense_blow;
    cur_avatar_status.defense_stab += r->defense_stab;
    cur_avatar_status.defense_holy += r->defense_holy;
    cur_avatar_status.defense_fire += r->defense_fire;
    cur_avatar_status.defense_earth += r->defense_earth;
    cur_avatar_status.defense_wind += r->defense_wind;
    cur_avatar_status.defense_water += r->defense_water;
    cur_avatar_status.poison_resistance += r->poison_resistance;
}

ADDRESS(0x80029500, 0xa1c)
void player_recalculate_combat_stats(void)
{
    s32 i;
    s32 v;

    cur_avatar_status.attack_components[0] = 0;
    cur_avatar_status.attack_components[1] = 0;
    cur_avatar_status.attack_components[2] = 0;
    cur_avatar_status.attack_components[3] = 0;
    cur_avatar_status.attack_components[4] = 0;
    cur_avatar_status.attack_components[5] = 0;
    cur_avatar_status.attack_components[6] = 0;
    cur_avatar_status.attack_components[7] = 0;
    cur_avatar_status.defense_slash = 0;
    cur_avatar_status.defense_blow = 0;
    cur_avatar_status.defense_stab = 0;
    cur_avatar_status.defense_holy = 0;
    cur_avatar_status.defense_fire = 0;
    cur_avatar_status.defense_earth = 0;
    cur_avatar_status.defense_wind = 0;
    cur_avatar_status.defense_water = 0;
    cur_avatar_status.poison_resistance = 0;
    cur_avatar_status.physical_power = cur_avatar_status.base_physical_power;
    for (i = 4; i != -1; i--) {
        cur_avatar_status.magic[i] = cur_avatar_status.base_magic[i];
    }
    if (cur_avatar_status.curse_strength != 0) {
        v = cur_avatar_status.physical_power >> 1;
        if (v < 0) {
            v = 0;
        }
        cur_avatar_status.physical_power = v;
        for (i = 4; i != -1; i--) {
            v = cur_avatar_status.magic[i] >> 1;
            cur_avatar_status.magic[i] = v < 0 ? 0 : v;
        }
    }
    if (cur_avatar_status.equipped_weapon_id != 0xff) {
        WeaponRecord *w = &weapon_table[cur_avatar_status.equipped_weapon_id];

        cur_avatar_status.attack_components[0] += w->attack_components[0];
        cur_avatar_status.attack_components[1] += w->attack_components[1];
        cur_avatar_status.attack_components[2] += w->attack_components[2];
        cur_avatar_status.attack_components[3] += w->attack_components[3];
        cur_avatar_status.attack_components[4] += w->attack_components[4];
        cur_avatar_status.attack_components[5] += w->attack_components[5];
        cur_avatar_status.attack_components[6] += w->attack_components[6];
        cur_avatar_status.attack_components[7] += w->attack_components[7];
    }
    player_add_equipment_bonuses(cur_avatar_status.equipped_head_id);
    player_add_equipment_bonuses(cur_avatar_status.equipped_body_id);
    player_add_equipment_bonuses(cur_avatar_status.equipped_shield_id);
    player_add_equipment_bonuses(cur_avatar_status.equipped_arm_id);
    player_add_equipment_bonuses(cur_avatar_status.equipped_leg_id);
    player_add_equipment_bonuses(cur_avatar_status.equipped_accessory_id);
    player_add_equipment_bonuses(cur_avatar_status.equipped_extra_id);
    if (cur_avatar_status.fire_defense_boost_timer != 0) {
        cur_avatar_status.defense_fire += 50;
    }
    if (cur_avatar_status.physical_defense_boost_timer != 0) {
        cur_avatar_status.defense_slash += 30;
        cur_avatar_status.defense_blow += 30;
        cur_avatar_status.defense_stab += 30;
    }
    if (cur_avatar_status.magic_boost_timer != 0) {
        for (i = 4; i != -1; i--) {
            cur_avatar_status.magic[i] += 5;
        }
    }
    if (cur_avatar_status.unka0 != 0) {
        cdrom_ctl.unkc = cur_avatar_status.unka0;
    }
    if (cur_avatar_status.equipped_weapon_id == 0x20) {
        cur_avatar_status.magic[2] += 20;
    }
    if (cur_avatar_status.equipped_weapon_id == 0x21) {
        cur_avatar_status.magic[0] += 20;
    }
    if (cur_avatar_status.equipped_head_id == 0x26) {
        cur_avatar_status.physical_power += 20;
    }
    if (cur_avatar_status.equipped_head_id == 0x29) {
        cur_avatar_status.magic[4] += 20;
    }
    if (cur_avatar_status.equipped_body_id == 0x31) {
        cur_avatar_status.magic[1] += 20;
    }
    if (cur_avatar_status.equipped_shield_id == 0x39) {
        cur_avatar_status.magic[4] += 30;
    }
    if (cur_avatar_status.equipped_shield_id == 0x3c) {
        cur_avatar_status.magic[2] += 20;
    }
    if (cur_avatar_status.equipped_arm_id == 0x43) {
        cur_avatar_status.attack_components[4] += 20;
    }
    if (cur_avatar_status.equipped_arm_id == 0x44) {
        cur_avatar_status.attack_components[0] += 10;
        cur_avatar_status.attack_components[1] += 10;
        cur_avatar_status.attack_components[2] += 10;
        cur_avatar_status.attack_components[4] += 10;
        cur_avatar_status.attack_components[5] += 10;
        cur_avatar_status.attack_components[6] += 10;
        cur_avatar_status.attack_components[7] += 10;
        cur_avatar_status.attack_components[3] += 10;
    }
    if (cur_avatar_status.equipped_arm_id == 0x46) {
        cur_avatar_status.magic[0] += 20;
    }
    if (cur_avatar_status.equipped_leg_id == 0x4f) {
        cur_avatar_status.magic[3] += 20;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x51) {
        cur_avatar_status.physical_power += 5;
    }
    if (cur_avatar_status.equipped_extra_id == 0x51) {
        cur_avatar_status.physical_power += 5;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x52 || cur_avatar_status.equipped_extra_id == 0x52) {
        cur_avatar_status.magic[0] += 8;
        cur_avatar_status.magic[1] += 8;
        cur_avatar_status.magic[2] += 8;
        cur_avatar_status.magic[3] += 8;
        cur_avatar_status.magic[4] += 8;
        cur_avatar_status.physical_power -= 30;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x55 || cur_avatar_status.equipped_extra_id == 0x55) {
        cur_avatar_status.attack_components[4] += 12;
        cur_avatar_status.attack_components[5] += 12;
        cur_avatar_status.attack_components[6] += 12;
        cur_avatar_status.attack_components[7] += 12;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x59 || cur_avatar_status.equipped_extra_id == 0x59) {
        cur_avatar_status.magic[3] += 20;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x5a || cur_avatar_status.equipped_extra_id == 0x5a) {
        cur_avatar_status.magic[1] += 20;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x5b || cur_avatar_status.equipped_extra_id == 0x5b) {
        cur_avatar_status.magic[4] += 20;
    }
    if (cur_evt_state.unk65 == 1 && asset_status.active_resource_ids[0] == 0x1a) {
        cur_avatar_status.magic[3] += 50;
    }
    if (cur_avatar_status.physical_power > 0x7fff) {
        cur_avatar_status.physical_power = 0;
    } else if (cur_avatar_status.physical_power >= 1000) {
        cur_avatar_status.physical_power = 999;
    }
    for (i = 4; i != -1; i--) {
        if (cur_avatar_status.magic[i] > 0x7fff) {
            cur_avatar_status.magic[i] = 0;
        } else if (cur_avatar_status.magic[i] >= 1000) {
            cur_avatar_status.magic[i] = 999;
        }
    }
    if (cur_avatar_status.attack_components[0] >= 0x8000) {
        cur_avatar_status.attack_components[0] = 0;
    }
    if (cur_avatar_status.attack_components[1] >= 0x8000) {
        cur_avatar_status.attack_components[1] = 0;
    }
    if (cur_avatar_status.attack_components[2] >= 0x8000) {
        cur_avatar_status.attack_components[2] = 0;
    }
    if (cur_avatar_status.attack_components[3] >= 0x8000) {
        cur_avatar_status.attack_components[3] = 0;
    }
    if (cur_avatar_status.attack_components[4] >= 0x8000) {
        cur_avatar_status.attack_components[4] = 0;
    }
    if (cur_avatar_status.attack_components[5] >= 0x8000) {
        cur_avatar_status.attack_components[5] = 0;
    }
    if (cur_avatar_status.attack_components[6] >= 0x8000) {
        cur_avatar_status.attack_components[6] = 0;
    }
    if (cur_avatar_status.attack_components[7] > 0x7fff) {
        cur_avatar_status.attack_components[7] = 0;
    }
    if (cur_avatar_status.defense_slash > 0x7fff) {
        cur_avatar_status.defense_slash = 0;
    }
    if (cur_avatar_status.defense_blow > 0x7fff) {
        cur_avatar_status.defense_blow = 0;
    }
    if (cur_avatar_status.defense_stab > 0x7fff) {
        cur_avatar_status.defense_stab = 0;
    }
    if (cur_avatar_status.defense_holy > 0x7fff) {
        cur_avatar_status.defense_holy = 0;
    }
    if (cur_avatar_status.defense_fire > 0x7fff) {
        cur_avatar_status.defense_fire = 0;
    }
    if (cur_avatar_status.defense_earth > 0x7fff) {
        cur_avatar_status.defense_earth = 0;
    }
    if (cur_avatar_status.defense_wind > 0x7fff) {
        cur_avatar_status.defense_wind = 0;
    }
    if (cur_avatar_status.defense_water > 0x7fff) {
        cur_avatar_status.defense_water = 0;
    }
    if (cur_avatar_status.poison_resistance > 0x7fff) {
        cur_avatar_status.poison_resistance = 0;
    }
}

/* Teaches the next spells of magic element once its level reaches them. */
ADDRESS(0x80029f1c, 0x158)
void player_learn_element_magic(u32 element)
{
    u8 *spell;
    MagicRecord *r;
    s32 n;
    s32 k;

    if (element >= 5) {
        return;
    }
    if (element != 4 && cur_evt_state.element_unlocked[element] == 0) {
        return;
    }
    for (n = 0; cur_avatar_status.base_magic[element] >= magic_element_learn_levels[element][n]; n++) {
        if (magic_element_learn_levels[element][n] == 0) {
            return;
        }
        spell = magic_element_learn_ids[element];
        for (k = n; k != -1; k--, spell++) {
            if (*spell == 0xff) {
                return;
            }
            r = &effect_status.magic_records[*spell];
            if (r->menu_available == 0) {
                r->menu_available = 1;
                notify_enqueue(3);
                cur_evt_state.interaction_handled = 1;
                cur_evt_state.element_magic_learned[element] = 1;
                break;
            }
        }
    }
}

ADDRESS(0x8002a074, 0x98)
void player_increment_physical_power_training(void)
{
    cur_avatar_status.physical_power_training++;
    if (cur_avatar_status.physical_power_training >= 100) {
        cur_avatar_status.base_physical_power++;
        cur_avatar_status.physical_power_training = 0;
        if (cur_avatar_status.base_physical_power >= 1000) {
            cur_avatar_status.base_physical_power = 999;
        } else {
            notify_enqueue(1);
        }
        player_recalculate_combat_stats();
    }
}

ADDRESS(0x8002a10c, 0x104)
void player_increment_magic_training(MagicRecord *spell)
{
    s32 mask = 0x10;
    s32 elements = 0;
    s32 i;

    if (spell != NULL) {
        elements = spell->element_mask;
    }
    for (i = 4; i != -1; mask >>= 1, i--) {
        if (elements & mask) {
            cur_avatar_status.magic_training[i] += spell->training_gain;
            if (cur_avatar_status.magic_training[i] >= 400) {
                cur_avatar_status.magic_training[i] = 0;
                cur_avatar_status.base_magic[i]++;
                if (cur_avatar_status.base_magic[i] >= 1000) {
                    cur_avatar_status.base_magic[i] = 999;
                } else {
                    notify_enqueue(i + 18);
                }
                player_learn_element_magic(i);
                player_recalculate_combat_stats();
            }
        }
    }
}

ADDRESS(0x8002a210, 0x100)
void player_grow_weapon_with_experience(void)
{
    if (game_counter_is_nonzero(0) > 0 || game_counter_is_nonzero(1) > 0) {
        if (cur_avatar_status.weapon_growth_experience == -1) {
            cur_avatar_status.weapon_growth_experience = cur_avatar_status.experience;
        } else if (cur_avatar_status.weapon_growth_experience + 13000 < cur_avatar_status.experience) {
            if (cur_avatar_status.equipped_weapon_id < 2) {
                player_equip_weapon(cur_avatar_status.equipped_weapon_id + 1);
            }
            if (game_counter_is_nonzero(1) > 0) {
                game_counter_decrement(1);
                game_counter_increment(2);
            }
            if (game_counter_is_nonzero(0) > 0) {
                game_counter_decrement(0);
                game_counter_increment(1);
            }
            notify_enqueue(4);
            cur_avatar_status.weapon_growth_experience = -1;
        }
    } else {
        cur_avatar_status.weapon_growth_experience = -1;
    }
}

ADDRESS(0x8002a310, 0x2e8)
void player_add_experience(int amount_in)
{
    s16 amount = amount_in;
    PlayerLevelGrowth *r;
    PlayerLevelGrowth *prev;
    s32 i;
    u8 level;

    cur_avatar_status.experience += amount;
    player_grow_weapon_with_experience();
    if (cur_avatar_status.experience > 999999) {
        cur_avatar_status.experience = 999999;
    }
    while (cur_avatar_status.experience >= cur_avatar_status.next_level_experience) {
        level = cur_avatar_status.level;
        if (cur_avatar_status.level >= 255) {
            return;
        }
        cur_avatar_status.level++;
        if (level < 99) {
            r = &player_lv_grow_list[level];
            cur_avatar_status.hp_max = r->hp_max;
            cur_avatar_status.mp_max = r->mp_max;
            cur_avatar_status.base_physical_power += r->physical_power_step;
            for (i = 4; i != -1; i--) {
                if (cur_avatar_status.base_magic[i] != 0 && rand() < 26213) {
                    cur_avatar_status.base_magic[i]++;
                }
            }
            cur_avatar_status.next_level_experience = r->experience_threshold;
        } else {
            r = &player_lv_grow_list[99];
            prev = &player_lv_grow_list[98];
            cur_avatar_status.hp_max += r->hp_max - prev->hp_max;
            cur_avatar_status.mp_max += player_lv_grow_list[99].mp_max - player_lv_grow_list[98].mp_max;
            cur_avatar_status.base_physical_power += player_lv_grow_list[99].physical_power_step;
            for (i = 4; i != -1; i--) {
                if (cur_avatar_status.base_magic[i] != 0 && rand() < 26213) {
                    cur_avatar_status.base_magic[i]++;
                }
            }
            cur_avatar_status.next_level_experience += r->experience_threshold - prev->experience_threshold;
        }
        notify_enqueue(0);
        if (cur_avatar_status.hp_max >= 1000) {
            cur_avatar_status.hp_max = 999;
        }
        if (cur_avatar_status.mp_max >= 1000) {
            cur_avatar_status.mp_max = 999;
        }
        if (cur_avatar_status.base_physical_power >= 1000) {
            cur_avatar_status.base_physical_power = 999;
        }
        for (i = 4; i != -1; i--) {
            player_learn_element_magic(i);
            if (cur_avatar_status.base_magic[i] >= 1000) {
                cur_avatar_status.base_magic[i] = 999;
            }
        }
        player_recalculate_combat_stats();
    }
}

ADDRESS(0x8002a5f8, 0xa8)
s32 player_calculate_damage_component(s32 power, s32 defense, s32 attack)
{
    s32 resisted;
    s32 left;

    resisted = power * cur_avatar_status.damage_scale >> 8;
    attack <<= 4;
    defense <<= 4;
    if (attack == 0) {
        return 0;
    }
    resisted += defense;
    left = attack - resisted;
    if (left < 0) {
        left = 0;
    }
    if (resisted == 0) {
        resisted = 16;
    }
    return (left + attack * attack / (resisted * 2)) / 5;
}

ADDRESS(0x8002a6a0, 0x54)
void player_adjust_hp_unclamped(s32 amount)
{
    s32 hp = cur_avatar_status.hp + amount;

    if (hp <= 0) {
        hp = 0;
        player_death_begin(0);
    }
    cur_avatar_status.hp = hp;
}

/*
 * Knocks the player back from from (NULL: in place) after damage, scaled by
 * the share of hp_max it took; 0 left is death.
 */
ADDRESS(0x8002a6f4, 0x360)
void player_apply_damage_reaction(VECTOR *from, s32 damage, s32 flags)
{
    SVECTOR push;
    SVECTOR dir;
    s32 hp;
    s32 ratio;
    s32 force;
    s32 y;
    s16 lift;

    if (cur_avatar_status.reaction_mode == 0x11) {
        return;
    }
    if (damage == 0) {
        return;
    }
    hp = cur_avatar_status.hp - damage;
    if (hp <= 0) {
        hp = 0;
    }
    ratio = (damage << 12) / cur_avatar_status.hp_max;
    cur_avatar_status.hp = hp;
    if (ratio > 950) {
        ratio = 950;
    }
    force = ratio >> 2;
    if (from != NULL) {
        y = from->vy + 850;
        vector_displacement_to_pitch_yaw(cur_avatar_status.pos.vx - from->vx, cur_avatar_status.pos.vy - y,
                                         cur_avatar_status.pos.vz - from->vz, &push);
        if (force < 80) {
            force = 80;
        }
        if (flags & 0x40) {
            force += force * 4;
            if (force > 600) {
                force = 600;
            }
        } else if (force > 450) {
            force = 450;
        }
        pitch_yaw_to_forward_vector(&push, &dir);
        vector3s_scale_shift12(force, &dir);
        if (dir.vy > 200) {
            dir.vy = 200;
        } else if (dir.vy < -200) {
            dir.vy = -200;
        }
    } else {
        dir.vz = 0;
        dir.vy = 0;
        dir.vx = 0;
    }
    lift = (1300 - (ratio >> 1)) >> 2;
    if (lift < 71) {
        lift = 70;
    }
    if (hp == 0) {
        player_death_begin(&dir);
        cur_avatar_status.damage_red_overlay_scale = 3500;
        cur_avatar_status.damage_red_overlay_decay = lift;
    } else if (from == NULL) {
        cur_avatar_status.damage_red_overlay_scale = 3500;
        cur_avatar_status.damage_red_overlay_decay = lift;
    } else {
        ratio = SquareRoot0(ratio << 2);
        if (ratio < 16) {
            ratio = 16;
        } else if (ratio >= 43) {
            ratio = 42;
        }
        if (flags & 0x40) {
            ratio <<= 1;
        }
        if (flags & 0x80) {
            push.vx = 0;
            if (rand() > 16384) {
                push.vz = ratio;
            } else {
                push.vz = -ratio;
            }
            push.vy = 0;
            player_begin_rotation_only_damage_reaction(&dir, &push, lift);
        } else {
            switch ((push.vy - cur_avatar_status.camera_rotation.vy + 256) >> 9 & 7) {
            case 0:
                push.vx = -ratio;
                push.vz = 0;
                break;
            case 1:
                push.vx = -ratio;
                push.vz = ratio;
                break;
            case 2:
                push.vx = 0;
                push.vz = ratio;
                break;
            case 3:
                push.vx = ratio;
                push.vz = ratio;
                break;
            case 4:
                push.vx = ratio;
                push.vz = 0;
                break;
            case 5:
                push.vx = ratio;
                push.vz = -ratio;
                break;
            case 6:
                push.vx = 0;
                push.vz = -ratio;
                break;
            case 7:
                push.vx = -ratio;
                push.vz = -ratio;
                break;
            }
            push.vy = 0;
            player_begin_moving_damage_reaction(&dir, &push, lift);
        }
    }
}

ADDRESS(0x8002aa54, 0xc4)
void player_cap_status_components(s32 flags)
{
    if (flags & 1) {
        if (cur_avatar_status.curse_strength > 64) {
            cur_avatar_status.curse_strength = 64;
        }
        cur_avatar_status.curse_phase_limit = 0;
    }
    if (flags & 2) {
        if (cur_avatar_status.darkness_phase > 64) {
            cur_avatar_status.darkness_phase = 64;
        }
        cur_avatar_status.darkness_phase_limit = 0;
    }
    if (flags & 3) {
        cur_avatar_status.poison_timer = 0;
    }
    if (flags & 5) {
        if (cur_avatar_status.slow_timer > 64) {
            cur_avatar_status.slow_timer = 64;
        }
    }
    if (flags & 4) {
        cur_avatar_status.paralysis_timer = 0;
    }
}

/* Damage to the player: three physical and five elemental attack values. */
ADDRESS(0x8002ab18, 0x5fc)
void player_apply_damage(u16 slash, u16 blow, u16 stab, u16 status_flags, u16 element0, u16 element1,
                         u16 element2, u16 element3, u16 element4, u16 scale, u16 damage_multiplier_tenths,
                         VECTOR *from)
{
    s32 time2;
    s32 time3;
    s32 time4;
    s32 time5;
    s32 time6;
    s32 damage;
    s32 v;

    if (cur_avatar_status.weapon_guard_active != 0) {
        return;
    }
    if (cur_avatar_status.damage_immunity_timer != 0) {
        return;
    }
    time2 = 1350;
    time3 = 2700;
    time4 = 900;
    time5 = 200;
    time6 = 450;
    if (cur_avatar_status.equipped_arm_id == 0x41) {
        time5 = 150;
    }
    if (cur_avatar_status.equipped_leg_id == 0x4a) {
        time4 = 675;
    }
    if (cur_avatar_status.equipped_leg_id == 0x4c) {
        time5 >>= 1;
        time3 = 1350;
        time2 = 675;
        time6 = 225;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x53 || cur_avatar_status.equipped_extra_id == 0x53) {
        time4 >>= 2;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x56 || cur_avatar_status.equipped_extra_id == 0x56) {
        time5 >>= 2;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x58 || cur_avatar_status.equipped_extra_id == 0x58) {
        time3 >>= 1;
        time2 >>= 1;
        time6 >>= 1;
    }
    if (cur_avatar_status.equipped_accessory_id == 0x5b || cur_avatar_status.equipped_extra_id == 0x5b) {
        time3 >>= 2;
        time2 >>= 2;
        time6 >>= 2;
    }
    if (cur_avatar_status.equipped_head_id == 0x29) {
        time3 >>= 2;
        time2 >>= 2;
        time6 >>= 2;
    }
    if (cur_avatar_status.equipped_leg_id == 0x4f) {
        time5 >>= 2;
        time4 >>= 2;
    }
    if (cur_avatar_status.equipped_body_id == 0x2d) {
        element1 = 0;
    }
    if (cur_avatar_status.equipped_head_id == 0x27 && rand() < 8192) {
        slash = 0;
        blow = 0;
        stab = 0;
    }
    if (*(u32 *)&cur_avatar_status.equipped_head_id == 0x453b2f28
        && cur_avatar_status.equipped_leg_id == 0x4e) {
        slash = 0;
        blow = 0;
        stab = 0;
    }
    if (cur_avatar_status.equipped_body_id == 0x30 && rand() < 8192) {
        slash = 0;
        blow = 0;
        stab = 0;
    }
    if (cur_avatar_status.equipped_body_id == 0x31 && rand() < 8192) {
        slash = 0;
        blow = 0;
        stab = 0;
    }
    if (cur_avatar_status.equipped_shield_id == 0x3a && rand() < 8192) {
        element1 = 0;
        element0 = 0;
        element2 = 0;
        element3 = 0;
        element4 = 0;
    }
    if (cur_avatar_status.equipped_shield_id == 0x3c && rand() < 8192) {
        element1 = 0;
        element0 = 0;
        element2 = 0;
        element3 = 0;
        element4 = 0;
    }
    if ((cur_avatar_status.equipped_accessory_id == 0x55 || cur_avatar_status.equipped_extra_id == 0x55)
        && rand() < 16384) {
        element1 = 0;
        element0 = 0;
        element2 = 0;
        element3 = 0;
        element4 = 0;
    }
    switch (status_flags & 0xf) {
    case 1:
        cur_avatar_status.curse_phase_limit = time2;
        cur_avatar_status.curse_strength = 1;
        player_recalculate_combat_stats();
        break;
    case 2:
        cur_avatar_status.darkness_phase_limit = time3;
        cur_avatar_status.magic_tint_phase_limit = 0;
        break;
    case 3:
        if (cur_avatar_status.poison_resistance < rand() * 100 >> 15) {
            cur_avatar_status.poison_timer = time4;
        }
        break;
    case 4:
        cur_avatar_status.paralysis_timer = time5;
        break;
    case 5:
        cur_avatar_status.slow_timer = time6;
        break;
    case 7:
        cur_avatar_status.poison_timer = 0;
        break;
    case 6:
        v = cur_avatar_status.mp_max >> 4;
        if (v < cur_avatar_status.mp) {
            cur_avatar_status.mp -= v;
        } else {
            cur_avatar_status.mp = 0;
        }
        break;
    }
    damage = player_calculate_damage_component(cur_avatar_status.physical_power,
                                               cur_avatar_status.defense_slash, slash);
    damage += player_calculate_damage_component(cur_avatar_status.physical_power,
                                                cur_avatar_status.defense_blow, blow);
    damage += player_calculate_damage_component(cur_avatar_status.physical_power,
                                                cur_avatar_status.defense_stab, stab);
    damage += player_calculate_damage_component(cur_avatar_status.physical_power,
                                                cur_avatar_status.defense_holy, element0);
    damage += player_calculate_damage_component(cur_avatar_status.physical_power,
                                                cur_avatar_status.defense_fire, element1);
    damage += player_calculate_damage_component(cur_avatar_status.physical_power,
                                                cur_avatar_status.defense_earth, element2);
    damage += player_calculate_damage_component(cur_avatar_status.physical_power,
                                                cur_avatar_status.defense_wind, element3);
    damage += player_calculate_damage_component(cur_avatar_status.physical_power,
                                                cur_avatar_status.defense_water, element4);
    damage = scale * damage + 0x8000 >> 16;
    player_apply_damage_reaction(from, damage_multiplier_tenths * damage / 10, status_flags);
}

ADDRESS(0x8002b114, 0x21c)
void player_apply_radial_damage(VECTOR *pos, s32 reach, s32 radius, s32 mode, u16 falloff, u16 slash,
                                u16 blow, u16 stab, u16 type, u16 element0, u16 element1, u16 element2,
                                u16 element3, u16 element4, s32 flags, u16 damage_multiplier_tenths)
{
    VECTOR *from = pos;
    u32 strength;
    u32 ratio;
    u32 scale;
    s32 hit;

    if (flags & 0x8000) {
        from = NULL;
    }
    strength = flags & 0x7fff;
    if (mode == 0x8000) {
        hit = vector_distance_between_with_reach(pos, radius, &cur_avatar_status.pos, 800, 1700);
    } else if (mode == 0x8001) {
        if (pos->vy < cur_avatar_status.pos.vy - 1700) {
            hit = -9999999;
        } else {
            hit = vector_distance_between_with_reach(pos, radius, &cur_avatar_status.pos, 800, 1700);
        }
    } else {
        hit = player_distance_to_point(pos->vx, pos->vy, pos->vz, radius, mode);
        if (hit == -1) {
            hit = -9999999;
        }
    }
    if (hit < reach) {
        return;
    }
    if (falloff != 0x1000) {
        ratio = (u16)((hit << 12) / radius);
        scale = strength * (u16)(0x1000 - (ratio * (0x1000 - falloff) >> 12)) >> 12;
    } else {
        scale = strength;
    }
    player_apply_damage(slash, blow, stab, type, element0, element1, element2, element3, element4,
                        scale, damage_multiplier_tenths, from);
}

/* The eye position and view angles the renderer takes. */
ADDRESS(0x8002b330, 0x78)
void player_get_camera_pose(VECTOR *pos, SVECTOR *angle)
{
    pos->vx = cur_avatar_status.pos.vx;
    pos->vz = cur_avatar_status.pos.vz;
    pos->vy = cur_avatar_status.camera_vertical_offset + cur_avatar_status.pos.vy
        + cur_avatar_status.landing_vertical_offset - 1600;
    angle->vx = cur_avatar_status.camera_rotation.vx;
    angle->vy = cur_avatar_status.camera_rotation.vy;
    angle->vz = cur_avatar_status.camera_rotation.vz;
}

ADDRESS(0x8002b3a8, 0xc0)
void player_reset_status(void)
{
    cur_avatar_status.magic_boost_timer = 0;
    cur_avatar_status.full_mp_timer = 0;
    cur_avatar_status.map_marker_visual_effect_timer = 0;
    cur_avatar_status.magic_tint_phase_limit = 0;
    cur_avatar_status.magic_tint_phase = 0;
    cur_avatar_status.physical_defense_boost_timer = 0;
    cur_avatar_status.damage_immunity_timer = 0;
    cur_avatar_status.fire_defense_boost_timer = 0;
    cur_avatar_status.paralysis_timer = 0;
    cur_avatar_status.slow_timer = 0;
    cur_avatar_status.darkness_phase_limit = 0;
    cur_avatar_status.darkness_phase = 0;
    cur_avatar_status.curse_phase_limit = 0;
    cur_avatar_status.curse_strength = 0;
    cur_avatar_status.poison_timer = 0;
    cur_avatar_status.fatal_fall_latch = 0;
    cur_avatar_status.hp = cur_avatar_status.hp_max;
    cur_avatar_status.mp = cur_avatar_status.mp_max;
    player_reset_view();
}

ADDRESS(0x8002b468, 0x1e4)
void player_initialize_state(void)
{
    MagicRecord *r;
    s32 i;

    cur_avatar_status.experience = 0;
    cur_avatar_status.weapon_growth_experience = -1;
    cur_avatar_status.level = 1;
    cur_avatar_status.hp_max = player_lv_grow_list[0].hp_max;
    cur_avatar_status.mp_max = player_lv_grow_list[0].mp_max;
    cur_avatar_status.base_physical_power = player_lv_grow_list[0].physical_power_step;
    for (i = 4; i != -1; i--) {
        cur_avatar_status.base_magic[i] = player_lv_grow_list[0].magic_step;
    }
    cur_avatar_status.wisdom = 0;
    cur_avatar_status.unk3e = 0;
    cur_avatar_status.gold = 0;
    cur_avatar_status.damage_red_overlay_scale = 0;
    cur_avatar_status.damage_red_overlay_decay = 0;
    cur_avatar_status.play_minutes = 0;
    cur_avatar_status.equipped_head_id = 0xff;
    cur_avatar_status.equipped_body_id = 0xff;
    cur_avatar_status.equipped_arm_id = 0xff;
    cur_avatar_status.equipped_leg_id = 0xff;
    cur_avatar_status.equipped_shield_id = 0xff;
    cur_avatar_status.equipped_accessory_id = 0xff;
    cur_avatar_status.equipped_extra_id = 0xff;
    cur_avatar_status.next_level_experience = player_lv_grow_list[0].experience_threshold;
    cur_avatar_status.base_magic[4] += 10;
    player_set_equipment_slot(0xff, 0xff);
    cur_avatar_status.selected_magic_record = 0;
    player_set_primary_magic_shortcut_id(0xff);
    player_equip_weapon(0);
    player_set_equipment_slot(0x2a, 1);
    player_set_secondary_magic_shortcut_id(0xff);
    player_set_secondary_item_shortcut_id(0xff);
    cur_avatar_status.pos.vy = -15360;
    cur_avatar_status.pos.vz = 0x4000;
    cur_avatar_status.camera_rotation_target.vx = 0;
    cur_avatar_status.camera_rotation_target.vy = 0;
    cur_avatar_status.camera_rotation_target.vz = 0;
    cur_avatar_status.pos.vx = 0x1f000;
    cur_avatar_status.map_layer_offset = 5;
    r = effect_status.magic_records;
    for (i = 95; i != -1; i--) {
        r->menu_available = 0;
        r++;
    }
    effect_status.magic_records[29].menu_available = 1;
    player_reset_status();
}

ADDRESS(0x8002b64c, 0xa4)
void game_initialize_session(void)
{
    cur_avatar_status.weapon_asset_buffer = player_weapon_asset_bufs;
    player_initialize_state();
    cur_avatar_status.se_volume = 0x80;
    cur_avatar_status.bgm_volume = 0x50;
    cur_avatar_status.vitals_display = 2;
    cur_avatar_status.compass_display = 2;
    cur_avatar_status.item_preview_enabled = 1;
    cur_avatar_status.walking_bob_enabled = 1;
    cur_avatar_status.brightness = 0x80;
    cur_avatar_status.button_style = 3;
    cur_avatar_status.button_layout = 0;
    cur_avatar_status.panel_translucent = 1;
    menu_apply_button_config();
    cur_avatar_status.force_actor_lifecycle_refresh = 0;
}

ADDRESS(0x8002b6f0, 0x70)
void player_clear_motion(void)
{
    cur_avatar_status.yaw_step = 0;
    cur_avatar_status.pitch_step = 0;
    cur_avatar_status.movement_speed = 0;
    cur_avatar_status.forward_velocity = 0;
    cur_avatar_status.strafe_velocity = 0;
    cur_avatar_status.pad_buttons &= action_button_masks[7] | action_button_masks[4] | action_button_masks[6]
        | action_button_masks[12];
}

ADDRESS(0x8002b760, 0xac)
void player_sync_position_to_map(void)
{
    cur_avatar_status.equipment_effect_ticks = 0;
    cur_avatar_status.pos.vy = collision_sample_map_layer_height(cur_avatar_status.map_layer_offset == 0
                                                                 ? 1 : 2,
                                                                 cur_avatar_status.pos.vx,
                                                                 cur_avatar_status.pos.vz, 800, 1700);
    player_update_collision_bounds();
    cur_avatar_status.force_actor_lifecycle_refresh = 1;
    cur_avatar_status.vertical_motion_state = 0;
    cur_avatar_status.vertical_velocity = 0;
    cur_avatar_status.reaction_mode = 0;
    player_clear_motion();
    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, 1);
}

ADDRESS(0x8002b80c, 0xcc)
s32 player_distance_to_point_in_cone(VECTOR *pos, s32 angle, s32 radius, s32 range)
{
    s32 hit = player_distance_to_point(pos->vx, 0xffff, pos->vz, radius, 0);
    s16 d;

    if (hit != -1) {
        d = (vector_xz_to_angle(cur_avatar_status.pos.vx - pos->vx,
                                cur_avatar_status.pos.vz - pos->vz) - angle) & 0xfff;
        if (d > 2048) {
            d = 0x1000 - d;
        }
        if (d > range) {
            hit = -1;
        }
    }
    return hit;
}

ADDRESS(0x8002b8d8, 0x50)
s32 player_distance_to_point(s32 x, s32 y, s32 z, s32 radius, s32 reach_up)
{
    return vector_distance_to_point(&cur_avatar_status.pos, x, y, z, radius, 1700, reach_up);
}

ADDRESS(0x8002b928, 0x110)
void player_set_primary_magic_shortcut_id(u8 id)
{
    SoundBank *bank = NULL;
    MagicRecord *r;
    s16 n;
    u16 se;
    SoundEffect *effect;

    sound_status.bank_select = 0;
    cur_avatar_status.primary_magic_shortcut_id = id;
    if (cur_avatar_status.primary_magic_shortcut_id != 0xff) {
        r = &effect_status.magic_records[cur_avatar_status.primary_magic_shortcut_id];
        if (r->sound_id != 0xff) {
            se = r->sound_id + 144;
            effect = &sound_status.effects[se];
            if ((n = effect->bank) != -1) {
                bank = &sound_status.banks[n];
            }
        }
    }
    if (id != 0xff) {
        r = &effect_status.magic_records[id];
        if (r->sound_id != 0xff) {
            se = r->sound_id + 144;
            if (bank != NULL && bank->slot == NULL) {
                n = sound_status.effects[se].bank;
                audio_queue_vab_stream(2, n + 94, n);
            }
        }
    }
}

ADDRESS(0x8002ba38, 0x124)
void player_set_secondary_magic_shortcut_id(u8 id)
{
    SoundBank *bank = NULL;
    MagicRecord *r;
    s16 n;
    u16 se;
    SoundEffect *effect;

    sound_status.bank_select = 1;
    cur_avatar_status.secondary_magic_shortcut_id = id;
    if (cur_avatar_status.secondary_magic_shortcut_id != 0xff) {
        r = &effect_status.magic_records[cur_avatar_status.secondary_magic_shortcut_id];
        if (r->sound_id != 0xff) {
            se = r->sound_id + 144;
            effect = &sound_status.effects[se];
            if ((n = effect->bank) != -1) {
                bank = &sound_status.banks[n];
            }
        }
    }
    if (id != 0xff) {
        r = &effect_status.magic_records[id];
        if (r->sound_id != 0xff) {
            se = r->sound_id + 144;
            if (bank != NULL && bank->slot == NULL) {
                n = sound_status.effects[se].bank;
                audio_queue_vab_stream(2, n + 94, n);
            }
        }
        cur_avatar_status.secondary_item_shortcut_id = 0xff;
    }
}

ADDRESS(0x8002bb5c, 0x28)
void player_set_secondary_item_shortcut_id(u8 id)
{
    cur_avatar_status.secondary_item_shortcut_id = id;
    if (id != 0xff) {
        cur_avatar_status.secondary_magic_shortcut_id = 0xff;
    }
}

ADDRESS(0x8002bb84, 0x23c)
void player_set_equipment_slot(u8 id, u8 slot)
{
    switch (slot) {
    case 0:
        cur_avatar_status.equipped_head_id = id;
        break;
    case 1:
        cur_avatar_status.equipped_body_id = id;
        break;
    case 2:
        cur_avatar_status.equipped_arm_id = id;
        break;
    case 3:
        cur_avatar_status.equipped_leg_id = id;
        break;
    case 4:
        cur_avatar_status.equipped_shield_id = id;
        break;
    case 5:
        cur_avatar_status.equipped_accessory_id = id;
        break;
    case 6:
        cur_avatar_status.equipped_extra_id = id;
        break;
    }
    if (cur_avatar_status.equipped_head_id != 0xff) {
        cur_avatar_status.equipped_head_record =
            &item_gear_recs[cur_avatar_status.equipped_head_id - EQUIPMENT_RECORD_FIRST_ID];
    } else {
        cur_avatar_status.equipped_head_record = NULL;
    }
    if (cur_avatar_status.equipped_body_id != 0xff) {
        cur_avatar_status.equipped_body_record =
            &item_gear_recs[cur_avatar_status.equipped_body_id - EQUIPMENT_RECORD_FIRST_ID];
    } else {
        cur_avatar_status.equipped_body_record = NULL;
    }
    if (cur_avatar_status.equipped_arm_id != 0xff) {
        cur_avatar_status.equipped_arm_record =
            &item_gear_recs[cur_avatar_status.equipped_arm_id - EQUIPMENT_RECORD_FIRST_ID];
    } else {
        cur_avatar_status.equipped_arm_record = NULL;
    }
    if (cur_avatar_status.equipped_leg_id != 0xff) {
        cur_avatar_status.equipped_leg_record =
            &item_gear_recs[cur_avatar_status.equipped_leg_id - EQUIPMENT_RECORD_FIRST_ID];
    } else {
        cur_avatar_status.equipped_leg_record = NULL;
    }
    if (cur_avatar_status.equipped_shield_id != 0xff) {
        cur_avatar_status.equipped_shield_record =
            &item_gear_recs[cur_avatar_status.equipped_shield_id - EQUIPMENT_RECORD_FIRST_ID];
    } else {
        cur_avatar_status.equipped_shield_record = NULL;
    }
    if (cur_avatar_status.equipped_accessory_id != 0xff) {
        cur_avatar_status.equipped_accessory_record =
            &item_gear_recs[cur_avatar_status.equipped_accessory_id - EQUIPMENT_RECORD_FIRST_ID];
    } else {
        cur_avatar_status.equipped_accessory_record = NULL;
    }
    if (cur_avatar_status.equipped_extra_id != 0xff) {
        cur_avatar_status.equipped_extra_record =
            &item_gear_recs[cur_avatar_status.equipped_extra_id - EQUIPMENT_RECORD_FIRST_ID];
    } else {
        cur_avatar_status.equipped_extra_record = NULL;
    }
    player_recalculate_combat_stats();
}

ADDRESS(0x8002bdc0, 0x280)
void player_equip_weapon(u8 id)
{
    u8 weapon = id;
    u8 effect[34] = {
        0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x04, 0xff, 0xff,
        0x07, 0x02, 0x05, 0x01, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
    };
    u16 entry;

    cur_avatar_status.weapon_charge_delay = 10;
    cur_avatar_status.attack_charge_current = 0;
    cur_avatar_status.attack_charge_committed = 0;
    cur_avatar_status.equipped_weapon_id = id;
    if (id != 0xff) {
        cur_avatar_status.equipped_weapon_record = &weapon_table[weapon];
        entry = weapon + 98;
        cd_archive_read(4, entry, cur_avatar_status.weapon_asset_buffer);
        asset_registry_set(0x20, cur_avatar_status.weapon_asset_buffer);
        if (effect[cur_avatar_status.equipped_weapon_id] != 0xff) {
            audio_queue_vab_stream(2, sound_status.effects[effect[cur_avatar_status.equipped_weapon_id]
                                   + 96].bank + 47,
                                   2);
        }
    }
    cur_avatar_status.weapon_attack_phase = -1;
    cur_avatar_status.weapon_animation_cache = 0;
    cur_avatar_status.weapon_magic_shots_remaining = 0;
    cur_avatar_status.weapon_guard_active = 0;
    if (cur_evt_state.unk2b != 0) {
        effect_status.magic_records[56].menu_available = 1;
        effect_status.magic_records[58].menu_available = 1;
        effect_status.magic_records[60].menu_available = 1;
        effect_status.magic_records[62].menu_available = 1;
        effect_status.magic_records[64].menu_available = 1;
        effect_status.magic_records[76].menu_available = 1;
    }
    if (cur_evt_state.unk31 != 0) {
        effect_status.magic_records[54].menu_available = 1;
        effect_status.magic_records[55].menu_available = 1;
        effect_status.magic_records[66].menu_available = 1;
        effect_status.magic_records[67].menu_available = 1;
        effect_status.magic_records[68].menu_available = 1;
        effect_status.magic_records[69].menu_available = 1;
        effect_status.magic_records[70].menu_available = 1;
        effect_status.magic_records[71].menu_available = 1;
        effect_status.magic_records[72].menu_available = 1;
        effect_status.magic_records[73].menu_available = 1;
        effect_status.magic_records[74].menu_available = 1;
        effect_status.magic_records[75].menu_available = 1;
    }
    player_recalculate_combat_stats();
}

ADDRESS(0x8002c040, 0x124)
void player_begin_weapon_attack(s32 hand)
{
    if (cur_avatar_status.weapon_attack_phase != -1) {
        return;
    }
    if (cur_avatar_status.equipped_weapon_id == 0xff) {
        return;
    }
    if (cur_avatar_status.paralysis_timer != 0) {
        return;
    }
    cur_avatar_status.weapon_attack_mode = hand;
    cur_avatar_status.weapon_attack_phase = 0;
    if (hand == 0) {
        cur_avatar_status.weapon_attack_window =
            cur_avatar_status.equipped_weapon_record->normal_attack_end_phase;
        cur_avatar_status.weapon_next_sound_phase =
            cur_avatar_status.equipped_weapon_record->normal_attack_sound_phase;
    } else {
        cur_avatar_status.weapon_attack_window =
            cur_avatar_status.equipped_weapon_record->alternate_attack_window_start;
        cur_avatar_status.weapon_next_sound_phase =
            cur_avatar_status.equipped_weapon_record->alternate_attack_sound_start_phase;
    }
    cur_avatar_status.attack_charge_committed = cur_avatar_status.attack_charge_current;
    if (cur_avatar_status.attack_charge_current == 5000
        && cur_avatar_status.magic_charge == cur_avatar_status.attack_charge_current) {
        cur_avatar_status.weapon_attack_fully_charged = 1;
        cur_avatar_status.weapon_magic_shots_configured =
            cur_avatar_status.equipped_weapon_record->magic_shots;
    } else {
        cur_avatar_status.weapon_attack_fully_charged = 0;
    }
    cur_avatar_status.attack_charge_current = 0;
    cur_avatar_status.unkcc = 0;
}

ADDRESS(0x8002c164, 0x1a8)
Actor *player_probe_view_target_and_vectors(s32 speed, VECTOR *offset, SVECTOR *dir, s32 *dist)
{
    SVECTOR rot;
    Actor *object;
    u8 *found;
    s32 y;

    if (offset != NULL) {
        rot.vx = -cur_avatar_status.camera_rotation.vx;
        rot.vy = cur_avatar_status.camera_rotation.vy;
        rot.vz = cur_avatar_status.camera_rotation.vz;
        vector_rotate_yxz(&rot, &cur_avatar_status.magic_origin_offset, offset);
        offset->vx += cur_avatar_status.pos.vx;
        y = offset->vy - 1600;
        offset->vy = y + cur_avatar_status.pos.vy;
        offset->vz += cur_avatar_status.pos.vz;
    }
    object = actor_find_best_in_cone(&cur_avatar_status.pos, cur_avatar_status.camera_rotation.vy,
                                     cur_avatar_status.camera_rotation.vx, 0x55f0, 0x200, 0x200, dist, 0);
    actor_status.player_view_target = object;
    if (object != NULL && (found = actor_find_target_of_type(&actor_status.groups[object->group_index],
                                                             0x82)) != NULL
        && !(object->flags & 0x100)) {
        actor_set_target(object, found);
    }
    if (dir != NULL) {
        rot.vx = cur_avatar_status.camera_rotation.vx;
        rot.vy = cur_avatar_status.camera_rotation.vy;
        rot.vz = cur_avatar_status.camera_rotation.vz;
        pitch_yaw_to_forward_vector(&rot, dir);
        vector3s_scale_shift12(speed, dir);
    }
    return object;
}

/*
 * Casts spell id (77 kinds, MagicRecord indices): aims along magic_origin_offset with
 * player_probe_view_target_and_vectors and spawns the spell's Effect records with effect_construct_record;
 * spells 68, 69, 72 and 73/74 spawn under the ids 120..124. Declared with an
 * ellipsis: the prologue spills all four argument registers and va_start
 * keeps id in its stack slot (read back as a byte for the spawns).
 */
ADDRESS(0x8002c30c, 0xb9c)
void player_dispatch_magic_effect(s32 id, ...)
{
    VECTOR off;
    SVECTOR dir;
    SVECTOR unused;
    s32 dist;
    s32 range;
    Actor *obj;
    Effect *made;
    SVECTOR *row;
    s32 i;
    s32 k;
    s32 index;
    s32 target;
    s32 n;
    s16 yaw;
    va_list args;

    va_start(args, id);
    switch (id) {
    case 0:
        player_probe_view_target_and_vectors(900, &off, &dir, &dist);
    spawn:
        effect_construct_record(10, 0x42, (u8)id, &off, &dir);
        break;
    case 6:
        player_probe_view_target_and_vectors(500, &off, &dir, &dist);
        goto spawn;
    case 62:
        player_probe_view_target_and_vectors(750, &off, &dir, &dist);
        effect_construct_record(10, 0x42, (u8)id, &off, &dir);
        break;
    case 54:
        for (i = 0; i < 4; i++) {
            k = 3 - i;
            cur_avatar_status.magic_origin_offset.vz = k * 800;
            player_probe_view_target_and_vectors(500, &off, &dir, &dist);
            effect_construct_record(10, 0x42, (u8)id, &off, &dir, k);
        }
        break;
    case 55:
        player_probe_view_target_and_vectors(500, &off, &dir, &dist);
        goto spawn;
    case 70:
        player_probe_view_target_and_vectors(500, &off, &dir, &dist);
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, 0);
        break;
    case 64:
        player_probe_view_target_and_vectors(440, &off, &dir, &dist);
        goto spawn;
    case 76:
        player_probe_view_target_and_vectors(450, &off, &dir, &dist);
        goto spawn;
    case 60:
        player_probe_view_target_and_vectors(320, &off, &dir, &dist);
        goto spawn;
    case 58:
        player_probe_view_target_and_vectors(550, &off, &dir, &dist);
        goto spawn;
    case 75:
        player_probe_view_target_and_vectors(720, &off, &dir, &dist);
        effect_construct_record(10, 0x42, 75, &off, &dir);
        break;
    case 15:
        player_probe_view_target_and_vectors(300, &off, &dir, &dist);
        off.vy += 1344;
        dir.vy = 0;
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation, 0);
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation, 1);
        break;
    case 7:
        effect_construct_record(10, 0x43, (u8)id, &cur_avatar_status.pos, 0, 0x1000, 0x100, 0x1000, 1);
        break;
    case 8:
        obj = player_probe_view_target_and_vectors(7000, &off, &dir, &dist);
        if (obj == NULL) {
            off.vx += dir.vx;
            off.vz += dir.vz;
            off.vy = cur_avatar_status.pos.vy;
        } else {
            off.vx = (obj->vel.vx << 14) / 650 + obj->pos.vx;
            off.vy = (obj->vel.vy << 14) / 650 + obj->pos.vy;
            off.vz = (obj->vel.vz << 14) / 650 + obj->pos.vz;
            if (collision_query_shapes_with_layer_sample(off.vx, off.vy, off.vz, 10, 10)) {
                off.vx = obj->pos.vx;
                off.vy = obj->pos.vy;
                off.vz = obj->pos.vz;
            }
        }
        effect_construct_record(10, 0x43, 0x5f, &off, 0);
        break;
    case 12:
        obj = player_probe_view_target_and_vectors(7000, &off, &dir, &dist);
        if (obj == NULL) {
            off.vx += dir.vx;
            off.vz += dir.vz;
            off.vy = cur_avatar_status.pos.vy;
        } else {
            off.vx = obj->pos.vx;
            off.vy = obj->pos.vy;
            off.vz = obj->pos.vz;
        }
        effect_construct_record(10, 0x42, 12, &off, 0);
        break;
    case 56:
    case 57:
        player_probe_view_target_and_vectors(700, &off, &dir, &dist);
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation);
        break;
    case 13:
        player_probe_view_target_and_vectors(700, &off, &dir, &dist);
    spawn2:
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation);
        break;
    case 11:
        player_probe_view_target_and_vectors(260, &off, &dir, &dist);
        dir.vx += (rand() >> 10) - 16;
        dir.vy += (rand() >> 10) - 16;
        effect_construct_record(10, 0x42, (u8)id, &off, &dir,
                                14 - cur_avatar_status.queued_magic_casts_remaining);
        break;
    case 21:
        player_probe_view_target_and_vectors(260, &off, &dir, &dist);
        effect_construct_record(10, 0x42, (u8)id, &off, &dir);
        break;
    case 19:
        player_probe_view_target_and_vectors(600, &off, &dir, &range);
        if (range != -1) {
            range = range / 600 - 8;
            if (range < 2) {
                range = 2;
            }
        } else {
            range = 10;
        }
        dir.vy = 0;
        effect_construct_record(10, 0x42, 0x71, &off, &dir, range);
        break;
    case 2:
        obj = player_probe_view_target_and_vectors(500, &off, &dir, &dist);
        if (obj == NULL) {
            target = 0xff;
        } else {
            target = obj - actor_status.objects;
        }
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, target,
                                cur_avatar_status.queued_magic_casts_remaining);
        break;
    case 1:
        player_probe_view_target_and_vectors(570, &off, &dir, &dist);
        effect_construct_record(10, 0x42, 1, &off, &dir, &cur_avatar_status.camera_rotation);
        cur_avatar_status.magic_origin_offset.vx += 1000;
        break;
    case 4:
    case 9:
    case 17:
        player_probe_view_target_and_vectors(250, &off, &dir, &dist);
        goto spawn2;
    case 16:
        player_probe_view_target_and_vectors(250, &off, &dir, &dist);
        goto spawn2;
    case 5:
        player_probe_view_target_and_vectors(250, &off, &dir, &dist);
        goto spawn2;
    case 20:
    case 71: {
        SVECTOR unused2;

        yaw = cur_avatar_status.camera_rotation.vy;
        cur_avatar_status.camera_rotation.vy = yaw - (cur_avatar_status.magic_origin_offset.vx << 1);
        player_probe_view_target_and_vectors(170, &off, &dir, &dist);
        cur_avatar_status.magic_origin_offset.vx += 100;
        cur_avatar_status.camera_rotation.vy = yaw;
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation, 600, 60,
                                128, 140, 160);
        break;
    }
    case 73:
    case 74:
        id = 0x7a;
        row = player_magic_spawn_offsets;
        cur_avatar_status.magic_origin_offset = *row;
        player_probe_view_target_and_vectors(600, &off, &dir, &dist);
        made = effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation);
        if (made != NULL) {
            index = made - effect_status.objects;
            row++;
            for (n = 3; n != -1; n--) {
                cur_avatar_status.magic_origin_offset = *row;
                player_probe_view_target_and_vectors(600, &off, &dir, &dist);
                effect_construct_record(10, 0x42, 0x5d, &off, &dir, index);
                row++;
            }
        }
        break;
    case 72:
        id = 0x78;
        row = player_magic_spawn_offsets;
        cur_avatar_status.magic_origin_offset = *row;
        player_probe_view_target_and_vectors(600, &off, &dir, &dist);
        made = effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation);
        if (made != NULL) {
            index = made - effect_status.objects;
            row++;
            for (n = 3; n != -1; n--) {
                cur_avatar_status.magic_origin_offset = *row;
                player_probe_view_target_and_vectors(600, &off, &dir, &dist);
                effect_construct_record(10, 0x42, 0x5c, &off, &dir, index);
                row++;
            }
        }
        break;
    case 68:
        player_probe_view_target_and_vectors(1000, &off, &dir, &dist);
        id = 0x7c;
        goto spawn;
    case 69:
        player_probe_view_target_and_vectors(1000, &off, &dir, &dist);
        id = 0x7b;
        goto spawn2;
    case 66:
        player_probe_view_target_and_vectors(900, &off, &dir, &dist);
        effect_construct_record(10, 0x42, (u8)id, &off, &dir, &cur_avatar_status.camera_rotation);
        break;
    case 26:
        effect_construct_record(10, 0x40, 0x1a, &cur_avatar_status.pos, &dir, 0);
        effect_construct_record(10, 0x40, 0x1a, &cur_avatar_status.pos, &dir, 1);
        cur_avatar_status.fire_defense_boost_timer = 1800;
        player_recalculate_combat_stats();
        break;
    case 28:
        effect_construct_record(10, 0x40, 0x1c, &cur_avatar_status.pos, &dir, 0);
        effect_construct_record(10, 0x40, 0x1c, &cur_avatar_status.pos, &dir, 1);
        cur_avatar_status.physical_defense_boost_timer = 1800;
        player_recalculate_combat_stats();
        break;
    case 27:
        effect_construct_record(10, 0x40, 0x1b, &cur_avatar_status.pos, &dir);
        cur_avatar_status.damage_immunity_timer = 200;
        player_recalculate_combat_stats();
        break;
    case 25:
        effect_construct_record(10, 0x40, 0x19, &cur_avatar_status.pos, &dir);
        break;
    case 23:
        effect_construct_record(10, 0x40, 0x17, &cur_avatar_status.pos, &dir);
        break;
    case 30:
        effect_construct_record(10, 0x40, 0x1e, &cur_avatar_status.pos, &dir);
        break;
    }
}

/* World position of vertex of the weapon model (no callers). */
ADDRESS(0x8002cea8, 0x138)
void player_sample_weapon_world_vertex(s32 vertex, VECTOR *out)
{
    SVECTOR v;
    SVECTOR rot;
    s32 y;

    rot.vx = cur_avatar_status.camera_rotation.vx
        - cur_avatar_status.equipped_weapon_record->rotation_offset_x;
    rot.vy = cur_avatar_status.camera_rotation.vy
        - cur_avatar_status.equipped_weapon_record->rotation_offset_y;
    rot.vz = cur_avatar_status.camera_rotation.vz
        + cur_avatar_status.equipped_weapon_record->rotation_offset_z;
    animation_sample_vertex(0x20, cur_avatar_status.weapon_attack_mode, cur_avatar_status.weapon_attack_phase,
                            vertex, &v);
    v.vx -= cur_avatar_status.equipped_weapon_record->position_offset_x;
    v.vy += cur_avatar_status.equipped_weapon_record->position_offset_y;
    v.vz -= cur_avatar_status.equipped_weapon_record->position_offset_z;
    vector_rotate_yxz(&rot, &v, out);
    out->vx += cur_avatar_status.pos.vx;
    out->vz += cur_avatar_status.pos.vz;
    y = out->vy - 1600;
    out->vy = y + (cur_avatar_status.camera_vertical_offset + cur_avatar_status.pos.vy
                   + cur_avatar_status.landing_vertical_offset);
}

/*
 * Whether the weapon's spell of hand (0: WeaponRecord.unk3, 1: alternate_magic_id) may be cast:
 * the spell must be learned; weapon 2 and 13..17 always may, others need
 * physical_power and every magic[] at 50 or more.
 */
ADDRESS(0x8002cfe0, 0x150)
s32 player_meets_weapon_magic_requirement(s32 hand)
{
    s32 n = 5;
    s32 low;

    if (cur_avatar_status.equipped_weapon_id != 0xff) {
        if (hand == 0 && cur_avatar_status.equipped_weapon_record->unk0[3] != 0xff
            && effect_status.magic_records[cur_avatar_status.equipped_weapon_record->unk0[3]].menu_available == 0) {
            return 0;
        }
        if (hand == 1 && cur_avatar_status.equipped_weapon_record->alternate_magic_id != 0xff
            && effect_status.magic_records[cur_avatar_status.equipped_weapon_record->alternate_magic_id].menu_available == 0) {
            return 0;
        }
    }
    switch (cur_avatar_status.equipped_weapon_id) {
    case 2:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
        return 1;
    }
    low = 999;
    while (--n != -1) {
        if (cur_avatar_status.magic[n] < low) {
            low = cur_avatar_status.magic[n];
        }
    }
    return cur_avatar_status.physical_power >= 50 && low >= 50;
}

/* Casts spell id (MagicRecord) along dir when mp covers its cost; pay takes the cost. */
ADDRESS(0x8002d130, 0x170)
void player_dispatch_weapon_magic(s32 id, s32 pay, s32 dir)
{
    VECTOR unused;
    MagicRecord *r = &effect_status.magic_records[id];

    if (cur_avatar_status.mp < r->mp_cost) {
        return;
    }
    cur_avatar_status.selected_magic_record = r;
    if (pay) {
        cur_avatar_status.magic_charge = 0;
        cur_avatar_status.magic_charge_delay = 0;
        cur_avatar_status.mp -= r->mp_cost;
        if (id == 0x36) {
            audio_play_sound(0x63, 0x6e);
        }
    }
    switch (id) {
    case 0x42:
        dir <<= 9;
        cur_avatar_status.magic_origin_offset.vx = rcos(dir) >> 3;
        cur_avatar_status.magic_origin_offset.vy = rsin(dir) >> 3;
        cur_avatar_status.magic_origin_offset.vz = 600;
        player_dispatch_magic_effect(id);
        break;
    case 0x47:
        audio_play_sound(0x61, 0x6e);
        cur_avatar_status.magic_origin_offset.vx = 0;
        cur_avatar_status.magic_origin_offset.vy = dir * 300 - 300;
        cur_avatar_status.magic_origin_offset.vz = 400;
        player_dispatch_magic_effect(id);
        break;
    default:
        cur_avatar_status.magic_origin_offset.vx = 200;
        cur_avatar_status.magic_origin_offset.vy = 200;
        cur_avatar_status.magic_origin_offset.vz = 400;
        player_dispatch_magic_effect(id);
        break;
    }
}

/*
 * Weapon swing and bow update (once per frame): weapon_attack_phase runs the swing of the
 * held weapon (equipped_weapon_record) from 0 to 4096; the bows (27, 28) spend an
 * arrow (event_item_counters_table) and aim it, the other weapons cast the weapon spell,
 * play the swing sound and probe for an Actor along the swing.
 */
ADDRESS(0x8002d2a0, 0xc4c)
void player_update_weapon_attack(void)
{
    SVECTOR v;
    SVECTOR rot;
    VECTOR a;
    VECTOR step;
    VECTOR from;
    VECTOR b;
    VECTOR probe;
    SVECTOR dir;
    s32 dist;
    WeaponRecord *w;
    Effect *arrow;
    Actor *obj;
    ActorGroup *def;
    VECTOR *target;
    s32 speed;
    s32 hit;
    s32 div;
    s32 limit;
    s32 add;
    u8 id;
    s32 item;
    s32 count;
    s32 d;
    s32 weight;
    s32 n;
    s32 found;
    s32 angle;

    w = cur_avatar_status.equipped_weapon_record;
    switch (cur_avatar_status.equipped_weapon_id) {
    case 27:
    case 28:
        if (cur_avatar_status.weapon_attack_phase != -1) {
        switch (cur_avatar_status.weapon_attack_mode) {
        case 0:
            if (cur_avatar_status.weapon_attack_phase == 0) {
                switch (cur_avatar_status.equipped_weapon_id) {
                case 27:
                    id = 0x51;
                    item = 0x93;
                    break;
                case 28:
                    id = 0x52;
                    item = 0x94;
                    break;
                }
                count = event_item_counters_table[0][item];
                if (count != 0) {
                    event_item_counters_table[0][item] = --count;
                    arrow = effect_construct_record(10, 0x42, id, &cur_avatar_status.pos, 0,
                                                    &cur_avatar_status.camera_rotation);
                    cur_avatar_status.weapon_effect = arrow;
                    if (arrow != NULL) {
                        arrow->phase = 99;
                    }
                    if (cur_avatar_status.equipped_weapon_id == 28) {
                        d = (rand() * 5 >> 15) + 3;
                        if (d >= cur_avatar_status.hp) {
                            cur_avatar_status.hp = 0;
                            player_death_begin(NULL);
                        } else {
                            cur_avatar_status.hp -= d;
                        }
                    }
                } else {
                    cur_avatar_status.weapon_effect = NULL;
                }
            }
            cur_avatar_status.weapon_attack_phase += w->attack_phase_step;
            if (cur_avatar_status.weapon_attack_phase >= 4095) {
                cur_avatar_status.weapon_attack_phase = 0xfff;
            }
            if (cur_avatar_status.weapon_attack_phase >= w->normal_attack_end_phase) {
                if (cur_avatar_status.attack_charge_current == 0) {
                    audio_play_sound(0x60, 0x6e);
                }
                cur_avatar_status.attack_charge_current = (cur_avatar_status.weapon_attack_phase - w->normal_attack_end_phase) * 5000 / (0xfff - w->normal_attack_end_phase);
            } else {
                cur_avatar_status.attack_charge_current = 0;
            }
            arrow = cur_avatar_status.weapon_effect;
            if (arrow != NULL) {
                rot.vx = cur_avatar_status.camera_rotation.vx - w->rotation_offset_x;
                rot.vy = cur_avatar_status.camera_rotation.vy - w->rotation_offset_y;
                rot.vz = cur_avatar_status.camera_rotation.vz + w->rotation_offset_z;
                animation_sample_vertex(0x20, cur_avatar_status.weapon_attack_mode,
                                        cur_avatar_status.weapon_attack_phase, w->initial_vertex_index, &v);
                v.vx -= w->position_offset_x;
                v.vy += w->position_offset_y;
                v.vz -= w->position_offset_z;
                vector_rotate_yxz(&rot, &v, &a);
                arrow->pos.vx = cur_avatar_status.pos.vx + a.vx;
                arrow->pos.vz = cur_avatar_status.pos.vz + a.vz;
                arrow->pos.vy = cur_avatar_status.pos.vy + a.vy + cur_avatar_status.camera_vertical_offset
                    + cur_avatar_status.landing_vertical_offset - 1600;
                animation_sample_vertex(0x20, cur_avatar_status.weapon_attack_mode,
                                        cur_avatar_status.weapon_attack_phase, w->final_vertex_index, &v);
                v.vx -= w->position_offset_x;
                v.vy += w->position_offset_y;
                v.vz -= w->position_offset_z;
                vector_rotate_yxz(&rot, &v, &b);
                vector_displacement_to_pitch_yaw(a.vx - b.vx, a.vy - b.vy, a.vz - b.vz, &arrow->angle);
            }
            if (cur_avatar_status.pad_buttons & action_button_masks[4]) {
                break;
            }
            if (arrow != NULL) {
                arrow->phase = 0;
                pitch_yaw_to_forward_vector(&arrow->angle, &arrow->vel);
                vector3s_scale_shift12(cur_avatar_status.attack_charge_current * 900 / 5000, &arrow->vel);
                arrow->updates_remaining = 50;
                arrow->arg.h.unk44 = arrow->pos.vy;
            }
            cur_avatar_status.weapon_attack_mode = 1;
            cur_avatar_status.weapon_attack_phase = 0;
            break;
        case 1:
            cur_avatar_status.weapon_attack_phase += 400;
            if (cur_avatar_status.weapon_attack_phase >= 4095) {
                cur_avatar_status.weapon_attack_phase = -1;
                cur_avatar_status.attack_charge_current = 0;
            }
            break;
        }
        } else {
            cur_avatar_status.attack_charge_current = 0;
            cur_avatar_status.weapon_charge_delay = 0;
        }
        break;
    case 0xff:
        break;
    default:
        if (cur_avatar_status.weapon_attack_phase != -1) {
            s32 scale;

            div = 0;
            if (cur_avatar_status.weapon_attack_mode == 0) {
                speed = w->attack_phase_step;
                hit = w->normal_attack_end_phase;
                limit = w->normal_attack_sound_phase;
                add = 0;
            } else {
                speed = w->alternate_attack_phase_step;
                hit = w->alternate_attack_end_phase;
                div = w->magic_phase_step;
                limit = w->alternate_attack_sound_end_phase;
                add = w->alternate_attack_sound_phase_step;
            }
            cur_avatar_status.weapon_attack_phase += speed;
            if (cur_avatar_status.weapon_attack_mode == 0 && w->unk0[3] != 0xff
                && cur_avatar_status.weapon_attack_fully_charged != 0
                && player_meets_weapon_magic_requirement(0)
                && (cur_avatar_status.pad_buttons & action_button_masks[6])) {
                if (cur_avatar_status.weapon_attack_phase >= w->magic_window_start
                    && cur_avatar_status.weapon_attack_phase <= w->magic_window_end) {
                    if (cur_avatar_status.weapon_magic_shots_configured != 0) {
                        player_dispatch_weapon_magic(w->unk0[3], cur_avatar_status.weapon_magic_shots_configured == w->magic_shots,
                                                     cur_avatar_status.weapon_magic_shots_configured);
                        cur_avatar_status.weapon_magic_shots_configured--;
                    }
                } else {
                    cur_avatar_status.weapon_magic_shots_configured = 0;
                }
            }
            if (cur_avatar_status.weapon_attack_phase >= cur_avatar_status.weapon_next_sound_phase && cur_avatar_status.weapon_attack_phase < cur_avatar_status.weapon_next_sound_phase + speed) {
                if (w->unk0[0] != 0xff) {
                    audio_play_sound(w->unk0[0], 0x50);
                }
                if (cur_avatar_status.weapon_next_sound_phase < limit) {
                    cur_avatar_status.weapon_next_sound_phase += add;
                } else {
                    cur_avatar_status.weapon_next_sound_phase = 5000;
                }
            }
            if (cur_avatar_status.weapon_attack_phase >= cur_avatar_status.weapon_attack_window
                && cur_avatar_status.weapon_attack_phase < cur_avatar_status.weapon_attack_window + speed) {
                if (cur_avatar_status.equipped_weapon_id == 0x21 && cur_avatar_status.magic_charge == 5000) {
                    cur_avatar_status.magic_origin_offset.vx = -200;
                    cur_avatar_status.magic_origin_offset.vy = 200;
                    cur_avatar_status.magic_origin_offset.vz = 400;
                    player_probe_view_target_and_vectors(900, &probe, &dir, &dist);
                    effect_construct_record(10, 0x42, 0, &probe, &dir);
                }
                cur_avatar_status.weapon_guard_active = 0;
                if (cur_avatar_status.weapon_attack_mode == 1) {
                    if (cur_avatar_status.equipped_weapon_id == 13
                        && (cur_avatar_status.pad_buttons & action_button_masks[6])) {
                        cur_avatar_status.weapon_guard_active = 1;
                        cur_avatar_status.weapon_attack_phase -= speed;
                        break;
                    }
                    player_dispatch_weapon_magic(w->alternate_magic_id,
                                                 cur_avatar_status.weapon_attack_phase >= hit,
                                                 (cur_avatar_status.weapon_attack_phase
                                                  - w->alternate_attack_window_start) / div);
                }
                if (cur_avatar_status.weapon_attack_phase >= hit) {
                    target = &from;
                    scale = cur_avatar_status.attack_charge_committed;
                    cur_avatar_status.weapon_attack_window = 5000;
                    cur_avatar_status.weapon_charge_delay = 10;
                    cur_avatar_status.attack_charge_current = 0;
                    from.vx = cur_avatar_status.pos.vx;
                    from.vy = cur_avatar_status.pos.vy - 1000;
                    from.vz = cur_avatar_status.pos.vz;
                } else {
                    target = NULL;
                    scale = cur_avatar_status.attack_charge_committed >> 2;
                    cur_avatar_status.weapon_attack_window += div;
                }
                v.vx = 0;
                v.vy = 0;
                v.vz = w->attack_reach;
                rot.vx = -cur_avatar_status.camera_rotation_target.vx;
                rot.vy = cur_avatar_status.camera_rotation_target.vy;
                rot.vz = 0;
                vector_rotate_yxz(&rot, &v, &step);
                step.vy = (3600 - w->attack_reach) * step.vy / 1800;
                step.vx /= 4;
                step.vy /= 4;
                step.vz /= 4;
                probe.vx = cur_avatar_status.pos.vx;
                probe.vy = cur_avatar_status.pos.vy - 800;
                probe.vz = cur_avatar_status.pos.vz;
                for (n = 3; n != -1; n--) {
                    probe.vx += step.vx;
                    probe.vy += step.vy;
                    probe.vz += step.vz;
                    found = actor_find_overlap_excluding_target_type3(probe.vx, probe.vy, probe.vz, 400, 600);
                    if (found != -1) {
                        obj = &actor_status.objects[found];
                        def = &actor_status.groups[obj->group_index];
                        angle = vector_xz_to_angle(obj->pos.vx - cur_avatar_status.pos.vx,
                                                   obj->pos.vz - cur_avatar_status.pos.vz);
                        if (angle_within_tolerance(cur_avatar_status.camera_rotation.vy, angle,
                                                   def->player_facing_tolerance)
                            && angle_within_tolerance(obj->angle.vy, angle + 2048,
                                                      def->actor_facing_tolerance)) {
                            actor_apply_magic_to_actor(found, cur_avatar_status.physical_power,
                                                       cur_avatar_status.attack_components[0],
                                                       cur_avatar_status.attack_components[1],
                                                       cur_avatar_status.attack_components[2],
                                                       cur_avatar_status.attack_components[3],
                                                       cur_avatar_status.attack_components[4],
                                                       cur_avatar_status.attack_components[5],
                                                       cur_avatar_status.attack_components[6],
                                                       cur_avatar_status.attack_components[7], scale, 80,
                                                       NULL, target);
                            break;
                        }
                    }
                }
            }
            if (cur_avatar_status.weapon_attack_phase >= 4096) {
                cur_avatar_status.weapon_attack_phase = -1;
                cur_avatar_status.weapon_magic_shots_configured = 0;
            }
        } else if (!(cur_avatar_status.pad_buttons & action_button_masks[4])) {
            if (cur_avatar_status.weapon_charge_delay == 0) {
                weight = w->unk0[1];
                if (cur_avatar_status.equipped_arm_id == 0x42) {
                    weight -= weight >> 2;
                }
                if (cur_avatar_status.equipped_arm_id == 0x44) {
                    weight += weight >> 1;
                }
                if (cur_avatar_status.equipped_arm_id == 0x46) {
                    weight >>= 1;
                }
                cur_avatar_status.attack_charge_current +=
                    player_charge_gain_for_rank(cur_avatar_status.physical_power, weight) * 2;
                if (cur_avatar_status.attack_charge_current > 5000) {
                    cur_avatar_status.attack_charge_current = 5000;
                }
            } else {
                cur_avatar_status.weapon_charge_delay--;
            }
        }
        break;
    }
}

/*
 * Starts spell id (MagicRecord): checks and pays its mp cost, then sets the
 * repeat count queued_magic_casts_remaining, the interval queued_magic_repeat_interval
 * and the direction magic_origin_offset that
 * player_update_actions_and_charge casts it with.
 */
ADDRESS(0x8002deec, 0x50c)
void player_select_magic_action(s32 id)
{
    SVECTOR unused;
    MagicRecord *r;
    s32 cost;

    if (cur_avatar_status.queued_magic_id != 0xff) {
        return;
    }
    if (id == 0xff) {
        return;
    }
    r = &effect_status.magic_records[id];
    cost = r->mp_cost;
    if (cur_avatar_status.equipped_head_id == 0x26) {
        cost <<= 1;
    }
    if (cur_avatar_status.equipped_body_id == 0x2e) {
        cost >>= 1;
    }
    if (cur_avatar_status.mp < cost) {
        notify_enqueue(9);
        return;
    }
    if (cur_avatar_status.equipped_weapon_id == 0x1f) {
        return;
    }
    switch (id) {
    case 26:
        if (cur_avatar_status.fire_defense_boost_timer != 0) {
            return;
        }
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 27:
        if (cur_avatar_status.damage_immunity_timer != 0) {
            return;
        }
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 28:
        if (cur_avatar_status.physical_defense_boost_timer != 0) {
            return;
        }
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 25:
        if (cur_avatar_status.poison_timer == 0) {
            return;
        }
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 23:
        if (cur_avatar_status.hp == cur_avatar_status.hp_max) {
            return;
        }
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 29:
        if (cur_avatar_status.magic_tint_phase_limit != 0) {
            return;
        }
        cur_avatar_status.magic_tint_phase_limit = 900;
        cur_avatar_status.mp -= cost;
        player_increment_magic_training(r);
        return;
    case 30:
        if (cur_avatar_status.hp == cur_avatar_status.hp_max && cur_avatar_status.curse_strength == 0
            && cur_avatar_status.darkness_phase == 0
            && cur_avatar_status.slow_timer == 0 && cur_avatar_status.paralysis_timer == 0) {
            return;
        }
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    }
    if (cur_avatar_status.magic_charge < 5000) {
        return;
    }
    switch (id) {
    case 23:
    case 25:
    case 26:
    case 27:
    case 28:
    case 30:
        player_increment_magic_training(r);
        break;
    }
    cur_avatar_status.magic_origin_offset.vx = -200;
    cur_avatar_status.magic_origin_offset.vy = 200;
    cur_avatar_status.magic_origin_offset.vz = 400;
    cur_avatar_status.magic_charge = 0;
    cur_avatar_status.weapon_charge_delay = 0;
    cur_avatar_status.queued_magic_id = id;
    cur_avatar_status.queued_magic_countdown = 1;
    cur_avatar_status.mp -= cost;
    switch (id) {
    case 4:
        cur_avatar_status.magic_origin_offset.vy = -512;
        cur_avatar_status.magic_origin_offset.vx = 0;
        cur_avatar_status.magic_origin_offset.vz = 2000;
        /* fall through */
    case 0:
    case 5:
    case 6:
    case 12:
    case 15:
    case 16:
    case 19:
    case 21:
    case 29:
    case 54:
    case 55:
    case 58:
    case 60:
    case 62:
    case 68:
    case 69:
    case 70:
    case 72:
    case 73:
    case 75:
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 76:
        cur_avatar_status.magic_origin_offset.vz = 600;
        cur_avatar_status.magic_origin_offset.vx = 0;
        cur_avatar_status.magic_origin_offset.vy = 0;
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 1:
        cur_avatar_status.magic_origin_offset.vx = -2000;
        cur_avatar_status.magic_origin_offset.vy = 1600;
        cur_avatar_status.magic_origin_offset.vz = 1000;
        cur_avatar_status.queued_magic_casts_remaining = 5;
        cur_avatar_status.queued_magic_repeat_interval = 3;
        break;
    case 56:
        cur_avatar_status.magic_origin_offset.vy = 500;
        cur_avatar_status.queued_magic_casts_remaining = 3;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 13:
        cur_avatar_status.magic_origin_offset.vy = 500;
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 64:
        cur_avatar_status.queued_magic_casts_remaining = 3;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 66:
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 9:
        cur_avatar_status.magic_origin_offset.vy = -400;
        cur_avatar_status.magic_origin_offset.vz = 400;
        cur_avatar_status.magic_origin_offset.vx = 0;
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 17:
        cur_avatar_status.magic_origin_offset.vz = 600;
        cur_avatar_status.magic_origin_offset.vy = -600;
        cur_avatar_status.magic_origin_offset.vx = 0;
        cur_avatar_status.queued_magic_casts_remaining = 1;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 20:
    case 71:
        cur_avatar_status.magic_origin_offset.vx = -200;
        cur_avatar_status.queued_magic_casts_remaining = 5;
        cur_avatar_status.queued_magic_repeat_interval = 2;
        break;
    case 2:
        cur_avatar_status.queued_magic_casts_remaining = 3;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 11:
        cur_avatar_status.queued_magic_casts_remaining = 12;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        break;
    case 7:
        cur_avatar_status.queued_magic_casts_remaining = 2;
        cur_avatar_status.queued_magic_repeat_interval = 1;
        cur_avatar_status.magic_origin_offset.vz = 0;
        cur_avatar_status.magic_origin_offset.vy = 0;
        cur_avatar_status.magic_origin_offset.vx = 0;
        break;
    case 8:
        cur_avatar_status.queued_magic_casts_remaining = 4;
        cur_avatar_status.queued_magic_repeat_interval = 2;
        break;
    }
    cur_avatar_status.selected_magic_record = r;
}

/*
 * Moves the player speed units along angle (4096ths of a turn), retrying
 * around what collision_query_world hit: the push of the last test, up to six turns
 * along a pushing MapObject (flags bit 2), around a wall point, shorter steps,
 * one axis at a time, and along the slope of the hit record. Returns 1 when
 * the player moved freely; frame_displacement keeps the last step tried.
 */
ADDRESS(0x8002e3f8, 0x8c4)
s32 player_move_horizontal(s32 angle, s32 speed)
{
    VECTOR p;
    MapObject *obj;
    s32 dx;
    s32 dz;
    s32 hit;
    s32 blocked;
    s32 tries;
    s32 lz;
    s32 step;
    s32 a;
    s32 r;
    s32 dx0;
    s32 dz0;
    s32 pushed;
    s32 cornered;
    s32 stepped;
    s32 walled;
    s32 moved;
    s16 rel;

    dx = rcos(angle) * speed >> 12;
    dz = rsin(angle) * speed >> 12;
    tries = 0;
    moved = 0;
    walled = 0;
    stepped = 0;
    cornered = 0;
    pushed = 0;
    dx0 = dx;
    dz0 = dz;
retry:
    p.vx = dx + cur_avatar_status.pos.vx;
    p.vz = dz + cur_avatar_status.pos.vz;
    hit = collision_query_world(p.vx, cur_avatar_status.pos.vy, p.vz, 800, 1700, 0x31);
    if (hit == 0) {
    free:
        if (map_world.collision_slope_push >= 64) {
            s32 px;
            s32 pz;

            px = rcos(-map_world.collision_slope_angle - 1024) * map_world.collision_slope_push >> 12;
            pz = rsin(-map_world.collision_slope_angle - 1024) * map_world.collision_slope_push >> 12;
            if (!pushed) {
                pushed = 1;
                dx += px;
                dz += pz;
                goto retry;
            }
        }
        moved = 1;
        cur_avatar_status.pos.vx = p.vx;
        cur_avatar_status.pos.vz = p.vz;
        cur_avatar_status.map_layer_offset = map_world.collision_layer_offset;
        goto done;
    }
    blocked = 0;
    if (!(hit & ~0x207)) {
        if (map_world.collision_floor_height + 1280 >= cur_avatar_status.pos.vy
            && cur_avatar_status.reaction_mode == 0
            && map_world.collision_height_limit - map_world.collision_floor_height < -1700) {
            goto free;
        }
        if (!(hit & 2)) {
            blocked = 1;
        }
    }
    if (map_world.collision_object_index != -1) {
        obj = &map_objects_slots.objects[map_world.collision_object_index];
        if (obj->flags & 4) {
            tries++;
            lz = speed;
            if (lz < 0) {
                lz = -lz;
            }
            if (tries == 1) {
                s32 to;

                to = ratan2(dz, dx) - 1024;
                rel = to - obj->angle.vy;
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (lz != 0) {
                    dz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1024) >> 12) - (lz * rsin(obj->angle.vy + 1024) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1024) >> 12) + (lz * rcos(obj->angle.vy + 1024) >> 12);
                goto retry;
            } else if (tries == 2) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (dz != 0) {
                    lz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1024) >> 12) - (lz * rsin(obj->angle.vy + 1024) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1024) >> 12) + (lz * rcos(obj->angle.vy + 1024) >> 12);
                goto retry;
            } else if (tries == 3) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (lz != 0) {
                    dz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1000) >> 12) - (lz * rsin(obj->angle.vy + 1000) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1000) >> 12) + (lz * rcos(obj->angle.vy + 1000) >> 12);
                goto retry;
            } else if (tries == 4) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (dz != 0) {
                    lz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1000) >> 12) - (lz * rsin(obj->angle.vy + 1000) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1000) >> 12) + (lz * rcos(obj->angle.vy + 1000) >> 12);
                goto retry;
            } else if (tries == 5) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (lz != 0) {
                    dz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1048) >> 12) - (lz * rsin(obj->angle.vy + 1048) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1048) >> 12) + (lz * rcos(obj->angle.vy + 1048) >> 12);
                goto retry;
            } else if (tries == 6) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (dz != 0) {
                    lz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1048) >> 12) - (lz * rsin(obj->angle.vy + 1048) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1048) >> 12) + (lz * rcos(obj->angle.vy + 1048) >> 12);
                goto retry;
            }
        }
    }
    if (hit & 0x30) {
        s32 wx;
        s32 wz;

        if (++walled == 2) {
            goto done;
        }
        collision_cache_load_hit_bounds();
        wx = map_world.collision_hit_position.vx - cur_avatar_status.pos.vx;
        wz = map_world.collision_hit_position.vz - cur_avatar_status.pos.vz;
        a = ratan2(dz - wz, dx - wx);
        r = map_world.collision_hit_radius + 880;
        dx = rcos(a) * r / 4096;
        dz = rsin(a) * r / 4096;
        p.vx = dx + map_world.collision_hit_position.vx;
        dx = p.vx - cur_avatar_status.pos.vx;
        p.vz = dz + map_world.collision_hit_position.vz;
        dz = p.vz - cur_avatar_status.pos.vz;
        goto retry;
    }
    if (!stepped) {
        for (step = speed - 22; step >= 0; step -= 22) {
            p.vx = cur_avatar_status.pos.vx + rcos(angle) * step / 4096;
            p.vz = cur_avatar_status.pos.vz + rsin(angle) * step / 4096;
            if (collision_query_world(p.vx, cur_avatar_status.pos.vy, p.vz, 800, 1700, 0x31) == 0) {
                cur_avatar_status.pos.vx = p.vx;
                cur_avatar_status.pos.vz = p.vz;
                break;
            }
        }
        stepped = 1;
    }
    if (blocked || (hit & 1)) {
    slide:
        if (dx != 0) {
            dx = 0;
            goto retry;
        }
        if (dz != 0) {
            dx = dx0;
            dz = 0;
            goto retry;
        }
    }
    if (hit & 2) {
        s32 slope;

        if (cornered) {
            goto slide;
        }
        slope = map_world.collision_layer->quarter_turns & 3;
        if (slope == 0 || slope == 2) {
            dz = (dx0 + dz0) >> 1;
            dx = dz;
            cornered = 1;
            goto retry;
        }
        dx = (dx0 - dz0) >> 1;
        dz = -dx;
        cornered = 1;
        goto retry;
    }
    moved = 0;
done:
    cur_avatar_status.frame_displacement.vx = dx;
    cur_avatar_status.frame_displacement.vz = dz;
    return moved;
}

/* The pad bits of the sequence player_update_actions_and_charge follows, 0xffff ended (rebuilt
 * every frame). */
DATA(0x8009c3cc, 0x8, ".sbss")
static u16 player_magic_attack_masks[4];

/* A fall from 20480 above the floor at speed 400 or more is death. */
ADDRESS(0x8002ecbc, 0x60)
void player_check_fall_death(void)
{
    if (cur_avatar_status.vertical_velocity >= 400 && map_world.collision_floor_height
        - cur_avatar_status.pos.vy > 20480) {
        player_death_begin(NULL);
        cur_avatar_status.fatal_fall_latch = 1;
    }
}

/* The landing sound, louder with the fall speed. */
ADDRESS(0x8002ed1c, 0x44)
void player_play_landing_sound(s32 speed)
{
    if (speed >= 320) {
        speed -= 320;
        if (speed > 896) {
            speed = 0x380;
        }
        audio_play_sound(7, (speed >> 3) + 32);
    }
}

/* Vertical movement per vertical_motion_state: steps up and down, falls, jumps and landings. */
ADDRESS(0x8002ed60, 0x5c0)
void player_update_vertical_motion(void)
{
    s32 y;
    s32 hit;
    s32 d;
    s32 v;
    s32 speed;
    s32 bob;

    collision_probe_floor_height(cur_avatar_status.pos.vx, cur_avatar_status.pos.vy, cur_avatar_status.pos.vz,
                                 800, 1700);
    cur_avatar_status.frame_displacement.vy = 0;
    switch (cur_avatar_status.vertical_motion_state) {
    case 0x10:
        player_check_fall_death();
        cur_avatar_status.pos.vy += cur_avatar_status.vertical_velocity;
        cur_avatar_status.frame_displacement.vy = cur_avatar_status.vertical_velocity;
        cur_avatar_status.vertical_velocity += 80;
        if (cur_avatar_status.pos.vy > map_world.collision_floor_height + 100) {
            cur_avatar_status.pos.vy = map_world.collision_floor_height;
            cur_avatar_status.vertical_motion_state = 0;
        }
        break;
    case 0x20:
        player_check_fall_death();
        cur_avatar_status.pos.vy += cur_avatar_status.vertical_velocity;
        if (cur_avatar_status.pos.vy <= map_world.collision_floor_height
            || cur_avatar_status.vertical_velocity >= 0) {
            if (cur_avatar_status.pos.vy
                - cur_avatar_status.vertical_velocity > map_world.collision_floor_height) {
                cur_avatar_status.pos.vy = map_world.collision_floor_height;
            }
            cur_avatar_status.vertical_motion_state = 0;
        }
        cur_avatar_status.frame_displacement.vy = cur_avatar_status.vertical_velocity;
        cur_avatar_status.vertical_velocity += 10;
        break;
    case 0x40:
        player_check_fall_death();
        y = cur_avatar_status.vertical_velocity + cur_avatar_status.pos.vy;
        hit = collision_query_world(cur_avatar_status.pos.vx, y, cur_avatar_status.pos.vz, 800, 1700, 0x31);
        if (hit == 0) {
            cur_avatar_status.pos.vy = y;
            cur_avatar_status.frame_displacement.vy = cur_avatar_status.vertical_velocity;
            cur_avatar_status.vertical_velocity += 40;
            cur_avatar_status.vertical_motion_pitch_offset = cur_avatar_status.vertical_velocity >> 1;
            break;
        }
        if (cur_avatar_status.vertical_velocity < 0) {
            cur_avatar_status.vertical_velocity = 0;
            break;
        }
        player_play_landing_sound(cur_avatar_status.vertical_velocity);
        if (cur_avatar_status.vertical_velocity >= 480) {
            v = cur_avatar_status.vertical_velocity * cur_avatar_status.vertical_velocity >> 12;
            player_apply_damage_reaction(NULL, v * v * v / 118000, 0);
        }
        if (hit & 4) {
            cur_avatar_status.pos.vy = map_world.collision_floor_height;
            if (cur_avatar_status.vertical_velocity >= 512) {
                cur_avatar_status.pos.vy = map_world.collision_floor_height
                    - (cur_avatar_status.vertical_velocity >> 1);
            }
        } else {
            collision_cache_load_hit_bounds();
            y = map_world.collision_hit_position.vy - map_world.collision_hit_height - 1;
            hit = collision_query_world(cur_avatar_status.pos.vx, y, cur_avatar_status.pos.vz, 800, 1700,
                                        0x31);
            if (hit == 0) {
                cur_avatar_status.pos.vy = y;
            }
        }
        cur_avatar_status.landing_vertical_offset = 1;
        cur_avatar_status.vertical_motion_state = 0x50;
        /* fall through */
    case 0x50:
        if (cur_avatar_status.landing_vertical_offset > 0) {
            cur_avatar_status.landing_vertical_offset += cur_avatar_status.vertical_velocity >> 2;
        }
        cur_avatar_status.vertical_velocity -= 300;
        if (cur_avatar_status.vertical_motion_pitch_offset > 0) {
            if (cur_avatar_status.vertical_velocity > 0) {
                cur_avatar_status.vertical_motion_pitch_offset += 10;
            } else {
                cur_avatar_status.vertical_motion_pitch_offset -= 30;
            }
        }
        if (cur_avatar_status.landing_vertical_offset <= 0
            && cur_avatar_status.vertical_motion_pitch_offset <= 0) {
            cur_avatar_status.vertical_motion_state = 0;
            cur_avatar_status.vertical_velocity = 0;
            cur_avatar_status.landing_vertical_offset = 0;
            cur_avatar_status.vertical_motion_pitch_offset = 0;
        }
        /* fall through */
    case 0:
        d = map_world.collision_floor_height - cur_avatar_status.pos.vy;
        if (d < 0) {
            if (d >= -256) {
                if (d < -128) {
                    cur_avatar_status.pos.vy -= 128;
                } else {
                    cur_avatar_status.pos.vy = map_world.collision_floor_height;
                }
                break;
            }
            if (d >= -512) {
                cur_avatar_status.pos.vy -= 256;
                break;
            }
            if (d >= -1024) {
                cur_avatar_status.pos.vy -= 512;
                break;
            }
            speed = cur_avatar_status.movement_speed;
            cur_avatar_status.vertical_motion_state = 0x20;
            cur_avatar_status.vertical_velocity = speed > 200 ? -300 : -150;
        } else {
            if (d <= 0) {
                break;
            }
            hit = collision_query_world(cur_avatar_status.pos.vx, cur_avatar_status.pos.vy + 1,
                                        cur_avatar_status.pos.vz, 800, 1700, 0x31);
            if (hit != 0) {
                break;
            }
            if (d <= 256) {
                if (d > 128) {
                    cur_avatar_status.pos.vy += 128;
                } else {
                    cur_avatar_status.pos.vy = map_world.collision_floor_height;
                }
                break;
            }
            if (d <= 512) {
                cur_avatar_status.pos.vy += 256;
                break;
            }
            cur_avatar_status.vertical_motion_state = d <= 1024 ? 0x10 : 0x40;
            cur_avatar_status.vertical_velocity = 40;
        }
        cur_avatar_status.landing_vertical_offset = 0;
        cur_avatar_status.vertical_motion_pitch_offset = 0;
        break;
    }
    if (cur_avatar_status.vertical_motion_state == 0) {
        if (cur_avatar_status.walking_bob_enabled != 0) {
            cur_avatar_status.walking_bob_phase = (cur_avatar_status.walking_bob_phase
                                                   + cur_avatar_status.movement_speed) & 0xfff;
            bob = rsin(cur_avatar_status.walking_bob_phase) >> 5;
            if (bob < 0) {
                bob = -bob;
            }
            cur_avatar_status.camera_vertical_offset = bob - (bob >> 2);
        } else {
            cur_avatar_status.camera_vertical_offset = 0;
        }
    }
    player_update_collision_bounds();
}

/* Mode 0x10 flight: moves the player along reaction.damage.vel, sliding on walls; 1 when it lands. */
ADDRESS(0x8002f320, 0x2a0)
s32 player_move_reaction_with_collision(void)
{
    VECTOR p;
    s32 hit;
    s32 friction;
    s32 d;

    p.vx = cur_avatar_status.pos.vx + cur_avatar_status.reaction.damage.vel.vx;
    p.vy = cur_avatar_status.pos.vy + cur_avatar_status.reaction.damage.vel.vy;
    p.vz = cur_avatar_status.pos.vz + cur_avatar_status.reaction.damage.vel.vz;
    if (collision_query_world(p.vx, p.vy, p.vz, 800, 1700, 0x31) == 0) {
    moved:
        if (cur_avatar_status.reaction.damage.vel.vy >= 300 && map_world.collision_floor_height
            - cur_avatar_status.pos.vy > 20480) {
            player_death_begin(NULL);
            cur_avatar_status.fatal_fall_latch = 1;
        }
        cur_avatar_status.pos.vx = p.vx;
        cur_avatar_status.pos.vy = p.vy;
        cur_avatar_status.pos.vz = p.vz;
        cur_avatar_status.reaction.damage.vel.vy += 32;
    } else {
        p.vy = cur_avatar_status.pos.vy;
        hit = collision_query_world(p.vx, p.vy, p.vz, 800, 1700, 0x31);
        if (hit == 0) {
            cur_avatar_status.reaction.damage.vel.vy = 1;
            hit = collision_query_world(p.vx, p.vy, p.vz, 800, 1700, 0x31);
            if (hit == 0) {
                friction = 0x20;
            slide:
                d = fixed_vector2_length(cur_avatar_status.reaction.damage.vel.vx,
                                         cur_avatar_status.reaction.damage.vel.vz);
                if (d <= friction) {
                    cur_avatar_status.reaction.damage.vel.vz = 0;
                    cur_avatar_status.reaction.damage.vel.vx = 0;
                    goto stop;
                }
                cur_avatar_status.reaction.damage.vel.vx = cur_avatar_status.reaction.damage.vel.vx * (d - friction) / d;
                cur_avatar_status.reaction.damage.vel.vz = cur_avatar_status.reaction.damage.vel.vz * (d - friction) / d;
                goto moved;
            }
        }
        if ((hit & ~0x205) || map_world.collision_floor_height + 256 < cur_avatar_status.pos.vy) {
            return 1;
        }
        p.vy = map_world.collision_floor_height;
        friction = 0x38;
        goto slide;
    stop:
        return 1;
    }
    player_update_collision_bounds();
    return 0;
}

/* Turning (yaw_step into camera_rotation_target.vy) and looking up and down (pitch_step into the pitch). */
ADDRESS(0x8002f5c0, 0x3fc)
void player_update_camera_rotation(void)
{
    s16 pitch;
    s32 limit;

    if (cur_avatar_status.pad_buttons & action_button_masks[2]) {
        cur_avatar_status.yaw_step += cur_avatar_status.turn_step_limit >> 2;
        if (cur_avatar_status.yaw_step > cur_avatar_status.turn_step_limit) {
            cur_avatar_status.yaw_step = cur_avatar_status.turn_step_limit;
        }
    } else if (cur_avatar_status.pad_buttons & action_button_masks[3]) {
        cur_avatar_status.yaw_step -= cur_avatar_status.turn_step_limit >> 2;
        if (cur_avatar_status.yaw_step < -cur_avatar_status.turn_step_limit) {
            cur_avatar_status.yaw_step = -cur_avatar_status.turn_step_limit;
        }
    } else if (cur_avatar_status.yaw_step > 0) {
        cur_avatar_status.yaw_step -= cur_avatar_status.turn_step_limit >> 2;
        if (cur_avatar_status.yaw_step < 0) {
            cur_avatar_status.yaw_step = 0;
        }
    } else if (cur_avatar_status.yaw_step < 0) {
        cur_avatar_status.yaw_step += cur_avatar_status.turn_step_limit >> 2;
        if (cur_avatar_status.yaw_step > 0) {
            cur_avatar_status.yaw_step = 0;
        }
    }
    cur_avatar_status.camera_rotation_target.vy = (cur_avatar_status.camera_rotation_target.vy
                                                   + cur_avatar_status.yaw_step) & 0xfff;
    if (cur_avatar_status.pad_buttons & action_button_masks[11]) {
        if (cur_avatar_status.pad_buttons & action_button_masks[9]) {
            pitch = cur_avatar_status.camera_rotation_target.vx;
            if (pitch > 2048) {
                limit = (0x1000 - pitch) >> 2;
                cur_avatar_status.pitch_step += 8;
                if (cur_avatar_status.pitch_step > limit) {
                    cur_avatar_status.pitch_step = limit;
                    if (limit == 0) {
                        cur_avatar_status.pitch_step = 0x1000 - cur_avatar_status.camera_rotation_target.vx;
                    }
                }
            } else if (pitch < 2048) {
                cur_avatar_status.pitch_step -= 8;
                if (cur_avatar_status.pitch_step < -(pitch >> 2)) {
                    cur_avatar_status.pitch_step = -(pitch >> 2);
                    if (cur_avatar_status.pitch_step == 0) {
                        cur_avatar_status.pitch_step = -cur_avatar_status.camera_rotation_target.vx;
                    }
                }
            }
        } else {
            cur_avatar_status.pitch_step += 3;
            if (cur_avatar_status.pitch_step > 32) {
                cur_avatar_status.pitch_step = 32;
            }
        }
    } else if (cur_avatar_status.pad_buttons & action_button_masks[9]) {
        cur_avatar_status.pitch_step -= 3;
        if (cur_avatar_status.pitch_step < -32) {
            cur_avatar_status.pitch_step = -32;
        }
    } else if (cur_avatar_status.pitch_step > 0) {
        cur_avatar_status.pitch_step -= 3;
        if (cur_avatar_status.pitch_step < 0) {
            cur_avatar_status.pitch_step = 0;
        }
    } else if (cur_avatar_status.pitch_step < 0) {
        cur_avatar_status.pitch_step += 3;
        if (cur_avatar_status.pitch_step > 0) {
            cur_avatar_status.pitch_step = 0;
        }
    }
    if (cur_avatar_status.pitch_step > 0) {
        cur_avatar_status.camera_rotation_target.vx = (cur_avatar_status.camera_rotation_target.vx
                                                       + cur_avatar_status.pitch_step) & 0xfff;
        if (angle_signed_greater_than(cur_avatar_status.camera_rotation_target.vx, 700)) {
            cur_avatar_status.camera_rotation_target.vx = 700;
        }
    } else if (cur_avatar_status.pitch_step < 0) {
        cur_avatar_status.camera_rotation_target.vx = (cur_avatar_status.camera_rotation_target.vx
                                                       + cur_avatar_status.pitch_step) & 0xfff;
        if (angle_signed_greater_than(0xd44, cur_avatar_status.camera_rotation_target.vx)) {
            cur_avatar_status.camera_rotation_target.vx = 0xd44;
        }
    }
}

/* Walking (forward_velocity) and strafing (strafe_velocity), then the move along the yaw. */
ADDRESS(0x8002f9bc, 0x420)
void player_update_horizontal_motion(void)
{
    s16 side;
    s16 ahead;
    s32 sx;
    s32 sz;
    s16 len;

    if (cur_avatar_status.pad_buttons & action_button_masks[0]) {
        ahead = cur_avatar_status.forward_velocity + (cur_avatar_status.movement_step_limit >> 2);
        if (ahead > cur_avatar_status.movement_step_limit) {
            cur_avatar_status.forward_velocity = cur_avatar_status.movement_step_limit;
        } else {
            cur_avatar_status.forward_velocity = ahead;
        }
    } else if (cur_avatar_status.pad_buttons & action_button_masks[1]) {
        ahead = cur_avatar_status.forward_velocity - (cur_avatar_status.movement_step_limit >> 2);
        if (ahead >= -cur_avatar_status.movement_step_limit) {
            cur_avatar_status.forward_velocity = ahead;
        } else {
            cur_avatar_status.forward_velocity = -cur_avatar_status.movement_step_limit;
        }
    } else if (cur_avatar_status.forward_velocity > 0) {
        cur_avatar_status.forward_velocity -= cur_avatar_status.movement_step_limit >> 3;
        if (cur_avatar_status.forward_velocity < 0) {
            cur_avatar_status.forward_velocity = 0;
        }
    } else if (cur_avatar_status.forward_velocity < 0) {
        cur_avatar_status.forward_velocity += cur_avatar_status.movement_step_limit >> 3;
        if (cur_avatar_status.forward_velocity > 0) {
            cur_avatar_status.forward_velocity = 0;
        }
    }
    if (cur_avatar_status.pad_buttons & action_button_masks[10]) {
        side = cur_avatar_status.strafe_velocity + (cur_avatar_status.movement_step_limit >> 2);
        if (side > cur_avatar_status.movement_step_limit) {
            cur_avatar_status.strafe_velocity = cur_avatar_status.movement_step_limit;
        } else {
            cur_avatar_status.strafe_velocity = side;
        }
    } else if (cur_avatar_status.pad_buttons & action_button_masks[8]) {
        side = cur_avatar_status.strafe_velocity - (cur_avatar_status.movement_step_limit >> 2);
        if (side >= -cur_avatar_status.movement_step_limit) {
            cur_avatar_status.strafe_velocity = side;
        } else {
            cur_avatar_status.strafe_velocity = -cur_avatar_status.movement_step_limit;
        }
    } else if (cur_avatar_status.strafe_velocity > 0) {
        cur_avatar_status.strafe_velocity -= cur_avatar_status.movement_step_limit >> 2;
        if (cur_avatar_status.strafe_velocity < 0) {
            cur_avatar_status.strafe_velocity = 0;
        }
    } else if (cur_avatar_status.strafe_velocity < 0) {
        cur_avatar_status.strafe_velocity += cur_avatar_status.movement_step_limit >> 2;
        if (cur_avatar_status.strafe_velocity > 0) {
            cur_avatar_status.strafe_velocity = 0;
        }
    }
    sx = cur_avatar_status.strafe_velocity;
    sx *= sx;
    sz = cur_avatar_status.forward_velocity;
    sz *= sz;
    len = SquareRoot0(sx + sz);
    if (len == 0) {
        ahead = 0;
        side = 0;
    } else {
        side = sx / len;
        if (cur_avatar_status.strafe_velocity < 0) {
            side = -side;
        }
        ahead = sz / len;
        if (cur_avatar_status.forward_velocity < 0) {
            ahead = -ahead;
        }
    }
    cur_avatar_status.movement_speed = SquareRoot0(side * side + ahead * ahead);
    player_move_horizontal((cur_avatar_status.camera_rotation_target.vy + 1024) & 0xfff, ahead);
    player_move_horizontal(cur_avatar_status.camera_rotation_target.vy & 0xfff, side);
}

ADDRESS(0x8002fddc, 0x40)
s32 item_id_is_consumable(s32 id)
{
    switch (id) {
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 119:
    case 120:
        return 1;
    }
    return 0;
}

ADDRESS(0x8002fe1c, 0x660)
void player_update_actions_and_charge(void)
{
    s32 id;
    s32 sum;
    s32 i;
    s32 weight;
    u16 mask;
    u16 *next;
    s32 flag;

    player_magic_attack_masks[3] = 0xffff;
    player_magic_attack_masks[0] = action_button_masks[7];
    player_magic_attack_masks[1] = action_button_masks[4];
    player_magic_attack_masks[2] = action_button_masks[6];
    if (cur_avatar_status.weapon_magic_shots_configured == 0
        && (cur_avatar_status.pad_buttons & action_button_masks[6])
        && !(cur_avatar_status.pad_buttons_previous & action_button_masks[6])
        && cur_avatar_status.weapon_magic_shots_remaining == 0) {
        player_select_magic_action(cur_avatar_status.primary_magic_shortcut_id);
    }
    if ((cur_avatar_status.pad_buttons & action_button_masks[12])
        && !(cur_avatar_status.pad_buttons_previous & action_button_masks[12])) {
        if (cur_avatar_status.secondary_magic_shortcut_id != 0xff) {
            player_select_magic_action(cur_avatar_status.secondary_magic_shortcut_id);
        }
        if (cur_avatar_status.secondary_item_shortcut_id != 0xff) {
            if (event_item_counters_table[0][cur_avatar_status.secondary_item_shortcut_id] != 0) {
                if (item_id_is_consumable(cur_avatar_status.secondary_item_shortcut_id)) {
                    menu_apply_item_effect(cur_avatar_status.secondary_item_shortcut_id);
                }
                event_scene_command_dispatch(&cur_avatar_status.pos,
                                             &cur_avatar_status.camera_rotation_target,
                                             cur_avatar_status.secondary_item_shortcut_id);
            } else {
                notify_enqueue(10);
            }
        }
    }
    if (cur_avatar_status.queued_magic_id != 0xff && --cur_avatar_status.queued_magic_countdown == 0) {
        player_dispatch_magic_effect(cur_avatar_status.queued_magic_id);
        if (--cur_avatar_status.queued_magic_casts_remaining == 0) {
            cur_avatar_status.queued_magic_id = 0xff;
        } else {
            cur_avatar_status.queued_magic_countdown = cur_avatar_status.queued_magic_repeat_interval;
        }
    }
    if ((cur_avatar_status.pad_buttons & action_button_masks[7])
        && (cur_avatar_status.pad_buttons_previous & action_button_masks[7])) {
        id = cur_avatar_status.equipped_leg_id;
        if (id != 0x4c) {
            if (cur_avatar_status.movement_speed_adjustment_decay_latch != 0) {
                cur_avatar_status.movement_speed_adjustment_decay_latch--;
            }
            cur_avatar_status.weapon_charge_delay = 1;
            cur_avatar_status.attack_charge_current -= 500;
            if ((s16)cur_avatar_status.attack_charge_current <= 0) {
                cur_avatar_status.attack_charge_current = 0;
                if (id == 0x4d || id == 0x4f) {
                    flag = 1;
                } else {
                    flag = 0x3c;
                }
                cur_avatar_status.weapon_charge_delay = flag;
            }
            cur_avatar_status.magic_charge -= 500;
            cur_avatar_status.magic_charge_delay = 1;
            if ((s16)cur_avatar_status.magic_charge <= 0) {
                cur_avatar_status.magic_charge = 0;
                if (cur_avatar_status.equipped_leg_id != 0x4d && cur_avatar_status.equipped_leg_id != 0x4f) {
                    cur_avatar_status.magic_charge_delay = 0x3c;
                } else {
                    cur_avatar_status.magic_charge_delay = 0;
                }
            }
            cur_avatar_status.damage_scale -= 128;
            if (cur_avatar_status.damage_scale < 1024) {
                cur_avatar_status.damage_scale = 0x400;
            }
            return;
        }
    }
    sum = 0;
    for (i = 4; i != -1; i--) {
        sum += cur_avatar_status.magic[i];
    }
    sum /= 5;
    if (cur_avatar_status.magic_charge_delay == 0) {
        if (cur_avatar_status.selected_magic_record == NULL) {
            cur_avatar_status.magic_charge += player_charge_gain_for_rank(sum, 0);
        } else {
            weight = cur_avatar_status.selected_magic_record->charge_rate;
            if (cur_avatar_status.equipped_head_id == 0x25) {
                weight -= weight >> 2;
            }
            if (cur_avatar_status.equipped_body_id == 0x30) {
                weight <<= 1;
            }
            cur_avatar_status.magic_charge += player_charge_gain_for_rank(sum, weight);
        }
    } else {
        cur_avatar_status.magic_charge_delay--;
    }
    if (cur_avatar_status.magic_charge > 5000) {
        cur_avatar_status.magic_charge = 5000;
    }
    cur_avatar_status.damage_scale += 128;
    if (cur_avatar_status.damage_scale > 4096) {
        cur_avatar_status.damage_scale = 0x1000;
    }
    cur_avatar_status.movement_speed_adjustment_decay_latch = 1;
    if (cur_avatar_status.weapon_magic_shots_remaining != 0) {
        cur_avatar_status.weapon_magic_shots_remaining--;
    } else {
        cur_avatar_status.magic_attack_mask_cursor = player_magic_attack_masks;
    }
    mask = action_button_masks[7] | action_button_masks[4] | action_button_masks[6];
    if ((cur_avatar_status.pad_buttons & mask) && !(cur_avatar_status.pad_buttons_previous & mask)
        && cur_avatar_status.equipped_weapon_record->alternate_attack_phase_step != 0) {
        if (player_meets_weapon_magic_requirement(1)
            && cur_avatar_status.equipped_weapon_record->alternate_magic_id != 0xff
            && (cur_avatar_status.pad_buttons & *cur_avatar_status.magic_attack_mask_cursor)
            && (cur_avatar_status.magic_attack_mask_cursor != player_magic_attack_masks
                || (cur_avatar_status.attack_charge_current == 5000
                    && cur_avatar_status.magic_charge == cur_avatar_status.attack_charge_current))) {
            next = cur_avatar_status.magic_attack_mask_cursor;
            cur_avatar_status.magic_attack_mask_cursor = next + 1;
            if (next[1] == 0xffff) {
                player_begin_weapon_attack(1);
                return;
            }
            cur_avatar_status.weapon_magic_shots_remaining = 20;
        } else {
            cur_avatar_status.weapon_magic_shots_remaining = 0;
        }
    }
    if (cur_avatar_status.weapon_charge_delay == 0 && cur_avatar_status.weapon_magic_shots_remaining == 0
        && (cur_avatar_status.pad_buttons & action_button_masks[4])
        && !(cur_avatar_status.pad_buttons_previous & action_button_masks[4])) {
        player_begin_weapon_attack(0);
    }
}

ADDRESS(0x8003047c, 0xec)
void player_update_reaction_rotation_offsets(void)
{
    cur_avatar_status.reaction.damage.angular_vel.vx =
        angle_velocity_step(0, cur_avatar_status.reaction_rotation_offset_x,
                            cur_avatar_status.reaction.damage.angular_vel.vx, 8, 4);
    cur_avatar_status.reaction.damage.angular_vel.vy =
        angle_velocity_step(0, cur_avatar_status.reaction_rotation_offset_y,
                            cur_avatar_status.reaction.damage.angular_vel.vy, 8, 4);
    cur_avatar_status.reaction.damage.angular_vel.vz =
        angle_velocity_step(0, cur_avatar_status.reaction_rotation_offset_z,
                            cur_avatar_status.reaction.damage.angular_vel.vz, 8, 4);
    cur_avatar_status.reaction_rotation_offset_x += cur_avatar_status.reaction.damage.angular_vel.vx;
    cur_avatar_status.reaction_rotation_offset_y += cur_avatar_status.reaction.damage.angular_vel.vy;
    cur_avatar_status.reaction_rotation_offset_z += cur_avatar_status.reaction.damage.angular_vel.vz;
}

ADDRESS(0x80030568, 0x70)
void player_render_frame_and_release_pool(void)
{
    u8 f9 = cur_avatar_status.vitals_display;
    u8 fa = cur_avatar_status.compass_display;

    cur_avatar_status.vitals_display = 0;
    cur_avatar_status.compass_display = 0;
    render_game_frame(NULL, NULL);
    cur_avatar_status.vitals_display = f9;
    cur_avatar_status.compass_display = fa;
    pool_release_all();
}

ADDRESS(0x800305d8, 0x1b4)
void player_handle_interaction_and_menu(void)
{
    s32 result;

    if (cur_avatar_status.reaction_mode == 0x11) {
        return;
    }
    if ((cur_avatar_status.pad_buttons & action_button_masks[7])
        && !(cur_avatar_status.pad_buttons_previous & action_button_masks[7])) {
        event_world_dispatch_interaction(&cur_avatar_status.pos, &cur_avatar_status.camera_rotation_target);
    }
    if ((cur_avatar_status.pad_buttons & action_button_masks[5])
        && !(cur_avatar_status.pad_buttons_previous & action_button_masks[5])
        && cur_avatar_status.weapon_attack_phase == -1) {
        player_render_frame_and_release_pool();
        do {
            cd_request_yield();
            resource_advance_transition();
        } while ((s16)asset_status.loading == 1 && asset_status.area_records_loaded == 0);
        map_reveal_visited_rects();
        resource_mark_area_models_reclaimable(asset_status.active_resource_ids[0]);
        result = menu_run_root_controller();
        if (result >= 0) {
            render_game_frame(NULL, NULL);
            event_scene_command_dispatch(&cur_avatar_status.pos, &cur_avatar_status.camera_rotation_target,
                                         result);
        } else if (result == -2) {
            main_end_flag = 2;
        } else if (result == -3) {
            player_restore_equipment_effects();
            player_reload_map_resources(asset_status.active_resource_ids[0],
                                        asset_status.active_resource_ids[0],
                                        asset_status.active_resource_ids[0],
                                        asset_status.active_resource_ids[3],
                                        asset_status.active_resource_ids[4], 0xff);
        }
        player_clear_motion();
    }
}

ADDRESS(0x8003078c, 0x68)
void player_reset_reaction_state(void)
{
    cur_avatar_status.reaction_mode = 0;
    cur_avatar_status.forward_velocity = 0;
    cur_avatar_status.strafe_velocity = 0;
    cur_avatar_status.reaction_rotation_offset_z = 0;
    cur_avatar_status.reaction_rotation_offset_y = 0;
    cur_avatar_status.reaction_rotation_offset_x = 0;
    cur_avatar_status.view_rotation_offset_z = 0;
    cur_avatar_status.view_rotation_offset_y = 0;
    cur_avatar_status.view_rotation_offset_x = 0;
    cur_avatar_status.camera_roll_offset = 0;
    cur_avatar_status.camera_yaw_offset = 0;
    cur_avatar_status.vertical_motion_pitch_offset = 0;
}

ADDRESS(0x800307f4, 0x1c)
void player_begin_movie_playback(u8 movie_id)
{
    cur_avatar_status.reaction_mode = 8;
    cur_avatar_status.reaction.movie.movie_id = movie_id;
}

ADDRESS(0x80030810, 0x54)
void player_begin_movie_approach(u8 movie_id)
{
    cur_avatar_status.reaction_mode = 7;
    cur_avatar_status.reaction.movie.movie_id = movie_id;
    cur_avatar_status.reaction.movie.step = 0;
    cur_avatar_status.reaction.movie.start_rotation = cur_avatar_status.camera_rotation_target;
}

ADDRESS(0x80030864, 0x54)
void player_begin_movie_return(u8 movie_id)
{
    cur_avatar_status.reaction_mode = 9;
    cur_avatar_status.reaction.movie.movie_id = movie_id;
    cur_avatar_status.reaction.movie.step = 0;
    cur_avatar_status.reaction.movie.start_rotation = cur_avatar_status.camera_rotation_target;
}

ADDRESS(0x800308b8, 0x3c)
void player_begin_rotation_reaction(SVECTOR *dir)
{
    cur_avatar_status.reaction_mode = 3;
    cur_avatar_status.reaction.damage.vel = *dir;
}

ADDRESS(0x800308f4, 0x14)
void func_800308f4(void)
{
    cur_avatar_status.reaction_mode = 4;
}

ADDRESS(0x80030908, 0xc0)
void player_begin_moving_damage_reaction(SVECTOR *dir, SVECTOR *push, s16 lift)
{
    if (cur_avatar_status.reaction_mode == 7 || cur_avatar_status.reaction_mode == 8
        || cur_avatar_status.reaction_mode == 9) {
        return;
    }
    cur_avatar_status.reaction_mode = 0x10;
    cur_avatar_status.reaction.damage.vel = *dir;
    cur_avatar_status.reaction.damage.angular_vel = *push;
    cur_avatar_status.damage_red_overlay_scale = 3500;
    cur_avatar_status.damage_red_overlay_decay = lift;
    cur_avatar_status.vertical_motion_state = 0x40;
    cur_avatar_status.vertical_velocity = cur_avatar_status.reaction.damage.vel.vy;
}

ADDRESS(0x800309c8, 0xa4)
void player_begin_rotation_only_damage_reaction(SVECTOR *dir, SVECTOR *push, s16 lift)
{
    if (cur_avatar_status.reaction_mode == 7 || cur_avatar_status.reaction_mode == 8
        || cur_avatar_status.reaction_mode == 9) {
        return;
    }
    cur_avatar_status.reaction_mode = 0x12;
    cur_avatar_status.reaction.damage.vel = *dir;
    cur_avatar_status.reaction.damage.angular_vel = *push;
    cur_avatar_status.damage_red_overlay_scale = 3500;
    cur_avatar_status.damage_red_overlay_decay = lift;
}

/* Death: mode 0x11, knocked along dir (NULL: in place). */
ADDRESS(0x80030a6c, 0x8c)
void player_death_begin(SVECTOR *dir)
{
    if (cur_avatar_status.reaction_mode == 0x11) {
        return;
    }
    cur_avatar_status.reaction_mode = 0x11;
    audio_play_sound(6, 0x6e);
    if (dir != NULL) {
        cur_avatar_status.reaction.damage.vel = *dir;
    }
    cur_avatar_status.reaction.damage.angular_vel.vx = 0;
    cur_avatar_status.death_transition_frame = 0;
}

ADDRESS(0x80030af8, 0x2c)
void player_begin_actor_overlap_bob(void)
{
    if (cur_avatar_status.reaction_mode == 0) {
        cur_avatar_status.reaction_mode = 5;
        cur_avatar_status.reaction.bob_phase = 0;
    }
}

/*
 * Steps an effect counter towards *timer (up while below it, else down with
 * the timer cleared); returns count scaled to scale over range frames, -1 when
 * both are 0.
 */
ADDRESS(0x80030b24, 0xbc)
s16 status_phase_step_scaled(s16 *count, s16 *timer, s32 range, s32 scale)
{
    s16 v;
    s16 n = *count;
    s16 t = 0;

    if (timer != NULL) {
        t = *timer;
    }
    if (n != 0 || t != 0) {
        if (n < t) {
            n++;
        } else {
            t = 0;
            n--;
        }
        if (n < range) {
            v = n * scale / range;
        } else {
            v = scale;
        }
        *count = n;
        if (timer != NULL) {
            *timer = t;
        }
        return v;
    }
    return -1;
}

ADDRESS(0x80030be0, 0x6c)
void player_adjust_hp(s32 amount)
{
    s32 hp = cur_avatar_status.hp;

    hp += amount;
    if (hp <= 0) {
        cur_avatar_status.hp = 0;
        player_death_begin(NULL);
    } else if (hp > cur_avatar_status.hp_max) {
        cur_avatar_status.hp = cur_avatar_status.hp_max;
    } else {
        cur_avatar_status.hp = hp;
    }
}

ADDRESS(0x80030c4c, 0x20)
void player_adjust_hp_on_weapon_hit(s32 amount)
{
    player_adjust_hp(amount);
}

ADDRESS(0x80030c6c, 0x50)
void player_adjust_mp(s32 amount)
{
    s32 v = cur_avatar_status.mp;

    v += amount;
    if (v <= 0) {
        cur_avatar_status.mp = 0;
    } else if (v > cur_avatar_status.mp_max) {
        cur_avatar_status.mp = cur_avatar_status.mp_max;
    } else {
        cur_avatar_status.mp = v;
    }
}

ADDRESS(0x80030cbc, 0xac)
void player_apply_equipment_hp_tick(EquipmentRecord *r)
{
    if (r->hp_regen_interval != 0 && cur_avatar_status.equipment_effect_ticks % r->hp_regen_interval == 0) {
        player_adjust_hp(1);
    }
    if (r->hp_drain_interval != 0 && cur_avatar_status.equipment_effect_ticks % r->hp_drain_interval == 0) {
        player_adjust_hp(-1);
    }
}

ADDRESS(0x80030d68, 0xac)
void player_apply_equipment_mp_tick(EquipmentRecord *r)
{
    if (r->mp_regen_interval != 0 && cur_avatar_status.equipment_effect_ticks % r->mp_regen_interval == 0) {
        player_adjust_mp(1);
    }
    if (r->mp_drain_interval != 0 && cur_avatar_status.equipment_effect_ticks % r->mp_drain_interval == 0) {
        player_adjust_mp(-1);
    }
}

ADDRESS(0x80030e14, 0x1b8)
void player_update_status_visuals(void)
{
    s16 v;
    s16 t;

    v = status_phase_step_scaled(&cur_avatar_status.darkness_phase, &cur_avatar_status.darkness_phase_limit,
                                 0x40, 0xc00);
    if (v != -1) {
        light_interpolate_color_and_fog(10, 10, 10, 5000, 6000, v);
        set_accumulated_color_overlay(0x80, 0x80, 0x80, v >> 2, 2);
    }
    v = status_phase_step_scaled(&cur_avatar_status.magic_tint_phase,
                                 &cur_avatar_status.magic_tint_phase_limit, 0x40, 0xe10);
    if (v != -1) {
        light_interpolate_color_and_fog(0xdc, 0xdc, 0xa0, 0xffff, 18000, v);
    }
    if (cur_avatar_status.damage_red_overlay_scale != 0) {
        cur_avatar_status.damage_red_overlay_scale -= cur_avatar_status.damage_red_overlay_decay;
        if (cur_avatar_status.damage_red_overlay_scale < 0) {
            cur_avatar_status.damage_red_overlay_scale = 0;
        }
        if (cur_avatar_status.damage_red_overlay_scale >= 4096) {
            t = 4096;
        } else {
            t = cur_avatar_status.damage_red_overlay_scale;
        }
        accumulate_color_overlay(0x3c, 0, 0, t, 1);
    }
    v = status_phase_step_scaled(&cur_avatar_status.curse_strength, &cur_avatar_status.curse_phase_limit,
                                 0x40, 0xc00);
    if (v != -1) {
        if (cur_avatar_status.curse_strength == 0) {
            player_recalculate_combat_stats();
        }
        accumulate_color_overlay(0x1e, 0x1e, 10, v, 1);
    }
}

/*
 * The player's frame: marks the map cell visited, reads the pad, steps the
 * walk speeds (movement_step_limit/turn_step_limit), runs the reaction_mode, then the per-frame
 * timers, the view angle sums, the Actor hit test, player_update_weapon_attack and the
 * effects of the weapon and the EquipmentRecord slots.
 */
ADDRESS(0x80030fcc, 0x1524)
void player_update_frame(void)
{
    MovieRecord *rec;
    MovieRecord *movie;
    s32 t;
    s32 u;
    s32 v;
    s32 hit;
    s32 i;
    s32 cz;
    u8 c;
    s32 part;
    Effect *o4c;

    actor_status.actor_overlap_exclusion_flags = 4;
    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, -1);
    if ((s16)asset_status.loading == 0
        || ((s16)asset_status.loading == 1 && asset_status.step != 0 && asset_status.step != 1
            && asset_status.step != 2 && asset_status.step != 3 && asset_status.step != 0xf0)) {
        cz = cur_avatar_status.pos.vz >> 11;
        map_visited_cells[cur_avatar_status.pos.vx >> 11][cz >> 3] |= 1 << (7 - cz % 8);
    }
    player_update_status_visuals();
    cur_avatar_status.pad_buttons = PadRead(1);
    if (cur_avatar_status.pad_buttons & action_button_masks[12]) {
        cd_report_error(3);
    }
    if (cur_avatar_status.pad_buttons & action_button_masks[13]) {
        menu_run_pause_screen();
    }
    cur_avatar_status.movement_step_limit = 200;
    cur_avatar_status.turn_step_limit = 32;
    if (!(cur_avatar_status.pad_buttons & (action_button_masks[0] | action_button_masks[1]))) {
        cur_avatar_status.turn_step_limit = 40;
    }
    if (cur_avatar_status.slow_timer != 0) {
        if (cur_avatar_status.slow_timer < 64) {
            cur_avatar_status.movement_speed_adjustment_q12 += 100;
            if (cur_avatar_status.movement_speed_adjustment_q12 > 0) {
                cur_avatar_status.movement_speed_adjustment_q12 = 0;
            }
        } else {
            cur_avatar_status.movement_speed_adjustment_q12 -= 100;
            if (cur_avatar_status.movement_speed_adjustment_q12 < -3300) {
                cur_avatar_status.movement_speed_adjustment_q12 = -3300;
            }
        }
        cur_avatar_status.turn_step_limit >>= 1;
        cur_avatar_status.slow_timer--;
    } else if (cur_avatar_status.movement_speed_adjustment_decay_latch == 0) {
        cur_avatar_status.movement_speed_adjustment_q12 += 800;
        if (cur_avatar_status.movement_speed_adjustment_q12 > 2800) {
            cur_avatar_status.movement_speed_adjustment_q12 = 2800;
        }
    } else {
        cur_avatar_status.movement_speed_adjustment_q12 -= 800;
        if (cur_avatar_status.movement_speed_adjustment_q12 < 0) {
            cur_avatar_status.movement_speed_adjustment_q12 = 0;
        }
    }
    cur_avatar_status.movement_step_limit += cur_avatar_status.movement_speed_adjustment_q12
        * cur_avatar_status.movement_step_limit >> 12;
    cur_avatar_status.turn_step_limit += cur_avatar_status.movement_speed_adjustment_q12
        * cur_avatar_status.turn_step_limit >> 13;
    if (cur_avatar_status.paralysis_timer != 0) {
        cur_avatar_status.movement_step_limit = 0;
        cur_avatar_status.turn_step_limit = 0;
        cur_avatar_status.paralysis_timer--;
        cur_avatar_status.strafe_velocity >>= 1;
        cur_avatar_status.forward_velocity >>= 1;
        if ((cur_avatar_status.paralysis_timer & 3) == 3) {
            accumulate_color_overlay(0x3c, 0x1e, 0, 0xc00, 1);
        }
    }
    if (cur_avatar_status.weapon_guard_active != 0) {
        cur_avatar_status.movement_step_limit >>= 1;
        cur_avatar_status.turn_step_limit >>= 1;
    }
    switch (cur_avatar_status.reaction_mode) {
    case 0:
    case 4:
        player_update_actions_and_charge();
        player_update_camera_rotation();
        player_update_horizontal_motion();
        player_update_vertical_motion();
        player_handle_interaction_and_menu();
        break;
    case 8:
        movie = &movie_state.movies[cur_avatar_status.reaction.movie.movie_id];
        movie_update_state();
        if (movie->overlay_mode == 0x82) {
            render_frames_with_color_overlay(0x82, 0x1000, 0x1000, 0);
            cd_request_wait_idle();
            render_frames_with_color_overlay(movie->overlay_mode, 0x1000, 0, -256);
            render_set_color_overlay(0xff, 0, 0, 0);
        } else if (movie->overlay_mode == 1) {
            MoveImage(&scene_display_sys.disp[scene_display_sys.frame].disp,
                      scene_display_sys.draw[scene_display_sys.frame].clip.x,
                      scene_display_sys.draw[scene_display_sys.frame].clip.y);
            DrawSync(0);
            render_frames_with_color_overlay(1, 0x1000, 0x1000, 0);
            cd_request_wait_idle();
            render_frames_with_color_overlay(movie->overlay_mode, 0x1000, 0, -256);
        } else {
            MoveImage(&scene_display_sys.disp[scene_display_sys.frame].disp,
                      scene_display_sys.draw[scene_display_sys.frame].clip.x,
                      scene_display_sys.draw[scene_display_sys.frame].clip.y);
            DrawSync(0);
            render_frames_with_color_overlay(1, 0, 0, 0);
            cd_request_wait_idle();
        }
        break;
    case 7:
        rec = &movie_state.movies[cur_avatar_status.reaction.movie.movie_id];
        t = cur_avatar_status.reaction.movie.step << 7;
        if (rec->approach_x != -1) {
            cur_avatar_status.pos.vx = fixed_lerp_q12(cur_avatar_status.pos.vx, rec->approach_x, t + 1);
            cur_avatar_status.pos.vz = fixed_lerp_q12(cur_avatar_status.pos.vz, rec->approach_z, t + 1);
        }
        if (rec->approach_y != -1) {
            cur_avatar_status.pos.vy = fixed_lerp_q12(cur_avatar_status.pos.vy, rec->approach_y, t + 1);
        }
        if (rec->approach_angle_x != -1) {
            cur_avatar_status.view_rotation_offset_x = angle_lerp_shortest_q12(0, rec->approach_angle_x, t
                                                                               + 1);
            cur_avatar_status.camera_rotation_target.vx =
                angle_lerp_shortest_q12(cur_avatar_status.reaction.movie.start_rotation.vx, 0,
                                                                                  t + 1);
        }
        if (rec->approach_angle_y != -1) {
            cur_avatar_status.view_rotation_offset_y = 0;
            cur_avatar_status.camera_rotation_target.vy =
                angle_lerp_shortest_q12(cur_avatar_status.reaction.movie.start_rotation.vy,
                                                                                  rec->approach_angle_y,
                                        t + 1);
        }
        if (rec->approach_angle_z != -1) {
            cur_avatar_status.view_rotation_offset_z = angle_lerp_shortest_q12(0, rec->approach_angle_z, t
                                                                               + 1);
            cur_avatar_status.camera_rotation_target.vz =
                angle_lerp_shortest_q12(cur_avatar_status.reaction.movie.start_rotation.vz, 0,
                                                                                  t + 1);
        }
        if (0x7f - cur_avatar_status.reaction.movie.step * 4 < 0) {
            SsSetMVol(0, 0);
        } else {
            SsSetMVol(0x7f - cur_avatar_status.reaction.movie.step * 4,
                      0x7f - cur_avatar_status.reaction.movie.step * 4);
        }
        if (cur_avatar_status.reaction.movie.step++ >= 32) {
            if (cur_avatar_status.bgm_volume != 0 && sound_status.seq_playing == 1) {
                SsSeqPause(sound_status.seq_id);
            }
            player_begin_movie_playback(cur_avatar_status.reaction.movie.movie_id);
            if (rec->approach_angle_x != -1) {
                cur_avatar_status.camera_rotation_target.vx = rec->approach_angle_x;
                cur_avatar_status.view_rotation_offset_x = 0;
            }
            if (rec->approach_angle_y != -1) {
                cur_avatar_status.camera_rotation_target.vy = rec->approach_angle_y;
                cur_avatar_status.view_rotation_offset_y = 0;
            }
            if (rec->approach_angle_z != -1) {
                cur_avatar_status.camera_rotation_target.vz = rec->approach_angle_z;
                cur_avatar_status.view_rotation_offset_z = 0;
            }
            movie_state.stream_status = 1;
            if (rec->fade_back != 0) {
                render_frames_with_color_overlay(rec->overlay_mode, 0, 0x1000, 0x100);
            }
        }
        break;
    case 9:
        rec = &movie_state.movies[cur_avatar_status.reaction.movie.movie_id];
        u = cur_avatar_status.reaction.movie.step << 7;
        if (rec->return_x != -1) {
            cur_avatar_status.pos.vx = fixed_lerp_q12(cur_avatar_status.pos.vx, rec->return_x, u);
            cur_avatar_status.pos.vz = fixed_lerp_q12(cur_avatar_status.pos.vz, rec->return_z, u);
        }
        if (rec->return_y != -1) {
            cur_avatar_status.pos.vy = fixed_lerp_q12(cur_avatar_status.pos.vy, rec->return_y, u);
        }
        if (rec->return_angle_x != -1) {
            cur_avatar_status.view_rotation_offset_x = angle_lerp_shortest_q12(0, rec->return_angle_x, u);
            cur_avatar_status.camera_rotation_target.vx =
                angle_lerp_shortest_q12(cur_avatar_status.reaction.movie.start_rotation.vx, 0,
                                                                                  u);
        }
        if (rec->return_angle_y != -1) {
            cur_avatar_status.view_rotation_offset_y = 0;
            cur_avatar_status.camera_rotation_target.vy =
                angle_lerp_shortest_q12(cur_avatar_status.reaction.movie.start_rotation.vy,
                                                                                  rec->return_angle_y, u);
        }
        if (rec->return_angle_z != -1) {
            cur_avatar_status.view_rotation_offset_z = angle_lerp_shortest_q12(0, rec->return_angle_z, u);
            cur_avatar_status.camera_rotation_target.vz =
                angle_lerp_shortest_q12(cur_avatar_status.reaction.movie.start_rotation.vz, 0,
                                                                                  u);
        }
        if (cur_avatar_status.reaction.movie.step == 0 && cur_avatar_status.bgm_volume != 0
            && sound_status.seq_playing == 1) {
            SsSeqReplay(sound_status.seq_id);
        }
        if (cur_avatar_status.reaction.movie.step * 4 < 0x7f) {
            SsSetMVol(cur_avatar_status.reaction.movie.step * 4, cur_avatar_status.reaction.movie.step * 4);
        } else {
            SsSetMVol(0x7f, 0x7f);
        }
        if (cur_avatar_status.reaction.movie.step++ >= 32) {
            player_reset_reaction_state();
            if (rec->return_angle_x != -1) {
                cur_avatar_status.camera_rotation_target.vx = rec->return_angle_x;
                cur_avatar_status.view_rotation_offset_x = 0;
            }
            if (rec->return_angle_y != -1) {
                cur_avatar_status.camera_rotation_target.vy = rec->return_angle_y;
                cur_avatar_status.view_rotation_offset_y = 0;
            }
            if (rec->return_angle_z != -1) {
                cur_avatar_status.camera_rotation_target.vz = rec->return_angle_z;
                cur_avatar_status.view_rotation_offset_z = 0;
            }
            movie_state.stream_status = 1;
        }
        break;
    case 3:
        if (player_move_reaction_with_collision()) {
            player_reset_reaction_state();
        }
        player_handle_interaction_and_menu();
        break;
    case 5:
        player_update_actions_and_charge();
        player_update_camera_rotation();
        player_update_horizontal_motion();
        cur_avatar_status.camera_vertical_offset = 0;
        player_update_vertical_motion();
        cur_avatar_status.camera_vertical_offset += (rcos(cur_avatar_status.reaction.bob_phase) >> 4) - 256;
        cur_avatar_status.view_rotation_offset_z = rsin(cur_avatar_status.reaction.bob_phase) >> 6;
        cur_avatar_status.reaction.bob_phase = (cur_avatar_status.reaction.bob_phase + 128) & 0xfff;
        if (cur_avatar_status.reaction.bob_phase == 0) {
            cur_avatar_status.view_rotation_offset_z = 0;
            player_reset_reaction_state();
        }
        player_handle_interaction_and_menu();
        break;
    case 0x10:
        player_update_actions_and_charge();
        player_update_camera_rotation();
        player_update_horizontal_motion();
        cur_avatar_status.reaction.damage.vel.vy = 1;
        player_move_reaction_with_collision();
        player_update_reaction_rotation_offsets();
        /* reaction_rotation_offset_x and _y are tested as one word (lw). */
        if (*(s32 *)&cur_avatar_status.reaction_rotation_offset_x == 0
            && cur_avatar_status.reaction_rotation_offset_z == 0) {
            player_reset_reaction_state();
        }
        player_update_vertical_motion();
        player_handle_interaction_and_menu();
        break;
    case 0x12:
        player_update_reaction_rotation_offsets();
        if (*(s32 *)&cur_avatar_status.reaction_rotation_offset_x == 0
            && cur_avatar_status.reaction_rotation_offset_z == 0) {
            player_reset_reaction_state();
        }
        player_handle_interaction_and_menu();
        break;
    case 0x11:
        cur_avatar_status.hp = 0;
        cur_avatar_status.reaction.damage.angular_vel.vx =
            angle_velocity_step(-1024, cur_avatar_status.reaction_rotation_offset_x,
                                cur_avatar_status.reaction.damage.angular_vel.vx, 8, 4);
        cur_avatar_status.reaction_rotation_offset_x += cur_avatar_status.reaction.damage.angular_vel.vx
            * 3 >> 1;
        if (cur_avatar_status.camera_vertical_offset < 1500) {
            cur_avatar_status.camera_vertical_offset += 500;
        } else {
            cur_avatar_status.camera_vertical_offset = 1500;
        }
        player_move_reaction_with_collision();
        if (++cur_avatar_status.death_transition_frame >= 32) {
            if (cur_avatar_status.death_transition_frame < 65) {
                c = fixed_lerp_q12(0, 0xff, (cur_avatar_status.death_transition_frame - 32) << 7);
                render_set_color_overlay(0x82, c, c, c);
            } else {
                o4c = effect_status.objects;
                for (i = 128; i != 0; i--) {
                    o4c->type = 0xff;
                    o4c++;
                }
                part = 0xff;
                if (game_counter_is_nonzero(0x6b)) {
                    actor_disable_type3_transition_actors();
                    game_counter_decrement(0x6b);
                    cur_avatar_status.camera_rotation_target.vy = 0x2d5;
                    cur_avatar_status.pos.vy = -14976;
                    cur_avatar_status.pos.vz = 0xc000;
                    cur_avatar_status.camera_rotation_target.vx = 0;
                    cur_avatar_status.camera_rotation_target.vz = 0;
                    cur_avatar_status.pos.vx = 0x1c800;
                    cur_avatar_status.map_layer_offset = 5;
                    map_reveal_visited_rects();
                    repeat_store_word((u32 *)map_visited_cells, 0, 200);
                    event_world_state_save_slot(asset_status.active_resource_ids[0]);
                    player_reset_status();
                    player_recalculate_combat_stats();
                } else {
                    player_initialize_state();
                    player_recalculate_combat_stats();
                    movie_state_reset();
                    event_state_initialize();
                    cur_evt_state.unkd = 1;
                    conversation_records_initialize();
                    repeat_store_word((u32 *)map_visited_cells, 0, 200);
                }
                player_reload_map_resources(0, 0, 0, 0, 0, part);
            }
        }
        break;
    }
    cur_avatar_status.pad_buttons_previous = cur_avatar_status.pad_buttons;
    if (cur_avatar_status.poison_timer != 0) {
        if (cur_avatar_status.poison_timer % 30 == 0) {
            cur_avatar_status.damage_red_overlay_scale = 2400;
            cur_avatar_status.damage_red_overlay_decay = 60;
            player_adjust_hp_unclamped(-1);
        }
        cur_avatar_status.poison_timer--;
    }
    if (cur_avatar_status.fire_defense_boost_timer != 0) {
        if (--cur_avatar_status.fire_defense_boost_timer == 0) {
            player_recalculate_combat_stats();
        }
    }
    if (cur_avatar_status.damage_immunity_timer != 0) {
        if (--cur_avatar_status.damage_immunity_timer == 0) {
            player_recalculate_combat_stats();
        }
    }
    if (cur_avatar_status.physical_defense_boost_timer != 0) {
        if (--cur_avatar_status.physical_defense_boost_timer == 0) {
            player_recalculate_combat_stats();
        }
    }
    if (cur_avatar_status.full_mp_timer != 0) {
        cur_avatar_status.mp = cur_avatar_status.mp_max;
        if (--cur_avatar_status.full_mp_timer == 0) {
            notify_enqueue(0x1b);
        }
    }
    if (cur_avatar_status.magic_boost_timer != 0) {
        if (--cur_avatar_status.magic_boost_timer == 0) {
            player_recalculate_combat_stats();
            notify_enqueue(0x1b);
        }
    }
    cur_avatar_status.camera_rotation.vx = cur_avatar_status.camera_rotation_target.vx + cur_avatar_status.reaction_rotation_offset_x + cur_avatar_status.view_rotation_offset_x + cur_avatar_status.vertical_motion_pitch_offset;
    cur_avatar_status.camera_rotation.vy = cur_avatar_status.camera_rotation_target.vy + cur_avatar_status.reaction_rotation_offset_y + cur_avatar_status.view_rotation_offset_y + cur_avatar_status.camera_yaw_offset;
    cur_avatar_status.camera_rotation.vz = cur_avatar_status.camera_rotation_target.vz + cur_avatar_status.reaction_rotation_offset_z + cur_avatar_status.view_rotation_offset_z + cur_avatar_status.camera_roll_offset;
    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, 1);
    actor_status.actor_overlap_exclusion_flags = 0;
    hit = actor_find_overlap_excluding_target_type3(cur_avatar_status.pos.vx, cur_avatar_status.pos.vy,
                                                    cur_avatar_status.pos.vz, 1, 0x6a4);
    if (hit != -1 && (actor_status.objects[hit].flags & 8)) {
        player_begin_actor_overlap_bob();
    }
    player_update_weapon_attack();
    if (cur_avatar_status.equipped_weapon_id != 0xff) {
        if (cur_avatar_status.equipped_weapon_record->hp_regen_interval != 0 && cur_avatar_status.equipment_effect_ticks % cur_avatar_status.equipped_weapon_record->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (cur_avatar_status.equipped_weapon_record->mp_regen_interval != 0 && cur_avatar_status.equipment_effect_ticks % cur_avatar_status.equipped_weapon_record->mp_regen_interval == 0) {
            player_adjust_mp(1);
        }
        if (cur_avatar_status.equipped_shield_id == 0x35 && scene_display_sys.unk15996 != 0
            && cur_avatar_status.equipment_effect_ticks % 900 == 0) {
            player_adjust_mp(1);
        }
    }
    if (cur_avatar_status.equipped_head_id != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.equipped_head_record);
        player_apply_equipment_mp_tick(cur_avatar_status.equipped_head_record);
    }
    if (cur_avatar_status.equipped_body_id != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.equipped_body_record);
        player_apply_equipment_mp_tick(cur_avatar_status.equipped_body_record);
    }
    if (cur_avatar_status.equipped_shield_id != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.equipped_shield_record);
        player_apply_equipment_mp_tick(cur_avatar_status.equipped_shield_record);
    }
    if (cur_avatar_status.equipped_arm_id != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.equipped_arm_record);
        player_apply_equipment_mp_tick(cur_avatar_status.equipped_arm_record);
    }
    if (cur_avatar_status.equipped_leg_id != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.equipped_leg_record);
        player_apply_equipment_mp_tick(cur_avatar_status.equipped_leg_record);
    }
    if (cur_avatar_status.equipped_accessory_id != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.equipped_accessory_record);
        player_apply_equipment_mp_tick(cur_avatar_status.equipped_accessory_record);
    }
    if (cur_avatar_status.equipped_extra_id != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.equipped_extra_record);
        player_apply_equipment_mp_tick(cur_avatar_status.equipped_extra_record);
    }
    if (cur_avatar_status.equipped_weapon_id == 0x16 && cur_avatar_status.equipment_effect_ticks % 150 == 0
        && (rand() >> 12) == 0) {
        cur_avatar_status.paralysis_timer = 200;
    }
    if (cur_avatar_status.equipped_shield_id == 0x3a) {
        light_interpolate_color_and_fog(10, 10, 10, 0xd05, 6000, 0x800);
    }
    if ((cur_avatar_status.equipped_accessory_id == 0x52 || cur_avatar_status.equipped_extra_id == 0x52)
        && (rand() >> 7) == 0) {
        cur_avatar_status.poison_timer = 900;
    }
    cur_avatar_status.equipment_effect_ticks++;
}

/*
 * Selects the map cell under x/z (2048 units a cell) and the layer of it
 * (0 or 5 bytes in) whose height suits y; returns that floor height.
 */
ADDRESS(0x800324f0, 0x11c)
s32 collision_sample_map_cell_layer(s32 x, s32 y, s32 z)
{
    MapCell *cell;
    u16 h;

    if (x >= 0 && x <= 0x27fff && z >= 0 && z <= 0x27fff) {
        cell = &map_world.cells[z >> 11][x >> 11];
        h = -(y >> 7);
        if (cell->layer[0].elevation > cell->layer[1].elevation) {
            if (h >= cell->layer[0].elevation || cell->layer[1].elevation == 0) {
            lower:
                map_world.collision_layer_offset = 0;
                map_world.collision_layer_height = -cell->layer[0].elevation << 7;
            } else {
            upper:
                map_world.collision_layer_offset = 5;
                map_world.collision_layer_height = -cell->layer[1].elevation << 7;
            }
        } else {
            if (h < cell->layer[1].elevation && cell->layer[0].elevation != 0) {
                goto lower;
            }
            goto upper;
        }
    } else {
        cell = &collision_default_cell;
        map_world.collision_layer_offset = 0;
        map_world.collision_layer_height = 0;
    }
    map_world.collision_cell = cell;
    return map_world.collision_layer_height;
}
