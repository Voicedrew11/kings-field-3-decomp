/*
 * FDAT_071.BIN (WIP module): the handler table, its slot 0 handler, a key
 * handler (slot 2), empty handlers and the load handler (slot 5).
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
void func_801e8398(VECTOR *pos, SVECTOR *angle);
void func_801e85e4(void);
void func_801e85ec(VECTOR *pos, SVECTOR *angle, s32 item);
void func_801e86e4(void);
void func_801e86ec(void);
void func_801e86f4(void);
void func_801e86fc(void);
void func_801e8704(void);
void func_801e870c(void);
void func_801e8714(void);
void func_801e871c(void);
void func_801e8724(void);
void func_801e872c(void);

const FdatHeader fdat_header = {
    0x1c,
    {
        func_801e8398, func_801e85e4, func_801e85ec, func_801e86f4,
        func_801e8724, func_801e872c, func_801e871c, func_801e8390,
        func_801e86fc, func_801e8704, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e870c, func_801e8714, func_801e86e4, func_801e86ec,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x3c,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e8398, 0x24c)
void func_801e8398(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    for (i = 0; (i = func_80045c7c(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        if ((def->unk0[0] == 9 || def->unk0[0] == 0x15) && obj->unk3c != 0) {
            if (obj->unk3a.index == 0xffff) {
            message:
                if (obj->unk3f != 0) {
                    if (obj->unk3f < 35) {
                        func_80041eec(obj->unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7) {
                        func_80041eec(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        func_80041eec(0x1e);
                    } else if (obj->unk6 == 0x145) {
                        func_80041eec(0x1f);
                    } else if (obj->unk6 == 0x146) {
                        func_80041eec(0x20);
                    } else if (obj->unk6 == 0xf5) {
                        func_80041eec(0xe);
                    } else if (obj->unk6 == 0xba || obj->unk6 == 0x142 || obj->unk6 == 0x143 || obj->unk6 == 0x144) {
                        func_80041eec(0x22);
                    } else if (obj->unk6 >= 190 && obj->unk6 < 198) {
                        func_80041eec(8);
                    }
                }
            } else {
                link = &DAT_8018fb3c.objects[obj->unk3a.index];
                if (link->unk6 == 0xff) {
                    goto message;
                }
                link->unk0[0] = obj->unk40;
                link->unk38 = 0xff;
                func_8005db30(link);
                link->unk0[0] = 0;
                link->unk38 = 0;
                if (link->unk6 == 0xff) {
                    obj->unk3a.index = 0xffff;
                }
            }
        }
    }
}

ADDRESS(0x801e85e4, 0x8)
void func_801e85e4(void)
{
}

/* Slot 2: item 0x86 used on object 0x13f in front unlocks it. */
ADDRESS(0x801e85ec, 0xf8)
void func_801e85ec(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;
    Object44 *obj;

    if (item != 0x86) {
        return;
    }
    i = func_80045c7c(0, pos, 800, 0x6a4, angle->vy, 0x200);
    if (i == -1) {
        return;
    }
    obj = &DAT_8018fb3c.objects[i];
    if (obj->unk6 == 0x13f) {
        if (obj->unk38 != 0xff) {
            return;
        }
        obj->unk38 = item;
        obj->unk8 = 0;
        DAT_801ba984.unk0 = 1;
        func_8005d7f8(0x86);
        func_8005c7e8(obj, 0x86);
    } else if (func_80046608(obj, 0x86) == 3) {
        func_80041eec(7);
        DAT_801ba984.unk0 = 1;
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

ADDRESS(0x801e86f4, 0x8)
void func_801e86f4(void)
{
}

ADDRESS(0x801e86fc, 0x8)
void func_801e86fc(void)
{
}

ADDRESS(0x801e8704, 0x8)
void func_801e8704(void)
{
}

ADDRESS(0x801e870c, 0x8)
void func_801e870c(void)
{
}

ADDRESS(0x801e8714, 0x8)
void func_801e8714(void)
{
}

ADDRESS(0x801e871c, 0x8)
void func_801e871c(void)
{
}

ADDRESS(0x801e8724, 0x8)
void func_801e8724(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e872c, 0x14)
void func_801e872c(void)
{
    DAT_801ba984.unk84[5] = 0x17;
}
