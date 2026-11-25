/*
 * FDAT_008.BIN (WIP module): the handler table and the switch tables of its
 * slot 9 and slot 4 handlers, an empty default handler, its slot 0 handler,
 * a gate handler (slot 1), an item handler (slot 2), a record handler (slot
 * 19), a switch handler (slot 3), the kind 0xa6 handlers (slots 8 and 9), an
 * item check (slot 6), event handlers (slots 10 and 7), a record event
 * handler (slot 4) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0xb8)

void area_handler_noop(void);
void area_interact_item_containers_and_dials(VECTOR *pos, SVECTOR *angle);
void area_update_cell_toggles_by_worn_item(void);
void area_use_item_offer_items_in_order(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_open_gate(Actor *obj);
void area_region_trigger_lock_door_and_warp_cells(MapObject *obj);
void area_init_map_object_dial(MapObject *obj, MapObjectDef *def);
void area_update_map_object_dial(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_check_item_held(void);
void area_on_movie_start_unlock_element(u8 id);
void area_on_movie_end_raise_element_and_fade_objects(u8 id);
void area_talk_event_request_item_or_give(Actor *record, s32 event);
void area_on_load_restore_objects(void);

const FdatHeader fdat_header = {
    0x7,
    {
        /*  0 */ area_interact_item_containers_and_dials,
        /*  1 */ area_update_cell_toggles_by_worn_item,
        /*  2 */ area_use_item_offer_items_in_order,
        /*  3 */ area_region_trigger_lock_door_and_warp_cells,
        /*  4 */ area_talk_event_request_item_or_give,
        /*  5 */ area_on_load_restore_objects,
        /*  6 */ area_on_item_added_check_item_held,
        /*  7 */ area_on_movie_end_raise_element_and_fade_objects,
        /*  8 */ area_init_map_object_dial,
        /*  9 */ area_update_map_object_dial,
        /* 10 */ area_on_movie_start_unlock_element,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_open_gate,
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
    0x27,
};

ADDRESS(0x801e83c0, 0x8)
void area_handler_noop(void)
{
}

/*
 * Slot 0: area_interact_item_containers's shape with this module's message table; a kind
 * 0xa6 definition in front advances its object's unk38 (0..5) once.
 */
ADDRESS(0x801e83c8, 0x284)
void area_interact_item_containers_and_dials(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    MapObject *obj;
    MapObject *link;
    MapObjectDef *def;

    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &map_objects_slots.objects[i];
        def = &map_objects_slots.defs[obj->object_id];
        cur_evt_state.unk0 = 1;
        switch (def->action) {
        case 9:
        case 0x15:
            if (obj->arg.b.unk3c != 0) {
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
            break;
        case 0xa6:
            if (obj->action_state == 0) {
                obj->action_state = 1;
                obj->arg.b.unk38 = (obj->arg.b.unk38 + 1) % 6;
            }
            break;
        }
    }
}

/*
 * Slot 1: unless the player's unkf4 is 0x4b, raises objects 100..116 (when
 * object 82 is not in state 1); otherwise lets object 82 close.
 */
ADDRESS(0x801e864c, 0xa8)
void area_update_cell_toggles_by_worn_item(void)
{
    MapObject *gate;
    MapObject *obj;
    s32 i;

    if (cur_avatar_status.unkf4 != 0x4b) {
        gate = &map_objects_slots.objects[82];
        if (map_objects_slots.objects[82].arg.b.unk40 != 1) {
            for (obj = &map_objects_slots.objects[100], i = 16; i != -1; i--, obj++) {
                if (obj->arg.b.unk3f != 1) {
                    obj->arg.b.unk3f = 1;
                    obj->action_state = 1;
                }
            }
        }
        gate->arg.b.unk40 = 1;
    } else {
        obj = &map_objects_slots.objects[82];
        if (map_objects_slots.objects[82].action_state == 0 && map_objects_slots.objects[82].arg.b.unk40 != 0) {
            map_objects_slots.objects[82].action_state = 4;
        }
        obj->arg.b.unk40 = 0;
    }
}

