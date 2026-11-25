/*
 * FDAT_053.BIN (WIP module): the handler table and the switch table of its
 * slot 19 handler, an empty default handler, the shared slot 0 handler, a
 * facing check (slot 1), an item handler (slot 2), the shared record run
 * fragment, an object state helper, a record handler (slot 19), a switch
 * handler (slot 3), event handlers (slots 10 and 7), a record event handler
 * (slot 4) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x140)

void func_801e8448(void);
void func_801e8658(void);
void func_801e86f8(VECTOR *pos, SVECTOR *angle, s32 item);
void func_801e891c(void);
void func_801e8958(s32 index, s32 state);
void func_801e89c0(Object88 *obj);
void func_801e8b10(Object44 *obj);
void func_801e8d24(void);
void func_801e8d2c(void);
void func_801e8d34(void);
void func_801e8d3c(void);
void func_801e8d44(void);
void func_801e8d4c(u8 id);
void func_801e8de8(u8 id);
void func_801e8e08(Object88 *record, s32 event);
void func_801e8f84(void);

const FdatHeader fdat_header = {
    0x16,
    {
        fdat_801e83b0, func_801e8658, func_801e86f8, func_801e8b10,
        func_801e8e08, func_801e8f84, func_801e8d44, func_801e8de8,
        func_801e8d24, func_801e8d2c, func_801e8d4c, func_801e8448,
        func_801e8448, func_801e8448, func_801e8448, func_801e8448,
        func_801e8d34, func_801e8d3c, func_801e891c, func_801e89c0,
        func_801e8448, func_801e8448, func_801e8448, func_801e8448,
        func_801e8448, func_801e8448, func_801e8448, func_801e8448,
        func_801e8448, func_801e8448, func_801e8448, func_801e8448,
    },
    0x36,
};

ADDRESS(0x801e8448, 0x8)
void func_801e8448(void)
{
}

#include "shared/fdat_801e83b0.inc"

/* Slot 1: with flag 0x55 at 1, record 15 stops when it faces away from the player. */
ADDRESS(0x801e8658, 0xa0)
void func_801e8658(void)
{
    s32 i = 15;

    if (DAT_801ba984.unk32[0x23] == 1 && DAT_80185da8.objects[i].unk9 != 1) {
        if (angle_within_tolerance(DAT_801b24e4.unk124.vy,
                                   vector_xz_to_angle(DAT_80185da8.objects[i].pos.vx - DAT_801b24e4.pos.vx,
                                                      DAT_80185da8.objects[i].pos.vz - DAT_801b24e4.pos.vz),
                                   600) == 0) {
            DAT_80185da8.objects[i].unk9 = 0;
            DAT_80185da8.objects[i].unk0 = 2;
        }
    }
    DAT_80185da8.objects[56].unk3 = 0;
}

/*
 * Slot 2: item 0x8f used on object 11 in front spawns an object of that
 * kind at the player and moves it onto object 11 over 64 frames.
 */
ADDRESS(0x801e86f8, 0x224)
void func_801e86f8(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;
    Object44 *obj;
    Object44 *target;
    VECTOR from;
    VECTOR to;
    s32 t;
    u16 id;

    if (item != 0x8f || DAT_801ba984.unk32[0x25] != 0) {
        return;
    }
    for (i = 0; (i = func_80045c7c(i, pos, 800, 0x6a4, angle->vy, 0x200)) != -1; i++) {
        if (i != 11) {
            continue;
        }
        target = &DAT_8018fb3c.objects[11];
        obj = func_80046034(0x15e, 10, DAT_8018fb3c.unk885e);
        func_800448b8(obj);
        id = DAT_8018fb3c.unk885e++;
        obj->unke = -100;
        obj->unk0 = 2;
        obj->arg.h.unk3c = id;
        obj->unk6 = item;
        obj->unk4 = 0xff;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = DAT_8018fb3c.objects[i].angle.vy;
        from = DAT_801b24e4.pos;
        obj->pos = from;
        obj->pos.vy -= 1330;
        obj->arg.b.unk38 = 0;
        to = target->pos;
        to.vy -= 50;
        for (t = 0; t < 4096; t += 64) {
            func_8005bee0(obj, &from, &to, NULL, NULL, t);
            cd_request_service_vab();
            cd_request_service_stream();
            func_800422b8(NULL, NULL);
        }
        DAT_801ba984.unk32[0x25] = 1;
        DAT_801ba984.unk32[0x26] = 0;
        func_8005d7f8(item);
        DAT_801ba984.unk0 = 1;
    }
}

ADDRESS(0x801e891c, 0x8)
void func_801e891c(void)
{
}

#include "shared/fdat_801e86d8.inc"

