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
void area_use_item_noop(void);
void area_adjust_actor_magic_damage_lethal_only(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9);
void area_on_actor_removed_unlock_door(Object88 *obj);
void area_region_trigger_set_doors_and_warp_cells(Object44 *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_check_items_held(void);
void area_on_movie_start_noop(void);
void area_on_movie_end_restore_hp_and_mp(u8 id);
void area_talk_event_collect_items_and_wait(Object88 *record, s32 event);
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
        /* 16 */ area_actor_target_check_noop,
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
 * Slot 1: drifts object 158's light blend (unk10) at random, a flicker; with
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
        DAT_8018fb3c.objects[k].unk10 += DAT_8009c21c * 16;
    }
    if (DAT_8018fb3c.objects[k].unk10 > 2048) {
        DAT_8009c21c = -DAT_8009c21c;
        if (DAT_8018fb3c.objects[k].unk10 >= 4096) {
            DAT_8018fb3c.objects[k].unk10 = 0;
        }
    }
    if (DAT_801ba984.unk4[1] != 0 && DAT_801ba984.unk4[2] != 0 && DAT_801ba984.unk4[7] == 0) {
        dx = (DAT_80185da8.objects[1].pos.vx - DAT_801b24e4.pos.vx) / 2048;
        dz = (DAT_80185da8.objects[1].pos.vz - DAT_801b24e4.pos.vz) / 2048;
        /* ^ is exclusive or here, as written. */
        if ((dx ^ 2 + dz ^ 2) > 400) {
            DAT_80185da8.objects[0].unk0 = 8;
            DAT_80185da8.objects[1].unk0 = 8;
            DAT_80185da8.objects[2].unk0 = 1;
        }
    }
}

ADDRESS(0x801e8704, 0x8)
void area_use_item_noop(void)
{
}

/* Slot 18: record 32 reports 0 while its unk1a is above the value. */
ADDRESS(0x801e870c, 0x5c)
void area_adjust_actor_magic_damage_lethal_only(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9)
{
    if (obj - DAT_80185da8.objects == 32 && *out < obj->unk1a) {
        *out = 0;
    }
}

