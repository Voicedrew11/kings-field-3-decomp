/*
 * GAME.EXE: the game state a memory card save holds (WIP module):
 * card_payload_capture_game_state copies it into the save data (SaveData, game/card.h) and
 * card_payload_restore_game_state copies it back. Every global is reached through lui/low
 * pairs, the menu's .sdata cursors included, so the unit is -G0.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/card.h>
#include <game/data.h>
#include <game/render.h>
#include <game/menu.h>
#include <game/funcs.h>

/* Copies the game state into the save data at data. */
ADDRESS(0x8005f7bc, 0x814)
void card_payload_capture_game_state(u8 *data)
{
    SaveData *save = (SaveData *)data;
    MagicRecord *r;
    u8 *p;
    s32 i;

    /* Through the byte pointer: retail moves the area list unaligned (lwl/lwr). */
    memcpy((void *)data, (const void *)asset_status.active_resource_ids,
           sizeof(asset_status.active_resource_ids));
    save->pos = cur_avatar_status.pos;
    save->camera_rotation_target = cur_avatar_status.camera_rotation_target;
    save->play_minutes = cur_avatar_status.play_minutes;
    save->experience = cur_avatar_status.experience;
    save->next_level_experience = cur_avatar_status.next_level_experience;
    save->gold = cur_avatar_status.gold;
    save->weapon_growth_experience = cur_avatar_status.weapon_growth_experience;
    save->map_layer_offset = cur_avatar_status.map_layer_offset;
    save->hp_max = cur_avatar_status.hp_max;
    save->hp = cur_avatar_status.hp;
    save->mp_max = cur_avatar_status.mp_max;
    save->mp = cur_avatar_status.mp;
    save->base_physical_power = cur_avatar_status.base_physical_power;
    for (i = 4; i != -1; i--) {
        save->base_magic[i] = cur_avatar_status.base_magic[i];
        save->magic_training[i] = cur_avatar_status.magic_training[i];
    }
    save->unk56d6 = cur_avatar_status.unk3e;
    save->wisdom = cur_avatar_status.wisdom;
    save->physical_power_training = cur_avatar_status.physical_power_training;
    save->unk56e6 = cur_avatar_status.unk30;
    save->status_timers[0] = cur_avatar_status.poison_timer;
    save->status_timers[1] = cur_avatar_status.curse_strength;
    save->status_timers[2] = cur_avatar_status.curse_phase_limit;
    save->status_timers[3] = cur_avatar_status.darkness_phase;
    save->status_timers[4] = cur_avatar_status.darkness_phase_limit;
    save->status_timers[5] = cur_avatar_status.slow_timer;
    save->status_timers[6] = cur_avatar_status.paralysis_timer;
    save->status_timers[7] = cur_avatar_status.fire_defense_boost_timer;
    save->status_timers[8] = cur_avatar_status.damage_immunity_timer;
    save->status_timers[9] = cur_avatar_status.physical_defense_boost_timer;
    save->status_timers[10] = cur_avatar_status.magic_tint_phase;
    save->status_timers[11] = cur_avatar_status.magic_tint_phase_limit;
    save->status_timers[12] = cur_avatar_status.map_marker_visual_effect_timer;
    save->status_timers[13] = cur_avatar_status.full_mp_timer;
    save->status_timers[14] = cur_avatar_status.magic_boost_timer;
    save->level = cur_avatar_status.level;
    save->unk5707 = cur_avatar_status.unkd;
    save->equipped_ids[0] = cur_avatar_status.equipped_head_id;
    save->equipped_ids[1] = cur_avatar_status.equipped_body_id;
    save->equipped_ids[2] = cur_avatar_status.equipped_shield_id;
    save->equipped_ids[3] = cur_avatar_status.equipped_arm_id;
    save->equipped_ids[4] = cur_avatar_status.equipped_leg_id;
    save->equipped_ids[5] = cur_avatar_status.equipped_accessory_id;
    save->equipped_ids[6] = cur_avatar_status.equipped_extra_id;
    save->primary_magic_shortcut_id = cur_avatar_status.primary_magic_shortcut_id;
    save->secondary_magic_shortcut_id = cur_avatar_status.secondary_magic_shortcut_id;
    save->secondary_item_shortcut_id = cur_avatar_status.secondary_item_shortcut_id;
    save->equipped_weapon_id = cur_avatar_status.equipped_weapon_id;
    save->se_volume = cur_avatar_status.se_volume;
    save->bgm_volume = cur_avatar_status.bgm_volume;
    save->options[0] = cur_avatar_status.vitals_display;
    save->options[1] = cur_avatar_status.compass_display;
    save->options[2] = cur_avatar_status.item_preview_enabled;
    save->options[3] = cur_avatar_status.walking_bob_enabled;
    save->options[4] = cur_avatar_status.brightness;
    save->options[5] = cur_avatar_status.button_style;
    save->options[6] = cur_avatar_status.button_layout;
    save->options[7] = cur_avatar_status.panel_translucent;
    p = save->magic_menu_available;
    r = effect_status.magic_records;
    for (i = 95; i != -1; i--) {
        *p++ = r->menu_available;
        r++;
    }
    memcpy((void *)save->game_counters, (const void *)event_item_counters_table, sizeof(save->game_counters));
    memcpy((void *)save->event_flags, (const void *)cur_evt_state.flags, sizeof(save->event_flags));
    memcpy((void *)save->event_arena, (const void *)cur_evt_state.arena, sizeof(save->event_arena));
    memcpy((void *)save->saved_event_offsets, (const void *)cur_evt_state.saved_offsets,
           sizeof(save->saved_event_offsets));
    memcpy((void *)save->conversation_page_states, (const void *)conversation_records.page_states,
           sizeof(save->conversation_page_states));
    memcpy((void *)save->revealed_map_rects, (const void *)map_revealed_rects,
           sizeof(save->revealed_map_rects));
    memcpy((void *)save->item_guide_levels, (const void *)item_guide_level, sizeof(save->item_guide_levels));
    memcpy((void *)save->shop_stock, (const void *)menu_shop_stock_pages[0], sizeof(save->shop_stock));
    save->menu_list_cursors[0] = menu_item_list_cursor;
    save->menu_list_cursors[1] = menu_magic_list_cursor;
    save->menu_list_cursors[2] = menu_store_list_cursor;
    save->menu_list_cursors[3] = menu_take_out_list_cursor;
}

