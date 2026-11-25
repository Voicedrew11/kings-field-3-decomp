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
void area_on_actor_removed_set_toggles(Object88 *obj);
void area_region_trigger_trade_item_and_damage(Object44 *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_show_objects_and_wake_actor(u8 id);
void area_on_movie_end_set_flag(u8 id);
void area_talk_event_give_equipment_and_raise_element(Object88 *record, s32 event);
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

    if (DAT_801ba984.unk32[0x23] == 1 && DAT_80185da8.objects[i].unk9 != 1) {
        if (angle_within_tolerance(DAT_801b24e4.unk124.vy,
                                   vector_xz_to_angle(DAT_80185da8.objects[i].pos.vx - DAT_801b24e4.pos.vx,
                                                      DAT_80185da8.objects[i].pos.vz - DAT_801b24e4.pos.vz),
                                   600) == 0) {
            DAT_80185da8.objects[i].unk9 = 0;
            DAT_80185da8.objects[i].unk0 = 2;
        }
    }
    DAT_80185da8.objects[56].unk3 = 0;
}

/*
 * Slot 2: item 0x8f used on object 11 in front spawns an object of that
 * kind at the player and moves it onto object 11 over 64 frames.
 */
ADDRESS(0x801e86f8, 0x224)
void area_use_item_place_on_object(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;
    Object44 *obj;
    Object44 *target;
    VECTOR from;
    VECTOR to;
    s32 t;
    u16 id;

    if (item != 0x8f || DAT_801ba984.unk32[0x25] != 0) {
        return;
    }
    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 0x6a4, angle->vy, 0x200)) != -1; i++) {
        if (i != 11) {
            continue;
        }
        target = &DAT_8018fb3c.objects[11];
        obj = map_object_effect_pool_acquire(0x15e, 10, DAT_8018fb3c.unk885e);
        map_object_reset(obj);
        id = DAT_8018fb3c.unk885e++;
        obj->unke = -100;
        obj->unk0 = 2;
        obj->arg.h.unk3c = id;
        obj->unk6 = item;
        obj->unk4 = 0xff;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = DAT_8018fb3c.objects[i].angle.vy;
        from = DAT_801b24e4.pos;
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
        DAT_801ba984.unk32[0x25] = 1;
        DAT_801ba984.unk32[0x26] = 0;
        game_counter_decrement(item);
        DAT_801ba984.unk0 = 1;
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
    Object44 *obj;

    switch (state) {
    case 0:
        obj = &DAT_8018fb3c.objects[index];
        obj->arg.b.unk3f = 0;
        obj->unk8 = 1;
        break;
    case 1:
        obj = &DAT_8018fb3c.objects[index];
        obj->arg.b.unk3f = 1;
        obj->unk8 = 1;
        break;
    }
}

/*
 * Slot 19: actors 11..13 each disable one actor and set a cell toggle once;
 * actor 15 is disabled once, actor 56 enabled.
 */
ADDRESS(0x801e89c0, 0x150)
void area_on_actor_removed_set_toggles(Object88 *obj)
{
    switch (obj - DAT_80185da8.objects) {
    case 11:
        if (DAT_801ba984.unk32[0x20] == 0) {
            DAT_801ba984.unk32[0x20] = 1;
            area_disable_actor_range(0x10, 1);
            area_set_cell_toggle(0x50, 1);
        }
        break;
    case 12:
        if (DAT_801ba984.unk32[0x21] == 0) {
            DAT_801ba984.unk32[0x21] = 1;
            area_disable_actor_range(0x37, 1);
            area_set_cell_toggle(0x51, 1);
        }
        break;
    case 13:
        if (DAT_801ba984.unk32[0x22] == 0) {
            DAT_801ba984.unk32[0x22] = 1;
            area_disable_actor_range(0x2e, 1);
            area_set_cell_toggle(5, 1);
        }
        break;
    case 15:
        if (DAT_801ba984.unk32[0x23] == 1) {
            DAT_801ba984.unk32[0x23] = 2;
            DAT_80185da8.objects[15].unk0 = 8;
        }
        break;
    case 56:
        DAT_80185da8.objects[56].unk0 = 1;
        DAT_80185da8.objects[56].unk3 = 0;
        break;
    }
}

/*
 * Slot 3: region operand 0 sets flag 0x55, region 1 trades item 0x38 for 0x39
 * behind a fade, regions 0x61 and 0x63 hurt the player.
 */
