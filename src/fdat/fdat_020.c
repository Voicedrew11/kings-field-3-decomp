/*
 * FDAT_020.BIN (WIP module): the handler table and the switch table of its
 * slot 4 handler, the shared light-record blend, the light helper, its slot
 * 0 handler, empty handlers, a record handler (slot 19), a switch handler
 * (slot 3), an item check (slot 6), a record event handler (slot 4) and the
 * load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0xa0)

void area_set_light_blend(u8 n);
void area_handler_noop(void);
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle);
void area_update_noop(void);
void area_use_item_noop(void);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_make_persistent(Object88 *obj);
void area_region_trigger_worn_item_cells_and_warp_cells(Object44 *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_check_item_held(void);
void area_talk_event_give_items_and_grant_magic(Object88 *record, s32 event);
void area_on_load_restore_actors_doors_and_light(void);

const FdatHeader fdat_header = {
    0xb,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_worn_item_cells_and_warp_cells,
        /*  4 */ area_talk_event_give_items_and_grant_magic,
        /*  5 */ area_on_load_restore_actors_doors_and_light,
        /*  6 */ area_on_item_added_check_item_held,
        /*  7 */ area_handler_noop,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_handler_noop,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_make_persistent,
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
    0x2b,
};

#include "shared/light_blend_records.inc"

