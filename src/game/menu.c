/*
 * GAME.EXE: the in-game menu (WIP module, the first part of the menu
 * translation unit that runs to 0x80027f2c): the pause screen, the button
 * page, the item and equipment pages and their list builders. Its unit is
 * -G8: the menu state words are $gp-relative .sdata/.sbss globals the unit
 * defines, and short labels are .sdata literals.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/kernel.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/render.h>
#include <game/card.h>
#include <game/player.h>
#include <game/menu.h>

RODATA(0x80011448, 0x18)
SDATA(0x8009c280, 0xf)

/* Saved list cursors of four menu lists. */
DATA(0x8009c22c, 0x1, ".sdata")
u8 DAT_8009c22c = 0;
DATA(0x8009c22d, 0x1, ".sdata")
u8 DAT_8009c22d = 0;
DATA(0x8009c22e, 0x1, ".sdata")
u8 DAT_8009c22e = 0;
DATA(0x8009c22f, 0x1, ".sdata")
u8 DAT_8009c22f = 0;

/* Menu state words (WIP). */
DATA(0x8009c230, 0x4, ".sdata")
s32 DAT_8009c230 = 0;
DATA(0x8009c234, 0x4, ".sdata")
s32 DAT_8009c234 = 0;
DATA(0x8009c238, 0x4, ".sdata")
s32 DAT_8009c238 = 0;
DATA(0x8009c23c, 0x4, ".sdata")
s32 DAT_8009c23c = -1;
DATA(0x8009c240, 0x4, ".sdata")
s32 DAT_8009c240 = 0;
DATA(0x8009c244, 0x4, ".sdata")
s32 DAT_8009c244 = 0;
DATA(0x8009c248, 0x4, ".sdata")
s32 DAT_8009c248 = 0;
DATA(0x8009c24c, 0x4, ".sdata")
s32 DAT_8009c24c = 0x20;
DATA(0x8009c250, 0x4, ".sdata")
s32 DAT_8009c250 = 0x40;
DATA(0x8009c254, 0x4, ".sdata")
s32 DAT_8009c254 = 1;
DATA(0x8009c258, 0x4, ".sdata")
s32 DAT_8009c258 = 3;
DATA(0x8009c25c, 0x4, ".sdata")
s32 DAT_8009c25c = 1;
DATA(0x8009c260, 0x4, ".sdata")
s32 DAT_8009c260 = 0;
DATA(0x8009c264, 0x4, ".sdata")
s32 DAT_8009c264 = 0;
DATA(0x8009c268, 0x4, ".sdata")
s32 DAT_8009c268 = 0;
DATA(0x8009c26c, 0x4, ".sdata")
s32 DAT_8009c26c = 0x57;
DATA(0x8009c270, 0x4, ".sdata")
s32 DAT_8009c270 = 0x30;
DATA(0x8009c274, 0x4, ".sdata")
s32 DAT_8009c274 = 0x30;
DATA(0x8009c278, 0x4, ".sdata")
s32 DAT_8009c278 = 0x30;
DATA(0x8009c27c, 0x4, ".sdata")
s32 DAT_8009c27c = 0x80;

/* Menu primitives and the loaded picture (WIP). */
DATA(0x8009c374, 0x4, ".sbss")
POLY_FT4 *DAT_8009c374;
DATA(0x8009c378, 0x4, ".sbss")
POLY_FT4 *DAT_8009c378;
DATA(0x8009c37c, 0x4, ".sbss")
u8 *DAT_8009c37c;
DATA(0x8009c380, 0x4, ".sbss")
u8 *DAT_8009c380;
DATA(0x8009c384, 0x1, ".sbss")
u8 DAT_8009c384;

ADDRESS(0x8001a634, 0x140)
void func_8001a634(void)
{
    MenuText text;
    s32 i;

    func_80027198();
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqPause(DAT_801b09fc.seq_id);
    }
    func_8002792c(0xc);
    func_80027a40();
    text.x = 0x6a;
    text.y = 0x71;
    text.text[0] = 0xff;
    do {
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], &text);
            func_80026570(0x8c, 0x74, "pause", 1, 0);
            func_800270f8();
        }
    } while (!(func_800279a4() & DAT_80081868[13]));
    func_8002792c(0xc);
    func_80027a40();
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqReplay(DAT_801b09fc.seq_id);
    }
    func_80027310(0);
}

