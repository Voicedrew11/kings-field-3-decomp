/*
 * FDAT_020.BIN (WIP module): the handler table and the switch table of its
 * slot 4 handler, the shared light-record blend, the light helper, its slot
 * 0 handler, empty handlers, a record handler (slot 19), a switch handler
 * (slot 3), an item check (slot 6), a record event handler (slot 4) and the
 * load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0xa0)

void func_801e848c(u8 n);
void func_801e8534(void);
void func_801e853c(VECTOR *pos, SVECTOR *angle);
void func_801e874c(void);
void func_801e8754(void);
void func_801e875c(void);
void func_801e8764(Object88 *obj);
void func_801e87a8(Object44 *obj);
void func_801e88e4(void);
void func_801e88ec(void);
void func_801e88f4(void);
void func_801e88fc(void);
void func_801e8904(void);
void func_801e8934(Object88 *record, s32 event);
void func_801e8a80(void);

const FdatHeader fdat_header = {
    0xb,
    {
        func_801e853c, func_801e874c, func_801e8754, func_801e87a8,
        func_801e8934, func_801e8a80, func_801e8904, func_801e8534,
        func_801e88e4, func_801e88ec, func_801e8534, func_801e8534,
        func_801e8534, func_801e8534, func_801e8534, func_801e8534,
        func_801e88f4, func_801e88fc, func_801e875c, func_801e8764,
        func_801e8534, func_801e8534, func_801e8534, func_801e8534,
        func_801e8534, func_801e8534, func_801e8534, func_801e8534,
        func_801e8534, func_801e8534, func_801e8534, func_801e8534,
    },
    0x2b,
};

#include "shared/fdat_801e83a4.inc"

DATA(0x801e8ce8, 0x30, ".data")
static Record30 DAT_801e8ce8 = {
    {{{2500, -1250, 1250}, {-2500, -1250, -1250}, {0, 2500, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    9000,
    18000,
};
DATA(0x801e8d18, 0x30, ".data")
static Record30 DAT_801e8d18 = {
    {{{2000, -1000, 1000}, {-2000, -1000, -1000}, {0, 2000, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    9000,
    18000,
};
DATA(0x801e8d48, 0x30, ".data")
static Record30 DAT_801e8d48 = {
    {{{1500, -750, 750}, {-1500, -750, -750}, {0, 1500, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    9000,
    18000,
};
DATA(0x801e8d78, 0x30, ".data")
static Record30 DAT_801e8d78 = {
    {{{3000, -1500, 1500}, {-3000, -1500, -1500}, {0, 3000, 0}}, 0},
    {{{2048, 2304, 3072}, {2048, 2304, 3072}, {2048, 2304, 3072}}, 0},
    {0x80, 0x80, 0x80, 0},
    18000,
    24000,
};

/* Blends area light records 42..44 by n sixteenths of the way to the lit state. */
ADDRESS(0x801e848c, 0xa8)
void func_801e848c(u8 n)
{
    s32 i;
    s32 t;

    for (i = 0, t = 0; i < n; i++) {
        t += 256;
    }
    fdat_801e83a4(&DAT_801e8ce8, &DAT_801e8d78, &DAT_80081c8c[42], t);
    fdat_801e83a4(&DAT_801e8d18, &DAT_801e8d78, &DAT_80081c8c[43], t);
    fdat_801e83a4(&DAT_801e8d48, &DAT_801e8d78, &DAT_80081c8c[44], t);
}

