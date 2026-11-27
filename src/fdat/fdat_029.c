/*
 * FDAT_029.BIN (WIP module): the shared light-record blend, the handler
 * table, its slot 0 handler, empty handlers, a switch handler (slot 3), a
 * record event handler (slot 4) and the load handler (slot 5).
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
void area_interact_item_containers_and_unlock(VECTOR *pos, SVECTOR *angle);
void area_update_noop(void);
void area_use_item_noop(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7);
void area_on_actor_removed_noop(Actor *obj);
void area_region_trigger_damage_barrier_and_doors(MapObject *obj);
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_set_flags(Actor *record, s32 event);
void area_on_load_restore_doors_and_barrier(void);

const FdatHeader fdat_header = {
    0xe,
    {
        /*  0 */ area_interact_item_containers_and_unlock,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_damage_barrier_and_doors,
        /*  4 */ area_talk_event_set_flags,
        /*  5 */ area_on_load_restore_doors_and_barrier,
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
    0x2e,
};

#include "shared/light_blend_records.inc"

ADDRESS(0x801e8474, 0x8)
void area_handler_noop(void)
{
}

/*
 * Slot 0: first sets flag 0x3a (and starts sound 0xf1) once item 0x7d is
 * held or the level reaches 50, then area_interact_item_containers's shape with this
 * module's message table.
 */
ADDRESS(0x801e847c, 0x278)
void area_interact_item_containers_and_unlock(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    MapObject *obj;
    MapObject *link;
    MapObjectDef *def;

    if (cur_evt_state.unk32[8] == 0) {
        if (game_counter_is_nonzero(0x7d) > 0 || cur_avatar_status.level >= 50) {
            cur_evt_state.unk32[8] = 1;
            map_object_process_marker_all(0xf1);
        }
    }
    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &map_objects_slots.objects[i];
        def = &map_objects_slots.defs[obj->object_id];
        cur_evt_state.interaction_handled = 1;
        if ((def->action == 9 || def->action == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        notify_enqueue(obj->arg.b.unk3f);
                    }
                    if (obj->object_id == 212 || obj->object_id == 213 || obj->object_id == 0xf7) {
                        notify_enqueue(0x1a);
                    } else if (obj->object_id == 0xb7) {
                        notify_enqueue(0x1e);
                    } else if (obj->object_id == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->object_id == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->object_id == 312 || obj->object_id == 313 || obj->object_id == 0x13a) {
                        notify_enqueue(0x21);
                    } else if (obj->object_id >= 190 && obj->object_id < 198) {
                        notify_enqueue(8);
                    }
                }
            } else {
                link = &map_objects_slots.objects[obj->arg.b.unk3a.index];
                if (link->object_id == 0xff) {
                    goto message;
                }
                link->layer_mask = obj->arg.b.unk40;
                link->arg.b.unk38 = 0xff;
                event_map_object_interact(link);
                link->layer_mask = 0;
                link->arg.b.unk38 = 0;
                if (link->object_id == 0xff) {
                    obj->arg.b.unk3a.index = 0xffff;
                }
            }
        }
    }
}

ADDRESS(0x801e86f4, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e86fc, 0x8)
void area_use_item_noop(VECTOR *pos, SVECTOR *angle, s32 item)
{
}

ADDRESS(0x801e8704, 0x8)
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7)
{
}

ADDRESS(0x801e870c, 0x8)
void area_on_actor_removed_noop(Actor *obj)
{
}

/*
 * Slot 3: region operand 0 (with flag 0x38 set) opens objects 249 and 262 and
 * disables actor 2; region 1: object 58 deals radial damage until the four
 * element flags are set, then fades out (death-fade preset 0x32, light blend
 * up over 16 frames) and is removed; region 10 swaps the warp-arrival cells.
 */