ADDRESS(0x8001a774, 0x268)
s32 func_8001a774(void)
{
    s32 selected;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;
    u32 pad;

    selected = 0;
    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    func_80027198();
    for (i = 0; i < 2; i++) {
        func_80026fe4();
        func_800227ec();
        func_800252f4(0, 6, selected, 0);
        func_800270f8();
    }
    func_8002792c(0xc);
    func_80027a40();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            func_80027a40();
        }
        switch (chosen) {
        case 0:
            result = func_8001aa84();
            if (result == MENU_CANCEL) {
                result = MENU_NONE;
            }
            break;
        case 1:
            result = func_8001c48c();
            if (result == MENU_CANCEL) {
                result = MENU_NONE;
            }
            break;
        case 2:
            func_8001c7a8();
            break;
        case 3:
            func_8001d784();
            break;
        case 4:
            func_8001dd94();
            break;
        case 5:
            result = func_8001e484();
            if (result == MENU_CANCEL) {
                result = MENU_NONE;
            }
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = func_800221e8(selected, 5, &chosen, &confirmed, &result);
        pad = PadRead(1);
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if ((pad & PADR1) && (pad & PADL1)) {
                func_8001a9dc();
            }
            func_800227ec();
            func_800252f4(0, 6, selected, 0);
            func_800270f8();
        }
    }
    if (result == -2 || result == -3) {
        func_80027310(1);
    } else {
        func_80027310(0);
    }
    if (result != -1 && result != -2 && result != -3 && (result & 0x1000)) {
        func_8002deec(result & 0xfff);
        result = -1;
    }
    return result;
}
ADDRESS(0x8001a9dc, 0xa8)
void func_8001a9dc(void)
{
    MenuText text;
    s32 x = DAT_801b24e4.pos.vx >> 11;
    s32 z = DAT_801b24e4.pos.vz >> 11;

    text.x = 0xf7;
    text.y = 0xd2;
    func_800277c0(DAT_8018fad4.unk9 * 10000 + x * 100 + z,
                  6, 1, 0, text.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
}

ADDRESS(0x8001aa84, 0x420)
s32 func_8001aa84(void)
{
    MenuList list;
    MenuEntry entries[60];
    u8 counts[64];
    u8 ids[64];
    s32 confirmed;
    s32 result;
    s32 i;
    u32 pad;
    u8 item;
    s32 picked;
    s32 n;

    result = MENU_NONE;
    confirmed = 0;
    n = func_8001aea4(DAT_800c85e8, entries, counts, ids, 0x5f, 0x95);
    func_80027688(&list, 0, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    func_80027728(&list, DAT_8009c22c);
    func_80027bb4();
    pad = 0;
    if (list.count != 0) {
        func_80027a9c(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 0, 1, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            func_80024f88(&list, 0, item);
        }
        pad = func_800222fc(&list, ids, &confirmed, &result, 0);
        DAT_8009c22c = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            func_8002792c(0xd);
            if (item == 95 || item == 96 || item == 97 || item == 98 || item == 99 || item == 100) {
                func_80027e60();
                func_80027b5c();
                if (item == 0x60) {
                    func_8001babc();
                } else {
                    func_8001b5c4(item);
                }
                func_8002792c(0xe);
                func_80027a9c(item);
                confirmed = 0;
            } else if (item == 0x85) {
                if (DAT_8018fad4.unk9 != 0x19 && DAT_8018fad4.unk9 != 0x1b
                    && (DAT_8018fad4.unk9 != 7
                        || !(DAT_801b24e4.pos.vx / 2048 >= 62 && DAT_801b24e4.pos.vx / 2048 <= 70
                             && DAT_801b24e4.pos.vz / 2048 < 12))) {
                    func_80027e60();
                    func_80027b5c();
                    picked = func_8001c030();
                    if (picked == -1) {
                        func_80027a9c(item);
                        confirmed = 0;
                    } else {
                        result = picked + 150;
                        break;
                    }
                }
            }
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, 1);
            func_800270f8();
        }
    }
    func_80027c8c();
    if (result >= 104 && result <= 120) {
        if (!(DAT_8018fad4.unk9 == 2
              && DAT_801b24e4.pos.vx / 2048 >= 10 && DAT_801b24e4.pos.vx / 2048 <= 13
              && DAT_801b24e4.pos.vz / 2048 >= 38 && DAT_801b24e4.pos.vz / 2048 <= 42
              && (result == 104 || result == 107 || result == 108 || result == 109))) {
            func_8001b254(result);
        }
    }
    return result;
}

