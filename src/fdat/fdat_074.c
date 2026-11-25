/*
 * FDAT_074.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, a flag setter and the load handler (slot 5).
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
void area_on_movie_start_set_flag(void);
void area_on_movie_end_noop(void);
void area_talk_event_noop(void);
void area_on_load_reset_flag(void);

const FdatHeader fdat_header = {
    0x1d,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_noop,
        /*  5 */ area_on_load_reset_flag,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_on_movie_end_noop,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_on_movie_start_set_flag,
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
    0x3d,
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
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7) {
                        notify_enqueue(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        notify_enqueue(0x1e);
                    } else if (obj->unk6 == 0x13f) {
                        notify_enqueue(0x1f);
                    } else if (obj->unk6 == 0x140) {
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

ADDRESS(0x801e85d0, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e85d8, 0x8)
void area_use_item_noop(void)
{
}

ADDRESS(0x801e85e0, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e85e8, 0x8)
void area_on_actor_removed_noop(void)
{
}

ADDRESS(0x801e85f0, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e85f8, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8600, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8608, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8610, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8618, 0x8)
void area_on_item_added_noop(void)
{
}

/* Sets flag 0x2f. */
ADDRESS(0x801e8620, 0x14)
void area_on_movie_start_set_flag(void)
{
    DAT_801ba984.unk32[0x2f] = 1;
}

ADDRESS(0x801e8634, 0x8)
void area_on_movie_end_noop(void)
{
}

ADDRESS(0x801e863c, 0x8)
void area_talk_event_noop(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8644, 0x2c)
void area_on_load_reset_flag(void)
{
    if (DAT_801ba984.unk32[0x37] == 0) {
        DAT_801ba984.unk32[0x2f] = 0;
    }
    DAT_801ba984.unk84[5] = 0x18;
}
