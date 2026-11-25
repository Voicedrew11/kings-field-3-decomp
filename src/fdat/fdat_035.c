/*
 * FDAT_035.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, a record handler (slot 19), a warp-arrival cell handler (slot 3) and the load
 * handler (slot 5).
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
void area_on_actor_removed_release_container_item(Actor *obj);
void area_region_trigger_update_warp_arrival_cells(MapObject *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_noop(void);
void area_on_load_restore_container(void);

const FdatHeader fdat_header = {
    0x10,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_update_warp_arrival_cells,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_restore_container,
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
        /* 19 */ area_on_actor_removed_release_container_item,
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
    0x30,
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
                    } else if (obj->object_id == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->object_id == 0xba || obj->object_id == 0x12d || obj->object_id == 0x12e) {
                        notify_enqueue(0x22);
                    } else if (obj->object_id == 0x12f) {
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

/* Slot 19: record 3 disarms object 20 and triggers its link. */
ADDRESS(0x801e85dc, 0xa4)
void area_on_actor_removed_release_container_item(Actor *obj)
{
    MapObject *link;

    if (obj - actor_status.objects == 3) {
        cur_evt_state.unk32[0xe] = 1;
        map_objects_slots.objects[20].layer_mask = 0;
        map_objects_slots.objects[20].arg.b.unk3c = 0;
        if (map_objects_slots.objects[20].arg.b.unk3a.index != 0xffff) {
            link = &map_objects_slots.objects[map_objects_slots.objects[20].arg.b.unk3a.index];
            if (link->object_id != 0xff) {
                link->layer_mask = 2;
                link->arg.b.unk38 = 0xff;
            }
        }
        map_objects_slots.objects[33].arg.b.unk3d[1] = 0;
    }
}

/*
 * Slot 3: region operand 10 (unk3a.bytes[1]): while the player is on the warp
 * arrival pad copies the arrival cell pattern, else clears the arrival flag
 * (player +0x10a) and copies the normal pattern back.
 */
ADDRESS(0x801e8680, 0xc0)
void area_region_trigger_update_warp_arrival_cells(MapObject *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(0x39, 0x21, 3, 3, 0x8000) != 0) {
        if (cur_avatar_status.unk10a != 0) {
            map_cell_copy_rotated_fields(2, 0x4a, 0, 0x39, 0x23, 3, 1, 0, 0x2f);
        }
    } else {
        cur_avatar_status.unk10a = 0;
        map_cell_copy_rotated_fields(2, 0x4d, 0, 0x39, 0x23, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e8740, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8748, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8750, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8758, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8760, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e8768, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8770, 0xb4)
void area_on_load_restore_container(void)
{
    MapObject *link;

    if (cur_evt_state.unk32[0xe] == 0) {
        map_objects_slots.objects[20].layer_mask = 2;
        map_objects_slots.objects[20].arg.b.unk3c = 0xff;
        map_objects_slots.objects[33].arg.b.unk3d[1] = 0xff;
    } else {
        map_objects_slots.objects[20].layer_mask = 0;
        map_objects_slots.objects[20].arg.b.unk3c = 0;
        if (map_objects_slots.objects[20].arg.b.unk3a.index != 0xffff) {
            link = &map_objects_slots.objects[map_objects_slots.objects[20].arg.b.unk3a.index];
            if (link->object_id != 0xff) {
                link->layer_mask = 2;
                link->arg.b.unk38 = 0xff;
                map_objects_slots.objects[20].arg.b.unk3a.index = 0xffff;
            }
        }
        map_objects_slots.objects[33].arg.b.unk3d[1] = 0;
    }
    cur_evt_state.unk84[5] = 0xb;
}
