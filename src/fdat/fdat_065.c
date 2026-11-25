/*
 * FDAT_065.BIN (WIP module): the handler table, then its code.
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
void area_use_item_noop(void);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_noop(void);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_noop(void);
void area_on_load_store_area_index(void);

const FdatHeader fdat_header = {
    0x1a,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
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
    0x3a,
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

ADDRESS(0x801e85a8, 0x8)
void area_use_item_noop(void)
{
}

ADDRESS(0x801e85b0, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e85b8, 0x8)
void area_on_actor_removed_noop(void)
{
}

ADDRESS(0x801e85c0, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e85c8, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e85d0, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e85d8, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e85e0, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e85e8, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e85f0, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e85f8, 0x14)
void area_on_load_store_area_index(void)
{
    DAT_801ba984.unk84[5] = 0x15;
}
