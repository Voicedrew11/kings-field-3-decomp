/*
 * FDAT_047.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers, a two-door opener, record 3's and 40's handler (slot 19)
 * and the load handler (slot 5).
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
void func_801e85a8(void);
void func_801e85b0(void);
void func_801e85b8(s32 a, s32 b);
void func_801e870c(Object88 *obj);
void func_801e87d8(void);
void func_801e87e0(void);
void func_801e87e8(void);
void func_801e87f0(void);
void func_801e87f8(void);
void func_801e8800(void);
void func_801e8808(void);
void func_801e8810(void);

const FdatHeader fdat_header = {
    0x14,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e87d8,
        func_801e8808, func_801e8810, func_801e8800, func_801e8390,
        func_801e87e0, func_801e87e8, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e87f0, func_801e87f8, func_801e85b0, func_801e870c,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x34,
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

ADDRESS(0x801e85a8, 0x8)
void func_801e85a8(void)
{
}

ADDRESS(0x801e85b0, 0x8)
void func_801e85b0(void)
{
}

/* Opens objects a and b over 16 frames (unk10 up to 0xfff), then frees them. */
ADDRESS(0x801e85b8, 0x120)
void func_801e85b8(s32 a, s32 b)
{
    VECTOR pos;
    SVECTOR angle;
    Object44 *first;
    Object44 *second;
    s32 i;

    first = &DAT_8018fb3c.objects[a];
    second = &DAT_8018fb3c.objects[b];
    first->unk5 = 0x32;
    second->unk5 = 0x32;
    first->unk2 = 5;
    second->unk2 = 5;
    for (i = 15; i != -1; i--) {
        first->unk10 += 256;
        second->unk10 += 256;
        if (first->unk10 >= 4095) {
            first->unk10 = 0xfff;
        }
        if (second->unk10 >= 4095) {
            second->unk10 = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        func_8002b330(&pos, &angle);
        func_800422b8(&pos, &angle);
    }
    first->unk6 = 0xff;
    second->unk6 = 0xff;
}

#include "shared/fdat_801e86d8.inc"

/* Slot 19: records 3 and 40 each open their pair of doors once. */
ADDRESS(0x801e870c, 0xcc)
void func_801e870c(Object88 *obj)
{
    switch (obj - DAT_80185da8.objects) {
    case 3:
        if (DAT_801ba984.unk32[0x1b] == 0) {
            DAT_801ba984.unk32[0x1b] = 1;
            fdat_801e86d8(4, 2);
            func_801e85b8(0x73, 0x74);
        }
        break;
    case 40:
        if (DAT_801ba984.unk32[0x1c] == 0) {
            DAT_801ba984.unk32[0x1c] = 1;
            fdat_801e86d8(10, 2);
            func_801e85b8(0x75, 0x76);
        }
        break;
    }
}

ADDRESS(0x801e87d8, 0x8)
void func_801e87d8(void)
{
}

ADDRESS(0x801e87e0, 0x8)
void func_801e87e0(void)
{
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

/* Slot 5, called after the module loads: restores the opened doors. */
ADDRESS(0x801e8810, 0x80)
void func_801e8810(void)
{
    if (DAT_801ba984.unk32[0x1b] != 0) {
        fdat_801e86d8(4, 2);
        DAT_8018fb3c.objects[0x73].unk6 = 0xff;
        DAT_8018fb3c.objects[0x74].unk6 = 0xff;
    }
    if (DAT_801ba984.unk32[0x1c] != 0) {
        fdat_801e86d8(10, 2);
        DAT_8018fb3c.objects[0x75].unk6 = 0xff;
        DAT_8018fb3c.objects[0x76].unk6 = 0xff;
    }
    DAT_801ba984.unk84[5] = 0xf;
}
