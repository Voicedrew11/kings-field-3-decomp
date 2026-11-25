/*
 * FDAT_017.BIN (WIP module): the handler table, the shared slot 0 handler,
 * an item handler (slot 2), a note handler (slot 3), empty handlers, an event
 * handler (slot 4) and the load handler (slot 5).
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
void area_use_item_operate_switch(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_noop(void);
void area_region_trigger_update_warp_arrival_cells(Object44 *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_clear_objects_and_give_items(Object88 *record, s32 event);
void area_on_load_restore_cleared_objects(void);

const FdatHeader fdat_header = {
    0xa,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_operate_switch,
        /*  3 */ area_region_trigger_update_warp_arrival_cells,
        /*  4 */ area_talk_event_clear_objects_and_give_items,
        /*  5 */ area_on_load_restore_cleared_objects,
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
    0x2a,
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

/* Slot 2: item 0x66 on a kind 0x53 object in front starts it. */
ADDRESS(0x801e85a8, 0xd4)
void area_use_item_operate_switch(VECTOR *pos, SVECTOR *angle, s32 item)
{
    VECTOR unused;
    SVECTOR unused_angle;
    s32 i;
    Object44 *obj;

    if (item != 0x66) {
        return;
    }
    i = map_object_find_interaction_target(0, pos, 800, 0x6a4, angle->vy, 0x200);
    if (i == -1) {
        return;
    }
    obj = &DAT_8018fb3c.objects[i];
    if (obj->unk4 != 0x53) {
        return;
    }
    if (obj->arg.b.unk3d[0] >= 151) {
        return;
    }
    if (game_counter_is_nonzero(obj->arg.b.unk3d[0]) == 0) {
        return;
    }
    if (obj->unka == 0 || obj->unka >= 4095) {
        obj->unk8 = 1;
        DAT_801ba984.unk0 = 1;
    }
}

ADDRESS(0x801e867c, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e8684, 0x8)
void area_on_actor_removed_noop(void)
{
}

/* Slot 3: for an object whose unk3a.bytes[1] is 10, shows one of two notes. */
ADDRESS(0x801e868c, 0xc0)
void area_region_trigger_update_warp_arrival_cells(Object44 *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(0x1c, 0x1e, 3, 4, 0x8000) != 0) {
        if (DAT_801b24e4.unk10a != 0) {
            map_cell_copy_rotated_fields(2, 0x4a, 0, 0x1c, 0x21, 3, 1, 0, 0x2f);
        }
    } else {
        DAT_801b24e4.unk10a = 0;
        map_cell_copy_rotated_fields(2, 0x4d, 0, 0x1c, 0x21, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e874c, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8754, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e875c, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8764, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e876c, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 4: script events 0..2 of an object record. */
ADDRESS(0x801e8774, 0x10c)
void area_talk_event_clear_objects_and_give_items(Object88 *record, s32 event)
{
    Object44 *obj;
    s32 i;

    switch (event) {
    case 0:
        obj = DAT_8018fb3c.objects;
        for (i = 0x18c; i != 0; i--, obj++) {
            if (obj->unk4 == 0x56) {
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->arg.b.unk38 = 0;
                obj->unk0 = 0;
                obj->unk4 = 0xff;
            }
        }
        if (game_counter_is_nonzero(0x66) != 0) {
            game_counter_decrement(0x66);
        }
        event_map_object_interact(NULL, 0x82);
        DAT_801ba984.unke[0x14] = 1;
        break;
    case 1:
        if (DAT_801ba984.unke[0x13] == 0) {
            event_map_object_interact(NULL, 0x60);
            DAT_801ba984.unke[0x13] = 1;
        }
        break;
    case 2:
        event_map_object_interact(NULL, 0x66);
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8880, 0xac)
void area_on_load_restore_cleared_objects(void)
{
    Object44 *obj;
    s32 i;

    if (DAT_801ba984.unke[0x12] == 0) {
        DAT_801ba984.unke[0x12] = 1;
    }
    if (DAT_801ba984.unke[0] != 0) {
        DAT_801ba984.unke[0x16] = 1;
    }
    if (DAT_801ba984.unke[0x14] != 0) {
        obj = DAT_8018fb3c.objects;
        for (i = 0x18c; i != 0; i--, obj++) {
            if (obj->unk4 == 0x56) {
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->arg.b.unk38 = 0;
                obj->unk0 = 0;
                obj->unk4 = 0xff;
            }
        }
    }
    DAT_801ba984.unk84[5] = 5;
}
