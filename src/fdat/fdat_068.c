/*
 * FDAT_068.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, an event handler (slot 4) and the load handler (slot 5).
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
void func_801e85f4(void);
void func_801e85fc(void);
void func_801e8604(void);
void func_801e860c(void);
void func_801e8614(void);
void func_801e861c(void);
void func_801e8624(void);
void func_801e862c(void);
void func_801e8634(void);
void func_801e863c(void);
void func_801e8644(Object88 *record, s32 event);
void func_801e8680(void);

const FdatHeader fdat_header = {
    0x1b,
    {
        func_801e8398, func_801e85f4, func_801e85fc, func_801e8614,
        func_801e8644, func_801e8680, func_801e863c, func_801e8390,
        func_801e861c, func_801e8624, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e862c, func_801e8634, func_801e8604, func_801e860c,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x3b,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e8398, 0x25c)
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
                    } else if (obj->unk6 == 0x141) {
                        notify_enqueue(0x1f);
                    } else if (obj->unk6 == 0x142) {
                        notify_enqueue(0x20);
                    } else if (obj->unk6 == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->unk6 == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->unk6 == 0x13c || obj->unk6 == 0x13d || obj->unk6 == 0x13e) {
                        notify_enqueue(0x21);
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

ADDRESS(0x801e85f4, 0x8)
void func_801e85f4(void)
{
}

ADDRESS(0x801e85fc, 0x8)
void func_801e85fc(void)
{
}

ADDRESS(0x801e8604, 0x8)
void func_801e8604(void)
{
}

ADDRESS(0x801e860c, 0x8)
void func_801e860c(void)
{
}

ADDRESS(0x801e8614, 0x8)
void func_801e8614(void)
{
}

ADDRESS(0x801e861c, 0x8)
void func_801e861c(void)
{
}

ADDRESS(0x801e8624, 0x8)
void func_801e8624(void)
{
}

ADDRESS(0x801e862c, 0x8)
void func_801e862c(void)
{
}

ADDRESS(0x801e8634, 0x8)
void func_801e8634(void)
{
}

ADDRESS(0x801e863c, 0x8)
void func_801e863c(void)
{
}

/* Slot 4: script events 0 and 1 of an object record set flags 0x2d and 0x2e. */
ADDRESS(0x801e8644, 0x3c)
void func_801e8644(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        DAT_801ba984.unk32[0x2d] = 1;
        break;
    case 1:
        DAT_801ba984.unk32[0x2e] = 1;
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8680, 0x7c)
void func_801e8680(void)
{
    if (DAT_801ba984.unk32[0x2e] == 0) {
        DAT_80185da8.objects[25].unk0 = 1;
        DAT_8018fb3c.objects[28].unk0 = 0;
        DAT_8018fb3c.objects[28].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[95].arg.b.unk3d[1] = 0;
    } else {
        DAT_80185da8.objects[25].unk0 = 8;
        DAT_8018fb3c.objects[28].unk0 = 2;
        DAT_8018fb3c.objects[28].arg.b.unk3c = 0xff;
        DAT_8018fb3c.objects[95].arg.b.unk3d[1] = 0xff;
    }
    DAT_801ba984.unk84[5] = 0x16;
}
