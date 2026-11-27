/*
 * FDAT_005.BIN (WIP module): the handler table, an empty default handler,
 * its slot 0 handler, a distance check (slot 1), an item handler (slot 2),
 * a record handler (slot 19), a warp-arrival cell handler (slot 3), an event handler
 * (slot 7), a record event handler (slot 4) and the load handler (slot 5).
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
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle);
void area_update_actor_swap_when_far(void);
void area_use_item_on_actor_unlock_door(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7);
void area_on_actor_removed_set_flag(Actor *obj);
void func_801e87c4(void);
void func_801e87cc(void);
void func_801e87d4(void);
void area_region_trigger_update_warp_arrival_cells(MapObject *obj);
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_on_movie_start_noop(u8 movie_id);
void area_on_movie_end_restore_hp(u8 id);
void area_talk_event_give_items_and_copy_cells(Actor *record, s32 event);
void area_on_load_restore_actors_and_cells(void);

const FdatHeader fdat_header = {
    0x6,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_actor_swap_when_far,
        /*  2 */ area_use_item_on_actor_unlock_door,
        /*  3 */ area_region_trigger_update_warp_arrival_cells,
        /*  4 */ area_talk_event_give_items_and_copy_cells,
        /*  5 */ area_on_load_restore_actors_and_cells,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_restore_hp,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_noop,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ (FdatActorTargetCheckHandler)area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_set_flag,
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
    0x26,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

/* Slot 0: area_interact_item_containers's shape with this module's message table. */
ADDRESS(0x801e8398, 0x210)
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
                    if (obj->object_id == 212 || obj->object_id == 213 || obj->object_id == 0xf7 ||
                        obj->object_id == 0x134) {
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
    }
}

/* Slot 1: once record 20 is more than 20 map cells from the player, flag 0xf moves the records on. */
ADDRESS(0x801e85a8, 0xd8)
void area_update_actor_swap_when_far(void)
{
    s32 dx;
    s32 dz;

    if (cur_evt_state.unke[0] != 0 && cur_evt_state.unke[1] == 0) {
        dx = (actor_status.objects[20].pos.vx - cur_avatar_status.pos.vx) / 2048;
        dz = (actor_status.objects[20].pos.vz - cur_avatar_status.pos.vz) / 2048;
        dx *= dx;
        dz *= dz;
        if (dx + dz > 400) {
            cur_evt_state.unke[1] = 1;
            actor_status.objects[20].slot_state = 8;
            actor_status.objects[10].slot_state = 1;
            actor_status.objects[25].slot_state = 8;
            actor_status.objects[23].slot_state = 1;
            actor_status.objects[24].slot_state = 1;
            actor_status.objects[40].slot_state = 8;
        }
    }
}

/* Slot 2: item 0x7f used on record 20 starts it and opens object 71. */
ADDRESS(0x801e8680, 0xd8)
void area_use_item_on_actor_unlock_door(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;
    s32 k;

    if (item != 0x7f || cur_evt_state.unke[0] != 0) {
        return;
    }
    i = actor_find_overlap_excluding_target_type3(pos->vx, pos->vy, pos->vz, 0x578, 0x6a4);
    if (i != -1 && i == 20) {
        game_counter_decrement(0x7f);
        cur_evt_state.unke[0] = 1;
        event_target_stream_execute(&actor_status.objects[20]);
        k = 71;
        if (map_objects_slots.objects[k].arg.b.unk38 == 0xc8) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
        actor_status.objects[22].slot_state = 8;
        actor_status.objects[39].slot_state = 1;
        cur_evt_state.interaction_handled = 1;
    }
}

ADDRESS(0x801e8758, 0x8)
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7)
{
}

/* Slot 19: records 24 and 40 set flag 0x10 once. */
ADDRESS(0x801e8760, 0x64)
void area_on_actor_removed_set_flag(Actor *obj)
{
    s32 index = obj - actor_status.objects;

    if (index == 24 || index == 40) {
        if (cur_evt_state.unke[2] == 0) {
            cur_evt_state.unke[2] = 1;
        }
    }
}

ADDRESS(0x801e87c4, 0x8)
void func_801e87c4(void)
{
}

ADDRESS(0x801e87cc, 0x8)
void func_801e87cc(void)
{
}