ADDRESS(0x8001aea4, 0xe4)
s32 func_8001aea4(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last)
{
    s32 n = 0;
    MenuEntry *src = &DAT_8007f620[first];
    u8 *count = &bag[first];
    s32 i;

    last++;
    for (i = first; i < last; count++, i++, src++) {
        if (*count != 0) {
            *entries++ = *src;
            *counts++ = *count;
            n++;
            *ids++ = i;
        }
    }
    return n;
}

ADDRESS(0x8001af88, 0x1c8)
s32 func_8001af88(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last)
{
    s32 n = 0;
    MenuEntry *src = &DAT_8007f620[first];
    u8 *count = &bag[first];
    s32 i;

    last++;
    for (i = first; i < last; count++, i++, src++) {
        if (*count != 0) {
            *counts = *count;
            if (i == DAT_801b24e4.unkcb || i == DAT_801b24e4.unkf0 || i == DAT_801b24e4.unkf1
                || i == DAT_801b24e4.unkf2 || i == DAT_801b24e4.unkf3
                || i == DAT_801b24e4.unkf4 || i == DAT_801b24e4.unkf5
                || i == DAT_801b24e4.unkf6 || i == DAT_801b24e4.unkc9) {
                (*counts)--;
                if (i == DAT_801b24e4.unkf5 && i == DAT_801b24e4.unkf6) {
                    (*counts)--;
                }
            }
            if (*counts != 0) {
                *entries = *src;
                *ids = i;
                entries++;
                counts++;
                ids++;
                n++;
            }
        }
    }
    return n;
}

ADDRESS(0x8001b150, 0x104)
s32 func_8001b150(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last)
{
    s32 n = 0;
    MenuEntry *src = &DAT_8007f620[first];
    u8 *count = &bag[first];
    s32 i;

    last++;
    for (i = first; i < last; count++, i++, src++) {
        if (*count != 0 && DAT_80081780[i] == 5) {
            *entries++ = *src;
            *counts++ = *count;
            n++;
            *ids++ = i;
        }
    }
    return n;
}

ADDRESS(0x8001b254, 0x2a4)
void func_8001b254(s32 item)
{
    if (item == 110 || item == 113 || item == 114 || item == 115 || item == 116 || item == 117
        || item == 118) {
        return;
    }
    if (DAT_800c85e8[item] == 0) {
        return;
    }
    if (item == 104) {
        DAT_801b24e4.hp += 50;
    } else if (item == 105) {
        DAT_801b24e4.hp += 15;
        DAT_801b24e4.unk78 = 0;
    } else if (item == 106) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
    } else if (item == 107) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
        DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
        func_8001b4f8();
    } else if (item == 108) {
        if (DAT_801b24e4.unk84 > 0) {
            DAT_801b24e4.unk84 = 0;
        }
        if (DAT_801b24e4.unk82 > 64) {
            DAT_801b24e4.unk82 = 64;
        }
        func_8001b594();
        func_8001b564();
    } else if (item == 109) {
        DAT_801b24e4.unk1c += 40;
    } else if (item == 111) {
    } else if (item == 112) {
    } else if (item == 119) {
        DAT_801b24e4.hp += 100;
    } else if (item == 120) {
        DAT_801b24e4.hp += 200;
        DAT_801b24e4.unk1c += 100;
        func_8001b4f8();
    }
    if (DAT_801b24e4.hp > DAT_801b24e4.hp_max) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
    }
    if (DAT_801b24e4.unk1c > DAT_801b24e4.unk1a) {
        DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
    }
    DAT_800c85e8[item]--;
    if (item == 119 || item == 120) {
        DAT_800c85e8[118]++;
    }
    func_8002792c(9);
}

