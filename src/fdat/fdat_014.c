/*
 * FDAT_014.BIN (WIP module): the handler table and the switch table of its
 * slot 4 handler, the shared light-record blend, the lamp helper, its slot 0
 * handler, an item handler (slot 2), a switch handler (slot 3), event
 * handlers (slots 10 and 7), a record event handler (slot 4) and the load
 * handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x9c)

void area_set_light_blend(u8 n);
void area_handler_noop(void);
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle);
void area_update_noop(void);
void area_use_item_fade_object_and_brighten_light(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7);
void area_on_actor_removed_noop(Actor *obj);
void area_region_trigger_move_object_and_warp_cells(MapObject *obj);
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_unlock_element(u8 id);
void area_on_movie_end_raise_element_and_fade_objects(u8 id);
void area_talk_event_give_items_and_set_flags(Actor *record, s32 event);
void area_on_load_restore_objects_and_light(void);

const FdatHeader fdat_header = {
    0x9,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_fade_object_and_brighten_light,
        /*  3 */ area_region_trigger_move_object_and_warp_cells,
        /*  4 */ area_talk_event_give_items_and_set_flags,
        /*  5 */ area_on_load_restore_objects_and_light,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_raise_element_and_fade_objects,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_unlock_element,
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
    0x29,
};

#include "shared/light_blend_records.inc"

DATA(0x801e8e74, 0x30, ".data")
static LightSourceRecord area_light_blend_from = {
    {{{3000, 800, -1500}, {-2000, 1500, -3400}, {-1300, -2000, 1024}}, 0},
    {{{128, 128, 128}, {128, 128, 128}, {128, 128, 128}}, 0},
    {0x50, 0x50, 0x50, 0},
    6000,
    12000,
};
DATA(0x801e8ea4, 0x30, ".data")
static LightSourceRecord area_light_blend_to = {
    {{{3000, -1500, 1500}, {-3000, -1500, -1500}, {0, 3000, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    18000,
    24000,
};

/* Blends area light record 47 by n sixteenths of the way to the opened state. */
ADDRESS(0x801e8488, 0x58)
void area_set_light_blend(u8 n)
{
    s32 i;
    s32 t;

    for (i = 0, t = 0; i < n; i++) {
        t += 256;
    }
    light_blend_records(&area_light_blend_from, &area_light_blend_to, &light_default_records[47], t);
}

ADDRESS(0x801e84e0, 0x8)
void area_handler_noop(void)
{
}

/* Slot 0: area_interact_item_containers's shape with this module's message table. */
ADDRESS(0x801e84e8, 0x25c)
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    MapObject *obj;
    MapObject *link;
    MapObjectDef *def;

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
                    } else if (obj->object_id == 0x12d) {
                        notify_enqueue(0x1f);
                    } else if (obj->object_id == 0x12e) {
                        notify_enqueue(0x20);
                    } else if (obj->object_id == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->object_id == 0xba || obj->object_id == 303 || obj->object_id == 304) {
                        notify_enqueue(0x22);
                    } else if (obj->object_id == 0x131) {
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

ADDRESS(0x801e8744, 0x8)
void area_update_noop(void)
{
}

/*
 * Slot 2: item 0x7b used on object 0x8d in front lowers its light blend from
 * 0xfff to 0, then fades it out (death-fade preset 0x32) while the area light
 * brightens over 16 frames, and frees object 159.
 */
ADDRESS(0x801e874c, 0x258)
void area_use_item_fade_object_and_brighten_light(VECTOR *pos, SVECTOR *angle, s32 item)
{
    VECTOR at;
    SVECTOR facing;
    MapObject *obj;
    s32 i;
    s32 j;
    s32 k;

    if (item != 0x7b || cur_evt_state.unke[0xf] == 1) {
        return;
    }
    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 0x6a4, angle->vy, 0x200)) != -1; i++) {
        if (i != 0x8d) {
            continue;
        }
        game_counter_decrement(0x7b);
        obj = &map_objects_slots.objects[0x8d];
        actor_status.objects[7].slot_state = 8;
        actor_status.objects[26].slot_state = 1;
        map_objects_slots.objects[0x8d].render_mode = 5;
        map_objects_slots.objects[0x8d].lighting_blend = 0xfff;
        for (j = 15; j != -1; j--) {
            if ((s16)(obj->lighting_blend -= 256) < 0) {
                obj->lighting_blend = 0;
            }
            cd_request_wait_idle();
            cd_request_service_vab();
            cd_request_service_stream();
            player_get_camera_pose(&at, &facing);
            render_game_frame(&at, &facing);
        }
        obj->lighting_override_index = 0x32;
        obj->render_mode = 5;
        for (j = 15; j != -1; j--) {
            area_set_light_blend(16 - j);
            light_reset_records();
            light_refresh_record_rotations();
            player_update_status_visuals();
            obj->lighting_blend += 256;
            if (obj->lighting_blend >= 4095) {
                obj->lighting_blend = 0xfff;
            }
            cd_request_wait_idle();
            cd_request_service_vab();
            cd_request_service_stream();
            player_get_camera_pose(&at, &facing);
            render_game_frame(&at, &facing);
        }
        obj->object_id = 0xff;
        obj->action = 0xff;
        obj->layer_mask = 0;
        map_objects_slots.objects[159].object_id = 0xff;
        k = 74;
        if (map_objects_slots.objects[k].arg.b.unk38 < 253) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
            notify_enqueue(0x18);
        }
        cur_evt_state.unke[0xf] = 1;
        cur_evt_state.interaction_handled = 1;
    }
}