/* Sets object index's unk3f to state (0 or 1) and marks it changed. */
ADDRESS(0x801e8958, 0x68)
void func_801e8958(s32 index, s32 state)
{
    Object44 *obj;

    switch (state) {
    case 0:
        obj = &DAT_8018fb3c.objects[index];
        obj->arg.b.unk3f = 0;
        obj->unk8 = 1;
        break;
    case 1:
        obj = &DAT_8018fb3c.objects[index];
        obj->arg.b.unk3f = 1;
        obj->unk8 = 1;
        break;
    }
}

/* Slot 19: records 11..13 each start their record run once, 15 and 56 move on. */
ADDRESS(0x801e89c0, 0x150)
void func_801e89c0(Object88 *obj)
{
    switch (obj - DAT_80185da8.objects) {
    case 11:
        if (DAT_801ba984.unk32[0x20] == 0) {
            DAT_801ba984.unk32[0x20] = 1;
            fdat_801e86d8(0x10, 1);
            func_801e8958(0x50, 1);
        }
        break;
    case 12:
        if (DAT_801ba984.unk32[0x21] == 0) {
            DAT_801ba984.unk32[0x21] = 1;
            fdat_801e86d8(0x37, 1);
            func_801e8958(0x51, 1);
        }
        break;
    case 13:
        if (DAT_801ba984.unk32[0x22] == 0) {
            DAT_801ba984.unk32[0x22] = 1;
            fdat_801e86d8(0x2e, 1);
            func_801e8958(5, 1);
        }
        break;
    case 15:
        if (DAT_801ba984.unk32[0x23] == 1) {
            DAT_801ba984.unk32[0x23] = 2;
            DAT_80185da8.objects[15].unk0 = 8;
        }
        break;
    case 56:
        DAT_80185da8.objects[56].unk0 = 1;
        DAT_80185da8.objects[56].unk3 = 0;
        break;
    }
}

/*
 * Slot 3: switch 0 sets flag 0x55, switch 1 trades item 0x38 for 0x39 behind
 * a fade, switches 0x61 and 0x63 hurt the player.
 */
