/*
 * FDAT_050.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers and its load handler (slot 5).
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
void func_801e85c0(void);
void func_801e85c8(void);
void func_801e85d0(void);
void func_801e85d8(void);
void func_801e85e0(void);
void func_801e85e8(void);
void func_801e85f0(void);
void func_801e85f8(void);

const FdatHeader fdat_header = {
    0x15,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e85c0,
        func_801e85f0, func_801e85f8, func_801e85e8, func_801e8390,
        func_801e85c8, func_801e85d0, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e85d8, func_801e85e0, func_801e85b0, func_801e85b8,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x35,
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

ADDRESS(0x801e85c0, 0x8)
void func_801e85c0(void)
{
}

ADDRESS(0x801e85c8, 0x8)
void func_801e85c8(void)
{
}

ADDRESS(0x801e85d0, 0x8)
void func_801e85d0(void)
{
}

ADDRESS(0x801e85d8, 0x8)
void func_801e85d8(void)
{
}

ADDRESS(0x801e85e0, 0x8)
void func_801e85e0(void)
{
}

ADDRESS(0x801e85e8, 0x8)
void func_801e85e8(void)
{
}

ADDRESS(0x801e85f0, 0x8)
void func_801e85f0(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e85f8, 0x70)
void func_801e85f8(void)
{
    if (DAT_801ba984.unk32[0x24] == 0) {
        func_800445b8(2, 0x13, 0, 12, 1, 1, 3, 0, 0x2f);
    } else {
        func_800445b8(2, 0x14, 0, 12, 1, 1, 3, 0, 0x2f);
    }
    DAT_801ba984.unk84[5] = 0x10;
}
