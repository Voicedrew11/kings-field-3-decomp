/*
 * FDAT_002.BIN (WIP module): the handler table and the switch table of its
 * slot 4 handler, an empty default handler, the shared slot 0 handler (this
 * module's copy is the claim), a light flicker and actor swap (slot 1),
 * actor handlers (slots 18 and 19), a region-trigger handler (slot 3), an
 * item check (slot 6), a movie handler (slot 7), a talk event handler
 * (slot 4) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0xa0)

void area_handler_noop(void);
void area_update_light_flicker_and_actor_swap(void);
void area_use_item_noop(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_lethal_only(Actor *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9);
void area_on_actor_removed_unlock_door(Actor *obj);
void area_region_trigger_set_doors_and_warp_cells(MapObject *obj);
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_check_items_held(void);
void area_on_movie_start_noop(u8 movie_id);
void area_on_movie_end_restore_hp_and_mp(u8 id);
void area_talk_event_collect_items_and_wait(Actor *record, s32 event);
void area_on_load_restore_actors_and_doors(void);

const FdatHeader fdat_header = {
    0x5,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_light_flicker_and_actor_swap,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_set_doors_and_warp_cells,
        /*  4 */ area_talk_event_collect_items_and_wait,
        /*  5 */ area_on_load_restore_actors_and_doors,
        /*  6 */ area_on_item_added_check_items_held,
        /*  7 */ area_on_movie_end_restore_hp_and_mp,
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
        /* 18 */ area_adjust_actor_magic_damage_lethal_only,
        /* 19 */ area_on_actor_removed_unlock_door,
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
    0x25,
};

ADDRESS(0x801e83a8, 0x8)
void area_handler_noop(void)
{
}

#include "shared/area_interact_item_containers.inc"

/*
 * Slot 1: drifts object 158's light blend (lighting_blend) at random, a flicker; with
 * flags 5 and 6 set and 0xb clear, once actor 1 is far from the player it
 * disables actors 0 and 1 (state 8) and enables actor 2.
 */
ADDRESS(0x801e85b8, 0x14c)
void area_update_light_flicker_and_actor_swap(void)
{
    s32 k = 158;
    s32 dx;
    s32 dz;

    if (rand() < 8192) {
        map_objects_slots.objects[k].lighting_blend += light_flicker_step * 16;
    }
    if (map_objects_slots.objects[k].lighting_blend > 2048) {
        light_flicker_step = -light_flicker_step;
        if (map_objects_slots.objects[k].lighting_blend >= 4096) {
            map_objects_slots.objects[k].lighting_blend = 0;
        }
    }
    if (cur_evt_state.flags[1] != 0 && cur_evt_state.flags[2] != 0 && cur_evt_state.flags[7] == 0) {
        dx = (actor_status.objects[1].pos.vx - cur_avatar_status.pos.vx) / 2048;
        dz = (actor_status.objects[1].pos.vz - cur_avatar_status.pos.vz) / 2048;
        /* ^ is exclusive or here, as written. */
        if ((dx ^ 2 + dz ^ 2) > 400) {
            actor_status.objects[0].slot_state = 8;
            actor_status.objects[1].slot_state = 8;
            actor_status.objects[2].slot_state = 1;
        }
    }
}

ADDRESS(0x801e8704, 0x8)
void area_use_item_noop(VECTOR *pos, SVECTOR *angle, s32 item)
{
}

/* Slot 18: record 32 reports 0 while its health is above the value. */
ADDRESS(0x801e870c, 0x5c)
void area_adjust_actor_magic_damage_lethal_only(Actor *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9)
{
    if (obj - actor_status.objects == 32 && *out < obj->health) {
        *out = 0;
    }
}

/* Slot 19: actor 32 sets flag 0xc once and clears object 344's 0xf1 lock marker. */
ADDRESS(0x801e8768, 0x78)
void area_on_actor_removed_unlock_door(Actor *obj)
{
    s32 k;

    if (obj - actor_status.objects == 32) {
        if (cur_evt_state.flags[8] == 0) {
            cur_evt_state.flags[8] = 1;
            k = 344;
            if (map_objects_slots.objects[k].arg.b.unk38 == 0xf1) {
                map_objects_slots.objects[k].arg.b.unk38 = 0xff;
            }
        }
    }
}

/*
 * Slot 3: region operand 0 (with flag 0xa set and 0xb clear) disables actors
 * 0..2 and sets the doors, then, like region 10, copies the warp-arrival or
 * the normal cell pattern.
 */
