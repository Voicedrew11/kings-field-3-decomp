/*
 * FDAT_047.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers, a two-door opener, record 3's and 40's handler (slot 19)
 * and the load handler (slot 5).
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
void area_use_item_noop(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7);
void area_fade_out_object_pair(s32 a, s32 b);
void area_on_actor_removed_fade_object_pair(Actor *obj);
void area_region_trigger_noop(MapObject *obj);
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_noop(Actor *obj, s32 event);
void area_on_load_restore_removed_objects(void);

const FdatHeader fdat_header = {
    0x14,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_restore_removed_objects,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ (FdatMovieHandler)area_handler_noop,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ (FdatMovieHandler)area_handler_noop,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ (FdatActorTargetCheckHandler)area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_fade_object_pair,
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
    0x34,
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
void area_use_item_noop(VECTOR *pos, SVECTOR *angle, s32 item)
{
}

ADDRESS(0x801e85b0, 0x8)
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7)
{
}

/*
 * Objects a and b fade out and are removed (death-fade preset 0x32, light
 * blend (lighting_blend) up to 0xfff over 16 frames).
 */
ADDRESS(0x801e85b8, 0x120)
void area_fade_out_object_pair(s32 a, s32 b)
{
    VECTOR pos;
    SVECTOR angle;
    MapObject *first;
    MapObject *second;
    s32 i;

    first = &map_objects_slots.objects[a];
    second = &map_objects_slots.objects[b];
    first->lighting_override_index = 0x32;
    second->lighting_override_index = 0x32;
    first->render_mode = 5;
    second->render_mode = 5;
    for (i = 15; i != -1; i--) {
        first->lighting_blend += 256;
        second->lighting_blend += 256;
        if (first->lighting_blend >= 4095) {
            first->lighting_blend = 0xfff;
        }
        if (second->lighting_blend >= 4095) {
            second->lighting_blend = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
    }
    first->object_id = 0xff;
    second->object_id = 0xff;
}

#include "shared/area_disable_actor_range.inc"

/* Slot 19: records 3 and 40 each open their pair of doors once. */
ADDRESS(0x801e870c, 0xcc)
void area_on_actor_removed_fade_object_pair(Actor *obj)
{
    switch (obj - actor_status.objects) {
    case 3:
        if (cur_evt_state.unk32[0x1b] == 0) {
            cur_evt_state.unk32[0x1b] = 1;
            area_disable_actor_range(4, 2);
            area_fade_out_object_pair(0x73, 0x74);
        }
        break;
    case 40:
        if (cur_evt_state.unk32[0x1c] == 0) {
            cur_evt_state.unk32[0x1c] = 1;
            area_disable_actor_range(10, 2);
            area_fade_out_object_pair(0x75, 0x76);
        }
        break;
    }
}

ADDRESS(0x801e87d8, 0x8)
void area_region_trigger_noop(MapObject *obj)
{
}

ADDRESS(0x801e87e0, 0x8)
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def)
{
}

ADDRESS(0x801e87e8, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e87f0, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e87f8, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8800, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e8808, 0x8)
void area_talk_event_noop(Actor *obj, s32 event)
{
}

/* Slot 5, called after the module loads: restores the opened doors. */
ADDRESS(0x801e8810, 0x80)
void area_on_load_restore_removed_objects(void)
{
    if (cur_evt_state.unk32[0x1b] != 0) {
        area_disable_actor_range(4, 2);
        map_objects_slots.objects[0x73].object_id = 0xff;
        map_objects_slots.objects[0x74].object_id = 0xff;
    }
    if (cur_evt_state.unk32[0x1c] != 0) {
        area_disable_actor_range(10, 2);
        map_objects_slots.objects[0x75].object_id = 0xff;
        map_objects_slots.objects[0x76].object_id = 0xff;
    }
    cur_evt_state.unk84[5] = 0xf;
}
