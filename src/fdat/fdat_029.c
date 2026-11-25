/*
 * FDAT_029.BIN (WIP module): the shared light-record blend, the handler
 * table, its slot 0 handler, empty handlers, a switch handler (slot 3), a
 * record event handler (slot 4) and the load handler (slot 5).
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
void area_interact_item_containers_and_unlock(VECTOR *pos, SVECTOR *angle);
void area_update_noop(void);
void area_use_item_noop(void);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_noop(void);
void area_region_trigger_damage_barrier_and_doors(Object44 *obj);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_set_flags(Object88 *record, s32 event);
void area_on_load_restore_doors_and_barrier(void);

const FdatHeader fdat_header = {
    0xe,
    {
        /*  0 */ area_interact_item_containers_and_unlock,
        /*  1 */ area_update_noop,
        /*  2 */ area_use_item_noop,
        /*  3 */ area_region_trigger_damage_barrier_and_doors,
        /*  4 */ area_talk_event_set_flags,
        /*  5 */ area_on_load_restore_doors_and_barrier,
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
    0x2e,
};

#include "shared/light_blend_records.inc"

ADDRESS(0x801e8474, 0x8)
void area_handler_noop(void)
{
}

/*
 * Slot 0: first sets flag 0x3a (and starts sound 0xf1) once item 0x7d is
 * held or the level reaches 50, then area_interact_item_containers's shape with this
 * module's message table.
 */
ADDRESS(0x801e847c, 0x278)
void area_interact_item_containers_and_unlock(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    if (DAT_801ba984.unk32[8] == 0) {
        if (game_counter_is_nonzero(0x7d) > 0 || DAT_801b24e4.level >= 50) {
            DAT_801ba984.unk32[8] = 1;
            map_object_process_marker_all(0xf1);
        }
    }
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
                    } else if (obj->unk6 == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->unk6 == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->unk6 == 312 || obj->unk6 == 313 || obj->unk6 == 0x13a) {
                        notify_enqueue(0x21);
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

ADDRESS(0x801e86f4, 0x8)
void area_update_noop(void)
{
}

ADDRESS(0x801e86fc, 0x8)
void area_use_item_noop(void)
{
}

ADDRESS(0x801e8704, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

ADDRESS(0x801e870c, 0x8)
void area_on_actor_removed_noop(void)
{
}

/*
 * Slot 3: switch 0 (with flag 0x38 set) opens objects 249 and 262 and starts
 * record 2; switch 1 strikes around object 58 until the four element flags
 * are set, then dissolves it over 16 frames; switch 10 shows one of two notes.
 */
ADDRESS(0x801e8714, 0x354)
void area_region_trigger_damage_barrier_and_doors(Object44 *obj)
{
    VECTOR pos;
    SVECTOR angle;
    s32 i;
    Object44 *other;

    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unk32[6] == 1 && player_camera_within_map_region(0x36, 0xf, 10, 4, 0x8000) == 0) {
            DAT_80185da8.objects[2].unk0 = 8;
            DAT_8018fb3c.objects[249].unk0 = 2;
            DAT_8018fb3c.objects[249].arg.b.unk3c = 0xff;
            DAT_8018fb3c.objects[262].arg.b.unk3d[1] = 0xff;
        }
        break;
    case 1:
        if (DAT_801ba984.unk32[7] == 0) {
            if (DAT_801ba984.unk7f[0] != 0 && DAT_801ba984.unk7f[1] != 0 &&
                DAT_801ba984.unk7f[2] != 0 && DAT_801ba984.unk7f[3] != 0) {
                DAT_801ba984.unk32[7] = 1;
            }
            obj = &DAT_8018fb3c.objects[58];
            obj->unk0 = 2;
            if (DAT_801b24e4.unk101 == 0x10) {
                audio_play_spatial_default_range(0x28, &obj->pos, 0x78, 0);
            }
            player_apply_radial_damage(&obj->pos, 0, 0x1000, 0x8000, 0x1000, 0x40, 0x40, 0x40, 0x40, 0x40,
                                       0x40, 0x40, 0x40, 0x40, 0x400, 10);
        } else if (DAT_801ba984.unk32[7] == 1) {
            other = &DAT_8018fb3c.objects[58];
            other->unk0 = 2;
            DAT_8018fb3c.objects[58].unk5 = 0x32;
            DAT_8018fb3c.objects[58].unk2 = 5;
            DAT_8018fb3c.objects[58].unk10 = 0;
            for (i = 15; i != -1; i--) {
                other->unk10 += 256;
                if (other->unk10 >= 4095) {
                    other->unk10 = 0xfff;
                }
                cd_request_wait_idle();
                cd_request_service_vab();
                cd_request_service_stream();
                player_get_camera_pose(&pos, &angle);
                render_game_frame(&pos, &angle);
            }
            other->unk6 = 0xff;
            other->unk4 = 0xff;
            other->unk0 = 0;
            DAT_801ba984.unk32[7] = 2;
        } else {
            DAT_8018fb3c.objects[58].unk6 = 0xff;
            DAT_8018fb3c.objects[58].unk4 = 0xff;
            DAT_8018fb3c.objects[58].unk0 = 0;
        }
        break;
    case 10:
        if (player_camera_within_map_region(0x24, 0x36, 7, 0xc, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                map_cell_copy_rotated_fields(2, 0x2a, 0x4f, 0x26, 0x36, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            map_cell_copy_rotated_fields(2, 0x2d, 0x4f, 0x26, 0x36, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8a68, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e8a70, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8a78, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8a80, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8a88, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 4: script events 0 and 1 of an object record set flags 0x37 and 0x38. */
ADDRESS(0x801e8a90, 0x3c)
void area_talk_event_set_flags(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        DAT_801ba984.unk32[5] = 1;
        break;
    case 1:
        DAT_801ba984.unk32[6] = 1;
        break;
    }
}

/* Slot 5, called after the module loads: restores the switch states. */
ADDRESS(0x801e8acc, 0x140)
void area_on_load_restore_doors_and_barrier(void)
{
    if (DAT_801ba984.unke[0x1a] != 0 && DAT_801ba984.unk2b != 0) {
        DAT_801ba984.unk32[4] = 1;
    }
    if (DAT_801ba984.unk32[6] == 0) {
        DAT_80185da8.objects[2].unk0 = 1;
        DAT_8018fb3c.objects[249].unk0 = 0;
        DAT_8018fb3c.objects[249].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[262].arg.b.unk3d[1] = 0;
    } else {
        DAT_80185da8.objects[2].unk0 = 8;
        DAT_8018fb3c.objects[249].unk0 = 2;
        DAT_8018fb3c.objects[249].arg.b.unk3c = 0xff;
        DAT_8018fb3c.objects[262].arg.b.unk3d[1] = 0xff;
    }
    if (DAT_801ba984.unk32[7] != 0) {
        DAT_8018fb3c.objects[58].unk6 = 0xff;
        DAT_8018fb3c.objects[58].unk4 = 0xff;
        DAT_8018fb3c.objects[58].unk0 = 0;
        DAT_801ba984.unk32[7] = 2;
    }
    if (DAT_801ba984.unk32[8] == 0) {
        if (game_counter_is_nonzero(0x7d) > 0 || DAT_801b24e4.level >= 40) {
            DAT_801ba984.unk32[8] = 1;
            map_object_process_marker_all(0xf1);
        }
    }
    DAT_801ba984.unk84[5] = 9;
}
