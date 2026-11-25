/*
 * FDAT_008.BIN (WIP module): the handler table and the switch tables of its
 * slot 9 and slot 4 handlers, an empty default handler, its slot 0 handler,
 * a gate handler (slot 1), an item handler (slot 2), a record handler (slot
 * 19), a switch handler (slot 3), the kind 0xa6 handlers (slots 8 and 9), an
 * item check (slot 6), event handlers (slots 10 and 7), a record event
 * handler (slot 4) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0xb8)

void func_801e83c0(void);
void func_801e83c8(VECTOR *pos, SVECTOR *angle);
void func_801e864c(void);
void func_801e86f4(VECTOR *pos, SVECTOR *angle, s32 item);
void func_801e8824(void);
void func_801e882c(Object88 *obj);
void func_801e888c(Object44 *obj);
void func_801e89d0(Object44 *obj, Object44Def *def);
void func_801e89fc(void);
void func_801e8bdc(void);
void func_801e8be4(void);
void func_801e8bec(void);
void func_801e8c1c(u8 id);
void func_801e8c3c(u8 id);
void func_801e8e08(Object88 *record, s32 event);
void func_801e8eb4(void);

const FdatHeader fdat_header = {
    0x7,
    {
        func_801e83c8, func_801e864c, func_801e86f4, func_801e888c,
        func_801e8e08, func_801e8eb4, func_801e8bec, func_801e8c3c,
        func_801e89d0, func_801e89fc, func_801e8c1c, func_801e83c0,
        func_801e83c0, func_801e83c0, func_801e83c0, func_801e83c0,
        func_801e8bdc, func_801e8be4, func_801e8824, func_801e882c,
        func_801e83c0, func_801e83c0, func_801e83c0, func_801e83c0,
        func_801e83c0, func_801e83c0, func_801e83c0, func_801e83c0,
        func_801e83c0, func_801e83c0, func_801e83c0, func_801e83c0,
    },
    0x27,
};

ADDRESS(0x801e83c0, 0x8)
void func_801e83c0(void)
{
}

/*
 * Slot 0: fdat_801e83b0's shape with this module's message table; a kind
 * 0xa6 definition in front advances its object's unk38 (0..5) once.
 */
