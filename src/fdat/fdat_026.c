/*
 * FDAT_026.BIN (WIP module): the handler table, the shared slot 0 handler,
 * a facing check (slot 1), an item handler (slot 2), empty handlers, a
 * record handler (slot 19), an event handler (slot 4) and the load handler
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

void func_801e8390(void);
void func_801e85a0(void);
void func_801e8644(VECTOR *pos, SVECTOR *angle, s32 item);
void func_801e8778(void);
void func_801e8780(Object88 *obj);
void func_801e87e8(void);
void func_801e87f0(void);
void func_801e87f8(void);
void func_801e8800(void);
void func_801e8808(void);
void func_801e8810(void);
void func_801e8818(Object88 *record, s32 event);
void func_801e8918(void);

const FdatHeader fdat_header = {
    0xd,
    {
        fdat_801e83b0, func_801e85a0, func_801e8644, func_801e87e8,
        func_801e8818, func_801e8918, func_801e8810, func_801e8390,
        func_801e87f0, func_801e87f8, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8800, func_801e8808, func_801e8778, func_801e8780,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x2d,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

#include "shared/fdat_801e83b0.inc"

/* Slot 1: with flag 0x30 alone set, record 1 stops when it faces away from the player. */
ADDRESS(0x801e85a0, 0xa4)
void func_801e85a0(void)
{
    if ((*(u32 *)&DAT_801ba984.unk2c[4] & 0xff00ff) != 1) {
        return;
    }
    if (DAT_80185da8.objects[1].unk9 == 1) {
        return;
    }
    if (func_80016a2c(DAT_801b24e4.unk124.vy,
                      func_80016ab8(DAT_80185da8.objects[1].pos.vx - DAT_801b24e4.pos.vx,
                                    DAT_80185da8.objects[1].pos.vz - DAT_801b24e4.pos.vz),
                      600) == 0) {
        DAT_80185da8.objects[1].unk9 = 0;
        DAT_80185da8.objects[1].unk0 = 2;
    }
}

/* Slot 2: item 0x81 used on object 0x34 in front fades the screen and sets flag 0x30. */
ADDRESS(0x801e8644, 0x134)
void func_801e8644(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;

    if (item != 0x81) {
        return;
    }
    if (DAT_801ba984.unk2c[4] != 0) {
        return;
    }
    for (i = 0; (i = func_80045c7c(i, pos, 800, 0x6a4, angle->vy, 0x200)) != -1; i++) {
        if (i == 0x34) {
            func_8005d948(0x200, 0, 0, 0, 0x80, 0x80, 200);
            func_8005d948(0x200, 0x80, 0x80, 200, 0, 0, 0);
            func_8003da10(0, 0, 0, 0, 1);
            DAT_801ba984.unk2c[4] = 1;
            func_8005d7f8(item);
            DAT_801ba984.unk0 = 1;
        }
    }
}

ADDRESS(0x801e8778, 0x8)
void func_801e8778(void)
{
}

/* Slot 19: record 1 moves on once flag 0x30 is set. */
ADDRESS(0x801e8780, 0x68)
void func_801e8780(Object88 *obj)
{
    if (obj - DAT_80185da8.objects == 1) {
        if (DAT_801ba984.unk2c[4] == 1) {
            DAT_801ba984.unk2c[4] = 2;
            DAT_80185da8.objects[1].unk0 = 8;
        }
    }
}

ADDRESS(0x801e87e8, 0x8)
void func_801e87e8(void)
{
}

ADDRESS(0x801e87f0, 0x8)
void func_801e87f0(void)
{
}

ADDRESS(0x801e87f8, 0x8)
void func_801e87f8(void)
{
}

ADDRESS(0x801e8800, 0x8)
void func_801e8800(void)
{
}

ADDRESS(0x801e8808, 0x8)
void func_801e8808(void)
{
}

ADDRESS(0x801e8810, 0x8)
void func_801e8810(void)
{
}

/* Slot 4: script events 0 and 1 of an object record. */
ADDRESS(0x801e8818, 0x100)
void func_801e8818(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        func_8005d948(0x200, 0, 0, 0, 0x80, 200, 0x80);
        func_8005d948(0x200, 0x80, 200, 0x80, 0, 0, 0);
        DAT_801ba984.unk31 = 1;
        DAT_801b77ec.unk0[54].unk0 = 1;
        DAT_801b77ec.unk0[55].unk0 = 1;
        DAT_801b77ec.unk0[66].unk0 = 1;
        DAT_801b77ec.unk0[67].unk0 = 1;
        DAT_801b77ec.unk0[68].unk0 = 1;
        DAT_801b77ec.unk0[69].unk0 = 1;
        DAT_801b77ec.unk0[70].unk0 = 1;
        DAT_801b77ec.unk0[71].unk0 = 1;
        DAT_801b77ec.unk0[72].unk0 = 1;
        DAT_801b77ec.unk0[73].unk0 = 1;
        DAT_801b77ec.unk0[74].unk0 = 1;
        DAT_801b77ec.unk0[75].unk0 = 1;
        break;
    case 1:
        DAT_801ba984.unk32[0] = 1;
        DAT_80185da8.objects[1].unk0 = 8;
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8918, 0xb0)
void func_801e8918(void)
{
    if (DAT_801ba984.unk2c[4] != 0 && DAT_801ba984.unk32[0] == 0) {
        DAT_80185da8.objects[1].unk0 = 1;
    } else {
        DAT_80185da8.objects[1].unk0 = 8;
    }
    if (func_8005d7bc(0x85) > 0) {
        DAT_801ba984.unk32[1] = 1;
    } else {
        DAT_801ba984.unk32[1] = 0;
    }
    if (func_8005d7bc(0x10) > 0) {
        DAT_801ba984.unk32[2] = 1;
    }
    if (func_8005d7bc(0x7d) > 0) {
        DAT_801ba984.unk32[3] = 1;
    }
    DAT_801ba984.unk84[5] = 8;
}
