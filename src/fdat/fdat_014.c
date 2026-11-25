/*
 * FDAT_014.BIN (WIP module): the handler table and the switch table of its
 * slot 4 handler, the shared light-record blend, the lamp helper, its slot 0
 * handler, an item handler (slot 2), a switch handler (slot 3), event
 * handlers (slots 10 and 7), a record event handler (slot 4) and the load
 * handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x9c)

void func_801e8488(u8 n);
void func_801e84e0(void);
void func_801e84e8(VECTOR *pos, SVECTOR *angle);
void func_801e8744(void);
void func_801e874c(VECTOR *pos, SVECTOR *angle, s32 item);
void func_801e89a4(void);
void func_801e89ac(void);
void func_801e89b4(Object44 *obj);
void func_801e8ae8(void);
void func_801e8af0(void);
void func_801e8af8(void);
void func_801e8b00(void);
void func_801e8b08(void);
void func_801e8b10(u8 id);
void func_801e8b30(u8 id);
void func_801e8cc8(Object88 *record, s32 event);
void func_801e8da8(void);

const FdatHeader fdat_header = {
    0x9,
    {
        func_801e84e8, func_801e8744, func_801e874c, func_801e89b4,
        func_801e8cc8, func_801e8da8, func_801e8b08, func_801e8b30,
        func_801e8ae8, func_801e8af0, func_801e8b10, func_801e84e0,
        func_801e84e0, func_801e84e0, func_801e84e0, func_801e84e0,
        func_801e8af8, func_801e8b00, func_801e89a4, func_801e89ac,
        func_801e84e0, func_801e84e0, func_801e84e0, func_801e84e0,
        func_801e84e0, func_801e84e0, func_801e84e0, func_801e84e0,
        func_801e84e0, func_801e84e0, func_801e84e0, func_801e84e0,
    },
    0x29,
};

#include "shared/fdat_801e83a4.inc"

DATA(0x801e8e74, 0x30, ".data")
static Record30 DAT_801e8e74 = {
    {{{3000, 800, -1500}, {-2000, 1500, -3400}, {-1300, -2000, 1024}}, 0},
    {{{128, 128, 128}, {128, 128, 128}, {128, 128, 128}}, 0},
    {0x50, 0x50, 0x50, 0},
    6000,
    12000,
};
DATA(0x801e8ea4, 0x30, ".data")
static Record30 DAT_801e8ea4 = {
    {{{3000, -1500, 1500}, {-3000, -1500, -1500}, {0, 3000, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    18000,
    24000,
};

/* Blends area light record 47 by n sixteenths of the way to the opened state. */
ADDRESS(0x801e8488, 0x58)
void func_801e8488(u8 n)
{
    s32 i;
    s32 t;

    for (i = 0, t = 0; i < n; i++) {
        t += 256;
    }
    fdat_801e83a4(&DAT_801e8e74, &DAT_801e8ea4, &DAT_80081c8c[47], t);
}

ADDRESS(0x801e84e0, 0x8)
void func_801e84e0(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e84e8, 0x25c)
void func_801e84e8(VECTOR *pos, SVECTOR *angle)
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
                    } else if (obj->unk6 == 0x12d) {
                        notify_enqueue(0x1f);
                    } else if (obj->unk6 == 0x12e) {
                        notify_enqueue(0x20);
                    } else if (obj->unk6 == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->unk6 == 0xba || obj->unk6 == 303 || obj->unk6 == 304) {
                        notify_enqueue(0x22);
                    } else if (obj->unk6 == 0x131) {
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
                func_8005db30(link);
                link->unk0 = 0;
                link->arg.b.unk38 = 0;
                if (link->unk6 == 0xff) {
                    obj->arg.b.unk3a.index = 0xffff;
                }
            }
        }
    }
}

ADDRESS(0x801e8744, 0x8)
void func_801e8744(void)
{
}

/*
 * Slot 2: item 0x7b used on object 0x8d in front sinks it, then raises it
 * again while the area light brightens over 16 frames, and frees object 159.
 */
ADDRESS(0x801e874c, 0x258)
void func_801e874c(VECTOR *pos, SVECTOR *angle, s32 item)
{
    VECTOR at;
    SVECTOR facing;
    Object44 *obj;
    s32 i;
    s32 j;
    s32 k;

    if (item != 0x7b || DAT_801ba984.unke[0xf] == 1) {
        return;
    }
    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 0x6a4, angle->vy, 0x200)) != -1; i++) {
        if (i != 0x8d) {
            continue;
        }
        func_8005d7f8(0x7b);
        obj = &DAT_8018fb3c.objects[0x8d];
        DAT_80185da8.objects[7].unk0 = 8;
        DAT_80185da8.objects[26].unk0 = 1;
        DAT_8018fb3c.objects[0x8d].unk2 = 5;
        DAT_8018fb3c.objects[0x8d].unk10 = 0xfff;
        for (j = 15; j != -1; j--) {
            if ((s16)(obj->unk10 -= 256) < 0) {
                obj->unk10 = 0;
            }
            cd_request_wait_idle();
            cd_request_service_vab();
            cd_request_service_stream();
            player_get_camera_pose(&at, &facing);
            render_game_frame(&at, &facing);
        }
        obj->unk5 = 0x32;
        obj->unk2 = 5;
        for (j = 15; j != -1; j--) {
            func_801e8488(16 - j);
            light_reset_records();
            light_refresh_record_rotations();
            player_update_status_visuals();
            obj->unk10 += 256;
            if (obj->unk10 >= 4095) {
                obj->unk10 = 0xfff;
            }
            cd_request_wait_idle();
            cd_request_service_vab();
            cd_request_service_stream();
            player_get_camera_pose(&at, &facing);
            render_game_frame(&at, &facing);
        }
        obj->unk6 = 0xff;
        obj->unk4 = 0xff;
        obj->unk0 = 0;
        DAT_8018fb3c.objects[159].unk6 = 0xff;
        k = 74;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 < 253) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
            notify_enqueue(0x18);
        }
        DAT_801ba984.unke[0xf] = 1;
        DAT_801ba984.unk0 = 1;
    }
}