ADDRESS(0x801e89a4, 0x8)
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7)
{
}

ADDRESS(0x801e89ac, 0x8)
void area_on_actor_removed_noop(Actor *obj)
{
}

/*
 * Slot 3: region operands 0 and 1 move object 115 beside them; region 10
 * swaps the warp-arrival cells.
 */
ADDRESS(0x801e89b4, 0x134)
void area_region_trigger_move_object_and_warp_cells(MapObject *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (cur_evt_state.unke[0xf] == 0) {
            map_objects_slots.objects[115].pos.vx = obj->pos.vx - 1024;
        }
        break;
    case 1:
        map_objects_slots.objects[115].pos.vx = obj->pos.vx + 1023;
        break;
    case 10:
        if (player_camera_within_map_region(0x37, 0xc, 3, 3, 0x8000) != 0) {
            if (cur_avatar_status.warp_arrived != 0) {
                map_cell_copy_rotated_fields(2, 0x4a, 0, 0x37, 0xc, 3, 1, 0, 0x2f);
            }
        } else {
            cur_avatar_status.warp_arrived = 0;
            map_cell_copy_rotated_fields(2, 0x4d, 0, 0x37, 0xc, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8ae8, 0x8)
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def)
{
}

ADDRESS(0x801e8af0, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8af8, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8b00, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8b08, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 10: event 12 sets the first magic element flag. */
ADDRESS(0x801e8b10, 0x20)
void area_on_movie_start_unlock_element(u8 id)
{
    if (id == 12) {
        cur_evt_state.element_unlocked[0] = 1;
    }
}

/*
 * Slot 7: raises a stat; objects 55 and 148 each fade out and are removed
 * (death-fade preset 0x32, light blend up over 16 frames).
 */
ADDRESS(0x801e8b30, 0x198)
void area_on_movie_end_raise_element_and_fade_objects(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    MapObject *obj;
    s32 i;

    cur_avatar_status.base_magic[0] += 20;
    player_learn_element_magic(0);
    player_recalculate_combat_stats();
    obj = &map_objects_slots.objects[55];
    map_objects_slots.objects[55].lighting_override_index = 0x32;
    map_objects_slots.objects[55].render_mode = 5;
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
    obj = &map_objects_slots.objects[148];
    map_objects_slots.objects[148].lighting_override_index = 0x32;
    map_objects_slots.objects[148].render_mode = 5;
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
    conversation_record_mark_page(10, 0);
    conversation_record_mark_page(10, 1);
}

/* Slot 4: script events 0..4 of an object record. */
ADDRESS(0x801e8cc8, 0xe0)
void area_talk_event_give_items_and_set_flags(Actor *record, s32 event)
{
    switch (event) {
    case 0:
        if (cur_evt_state.unke[0xd] != 1) {
            cur_evt_state.unke[0xd] = 1;
            event_map_object_interact(NULL, 0x7b);
        }
        break;
    case 1:
        if (cur_evt_state.unke[0xe] != 1) {
            cur_evt_state.unke[0xe] = 1;
            event_map_object_interact(NULL, 0x5f);
        }
        break;
    case 2:
        if (game_counter_is_nonzero(0x84) > 0) {
            cur_evt_state.unke[0x10] = 1;
        }
        break;
    case 3:
        actor_status.objects[8].slot_state = 8;
        break;
    case 4:
        cur_evt_state.unke[0xc] = 1;
        break;
    }
}

/* Slot 5, called after the module loads: restores the sunk object and the area light. */
ADDRESS(0x801e8da8, 0xcc)
void area_on_load_restore_objects_and_light(void)
{
    s32 k;

    if (cur_evt_state.unke[0xf] != 0) {
        actor_status.objects[7].slot_state = 8;
        actor_status.objects[26].slot_state = 1;
        map_objects_slots.objects[0x8d].object_id = 0xff;
        map_objects_slots.objects[159].object_id = 0xff;
        k = 74;
        if (map_objects_slots.objects[k].arg.b.unk38 < 253) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
        area_set_light_blend(16);
    } else {
        actor_status.objects[7].slot_state = 1;
        actor_status.objects[26].slot_state = 8;
        area_set_light_blend(0);
    }
    if (cur_evt_state.element_magic_learned[0] != 0) {
        map_objects_slots.objects[55].object_id = 0xff;
        map_objects_slots.objects[148].object_id = 0xff;
    }
    cur_evt_state.unk84[5] = 4;
}
