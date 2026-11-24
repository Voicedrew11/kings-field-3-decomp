/*
 * GAME.EXE: the area warp picker (WIP module, one function of the menu
 * translation unit). It is split from game.menu because its two position
 * tables are the first constants of the unit's .rdata, ahead of the later
 * pages' constants and every jump table. The menu's $gp globals are defined
 * in game.menu; this slice repeats them as tentative definitions because
 * GCC 2.6 reaches only a translation unit's own small globals through $gp.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/kernel.h>
#include <game/data.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/render.h>
#include <game/card.h>
#include <game/player.h>
#include <game/menu.h>

RODATA(0x800110b0, 0x60)

s32 DAT_8009c230;
s32 DAT_8009c234;
s32 DAT_8009c24c;
s32 DAT_8009c250;
s32 DAT_8009c254;
s32 DAT_8009c258;
s32 DAT_8009c260;
POLY_FT4 *DAT_8009c374;
u8 *DAT_8009c37c;
u8 *DAT_8009c380;

ADDRESS(0x8001c030, 0x45c)
s32 func_8001c030(void)
{
    s32 selected = 0;
    s32 xs[12] = {10, 32, 56, 7, 56, 29, 34, 19, 0, 39, 37, 58};
    s32 ys[12] = {56, 14, 35, 31, 13, 31, 67, 29, 0, 56, 51, 34};
    s32 i;
    s32 bytes;
    s32 n;
    u32 pad;

    if (DAT_8018fad4.unk9 < 12 && DAT_8018fad4.unk9 != 8) {
        selected = DAT_8018fad4.unk9;
    }
    bytes = func_80019cc4(6, 0x2f1, 0);
    DAT_8009c234 = (bytes + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = func_800194f0(bytes);
    func_80027d88(6, 0x2f1);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    func_80043b38(DAT_8009c37c);
    DrawSync(0);
    func_80019518(DAT_8009c37c);
    DAT_8009c230 = 0;
    func_80027a40();
    for (;;) {
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_8001b88c(0x5f);
            func_80027414();
            if (DAT_8009c260 < 48) {
                n = DAT_8009c260;
            } else {
                n = 0x60 - DAT_8009c260;
            }
            setRGB0(DAT_8009c374, n - 96, n - 96, n / 2 - 96);
            DAT_8009c374->tpage = 0x1e;
            DAT_8009c374->clut = 0x7d85;
            func_80027494(selected % 4 * 56 + (xs[selected] * 4 / 5 + 40),
                          0xbc - selected / 4 * 56 - ys[selected] * 4 / 5, 16, 16, 0xa0, 0x80, 16, 16);
            func_80027530(0x1c);
            func_800269c0(0x5f, 0xd6, DAT_8009c254);
            func_80026570(0x69, 0xd6, "select", 0, 0);
            func_800269c0(0xa0, 0xd6, DAT_8009c258);
            func_80026570(0xaa, 0xd6, "return", 0, 0);
            func_800270f8();
        }
        func_800279d8();
        pad = func_800279a4();
        if (pad == PADLup) {
            func_8002792c(0xc);
            selected += 4;
            if (selected == 8) {
                selected = 0;
            } else if (selected >= 12) {
                selected -= 12;
            }
        } else if (pad == PADLdown) {
            func_8002792c(0xc);
            selected -= 4;
            if (selected == -4) {
                selected = 4;
            } else if (selected < 0) {
                selected += 12;
            }
        } else if (pad == PADLright) {
            func_8002792c(0xc);
            if (selected == 3 || selected == 7) {
                selected -= 3;
            } else if (selected == 11) {
                selected = 9;
            } else {
                selected++;
            }
        } else if (pad == PADLleft) {
            func_8002792c(0xc);
            if (selected == 0 || selected == 4) {
                selected += 3;
            } else if (selected == 9) {
                selected = 11;
            } else {
                selected--;
            }
        } else if (pad == DAT_8009c24c) {
            func_8002792c(0xd);
            break;
        } else if (pad == DAT_8009c250) {
            selected = -1;
            func_8002792c(0xe);
            break;
        }
    }
    func_80027a70();
    func_80027a40();
    return selected;
}
