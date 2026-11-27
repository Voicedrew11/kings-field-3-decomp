/*
 * FDAT_041.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers, a record handler (slot 19), switch handlers (slots 10 and
 * 7), an event handler (slot 4) and the load handler (slot 5).
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
void area_on_actor_removed_track_group(Actor *obj);
void area_region_trigger_noop(MapObject *obj);
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_unlock_element_and_remove_actor(u8 id);
void area_on_movie_end_raise_element_and_fade_objects(u8 id);
void area_talk_event_set_flag(Actor *record, s32 event);
void area_on_load_restore_objects_and_actor(void);

const FdatHeader fdat_header = {
    0x12,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_set_flag,
        /*  5 */ area_on_load_restore_objects_and_actor,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_raise_element_and_fade_objects,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_unlock_element_and_remove_actor,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ (FdatActorTargetCheckHandler)area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_track_group,
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
    0x32,
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

/* Slot 19: records 0..12 each set their bit of the 13-bit mask at +0x48. */
ADDRESS(0x801e85b8, 0x8c)
void area_on_actor_removed_track_group(Actor *obj)
{
    s32 index = obj - actor_status.objects;
    s32 bit;

    if (index >= 0 && index < 13) {
        bit = 1 << index;
        cur_evt_state.unk32[0x16] |= bit;
        cur_evt_state.unk32[0x17] |= bit >> 8;
    }
    if (*(u16 *)&cur_evt_state.unk32[0x16] == 0x1fff) {
        cur_evt_state.unk32[0x18] = 1;
    }
}

ADDRESS(0x801e8644, 0x8)
void area_region_trigger_noop(MapObject *obj)
{
}

ADDRESS(0x801e864c, 0x8)
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def)
{
}

ADDRESS(0x801e8654, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e865c, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8664, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e866c, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 10. */
ADDRESS(0x801e8674, 0x64)
void area_on_movie_start_unlock_element_and_remove_actor(u8 id)
{
    if (id == 4) {
        if (cur_evt_state.unk32[0x14] != 0) {
            map_objects_slots.objects[20].object_id = 0xff;
            actor_status.objects[30].slot_state = 8;
            actor_status.objects[30].lifecycle = 3;
        }
    }
    if (id == 13) {
        cur_evt_state.element_unlocked[1] = 1;
    }
}

/*
 * Slot 7: movie 13 raises a stat; objects 15 and 86 each fade out and are
 * removed (death-fade preset 0x32, light blend up over 16 frames).
 */
ADDRESS(0x801e86d8, 0x1c8)
void area_on_movie_end_raise_element_and_fade_objects(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    MapObject *obj;
    s32 i;

    if (id == 13) {
        cur_avatar_status.base_magic[1] += 20;
        player_learn_element_magic(1);
        player_recalculate_combat_stats();
        obj = &map_objects_slots.objects[15];
        map_objects_slots.objects[15].lighting_override_index = 0x32;
        map_objects_slots.objects[15].render_mode = 5;
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
        obj = &map_objects_slots.objects[86];
        map_objects_slots.objects[86].lighting_override_index = 0x32;
        map_objects_slots.objects[86].render_mode = 5;
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
        conversation_record_mark_page(0xb, 0);
        conversation_record_mark_page(0xb, 1);
    }
    if (id == 4) {
        cur_evt_state.unk32[0x15] = 1;
    }
}

/* Slot 4: script event 0 of an object record sets flag 0x14 once. */
ADDRESS(0x801e88a0, 0x34)
void area_talk_event_set_flag(Actor *record, s32 event)
{
    if (event == 0) {
        if (cur_evt_state.unk32[0x14] == 0) {
            cur_evt_state.unk32[0x14] = 1;
            cur_evt_state.unk32[0x15] = 0;
        }
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e88d4, 0x84)
void area_on_load_restore_objects_and_actor(void)
{
    cur_evt_state.unk32[0x19] = 1;
    if (cur_evt_state.element_magic_learned[1] != 0) {
        map_objects_slots.objects[15].object_id = 0xff;
        map_objects_slots.objects[86].object_id = 0xff;
    }
    if (cur_evt_state.unk32[0x14] == 0) {
        cur_evt_state.unk32[0x15] = 1;
    } else {
        map_objects_slots.objects[20].object_id = 0xff;
        actor_status.objects[30].slot_state = 8;
        actor_status.objects[30].lifecycle = 3;
    }
    cur_evt_state.unk84[5] = 0xd;
}