ADDRESS(0x8001b4f8, 0x6c)
void func_8001b4f8(void)
{
    if (DAT_801b24e4.unk84 > 0) {
        DAT_801b24e4.unk84 = 0;
    }
    if (DAT_801b24e4.unk82 > 64) {
        DAT_801b24e4.unk82 = 64;
    }
    DAT_801b24e4.unk78 = 0;
    func_8001b594();
    func_8001b564();
}

ADDRESS(0x8001b564, 0x30)
void func_8001b564(void)
{
    if (DAT_801b24e4.unk7e > 64) {
        DAT_801b24e4.unk7e = 64;
        DAT_801b24e4.unk80 = 0;
    }
}

ADDRESS(0x8001b594, 0x30)
void func_8001b594(void)
{
    if (DAT_801b24e4.unk7a > 64) {
        DAT_801b24e4.unk7a = 64;
        DAT_801b24e4.unk7c = 0;
    }
}

ADDRESS(0x8001b5c4, 0x2c8)
void func_8001b5c4(u8 item)
{
    s32 n;
    s32 i;
    s32 bytes;
    u8 area;

    if (item == 0x5f) {
        n = 0x2f1;
    } else if (item == 0x63) {
        n = 0x2f2;
    } else {
        n = 0x2f3;
    }
    bytes = func_80019cc4(6, n, 0);
    DAT_8009c234 = (bytes + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = func_800194f0(bytes);
    func_80027d88(6, n);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    i = 0;
    func_80043b38(DAT_8009c37c);
    DrawSync(0);
    func_80019518(DAT_8009c37c);
    DAT_8009c230 = 0;
    func_80027a40();
    do {
        do {
            func_80026fe4();
            func_8001b88c(item);
            if (DAT_8018fad4.unk9 < 12 && item == 0x5f) {
                func_80027414();
                if (DAT_8009c260 < 48) {
                    n = DAT_8009c260;
                } else {
                    n = 0x60 - DAT_8009c260;
                }
                setRGB0(DAT_8009c374, n - 128, n - 128, n - 128);
                DAT_8009c374->tpage = 0x1e;
                DAT_8009c374->clut = 0x7e45;
                n = ((DAT_801b24e4.unk124.vy & 0xfff) + 256) >> 9;
                if (n == 8) {
                    n = 0;
                }
                area = DAT_8018fad4.unk9;
                func_80027494((area & 3) * 56 + DAT_801b24e4.pos.vx / 2560 + 40,
                              0xbc - DAT_801b24e4.pos.vz / 2560 - (area >> 2) * 56,
                              15, 15, n * 16 + 128, 0x90, 15, 15);
                func_80027530(0x1c);
            }
            func_800269c0(0x80, 0xd6, DAT_8009c258);
            func_80026570(0x8a, 0xd6, "return", 0, 0);
            func_800270f8();
            i++;
        } while (i < 2);
    } while (!func_800279a4());
    func_80027a70();
    func_80027a40();
}

ADDRESS(0x8001b88c, 0x230)
void func_8001b88c(s32 item)
{
    s32 shade = 0;

    func_80027414();
    if (DAT_801b24e4.unkfd > 128) {
        shade = (DAT_801b24e4.unkfd - 128) / 4;
    }
    setRGB0(DAT_8009c374, shade + 64, shade + 64, shade + 64);
    SetSemiTrans(DAT_8009c374, DAT_801b24e4.unk100);
    DAT_8009c374->tpage = GetTPage(0, 0, 0x3c0, 0x100);
    DAT_8009c374->clut = 0x7806;
    if (item == 0x5f) {
        func_80027494(0x2b, 0x17, 0xea, 0xb2, 0, 0, 0xea, 0xb2);
    } else if (item == 0x63) {
        func_80027494(0x50, 0x16, 0xa0, 0xb4, 0, 0, 0xa0, 0xb4);
    } else {
        func_80027494(0x46, 0x16, 0xb4, 0xb4, 0, 0, 0xb4, 0xb4);
    }
    func_80027530(0x1e);
    func_80027414();
    SetSemiTrans(DAT_8009c374, 1);
    setRGB0(DAT_8009c374, shade + 64, shade + 64, shade + 64);
    DAT_8009c374->tpage = GetTPage(0, 1, 0x3c0, 0x100);
    DAT_8009c374->clut = 0x7806;
    if (item == 0x5f) {
        func_80027494(0x2b, 0x17, 0xea, 0xb2, 0, 0, 0xea, 0xb2);
    } else if (item == 0x63) {
        func_80027494(0x50, 0x16, 0xa0, 0xb4, 0, 0, 0xa0, 0xb4);
    } else {
        func_80027494(0x46, 0x16, 0xb4, 0xb4, 0, 0, 0xb4, 0xb4);
    }
    func_80027530(0x1d);
}

ADDRESS(0x8001babc, 0x28c)
void func_8001babc(void)
{
    s32 n;
    s32 i;
    s32 bytes;

    n = DAT_8018fad4.unk9 + 721;
    bytes = func_80019cc4(6, n, 0);
    DAT_8009c234 = (bytes + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = func_800194f0(bytes);
    func_80027d88(6, n);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    i = 0;
    func_80043b38(DAT_8009c37c);
    DrawSync(0);
    func_80019518(DAT_8009c37c);
    DAT_8009c230 = 0;
    func_8001bea8();
    func_80027a40();
    do {
        do {
            func_80026fe4();
            func_8001bd48();
            func_80027414();
            if (DAT_8009c260 < 48) {
                n = DAT_8009c260;
            } else {
                n = 0x60 - DAT_8009c260;
            }
            setRGB0(DAT_8009c374, n - 128, n - 128, n - 128);
            DAT_8009c374->tpage = 0x1e;
            DAT_8009c374->clut = 0x7e45;
            n = ((DAT_801b24e4.unk124.vy & 0xfff) + 256) >> 9;
            if (n == 8) {
                n = 0;
            }
            if (DAT_8018fad4.unk9 == 0x19 || DAT_8018fad4.unk9 == 0x1b) {
                func_80027494(0x98, 0x70, 15, 15, n * 16 + 128, 0x90, 15, 15);
            } else {
                func_80027494(DAT_801b24e4.pos.vx / 1024 + 72, 0xbc - DAT_801b24e4.pos.vz / 1024, 15, 15,
                              n * 16 + 128, 0x90, 15, 15);
            }
            func_80027530(0x1d);
            func_80026e4c(0x32, 0x1a, 0xdc, 0xb4);
            func_800269c0(0x80, 0xd6, DAT_8009c258);
            func_80026570(0x8a, 0xd6, "return", 0, 0);
            func_800270f8();
            i++;
        } while (i < 2);
    } while (!func_800279a4());
    func_80027a70();
    func_80027a40();
}

ADDRESS(0x8001bd48, 0x160)
void func_8001bd48(void)
{
    s32 i;
    MapRect *rect;
    s32 bit;

    for (i = 159; i != -1; i--) {
        rect = DAT_801ba6fc + i;
        if (DAT_801b56cc[DAT_8018fad4.unk9][i / 8] & (bit = 1 << (7 - i % 8))) {
            func_80027414();
            setRGB0(DAT_8009c374, 0x7f, 0x7f, 0x7f);
            DAT_8009c374->tpage = GetTPage(0, 0, 0x3c0, 0x100);
            DAT_8009c374->clut = 0x7806;
            func_80027494(rect->x + 80, 0xc4 - rect->y - rect->h, rect->w, rect->h,
                          rect->x, 0xa0 - rect->y - rect->h, rect->w, rect->h);
            func_80027530(0x1e);
        }
    }
}

ADDRESS(0x8001bea8, 0x188)
void func_8001bea8(void)
{
    s32 i;
    s32 x;
    s32 y;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    MapRect *rect;
    s32 bit;

    for (i = 159; i != -1; i--) {
        rect = DAT_801ba6fc + i;
        if (!(DAT_801b56cc[DAT_8018fad4.unk9][i / 8] & (bit = 1 << (7 - i % 8)))) {
            x0 = rect->x / 2;
            y0 = rect->y / 2;
            x1 = x0 + rect->w / 2;
            y1 = y0 + rect->h / 2;
            for (x = x0; x < x1; x++) {
                for (y = y0; y < y1; y++) {
                    if (DAT_801b594c[x][y / 8] & (bit = 1 << (7 - y % 8))) {
                        DAT_801b56cc[DAT_8018fad4.unk9][i / 8] |= 1 << (7 - i % 8);
                    }
                }
            }
        }
    }
}
