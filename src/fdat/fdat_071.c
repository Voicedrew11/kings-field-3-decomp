/*
 * FDAT_071.BIN (WIP module): the handler table, its slot 0 handler, a key
 * handler (slot 2), empty handlers and the load handler (slot 5).
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
void area_use_item_place_in_socket(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_noop(void);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_noop(void);
void area_on_load_store_area_index(void);

const FdatHeader fdat_header = {
    0x1c,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_place_in_socket,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_store_area_index,
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
    0x3c,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

/* Slot 0: area_interact_item_containers's shape with this module's message table. */
ADDRESS(0x801e8398, 0x24c)
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
                    } else if (obj->object_id == 0x145) {
                        notify_enqueue(0x1f);
                    } else if (obj->object_id == 0x146) {
                        notify_enqueue(0x20);
                    } else if (obj->object_id == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->object_id == 0xba || obj->object_id == 0x142 || obj->object_id == 0x143 || obj->object_id == 0x144) {
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

ADDRESS(0x801e85e4, 0x8)
void area_update_noop(void)
{
}

/* Slot 2: item 0x86 used on object 0x13f in front unlocks it. */
ADDRESS(0x801e85ec, 0xf8)
void area_use_item_place_in_socket(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;
    MapObject *obj;

    if (item != 0x86) {
        return;
    }
    i = map_object_find_interaction_target(0, pos, 800, 0x6a4, angle->vy, 0x200);
    if (i == -1) {
        return;
    }
    obj = &map_objects_slots.objects[i];
    if (obj->object_id == 0x13f) {
        if (obj->arg.b.unk38 != 0xff) {
            return;
        }
        obj->arg.b.unk38 = item;
        obj->action_state = 0;
        cur_evt_state.interaction_handled = 1;
        game_counter_decrement(0x86);
        event_spawn_effect_object(obj, 0x86);
    } else if (map_object_check_and_consume_marker(obj, 0x86) == 3) {
        notify_enqueue(7);
        cur_evt_state.interaction_handled = 1;
    }
}

ADDRESS(0x801e86e4, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e86ec, 0x8)
void area_on_actor_removed_noop(void)
{
}

ADDRESS(0x801e86f4, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e86fc, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8704, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e870c, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8714, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e871c, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e8724, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e872c, 0x14)
void area_on_load_store_area_index(void)
{
    cur_evt_state.unk84[5] = 0x17;
}