ADDRESS(0x801e8b10, 0x214)
void area_region_trigger_trade_item_and_damage(Object44 *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unk32[0x23] == 0) {
            DAT_801ba984.unk32[0x23] = 1;
        }
        break;
    case 1:
        if (game_counter_is_nonzero(0x38) > 0) {
            color_overlay_transition(0x200, 0, 0, 0, 100, 100, 100);
            if (DAT_801b24e4.unkf2 == 0x38) {
                player_set_equipment_slot(0x39, 4);
            }
            game_counter_decrement(0x38);
            game_counter_increment(0x39);
            color_overlay_transition(0x200, 100, 100, 100, 0, 0, 0);
        }
        break;
    case 0x61:
        if (DAT_801ba984.unk32[0x20] == 0) {
            player_apply_damage(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        if (DAT_801ba984.unk32[0x21] == 0) {
            player_apply_damage(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        if (DAT_801ba984.unk32[0x22] == 0) {
            player_apply_damage(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        break;
    case 0x63:
        player_apply_damage(0, 0, 0, 0x43, 0, 0, 0, 0, 0, 0, 0, &DAT_801b24e4.pos);
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
    Object44 *obj;
    s32 i;

    if (id == 9) {
        obj = &DAT_8018fb3c.objects[50];
        obj->unk0 = 0;
        obj += 2;
        for (i = 9; i != -1; i--) {
            obj->unk0 = 2;
            obj++;
        }
        map_cell_copy_rotated_fields(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk9 = 0;
        DAT_80185da8.objects[56].unk0 = 2;
    }
}

/* Slot 7: event 9 sets flag 0x58. */
ADDRESS(0x801e8de8, 0x20)
void area_on_movie_end_set_flag(u8 id)
{
    if (id == 9) {
        DAT_801ba984.unk32[0x26] = 1;
    }
}

/* Slot 4: script events 0 and 1 of an object record. */
ADDRESS(0x801e8e08, 0x17c)
void area_talk_event_give_equipment_and_raise_element(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        if (DAT_801ba984.unk32[0x24] != 1) {
            DAT_801ba984.unk32[0x24] = 1;
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
            DAT_8018fb3c.objects[63].unk0 = 2;
            DAT_8018fb3c.objects[63].arg.b.unk3f = 0xff;
        }
        break;
    case 1:
        if (DAT_801ba984.unk32[0x25] == 1) {
            DAT_801ba984.unk32[0x25] = 2;
            color_overlay_transition(0x200, 0, 0, 0, 200, 200, 200);
            color_overlay_transition(0x200, 200, 200, 200, 0, 0, 0);
            DAT_801b24e4.unk34[4] += 30;
            player_learn_element_magic(4);
            player_recalculate_combat_stats();
            DAT_80185da8.objects[56].unk0 = 8;
            DAT_80185da8.objects[56].unk3 = 0;
        }
        break;
    }
}

/* Slot 5, called after the module loads: restores the objects of the flags. */
ADDRESS(0x801e8f84, 0x374)
void area_on_load_restore_objects_and_actors(void)
{
    Object44 *obj;
    Object44 *p;
    s32 i;
    u16 id;

    if (DAT_801ba984.unk32[0x20] != 0) {
        area_disable_actor_range(0x10, 1);
        area_set_cell_toggle(0x50, 1);
    } else {
        area_set_cell_toggle(0x50, 0);
    }
    if (DAT_801ba984.unk32[0x21] != 0) {
        area_disable_actor_range(0x37, 1);
        area_set_cell_toggle(0x51, 1);
    } else {
        area_set_cell_toggle(0x51, 0);
    }
    if (DAT_801ba984.unk32[0x22] != 0) {
        area_disable_actor_range(0x2e, 1);
        area_set_cell_toggle(5, 1);
    } else {
        area_set_cell_toggle(5, 0);
    }
    DAT_80185da8.objects[15].unk0 = 8;
    if (DAT_801ba984.unk32[0x24] != 0) {
        DAT_8018fb3c.objects[63].unk0 = 2;
        DAT_8018fb3c.objects[63].arg.b.unk3f = 0xff;
    } else {
        DAT_8018fb3c.objects[63].unk0 = 0;
        DAT_8018fb3c.objects[63].arg.b.unk3f = 0;
    }
    if (DAT_801ba984.unk32[0x25] == 0) {
        p = &DAT_8018fb3c.objects[50];
        p->unk0 = 2;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->unk0 = 0;
            p++;
        }
        map_cell_copy_rotated_fields(2, 0x38, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk0 = 8;
        DAT_80185da8.objects[56].unk3 = 0;
        DAT_8018fb3c.objects[11].unk0 = 2;
    } else if (DAT_801ba984.unk32[0x25] == 1) {
        p = &DAT_8018fb3c.objects[50];
        p->unk0 = 0;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->unk0 = 2;
            p++;
        }
        map_cell_copy_rotated_fields(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk0 = 1;
        DAT_80185da8.objects[56].unk3 = 0;
        DAT_8018fb3c.objects[11].unk0 = 2;
        obj = map_object_effect_pool_acquire(0x15e, 10, DAT_8018fb3c.unk885e);
        map_object_reset(obj);
        id = DAT_8018fb3c.unk885e++;
        obj->unke = -100;
        obj->unk6 = 0x8f;
        obj->arg.h.unk3c = id;
        obj->unk0 = 2;
        obj->unk4 = 0xff;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = DAT_8018fb3c.objects[11].angle.vy;
        obj->arg.b.unk38 = 0;
        obj->pos.vx = DAT_8018fb3c.objects[11].pos.vx;
        obj->pos.vy = DAT_8018fb3c.objects[11].pos.vy - 50;
        obj->pos.vz = DAT_8018fb3c.objects[11].pos.vz;
    } else {
        p = &DAT_8018fb3c.objects[50];
        p->unk0 = 0;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->unk0 = 2;
            p++;
        }
        map_cell_copy_rotated_fields(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk0 = 8;
        DAT_80185da8.objects[56].unk3 = 0;
        DAT_8018fb3c.objects[11].unk0 = 0;
    }
    DAT_801ba984.unk32[0x26] = 1;
    DAT_801ba984.unk84[5] = 0x11;
}
