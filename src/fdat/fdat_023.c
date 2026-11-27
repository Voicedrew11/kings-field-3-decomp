/*
 * FDAT_023.BIN (WIP module): the handler table, the shared slot 0 handler,
 * a key handler (slot 2), a warp-arrival cell handler (slot 3), object handlers (slots 8
 * and 9), an event handler (slot 4), the load handler (slot 5) and the two
 * offsets slot 9 passes.
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
void area_use_item_place_in_socket(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7);
void area_on_actor_removed_noop(Actor *obj);
void area_region_trigger_update_warp_arrival_cells(MapObject *obj);
void area_init_map_object_socket(MapObject *obj, MapObjectDef *def);
void area_update_map_object_socket(void);
void func_801e8884(void);
void func_801e888c(void);
void func_801e8894(void);
void func_801e889c(void);
void func_801e88a4(void);
void func_801e88ac(void);
void func_801e88b4(void);
void func_801e88bc(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_set_flag(Actor *obj, s32 event);
void area_on_load_store_area_index(void);

const FdatHeader fdat_header = {
    0xc,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_place_in_socket,
        /*  3 */ area_region_trigger_update_warp_arrival_cells,
        /*  4 */ area_talk_event_set_flag,
        /*  5 */ area_on_load_store_area_index,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ (FdatMovieHandler)area_handler_noop,
        /*  8 */ area_init_map_object_socket,
        /*  9 */ area_update_map_object_socket,
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
    0x2c,
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

/* Slot 2: item 0x85 used on object 0x140 in front unlocks it. */
ADDRESS(0x801e85a8, 0x13c)
void area_use_item_place_in_socket(VECTOR *pos, SVECTOR *angle, s32 item)
{
    VECTOR unused;
    SVECTOR unused_angle;
    s32 i;
    MapObject *obj;

    if (item != 0x85) {
        return;
    }
    i = map_object_find_interaction_target(0, pos, 800, 0x6a4, angle->vy, 0x200);
    if (i != -1) {
        obj = &map_objects_slots.objects[i];
        if (obj->object_id == 0x140) {
            if (angle_within_tolerance(cur_avatar_status.camera_rotation_target.vy + 2048, obj->angle.vy,
                                       0x100) == 0) {
                return;
            }
            if (obj->arg.b.unk38 != 0xff) {
                return;
            }
            obj->arg.b.unk38 = item;
            obj->action_state = 0;
            cur_evt_state.interaction_handled = 1;
            game_counter_decrement(0x85);
            event_spawn_effect_object(obj, 0x85);
        } else if (map_object_check_and_consume_marker(obj, 0x85) == 3) {
            notify_enqueue(7);
            cur_evt_state.interaction_handled = 1;
        }
    } else {
        cur_evt_state.interaction_handled = 1;
        notify_enqueue(0x23);
    }
}

ADDRESS(0x801e86e4, 0x8)
void area_adjust_actor_magic_damage_noop(Actor *obj, s32 *damage, s32 component0, s32 component1,
                                         s32 component2, s32 component3, s32 component4, s32 component5,
                                         s32 component6, s32 component7)
{
}

ADDRESS(0x801e86ec, 0x8)
void area_on_actor_removed_noop(Actor *obj)
{
}

/*
 * Slot 3: region operand 10 (unk3a.bytes[1]): while the player is on the warp
 * arrival pad copies the arrival cell pattern, else clears the arrival flag
 * (player +0x10a) and copies the normal pattern back.
 */
ADDRESS(0x801e86f4, 0xc0)
void area_region_trigger_update_warp_arrival_cells(MapObject *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(0x12, 0x1c, 3, 3, 0x8000) != 0) {
        if (cur_avatar_status.warp_arrived != 0) {
            map_cell_copy_rotated_fields(2, 0x15, 0x4f, 0x12, 0x1e, 3, 1, 0, 0x2f);
        }
    } else {
        cur_avatar_status.warp_arrived = 0;
        map_cell_copy_rotated_fields(2, 0x18, 0x4f, 0x12, 0x1e, 3, 1, 0, 0x2f);
    }
}

/* Slot 8: an object made from definition kind 0xa7 takes that kind. */
ADDRESS(0x801e87b4, 0x1c)
void area_init_map_object_socket(MapObject *obj, MapObjectDef *def)
{
    if (def->action == 0xa7) {
        obj->action = 0xa7;
    }
}

DATA(0x801e8908, 0x8, ".data")
static SVECTOR area_socket_motion_start_offset = {0, -1600, 0x400, 0};
DATA(0x801e8910, 0x8, ".data")
static SVECTOR area_socket_motion_end_offset = {0, -1600, 0x100, 0};

/* Slot 9: when the tested kind 0xa7 object reaches its target, frees every 0xe8 object. */
ADDRESS(0x801e87d0, 0xb4)
void area_update_map_object_socket(void)
{
    MapObject *obj = map_objects_slots.current;
    MapObject *other;
    s32 i;

    if (obj->action != 0xa7) {
        return;
    }
    if (map_object_step_offset_motion(obj, &map_objects_slots.objects[380 + obj->arg.b.unk39],
                                      &area_socket_motion_start_offset, &area_socket_motion_end_offset, 0,
                                      20) == 0) {
        return;
    }
    other = map_objects_slots.objects;
    for (i = 0x18c; i != 0; i--, other++) {
        if (other->action == 0xe8) {
            other->animation_id = 0x81;
            other->arg.b.unk40 = 0;
        }
    }
    cur_evt_state.interaction_handled = 1;
}

ADDRESS(0x801e8884, 0x8)
void func_801e8884(void)
{
}

ADDRESS(0x801e888c, 0x8)
void func_801e888c(void)
{
}

ADDRESS(0x801e8894, 0x8)
void func_801e8894(void)
{
}

ADDRESS(0x801e889c, 0x8)
void func_801e889c(void)
{
}

ADDRESS(0x801e88a4, 0x8)
void func_801e88a4(void)
{
}

ADDRESS(0x801e88ac, 0x8)
void func_801e88ac(void)
{
}

ADDRESS(0x801e88b4, 0x8)
void func_801e88b4(void)
{
}

ADDRESS(0x801e88bc, 0x8)
void func_801e88bc(void)
{
}

ADDRESS(0x801e88c4, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e88cc, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e88d4, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 4: script event 0 of an object record sets flag 2. */
ADDRESS(0x801e88dc, 0x18)
void area_talk_event_set_flag(Actor *obj, s32 event)
{
    if (event == 0) {
        cur_evt_state.unk2c[2] = 1;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e88f4, 0x14)
void area_on_load_store_area_index(void)
{
    cur_evt_state.unk84[5] = 7;
}