ADDRESS(0x801e87d4, 0x8)
void func_801e87d4(void)
{
}

/*
 * Slot 3: region operand 10 (unk3a.bytes[1]): while the player is on the warp
 * arrival pad copies the arrival cell pattern, else clears the arrival flag
 * (player +0x10a) and copies the normal pattern back.
 */
ADDRESS(0x801e87dc, 0xc0)
void area_region_trigger_update_warp_arrival_cells(MapObject *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(0x1c, 6, 9, 0xb, 0x8000) != 0) {
        if (cur_avatar_status.warp_arrived != 0) {
            map_cell_copy_rotated_fields(2, 0x48, 0, 0x1f, 0x10, 3, 1, 0, 0x2f);
        }
    } else {
        cur_avatar_status.warp_arrived = 0;
        map_cell_copy_rotated_fields(2, 0x4b, 0, 0x1f, 0x10, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e889c, 0x8)
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def)
{
}

ADDRESS(0x801e88a4, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e88ac, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e88b4, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e88bc, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e88c4, 0x8)
void area_on_movie_start_noop(u8 movie_id)
{
}

/* Slot 7: restores the player's hp. */
ADDRESS(0x801e88cc, 0x1c)
void area_on_movie_end_restore_hp(u8 id)
{
    cur_avatar_status.hp = cur_avatar_status.hp_max;
}

/* Slot 4: script events 0..2 of an object record. */
ADDRESS(0x801e88e8, 0xc0)
void area_talk_event_give_items_and_copy_cells(Actor *record, s32 event)
{
    switch (event) {
    case 0:
        if (cur_evt_state.unke[3] == 0) {
            cur_evt_state.unke[3] = 1;
            map_cell_copy_rotated_fields(2, 0x4e, 0, 0x1b, 0x11, 1, 7, 0, 0x2f);
        }
        break;
    case 1:
        event_map_object_interact(NULL, 0x74);
        break;
    case 2:
        event_map_object_interact(NULL, 0x72);
        break;
    }
}

/* Slot 5, called after the module loads: restores the records of the flags. */
ADDRESS(0x801e89a8, 0x1d0)
void area_on_load_restore_actors_and_cells(void)
{
    s32 k;

    if (cur_evt_state.unke[0] != 0) {
        actor_status.objects[20].slot_state = 8;
        actor_status.objects[10].slot_state = 1;
        actor_status.objects[25].slot_state = 8;
        actor_status.objects[23].slot_state = 1;
        actor_status.objects[24].slot_state = 1;
        actor_status.objects[40].slot_state = 8;
        k = 71;
        if (map_objects_slots.objects[k].arg.b.unk38 == 0xc8) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
        actor_status.objects[22].slot_state = 8;
        actor_status.objects[39].slot_state = 1;
    } else {
        actor_status.objects[20].slot_state = 1;
        actor_status.objects[10].slot_state = 8;
        actor_status.objects[25].slot_state = 1;
        actor_status.objects[23].slot_state = 8;
        actor_status.objects[24].slot_state = 1;
        actor_status.objects[40].slot_state = 8;
        actor_status.objects[22].slot_state = 1;
        actor_status.objects[39].slot_state = 8;
    }
    if (cur_evt_state.unke[0x16] != 0) {
        actor_status.objects[24].slot_state = 8;
        actor_status.objects[40].slot_state = 1;
    } else {
        actor_status.objects[24].slot_state = 1;
        actor_status.objects[40].slot_state = 8;
    }
    if (cur_evt_state.unke[2] == 0 && cur_evt_state.unke[3] != 0) {
        map_cell_copy_rotated_fields(2, 0x4e, 0, 0x1b, 0x11, 1, 7, 0, 0x2f);
    } else {
        map_cell_copy_rotated_fields(2, 0x4f, 0, 0x1b, 0x11, 1, 7, 0, 0x2f);
    }
    if (cur_evt_state.unk32[0xa] == 1) {
        cur_evt_state.unk32[0xa] = 2;
        actor_status.objects[42].slot_state = 8;
        map_objects_slots.objects[276].arg.b.unk38 = 0x90;
    } else if (cur_evt_state.unk32[0xa] == 2) {
        actor_status.objects[42].slot_state = 8;
    }
    cur_evt_state.unk84[5] = 1;
}
