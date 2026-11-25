/*
 * FDAT_038.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, records 50..52 handlers (slots 18 and 19) and the load handler
 * (slot 5).
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
void area_adjust_actor_magic_damage_element_kill(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                 s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9);
void area_on_actor_removed_send_signals(Object88 *obj);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_noop(void);
void area_on_load_clear_signalled_markers(void);

const FdatHeader fdat_header = {
    0x11,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_clear_signalled_markers,
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
        /* 18 */ area_adjust_actor_magic_damage_element_kill,
        /* 19 */ area_on_actor_removed_send_signals,
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
    0x31,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

/* Slot 0: area_interact_item_containers's shape with this module's message table. */
ADDRESS(0x801e8398, 0x238)
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &map_objects_slots.objects[i];
        def = &map_objects_slots.defs[obj->unk6];
        cur_evt_state.unk0 = 1;
        if ((def->unk0 == 9 || def->unk0 == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        notify_enqueue(obj->arg.b.unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7) {
                        notify_enqueue(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        notify_enqueue(0x1e);
                    } else if (obj->unk6 == 0x132) {
                        notify_enqueue(0x1f);
                    } else if (obj->unk6 == 0x133) {
                        notify_enqueue(0x20);
                    } else if (obj->unk6 == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->unk6 == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->unk6 >= 190 && obj->unk6 < 198) {
                        notify_enqueue(8);
                    }
                }
            } else {
                link = &map_objects_slots.objects[obj->arg.b.unk3a.index];
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

ADDRESS(0x801e85d0, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e85d8, 0x8)
void area_use_item_noop(void)
{
}

/* Slot 18: records 50..52 report their unk1a while the last four arguments are 0. */
ADDRESS(0x801e85e0, 0x8c)
void area_adjust_actor_magic_damage_element_kill(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4,
                                                 s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9)
{
    s32 index = obj - actor_status.objects;

    if (index == 50 || index == 51 || index == 52) {
        if (arg6 == 0 && arg7 == 0 && arg8 == 0 && arg9 == 0) {
            *out = obj->unk1a;
        } else {
            *out = 0;
        }
    }
}

/* Slot 19: records 50..52 each set their flag and start their object once. */
ADDRESS(0x801e866c, 0xf0)
void area_on_actor_removed_send_signals(Object88 *obj)
{
    s32 index = obj - actor_status.objects;

    switch (index) {
    case 50:
        if (cur_evt_state.unk32[0x10] == 0) {
            cur_evt_state.unk32[0x10] = 1;
            map_object_apply_marker_signal(0x64);
        }
        break;
    case 51:
        if (cur_evt_state.unk32[0x11] == 0) {
            cur_evt_state.unk32[0x11] = 1;
            map_object_apply_marker_signal(0x65);
        }
        break;
    case 52:
        if (cur_evt_state.unk32[0x12] == 0) {
            cur_evt_state.unk32[0x12] = 1;
            map_object_apply_marker_signal(0x66);
        }
        break;
    }
}

ADDRESS(0x801e875c, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e8764, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e876c, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8774, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e877c, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8784, 0x8)
void area_on_item_added_noop(void)
{
}

ADDRESS(0x801e878c, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8794, 0xa8)
void area_on_load_clear_signalled_markers(void)
{
    s32 k;

    k = 241;
    if (cur_evt_state.unk32[0x10] == 1 && map_objects_slots.objects[k].arg.b.unk38 == 0x64) {
        map_objects_slots.objects[k].arg.b.unk38 = 0xff;
    }
    k = 242;
    if (cur_evt_state.unk32[0x11] == 1 && map_objects_slots.objects[k].arg.b.unk38 == 0x65) {
        map_objects_slots.objects[k].arg.b.unk38 = 0xff;
    }
    k = 243;
    if (cur_evt_state.unk32[0x12] == 1 && map_objects_slots.objects[k].arg.b.unk38 == 0x66) {
        map_objects_slots.objects[k].arg.b.unk38 = 0xff;
    }
    cur_evt_state.unk84[5] = 0xc;
}
