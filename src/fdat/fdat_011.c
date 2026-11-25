/*
 * FDAT_011.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers, an object handler (slot 3), an event handler (slot 4) and
 * the load handler (slot 5).
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
void func_801e8398(void);
void func_801e85a8(void);
void func_801e85b0(void);
void area_update_noop(void);
void area_use_item_noop(void);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_noop(void);
void area_region_trigger_update_warp_arrival_cells(Object44 *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_blacksmith(Object88 *obj, s32 event);
void area_on_load_reset_blacksmith(void);

const FdatHeader fdat_header = {
    0x8,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_update_warp_arrival_cells,
        /*  4 */ area_talk_event_blacksmith,
        /*  5 */ area_on_load_reset_blacksmith,
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
    0x28,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

ADDRESS(0x801e8398, 0x8)
void func_801e8398(void)
{
}

#include "shared/area_interact_item_containers.inc"

ADDRESS(0x801e85a8, 0x8)
void func_801e85a8(void)
{
}

ADDRESS(0x801e85b0, 0x8)
void func_801e85b0(void)
{
}

ADDRESS(0x801e85b8, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e85c0, 0x8)
void area_use_item_noop(void)
{
}

ADDRESS(0x801e85c8, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e85d0, 0x8)
void area_on_actor_removed_noop(void)
{
}

/*
 * Slot 3: region operand 10 (unk3a.bytes[1]): while the player is on the warp
 * arrival pad copies the arrival cell pattern, else clears the arrival flag
 * (player +0x10a) and copies the normal pattern back.
 */
ADDRESS(0x801e85d8, 0xc0)
void area_region_trigger_update_warp_arrival_cells(Object44 *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(6, 0x1e, 3, 4, 0x8000) != 0) {
        if (cur_avatar_status.unk10a != 0) {
            map_cell_copy_rotated_fields(2, 0x4a, 0, 6, 0x21, 3, 1, 0, 0x2f);
        }
    } else {
        cur_avatar_status.unk10a = 0;
        map_cell_copy_rotated_fields(2, 0x4d, 0, 6, 0x21, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e8698, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e86a0, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e86a8, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e86b0, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e86b8, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 4: script events 0..2 of an object record. */
ADDRESS(0x801e86c0, 0x100)
void area_talk_event_blacksmith(Object88 *obj, s32 event)
{
    s32 result;

    switch (event) {
    case 0:
        if (game_counter_is_nonzero(0x7c) > 0) {
            cur_evt_state.unk76[5] = 1;
        } else {
            cur_evt_state.unk76[5] = 0;
        }
        break;
    case 1:
        player_render_frame_and_release_pool();
        result = menu_blacksmith_make_item();
        cur_avatar_status.unk178 = ~action_button_masks[7];
        if (result < 0) {
            cur_evt_state.unk76[2] = 0;
            cur_evt_state.unk76[0] = 0x68;
        } else {
            cur_evt_state.unk76[3] = 0;
            cur_evt_state.unk76[2] = 1;
            cur_evt_state.unk76[0] = result;
        }
        break;
    case 2:
        event_map_object_interact(NULL, cur_evt_state.unk76[0]);
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e87c0, 0x20)
void area_on_load_reset_blacksmith(void)
{
    cur_evt_state.unk76[3] = 1;
    cur_evt_state.unk84[5] = 3;
}
