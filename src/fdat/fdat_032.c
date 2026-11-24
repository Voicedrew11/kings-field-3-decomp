/*
 * FDAT_032.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers, a note handler (slot 3), an event handler (slot 4) and the
 * load handler (slot 5), which sets up the two gates.
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
void func_801e85b8(void);
void func_801e85c0(Object44 *obj);
void func_801e8680(void);
void func_801e8688(void);
void func_801e8690(void);
void func_801e8698(void);
void func_801e86a0(void);
void func_801e86a8(Object88 *record, s32 event);
void func_801e8708(void);

const FdatHeader fdat_header = {
    0xf,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e85c0,
        func_801e86a8, func_801e8708, func_801e86a0, func_801e8390,
        func_801e8680, func_801e8688, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8690, func_801e8698, func_801e85b0, func_801e85b8,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x2f,
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

ADDRESS(0x801e85b8, 0x8)
void func_801e85b8(void)
{
}

/* Slot 3: for an object whose unk3a.bytes[1] is 10, shows one of two notes. */
ADDRESS(0x801e85c0, 0xc0)
void func_801e85c0(Object44 *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (func_80046884(0x24, 0x32, 3, 3, 0x8000) != 0) {
        if (DAT_801b24e4.unk10a != 0) {
            func_800445b8(2, 0x4a, 5, 0x24, 0x32, 3, 1, 0, 0x2f);
        }
    } else {
        DAT_801b24e4.unk10a = 0;
        func_800445b8(2, 0x4d, 5, 0x24, 0x32, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e8680, 0x8)
void func_801e8680(void)
{
}

ADDRESS(0x801e8688, 0x8)
void func_801e8688(void)
{
}

ADDRESS(0x801e8690, 0x8)
void func_801e8690(void)
{
}

ADDRESS(0x801e8698, 0x8)
void func_801e8698(void)
{
}

ADDRESS(0x801e86a0, 0x8)
void func_801e86a0(void)
{
}

/* Slot 4: script events 0 and 1 of an object record. */
ADDRESS(0x801e86a8, 0x60)
void func_801e86a8(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        func_8005db30(NULL, 0x90);
        break;
    case 1:
        if (DAT_801ba984.unk32[0xb] == 0) {
            DAT_801ba984.unk32[0xb] = 1;
        }
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8708, 0x35c)
void func_801e8708(void)
{
    Object88 *record;
    s32 i;

    if (DAT_801ba984.unk32[0xa] == 0) {
        DAT_801ba984.unk32[0xa] = 1;
    }
    if (DAT_801ba984.unk32[0xb] == 0) {
        DAT_80185da8.objects[29].unk0 = 1;
        DAT_8018fb3c.objects[253].unk0 = 0;
        DAT_8018fb3c.objects[253].arg.b.unk38 = 0;
        func_800445b8(2, 0x48, 5, 0x41, 0x31, 1, 1, 0, 0x2f);
        DAT_8018fb3c.objects[90].unk0 = 0;
        DAT_8018fb3c.objects[90].arg.b.unk3d[1] = 0;
        record = DAT_80185da8.objects;
        for (i = 14; i != -1; i--, record++) {
            record->unk0 = 1;
        }
        record = &DAT_80185da8.objects[15];
        for (i = 13; i != -1; i--, record++) {
            record->unk0 = 8;
        }
    } else if (DAT_801ba984.unk84[5] == 10) {
        if (DAT_801ba984.unk32[0xc] == 0) {
            DAT_80185da8.objects[29].unk0 = 1;
            DAT_8018fb3c.objects[253].unk0 = 0;
            DAT_8018fb3c.objects[253].arg.b.unk38 = 0;
            func_800445b8(2, 0x48, 5, 0x41, 0x31, 1, 1, 0, 0x2f);
            DAT_8018fb3c.objects[90].unk0 = 0;
            DAT_8018fb3c.objects[90].arg.b.unk3d[1] = 0;
            record = DAT_80185da8.objects;
            for (i = 14; i != -1; i--, record++) {
                record->unk0 = 1;
            }
            record = &DAT_80185da8.objects[15];
            for (i = 13; i != -1; i--, record++) {
                record->unk0 = 8;
            }
        } else {
            func_800445b8(2, 0x47, 5, 0x41, 0x31, 1, 1, 0, 0x2f);
            DAT_8018fb3c.objects[90].unk0 = 2;
            DAT_8018fb3c.objects[90].arg.b.unk3d[1] = 0xff;
            record = DAT_80185da8.objects;
            for (i = 14; i != -1; i--, record++) {
                record->unk0 = 8;
            }
        }
    } else {
        func_800445b8(2, 0x47, 5, 0x41, 0x31, 1, 1, 0, 0x2f);
        DAT_8018fb3c.objects[90].unk0 = 2;
        DAT_8018fb3c.objects[90].arg.b.unk3d[1] = 0xff;
        record = DAT_80185da8.objects;
        for (i = 14; i != -1; i--, record++) {
            record->unk0 = 8;
        }
        if (DAT_801ba984.unk32[0xc] == 0) {
            DAT_801ba984.unk32[0xc] = 1;
            record = &DAT_80185da8.objects[15];
            for (i = 13; i != -1; i--, record++) {
                record->unk0 = 1;
            }
        }
    }
    if (DAT_801ba984.unk32[0xc] != 0) {
        if (DAT_801ba984.unk32[0xc] == 1) {
            DAT_8018fb3c.objects[253].unk0 = 2;
            DAT_8018fb3c.objects[253].arg.b.unk38 = 0xff;
            DAT_801ba984.unk32[0xc] = 2;
        }
        DAT_80185da8.objects[29].unk0 = 8;
    }
    DAT_801ba984.unk84[5] = 10;
}
