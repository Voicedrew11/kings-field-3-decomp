/*
 * GAME.EXE: item counts (WIP module): the two counts per id in
 * DAT_800c85e8, a timed screen tint fade and a brightening loop, and the
 * item pick-up shown in front of the viewer.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/image.h>
#include <game/data.h>
#include <game/render.h>
#include <game/player.h>
#include <game/misc.h>
#include <game/object44.h>
#include <psyq/libc.h>
#include <stdarg.h>

/* Non-zero when either count of id is. */
ADDRESS(0x8005d7bc, 0x3c)
s32 func_8005d7bc(s32 id)
{
    if (DAT_800c85e8[0][id] == 0) {
        return DAT_800c85e8[1][id] != 0;
    }
    return 1;
}

/* Takes one of id (first count first); 1 when there is none. */
ADDRESS(0x8005d7f8, 0xa0)
s32 func_8005d7f8(s32 id)
{
    if (DAT_800c85e8[0][id] != 0) {
        DAT_800c85e8[0][id]--;
    } else if (DAT_800c85e8[1][id] != 0) {
        DAT_800c85e8[1][id]--;
    } else {
        return 1;
    }
    DAT_8018fad4.handlers[11]();
    return 0;
}

/* Adds one of id (each count holds 99); 1 and message 11 when both are full. */
ADDRESS(0x8005d898, 0xb0)
s32 func_8005d898(s32 id)
{
    if (DAT_800c85e8[0][id] < 99) {
        DAT_800c85e8[0][id]++;
    } else if (DAT_800c85e8[1][id] < 99) {
        DAT_800c85e8[1][id]++;
    } else {
        func_80041eec(11);
        return 1;
    }
    DAT_8018fad4.handlers[6]();
    return 0;
}

/* Fades the screen tint from (r0, g0, b0) to (r1, g1, b1), step per frame out of 4096. */
ADDRESS(0x8005d948, 0x14c)
void func_8005d948(s32 step, s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1)
{
    s32 t;
    s32 r;
    s32 g;

    t = 0;
    do {
        func_800341e8();
        func_80034180();
        func_80034300();
        r = func_80017158(r0, r1, t);
        g = func_80017158(g0, g1, t);
        func_8003d8ec(r, g, func_80017158(b0, b1, t), 0x800, 1);
        t += step;
        func_80030e14();
        func_800422b8(NULL, NULL);
    } while (t < 4096);
    func_800341e8();
    func_80034180();
    func_80034300();
    func_8003d8ec(r1, g1, b1, 0x800, 1);
    func_80030e14();
    func_800422b8(NULL, NULL);
}

/* Brightens the screen over 33 frames, then sets DAT_8009c3f8 to 1. */
ADDRESS(0x8005da94, 0x9c)
void func_8005da94(void)
{
    VECTOR pos;
    SVECTOR angle;
    s32 i;
    s32 v;

    i = 0;
    for (;;) {
        v = i * i >> 16;
        if (v >= 256) {
            v = 255;
        }
        func_8003d624(0x82, v, v, v);
        func_80015a48();
        func_80018cd0();
        func_8002b330(&pos, &angle);
        func_800422b8(&pos, &angle);
        if (i >= 4096) {
            break;
        }
        i += 128;
    }
    DAT_8009c3f8 = 1;
}

/*
 * Picks up obj: lifts it in front of the viewer (levelling the view pitch)
 * and spins it until a button press; the pick-up button (DAT_80081868[7]) or
 * a forced id adds it to the item counts, else it turns back and returns to
 * its place. obj NULL spawns a record for the def id given after it.
 * pad doubles as the taken flag after the press. This module passes
 * func_80017170 both angles narrowed to s16 (an s16 prototype in its view;
 * the callee narrows them again).
 */
