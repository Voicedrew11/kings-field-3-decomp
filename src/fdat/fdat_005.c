/*
 * FDAT_005.BIN (WIP module): the handler table, an empty default handler,
 * its slot 0 handler, a distance check (slot 1), an item handler (slot 2),
 * a record handler (slot 19), a note handler (slot 3), an event handler
 * (slot 7), a record event handler (slot 4) and the load handler (slot 5).
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
void func_801e85a8(void);
void func_801e8680(VECTOR *pos, SVECTOR *angle, s32 item);
void func_801e8758(void);
void func_801e8760(Object88 *obj);
void func_801e87c4(void);
void func_801e87cc(void);
void func_801e87d4(void);
void func_801e87dc(Object44 *obj);
void func_801e889c(void);
void func_801e88a4(void);
void func_801e88ac(void);
void func_801e88b4(void);
void func_801e88bc(void);
void func_801e88c4(void);
void func_801e88cc(u8 id);
void func_801e88e8(Object88 *record, s32 event);
void func_801e89a8(void);

const FdatHeader fdat_header = {
    0x6,
    {
        func_801e8398, func_801e85a8, func_801e8680, func_801e87dc,
        func_801e88e8, func_801e89a8, func_801e88bc, func_801e88cc,
        func_801e889c, func_801e88a4, func_801e88c4, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e88ac, func_801e88b4, func_801e8758, func_801e8760,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x26,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e8398, 0x210)
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
        if ((def->unk0 == 9 || def->unk0 == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        func_80041eec(obj->arg.b.unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7 ||
                        obj->unk6 == 0x134) {
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

/* Slot 1: once record 20 is more than 20 map cells from the player, flag 0xf moves the records on. */
ADDRESS(0x801e85a8, 0xd8)
void func_801e85a8(void)
{
    s32 dx;
    s32 dz;

    if (DAT_801ba984.unke[0] != 0 && DAT_801ba984.unke[1] == 0) {
        dx = (DAT_80185da8.objects[20].pos.vx - DAT_801b24e4.pos.vx) / 2048;
        dz = (DAT_80185da8.objects[20].pos.vz - DAT_801b24e4.pos.vz) / 2048;
        dx *= dx;
        dz *= dz;
        if (dx + dz > 400) {
            DAT_801ba984.unke[1] = 1;
            DAT_80185da8.objects[20].unk0 = 8;
            DAT_80185da8.objects[10].unk0 = 1;
            DAT_80185da8.objects[25].unk0 = 8;
            DAT_80185da8.objects[23].unk0 = 1;
            DAT_80185da8.objects[24].unk0 = 1;
            DAT_80185da8.objects[40].unk0 = 8;
        }
    }
}

/* Slot 2: item 0x7f used on record 20 starts it and opens object 71. */
ADDRESS(0x801e8680, 0xd8)
void func_801e8680(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;
    s32 k;

    if (item != 0x7f || DAT_801ba984.unke[0] != 0) {
        return;
    }
    i = func_8004d644(pos->vx, pos->vy, pos->vz, 0x578, 0x6a4);
    if (i != -1 && i == 20) {
        func_8005d7f8(0x7f);
        DAT_801ba984.unke[0] = 1;
        func_8005c308(&DAT_80185da8.objects[20]);
        k = 71;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        DAT_80185da8.objects[22].unk0 = 8;
        DAT_80185da8.objects[39].unk0 = 1;
        DAT_801ba984.unk0 = 1;
    }
}

ADDRESS(0x801e8758, 0x8)
void func_801e8758(void)
{
}

/* Slot 19: records 24 and 40 set flag 0x10 once. */
ADDRESS(0x801e8760, 0x64)
void func_801e8760(Object88 *obj)
{
    s32 index = obj - DAT_80185da8.objects;

    if (index == 24 || index == 40) {
        if (DAT_801ba984.unke[2] == 0) {
            DAT_801ba984.unke[2] = 1;
        }
    }
}

ADDRESS(0x801e87c4, 0x8)
void func_801e87c4(void)
{
}

ADDRESS(0x801e87cc, 0x8)
void func_801e87cc(void)
{
}

ADDRESS(0x801e87d4, 0x8)
void func_801e87d4(void)
{
}

