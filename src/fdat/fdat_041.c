/*
 * FDAT_041.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers, a record handler (slot 19), switch handlers (slots 10 and
 * 7), an event handler (slot 4) and the load handler (slot 5).
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
void func_801e85b8(Object88 *obj);
void func_801e8644(void);
void func_801e864c(void);
void func_801e8654(void);
void func_801e865c(void);
void func_801e8664(void);
void func_801e866c(void);
void func_801e8674(u8 id);
void func_801e86d8(u8 id);
void func_801e88a0(Object88 *record, s32 event);
void func_801e88d4(void);

const FdatHeader fdat_header = {
    0x12,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e8644,
        func_801e88a0, func_801e88d4, func_801e866c, func_801e86d8,
        func_801e864c, func_801e8654, func_801e8674, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e865c, func_801e8664, func_801e85b0, func_801e85b8,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x32,
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

/* Slot 19: records 0..12 each set their bit of the 13-bit mask at +0x48. */
ADDRESS(0x801e85b8, 0x8c)
void func_801e85b8(Object88 *obj)
{
    s32 index = obj - DAT_80185da8.objects;
    s32 bit;

    if (index >= 0 && index < 13) {
        bit = 1 << index;
        DAT_801ba984.unk32[0x16] |= bit;
        DAT_801ba984.unk32[0x17] |= bit >> 8;
    }
    if (*(u16 *)&DAT_801ba984.unk32[0x16] == 0x1fff) {
        DAT_801ba984.unk32[0x18] = 1;
    }
}

ADDRESS(0x801e8644, 0x8)
void func_801e8644(void)
{
}

ADDRESS(0x801e864c, 0x8)
void func_801e864c(void)
{
}

ADDRESS(0x801e8654, 0x8)
void func_801e8654(void)
{
}

ADDRESS(0x801e865c, 0x8)
void func_801e865c(void)
{
}

ADDRESS(0x801e8664, 0x8)
void func_801e8664(void)
{
}

ADDRESS(0x801e866c, 0x8)
void func_801e866c(void)
{
}

/* Slot 10. */
ADDRESS(0x801e8674, 0x64)
void func_801e8674(u8 id)
{
    if (id == 4) {
        if (DAT_801ba984.unk32[0x14] != 0) {
            DAT_8018fb3c.objects[20].unk6 = 0xff;
            DAT_80185da8.objects[30].unk0 = 8;
            DAT_80185da8.objects[30].unk9 = 3;
        }
    }
    if (id == 13) {
        DAT_801ba984.unk71[1] = 1;
    }
}

/* Slot 7: event 13 raises a stat and opens objects 15 and 86 over 16 frames each. */
ADDRESS(0x801e86d8, 0x1c8)
void func_801e86d8(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    Object44 *obj;
    s32 i;

    if (id == 13) {
        DAT_801b24e4.unk34[1] += 20;
        player_learn_element_magic(1);
        player_recalculate_combat_stats();
        obj = &DAT_8018fb3c.objects[15];
        DAT_8018fb3c.objects[15].unk5 = 0x32;
        DAT_8018fb3c.objects[15].unk2 = 5;
        for (i = 15; i != -1; i--) {
            obj->unk10 += 256;
            if (obj->unk10 >= 4095) {
                obj->unk10 = 0xfff;
            }
            cd_request_wait_idle();
            cd_request_service_vab();
            cd_request_service_stream();
            player_get_camera_pose(&pos, &angle);
            func_800422b8(&pos, &angle);
        }
        obj->unk6 = 0xff;
        obj = &DAT_8018fb3c.objects[86];
        DAT_8018fb3c.objects[86].unk5 = 0x32;
        DAT_8018fb3c.objects[86].unk2 = 5;
        for (i = 15; i != -1; i--) {
            obj->unk10 += 256;
            if (obj->unk10 >= 4095) {
                obj->unk10 = 0xfff;
            }
            cd_request_wait_idle();
            cd_request_service_vab();
            cd_request_service_stream();
            player_get_camera_pose(&pos, &angle);
            func_800422b8(&pos, &angle);
        }
        obj->unk6 = 0xff;
        func_800608ec(0xb, 0);
        func_800608ec(0xb, 1);
    }
    if (id == 4) {
        DAT_801ba984.unk32[0x15] = 1;
    }
}

/* Slot 4: script event 0 of an object record sets flag 0x14 once. */
ADDRESS(0x801e88a0, 0x34)
void func_801e88a0(Object88 *record, s32 event)
{
    if (event == 0) {
        if (DAT_801ba984.unk32[0x14] == 0) {
            DAT_801ba984.unk32[0x14] = 1;
            DAT_801ba984.unk32[0x15] = 0;
        }
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e88d4, 0x84)
void func_801e88d4(void)
{
    DAT_801ba984.unk32[0x19] = 1;
    if (DAT_801ba984.unk7f[1] != 0) {
        DAT_8018fb3c.objects[15].unk6 = 0xff;
        DAT_8018fb3c.objects[86].unk6 = 0xff;
    }
    if (DAT_801ba984.unk32[0x14] == 0) {
        DAT_801ba984.unk32[0x15] = 1;
    } else {
        DAT_8018fb3c.objects[20].unk6 = 0xff;
        DAT_80185da8.objects[30].unk0 = 8;
        DAT_80185da8.objects[30].unk9 = 3;
    }
    DAT_801ba984.unk84[5] = 0xd;
}
