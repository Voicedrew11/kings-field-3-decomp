/*
 * FDAT_053.BIN (WIP module): the handler table and the switch table of its
 * slot 19 handler, an empty default handler, the shared slot 0 handler, a
 * facing check (slot 1), an item handler (slot 2), the shared record run
 * fragment, an object state helper, a record handler (slot 19), a switch
 * handler (slot 3), event handlers (slots 10 and 7), a record event handler
 * (slot 4) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x140)

void area_handler_noop(void);
void area_update_respawn_actor_out_of_view(void);
void area_use_item_place_on_object(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(void);
void area_set_cell_toggle(s32 index, s32 state);
void area_on_actor_removed_set_toggles(Actor *obj);
void area_region_trigger_trade_item_and_damage(MapObject *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_show_objects_and_wake_actor(u8 id);
void area_on_movie_end_set_flag(u8 id);
void area_talk_event_give_equipment_and_raise_element(Actor *record, s32 event);
void area_on_load_restore_objects_and_actors(void);

const FdatHeader fdat_header = {
    0x16,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_respawn_actor_out_of_view,
        /*  2 */ area_use_item_place_on_object,
        /*  3 */ area_region_trigger_trade_item_and_damage,
        /*  4 */ area_talk_event_give_equipment_and_raise_element,
        /*  5 */ area_on_load_restore_objects_and_actors,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_set_flag,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_show_objects_and_wake_actor,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_set_toggles,
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
    0x36,
};

ADDRESS(0x801e8448, 0x8)
void area_handler_noop(void)
{
}

#include "shared/area_interact_item_containers.inc"

/* Slot 1: with flag 0x55 at 1, record 15 stops when it faces away from the player. */
ADDRESS(0x801e8658, 0xa0)
void area_update_respawn_actor_out_of_view(void)
{
    s32 i = 15;

    if (cur_evt_state.unk32[0x23] == 1 && actor_status.objects[i].lifecycle != 1) {
        if (angle_within_tolerance(cur_avatar_status.camera_rotation.vy,
                                   vector_xz_to_angle(actor_status.objects[i].pos.vx
                                                      - cur_avatar_status.pos.vx,
                                                      actor_status.objects[i].pos.vz
                                                      - cur_avatar_status.pos.vz),
                                   600) == 0) {
            actor_status.objects[i].lifecycle = 0;
            actor_status.objects[i].slot_state = 2;
        }
    }
    actor_status.objects[56].layer_mask = 0;
}

/*
 * Slot 2: item 0x8f used on object 11 in front spawns an object of that
 * kind at the player and moves it onto object 11 over 64 frames.
 */
ADDRESS(0x801e86f8, 0x224)
void area_use_item_place_on_object(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;
    MapObject *obj;
    MapObject *target;
    VECTOR from;
    VECTOR to;
    s32 t;
    u16 id;

    if (item != 0x8f || cur_evt_state.unk32[0x25] != 0) {
        return;
    }
    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 0x6a4, angle->vy, 0x200)) != -1; i++) {
        if (i != 11) {
            continue;
        }
        target = &map_objects_slots.objects[11];
        obj = map_object_effect_pool_acquire(0x15e, 10, map_objects_slots.scatter_pool_sequence);
        map_object_reset(obj);
        id = map_objects_slots.scatter_pool_sequence++;
        obj->render_depth_offset = -100;
        obj->layer_mask = 2;
        obj->arg.h.unk3c = id;
        obj->object_id = item;
        obj->action = 0xff;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = map_objects_slots.objects[i].angle.vy;
        from = cur_avatar_status.pos;
        obj->pos = from;
        obj->pos.vy -= 1330;
        obj->arg.b.unk38 = 0;
        to = target->pos;
        to.vy -= 50;
        for (t = 0; t < 4096; t += 64) {
            scene_pose_interpolate(obj, &from, &to, NULL, NULL, t);
            cd_request_service_vab();
            cd_request_service_stream();
            render_game_frame(NULL, NULL);
        }
        cur_evt_state.unk32[0x25] = 1;
        cur_evt_state.unk32[0x26] = 0;
        game_counter_decrement(item);
        cur_evt_state.interaction_handled = 1;
    }
}

