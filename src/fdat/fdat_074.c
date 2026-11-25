/*
 * FDAT_074.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, a flag setter and the load handler (slot 5).
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
void func_801e85e0(void);
void func_801e85e8(void);
void func_801e85f0(void);
void func_801e85f8(void);
void func_801e8600(void);
void func_801e8608(void);
void func_801e8610(void);
void func_801e8618(void);
void func_801e8620(void);
void func_801e8634(void);
void func_801e863c(void);
void func_801e8644(void);

const FdatHeader fdat_header = {
    0x1d,
    {
        func_801e8398, func_801e85d0, func_801e85d8, func_801e85f0,
        func_801e863c, func_801e8644, func_801e8618, func_801e8634,
        func_801e85f8, func_801e8600, func_801e8620, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8608, func_801e8610, func_801e85e0, func_801e85e8,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x3d,
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

    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        if ((def->unk0 == 9 || def->unk0 == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        notify_enqueue(obj->arg.b.unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7) {
                        notify_enqueue(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        notify_enqueue(0x1e);
                    } else if (obj->unk6 == 0x13f) {
                        notify_enqueue(0x1f);
                    } else if (obj->unk6 == 0x140) {
                        notify_enqueue(0x20);
                    } else if (obj->unk6 == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->unk6 == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->unk6 >= 190 && obj->unk6 < 198) {
                        notify_enqueue(8);
                    }
                }
            } else {
                link = &DAT_8018fb3c.objects[obj->arg.b.unk3a.index];
                if (link->unk6 == 0xff) {
                    goto message;
                }
                link->unk0 = obj->arg.b.unk40;
                link->arg.b.unk38 = 0xff;
                func_8005db30(link);
                link->unk0 = 0;
                link->arg.b.unk38 = 0;
                if (link->unk6 == 0xff) {
                    obj->arg.b.unk3a.index = 0xffff;
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

ADDRESS(0x801e85f8, 0x8)
void func_801e85f8(void)
{
}

ADDRESS(0x801e8600, 0x8)
void func_801e8600(void)
{
}

ADDRESS(0x801e8608, 0x8)
void func_801e8608(void)
{
}

ADDRESS(0x801e8610, 0x8)
void func_801e8610(void)
{
}

ADDRESS(0x801e8618, 0x8)
void func_801e8618(void)
{
}

/* Sets flag 0x2f. */
ADDRESS(0x801e8620, 0x14)
void func_801e8620(void)
{
    DAT_801ba984.unk32[0x2f] = 1;
}

ADDRESS(0x801e8634, 0x8)
void func_801e8634(void)
{
}

ADDRESS(0x801e863c, 0x8)
void func_801e863c(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8644, 0x2c)
void func_801e8644(void)
{
    if (DAT_801ba984.unk32[0x37] == 0) {
        DAT_801ba984.unk32[0x2f] = 0;
    }
    DAT_801ba984.unk84[5] = 0x18;
}
