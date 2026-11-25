/*
 * FDAT_035.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, a record handler (slot 19), a note handler (slot 3) and the load
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

void func_801e8390(void);
void func_801e8398(VECTOR *pos, SVECTOR *angle);
void func_801e85c4(void);
void func_801e85cc(void);
void func_801e85d4(void);
void func_801e85dc(Object88 *obj);
void func_801e8680(Object44 *obj);
void func_801e8740(void);
void func_801e8748(void);
void func_801e8750(void);
void func_801e8758(void);
void func_801e8760(void);
void func_801e8768(void);
void func_801e8770(void);

const FdatHeader fdat_header = {
    0x10,
    {
        func_801e8398, func_801e85c4, func_801e85cc, func_801e8680,
        func_801e8768, func_801e8770, func_801e8760, func_801e8390,
        func_801e8740, func_801e8748, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8750, func_801e8758, func_801e85d4, func_801e85dc,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x30,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e8398, 0x22c)
void func_801e8398(VECTOR *pos, SVECTOR *angle)
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
                    } else if (obj->unk6 == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->unk6 == 0xba || obj->unk6 == 0x12d || obj->unk6 == 0x12e) {
                        notify_enqueue(0x22);
                    } else if (obj->unk6 == 0x12f) {
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

ADDRESS(0x801e85c4, 0x8)
void func_801e85c4(void)
{
}

ADDRESS(0x801e85cc, 0x8)
void func_801e85cc(void)
{
}

ADDRESS(0x801e85d4, 0x8)
void func_801e85d4(void)
{
}

/* Slot 19: record 3 disarms object 20 and triggers its link. */
ADDRESS(0x801e85dc, 0xa4)
void func_801e85dc(Object88 *obj)
{
    Object44 *link;

    if (obj - DAT_80185da8.objects == 3) {
        DAT_801ba984.unk32[0xe] = 1;
        DAT_8018fb3c.objects[20].unk0 = 0;
        DAT_8018fb3c.objects[20].arg.b.unk3c = 0;
        if (DAT_8018fb3c.objects[20].arg.b.unk3a.index != 0xffff) {
            link = &DAT_8018fb3c.objects[DAT_8018fb3c.objects[20].arg.b.unk3a.index];
            if (link->unk6 != 0xff) {
                link->unk0 = 2;
                link->arg.b.unk38 = 0xff;
            }
        }
        DAT_8018fb3c.objects[33].arg.b.unk3d[1] = 0;
    }
}

/* Slot 3: for an object whose unk3a.bytes[1] is 10, shows one of two notes. */
ADDRESS(0x801e8680, 0xc0)
void func_801e8680(Object44 *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(0x39, 0x21, 3, 3, 0x8000) != 0) {
        if (DAT_801b24e4.unk10a != 0) {
            map_cell_copy_rotated_fields(2, 0x4a, 0, 0x39, 0x23, 3, 1, 0, 0x2f);
        }
    } else {
        DAT_801b24e4.unk10a = 0;
        map_cell_copy_rotated_fields(2, 0x4d, 0, 0x39, 0x23, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e8740, 0x8)
void func_801e8740(void)
{
}

ADDRESS(0x801e8748, 0x8)
void func_801e8748(void)
{
}

ADDRESS(0x801e8750, 0x8)
void func_801e8750(void)
{
}

ADDRESS(0x801e8758, 0x8)
void func_801e8758(void)
{
}

ADDRESS(0x801e8760, 0x8)
void func_801e8760(void)
{
}

ADDRESS(0x801e8768, 0x8)
void func_801e8768(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8770, 0xb4)
void func_801e8770(void)
{
    Object44 *link;

    if (DAT_801ba984.unk32[0xe] == 0) {
        DAT_8018fb3c.objects[20].unk0 = 2;
        DAT_8018fb3c.objects[20].arg.b.unk3c = 0xff;
        DAT_8018fb3c.objects[33].arg.b.unk3d[1] = 0xff;
    } else {
        DAT_8018fb3c.objects[20].unk0 = 0;
        DAT_8018fb3c.objects[20].arg.b.unk3c = 0;
        if (DAT_8018fb3c.objects[20].arg.b.unk3a.index != 0xffff) {
            link = &DAT_8018fb3c.objects[DAT_8018fb3c.objects[20].arg.b.unk3a.index];
            if (link->unk6 != 0xff) {
                link->unk0 = 2;
                link->arg.b.unk38 = 0xff;
                DAT_8018fb3c.objects[20].arg.b.unk3a.index = 0xffff;
            }
        }
        DAT_8018fb3c.objects[33].arg.b.unk3d[1] = 0;
    }
    DAT_801ba984.unk84[5] = 0xb;
}
