/*
 * FDAT_044.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, a trap handler (slot 3) and the load handler (slot 5).
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
void area_region_trigger_damage_player(MapObject *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_noop(void);
void area_on_load_restore_by_visit_count(void);

const FdatHeader fdat_header = {
    0x13,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_damage_player,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_restore_by_visit_count,
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
    0x33,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

/* Slot 0: area_interact_item_containers's shape with this module's message table. */
ADDRESS(0x801e8398, 0x22c)
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
                    } else if (obj->object_id == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->object_id == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->object_id == 0x12e || obj->object_id == 0x12f || obj->object_id == 0x130) {
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

ADDRESS(0x801e85c4, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e85cc, 0x8)
void area_use_item_noop(void)
{
}

ADDRESS(0x801e85d4, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e85dc, 0x8)
void area_on_actor_removed_noop(void)
{
}

/* Slot 3: objects 0x62 (when player_camera_within_map_region hits) and 0x63 hurt the player. */
ADDRESS(0x801e85e4, 0x9c)
void area_region_trigger_damage_player(MapObject *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0x62:
        if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                            obj->arg.b.unk39, obj->pos.vy + 3200) == 0) {
            break;
        }
        /* fall through */
    case 0x63:
        player_apply_damage(0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0);
        break;
    }
}

ADDRESS(0x801e8680, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8688, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8690, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8698, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e86a0, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e86a8, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e86b0, 0x14c)
void area_on_load_restore_by_visit_count(void)
{
    s32 state;

    if (cur_evt_state.unke[0xa] >= 2) {
        map_objects_slots.objects[7].object_id = 0xff;
        map_objects_slots.objects[49].object_id = 0xff;
        map_cell_copy_rotated_fields(2, 0x4e, 0, 0x21, 2, 2, 3, 0, 0x2f);
        map_cell_copy_rotated_fields(2, 0x4e, 3, 12, 0x48, 2, 3, 0, 0x2f);
        state = 8;
    } else {
        map_cell_copy_rotated_fields(2, 0x4c, 0, 0x21, 2, 2, 3, 0, 0x2f);
        map_cell_copy_rotated_fields(2, 0x4c, 3, 12, 0x48, 2, 3, 0, 0x2f);
        state = 1;
    }
    actor_status.objects[66].slot_state = state;
    actor_status.objects[70].slot_state = state;
    actor_status.objects[24].slot_state = state;
    actor_status.objects[43].slot_state = state;
    cur_evt_state.unk84[5] = 0xe;
}
