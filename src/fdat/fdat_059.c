/*
 * FDAT_059.BIN (WIP module): the handler table, then its code.
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
void func_801e8618(void);
void func_801e8620(void);
void func_801e8628(void);
void func_801e8630(void);
void func_801e8638(void);
void func_801e8640(void);
void func_801e8648(void);
void func_801e8650(void);

const FdatHeader fdat_header = {
    0x18,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e8618,
        func_801e8648, func_801e8650, func_801e8640, func_801e8390,
        func_801e8620, func_801e8628, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8630, func_801e8638, func_801e85b0, func_801e85b8,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x38,
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

/* Slot 19: object record 20 opens the way (sets the flag and object 85). */
ADDRESS(0x801e85b8, 0x60)
void func_801e85b8(Object88 *obj)
{
    if (obj - DAT_80185da8.objects == 20) {
        DAT_801ba984.unk32[0x27] = 1;
        DAT_8018fb3c.objects[85].arg.b.unk3f = 0xff;
        DAT_8018fb3c.objects[85].unk0 = 2;
    }
}

ADDRESS(0x801e8618, 0x8)
void func_801e8618(void)
{
}

ADDRESS(0x801e8620, 0x8)
void func_801e8620(void)
{
}

ADDRESS(0x801e8628, 0x8)
void func_801e8628(void)
{
}

ADDRESS(0x801e8630, 0x8)
void func_801e8630(void)
{
}

ADDRESS(0x801e8638, 0x8)
void func_801e8638(void)
{
}

ADDRESS(0x801e8640, 0x8)
void func_801e8640(void)
{
}

ADDRESS(0x801e8648, 0x8)
void func_801e8648(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8650, 0x74)
void func_801e8650(void)
{
    if (DAT_801ba984.unk32[0x2b] != 0) {
        DAT_80185da8.objects[17].unk0 = 8;
    } else {
        DAT_80185da8.objects[17].unk0 = 1;
    }
    if (DAT_801ba984.unk32[0x27] != 0) {
        DAT_8018fb3c.objects[85].arg.b.unk3f = 0xff;
        DAT_8018fb3c.objects[85].unk0 = 2;
    } else {
        DAT_8018fb3c.objects[85].arg.b.unk3f = 0;
        DAT_8018fb3c.objects[85].unk0 = 0;
    }
    DAT_801ba984.unk84[5] = 0x13;
}
