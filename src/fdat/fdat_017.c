/*
 * FDAT_017.BIN (WIP module): the handler table, the shared slot 0 handler,
 * an item handler (slot 2), a note handler (slot 3), empty handlers, an event
 * handler (slot 4) and the load handler (slot 5).
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
void func_801e85a0(void);
void func_801e85a8(VECTOR *pos, SVECTOR *angle, s32 item);
void func_801e867c(void);
void func_801e8684(void);
void func_801e868c(Object44 *obj);
void func_801e874c(void);
void func_801e8754(void);
void func_801e875c(void);
void func_801e8764(void);
void func_801e876c(void);
void func_801e8774(Object88 *record, s32 event);
void func_801e8880(void);

const FdatHeader fdat_header = {
    0xa,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e868c,
        func_801e8774, func_801e8880, func_801e876c, func_801e8390,
        func_801e874c, func_801e8754, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e875c, func_801e8764, func_801e867c, func_801e8684,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x2a,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

#include "shared/fdat_801e83b0.inc"

ADDRESS(0x801e85a0, 0x8)
void func_801e85a0(void)
{
}

/* Slot 2: item 0x66 on a kind 0x53 object in front starts it. */
ADDRESS(0x801e85a8, 0xd4)
void func_801e85a8(VECTOR *pos, SVECTOR *angle, s32 item)
{
    VECTOR unused;
    SVECTOR unused_angle;
    s32 i;
    Object44 *obj;

    if (item != 0x66) {
        return;
    }
    i = map_object_find_interaction_target(0, pos, 800, 0x6a4, angle->vy, 0x200);
    if (i == -1) {
        return;
    }
    obj = &DAT_8018fb3c.objects[i];
    if (obj->unk4 != 0x53) {
        return;
    }
    if (obj->arg.b.unk3d[0] >= 151) {
        return;
    }
    if (func_8005d7bc(obj->arg.b.unk3d[0]) == 0) {
        return;
    }
    if (obj->unka == 0 || obj->unka >= 4095) {
        obj->unk8 = 1;
        DAT_801ba984.unk0 = 1;
    }
}

ADDRESS(0x801e867c, 0x8)
void func_801e867c(void)
{
}

ADDRESS(0x801e8684, 0x8)
void func_801e8684(void)
{
}

/* Slot 3: for an object whose unk3a.bytes[1] is 10, shows one of two notes. */
ADDRESS(0x801e868c, 0xc0)
void func_801e868c(Object44 *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(0x1c, 0x1e, 3, 4, 0x8000) != 0) {
        if (DAT_801b24e4.unk10a != 0) {
            map_cell_copy_rotated_fields(2, 0x4a, 0, 0x1c, 0x21, 3, 1, 0, 0x2f);
        }
    } else {
        DAT_801b24e4.unk10a = 0;
        map_cell_copy_rotated_fields(2, 0x4d, 0, 0x1c, 0x21, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e874c, 0x8)
void func_801e874c(void)
{
}

ADDRESS(0x801e8754, 0x8)
void func_801e8754(void)
{
}

ADDRESS(0x801e875c, 0x8)
void func_801e875c(void)
{
}

ADDRESS(0x801e8764, 0x8)
void func_801e8764(void)
{
}

ADDRESS(0x801e876c, 0x8)
void func_801e876c(void)
{
}

/* Slot 4: script events 0..2 of an object record. */
ADDRESS(0x801e8774, 0x10c)
void func_801e8774(Object88 *record, s32 event)
{
    Object44 *obj;
    s32 i;

    switch (event) {
    case 0:
        obj = DAT_8018fb3c.objects;
        for (i = 0x18c; i != 0; i--, obj++) {
            if (obj->unk4 == 0x56) {
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->arg.b.unk38 = 0;
                obj->unk0 = 0;
                obj->unk4 = 0xff;
            }
        }
        if (func_8005d7bc(0x66) != 0) {
            func_8005d7f8(0x66);
        }
        func_8005db30(NULL, 0x82);
        DAT_801ba984.unke[0x14] = 1;
        break;
    case 1:
        if (DAT_801ba984.unke[0x13] == 0) {
            func_8005db30(NULL, 0x60);
            DAT_801ba984.unke[0x13] = 1;
        }
        break;
    case 2:
        func_8005db30(NULL, 0x66);
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8880, 0xac)
void func_801e8880(void)
{
    Object44 *obj;
    s32 i;

    if (DAT_801ba984.unke[0x12] == 0) {
        DAT_801ba984.unke[0x12] = 1;
    }
    if (DAT_801ba984.unke[0] != 0) {
        DAT_801ba984.unke[0x16] = 1;
    }
    if (DAT_801ba984.unke[0x14] != 0) {
        obj = DAT_8018fb3c.objects;
        for (i = 0x18c; i != 0; i--, obj++) {
            if (obj->unk4 == 0x56) {
                obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
                obj->arg.b.unk38 = 0;
                obj->unk0 = 0;
                obj->unk4 = 0xff;
            }
        }
    }
    DAT_801ba984.unk84[5] = 5;
}
