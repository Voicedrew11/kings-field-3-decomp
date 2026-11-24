/*
 * FDAT_038.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, records 50..52 handlers (slots 18 and 19) and the load handler
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
void func_801e8398(VECTOR *pos, SVECTOR *angle);
void func_801e85d0(void);
void func_801e85d8(void);
void func_801e85e0(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,;
void func_801e866c(Object88 *obj);
void func_801e875c(void);
void func_801e8764(void);
void func_801e876c(void);
void func_801e8774(void);
void func_801e877c(void);
void func_801e8784(void);
void func_801e878c(void);
void func_801e8794(void);

const FdatHeader fdat_header = {
    0x11,
    {
        func_801e8398, func_801e85d0, func_801e85d8, func_801e875c,
        func_801e878c, func_801e8794, func_801e8784, func_801e8390,
        func_801e8764, func_801e876c, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8774, func_801e877c, func_801e85e0, func_801e866c,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x31,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e8398, 0x238)
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
                    } else if (obj->unk6 == 0x132) {
                        func_80041eec(0x1f);
                    } else if (obj->unk6 == 0x133) {
                        func_80041eec(0x20);
                    } else if (obj->unk6 == 0xf5) {
                        func_80041eec(0xe);
                    } else if (obj->unk6 == 0xba) {
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

ADDRESS(0x801e85d0, 0x8)
void func_801e85d0(void)
{
}

ADDRESS(0x801e85d8, 0x8)
void func_801e85d8(void)
{
}

/* Slot 18: records 50..52 report their unk1a while the last four arguments are 0. */
ADDRESS(0x801e85e0, 0x8c)
void func_801e85e0(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
                   s32 arg7, s32 arg8, s32 arg9)
{
    s32 index = obj - DAT_80185da8.objects;

    if (index == 50 || index == 51 || index == 52) {
        if (arg6 == 0 && arg7 == 0 && arg8 == 0 && arg9 == 0) {
            *out = obj->unk1a;
        } else {
            *out = 0;
        }
    }
}

/* Slot 19: records 50..52 each set their flag and start their object once. */
ADDRESS(0x801e866c, 0xf0)
void func_801e866c(Object88 *obj)
{
    s32 index = obj - DAT_80185da8.objects;

    switch (index) {
    case 50:
        if (DAT_801ba984.unk32[0x10] == 0) {
            DAT_801ba984.unk32[0x10] = 1;
            func_800463bc(0x64);
        }
        break;
    case 51:
        if (DAT_801ba984.unk32[0x11] == 0) {
            DAT_801ba984.unk32[0x11] = 1;
            func_800463bc(0x65);
        }
        break;
    case 52:
        if (DAT_801ba984.unk32[0x12] == 0) {
            DAT_801ba984.unk32[0x12] = 1;
            func_800463bc(0x66);
        }
        break;
    }
}

ADDRESS(0x801e875c, 0x8)
void func_801e875c(void)
{
}

ADDRESS(0x801e8764, 0x8)
void func_801e8764(void)
{
}

ADDRESS(0x801e876c, 0x8)
void func_801e876c(void)
{
}

ADDRESS(0x801e8774, 0x8)
void func_801e8774(void)
{
}

ADDRESS(0x801e877c, 0x8)
void func_801e877c(void)
{
}

ADDRESS(0x801e8784, 0x8)
void func_801e8784(void)
{
}

ADDRESS(0x801e878c, 0x8)
void func_801e878c(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8794, 0xa8)
void func_801e8794(void)
{
    if (DAT_801ba984.unk32[0x10] == 1 && DAT_8018fb3c.objects[241].unk38 == 0x64) {
        DAT_8018fb3c.objects[241].unk38 = 0xff;
    }
    if (DAT_801ba984.unk32[0x11] == 1 && DAT_8018fb3c.objects[242].unk38 == 0x65) {
        DAT_8018fb3c.objects[242].unk38 = 0xff;
    }
    if (DAT_801ba984.unk32[0x12] == 1 && DAT_8018fb3c.objects[243].unk38 == 0x66) {
        DAT_8018fb3c.objects[243].unk38 = 0xff;
    }
    DAT_801ba984.unk84[5] = 0xc;
}