/* Copies the game state back from the save data at data; the area list
 * comes back with parts 1 and 2 set to part 0. */
ADDRESS(0x8005ffd0, 0x830)
void card_payload_restore_game_state(u8 *data)
{
    SaveData *save = (SaveData *)data;
    MagicRecord *r;
    u8 *p;
    s32 i;

    /* Through the byte pointer: retail moves the area list unaligned (lwl/lwr). */
    memcpy((void *)asset_status.active_resource_ids, (const void *)data,
           sizeof(asset_status.active_resource_ids));
    asset_status.active_resource_ids[2] = asset_status.active_resource_ids[1] = asset_status.active_resource_ids[0];
    cur_avatar_status.pos = save->pos;
    cur_avatar_status.camera_rotation_target = save->camera_rotation_target;
    cur_avatar_status.play_minutes = save->play_minutes;
    cur_avatar_status.experience = save->experience;
    cur_avatar_status.next_level_experience = save->next_level_experience;
    cur_avatar_status.gold = save->gold;
    cur_avatar_status.weapon_growth_experience = save->weapon_growth_experience;
    cur_avatar_status.map_layer_offset = save->map_layer_offset;
    cur_avatar_status.hp_max = save->hp_max;
    cur_avatar_status.hp = save->hp;
    cur_avatar_status.mp_max = save->mp_max;
    cur_avatar_status.mp = save->mp;
    cur_avatar_status.base_physical_power = save->base_physical_power;
    for (i = 4; i != -1; i--) {
        cur_avatar_status.base_magic[i] = save->base_magic[i];
        cur_avatar_status.magic_training[i] = save->magic_training[i];
    }
    cur_avatar_status.unk3e = save->unk56d6;
    cur_avatar_status.wisdom = save->wisdom;
    cur_avatar_status.physical_power_training = save->physical_power_training;
    cur_avatar_status.unk30 = save->unk56e6;
    cur_avatar_status.poison_timer = save->status_timers[0];
    cur_avatar_status.curse_strength = save->status_timers[1];
    cur_avatar_status.curse_phase_limit = save->status_timers[2];
    cur_avatar_status.darkness_phase = save->status_timers[3];
    cur_avatar_status.darkness_phase_limit = save->status_timers[4];
    cur_avatar_status.slow_timer = save->status_timers[5];
    cur_avatar_status.paralysis_timer = save->status_timers[6];
    cur_avatar_status.fire_defense_boost_timer = save->status_timers[7];
    cur_avatar_status.damage_immunity_timer = save->status_timers[8];
    cur_avatar_status.physical_defense_boost_timer = save->status_timers[9];
    cur_avatar_status.magic_tint_phase = save->status_timers[10];
    cur_avatar_status.magic_tint_phase_limit = save->status_timers[11];
    cur_avatar_status.map_marker_visual_effect_timer = save->status_timers[12];
    cur_avatar_status.full_mp_timer = save->status_timers[13];
    cur_avatar_status.magic_boost_timer = save->status_timers[14];
    cur_avatar_status.level = save->level;
    cur_avatar_status.unkd = save->unk5707;
    cur_avatar_status.equipped_head_id = save->equipped_ids[0];
    cur_avatar_status.equipped_body_id = save->equipped_ids[1];
    cur_avatar_status.equipped_shield_id = save->equipped_ids[2];
    cur_avatar_status.equipped_arm_id = save->equipped_ids[3];
    cur_avatar_status.equipped_leg_id = save->equipped_ids[4];
    cur_avatar_status.equipped_accessory_id = save->equipped_ids[5];
    cur_avatar_status.equipped_extra_id = save->equipped_ids[6];
    cur_avatar_status.primary_magic_shortcut_id = save->primary_magic_shortcut_id;
    cur_avatar_status.secondary_magic_shortcut_id = save->secondary_magic_shortcut_id;
    cur_avatar_status.secondary_item_shortcut_id = save->secondary_item_shortcut_id;
    cur_avatar_status.equipped_weapon_id = save->equipped_weapon_id;
    cur_avatar_status.se_volume = save->se_volume;
    cur_avatar_status.bgm_volume = save->bgm_volume;
    cur_avatar_status.vitals_display = save->options[0];
    cur_avatar_status.compass_display = save->options[1];
    cur_avatar_status.item_preview_enabled = save->options[2];
    cur_avatar_status.walking_bob_enabled = save->options[3];
    cur_avatar_status.brightness = save->options[4];
    cur_avatar_status.button_style = save->options[5];
    cur_avatar_status.button_layout = save->options[6];
    cur_avatar_status.panel_translucent = save->options[7];
    p = save->magic_menu_available;
    r = effect_status.magic_records;
    for (i = 95; i != -1; i--) {
        r->menu_available = *p++;
        r++;
    }
    memcpy((void *)event_item_counters_table, (const void *)save->game_counters, sizeof(save->game_counters));
    memcpy((void *)cur_evt_state.flags, (const void *)save->event_flags, sizeof(save->event_flags));
    memcpy((void *)cur_evt_state.arena, (const void *)save->event_arena, sizeof(save->event_arena));
    memcpy((void *)cur_evt_state.saved_offsets, (const void *)save->saved_event_offsets,
           sizeof(save->saved_event_offsets));
    memcpy((void *)conversation_records.page_states, (const void *)save->conversation_page_states,
           sizeof(save->conversation_page_states));
    memcpy((void *)map_revealed_rects, (const void *)save->revealed_map_rects,
           sizeof(save->revealed_map_rects));
    memcpy((void *)item_guide_level, (const void *)save->item_guide_levels, sizeof(save->item_guide_levels));
    memcpy((void *)menu_shop_stock_pages[0], (const void *)save->shop_stock, sizeof(save->shop_stock));
    menu_item_list_cursor = save->menu_list_cursors[0];
    menu_magic_list_cursor = save->menu_list_cursors[1];
    menu_store_list_cursor = save->menu_list_cursors[2];
    menu_take_out_list_cursor = save->menu_list_cursors[3];
}