ADDRESS(0x801e89a4, 0x8)
void func_801e89a4(void)
{
}

ADDRESS(0x801e89ac, 0x8)
void func_801e89ac(void)
{
}

/* Slot 3: switches 0 and 1 move object 115 beside them; switch 10 shows one of two notes. */
ADDRESS(0x801e89b4, 0x134)
void func_801e89b4(Object44 *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unke[0xf] == 0) {
            DAT_8018fb3c.objects[115].pos.vx = obj->pos.vx - 1024;
        }
        break;
    case 1:
        DAT_8018fb3c.objects[115].pos.vx = obj->pos.vx + 1023;
        break;
    case 10:
        if (player_camera_within_map_region(0x37, 0xc, 3, 3, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                map_cell_copy_rotated_fields(2, 0x4a, 0, 0x37, 0xc, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            map_cell_copy_rotated_fields(2, 0x4d, 0, 0x37, 0xc, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8ae8, 0x8)
void func_801e8ae8(void)
{
}

ADDRESS(0x801e8af0, 0x8)
void func_801e8af0(void)
{
}

ADDRESS(0x801e8af8, 0x8)
void func_801e8af8(void)
{
}

ADDRESS(0x801e8b00, 0x8)
void func_801e8b00(void)
{
}

ADDRESS(0x801e8b08, 0x8)
void func_801e8b08(void)
{
}

/* Slot 10: event 12 sets the first magic element flag. */
ADDRESS(0x801e8b10, 0x20)
void func_801e8b10(u8 id)
{
    if (id == 12) {
        DAT_801ba984.unk71[0] = 1;
    }
}

/* Slot 7: raises a stat and opens objects 55 and 148 over 16 frames each. */
ADDRESS(0x801e8b30, 0x198)
void func_801e8b30(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    Object44 *obj;
    s32 i;

    DAT_801b24e4.unk34[0] += 20;
    player_learn_element_magic(0);
    player_recalculate_combat_stats();
    obj = &DAT_8018fb3c.objects[55];
    DAT_8018fb3c.objects[55].unk5 = 0x32;
    DAT_8018fb3c.objects[55].unk2 = 5;
    for (i = 15; i != -1; i--) {
        obj->unk10 += 256;
        if (obj->unk10 >= 4095) {
            obj->unk10 = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
    }
    obj->unk6 = 0xff;
    obj = &DAT_8018fb3c.objects[148];
    DAT_8018fb3c.objects[148].unk5 = 0x32;
    DAT_8018fb3c.objects[148].unk2 = 5;
    for (i = 15; i != -1; i--) {
        obj->unk10 += 256;
        if (obj->unk10 >= 4095) {
            obj->unk10 = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
    }
    obj->unk6 = 0xff;
    func_800608ec(10, 0);
    func_800608ec(10, 1);
}

/* Slot 4: script events 0..4 of an object record. */
ADDRESS(0x801e8cc8, 0xe0)
void func_801e8cc8(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        if (DAT_801ba984.unke[0xd] != 1) {
            DAT_801ba984.unke[0xd] = 1;
            func_8005db30(NULL, 0x7b);
        }
        break;
    case 1:
        if (DAT_801ba984.unke[0xe] != 1) {
            DAT_801ba984.unke[0xe] = 1;
            func_8005db30(NULL, 0x5f);
        }
        break;
    case 2:
        if (func_8005d7bc(0x84) > 0) {
            DAT_801ba984.unke[0x10] = 1;
        }
        break;
    case 3:
        DAT_80185da8.objects[8].unk0 = 8;
        break;
    case 4:
        DAT_801ba984.unke[0xc] = 1;
        break;
    }
}

/* Slot 5, called after the module loads: restores the sunk object and the area light. */
ADDRESS(0x801e8da8, 0xcc)
void func_801e8da8(void)
{
    s32 k;

    if (DAT_801ba984.unke[0xf] != 0) {
        DAT_80185da8.objects[7].unk0 = 8;
        DAT_80185da8.objects[26].unk0 = 1;
        DAT_8018fb3c.objects[0x8d].unk6 = 0xff;
        DAT_8018fb3c.objects[159].unk6 = 0xff;
        k = 74;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 < 253) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        func_801e8488(16);
    } else {
        DAT_80185da8.objects[7].unk0 = 1;
        DAT_80185da8.objects[26].unk0 = 8;
        func_801e8488(0);
    }
    if (DAT_801ba984.unk7f[0] != 0) {
        DAT_8018fb3c.objects[55].unk6 = 0xff;
        DAT_8018fb3c.objects[148].unk6 = 0xff;
    }
    DAT_801ba984.unk84[5] = 4;
}