/* Slot 19: actor 32 sets flag 0xc once and clears object 344's 0xf1 lock marker. */
ADDRESS(0x801e8768, 0x78)
void area_on_actor_removed_unlock_door(Object88 *obj)
{
    s32 k;

    if (obj - DAT_80185da8.objects == 32) {
        if (DAT_801ba984.unk4[8] == 0) {
            DAT_801ba984.unk4[8] = 1;
            k = 344;
            if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xf1) {
                DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
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
void area_region_trigger_set_doors_and_warp_cells(Object44 *obj)
{
    Object44 *door;

    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unk4[6] == 1) {
            if (DAT_801ba984.unk4[7] == 0) {
                DAT_80185da8.objects[0].unk0 = 8;
                DAT_80185da8.objects[1].unk0 = 8;
                DAT_80185da8.objects[2].unk0 = 8;
                door = &DAT_8018fb3c.objects[214];
                door->unk0 = 2;
                door->arg.b.unk3c = 0xff;
                door = &DAT_8018fb3c.objects[177];
                door->arg.b.unk3d[1] = 0xff;
                door = &DAT_8018fb3c.objects[111];
                door->unk0 = 0;
                door->arg.b.unk3c = 0;
                door = &DAT_8018fb3c.objects[182];
                door->arg.b.unk3d[1] = 0;
                door = &DAT_8018fb3c.objects[62];
                door->unk0 = 2;
                door->arg.b.unk38 = 0xff;
                door = &DAT_8018fb3c.objects[195];
                door->unk0 = 2;
                door->arg.b.unk38 = 0xff;
                door = &DAT_8018fb3c.objects[172];
                door->unk0 = 2;
                door->arg.b.unk38 = 0xff;
            }
        }
    case 10:
        if (player_camera_within_map_region(9, 0x37, 3, 3, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                map_cell_copy_rotated_fields(2, 0x4a, 0, 9, 0x37, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            map_cell_copy_rotated_fields(2, 0x4d, 0, 9, 0x37, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8960, 0x8)
void area_init_map_object_noop(void)
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
        DAT_801ba984.unk4[7] = 1;
    }
}

ADDRESS(0x801e89d0, 0x8)
void area_on_movie_start_noop(void)
{
}

/* Slot 7: event 0 restores the player's hp and raises unk1c by 30 up to its maximum. */
ADDRESS(0x801e89d8, 0x5c)
void area_on_movie_end_restore_hp_and_mp(u8 id)
{
    if (id == 0) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
        DAT_801b24e4.unk1c += 30;
        if (DAT_801b24e4.unk1c > DAT_801b24e4.unk1a) {
            DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
        }
    }
}

/* Slot 4: script events 0..5 of an object record. */
ADDRESS(0x801e8a34, 0x334)
void area_talk_event_collect_items_and_wait(Object88 *record, s32 event)
{
    s32 hundreds;

    switch (event) {
    case 0:
        if (game_counter_is_nonzero(2) > 0 && game_counter_is_nonzero(0x82) > 0 &&
            game_counter_is_nonzero(0x83) > 0 && game_counter_is_nonzero(0x84) > 0) {
            DAT_801ba984.unk4[3] = 1;
        } else {
            DAT_801ba984.unk4[3] = 0;
        }
        break;
    case 1:
        if (game_counter_is_nonzero(0x82) > 0 && game_counter_is_nonzero(0x83) > 0 &&
            game_counter_is_nonzero(0x84) > 0 && game_counter_is_nonzero(2) > 0) {
            DAT_801ba984.unk4[1] = 1;
            DAT_801ba984.unk0 = 1;
            game_counter_decrement(0x82);
            game_counter_decrement(0x83);
            game_counter_decrement(0x84);
            game_counter_decrement(2);
            if (DAT_801b24e4.unkcb == 2) {
                player_equip_weapon(0xff);
            }
        }
        break;
    case 2:
        DAT_801ba984.unk4[2] = 1;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 1;
        DAT_801ba984.unk4[4] = DAT_801b24e4.play_minutes % 60;
        DAT_801ba984.unk4[5] = DAT_801b24e4.experience % 10000 / 100;
        break;
    case 4:
        if (DAT_801b24e4.play_minutes % 60 < DAT_801ba984.unk4[4] ||
            DAT_801b24e4.play_minutes % 60 >= DAT_801ba984.unk4[4] + 10) {
            DAT_801ba984.unk4[6] = 1;
        }
        hundreds = DAT_801b24e4.experience % 10000 / 100;
        if (hundreds < DAT_801ba984.unk4[5] || hundreds >= DAT_801ba984.unk4[5] + 10) {
            DAT_801ba984.unk4[6] = 1;
        }
        break;
    case 5:
        if (game_counter_is_nonzero(0x82) > 0 && game_counter_is_nonzero(0x83) > 0 &&
            game_counter_is_nonzero(0x84) > 0) {
            DAT_801ba984.unk4[1] = 1;
        } else {
            DAT_801ba984.unk4[1] = 0;
        }
        break;
    }
}

/* Slot 5, called after the module loads: restores the records and doors of the flags. */
ADDRESS(0x801e8d68, 0x39c)
void area_on_load_restore_actors_and_doors(void)
{
    Object44 *door;
    s32 k;

    if (DAT_801ba984.unke[0xc] != 0) {
        k = 70;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        k = 33;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        k = 116;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
    }
    if (DAT_801ba984.unke[0x14] == 0) {
        DAT_80185da8.objects[0].unk0 = 1;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        k = 214;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 177;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        k = 111;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 182;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        k = 62;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        k = 195;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        k = 172;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
    } else if (DAT_801ba984.unk4[2] == 0) {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 1;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        k = 214;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 177;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        k = 111;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 182;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        k = 62;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        k = 195;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        k = 172;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
    } else if (DAT_801ba984.unk4[6] == 0) {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 1;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        k = 214;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 177;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        k = 111;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 182;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        k = 62;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        k = 195;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        k = 172;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
    } else if (DAT_801ba984.unk4[7] == 0) {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 2;
        k = 214;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0xff;
        k = 177;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0xff;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        k = 111;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 182;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 2;
        k = 62;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 2;
        k = 195;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 2;
        k = 172;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
    } else {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        k = 214;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0;
        k = 177;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 2;
        k = 111;
        DAT_8018fb3c.objects[k].arg.b.unk3c = 0xff;
        k = 182;
        DAT_8018fb3c.objects[k].arg.b.unk3d[1] = 0xff;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        k = 62;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        k = 195;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        k = 172;
        DAT_8018fb3c.objects[k].arg.b.unk38 = 0;
    }
    if (game_counter_is_nonzero(2) > 0 && game_counter_is_nonzero(0x82) > 0 &&
        game_counter_is_nonzero(0x83) > 0 && game_counter_is_nonzero(0x84) > 0) {
        DAT_801ba984.unk4[3] = 1;
    } else {
        DAT_801ba984.unk4[3] = 0;
    }
    if (DAT_801ba984.unk4[8] != 0) {
        k = 344;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xf1) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
    }
    k = 158;
    if (DAT_8018fb3c.objects[k].unk5 == 0xff) {
        DAT_8018fb3c.objects[k].unk5 = 0x3f;
        DAT_8018fb3c.objects[k].unk2 = 7;
        DAT_8018fb3c.objects[k].unk10 = 0x800;
    }
    DAT_801ba984.unk84[5] = 0;
}