ADDRESS(0x801e8534, 0x8)
void func_801e8534(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e853c, 0x210)
void func_801e853c(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    for (i = 0; (i = func_80045c7c(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        if ((def->unk0 == 9 || def->unk0 == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        func_80041eec(obj->arg.b.unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7 ||
                        obj->unk6 == 0x133) {
                        func_80041eec(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        func_80041eec(0x1e);
                    } else if (obj->unk6 == 0xf5) {
                        func_80041eec(0xe);
                    } else if (obj->unk6 == 0xba) {
                        func_80041eec(0x22);
                    } else if (obj->unk6 >= 190 && obj->unk6 < 198) {
                        func_80041eec(8);
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

ADDRESS(0x801e874c, 0x8)
void func_801e874c(void)
{
}

ADDRESS(0x801e8754, 0x8)
void func_801e8754(void)
{
}

ADDRESS(0x801e875c, 0x8)
void func_801e875c(void)
{
}

/* Slot 19: record 3 moves on. */
ADDRESS(0x801e8764, 0x44)
void func_801e8764(Object88 *obj)
{
    if (obj - DAT_80185da8.objects == 3) {
        obj->unk0 = 1;
    }
}

/* Slot 3: switch 0 shows one of two notes by the player's unkf4; switch 10 one of two notes. */
ADDRESS(0x801e87a8, 0x13c)
void func_801e87a8(Object44 *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801b24e4.unkf4 == 0x4b) {
            func_800445b8(2, 0x4a, 5, 0x3a, 0x35, 6, 2, 0, 0x2f);
        } else {
            func_800445b8(2, 0x44, 5, 0x3a, 0x35, 6, 2, 0, 0x2f);
        }
        break;
    case 10:
        if (func_80046884(0x1f, 0x41, 7, 0xc, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                func_800445b8(2, 0x4a, 4, 0x21, 0x41, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            func_800445b8(2, 0x4d, 4, 0x21, 0x41, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e88e4, 0x8)
void func_801e88e4(void)
{
}

ADDRESS(0x801e88ec, 0x8)
void func_801e88ec(void)
{
}

ADDRESS(0x801e88f4, 0x8)
void func_801e88f4(void)
{
}

ADDRESS(0x801e88fc, 0x8)
void func_801e88fc(void)
{
}

/* Slot 6: sets flag 0x28 while item 0x83 is held. */
ADDRESS(0x801e8904, 0x30)
void func_801e8904(void)
{
    if (func_8005d7bc(0x83) > 0) {
        DAT_801ba984.unke[0x1a] = 1;
    }
}

/* Slot 4: script events 0..5 of an object record. */
ADDRESS(0x801e8934, 0x14c)
void func_801e8934(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        DAT_801ba984.unke[0x18] = 1;
        break;
    case 1:
        func_8005db30(NULL, 0x80);
        break;
    case 2:
        DAT_801ba984.unke[0x17] = 1;
        break;
    case 3:
        func_8005db30(NULL, 0x83);
        DAT_801ba984.unke[0x1a] = 1;
        break;
    case 4:
        func_8005d948(0x200, 0, 0, 0, 0x80, 0xc8, 0x80);
        func_8005d948(0x200, 0x80, 0xc8, 0x80, 0, 0, 0);
        func_8003da10(0, 0, 0, 0, 1);
        DAT_801ba984.unk2b = 1;
        DAT_801b77ec.unk0[56].unk0 = 1;
        DAT_801b77ec.unk0[58].unk0 = 1;
        DAT_801b77ec.unk0[60].unk0 = 1;
        DAT_801b77ec.unk0[62].unk0 = 1;
        DAT_801b77ec.unk0[64].unk0 = 1;
        DAT_801b77ec.unk0[76].unk0 = 1;
        break;
    case 5:
        func_8005db30(NULL, 10);
        break;
    }
}

/* Slot 5, called after the module loads: restores the records and the area light. */
ADDRESS(0x801e8a80, 0x268)
void func_801e8a80(void)
{
    Object88 *obj;
    s32 i;

    if (DAT_801b24e4.unk32 >= 50 && DAT_801b24e4.unk34[0] >= 50 && DAT_801b24e4.unk34[1] >= 50 &&
        DAT_801b24e4.unk34[2] >= 50 && DAT_801b24e4.unk34[3] >= 50 && DAT_801b24e4.unk34[4] >= 50) {
        DAT_801ba984.unke[0x1c] = 1;
    }
    if (DAT_801ba984.unke[0x19] == 0) {
        func_801e848c(0);
        DAT_8018fb3c.objects[121].unk0 = 0;
        DAT_8018fb3c.objects[121].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[173].arg.b.unk3d[1] = 0;
        DAT_80185da8.objects[3].unk0 = 8;
        DAT_80185da8.objects[0].unk0 = 1;
        DAT_80185da8.objects[1].unk0 = 8;
        for (i = 0x46, obj = &DAT_80185da8.objects[4]; i != -1; i--, obj++) {
            obj->unk0 = 1;
        }
    } else if (DAT_801ba984.unk32[4] == 0) {
        DAT_801ba984.unke[0x1b] = 1;
        func_801e848c(16);
        DAT_8018fb3c.objects[121].unk0 = 0;
        DAT_8018fb3c.objects[121].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[173].arg.b.unk3d[1] = 0;
        DAT_8018fb3c.objects[91].unk0 = 0;
        DAT_80185da8.objects[3].unk0 = 1;
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 1;
        for (i = 0x46, obj = &DAT_80185da8.objects[4]; i != -1; i--, obj++) {
            obj->unk0 = 8;
        }
    } else {
        func_801e848c(0);
        DAT_8018fb3c.objects[121].unk0 = 2;
        DAT_8018fb3c.objects[121].arg.b.unk3c = 0xff;
        DAT_8018fb3c.objects[173].arg.b.unk3d[1] = 0xff;
        DAT_8018fb3c.objects[91].unk0 = 0;
        DAT_80185da8.objects[3].unk0 = 8;
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 1;
        for (i = 0x46, obj = &DAT_80185da8.objects[4]; i != -1; i--, obj++) {
            obj->unk0 = 1;
        }
    }
    if (DAT_801ba984.unk2c[0] == 0) {
        DAT_80185da8.objects[58].unk0 = 8;
    } else {
        DAT_8018fb3c.objects[83].unk0 = 0;
    }
    DAT_801ba984.unk84[5] = 6;
}
