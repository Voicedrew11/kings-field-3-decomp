/*
 * FDAT_029.BIN (WIP module): the shared light-record blend, the handler
 * table, its slot 0 handler, empty handlers, a switch handler (slot 3), a
 * record event handler (slot 4) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x88)

void func_801e8474(void);
void func_801e847c(VECTOR *pos, SVECTOR *angle);
void func_801e86f4(void);
void func_801e86fc(void);
void func_801e8704(void);
void func_801e870c(void);
void func_801e8714(Object44 *obj);
void func_801e8a68(void);
void func_801e8a70(void);
void func_801e8a78(void);
void func_801e8a80(void);
void func_801e8a88(void);
void func_801e8a90(Object88 *record, s32 event);
void func_801e8acc(void);

const FdatHeader fdat_header = {
    0xe,
    {
        func_801e847c, func_801e86f4, func_801e86fc, func_801e8714,
        func_801e8a90, func_801e8acc, func_801e8a88, func_801e8474,
        func_801e8a68, func_801e8a70, func_801e8474, func_801e8474,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
        func_801e8a78, func_801e8a80, func_801e8704, func_801e870c,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
    },
    0x2e,
};

#include "shared/fdat_801e83a4.inc"

ADDRESS(0x801e8474, 0x8)
void func_801e8474(void)
{
}

/*
 * Slot 0: first sets flag 0x3a (and starts sound 0xf1) once item 0x7d is
 * held or the level reaches 50, then fdat_801e83b0's shape with this
 * module's message table.
 */
ADDRESS(0x801e847c, 0x278)
void func_801e847c(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    if (DAT_801ba984.unk32[8] == 0) {
        if (func_8005d7bc(0x7d) > 0 || DAT_801b24e4.level >= 50) {
            DAT_801ba984.unk32[8] = 1;
            func_800466e8(0xf1);
        }
    }
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
                    } else if (obj->unk6 == 0xf5) {
                        func_80041eec(0xe);
                    } else if (obj->unk6 == 0xba) {
                        func_80041eec(0x22);
                    } else if (obj->unk6 == 312 || obj->unk6 == 313 || obj->unk6 == 0x13a) {
                        func_80041eec(0x21);
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

/*
 * Slot 3: switch 0 (with flag 0x38 set) opens objects 249 and 262 and starts
 * record 2; switch 1 strikes around object 58 until the four element flags
 * are set, then dissolves it over 16 frames; switch 10 shows one of two notes.
 */
ADDRESS(0x801e8714, 0x354)
void func_801e8714(Object44 *obj)
{
    VECTOR pos;
    SVECTOR angle;
    s32 i;
    Object44 *other;

    switch (obj->unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unk32[6] == 1 && func_80046884(0x36, 0xf, 10, 4, 0x8000) == 0) {
            DAT_80185da8.objects[2].unk0 = 8;
            DAT_8018fb3c.objects[249].unk0[0] = 2;
            DAT_8018fb3c.objects[249].unk3c = 0xff;
            DAT_8018fb3c.objects[262].unk3d[1] = 0xff;
        }
        break;
    case 1:
        if (DAT_801ba984.unk32[7] == 0) {
            if (DAT_801ba984.unk7f[0] != 0 && DAT_801ba984.unk7f[1] != 0 &&
                DAT_801ba984.unk7f[2] != 0 && DAT_801ba984.unk7f[3] != 0) {
                DAT_801ba984.unk32[7] = 1;
            }
            obj = &DAT_8018fb3c.objects[58];
            obj->unk0[0] = 2;
            if (DAT_801b24e4.unk101 == 0x10) {
                func_80015654(0x28, &obj->pos, 0x78, 0);
            }
            func_8002b114(&obj->pos, 0, 0x1000, 0x8000, 0x1000, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                          0x40, 0x40, 0x40, 0x400, 10);
        } else if (DAT_801ba984.unk32[7] == 1) {
            other = &DAT_8018fb3c.objects[58];
            other->unk0[0] = 2;
            DAT_8018fb3c.objects[58].unk5 = 0x32;
            DAT_8018fb3c.objects[58].unk0[2] = 5;
            DAT_8018fb3c.objects[58].unk10 = 0;
            for (i = 15; i != -1; i--) {
                other->unk10 += 256;
                if (other->unk10 >= 4095) {
                    other->unk10 = 0xfff;
                }
                func_80019ab4();
                func_80015a48();
                func_80018cd0();
                func_8002b330(&pos, &angle);
                func_800422b8(&pos, &angle);
            }
            other->unk6 = 0xff;
            other->unk4 = 0xff;
            other->unk0[0] = 0;
            DAT_801ba984.unk32[7] = 2;
        } else {
            DAT_8018fb3c.objects[58].unk6 = 0xff;
            DAT_8018fb3c.objects[58].unk4 = 0xff;
            DAT_8018fb3c.objects[58].unk0[0] = 0;
        }
        break;
    case 10:
        if (func_80046884(0x24, 0x36, 7, 0xc, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                func_800445b8(2, 0x2a, 0x4f, 0x26, 0x36, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            func_800445b8(2, 0x2d, 0x4f, 0x26, 0x36, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8a68, 0x8)
void func_801e8a68(void)
{
}

ADDRESS(0x801e8a70, 0x8)
void func_801e8a70(void)
{
}

ADDRESS(0x801e8a78, 0x8)
void func_801e8a78(void)
{
}

ADDRESS(0x801e8a80, 0x8)
void func_801e8a80(void)
{
}

ADDRESS(0x801e8a88, 0x8)
void func_801e8a88(void)
{
}

/* Slot 4: script events 0 and 1 of an object record set flags 0x37 and 0x38. */
ADDRESS(0x801e8a90, 0x3c)
void func_801e8a90(Object88 *record, s32 event)
{
    switch (event) {
    case 0:
        DAT_801ba984.unk32[5] = 1;
        break;
    case 1:
        DAT_801ba984.unk32[6] = 1;
        break;
    }
}

/* Slot 5, called after the module loads: restores the switch states. */
ADDRESS(0x801e8acc, 0x140)
void func_801e8acc(void)
{
    if (DAT_801ba984.unke[0x1a] != 0 && DAT_801ba984.unk2b != 0) {
        DAT_801ba984.unk32[4] = 1;
    }
    if (DAT_801ba984.unk32[6] == 0) {
        DAT_80185da8.objects[2].unk0 = 1;
        DAT_8018fb3c.objects[249].unk0[0] = 0;
        DAT_8018fb3c.objects[249].unk3c = 0;
        DAT_8018fb3c.objects[262].unk3d[1] = 0;
    } else {
        DAT_80185da8.objects[2].unk0 = 8;
        DAT_8018fb3c.objects[249].unk0[0] = 2;
        DAT_8018fb3c.objects[249].unk3c = 0xff;
        DAT_8018fb3c.objects[262].unk3d[1] = 0xff;
    }
    if (DAT_801ba984.unk32[7] != 0) {
        DAT_8018fb3c.objects[58].unk6 = 0xff;
        DAT_8018fb3c.objects[58].unk4 = 0xff;
        DAT_8018fb3c.objects[58].unk0[0] = 0;
        DAT_801ba984.unk32[7] = 2;
    }
    if (DAT_801ba984.unk32[8] == 0) {
        if (func_8005d7bc(0x7d) > 0 || DAT_801b24e4.level >= 40) {
            DAT_801ba984.unk32[8] = 1;
            func_800466e8(0xf1);
        }
    }
    DAT_801ba984.unk84[5] = 9;
}
