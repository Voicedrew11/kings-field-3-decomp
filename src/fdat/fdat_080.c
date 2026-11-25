/*
 * FDAT_080.BIN (WIP module): the shared light-record blend, the handler
 * table, the shared slot 0 handler, empty handlers, a flag check (slot 1),
 * the lamp helper and records 40..51 (slot 18), a switch handler (slot 3),
 * an event handler (slot 7), an event handler (slot 4) and the load handler
 * (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x88)

void area_handler_noop(void);
void area_update_flag_on_element_stat(void);
void area_use_item_noop(void);
void area_refresh_lamp_effects_and_light(void);
void area_adjust_actor_magic_damage_light_lamps(Actor *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9);
void area_on_actor_removed_noop(void);
void area_region_trigger_set_flags_and_recalc_stats(Object44 *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_unlock_element(u8 id);
void area_on_movie_end_raise_element_and_fade_objects(u8 id);
void area_talk_event_signal_marker(Actor *record, s32 event);
void area_on_load_restore_lamps_and_markers(void);

const FdatHeader fdat_header = {
    0x1f,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_flag_on_element_stat,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_set_flags_and_recalc_stats,
        /*  4 */ area_talk_event_signal_marker,
        /*  5 */ area_on_load_restore_lamps_and_markers,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_raise_element_and_fade_objects,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_unlock_element,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_light_lamps,
        /* 19 */ area_on_actor_removed_noop,
        /* 20 */ area_handler_noop,
        /* 21 */ area_handler_noop,
        /* 22 */ area_handler_noop,
        /* 23 */ area_handler_noop,
        /* 24 */ area_handler_noop,
        /* 25 */ area_handler_noop,
        /* 26 */ area_handler_noop,
        /* 27 */ area_handler_noop,
        /* 28 */ area_handler_noop,
        /* 29 */ area_handler_noop,
        /* 30 */ area_handler_noop,
        /* 31 */ area_handler_noop,
    },
    0x3f,
};

#include "shared/light_blend_records.inc"

ADDRESS(0x801e8474, 0x8)
void area_handler_noop(void)
{
}

#include "shared/area_interact_item_containers.inc"

/* Slot 1: sets flag 0x31 once the player's unk34[3] is non-zero. */
ADDRESS(0x801e8684, 0x38)
void area_update_flag_on_element_stat(void)
{
    if (cur_avatar_status.unk34[3] != 0) {
        if (cur_evt_state.unk32[0x31] == 0) {
            cur_evt_state.unk32[0x31] = 1;
        }
    }
}

ADDRESS(0x801e86bc, 0x8)
void area_use_item_noop(void)
{
}

DATA(0x801e8bb8, 0x30, ".data")
static Record30 area_light_blend_from = {
    {{{3000, 800, -1500}, {-2000, 1500, -3400}, {-1300, -2000, 1024}}, 0},
    {{{128, 128, 128}, {128, 128, 128}, {128, 128, 128}}, 0},
    {0x50, 0x50, 0x50, 0},
    6000,
    12000,
};
DATA(0x801e8be8, 0x30, ".data")
static Record30 area_light_blend_to = {
    {{{3000, 800, -1500}, {-2000, 1500, -3400}, {-1300, -2000, 1024}}, 0},
    {{{2662, 2662, 2662}, {2662, 2662, 2662}, {2662, 2662, 2662}}, 0},
    {0x78, 0x78, 0x78, 0},
    6000,
    18000,
};

/* Lights the effects of the twelve lamp bits and blends the area light by their count. */
ADDRESS(0x801e86c4, 0x94)
void area_refresh_lamp_effects_and_light(void)
{
    u32 bits;
    Effect *effect;
    s32 i;
    s32 t;

    bits = cur_evt_state.unk32[0x34] | cur_evt_state.unk32[0x35] << 8;
    effect = map_placed_entries;
    for (i = 0, t = 0; i < 12; effect++, i++, bits >>= 1) {
        if (bits & 1) {
            effect->unk2 = 2;
            t += 320;
        } else {
            effect->unk2 = 0;
        }
    }
    light_blend_records(&area_light_blend_from, &area_light_blend_to, &light_default_records[41], t);
}