ADDRESS(0x801e891c, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

#include "shared/area_disable_actor_range.inc"

/* Sets object index's unk3f to state (0 or 1) and marks it changed. */
ADDRESS(0x801e8958, 0x68)
void area_set_cell_toggle(s32 index, s32 state)
{
    MapObject *obj;

    switch (state) {
    case 0:
        obj = &map_objects_slots.objects[index];
        obj->arg.b.unk3f = 0;
        obj->action_state = 1;
        break;
    case 1:
        obj = &map_objects_slots.objects[index];
        obj->arg.b.unk3f = 1;
        obj->action_state = 1;
        break;
    }
}

/*
 * Slot 19: actors 11..13 each disable one actor and set a cell toggle once;
 * actor 15 is disabled once, actor 56 enabled.
 */
ADDRESS(0x801e89c0, 0x150)
void area_on_actor_removed_set_toggles(Actor *obj)
{
    switch (obj - actor_status.objects) {
    case 11:
        if (cur_evt_state.unk32[0x20] == 0) {
            cur_evt_state.unk32[0x20] = 1;
            area_disable_actor_range(0x10, 1);
            area_set_cell_toggle(0x50, 1);
        }
        break;
    case 12:
        if (cur_evt_state.unk32[0x21] == 0) {
            cur_evt_state.unk32[0x21] = 1;
            area_disable_actor_range(0x37, 1);
            area_set_cell_toggle(0x51, 1);
        }
        break;
    case 13:
        if (cur_evt_state.unk32[0x22] == 0) {
            cur_evt_state.unk32[0x22] = 1;
            area_disable_actor_range(0x2e, 1);
            area_set_cell_toggle(5, 1);
        }
        break;
    case 15:
        if (cur_evt_state.unk32[0x23] == 1) {
            cur_evt_state.unk32[0x23] = 2;
            actor_status.objects[15].slot_state = 8;
        }
        break;
    case 56:
        actor_status.objects[56].slot_state = 1;
        actor_status.objects[56].layer_mask = 0;
        break;
    }
}

/*
 * Slot 3: region operand 0 sets flag 0x55, region 1 trades item 0x38 for 0x39
 * behind a fade, regions 0x61 and 0x63 hurt the player.
 */
ADDRESS(0x801e8b10, 0x214)
void area_region_trigger_trade_item_and_damage(MapObject *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (cur_evt_state.unk32[0x23] == 0) {
            cur_evt_state.unk32[0x23] = 1;
        }
        break;
    case 1:
        if (game_counter_is_nonzero(0x38) > 0) {
            color_overlay_transition(0x200, 0, 0, 0, 100, 100, 100);
            if (cur_avatar_status.equipped_shield_id == 0x38) {
                player_set_equipment_slot(0x39, 4);
            }
            game_counter_decrement(0x38);
            game_counter_increment(0x39);
            color_overlay_transition(0x200, 100, 100, 100, 0, 0, 0);
        }
        break;
    case 0x61:
        if (cur_evt_state.unk32[0x20] == 0) {
            player_apply_damage(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        if (cur_evt_state.unk32[0x21] == 0) {
            player_apply_damage(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        if (cur_evt_state.unk32[0x22] == 0) {
            player_apply_damage(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        break;
    case 0x63:
        player_apply_damage(0, 0, 0, 0x43, 0, 0, 0, 0, 0, 0, 0, &cur_avatar_status.pos);
        break;
    }
}

ADDRESS(0x801e8d24, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8d2c, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8d34, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8d3c, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8d44, 0x8)
void area_on_item_added_noop(void)
{
}

/*
 * Slot 10: movie 9 hides object 50, shows objects 52..61, copies a cell
 * pattern and wakes actor 56.
 */
ADDRESS(0x801e8d4c, 0x9c)
void area_on_movie_start_show_objects_and_wake_actor(u8 id)
{
    MapObject *obj;
    s32 i;

    if (id == 9) {
        obj = &map_objects_slots.objects[50];
        obj->layer_mask = 0;
        obj += 2;
        for (i = 9; i != -1; i--) {
            obj->layer_mask = 2;
            obj++;
        }
        map_cell_copy_rotated_fields(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        actor_status.objects[56].lifecycle = 0;
        actor_status.objects[56].slot_state = 2;
    }
}

/* Slot 7: event 9 sets flag 0x58. */
ADDRESS(0x801e8de8, 0x20)
void area_on_movie_end_set_flag(u8 id)
{
    if (id == 9) {
        cur_evt_state.unk32[0x26] = 1;
    }
}

/* Slot 4: script events 0 and 1 of an object record. */
ADDRESS(0x801e8e08, 0x17c)
void area_talk_event_give_equipment_and_raise_element(Actor *record, s32 event)
{
    switch (event) {
    case 0:
        if (cur_evt_state.unk32[0x24] != 1) {
            cur_evt_state.unk32[0x24] = 1;
            event_map_object_interact(NULL, 0x29);
            player_set_equipment_slot(0x29, 0);
            event_map_object_interact(NULL, 0x31);
            player_set_equipment_slot(0x31, 1);
            event_map_object_interact(NULL, 0x46);
            player_set_equipment_slot(0x46, 2);
            event_map_object_interact(NULL, 0x4f);
            player_set_equipment_slot(0x4f, 3);
            event_map_object_interact(NULL, 0x3c);
            player_set_equipment_slot(0x3c, 4);
            map_objects_slots.objects[63].layer_mask = 2;
            map_objects_slots.objects[63].arg.b.unk3f = 0xff;
        }
        break;
    case 1:
        if (cur_evt_state.unk32[0x25] == 1) {
            cur_evt_state.unk32[0x25] = 2;
            color_overlay_transition(0x200, 0, 0, 0, 200, 200, 200);
            color_overlay_transition(0x200, 200, 200, 200, 0, 0, 0);
            cur_avatar_status.base_magic[4] += 30;
            player_learn_element_magic(4);
            player_recalculate_combat_stats();
            actor_status.objects[56].slot_state = 8;
            actor_status.objects[56].layer_mask = 0;
        }
        break;
    }
}

/* Slot 5, called after the module loads: restores the objects of the flags. */
ADDRESS(0x801e8f84, 0x374)
void area_on_load_restore_objects_and_actors(void)
{
    MapObject *obj;
    MapObject *p;
    s32 i;
    u16 id;

    if (cur_evt_state.unk32[0x20] != 0) {
        area_disable_actor_range(0x10, 1);
        area_set_cell_toggle(0x50, 1);
    } else {
        area_set_cell_toggle(0x50, 0);
    }
    if (cur_evt_state.unk32[0x21] != 0) {
        area_disable_actor_range(0x37, 1);
        area_set_cell_toggle(0x51, 1);
    } else {
        area_set_cell_toggle(0x51, 0);
    }
    if (cur_evt_state.unk32[0x22] != 0) {
        area_disable_actor_range(0x2e, 1);
        area_set_cell_toggle(5, 1);
    } else {
        area_set_cell_toggle(5, 0);
    }
    actor_status.objects[15].slot_state = 8;
    if (cur_evt_state.unk32[0x24] != 0) {
        map_objects_slots.objects[63].layer_mask = 2;
        map_objects_slots.objects[63].arg.b.unk3f = 0xff;
    } else {
        map_objects_slots.objects[63].layer_mask = 0;
        map_objects_slots.objects[63].arg.b.unk3f = 0;
    }
    if (cur_evt_state.unk32[0x25] == 0) {
        p = &map_objects_slots.objects[50];
        p->layer_mask = 2;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->layer_mask = 0;
            p++;
        }
        map_cell_copy_rotated_fields(2, 0x38, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        actor_status.objects[56].slot_state = 8;
        actor_status.objects[56].layer_mask = 0;
        map_objects_slots.objects[11].layer_mask = 2;
    } else if (cur_evt_state.unk32[0x25] == 1) {
        p = &map_objects_slots.objects[50];
        p->layer_mask = 0;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->layer_mask = 2;
            p++;
        }
        map_cell_copy_rotated_fields(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        actor_status.objects[56].slot_state = 1;
        actor_status.objects[56].layer_mask = 0;
        map_objects_slots.objects[11].layer_mask = 2;
        obj = map_object_effect_pool_acquire(0x15e, 10, map_objects_slots.scatter_pool_sequence);
        map_object_reset(obj);
        id = map_objects_slots.scatter_pool_sequence++;
        obj->render_depth_offset = -100;
        obj->object_id = 0x8f;
        obj->arg.h.unk3c = id;
        obj->layer_mask = 2;
        obj->action = 0xff;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = map_objects_slots.objects[11].angle.vy;
        obj->arg.b.unk38 = 0;
        obj->pos.vx = map_objects_slots.objects[11].pos.vx;
        obj->pos.vy = map_objects_slots.objects[11].pos.vy - 50;
        obj->pos.vz = map_objects_slots.objects[11].pos.vz;
    } else {
        p = &map_objects_slots.objects[50];
        p->layer_mask = 0;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->layer_mask = 2;
            p++;
        }
        map_cell_copy_rotated_fields(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        actor_status.objects[56].slot_state = 8;
        actor_status.objects[56].layer_mask = 0;
        map_objects_slots.objects[11].layer_mask = 0;
    }
    cur_evt_state.unk32[0x26] = 1;
    cur_evt_state.unk84[5] = 0x11;
}