DATA(0x801e8ce8, 0x30, ".data")
static Record30 DAT_801e8ce8 = {
    {{{2500, -1250, 1250}, {-2500, -1250, -1250}, {0, 2500, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    9000,
    18000,
};
DATA(0x801e8d18, 0x30, ".data")
static Record30 DAT_801e8d18 = {
    {{{2000, -1000, 1000}, {-2000, -1000, -1000}, {0, 2000, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    9000,
    18000,
};
DATA(0x801e8d48, 0x30, ".data")
static Record30 DAT_801e8d48 = {
    {{{1500, -750, 750}, {-1500, -750, -750}, {0, 1500, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    9000,
    18000,
};
DATA(0x801e8d78, 0x30, ".data")
static Record30 DAT_801e8d78 = {
    {{{3000, -1500, 1500}, {-3000, -1500, -1500}, {0, 3000, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    18000,
    24000,
};

/* Blends area light records 42..44 by n sixteenths of the way to the lit state. */
ADDRESS(0x801e848c, 0xa8)
void area_set_light_blend(u8 n)
{
    s32 i;
    s32 t;

    for (i = 0, t = 0; i < n; i++) {
        t += 256;
    }
    light_blend_records(&DAT_801e8ce8, &DAT_801e8d78, &DAT_80081c8c[42], t);
    light_blend_records(&DAT_801e8d18, &DAT_801e8d78, &DAT_80081c8c[43], t);
    light_blend_records(&DAT_801e8d48, &DAT_801e8d78, &DAT_80081c8c[44], t);
}

ADDRESS(0x801e8534, 0x8)
void area_handler_noop(void)
{
}

/* Slot 0: area_interact_item_containers's shape with this module's message table. */
ADDRESS(0x801e853c, 0x210)
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        if ((def->unk0 == 9 || def->unk0 == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        notify_enqueue(obj->arg.b.unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7 ||
                        obj->unk6 == 0x133) {
                        notify_enqueue(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        notify_enqueue(0x1e);
                    } else if (obj->unk6 == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->unk6 == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->unk6 >= 190 && obj->unk6 < 198) {
                        notify_enqueue(8);
                    }
                }
            } else {
                link = &DAT_8018fb3c.objects[obj->arg.b.unk3a.index];
                if (link->unk6 == 0xff) {
                    goto message;
                }
                link->unk0 = obj->arg.b.unk40;
                link->arg.b.unk38 = 0xff;
                event_map_object_interact(link);
                link->unk0 = 0;
                link->arg.b.unk38 = 0;
                if (link->unk6 == 0xff) {
                    obj->arg.b.unk3a.index = 0xffff;
                }
            }
        }
    }
}

ADDRESS(0x801e874c, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e8754, 0x8)
void area_use_item_noop(void)
{
}

ADDRESS(0x801e875c, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

/* Slot 19: actor 3 is enabled again (state 1) whenever it is removed. */
ADDRESS(0x801e8764, 0x44)
void area_on_actor_removed_make_persistent(Object88 *obj)
{
    if (obj - DAT_80185da8.objects == 3) {
        obj->unk0 = 1;
    }
}

/*
 * Slot 3: region operand 0 copies one of two cell patterns by the armour in
 * player +0xf4 (item 0x4b or not); region 10 swaps the warp-arrival cells.
 */
ADDRESS(0x801e87a8, 0x13c)
void area_region_trigger_worn_item_cells_and_warp_cells(Object44 *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801b24e4.unkf4 == 0x4b) {
            map_cell_copy_rotated_fields(2, 0x4a, 5, 0x3a, 0x35, 6, 2, 0, 0x2f);
        } else {
            map_cell_copy_rotated_fields(2, 0x44, 5, 0x3a, 0x35, 6, 2, 0, 0x2f);
        }
        break;
    case 10:
        if (player_camera_within_map_region(0x1f, 0x41, 7, 0xc, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                map_cell_copy_rotated_fields(2, 0x4a, 4, 0x21, 0x41, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            map_cell_copy_rotated_fields(2, 0x4d, 4, 0x21, 0x41, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e88e4, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e88ec, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e88f4, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e88fc, 0x8)
void area_update_actor_target_noop(void)
{
}

/* Slot 6: sets flag 0x28 while item 0x83 is held. */
ADDRESS(0x801e8904, 0x30)
void area_on_item_added_check_item_held(void)
{
    if (game_counter_is_nonzero(0x83) > 0) {
        DAT_801ba984.unke[0x1a] = 1;
    }
}

/* Slot 4: script events 0..5 of an object record. */
ADDRESS(0x801e8934, 0x14c)
void area_talk_event_give_items_and_grant_magic(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        DAT_801ba984.unke[0x18] = 1;
        break;
    case 1:
        event_map_object_interact(NULL, 0x80);
        break;
    case 2:
        DAT_801ba984.unke[0x17] = 1;
        break;
    case 3:
        event_map_object_interact(NULL, 0x83);
        DAT_801ba984.unke[0x1a] = 1;
        break;
    case 4:
        color_overlay_transition(0x200, 0, 0, 0, 0x80, 0xc8, 0x80);
        color_overlay_transition(0x200, 0x80, 0xc8, 0x80, 0, 0, 0);
        set_accumulated_color_overlay(0, 0, 0, 0, 1);
        DAT_801ba984.unk2b = 1;
        DAT_801b77ec.unk0[56].unk0 = 1;
        DAT_801b77ec.unk0[58].unk0 = 1;
        DAT_801b77ec.unk0[60].unk0 = 1;
        DAT_801b77ec.unk0[62].unk0 = 1;
        DAT_801b77ec.unk0[64].unk0 = 1;
        DAT_801b77ec.unk0[76].unk0 = 1;
        break;
    case 5:
        event_map_object_interact(NULL, 10);
        break;
    }
}

/* Slot 5, called after the module loads: restores the records and the area light. */
ADDRESS(0x801e8a80, 0x268)
void area_on_load_restore_actors_doors_and_light(void)
{
    Object88 *obj;
    Object44 *obj44;
    s32 i;

    if (DAT_801b24e4.unk32 >= 50 && DAT_801b24e4.unk34[0] >= 50 && DAT_801b24e4.unk34[1] >= 50 &&
        DAT_801b24e4.unk34[2] >= 50 && DAT_801b24e4.unk34[3] >= 50 && DAT_801b24e4.unk34[4] >= 50) {
        DAT_801ba984.unke[0x1c] = 1;
    }
    if (DAT_801ba984.unke[0x19] == 0) {
        area_set_light_blend(0);
        obj44 = &DAT_8018fb3c.objects[121];
        obj44->unk0 = 0;
        DAT_8018fb3c.objects[121].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[173].arg.b.unk3d[1] = 0;
        DAT_80185da8.objects[3].unk0 = 8;
        obj = &DAT_80185da8.objects[4];
        DAT_80185da8.objects[0].unk0 = 1;
        DAT_80185da8.objects[1].unk0 = 8;
        for (i = 0x46; i != -1; i--, obj++) {
            obj->unk0 = 1;
        }
    } else if (DAT_801ba984.unk32[4] == 0) {
        DAT_801ba984.unke[0x1b] = 1;
        area_set_light_blend(16);
        obj44 = &DAT_8018fb3c.objects[121];
        obj44->unk0 = 0;
        DAT_8018fb3c.objects[121].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[173].arg.b.unk3d[1] = 0;
        obj44 = &DAT_8018fb3c.objects[91];
        obj44->unk0 = 0;
        DAT_80185da8.objects[3].unk0 = 1;
        obj = &DAT_80185da8.objects[4];
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 1;
        for (i = 0x46; i != -1; i--, obj++) {
            obj->unk0 = 8;
        }
    } else {
        area_set_light_blend(0);
        obj44 = &DAT_8018fb3c.objects[121];
        obj44->unk0 = 2;
        DAT_8018fb3c.objects[121].arg.b.unk3c = 0xff;
        DAT_8018fb3c.objects[173].arg.b.unk3d[1] = 0xff;
        obj44 = &DAT_8018fb3c.objects[91];
        obj44->unk0 = 0;
        DAT_80185da8.objects[3].unk0 = 8;
        obj = &DAT_80185da8.objects[4];
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 1;
        for (i = 0x46; i != -1; i--, obj++) {
            obj->unk0 = 1;
        }
    }
    if (DAT_801ba984.unk2c[0] == 0) {
        DAT_80185da8.objects[58].unk0 = 8;
    } else {
        obj44 = &DAT_8018fb3c.objects[83];
        obj44->unk0 = 0;
    }
    DAT_801ba984.unk84[5] = 6;
}