/* Slot 3: for an object whose unk3a.bytes[1] is 10, shows one of two notes. */
ADDRESS(0x801e87dc, 0xc0)
void func_801e87dc(Object44 *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (func_80046884(0x1c, 6, 9, 0xb, 0x8000) != 0) {
        if (DAT_801b24e4.unk10a != 0) {
            func_800445b8(2, 0x48, 0, 0x1f, 0x10, 3, 1, 0, 0x2f);
        }
    } else {
        DAT_801b24e4.unk10a = 0;
        func_800445b8(2, 0x4b, 0, 0x1f, 0x10, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e889c, 0x8)
void func_801e889c(void)
{
}

ADDRESS(0x801e88a4, 0x8)
void func_801e88a4(void)
{
}

ADDRESS(0x801e88ac, 0x8)
void func_801e88ac(void)
{
}

ADDRESS(0x801e88b4, 0x8)
void func_801e88b4(void)
{
}

ADDRESS(0x801e88bc, 0x8)
void func_801e88bc(void)
{
}

ADDRESS(0x801e88c4, 0x8)
void func_801e88c4(void)
{
}

/* Slot 7: restores the player's hp. */
ADDRESS(0x801e88cc, 0x1c)
void func_801e88cc(u8 id)
{
    DAT_801b24e4.hp = DAT_801b24e4.hp_max;
}

/* Slot 4: script events 0..2 of an object record. */
ADDRESS(0x801e88e8, 0xc0)
void func_801e88e8(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        if (DAT_801ba984.unke[3] == 0) {
            DAT_801ba984.unke[3] = 1;
            func_800445b8(2, 0x4e, 0, 0x1b, 0x11, 1, 7, 0, 0x2f);
        }
        break;
    case 1:
        func_8005db30(NULL, 0x74);
        break;
    case 2:
        func_8005db30(NULL, 0x72);
        break;
    }
}

/* Slot 5, called after the module loads: restores the records of the flags. */
ADDRESS(0x801e89a8, 0x1d0)
void func_801e89a8(void)
{
    s32 k;

    if (DAT_801ba984.unke[0] != 0) {
        DAT_80185da8.objects[20].unk0 = 8;
        DAT_80185da8.objects[10].unk0 = 1;
        DAT_80185da8.objects[25].unk0 = 8;
        DAT_80185da8.objects[23].unk0 = 1;
        DAT_80185da8.objects[24].unk0 = 1;
        DAT_80185da8.objects[40].unk0 = 8;
        k = 71;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        DAT_80185da8.objects[22].unk0 = 8;
        DAT_80185da8.objects[39].unk0 = 1;
    } else {
        DAT_80185da8.objects[20].unk0 = 1;
        DAT_80185da8.objects[10].unk0 = 8;
        DAT_80185da8.objects[25].unk0 = 1;
        DAT_80185da8.objects[23].unk0 = 8;
        DAT_80185da8.objects[24].unk0 = 1;
        DAT_80185da8.objects[40].unk0 = 8;
        DAT_80185da8.objects[22].unk0 = 1;
        DAT_80185da8.objects[39].unk0 = 8;
    }
    if (DAT_801ba984.unke[0x16] != 0) {
        DAT_80185da8.objects[24].unk0 = 8;
        DAT_80185da8.objects[40].unk0 = 1;
    } else {
        DAT_80185da8.objects[24].unk0 = 1;
        DAT_80185da8.objects[40].unk0 = 8;
    }
    if (DAT_801ba984.unke[2] == 0 && DAT_801ba984.unke[3] != 0) {
        func_800445b8(2, 0x4e, 0, 0x1b, 0x11, 1, 7, 0, 0x2f);
    } else {
        func_800445b8(2, 0x4f, 0, 0x1b, 0x11, 1, 7, 0, 0x2f);
    }
    if (DAT_801ba984.unk32[0xa] == 1) {
        DAT_801ba984.unk32[0xa] = 2;
        DAT_80185da8.objects[42].unk0 = 8;
        DAT_8018fb3c.objects[276].arg.b.unk38 = 0x90;
    } else if (DAT_801ba984.unk32[0xa] == 2) {
        DAT_80185da8.objects[42].unk0 = 8;
    }
    DAT_801ba984.unk84[5] = 1;
}