/* Slot 2: items 0x68 and 0x6b..0x6d, in the order flag 0x14 asks for, start record 4. */
ADDRESS(0x801e86f4, 0x130)
void area_use_item_offer_items_in_order(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;

    switch (item) {
    case 0x68:
    case 0x6b:
    case 0x6c:
    case 0x6d:
        if (cur_evt_state.unke[6] != item) {
            return;
        }
        i = actor_find_overlap_excluding_target_type3(pos->vx, pos->vy, pos->vz, 0x578, 0x6a4);
        if (i != -1 && i == 4) {
            switch (item) {
            case 0x68:
                cur_evt_state.unke[6] = 1;
                break;
            case 0x6c:
                cur_evt_state.unke[6] = 2;
                break;
            case 0x6d:
                cur_evt_state.unke[6] = 3;
                break;
            case 0x6b:
                cur_evt_state.unke[6] = 4;
                break;
            }
            game_counter_decrement(item);
            event_target_stream_execute(&actor_status.objects[i]);
            map_object_apply_marker_signal(0x9a);
            cur_evt_state.unk0 = 1;
        }
        break;
    }
}

ADDRESS(0x801e8824, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

/* Slot 19: record 4 sets flag 0x19 and starts object 82. */
ADDRESS(0x801e882c, 0x60)
void area_on_actor_removed_open_gate(Actor *obj)
{
    if (obj - actor_status.objects == 4) {
        cur_evt_state.unke[0xb] = 1;
        map_objects_slots.objects[82].action_state = 1;
        map_objects_slots.objects[82].arg.b.unk38 = 3;
    }
}

/*
 * Slot 3: region operands 0 and 1 arm and disarm object 44; region 10 swaps
 * the warp-arrival cells.
 */
ADDRESS(0x801e888c, 0x144)
void area_region_trigger_lock_door_and_warp_cells(MapObject *obj)
{
    s32 k;

    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (cur_evt_state.unke[8] == 0) {
            k = 44;
            if (map_objects_slots.objects[k].arg.b.unk38 != 0x9a) {
                map_objects_slots.objects[k].arg.b.unk38 = 0x9a;
            }
        }
        break;
    case 1:
        k = 44;
        if (map_objects_slots.objects[k].arg.b.unk38 == 0x9a) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
        break;
    case 10:
        if (player_camera_within_map_region(0x38, 0x23, 3, 3, 0x8000) != 0) {
            if (cur_avatar_status.unk10a != 0) {
                map_cell_copy_rotated_fields(2, 0x4a, 0, 0x38, 0x25, 3, 1, 0, 0x2f);
            }
        } else {
            cur_avatar_status.unk10a = 0;
            map_cell_copy_rotated_fields(2, 0x4d, 0, 0x38, 0x25, 3, 1, 0, 0x2f);
        }
        break;
    }
}

/* Slot 8: an object made from definition kind 0xa6 takes that kind. */
ADDRESS(0x801e89d0, 0x2c)
void area_init_map_object_dial(MapObject *obj, MapObjectDef *def)
{
    if (def->action == 0xa6) {
        obj->action = 0xa6;
        obj->action_state = 1;
        obj->arg.b.unk40 = 0xff;
    }
}

DATA(0x801e8f80, 0x44, ".data")
static u32 area_dial_setting_masks[17] = {
    63, 8, 62, 32, 8, 8, 42, 22, 20, 4, 4, 16, 16, 4, 34, 32, 32,
};

/*
 * Slot 9: steps the tested kind 0xa6 object: 1 sounds it and picks its next
 * setting, 2 turns it, 3 raises or lowers objects 100..116 by the setting's
 * bit in area_dial_setting_masks, 4 picks the setting again.
 */
ADDRESS(0x801e89fc, 0x1e0)
void area_update_map_object_dial(void)
{
    MapObject *obj = map_objects_slots.current;
    MapObjectDef *def = map_objects_slots.current_def;
    MapObject *other;
    s32 bit;
    s32 i;
    s32 j;

    if (obj->action != 0xa6) {
        return;
    }
    switch (obj->action_state) {
    case 0:
        break;
    case 1:
        map_object_play_spatial_sound(obj, def->arg.b.sound_id);
        if (obj->arg.b.unk38 != 0) {
            obj->animation_id = obj->arg.b.unk38 - 1;
        } else {
            obj->animation_id = 5;
        }
        obj->action_state = 2;
        obj->animation_phase = 0;
        break;
    case 2:
        obj->animation_phase += 128;
        if (obj->animation_phase >= 4096) {
            obj->animation_phase = 0xfff;
            obj->action_state = 3;
        }
        break;
    case 3:
        bit = 1 << (obj->animation_id + 1) % 6;
        other = &map_objects_slots.objects[100];
        for (j = 0, i = 16; i != -1; j++, i--, other++) {
            if (bit & area_dial_setting_masks[j]) {
                if (other->arg.b.unk38 == j + 200 && other->arg.b.unk3f != 0) {
                    other->arg.b.unk3f = 0;
                    other->action_state = 1;
                }
            } else if (other->arg.b.unk3f != 1) {
                other->arg.b.unk3f = 1;
                other->action_state = 1;
            }
        }
        obj->arg.b.unk40 = 0xff;
        obj->action_state = 0;
        break;
    case 4:
        if (obj->arg.b.unk38 != 0) {
            obj->animation_id = obj->arg.b.unk38 - 1;
        } else {
            obj->animation_id = 5;
        }
        obj->action_state = 3;
        break;
    }
}