ADDRESS(0x801e8714, 0x354)
void area_region_trigger_damage_barrier_and_doors(MapObject *obj)
{
    VECTOR pos;
    SVECTOR angle;
    s32 i;
    MapObject *other;

    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (cur_evt_state.unk32[6] == 1 && player_camera_within_map_region(0x36, 0xf, 10, 4, 0x8000) == 0) {
            actor_status.objects[2].slot_state = 8;
            map_objects_slots.objects[249].layer_mask = 2;
            map_objects_slots.objects[249].arg.b.unk3c = 0xff;
            map_objects_slots.objects[262].arg.b.unk3d[1] = 0xff;
        }
        break;
    case 1:
        if (cur_evt_state.unk32[7] == 0) {
            if (cur_evt_state.element_magic_learned[0] != 0 && cur_evt_state.element_magic_learned[1] != 0 &&
                cur_evt_state.element_magic_learned[2] != 0 && cur_evt_state.element_magic_learned[3] != 0) {
                cur_evt_state.unk32[7] = 1;
            }
            obj = &map_objects_slots.objects[58];
            obj->layer_mask = 2;
            if (cur_avatar_status.reaction_mode == 0x10) {
                audio_play_spatial_default_range(0x28, &obj->pos, 0x78, 0);
            }
            player_apply_radial_damage(&obj->pos, 0, 0x1000, 0x8000, 0x1000, 0x40, 0x40, 0x40, 0x40, 0x40,
                                       0x40, 0x40, 0x40, 0x40, 0x400, 10);
        } else if (cur_evt_state.unk32[7] == 1) {
            other = &map_objects_slots.objects[58];
            other->layer_mask = 2;
            map_objects_slots.objects[58].lighting_override_index = 0x32;
            map_objects_slots.objects[58].render_mode = 5;
            map_objects_slots.objects[58].lighting_blend = 0;
            for (i = 15; i != -1; i--) {
                other->lighting_blend += 256;
                if (other->lighting_blend >= 4095) {
                    other->lighting_blend = 0xfff;
                }
                cd_request_wait_idle();
                cd_request_service_vab();
                cd_request_service_stream();
                player_get_camera_pose(&pos, &angle);
                render_game_frame(&pos, &angle);
            }
            other->object_id = 0xff;
            other->action = 0xff;
            other->layer_mask = 0;
            cur_evt_state.unk32[7] = 2;
        } else {
            map_objects_slots.objects[58].object_id = 0xff;
            map_objects_slots.objects[58].action = 0xff;
            map_objects_slots.objects[58].layer_mask = 0;
        }
        break;
    case 10:
        if (player_camera_within_map_region(0x24, 0x36, 7, 0xc, 0x8000) != 0) {
            if (cur_avatar_status.warp_arrived != 0) {
                map_cell_copy_rotated_fields(2, 0x2a, 0x4f, 0x26, 0x36, 3, 1, 0, 0x2f);
            }
        } else {
            cur_avatar_status.warp_arrived = 0;
            map_cell_copy_rotated_fields(2, 0x2d, 0x4f, 0x26, 0x36, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8a68, 0x8)
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def)
{
}

ADDRESS(0x801e8a70, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8a78, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8a80, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8a88, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 4: script events 0 and 1 of an object record set flags 0x37 and 0x38. */
ADDRESS(0x801e8a90, 0x3c)
void area_talk_event_set_flags(Actor *record, s32 event)
{
    switch (event) {
    case 0:
        cur_evt_state.unk32[5] = 1;
        break;
    case 1:
        cur_evt_state.unk32[6] = 1;
        break;
    }
}

/* Slot 5, called after the module loads: restores the switch states. */
ADDRESS(0x801e8acc, 0x140)
void area_on_load_restore_doors_and_barrier(void)
{
    if (cur_evt_state.unke[0x1a] != 0 && cur_evt_state.unk2b != 0) {
        cur_evt_state.unk32[4] = 1;
    }
    if (cur_evt_state.unk32[6] == 0) {
        actor_status.objects[2].slot_state = 1;
        map_objects_slots.objects[249].layer_mask = 0;
        map_objects_slots.objects[249].arg.b.unk3c = 0;
        map_objects_slots.objects[262].arg.b.unk3d[1] = 0;
    } else {
        actor_status.objects[2].slot_state = 8;
        map_objects_slots.objects[249].layer_mask = 2;
        map_objects_slots.objects[249].arg.b.unk3c = 0xff;
        map_objects_slots.objects[262].arg.b.unk3d[1] = 0xff;
    }
    if (cur_evt_state.unk32[7] != 0) {
        map_objects_slots.objects[58].object_id = 0xff;
        map_objects_slots.objects[58].action = 0xff;
        map_objects_slots.objects[58].layer_mask = 0;
        cur_evt_state.unk32[7] = 2;
    }
    if (cur_evt_state.unk32[8] == 0) {
        if (game_counter_is_nonzero(0x7d) > 0 || cur_avatar_status.level >= 40) {
            cur_evt_state.unk32[8] = 1;
            map_object_process_marker_all(0xf1);
        }
    }
    cur_evt_state.unk84[5] = 9;
}
