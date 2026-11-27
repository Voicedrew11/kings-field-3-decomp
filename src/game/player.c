/*
 * GAME.EXE: player state (WIP module): fall and death heights, the
 * per-area reset, equipment and the derived stats, experience and levels,
 * damage, and the camera the renderer takes from the player record
 * cur_avatar_status.
 */
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
#include <game/object4c.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/funcs.h>
#include <game/player.h>
#include <game/entity.h>
#include <game/scene.h>
#include <kf/lib/null.h>

RODATA(0x80011760, 0x1bc)

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
    s16 *threshold;
    u16 *magic;
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
    n = 0;
    threshold = magic_element_learn_levels[element];
    magic = &cur_avatar_status.base_magic[element];
    goto check;

learn:
    r->menu_available = 1;
    notify_enqueue(3);
    cur_evt_state.interaction_handled = 1;
    cur_evt_state.element_magic_learned[element] = 1;
    goto advance;

next:
    if (*threshold == 0) {
        return;
    }
    spell = magic_element_learn_ids[element];
    for (k = n; k != -1; k--, spell++) {
        if (*spell == 0xff) {
            return;
        }
        r = &effect_status.magic_records[*spell];
        if (r->menu_available == 0) {
            goto learn;
        }
    }
advance:
    threshold++;
    n++;
check:
    if (*magic >= *threshold) {
        goto next;
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

/*
 * Unresolved (player_set_primary_magic_shortcut_id,
 * player_set_secondary_magic_shortcut_id): retail reads r->unk2 again after
 * each 0xff test, so both bodies compile 4 bytes short.
 */
ADDRESS(0x8002b928, 0x110)
void player_set_primary_magic_shortcut_id(u8 id)
{
    SoundBank *bank = NULL;
    MagicRecord *r;
    s16 n;
    s32 se;
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
    s32 se;
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
    u8 effect[34] = {
        0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x04, 0xff, 0xff,
        0x07, 0x02, 0x05, 0x01, 0x06, 0x06, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
    };

    cur_avatar_status.weapon_charge_delay = 10;
    cur_avatar_status.attack_charge_current = 0;
    cur_avatar_status.attack_charge_committed = 0;
    cur_avatar_status.equipped_weapon_id = id;
    if (id != 0xff) {
        cur_avatar_status.equipped_weapon_record = &weapon_table[id];
        cd_archive_read(4, id + 98, cur_avatar_status.weapon_asset_buffer);
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
