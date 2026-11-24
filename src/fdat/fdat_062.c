/*
 * FDAT_062.BIN (WIP module): the handler table, the shared slot 0 handler,
 * an item handler (slot 2), empty handlers, the load handler (slot 5) and
 * two door handlers (slots 7 and 10).
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
void func_801e8658(void);
void func_801e8660(void);
void func_801e8668(void);
void func_801e8670(void);
void func_801e8678(void);
void func_801e8680(void);
void func_801e8688(void);
void func_801e8690(void);
void func_801e8698(u8 id);
void func_801e8730(u8 id);
void func_801e8828(void);
void func_801e8830(void);

const FdatHeader fdat_header = {
    0x19,
    {
        fdat_801e83b0, func_801e85a0, func_801e85a8, func_801e8668,
        func_801e8828, func_801e8830, func_801e8690, func_801e8730,
        func_801e8670, func_801e8678, func_801e8698, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8680, func_801e8688, func_801e8658, func_801e8660,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x39,
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

/* Slot 2: item 0x7e opens the lock when item 0x7d is held. */
ADDRESS(0x801e85a8, 0xb0)
void func_801e85a8(VECTOR *pos, SVECTOR *angle, s32 item)
{
    if (item != 0x7e) {
        return;
    }
    if (func_80046884(0x26, 0x10, 3, 2, 0x8000) == 0) {
        return;
    }
    if (func_8005d7bc(0x7d) > 0) {
        if (DAT_801ba984.unk32[0x28] != 0) {
            return;
        }
        DAT_801ba984.unk32[0x28] = 1;
        DAT_801ba984.unk32[0x29] = 0;
        func_8005d7f8(0x7e);
    } else {
        if (DAT_801ba984.unk32[0x28] != 0) {
            return;
        }
        func_80041eec(0x10);
    }
    DAT_801ba984.unk0 = 1;
}

ADDRESS(0x801e8658, 0x8)
void func_801e8658(void)
{
}

ADDRESS(0x801e8660, 0x8)
void func_801e8660(void)
{
}

ADDRESS(0x801e8668, 0x8)
void func_801e8668(void)
{
}

ADDRESS(0x801e8670, 0x8)
void func_801e8670(void)
{
}

ADDRESS(0x801e8678, 0x8)
void func_801e8678(void)
{
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

/* Slot 10. */
ADDRESS(0x801e8698, 0x98)
void func_801e8698(u8 id)
{
    if (id == 5) {
        func_800445b8(2, 0x4d, 0, 0x26, 12, 3, id, 0, 0x2f);
    }
    if (id == 10) {
        if (func_8005d7bc(0x7d) > 0) {
            func_8005d7f8(0x7d);
            func_8005d898(0x10);
            DAT_801ba984.unk32[0x2b] = 1;
        }
    }
}

/* Slot 7: event 10 opens object 2 over 16 frames. */
ADDRESS(0x801e8730, 0xf8)
void func_801e8730(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    Object44 *obj;
    s32 i;

    if (id == 5) {
        DAT_801ba984.unk32[0x29] = 1;
    }
    if (id == 10) {
        obj = &DAT_8018fb3c.objects[2];
        DAT_801ba984.unk32[0x2a] = 1;
        DAT_8018fb3c.objects[2].unk5 = 0x32;
        DAT_8018fb3c.objects[2].unk0[2] = 5;
        for (i = 15; i != -1; i--) {
            obj->unk10 += 256;
            if (obj->unk10 >= 4095) {
                obj->unk10 = 0xfff;
            }
            func_80019ab4();
            func_80015a48();
            func_80018cd0();
            func_8002b330(&pos, &angle);
            func_800422b8(&pos, &angle);
        }
        obj->unk6 = 0xff;
    }
}

ADDRESS(0x801e8828, 0x8)
void func_801e8828(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8830, 0xa8)
void func_801e8830(void)
{
    DAT_801ba984.unk32[0x29] = 1;
    if (DAT_801ba984.unk32[0x28] == 0) {
        func_800445b8(2, 0x4a, 0, 0x26, 12, 3, 5, 0, 0x2f);
    } else {
        func_800445b8(2, 0x4d, 0, 0x26, 12, 3, 5, 0, 0x2f);
    }
    if (func_8005d7bc(0x7d) > 0) {
        DAT_801ba984.unk32[0x2a] = 0;
    } else {
        DAT_801ba984.unk32[0x2a] = 1;
    }
    DAT_801ba984.unk84[5] = 0x14;
}