ADDRESS(0x8005db30, 0x7a0)
void func_8005db30(Object44 *obj, ...)
{
    va_list args;
    SVECTOR angle_to;
    SVECTOR angle_from;
    VECTOR to;
    VECTOR from;
    u8 *model;
    Object44Def *def;
    Object44 *link;
    Object44 *driver;
    s32 id;
    s32 pitch;
    s32 start;
    s16 spin;
    s16 held;
    s32 model_id;
    s32 t;
    u32 pad;
    u32 prev;
    s32 n;

    id = -1;
    if (obj == NULL) {
        va_start(args, obj);
        id = va_arg(args, s32);
        va_end(args);
        obj = func_80046034(350, 10, DAT_8018fb3c.unk885e);
        func_800448b8(obj);
        obj->unk6 = id;
        obj->unk0 = 2;
        obj->unk4 = 0xff;
        obj->angle.vz = 0;
        obj->angle.vy = 0;
        obj->angle.vx = 0;
        obj->arg.b.unk38 = 0xff;
        obj->pos = DAT_801b24e4.pos;
        obj->pos.vx += rcos(DAT_80199154.unk15b08.vy - 3072) * 1024 >> 12;
        obj->pos.vz += rsin(DAT_80199154.unk15b08.vy - 3072) * 1024 >> 12;
        obj->pos.vy -= 1024;
    }
    def = &DAT_8018fb3c.defs[obj->unk6];
    spin = obj->unke;
    if (obj->arg.b.unk38 != 0xff) {
        return;
    }
    if (def->unk1 == 0x20) {
        func_80041eec(25, obj->arg.h.unk3a);
        obj->unk6 = 0xff;
        DAT_801b24e4.unk50 += obj->arg.h.unk3a;
        return;
    }
    if (obj->unk4 == 0x70 && obj->unk6 == 0x86) {
        driver = &DAT_8018fb3c.objects[obj->arg.h.unk40];
        link = &DAT_8018fb3c.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->unk4 == 2 && link->unk8 != 0) {
            return;
        }
    } else if (obj->unk4 == 0x70 && obj->unk6 == 0x8d) {
        driver = &DAT_8018fb3c.objects[obj->arg.h.unk40];
        link = &DAT_8018fb3c.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->unk4 == 2 && link->unk8 != 0) {
            return;
        }
    } else if (obj->unk4 == 0x70 && obj->unk6 == 0x88) {
        driver = &DAT_8018fb3c.objects[obj->arg.h.unk40];
        link = &DAT_8018fb3c.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->unk4 == 2 && link->unk8 != 0) {
            return;
        }
    } else if (obj->unk6 == 0x85 && DAT_8018fad4.area[0] == 7) {
        if (!func_80016a2c(DAT_801b24e4.unk12c.vy + 2048, obj->angle.vy, 256)) {
            return;
        }
    }
    pitch = DAT_801b24e4.unk124.vx & 0xfff;
    start = pitch;
    if (pitch < 2048) {
        if (pitch > 0) {
            pitch = 0;
        }
    } else if (pitch < 4095) {
        pitch = 4095;
    }
    prev = PadRead(1);
    held = def->arg.sh.unk10;
    angle_from = obj->angle;
    from = obj->pos;
    func_8005be28(0, 500, 1500, pitch, DAT_801b24e4.unk124.vy, def->arg.sh.unkc, def->arg.sh.unke, &to);
    angle_to.vz = 0;
    angle_to.vx = 0;
    angle_to.vy = (DAT_801b24e4.unk124.vy + 2048) & 0xfff;
    model_id = func_80040568(obj->unk6);
    func_800405e8(model_id, &model);
    while (model == NULL) {
        func_80015a48();
        func_80018cd0();
        func_800422b8(NULL, &DAT_801b24e4.unk124);
        func_800405e8(model_id, &model);
    }
    for (t = 0; t <= 4096; t += 512) {
        pad = PadRead(1);
        if (prev == 0 && pad != 0) {
            goto pressed;
        }
        prev = pad;
        func_8005bee0(obj, &from, &to, &angle_from, &angle_to, t);
        obj->unke = func_80015fdc(spin, held, t >> 4);
        DAT_801b24e4.unk124.vx = func_80017170((s16)start, (s16)pitch, t);
        func_80015a48();
        func_80018cd0();
        func_800422b8(NULL, &DAT_801b24e4.unk124);
    }
    obj->unke = held;
    for (;;) {
        pad = PadRead(1);
        if (prev == 0 && pad != 0) {
            break;
        }
        prev = pad;
        obj->angle.vy += 64;
        func_80015a48();
        func_80018cd0();
        func_800422b8(NULL, &DAT_801b24e4.unk124);
    }
pressed:
    if ((pad & DAT_80081868[7]) || id != -1) {
        if (func_8005d898(obj->unk6) == 0) {
            switch (obj->unk6) {
            case 0x93:
                n = (rand() * 6 >> 15) + 4;
                if (DAT_800c85e8[0][0x93] < 99 - n) {
                    DAT_800c85e8[0][0x93] += n;
                } else {
                    DAT_800c85e8[0][0x93] = 99;
                }
                break;
            case 0x94:
                DAT_800c85e8[0][0x94] = DAT_800c85e8[0][0x94] < 95 ? DAT_800c85e8[0][0x94] + 4 : 99;
                break;
            }
            pad = 1;
            func_8005be28(-500, 500, 0, pitch, DAT_801b24e4.unk124.vy, 0, 0, &from);
            angle_from = obj->angle;
            spin = 0;
            goto back;
        }
        if (id != -1) {
            obj->unk6 = 0xff;
            func_8002b6f0();
            return;
        }
    }
    pad = 0;
    for (;;) {
        if (func_80016a2c(obj->angle.vy, (DAT_801b24e4.unk124.vy + 2048) & 0xfff, 128)) {
            obj->angle.vy = (DAT_801b24e4.unk124.vy + 2048) & 0xfff;
            break;
        }
        obj->angle.vy = (obj->angle.vy + 256) & 0xfff;
        func_800422b8(NULL, &DAT_801b24e4.unk124);
    }
back:
    angle_to = obj->angle;
    pitch = DAT_801b24e4.unk124.vx;
    to = obj->pos;
    for (t = 4096; t >= 0; t -= 512) {
        func_8005bee0(obj, &from, &to, &angle_from, &angle_to, t);
        DAT_801b24e4.unk124.vx = func_80017170((s16)start, (s16)pitch, t);
        obj->unke = func_80015fdc(spin, held, t >> 4);
        func_800422b8(NULL, &DAT_801b24e4.unk124);
    }
    if (pad) {
        obj->unk6 = 0xff;
    }
    func_8002b6f0();
}
