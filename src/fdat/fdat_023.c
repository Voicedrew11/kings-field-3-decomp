/*
 * FDAT_023.BIN (WIP module): the handler table, the shared slot 0 handler,
 * a key handler (slot 2), a note handler (slot 3), object handlers (slots 8
 * and 9), an event handler (slot 4), the load handler (slot 5) and the two
 * offsets slot 9 passes.
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
void func_801e86e4(void);
void func_801e86ec(void);
void func_801e86f4(Object44 *obj);
void func_801e87b4(Object44 *obj, Object44Def *def);
void func_801e87d0(void);
void func_801e8884(void);
void func_801e888c(void);
void func_801e8894(void);
void func_801e889c(void);
void func_801e88a4(void);
void func_801e88ac(void);
void func_801e88b4(void);
void func_801e88bc(void);
void func_801e88c4(void);
void func_801e88cc(void);
void func_801e88d4(void);
void func_801e88dc(Object88 *obj, s32 event);
void func_801e88f4(void);

const FdatHeader fdat_header = {
    0xc,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e86f4,
        func_801e88dc, func_801e88f4, func_801e88d4, func_801e8390,
        func_801e87b4, func_801e87d0, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e88c4, func_801e88cc, func_801e86e4, func_801e86ec,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x2c,
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

/* Slot 2: item 0x85 used on object 0x140 in front unlocks it. */
ADDRESS(0x801e85a8, 0x13c)
void func_801e85a8(VECTOR *pos, SVECTOR *angle, s32 item)
{
    VECTOR unused;
    SVECTOR unused_angle;
    s32 i;
    Object44 *obj;

    if (item != 0x85) {
        return;
    }
    i = func_80045c7c(0, pos, 800, 0x6a4, angle->vy, 0x200);
    if (i != -1) {
        obj = &DAT_8018fb3c.objects[i];
        if (obj->unk6 == 0x140) {
            if (func_80016a2c(DAT_801b24e4.unk12c.vy + 2048, obj->angle.vy, 0x100) == 0) {
                return;
            }
            if (obj->unk38 != 0xff) {
                return;
            }
            obj->unk38 = item;
            obj->unk8 = 0;
            DAT_801ba984.unk0 = 1;
            func_8005d7f8(0x85);
            func_8005c7e8(obj, 0x85);
        } else if (func_80046608(obj, 0x85) == 3) {
            func_80041eec(7);
            DAT_801ba984.unk0 = 1;
        }
    } else {
        DAT_801ba984.unk0 = 1;
        func_80041eec(0x23);
    }
}

ADDRESS(0x801e86e4, 0x8)
void func_801e86e4(void)
{
}

ADDRESS(0x801e86ec, 0x8)
void func_801e86ec(void)
{
}

/* Slot 3: for an object whose unk3a.bytes[1] is 10, shows one of two notes. */
ADDRESS(0x801e86f4, 0xc0)
void func_801e86f4(Object44 *obj)
{
    if (obj->unk3a.bytes[1] != 10) {
        return;
    }
    if (func_80046884(0x12, 0x1c, 3, 3, 0x8000) != 0) {
        if (DAT_801b24e4.unk10a != 0) {
            func_800445b8(2, 0x15, 0x4f, 0x12, 0x1e, 3, 1, 0, 0x2f);
        }
    } else {
        DAT_801b24e4.unk10a = 0;
        func_800445b8(2, 0x18, 0x4f, 0x12, 0x1e, 3, 1, 0, 0x2f);
    }
}

/* Slot 8: an object made from definition kind 0xa7 takes that kind. */
ADDRESS(0x801e87b4, 0x1c)
void func_801e87b4(Object44 *obj, Object44Def *def)
{
    if (def->unk0[0] == 0xa7) {
        obj->unk4 = 0xa7;
    }
}

DATA(0x801e8908, 0x8, ".data")
static SVECTOR DAT_801e8908 = {0, -1600, 0x400, 0};
DATA(0x801e8910, 0x8, ".data")
static SVECTOR DAT_801e8910 = {0, -1600, 0x100, 0};

/* Slot 9: when the tested kind 0xa7 object reaches its target, frees every 0xe8 object. */
ADDRESS(0x801e87d0, 0xb4)
void func_801e87d0(void)
{
    Object44 *obj = DAT_8018fb3c.unk8858;
    Object44 *other;
    s32 i;

    if (obj->unk4 != 0xa7) {
        return;
    }
    if (func_80046914(obj, &DAT_8018fb3c.objects[380 + obj->unk39], &DAT_801e8908, &DAT_801e8910,
                      0, 20) == 0) {
        return;
    }
    other = DAT_8018fb3c.objects;
    for (i = 0x18c; i != 0; i--, other++) {
        if (other->unk4 == 0xe8) {
            other->unk0[1] = 0x81;
            other->unk40 = 0;
        }
    }
    DAT_801ba984.unk0 = 1;
}

ADDRESS(0x801e8884, 0x8)
void func_801e8884(void)
{
}

ADDRESS(0x801e888c, 0x8)
void func_801e888c(void)
{
}

ADDRESS(0x801e8894, 0x8)
void func_801e8894(void)
{
}

ADDRESS(0x801e889c, 0x8)
void func_801e889c(void)
{
}

ADDRESS(0x801e88a4, 0x8)
void func_801e88a4(void)
{
}

ADDRESS(0x801e88ac, 0x8)
void func_801e88ac(void)
{
}

ADDRESS(0x801e88b4, 0x8)
void func_801e88b4(void)
{
}

ADDRESS(0x801e88bc, 0x8)
void func_801e88bc(void)
{
}

ADDRESS(0x801e88c4, 0x8)
void func_801e88c4(void)
{
}

ADDRESS(0x801e88cc, 0x8)
void func_801e88cc(void)
{
}

ADDRESS(0x801e88d4, 0x8)
void func_801e88d4(void)
{
}

/* Slot 4: script event 0 of an object record sets flag 2. */
ADDRESS(0x801e88dc, 0x18)
void func_801e88dc(Object88 *obj, s32 event)
{
    if (event == 0) {
        DAT_801ba984.unk2c[2] = 1;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e88f4, 0x14)
void func_801e88f4(void)
{
    DAT_801ba984.unk84[5] = 7;
}