ADDRESS(0x801e83c8, 0x284)
void func_801e83c8(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    for (i = 0; (i = func_80045c7c(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        switch (def->unk0) {
        case 9:
        case 0x15:
            if (obj->arg.b.unk3c != 0) {
                if (obj->arg.b.unk3a.index == 0xffff) {
                message:
                    if (obj->arg.b.unk3f != 0) {
                        if (obj->arg.b.unk3f < 35) {
                            func_80041eec(obj->arg.b.unk3f);
                        }
                        if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7) {
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
            break;
        case 0xa6:
            if (obj->unk8 == 0) {
                obj->unk8 = 1;
                obj->arg.b.unk38 = (obj->arg.b.unk38 + 1) % 6;
            }
            break;
        }
    }
}

/*
 * Slot 1: unless the player's unkf4 is 0x4b, raises objects 100..116 (when
 * object 82 is not in state 1); otherwise lets object 82 close.
 */
ADDRESS(0x801e864c, 0xa8)
void func_801e864c(void)
{
    Object44 *gate;
    Object44 *obj;
    s32 i;

    if (DAT_801b24e4.unkf4 != 0x4b) {
        gate = &DAT_8018fb3c.objects[82];
        if (DAT_8018fb3c.objects[82].arg.b.unk40 != 1) {
            for (obj = &DAT_8018fb3c.objects[100], i = 16; i != -1; i--, obj++) {
                if (obj->arg.b.unk3f != 1) {
                    obj->arg.b.unk3f = 1;
                    obj->unk8 = 1;
                }
            }
        }
        gate->arg.b.unk40 = 1;
    } else {
        obj = &DAT_8018fb3c.objects[82];
        if (DAT_8018fb3c.objects[82].unk8 == 0 && DAT_8018fb3c.objects[82].arg.b.unk40 != 0) {
            DAT_8018fb3c.objects[82].unk8 = 4;
        }
        obj->arg.b.unk40 = 0;
    }
}

/* Slot 2: items 0x68 and 0x6b..0x6d, in the order flag 0x14 asks for, start record 4. */
ADDRESS(0x801e86f4, 0x130)
void func_801e86f4(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;

    switch (item) {
    case 0x68:
    case 0x6b:
    case 0x6c:
    case 0x6d:
        if (DAT_801ba984.unke[6] != item) {
            return;
        }
        i = func_8004d644(pos->vx, pos->vy, pos->vz, 0x578, 0x6a4);
        if (i != -1 && i == 4) {
            switch (item) {
            case 0x68:
                DAT_801ba984.unke[6] = 1;
                break;
            case 0x6c:
                DAT_801ba984.unke[6] = 2;
                break;
            case 0x6d:
                DAT_801ba984.unke[6] = 3;
                break;
            case 0x6b:
                DAT_801ba984.unke[6] = 4;
                break;
            }
            func_8005d7f8(item);
            func_8005c308(&DAT_80185da8.objects[i]);
            func_800463bc(0x9a);
            DAT_801ba984.unk0 = 1;
        }
        break;
    }
}

ADDRESS(0x801e8824, 0x8)
void func_801e8824(void)
{
}

/* Slot 19: record 4 sets flag 0x19 and starts object 82. */
ADDRESS(0x801e882c, 0x60)
void func_801e882c(Object88 *obj)
{
    if (obj - DAT_80185da8.objects == 4) {
        DAT_801ba984.unke[0xb] = 1;
        DAT_8018fb3c.objects[82].unk8 = 1;
        DAT_8018fb3c.objects[82].arg.b.unk38 = 3;
    }
}

/* Slot 3: switches 0 and 1 arm and disarm object 44; switch 10 shows one of two notes. */
ADDRESS(0x801e888c, 0x144)
void func_801e888c(Object44 *obj)
{
    s32 k;

    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unke[8] == 0) {
            k = 44;
            if (DAT_8018fb3c.objects[k].arg.b.unk38 != 0x9a) {
                DAT_8018fb3c.objects[k].arg.b.unk38 = 0x9a;
            }
        }
        break;
    case 1:
        k = 44;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0x9a) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        break;
    case 10:
        if (func_80046884(0x38, 0x23, 3, 3, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                func_800445b8(2, 0x4a, 0, 0x38, 0x25, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            func_800445b8(2, 0x4d, 0, 0x38, 0x25, 3, 1, 0, 0x2f);
        }
        break;
    }
}

/* Slot 8: an object made from definition kind 0xa6 takes that kind. */
ADDRESS(0x801e89d0, 0x2c)
void func_801e89d0(Object44 *obj, Object44Def *def)
{
    if (def->unk0 == 0xa6) {
        obj->unk4 = 0xa6;
        obj->unk8 = 1;
        obj->arg.b.unk40 = 0xff;
    }
}

DATA(0x801e8f80, 0x44, ".data")
static u32 DAT_801e8f80[17] = {
    63, 8, 62, 32, 8, 8, 42, 22, 20, 4, 4, 16, 16, 4, 34, 32, 32,
};

/*
 * Slot 9: steps the tested kind 0xa6 object: 1 sounds it and picks its next
 * setting, 2 turns it, 3 raises or lowers objects 100..116 by the setting's
 * bit in DAT_801e8f80, 4 picks the setting again.
 */
ADDRESS(0x801e89fc, 0x1e0)
void func_801e89fc(void)
{
    Object44 *obj = DAT_8018fb3c.unk8858;
    Object44Def *def = DAT_8018fb3c.unk8854;
    Object44 *other;
    s32 bit;
    s32 i;
    s32 j;

    if (obj->unk4 != 0xa6) {
        return;
    }
    switch (obj->unk8) {
    case 0:
        break;
    case 1:
        func_8004482c(obj, def->arg.b.unkf);
        if (obj->arg.b.unk38 != 0) {
            obj->unk1 = obj->arg.b.unk38 - 1;
        } else {
            obj->unk1 = 5;
        }
        obj->unk8 = 2;
        obj->unka = 0;
        break;
    case 2:
        obj->unka += 128;
        if (obj->unka >= 4096) {
            obj->unka = 0xfff;
            obj->unk8 = 3;
        }
        break;
    case 3:
        bit = 1 << (obj->unk1 + 1) % 6;
        other = &DAT_8018fb3c.objects[100];
        for (j = 0, i = 16; i != -1; j++, i--, other++) {
            if (bit & DAT_801e8f80[j]) {
                if (other->arg.b.unk38 == j + 200 && other->arg.b.unk3f != 0) {
                    other->arg.b.unk3f = 0;
                    other->unk8 = 1;
                }
            } else if (other->arg.b.unk3f != 1) {
                other->arg.b.unk3f = 1;
                other->unk8 = 1;
            }
        }
        obj->arg.b.unk40 = 0xff;
        obj->unk8 = 0;
        break;
    case 4:
        if (obj->arg.b.unk38 != 0) {
            obj->unk1 = obj->arg.b.unk38 - 1;
        } else {
            obj->unk1 = 5;
        }
        obj->unk8 = 3;
        break;
    }
}

ADDRESS(0x801e8bdc, 0x8)
void func_801e8bdc(void)
{
}

ADDRESS(0x801e8be4, 0x8)
void func_801e8be4(void)
{
}

/* Slot 6: sets flag 0x15 while item 0x4b is held. */
ADDRESS(0x801e8bec, 0x30)
void func_801e8bec(void)
{
    if (func_8005d7bc(0x4b) != 0) {
        DAT_801ba984.unke[7] = 1;
    }
}

/* Slot 10: event 15 sets the third magic element flag. */
ADDRESS(0x801e8c1c, 0x20)
void func_801e8c1c(u8 id)
{
    if (id == 15) {
        DAT_801ba984.unk71[2] = 1;
    }
}

/* Slot 7: raises a stat, opens objects 70 and 91 over 16 frames each and arms object 44. */
ADDRESS(0x801e8c3c, 0x1cc)
void func_801e8c3c(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    Object44 *obj;
    s32 i;

    DAT_801b24e4.unk34[2] += 20;
    func_80029f1c(2);
    func_80029500();
    obj = &DAT_8018fb3c.objects[70];
    DAT_8018fb3c.objects[70].unk5 = 0x32;
    DAT_8018fb3c.objects[70].unk2 = 5;
    for (i = 15; i != -1; i--) {
        obj->unk10 += 256;
        if (obj->unk10 >= 4095) {
            obj->unk10 = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        func_8002b330(&pos, &angle);
        func_800422b8(&pos, &angle);
    }
    obj->unk6 = 0xff;
    obj = &DAT_8018fb3c.objects[91];
    DAT_8018fb3c.objects[91].unk5 = 0x32;
    DAT_8018fb3c.objects[91].unk2 = 5;
    for (i = 15; i != -1; i--) {
        obj->unk10 += 256;
        if (obj->unk10 >= 4095) {
            obj->unk10 = 0xfff;
        }
        cd_request_wait_idle();
        cd_request_service_vab();
        cd_request_service_stream();
        func_8002b330(&pos, &angle);
        func_800422b8(&pos, &angle);
    }
    obj->unk6 = 0xff;
    func_800608ec(12, 16);
    func_800608ec(12, 17);
    if (DAT_801ba984.unke[8] == 0) {
        DAT_801ba984.unke[8] = 1;
        DAT_80185da8.objects[4].unk0 = 8;
        DAT_8018fb3c.objects[44].arg.b.unk38 = 0xff;
    }
}

/* Slot 4: script events 0..3 set the item flag 0x14 asks for next; 4 and 5 trigger objects. */
ADDRESS(0x801e8e08, 0xac)
void func_801e8e08(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        DAT_801ba984.unke[6] = 0x68;
        break;
    case 1:
        DAT_801ba984.unke[6] = 0x6c;
        break;
    case 2:
        DAT_801ba984.unke[6] = 0x6d;
        break;
    case 3:
        DAT_801ba984.unke[6] = 0x6b;
        break;
    case 4:
        func_8005db30(NULL, 0x56);
        break;
    case 5:
        func_8005db30(NULL, 0x52);
        break;
    }
}

/* Slot 5, called after the module loads: restores the opened objects and counts the visits. */
ADDRESS(0x801e8eb4, 0xcc)
void func_801e8eb4(void)
{
    s32 k;

    if (DAT_801ba984.unk7f[2] != 0) {
        DAT_8018fb3c.objects[70].unk6 = 0xff;
        DAT_8018fb3c.objects[91].unk6 = 0xff;
        DAT_801ba984.unke[8] = 1;
    }
    if (DAT_801ba984.unke[8] != 0) {
        DAT_80185da8.objects[4].unk0 = 8;
        DAT_8018fb3c.objects[44].arg.b.unk38 = 0xff;
    } else {
        k = 44;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 != 0x9a) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0x9a;
        }
    }
    if (DAT_801ba984.unke[0xb] != 0) {
        DAT_8018fb3c.objects[82].unk8 = 1;
        DAT_8018fb3c.objects[82].arg.b.unk38 = 3;
    }
    if (DAT_801ba984.unke[0xa] < 2) {
        DAT_801ba984.unke[0xa]++;
    }
    DAT_801ba984.unk84[5] = 2;
}
