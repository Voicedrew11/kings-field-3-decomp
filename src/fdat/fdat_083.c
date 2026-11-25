/*
 * FDAT_083.BIN (WIP module): the handler table, the shared slot 0, 2, 16 and
 * 17 handlers, empty handlers, a record handler (slot 19), a flag setter and
 * the load handler (slot 5).
 */
#define FADE_APPROACH_BLEND_STEP 64
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
void area_on_actor_removed_trade_item(Actor *obj);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_noop(void);
void area_on_movie_end_set_flag(void);
void area_talk_event_noop(void);
void area_on_load_store_area_index(void);

const FdatHeader fdat_header = {
    0x20,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_notify_no_target,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_store_area_index,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_set_flag,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_noop,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_approach_range,
        /* 17 */ area_update_actor_target_fade_approach,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_trade_item,
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
    0x40,
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

/* Slot 19: record 0 gives item 0x11 for item 0x10 once. */
ADDRESS(0x801e85e4, 0xb0)
void area_on_actor_removed_trade_item(Actor *obj)
{
    if (obj - actor_status.objects != 0) {
        return;
    }
    if (cur_avatar_status.unk101 == 0x11) {
        return;
    }
    cur_evt_state.unk32[0x37] = 1;
    if (game_counter_is_nonzero(0x10) <= 0) {
        return;
    }
    cur_evt_state.unk32[0x38] = 1;
    game_counter_decrement(0x10);
    game_counter_increment(0x11);
    if (cur_avatar_status.unkcb == 0x10) {
        player_equip_weapon(0x11);
    }
}

ADDRESS(0x801e8694, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e869c, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e86a4, 0x8)
void area_update_map_object_noop(void)
{
}

#include "shared/area_actor_target_check_approach_range.inc"

#include "shared/area_update_actor_target_fade_approach.inc"

ADDRESS(0x801e8a40, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e8a48, 0x8)
void area_on_movie_start_noop(void)
{
}

/* Sets flag 0x37. */
ADDRESS(0x801e8a50, 0x14)
void area_on_movie_end_set_flag(void)
{
    cur_evt_state.unk32[0x37] = 1;
}

ADDRESS(0x801e8a64, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8a6c, 0x14)
void area_on_load_store_area_index(void)
{
    cur_evt_state.unk84[5] = 0x1b;
}