ADDRESS(0x801e87e0, 0x180)
void area_region_trigger_set_doors_and_warp_cells(MapObject *obj)
{
    MapObject *door;

    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (cur_evt_state.flags[6] == 1) {
            if (cur_evt_state.flags[7] == 0) {
                actor_status.objects[0].slot_state = 8;
                actor_status.objects[1].slot_state = 8;
                actor_status.objects[2].slot_state = 8;
                door = &map_objects_slots.objects[214];
                door->layer_mask = 2;
                door->arg.b.unk3c = 0xff;
                door = &map_objects_slots.objects[177];
                door->arg.b.unk3d[1] = 0xff;
                door = &map_objects_slots.objects[111];
                door->layer_mask = 0;
                door->arg.b.unk3c = 0;
                door = &map_objects_slots.objects[182];
                door->arg.b.unk3d[1] = 0;
                door = &map_objects_slots.objects[62];
                door->layer_mask = 2;
                door->arg.b.unk38 = 0xff;
                door = &map_objects_slots.objects[195];
                door->layer_mask = 2;
                door->arg.b.unk38 = 0xff;
                door = &map_objects_slots.objects[172];
                door->layer_mask = 2;
                door->arg.b.unk38 = 0xff;
            }
        }
    case 10:
        if (player_camera_within_map_region(9, 0x37, 3, 3, 0x8000) != 0) {
            if (cur_avatar_status.warp_arrived != 0) {
                map_cell_copy_rotated_fields(2, 0x4a, 0, 9, 0x37, 3, 1, 0, 0x2f);
            }
        } else {
            cur_avatar_status.warp_arrived = 0;
            map_cell_copy_rotated_fields(2, 0x4d, 0, 9, 0x37, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8960, 0x8)
void area_init_map_object_noop(MapObject *obj, MapObjectDef *def)
{
}

ADDRESS(0x801e8968, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8970, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8978, 0x8)
void area_update_actor_target_noop(void)
{
}

/* Slot 6: sets flag 0xb once items 0, 0x85 and 0x64 are all held. */
ADDRESS(0x801e8980, 0x50)
void area_on_item_added_check_items_held(void)
{
    if (game_counter_is_nonzero(0) > 0 && game_counter_is_nonzero(0x85) > 0 &&
        game_counter_is_nonzero(0x64) > 0) {
        cur_evt_state.flags[7] = 1;
    }
}

ADDRESS(0x801e89d0, 0x8)
void area_on_movie_start_noop(u8 movie_id)
{
}

/* Slot 7: event 0 restores the player's hp and raises mp by 30 up to its maximum. */
ADDRESS(0x801e89d8, 0x5c)
void area_on_movie_end_restore_hp_and_mp(u8 id)
{
    if (id == 0) {
        cur_avatar_status.hp = cur_avatar_status.hp_max;
        cur_avatar_status.mp += 30;
        if (cur_avatar_status.mp > cur_avatar_status.mp_max) {
            cur_avatar_status.mp = cur_avatar_status.mp_max;
        }
    }
}

/* Slot 4: script events 0..5 of an object record. */
ADDRESS(0x801e8a34, 0x334)
void area_talk_event_collect_items_and_wait(Actor *record, s32 event)
{
    s32 hundreds;

    switch (event) {
    case 0:
        if (game_counter_is_nonzero(2) > 0 && game_counter_is_nonzero(0x82) > 0 &&
            game_counter_is_nonzero(0x83) > 0 && game_counter_is_nonzero(0x84) > 0) {
            cur_evt_state.flags[3] = 1;
        } else {
            cur_evt_state.flags[3] = 0;
        }
        break;
    case 1:
        if (game_counter_is_nonzero(0x82) > 0 && game_counter_is_nonzero(0x83) > 0 &&
            game_counter_is_nonzero(0x84) > 0 && game_counter_is_nonzero(2) > 0) {
            cur_evt_state.flags[1] = 1;
            cur_evt_state.interaction_handled = 1;
            game_counter_decrement(0x82);
            game_counter_decrement(0x83);
            game_counter_decrement(0x84);
            game_counter_decrement(2);
            if (cur_avatar_status.equipped_weapon_id == 2) {
                player_equip_weapon(0xff);
            }
        }
        break;
    case 2:
        cur_evt_state.flags[2] = 1;
        actor_status.objects[1].slot_state = 8;
        actor_status.objects[2].slot_state = 1;
        cur_evt_state.flags[4] = cur_avatar_status.play_minutes % 60;
        cur_evt_state.flags[5] = cur_avatar_status.experience % 10000 / 100;
        break;
    case 4:
        if (cur_avatar_status.play_minutes % 60 < cur_evt_state.flags[4] ||
            cur_avatar_status.play_minutes % 60 >= cur_evt_state.flags[4] + 10) {
            cur_evt_state.flags[6] = 1;
        }
        hundreds = cur_avatar_status.experience % 10000 / 100;
        if (hundreds < cur_evt_state.flags[5] || hundreds >= cur_evt_state.flags[5] + 10) {
            cur_evt_state.flags[6] = 1;
        }
        break;
    case 5:
        if (game_counter_is_nonzero(0x82) > 0 && game_counter_is_nonzero(0x83) > 0 &&
            game_counter_is_nonzero(0x84) > 0) {
            cur_evt_state.flags[1] = 1;
        } else {
            cur_evt_state.flags[1] = 0;
        }
        break;
    }
}

/* Slot 5, called after the module loads: restores the records and doors of the flags. */
ADDRESS(0x801e8d68, 0x39c)
void area_on_load_restore_actors_and_doors(void)
{
    MapObject *door;
    s32 k;

    if (cur_evt_state.unke[0xc] != 0) {
        k = 70;
        if (map_objects_slots.objects[k].arg.b.unk38 == 0xc8) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
        k = 33;
        if (map_objects_slots.objects[k].arg.b.unk38 == 0xc8) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
        k = 116;
        if (map_objects_slots.objects[k].arg.b.unk38 == 0xc8) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
    }
    if (cur_evt_state.unke[0x14] == 0) {
        actor_status.objects[0].slot_state = 1;
        actor_status.objects[1].slot_state = 8;
        actor_status.objects[2].slot_state = 8;
        door = &map_objects_slots.objects[214];
        door->layer_mask = 0;
        k = 214;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 177;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[111];
        door->layer_mask = 0;
        k = 111;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 182;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[62];
        door->layer_mask = 0;
        k = 62;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[195];
        door->layer_mask = 0;
        k = 195;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[172];
        door->layer_mask = 0;
        k = 172;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
    } else if (cur_evt_state.flags[2] == 0) {
        actor_status.objects[0].slot_state = 8;
        actor_status.objects[1].slot_state = 1;
        actor_status.objects[2].slot_state = 8;
        door = &map_objects_slots.objects[214];
        door->layer_mask = 0;
        k = 214;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 177;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[111];
        door->layer_mask = 0;
        k = 111;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 182;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[62];
        door->layer_mask = 0;
        k = 62;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[195];
        door->layer_mask = 0;
        k = 195;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[172];
        door->layer_mask = 0;
        k = 172;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
    } else if (cur_evt_state.flags[6] == 0) {
        actor_status.objects[0].slot_state = 8;
        actor_status.objects[1].slot_state = 8;
        actor_status.objects[2].slot_state = 1;
        door = &map_objects_slots.objects[214];
        door->layer_mask = 0;
        k = 214;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 177;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[111];
        door->layer_mask = 0;
        k = 111;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 182;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[62];
        door->layer_mask = 0;
        k = 62;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[195];
        door->layer_mask = 0;
        k = 195;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[172];
        door->layer_mask = 0;
        k = 172;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
    } else if (cur_evt_state.flags[7] == 0) {
        actor_status.objects[0].slot_state = 8;
        actor_status.objects[1].slot_state = 8;
        actor_status.objects[2].slot_state = 8;
        door = &map_objects_slots.objects[214];
        door->layer_mask = 2;
        k = 214;
        map_objects_slots.objects[k].arg.b.unk3c = 0xff;
        k = 177;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0xff;
        door = &map_objects_slots.objects[111];
        door->layer_mask = 0;
        k = 111;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 182;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[62];
        door->layer_mask = 2;
        k = 62;
        map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        door = &map_objects_slots.objects[195];
        door->layer_mask = 2;
        k = 195;
        map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        door = &map_objects_slots.objects[172];
        door->layer_mask = 2;
        k = 172;
        map_objects_slots.objects[k].arg.b.unk38 = 0xff;
    } else {
        actor_status.objects[0].slot_state = 8;
        actor_status.objects[1].slot_state = 8;
        actor_status.objects[2].slot_state = 8;
        door = &map_objects_slots.objects[214];
        door->layer_mask = 0;
        k = 214;
        map_objects_slots.objects[k].arg.b.unk3c = 0;
        k = 177;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0;
        door = &map_objects_slots.objects[111];
        door->layer_mask = 2;
        k = 111;
        map_objects_slots.objects[k].arg.b.unk3c = 0xff;
        k = 182;
        map_objects_slots.objects[k].arg.b.unk3d[1] = 0xff;
        door = &map_objects_slots.objects[62];
        door->layer_mask = 0;
        k = 62;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[195];
        door->layer_mask = 0;
        k = 195;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
        door = &map_objects_slots.objects[172];
        door->layer_mask = 0;
        k = 172;
        map_objects_slots.objects[k].arg.b.unk38 = 0;
    }
    if (game_counter_is_nonzero(2) > 0 && game_counter_is_nonzero(0x82) > 0 &&
        game_counter_is_nonzero(0x83) > 0 && game_counter_is_nonzero(0x84) > 0) {
        cur_evt_state.flags[3] = 1;
    } else {
        cur_evt_state.flags[3] = 0;
    }
    if (cur_evt_state.flags[8] != 0) {
        k = 344;
        if (map_objects_slots.objects[k].arg.b.unk38 == 0xf1) {
            map_objects_slots.objects[k].arg.b.unk38 = 0xff;
        }
    }
    k = 158;
    if (map_objects_slots.objects[k].lighting_override_index == 0xff) {
        map_objects_slots.objects[k].lighting_override_index = 0x3f;
        map_objects_slots.objects[k].render_mode = 7;
        map_objects_slots.objects[k].lighting_blend = 0x800;
    }
    cur_evt_state.unk84[5] = 0;
}
