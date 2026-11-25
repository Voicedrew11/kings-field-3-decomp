/*
 * FDAT_077.BIN (WIP module): the handler table, the shared slot 0, 2, 16 and
 * 17 handlers, empty handlers, a record handler (slot 19) and the load
 * handler (slot 5).
 */
#define FADE_APPROACH_BLEND_STEP 1024
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x88)

void area_handler_noop(void);
void area_update_noop(void);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_start_ending(Object88 *obj);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_noop(void);
void area_on_load_store_area_index(void);

const FdatHeader fdat_header = {
    0x1e,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_notify_no_target,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_store_area_index,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_handler_noop,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_handler_noop,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_approach_range,
        /* 17 */ area_update_actor_target_fade_approach,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_start_ending,
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
    0x3e,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

#include "shared/area_interact_item_containers.inc"

ADDRESS(0x801e85a0, 0x8)
void area_update_noop(void)
{
}

#include "shared/area_use_item_notify_no_target.inc"

ADDRESS(0x801e85dc, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

/* Slot 19: record 0 starts the ending once (flag 0x39): fades the sound out, stops the sequence. */
ADDRESS(0x801e85e4, 0x160)
void area_on_actor_removed_start_ending(Object88 *obj)
{
    s32 volume;
    s32 i;

    if (obj - actor_status.objects != 0) {
        return;
    }
    if (cur_evt_state.unk32[0x39] != 0) {
        return;
    }
    cur_evt_state.unk32[0x39] = 1;
    cur_avatar_status.unkc0 = -1;
    light_reset_records();
    render_reset_color_overlays();
    light_refresh_record_rotations();
    player_update_status_visuals();
    volume = 0x7f;
    for (i = 0x3f; i != -1; i--) {
        volume -= 2;
        if (volume < 0) {
            SsSetMVol(0, 0);
        } else {
            SsSetMVol(volume, volume);
        }
        DrawSync(0);
        VSync(0);
    }
    SsSetMVol(0, 0);
    if (sound_status.seq_playing == 1) {
        SsSeqSetVol(sound_status.seq_id, 0, 0);
        SsSeqStop(sound_status.seq_id);
        SsSeqClose(sound_status.seq_id);
        sound_status.seq_playing = 0;
    }
    main_end_flag = 3;
    render_frames_with_color_overlay(0x82, 0, 0x1000, 0x40);
}

ADDRESS(0x801e8744, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e874c, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8754, 0x8)
void area_update_map_object_noop(void)
{
}

#include "shared/area_actor_target_check_approach_range.inc"

#include "shared/area_update_actor_target_fade_approach.inc"

ADDRESS(0x801e8af0, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e8af8, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8b00, 0x14)
void area_on_load_store_area_index(void)
{
    cur_evt_state.unk84[5] = 0x19;
}