/* Slot 18: records 40..51 each light their lamp bit. */
ADDRESS(0x801e8758, 0xf8)
void area_adjust_actor_magic_damage_light_lamps(Actor *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9)
{
    s32 index = obj - actor_status.objects - 40;
    s32 bit;

    if (index >= 0 && index < 12) {
        cur_evt_state.unk32[0x34] |= bit = 1 << index;
        cur_evt_state.unk32[0x35] |= bit >> 8;
        *out = 0;
        if (arg6 == 0) {
            return;
        }
        area_refresh_lamp_effects_and_light();
        light_reset_records();
        light_refresh_record_rotations();
        player_update_status_visuals();
    }
    if ((*(u32 *)&cur_evt_state.unk32[0x32] & 0xffff0000) == 0x0fff0000) {
        if (cur_evt_state.unk32[0x36] == 0) {
            cur_evt_state.unk32[0x36] = 1;
            audio_play_sound_16();
        }
        map_object_process_marker_all(0xf1);
    }
}

ADDRESS(0x801e8850, 0x8)
void area_on_actor_removed_noop(void)
{
}

/* Slot 3: switch objects 0..2. */
ADDRESS(0x801e8858, 0xb8)
void area_region_trigger_set_flags_and_recalc_stats(Object44 *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (cur_evt_state.unk32[0x30] != 1) {
            cur_evt_state.unk32[0x30] = 1;
        }
        break;
    case 1:
        if (cur_evt_state.unk32[0x33] != 1) {
            cur_evt_state.unk32[0x33] = 1;
            player_recalculate_combat_stats();
        }
        break;
    case 2:
        if (cur_evt_state.unk32[0x33] != 0) {
            cur_evt_state.unk32[0x33] = 0;
            player_recalculate_combat_stats();
        }
        break;
    }
}

ADDRESS(0x801e8910, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8918, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8920, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8928, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8930, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 7. */
ADDRESS(0x801e8938, 0x20)
void area_on_movie_start_unlock_element(u8 id)
{
    if (id == 14) {
        cur_evt_state.unk71[3] = 1;
    }
}

/*
 * Slot 7: raises a stat; objects 6 and 87 each fade out and are removed
 * (death-fade preset 0x32, light blend up over 16 frames).
 */
ADDRESS(0x801e8958, 0x198)
void area_on_movie_end_raise_element_and_fade_objects(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    Object44 *obj;
    s32 i;

    cur_avatar_status.unk34[3] += 20;
    player_learn_element_magic(3);
    player_recalculate_combat_stats();
    obj = &map_objects_slots.objects[6];
    map_objects_slots.objects[6].unk5 = 0x32;
    map_objects_slots.objects[6].unk2 = 5;
    for (i = 15; i != -1; i--) {
        obj->unk10 += 256;
        if (obj->unk10 >= 4095) {
            obj->unk10 = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
    }
    obj->unk6 = 0xff;
    obj = &map_objects_slots.objects[87];
    map_objects_slots.objects[87].unk5 = 0x32;
    map_objects_slots.objects[87].unk2 = 5;
    for (i = 15; i != -1; i--) {
        obj->unk10 += 256;
        if (obj->unk10 >= 4095) {
            obj->unk10 = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
    }
    obj->unk6 = 0xff;
    conversation_record_mark_page(9, 0);
    conversation_record_mark_page(9, 1);
}

/* Slot 4: script event 0 of an object record sets flag 0x32 once. */
ADDRESS(0x801e8af0, 0x48)
void area_talk_event_signal_marker(Actor *record, s32 event)
{
    if (event == 0) {
        if (cur_evt_state.unk32[0x32] == 0) {
            cur_evt_state.unk32[0x32] = 1;
            map_object_apply_marker_signal(0xf2);
            audio_play_sound_16();
        }
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8b38, 0x80)
void area_on_load_restore_lamps_and_markers(void)
{
    if (cur_evt_state.unk32[0x32] != 0) {
        map_object_apply_marker_signal(0xf2);
    }
    if (cur_evt_state.unk7f[3] != 0) {
        map_objects_slots.objects[6].unk6 = 0xff;
        map_objects_slots.objects[87].unk6 = 0xff;
    }
    if (cur_evt_state.unk32[0x36] != 0) {
        map_object_process_marker_all(0xf1);
    }
    area_refresh_lamp_effects_and_light();
    cur_evt_state.unk84[5] = 0x1a;
}
