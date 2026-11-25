/*
 * FDAT_062.BIN (WIP module): the handler table, the shared slot 0 handler,
 * an item handler (slot 2), empty handlers, the load handler (slot 5) and
 * two door handlers (slots 7 and 10).
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
void area_update_noop(void);
void area_use_item_with_item_held(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_noop(void);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_copy_cells_and_trade_item(u8 id);
void area_on_movie_end_fade_object(u8 id);
void area_talk_event_noop(void);
void area_on_load_restore_cells_and_flags(void);

const FdatHeader fdat_header = {
    0x19,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_with_item_held,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_restore_cells_and_flags,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_fade_object,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_copy_cells_and_trade_item,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
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
    0x39,
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

/* Slot 2: item 0x7e opens the lock when item 0x7d is held. */
ADDRESS(0x801e85a8, 0xb0)
void area_use_item_with_item_held(VECTOR *pos, SVECTOR *angle, s32 item)
{
    if (item != 0x7e) {
        return;
    }
    if (player_camera_within_map_region(0x26, 0x10, 3, 2, 0x8000) == 0) {
        return;
    }
    if (game_counter_is_nonzero(0x7d) > 0) {
        if (cur_evt_state.unk32[0x28] != 0) {
            return;
        }
        cur_evt_state.unk32[0x28] = 1;
        cur_evt_state.unk32[0x29] = 0;
        game_counter_decrement(0x7e);
    } else {
        if (cur_evt_state.unk32[0x28] != 0) {
            return;
        }
        notify_enqueue(0x10);
    }
    cur_evt_state.unk0 = 1;
}

ADDRESS(0x801e8658, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e8660, 0x8)
void area_on_actor_removed_noop(void)
{
}

ADDRESS(0x801e8668, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e8670, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8678, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8680, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8688, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8690, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 10. */
ADDRESS(0x801e8698, 0x98)
void area_on_movie_start_copy_cells_and_trade_item(u8 id)
{
    if (id == 5) {
        map_cell_copy_rotated_fields(2, 0x4d, 0, 0x26, 12, 3, id, 0, 0x2f);
    }
    if (id == 10) {
        if (game_counter_is_nonzero(0x7d) > 0) {
            game_counter_decrement(0x7d);
            game_counter_increment(0x10);
            cur_evt_state.unk32[0x2b] = 1;
        }
    }
}

/*
 * Slot 7: movie 10: object 2 fades out and is removed (death-fade preset
 * 0x32, light blend up over 16 frames).
 */
ADDRESS(0x801e8730, 0xf8)
void area_on_movie_end_fade_object(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    MapObject *obj;
    s32 i;

    if (id == 5) {
        cur_evt_state.unk32[0x29] = 1;
    }
    if (id == 10) {
        obj = &map_objects_slots.objects[2];
        cur_evt_state.unk32[0x2a] = 1;
        map_objects_slots.objects[2].lighting_override_index = 0x32;
        map_objects_slots.objects[2].render_mode = 5;
        for (i = 15; i != -1; i--) {
            obj->lighting_blend += 256;
            if (obj->lighting_blend >= 4095) {
                obj->lighting_blend = 0xfff;
            }
            cd_request_wait_idle();
            cd_request_service_vab();
            cd_request_service_stream();
            player_get_camera_pose(&pos, &angle);
            render_game_frame(&pos, &angle);
        }
        obj->object_id = 0xff;
    }
}

ADDRESS(0x801e8828, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8830, 0xa8)
void area_on_load_restore_cells_and_flags(void)
{
    cur_evt_state.unk32[0x29] = 1;
    if (cur_evt_state.unk32[0x28] == 0) {
        map_cell_copy_rotated_fields(2, 0x4a, 0, 0x26, 12, 3, 5, 0, 0x2f);
    } else {
        map_cell_copy_rotated_fields(2, 0x4d, 0, 0x26, 12, 3, 5, 0, 0x2f);
    }
    if (game_counter_is_nonzero(0x7d) > 0) {
        cur_evt_state.unk32[0x2a] = 0;
    } else {
        cur_evt_state.unk32[0x2a] = 1;
    }
    cur_evt_state.unk84[5] = 0x14;
}
