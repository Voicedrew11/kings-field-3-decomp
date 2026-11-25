/*
 * FDAT_068.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, an event handler (slot 4) and the load handler (slot 5).
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
void area_update_noop(void);
void area_use_item_noop(void);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_noop(void);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_set_flags(Actor *record, s32 event);
void area_on_load_restore_actor_and_door(void);

const FdatHeader fdat_header = {
    0x1b,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_set_flags,
        /*  5 */ area_on_load_restore_actor_and_door,
        /*  6 */ area_on_item_added_noop,
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
    0x3b,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

/* Slot 0: area_interact_item_containers's shape with this module's message table. */
ADDRESS(0x801e8398, 0x25c)
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    MapObject *obj;
    MapObject *link;
    MapObjectDef *def;

    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &map_objects_slots.objects[i];
        def = &map_objects_slots.defs[obj->object_id];
        cur_evt_state.unk0 = 1;
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
                    } else if (obj->object_id == 0x141) {
                        notify_enqueue(0x1f);
                    } else if (obj->object_id == 0x142) {
                        notify_enqueue(0x20);
                    } else if (obj->object_id == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->object_id == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->object_id == 0x13c || obj->object_id == 0x13d || obj->object_id == 0x13e) {
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

ADDRESS(0x801e85f4, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e85fc, 0x8)
void area_use_item_noop(void)
{
}

ADDRESS(0x801e8604, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e860c, 0x8)
void area_on_actor_removed_noop(void)
{
}

ADDRESS(0x801e8614, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e861c, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8624, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e862c, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8634, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e863c, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 4: script events 0 and 1 of an object record set flags 0x2d and 0x2e. */
ADDRESS(0x801e8644, 0x3c)
void area_talk_event_set_flags(Actor *record, s32 event)
{
    switch (event) {
    case 0:
        cur_evt_state.unk32[0x2d] = 1;
        break;
    case 1:
        cur_evt_state.unk32[0x2e] = 1;
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8680, 0x7c)
void area_on_load_restore_actor_and_door(void)
{
    if (cur_evt_state.unk32[0x2e] == 0) {
        actor_status.objects[25].slot_state = 1;
        map_objects_slots.objects[28].layer_mask = 0;
        map_objects_slots.objects[28].arg.b.unk3c = 0;
        map_objects_slots.objects[95].arg.b.unk3d[1] = 0;
    } else {
        actor_status.objects[25].slot_state = 8;
        map_objects_slots.objects[28].layer_mask = 2;
        map_objects_slots.objects[28].arg.b.unk3c = 0xff;
        map_objects_slots.objects[95].arg.b.unk3d[1] = 0xff;
    }
    cur_evt_state.unk84[5] = 0x16;
}
