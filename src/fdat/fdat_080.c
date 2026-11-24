/*
 * FDAT_080.BIN (WIP module): the shared light-record blend, the handler
 * table, the shared slot 0 handler, empty handlers, a flag check (slot 1),
 * the lamp helper and records 40..51 (slot 18), a switch handler (slot 3),
 * an event handler (slot 7), an event handler (slot 4) and the load handler
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

void func_801e8474(void);
void func_801e8684(void);
void func_801e86bc(void);
void func_801e86c4(void);
void func_801e8758(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6, s32 arg7, s32 arg8, s32 arg9);
void func_801e8850(void);
void func_801e8858(Object44 *obj);
void func_801e8910(void);
void func_801e8918(void);
void func_801e8920(void);
void func_801e8928(void);
void func_801e8930(void);
void func_801e8938(u8 id);
void func_801e8958(u8 id);
void func_801e8af0(Object88 *record, s32 event);
void func_801e8b38(void);

const FdatHeader fdat_header = {
    0x1f,
    {
        fdat_801e83b0, func_801e8684, func_801e86bc, func_801e8858,
        func_801e8af0, func_801e8b38, func_801e8930, func_801e8958,
        func_801e8910, func_801e8918, func_801e8938, func_801e8474,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
        func_801e8920, func_801e8928, func_801e8758, func_801e8850,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
        func_801e8474, func_801e8474, func_801e8474, func_801e8474,
    },
    0x3f,
};

#include "shared/fdat_801e83a4.inc"

ADDRESS(0x801e8474, 0x8)
void func_801e8474(void)
{
}

#include "shared/fdat_801e83b0.inc"

/* Slot 1: sets flag 0x31 once the player's unk34[3] is non-zero. */
ADDRESS(0x801e8684, 0x38)
void func_801e8684(void)
{
    if (DAT_801b24e4.unk34[3] != 0) {
        if (DAT_801ba984.unk32[0x31] == 0) {
            DAT_801ba984.unk32[0x31] = 1;
        }
    }
}

ADDRESS(0x801e86bc, 0x8)
void func_801e86bc(void)
{
}

DATA(0x801e8bb8, 0x30, ".data")
static Record30 DAT_801e8bb8 = {
    {{{3000, 800, -1500}, {-2000, 1500, -3400}, {-1300, -2000, 1024}}, 0},
    {{{128, 128, 128}, {128, 128, 128}, {128, 128, 128}}, 0},
    {0x50, 0x50, 0x50, 0},
    6000,
    12000,
};
DATA(0x801e8be8, 0x30, ".data")
static Record30 DAT_801e8be8 = {
    {{{3000, 800, -1500}, {-2000, 1500, -3400}, {-1300, -2000, 1024}}, 0},
    {{{2662, 2662, 2662}, {2662, 2662, 2662}, {2662, 2662, 2662}}, 0},
    {0x78, 0x78, 0x78, 0},
    6000,
    18000,
};

/* Lights the effects of the twelve lamp bits and blends the area light by their count. */
ADDRESS(0x801e86c4, 0x94)
void func_801e86c4(void)
{
    u32 bits;
    Effect *effect;
    s32 i;
    s32 t;

    bits = DAT_801ba984.unk32[0x34] | DAT_801ba984.unk32[0x35] << 8;
    effect = DAT_80182968;
    for (i = 0, t = 0; i < 12; effect++, i++, bits >>= 1) {
        if (bits & 1) {
            effect->unk2 = 2;
            t += 320;
        } else {
            effect->unk2 = 0;
        }
    }
    fdat_801e83a4(&DAT_801e8bb8, &DAT_801e8be8, &DAT_80081c8c[41], t);
}

