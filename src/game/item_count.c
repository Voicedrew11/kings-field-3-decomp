/*
 * GAME.EXE: item counts (WIP module): the two counts per id in
 * DAT_800c85e8, a timed screen tint fade and a brightening loop. The large
 * functions func_8005db30 and func_8005e2d0 follow (not attempted).
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
