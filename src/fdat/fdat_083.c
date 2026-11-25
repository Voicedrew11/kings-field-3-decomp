/*
 * FDAT_083.BIN (WIP module): the handler table, the shared slot 0, 2, 16 and
 * 17 handlers, empty handlers, a record handler (slot 19), a flag setter and
 * the load handler (slot 5).
 */
#define FDAT_801E87A0_STEP 64
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
void func_801e85dc(void);
void func_801e85e4(Object88 *obj);
void func_801e8694(void);
void func_801e869c(void);
void func_801e86a4(void);
void func_801e8a40(void);
void func_801e8a48(void);
void func_801e8a50(void);
void func_801e8a64(void);
void func_801e8a6c(void);

const FdatHeader fdat_header = {
    0x20,
    {
        fdat_801e83b0, func_801e85a0, fdat_801e85a8, func_801e8694,
        func_801e8a64, func_801e8a6c, func_801e8a40, func_801e8a50,
        func_801e869c, func_801e86a4, func_801e8a48, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        fdat_801e875c, fdat_801e87a0, func_801e85dc, func_801e85e4,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x40,
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

#include "shared/fdat_801e85a8.inc"

ADDRESS(0x801e85dc, 0x8)
void func_801e85dc(void)
{
}

/* Slot 19: record 0 gives item 0x11 for item 0x10 once. */
ADDRESS(0x801e85e4, 0xb0)
void func_801e85e4(Object88 *obj)
{
    if (obj - DAT_80185da8.objects != 0) {
        return;
    }
    if (DAT_801b24e4.unk101 == 0x11) {
        return;
    }
    DAT_801ba984.unk32[0x37] = 1;
    if (func_8005d7bc(0x10) <= 0) {
        return;
    }
    DAT_801ba984.unk32[0x38] = 1;
    func_8005d7f8(0x10);
    func_8005d898(0x11);
    if (DAT_801b24e4.unkcb == 0x10) {
        player_equip_weapon(0x11);
    }
}

ADDRESS(0x801e8694, 0x8)
void func_801e8694(void)
{
}

ADDRESS(0x801e869c, 0x8)
void func_801e869c(void)
{
}

ADDRESS(0x801e86a4, 0x8)
void func_801e86a4(void)
{
}

#include "shared/fdat_801e875c.inc"

#include "shared/fdat_801e87a0.inc"

ADDRESS(0x801e8a40, 0x8)
void func_801e8a40(void)
{
}

ADDRESS(0x801e8a48, 0x8)
void func_801e8a48(void)
{
}

/* Sets flag 0x37. */
ADDRESS(0x801e8a50, 0x14)
void func_801e8a50(void)
{
    DAT_801ba984.unk32[0x37] = 1;
}

ADDRESS(0x801e8a64, 0x8)
void func_801e8a64(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8a6c, 0x14)
void func_801e8a6c(void)
{
    DAT_801ba984.unk84[5] = 0x1b;
}