/* Slot 18: records 40..51 each light their lamp bit. */
ADDRESS(0x801e8758, 0xf8)
void func_801e8758(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
                   s32 arg7, s32 arg8, s32 arg9)
{
    s32 index = obj - DAT_80185da8.objects - 40;
    s32 bit;

    if (index >= 0 && index < 12) {
        DAT_801ba984.unk32[0x34] |= bit = 1 << index;
        DAT_801ba984.unk32[0x35] |= bit >> 8;
        *out = 0;
        if (arg6 == 0) {
            return;
        }
        func_801e86c4();
        func_800341e8();
        func_80034300();
        func_80030e14();
    }
    if ((*(u32 *)&DAT_801ba984.unk32[0x32] & 0xffff0000) == 0x0fff0000) {
        if (DAT_801ba984.unk32[0x36] == 0) {
            DAT_801ba984.unk32[0x36] = 1;
            func_8005bd34();
        }
        func_800466e8(0xf1);
    }
}

ADDRESS(0x801e8850, 0x8)
void func_801e8850(void)
{
}

/* Slot 3: switch objects 0..2. */
ADDRESS(0x801e8858, 0xb8)
void func_801e8858(Object44 *obj)
{
    switch (obj->unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unk32[0x30] != 1) {
            DAT_801ba984.unk32[0x30] = 1;
        }
        break;
    case 1:
        if (DAT_801ba984.unk32[0x33] != 1) {
            DAT_801ba984.unk32[0x33] = 1;
            func_80029500();
        }
        break;
    case 2:
        if (DAT_801ba984.unk32[0x33] != 0) {
            DAT_801ba984.unk32[0x33] = 0;
            func_80029500();
        }
        break;
    }
}

ADDRESS(0x801e8910, 0x8)
void func_801e8910(void)
{
}

ADDRESS(0x801e8918, 0x8)
void func_801e8918(void)
{
}

ADDRESS(0x801e8920, 0x8)
void func_801e8920(void)
{
}

ADDRESS(0x801e8928, 0x8)
void func_801e8928(void)
{
}

ADDRESS(0x801e8930, 0x8)
void func_801e8930(void)
{
}

/* Slot 7. */
ADDRESS(0x801e8938, 0x20)
void func_801e8938(u8 id)
{
    if (id == 14) {
        DAT_801ba984.unk71[3] = 1;
    }
}

/* Slot 7: raises a stat and opens objects 6 and 87 over 16 frames each. */
ADDRESS(0x801e8958, 0x198)
void func_801e8958(u8 id)
{
    VECTOR pos;
    SVECTOR angle;
    Object44 *obj;
    s32 i;

    DAT_801b24e4.unk34[3] += 20;
    func_80029f1c(3);
    func_80029500();
    obj = &DAT_8018fb3c.objects[6];
    DAT_8018fb3c.objects[6].unk5 = 0x32;
    DAT_8018fb3c.objects[6].unk0[2] = 5;
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
    obj = &DAT_8018fb3c.objects[87];
    DAT_8018fb3c.objects[87].unk5 = 0x32;
    DAT_8018fb3c.objects[87].unk0[2] = 5;
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
    func_800608ec(9, 0);
    func_800608ec(9, 1);
}

/* Slot 4: script event 0 of an object record sets flag 0x32 once. */
ADDRESS(0x801e8af0, 0x48)
void func_801e8af0(Object88 *record, s32 event)
{
    if (event == 0) {
        if (DAT_801ba984.unk32[0x32] == 0) {
            DAT_801ba984.unk32[0x32] = 1;
            func_800463bc(0xf2);
            func_8005bd34();
        }
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8b38, 0x80)
void func_801e8b38(void)
{
    if (DAT_801ba984.unk32[0x32] != 0) {
        func_800463bc(0xf2);
    }
    if (DAT_801ba984.unk7f[3] != 0) {
        DAT_8018fb3c.objects[6].unk6 = 0xff;
        DAT_8018fb3c.objects[87].unk6 = 0xff;
    }
    if (DAT_801ba984.unk32[0x36] != 0) {
        func_800466e8(0xf1);
    }
    func_801e86c4();
    DAT_801ba984.unk84[5] = 0x1a;
}