ADDRESS(0x801e8b10, 0x214)
void func_801e8b10(Object44 *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unk32[0x23] == 0) {
            DAT_801ba984.unk32[0x23] = 1;
        }
        break;
    case 1:
        if (func_8005d7bc(0x38) > 0) {
            func_8005d948(0x200, 0, 0, 0, 100, 100, 100);
            if (DAT_801b24e4.unkf2 == 0x38) {
                func_8002bb84(0x39, 4);
            }
            func_8005d7f8(0x38);
            func_8005d898(0x39);
            func_8005d948(0x200, 100, 100, 100, 0, 0, 0);
        }
        break;
    case 0x61:
        if (DAT_801ba984.unk32[0x20] == 0) {
            func_8002ab18(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        if (DAT_801ba984.unk32[0x21] == 0) {
            func_8002ab18(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        if (DAT_801ba984.unk32[0x22] == 0) {
            func_8002ab18(0, 0, 0, 0x40, 0, 0x63, 0, 0, 0, 0, 0, NULL);
        }
        break;
    case 0x63:
        func_8002ab18(0, 0, 0, 0x43, 0, 0, 0, 0, 0, 0, 0, &DAT_801b24e4.pos);
        break;
    }
}

ADDRESS(0x801e8d24, 0x8)
void func_801e8d24(void)
{
}

ADDRESS(0x801e8d2c, 0x8)
void func_801e8d2c(void)
{
}

ADDRESS(0x801e8d34, 0x8)
void func_801e8d34(void)
{
}

ADDRESS(0x801e8d3c, 0x8)
void func_801e8d3c(void)
{
}

ADDRESS(0x801e8d44, 0x8)
void func_801e8d44(void)
{
}

/* Slot 10: event 9 closes object 50, opens objects 52..61 and shows a note. */
ADDRESS(0x801e8d4c, 0x9c)
void func_801e8d4c(u8 id)
{
    Object44 *obj;
    s32 i;

    if (id == 9) {
        obj = &DAT_8018fb3c.objects[50];
        obj->unk0 = 0;
        obj += 2;
        for (i = 9; i != -1; i--) {
            obj->unk0 = 2;
            obj++;
        }
        func_800445b8(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk9 = 0;
        DAT_80185da8.objects[56].unk0 = 2;
    }
}

/* Slot 7: event 9 sets flag 0x58. */
ADDRESS(0x801e8de8, 0x20)
void func_801e8de8(u8 id)
{
    if (id == 9) {
        DAT_801ba984.unk32[0x26] = 1;
    }
}

/* Slot 4: script events 0 and 1 of an object record. */
ADDRESS(0x801e8e08, 0x17c)
void func_801e8e08(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        if (DAT_801ba984.unk32[0x24] != 1) {
            DAT_801ba984.unk32[0x24] = 1;
            func_8005db30(NULL, 0x29);
            func_8002bb84(0x29, 0);
            func_8005db30(NULL, 0x31);
            func_8002bb84(0x31, 1);
            func_8005db30(NULL, 0x46);
            func_8002bb84(0x46, 2);
            func_8005db30(NULL, 0x4f);
            func_8002bb84(0x4f, 3);
            func_8005db30(NULL, 0x3c);
            func_8002bb84(0x3c, 4);
            DAT_8018fb3c.objects[63].unk0 = 2;
            DAT_8018fb3c.objects[63].arg.b.unk3f = 0xff;
        }
        break;
    case 1:
        if (DAT_801ba984.unk32[0x25] == 1) {
            DAT_801ba984.unk32[0x25] = 2;
            func_8005d948(0x200, 0, 0, 0, 200, 200, 200);
            func_8005d948(0x200, 200, 200, 200, 0, 0, 0);
            DAT_801b24e4.unk34[4] += 30;
            func_80029f1c(4);
            func_80029500();
            DAT_80185da8.objects[56].unk0 = 8;
            DAT_80185da8.objects[56].unk3 = 0;
        }
        break;
    }
}

/* Slot 5, called after the module loads: restores the objects of the flags. */
ADDRESS(0x801e8f84, 0x374)
void func_801e8f84(void)
{
    Object44 *obj;
    Object44 *p;
    s32 i;
    u16 id;

    if (DAT_801ba984.unk32[0x20] != 0) {
        fdat_801e86d8(0x10, 1);
        func_801e8958(0x50, 1);
    } else {
        func_801e8958(0x50, 0);
    }
    if (DAT_801ba984.unk32[0x21] != 0) {
        fdat_801e86d8(0x37, 1);
        func_801e8958(0x51, 1);
    } else {
        func_801e8958(0x51, 0);
    }
    if (DAT_801ba984.unk32[0x22] != 0) {
        fdat_801e86d8(0x2e, 1);
        func_801e8958(5, 1);
    } else {
        func_801e8958(5, 0);
    }
    DAT_80185da8.objects[15].unk0 = 8;
    if (DAT_801ba984.unk32[0x24] != 0) {
        DAT_8018fb3c.objects[63].unk0 = 2;
        DAT_8018fb3c.objects[63].arg.b.unk3f = 0xff;
    } else {
        DAT_8018fb3c.objects[63].unk0 = 0;
        DAT_8018fb3c.objects[63].arg.b.unk3f = 0;
    }
    if (DAT_801ba984.unk32[0x25] == 0) {
        p = &DAT_8018fb3c.objects[50];
        p->unk0 = 2;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->unk0 = 0;
            p++;
        }
        func_800445b8(2, 0x38, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk0 = 8;
        DAT_80185da8.objects[56].unk3 = 0;
        DAT_8018fb3c.objects[11].unk0 = 2;
    } else if (DAT_801ba984.unk32[0x25] == 1) {
        p = &DAT_8018fb3c.objects[50];
        p->unk0 = 0;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->unk0 = 2;
            p++;
        }
        func_800445b8(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk0 = 1;
        DAT_80185da8.objects[56].unk3 = 0;
        DAT_8018fb3c.objects[11].unk0 = 2;
        obj = func_80046034(0x15e, 10, DAT_8018fb3c.unk885e);
        func_800448b8(obj);
        id = DAT_8018fb3c.unk885e++;
        obj->unke = -100;
        obj->unk6 = 0x8f;
        obj->arg.h.unk3c = id;
        obj->unk0 = 2;
        obj->unk4 = 0xff;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = DAT_8018fb3c.objects[11].angle.vy;
        obj->arg.b.unk38 = 0;
        obj->pos.vx = DAT_8018fb3c.objects[11].pos.vx;
        obj->pos.vy = DAT_8018fb3c.objects[11].pos.vy - 50;
        obj->pos.vz = DAT_8018fb3c.objects[11].pos.vz;
    } else {
        p = &DAT_8018fb3c.objects[50];
        p->unk0 = 0;
        p += 2;
        for (i = 9; i != -1; i--) {
            p->unk0 = 2;
            p++;
        }
        func_800445b8(2, 0x3b, 0x4c, 0x47, 0x30, 3, 4, 0, 0x2f);
        DAT_80185da8.objects[56].unk0 = 8;
        DAT_80185da8.objects[56].unk3 = 0;
        DAT_8018fb3c.objects[11].unk0 = 0;
    }
    DAT_801ba984.unk32[0x26] = 1;
    DAT_801ba984.unk84[5] = 0x11;
}