ADDRESS(0x801e8bdc, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8be4, 0x8)
void area_update_actor_target_noop(void)
{
}

/* Slot 6: sets flag 0x15 while item 0x4b is held. */
ADDRESS(0x801e8bec, 0x30)
void area_on_item_added_check_item_held(void)
{
    if (game_counter_is_nonzero(0x4b) != 0) {
        cur_evt_state.unke[7] = 1;
    }
}

/* Slot 10: event 15 sets the third magic element flag. */
ADDRESS(0x801e8c1c, 0x20)
void area_on_movie_start_unlock_element(u8 id)
{
    if (id == 15) {
        cur_evt_state.unk71[2] = 1;
    }
}

/*
 * Slot 7: raises a stat, arms object 44, and objects 70 and 91 each fade out
 * and are removed (death-fade preset 0x32, light blend up over 16 frames).
 */
ADDRESS(0x801e8c3c, 0x1cc)
void area_on_movie_end_raise_element_and_fade_objects(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    MapObject *obj;
    s32 i;

    cur_avatar_status.unk34[2] += 20;
    player_learn_element_magic(2);
    player_recalculate_combat_stats();
    obj = &map_objects_slots.objects[70];
    map_objects_slots.objects[70].lighting_override_index = 0x32;
    map_objects_slots.objects[70].render_mode = 5;
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
    obj = &map_objects_slots.objects[91];
    map_objects_slots.objects[91].lighting_override_index = 0x32;
    map_objects_slots.objects[91].render_mode = 5;
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
    conversation_record_mark_page(12, 16);
    conversation_record_mark_page(12, 17);
    if (cur_evt_state.unke[8] == 0) {
        cur_evt_state.unke[8] = 1;
        actor_status.objects[4].slot_state = 8;
        map_objects_slots.objects[44].arg.b.unk38 = 0xff;
    }
}

/* Slot 4: script events 0..3 set the item flag 0x14 asks for next; 4 and 5 trigger objects. */
ADDRESS(0x801e8e08, 0xac)
void area_talk_event_request_item_or_give(Actor *record, s32 event)
{
    switch (event) {
    case 0:
        cur_evt_state.unke[6] = 0x68;
        break;
    case 1:
        cur_evt_state.unke[6] = 0x6c;
        break;
    case 2:
        cur_evt_state.unke[6] = 0x6d;
        break;
    case 3:
        cur_evt_state.unke[6] = 0x6b;
        break;
    case 4:
        event_map_object_interact(NULL, 0x56);
        break;
    case 5:
        event_map_object_interact(NULL, 0x52);
        break;
    }
}

/* Slot 5, called after the module loads: restores the opened objects and counts the visits. */
ADDRESS(0x801e8eb4, 0xcc)
void area_on_load_restore_objects(void)
{
    s32 k;

    if (cur_evt_state.unk7f[2] != 0) {
        map_objects_slots.objects[70].object_id = 0xff;
        map_objects_slots.objects[91].object_id = 0xff;
        cur_evt_state.unke[8] = 1;
    }
    if (cur_evt_state.unke[8] != 0) {
        actor_status.objects[4].slot_state = 8;
        map_objects_slots.objects[44].arg.b.unk38 = 0xff;
    } else {
        k = 44;
        if (map_objects_slots.objects[k].arg.b.unk38 != 0x9a) {
            map_objects_slots.objects[k].arg.b.unk38 = 0x9a;
        }
    }
    if (cur_evt_state.unke[0xb] != 0) {
        map_objects_slots.objects[82].action_state = 1;
        map_objects_slots.objects[82].arg.b.unk38 = 3;
    }
    if (cur_evt_state.unke[0xa] < 2) {
        cur_evt_state.unke[0xa]++;
    }
    cur_evt_state.unk84[5] = 2;
}
