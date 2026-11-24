/*
 * GAME.EXE: the in-game menu (WIP module, 0x8001a634..0x80027f2c): the pause
 * screen, the button page and the pages it opens (items, equipment, magic,
 * status, books, map, options, saves, shops), their list and yes/no input,
 * the item preview read from ITEM.T, and the menu's own drawing helpers
 * (frame begin/present, glyph text, windows) and CD read callback. Its unit
 * is -G8: the menu state words are $gp-relative .sdata globals and .sbss
 * statics the unit defines, short labels are .sdata literals, and the name
 * tables and jump tables are its .rdata.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/libc.h>
#include <psyq/kernel.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/render.h>
#include <game/card.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/misc.h>
#include <game/funcs.h>

RODATA(0x800110b0, 0x68c)
SDATA(0x8009c280, 0x3e)

/* PSX.EXE's mailbox byte beside its next-program byte (see src/game/main.c). */
#define MAILBOX_BYTE_8 ((u8 *)0x800102f8)
#define MAILBOX_BYTE_A ((u8 *)0x800102fa)

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

/* Menu primitives, the loaded picture and the screen copy (WIP). */
DATA(0x8009c374, 0x4, ".sbss")
static POLY_FT4 *DAT_8009c374;
DATA(0x8009c378, 0x4, ".sbss")
static SPRT *DAT_8009c378;
DATA(0x8009c37c, 0x4, ".sbss")
static u8 *DAT_8009c37c;
DATA(0x8009c380, 0x4, ".sbss")
static u8 *DAT_8009c380;
DATA(0x8009c384, 0x1, ".sbss")
static u8 DAT_8009c384;
/* The screen copy the menu draws over (func_80027198 stores it, the
 * present loads it back each frame) and where it lies in VRAM. */
DATA(0x8009c388, 0x4, ".sbss")
static u8 *DAT_8009c388;
DATA(0x8009c38c, 0x8, ".sbss")
static RECT DAT_8009c38c;
/* The rotating item preview: its translation (vz the distance), its rotation
 * and the spin added to rotation.vy each frame (debug pad edits them). */
DATA(0x8009c394, 0x8, ".sbss")
static SVECTOR DAT_8009c394;
DATA(0x8009c39c, 0x8, ".sbss")
static SVECTOR DAT_8009c39c;
DATA(0x8009c3a4, 0x4, ".sbss")
static s32 DAT_8009c3a4;

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
    n = func_8001aea4(DAT_800c85e8[0], entries, counts, ids, 0x5f, 0x95);
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
        if (DAT_8018fad4.unk9 == 2
            && DAT_801b24e4.pos.vx / 2048 >= 10 && DAT_801b24e4.pos.vx / 2048 <= 13
            && DAT_801b24e4.pos.vz / 2048 >= 38 && DAT_801b24e4.pos.vz / 2048 <= 42) {
            if (!(result == 104 || result == 107 || result == 108 || result == 109)) {
                func_8001b254(result);
            }
        } else {
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
    if (DAT_800c85e8[0][item] == 0) {
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
    DAT_800c85e8[0][item]--;
    if (item == 119 || item == 120) {
        DAT_800c85e8[0][118]++;
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
    s32 size = 15;

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
                func_80027494(0x98, 0x70, 15, 15, n * 16 + 128, 0x90, size, size);
            } else {
                func_80027494(DAT_801b24e4.pos.vx / 1024 + 72, 0xbc - DAT_801b24e4.pos.vz / 1024, 15, 15,
                              n * 16 + 128, 0x90, size, size);
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
            func_80027494(selected % 4 * 56 + 40 + xs[selected] * 4 / 5,
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

ADDRESS(0x8001c48c, 0x234)
s32 func_8001c48c(void)
{
    u32 pad = 0;
    MenuList list;
    MenuEntry entries[20];
    u32 values[20];
    u8 ids[24];
    s32 confirmed;
    s32 result;
    s32 i;
    s32 n;

    result = MENU_NONE;
    confirmed = 0;
    n = func_8001c6c0(DAT_801b77ec.unk0, entries, values, ids, 0x17, 0x1e);
    func_80027688(&list, 0, 1);
    list.rows = 4;
    list.count = n;
    list.entries = entries;
    list.values = values;
    list.y = 0x83;
    func_80027728(&list, DAT_8009c22d);
    func_80027bb4();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 0, 2, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = ids[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            func_80024f88(&list, 1, ids[list.cursor]);
        }
        pad = func_800222fc(&list, 0, &confirmed, &result, 0);
        DAT_8009c22d = list.cursor;
        if (confirmed == 1) {
            func_8002792c(0xd);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025468(&list, 2);
            func_800270f8();
        }
    }
    func_80027c8c();
    if (result != -1) {
        result |= 0x1000;
    }
    return result;
}

ADDRESS(0x8001c6c0, 0xe8)
s32 func_8001c6c0(Record18 *spells, MenuEntry *entries, u32 *values, u8 *ids, s32 first, s32 last)
{
    MenuEntry *src = &DAT_80080430[first];
    Record18 *spell = &spells[first];
    s32 n = 0;
    s32 i;

    last++;
    for (i = first; i < last; spell++, i++, src++) {
        if (spell->unk0 == 1) {
            *entries++ = *src;
            *values++ = spell->unk16;
            n++;
            *ids++ = i;
        }
    }
    return n;
}

ADDRESS(0x8001c7a8, 0x218)
void func_8001c7a8(void)
{
    MenuList list;
    MenuEntry labels[10];
    MenuEntry names[10];
    s32 confirmed;
    s32 result;
    s32 i;

    confirmed = 0;
    result = MENU_NONE;
    memcpy(labels, DAT_8007f230, sizeof(labels));
    func_8001c9c0(names);
    func_80027688(&list, 0, 2);
    list.count = 10;
    list.rows = 10;
    list.entries = labels;
    list.y = 0x23;
    list.unk28 = names;
    list.unk0 = 0;
    list.unk23 = 0x18;
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            if (list.cursor == 1) {
                func_8001d0c4();
            } else if (list.cursor == 9) {
                func_8001d3a0();
            } else {
                func_8001cbb8(list.cursor);
            }
            func_8001c9c0(names);
        }
        if (result != MENU_NONE) {
            break;
        }
        func_800222fc(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            func_8002792c(0xd);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025468(&list, 3);
            func_800270f8();
        }
    }
}

ADDRESS(0x8001c9c0, 0x1f8)
void func_8001c9c0(MenuEntry *out)
{
    u8 slots[10];
    s32 i;
    u8 *slot;

    slots[0] = DAT_801b24e4.unkcb;
    slots[1] = DAT_801b24e4.unkc7;
    slots[2] = DAT_801b24e4.unkf2;
    slots[3] = DAT_801b24e4.unkf0;
    slots[4] = DAT_801b24e4.unkf1;
    slots[5] = DAT_801b24e4.unkf3;
    slots[6] = DAT_801b24e4.unkf4;
    slots[7] = DAT_801b24e4.unkf5;
    slots[8] = DAT_801b24e4.unkf6;
    if (DAT_801b24e4.unkc8 == 0xff) {
        slots[9] = DAT_801b24e4.unkc9;
    } else {
        slots[9] = DAT_801b24e4.unkc8;
    }
    slot = slots;
    for (i = 0; i < 10; i++) {
        if (*slot != 0xff) {
            if (i == 1 || (i == 9 && DAT_801b24e4.unkc8 != 0xff)) {
                *out = DAT_80080430[*slot];
            } else {
                *out = DAT_8007f620[*slot];
            }
        } else {
            out->text[0] = 0xff;
        }
        out++;
        slot++;
    }
}

ADDRESS(0x8001cbb8, 0x50c)
void func_8001cbb8(s32 slot)
{
    u32 pad = 0;
    u8 equipped = 0;
    MenuList list;
    MenuEntry entries[40];
    u8 counts[40];
    u8 ids[40];
    s32 confirmed;
    s32 result;
    u8 other;
    u8 cursor = 0;
    s32 first;
    s32 last;
    s32 n;
    s32 i;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    switch (slot) {
    case 0:
        first = 0;
        equipped = DAT_801b24e4.unkcb;
        last = 0x21;
        break;
    case 2:
        first = 0x33;
        equipped = DAT_801b24e4.unkf2;
        last = 0x3e;
        break;
    case 3:
        first = 0x22;
        equipped = DAT_801b24e4.unkf0;
        last = 0x29;
        break;
    case 4:
        first = 0x2a;
        equipped = DAT_801b24e4.unkf1;
        last = 0x32;
        break;
    case 5:
        first = 0x3f;
        equipped = DAT_801b24e4.unkf3;
        last = 0x47;
        break;
    case 6:
        first = 0x48;
        equipped = DAT_801b24e4.unkf4;
        last = 0x50;
        break;
    case 7:
    case 8:
        if (slot == 7) {
            equipped = DAT_801b24e4.unkf5;
        } else {
            equipped = DAT_801b24e4.unkf6;
        }
        if (slot == 7) {
            other = DAT_801b24e4.unkf6;
        } else {
            other = DAT_801b24e4.unkf5;
        }
        first = 0x51;
        if (other != 0xff) {
            DAT_800c85e8[0][other]--;
        }
        last = 0x5e;
        break;
    }
    n = func_8001aea4(DAT_800c85e8[0], entries, counts, ids, first, last);
    memcpy(&entries[n], &DAT_8007f230[11], 10);
    counts[n] = 0xff;
    ids[n] = 0xff;
    n++;
    func_80027688(&list, 0, 2);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    for (i = 0; i < n - 1; i++) {
        if (ids[i] == equipped) {
            cursor = i;
            i = 40;
        }
    }
    func_80027728(&list, cursor);
    func_80027bb4();
    if (list.count >= 2) {
        func_80027a9c(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 5, 5, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && item != 0xff && !(pad & PADselect) && !(pad & PADstart)
            && (pad & PADRup)) {
            func_80024f88(&list, 0, item);
        }
        pad = func_800222fc(&list, ids, &confirmed, &result, 0);
        item = ids[list.cursor];
        if (confirmed == 1) {
            func_8002792c(0xd);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, 5);
            func_800270f8();
        }
    }
    func_80027c8c();
    if (result != -1) {
        switch (slot) {
        case 0:
            func_8002bdc0(result);
            break;
        case 2:
            func_8002bb84(result, 4);
            break;
        case 3:
            func_8002bb84(result, 0);
            break;
        case 4:
            func_8002bb84(result, 1);
            break;
        case 5:
            func_8002bb84(result, 2);
            break;
        case 6:
            func_8002bb84(result, 3);
            break;
        case 7:
            func_8002bb84(result, 5);
            break;
        case 8:
            func_8002bb84(result, 6);
            break;
        }
    }
    if ((slot == 7 || slot == 8) && other != 0xff) {
        DAT_800c85e8[0][other]++;
    }
}

ADDRESS(0x8001d0c4, 0x2dc)
void func_8001d0c4(void)
{
    u32 pad = 0;
    MenuList list;
    MenuEntry entries[32];
    u32 values[32];
    u8 ids[32];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    n = func_8001c6c0(DAT_801b77ec.unk0, entries, values, ids, 0, 0x16);
    memcpy(&entries[n], &DAT_8007f230[11], 12);
    values[n] = -1;
    ids[n] = 0xff;
    n++;
    func_80027688(&list, 0, 2);
    list.rows = 4;
    list.count = n;
    list.entries = entries;
    list.values = values;
    list.y = 0x83;
    for (i = 0; i < n - 1; i++) {
        if (ids[i] == DAT_801b24e4.unkc7) {
            func_80027728(&list, i);
            i = 32;
        }
    }
    func_80027bb4();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 5, 4, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = ids[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && ids[list.cursor] != 0xff && !(pad & PADselect) && !(pad & PADstart)
            && (pad & PADRup)) {
            func_80024f88(&list, 1, ids[list.cursor]);
        }
        pad = func_800222fc(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            func_8002792c(0xd);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025468(&list, 4);
            func_800270f8();
        }
    }
    func_80027c8c();
    if (result != -1) {
        func_8002b928(result);
    }
}

ADDRESS(0x8001d3a0, 0x3e4)
void func_8001d3a0(void)
{
    u32 pad = 0;
    s32 i = 0;
    MenuList list;
    MenuEntry entries[88];
    u8 counts[88];
    u32 values[88];
    u8 ids[88];
    u8 spells[88];
    s32 confirmed;
    s32 result;
    u8 saved;
    u8 item;
    s32 n;

    confirmed = 0;
    result = MENU_NONE;
    for (; i < 88; i++) {
        counts[i] = 0xff;
        values[i] = -1;
        ids[i] = 0xff;
        spells[i] = 0xff;
    }
    saved = DAT_800c85e8[0][0x85];
    DAT_800c85e8[0][0x85] = 0;
    n = func_8001aea4(DAT_800c85e8[0], entries, counts, ids, 0x65, 0x92);
    DAT_800c85e8[0][0x85] = saved;
    n += func_8001c6c0(DAT_801b77ec.unk0, &entries[n], &values[n], &spells[n], 0, 0x1e);
    memcpy(&entries[n], &DAT_8007f230[11], 12);
    n++;
    func_80027688(&list, 0, 2);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    list.values = values;
    if (DAT_801b24e4.unkc8 != 0xff || DAT_801b24e4.unkc9 != 0xff) {
        for (i = 0; i < 88; i++) {
            if (DAT_801b24e4.unkc8 == 0xff) {
                if (ids[i] == DAT_801b24e4.unkc9) {
                    func_80027728(&list, i);
                    i = 88;
                }
            } else if (spells[i] == DAT_801b24e4.unkc8) {
                func_80027728(&list, i);
                i = 88;
            }
        }
    }
    func_80027bb4();
    if (list.count >= 2) {
        func_80027a9c(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 5, 0x10, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = list.cursor;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            if (item != 0xff) {
                func_80024f88(&list, 0, item);
            } else if (spells[list.cursor] != 0xff) {
                func_80024f88(&list, 1, spells[list.cursor]);
            }
        }
        pad = func_800222fc(&list, ids, &confirmed, &result, 0);
        item = ids[list.cursor];
        if (confirmed == 1) {
            func_8002792c(0xd);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, 0x10);
            func_800270f8();
        }
    }
    func_80027c8c();
    if (result != -1) {
        func_8002ba38(spells[result]);
        func_8002bb5c(ids[result]);
    }
}

ADDRESS(0x8001d784, 0x120)
void func_8001d784(void)
{
    MenuList list;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 selected = 0;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            func_80027a40();
        }
        switch (chosen) {
        case 0:
            func_8001d8a4();
            break;
        case 1:
            func_8001d944();
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = func_800221e8(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(1, 2, selected, 0);
            func_800270f8();
        }
    }
}

ADDRESS(0x8001d8a4, 0xa0)
void func_8001d8a4(void)
{
    s32 result = MENU_NONE;
    s32 i;

    for (;;) {
        if (result != MENU_NONE) {
            func_80027a40();
            break;
        }
        func_800279d8();
        if (func_800279a4()) {
            func_8002792c(0xe);
            result = -1;
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80023300();
            func_800270f8();
        }
    }
}

ADDRESS(0x8001d944, 0x280)
void func_8001d944(void)
{
    s32 result = MENU_NONE;
    s32 page = 0;
    s32 row = 0;
    u8 books[64];
    u8 pages[256];
    s32 count;
    s32 n;
    s32 i;
    s32 j;
    u8 *book;
    u32 pad;

    count = func_80060b0c();
    i = count;
    j = 0;
    while (--i != -1) {
        books[j] = func_80060c54(j);
        j++;
    }
    if (count == 0) {
        for (i = 0; i < 3; i++) {
            func_80026fe4();
            func_80023ce8(-1);
            func_800270f8();
        }
        while (!func_800279a4()) {
        }
        func_8002792c(0xe);
        func_80027a40();
        return;
    }
    book = &books[page];
        n = func_8001dbc4(*book, page, pages);
        func_8001dcd0(page, pages[row]);
        while (result == MENU_NONE) {
            func_800279d8();
            pad = func_800279a4();
            if (pad & DAT_8009c250) {
                func_8002792c(0xe);
                result = -1;
            } else if (pad & PADLup) {
                func_8002792c(0xc);
                if (row > 0) {
                    row--;
                    func_8001dcd0(page, pages[row]);
                }
            } else if (pad & PADLdown) {
                func_8002792c(0xc);
                if (row < n - 1) {
                    row++;
                    func_8001dcd0(page, pages[row]);
                }
            } else if (pad & PADLright) {
                func_8002792c(0xc);
                if (page > 0) {
                    book--;
                    page--;
                    n = func_8001dbc4(*book, page, pages);
                    row = 0;
                    func_8001dcd0(page, pages[0]);
                }
            } else if (pad & PADLleft) {
                func_8002792c(0xc);
                if (page < count - 1) {
                    book++;
                    page++;
                    n = func_8001dbc4(*book, page, pages);
                    row = 0;
                    func_8001dcd0(page, pages[0]);
                }
            }
            for (i = 0; i < 2; i++) {
                func_80026fe4();
                func_80023ce8(*book);
                func_800270f8();
            }
        }
    func_80027a40();
}

ADDRESS(0x8001dbc4, 0x10c)
s32 func_8001dbc4(s32 index, s32 book, u8 *out)
{
    u16 entry = index + 840;
    u8 *buffer;
    s32 n;
    s32 i;
    u16 page;

    buffer = func_800194f0(func_80019cc4(8, entry, 0));
    func_8001a154(8, entry, buffer);
    func_80043b38(buffer);
    DrawSync(0);
    func_80019518(buffer);
    n = 0;
    for (i = 0; i < 256; i++) {
        page = func_80060a10(book, i);
        if (page != 0xffff && page != 0xfffe && page != 0xfffd) {
            *out++ = i;
            n++;
        }
        if (page == 0xffff) {
            i = 256;
        }
    }
    return n;
}

ADDRESS(0x8001dcd0, 0xc4)
void func_8001dcd0(s32 book, s32 page)
{
    RECT rect;
    u16 entry;
    u8 *buffer;

    setRECT(&rect, 0x3c0, 0x100, 0x40, 0xc8);
    ClearImage(&rect, 0, 0, 0);
    entry = func_80060a10(book, page);
    if (entry <= 0xf000) {
        buffer = func_800194f0(func_80019cc4(8, entry, 0));
        func_8001a154(8, entry, buffer);
        func_80043b38(buffer);
        DrawSync(0);
        func_80019518(buffer);
    }
}

ADDRESS(0x8001dd94, 0x150)
void func_8001dd94(void)
{
    s32 selected = 0;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    func_80027bb4();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            func_80027a40();
        }
        switch (chosen) {
        case 0:
            func_8001dee4();
            break;
        case 1:
            func_8001e1b4();
            break;
        }
        if (chosen != -1 && result == MENU_CANCEL) {
            result = MENU_NONE;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = func_800221e8(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(2, 2, selected, 0);
            func_800270f8();
        }
    }
    func_80027c8c();
}

ADDRESS(0x8001dee4, 0x2d0)
void func_8001dee4(void)
{
    MenuList list;
    MenuEntry entries[152];
    u8 counts[152];
    u8 ids[152];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u32 pad;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    n = func_8001af88(DAT_800c85e8[0], entries, counts, ids, 0, 0x95);
    func_80027688(&list, 2, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    func_80027728(&list, DAT_8009c22e);
    pad = 0;
    if (list.count != 0) {
        func_80027a9c(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 1, 7, ids[list.cursor]);
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
        pad = func_800222fc(&list, ids, &confirmed, &result, 2);
        DAT_8009c22e = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (counts[list.cursor] < DAT_8009c25c || DAT_800c85e8[1][item] + DAT_8009c25c >= 100) {
                func_8002792c(0xe);
                confirmed = 0;
            } else {
                func_8002792c(0xd);
            }
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, 7);
            func_800270f8();
        }
    }
    if (result != -1) {
        DAT_800c85e8[0][result] -= DAT_8009c25c;
        DAT_800c85e8[1][result] += DAT_8009c25c;
    }
}

ADDRESS(0x8001e1b4, 0x2d0)
void func_8001e1b4(void)
{
    MenuList list;
    MenuEntry entries[76];
    u8 counts[152];
    u8 ids[152];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u32 pad;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    n = func_8001aea4(DAT_800c85e8[1], entries, counts, ids, 0, 0x95);
    func_80027688(&list, 2, 1);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    func_80027728(&list, DAT_8009c22f);
    pad = 0;
    if (list.count != 0) {
        func_80027a9c(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 10, 7, ids[list.cursor]);
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
        pad = func_800222fc(&list, ids, &confirmed, &result, 2);
        DAT_8009c22f = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (counts[list.cursor] < DAT_8009c25c || DAT_800c85e8[0][item] + DAT_8009c25c >= 100) {
                func_8002792c(0xe);
                confirmed = 0;
            } else {
                func_8002792c(0xd);
            }
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, 7);
            func_800270f8();
        }
    }
    if (result != -1) {
        DAT_800c85e8[0][result] += DAT_8009c25c;
        DAT_800c85e8[1][result] -= DAT_8009c25c;
    }
}

ADDRESS(0x8001e484, 0x28c)
s32 func_8001e484(void)
{
    MenuText text;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 selected = 0;
    s32 i;
    s32 volume;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            func_80027a40();
        }
        switch (chosen) {
        case 0:
            result = func_8001e710();
            if (result == 0) {
                result = -3;
            }
            break;
        case 1:
            func_8001ec84();
            break;
        case 2:
            func_8001f004();
            break;
        case 3:
            func_8001f31c();
            break;
        case 4:
            result = func_8001f8b4();
            if (result == 0) {
                result = -2;
            }
            break;
        }
        if (chosen != -1 && result == MENU_CANCEL) {
            result = MENU_NONE;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = func_800221e8(selected, 4, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(3, 5, selected, 0);
            func_800270f8();
        }
    }
    if (result == -2) {
        u32 status;

        CdControl(CdlStop, 0, (u8 *)&status);
        text.x = 0x78;
        text.y = 0x70;
        text.text[0] = 0x0f;
        text.text[1] = 0x0b;
        text.text[2] = 4;
        text.text[3] = 0;
        text.text[4] = 0x12;
        text.text[5] = 4;
        text.text[6] = 0x7f;
        text.text[7] = 0x16;
        text.text[8] = 0;
        text.text[9] = 8;
        text.text[10] = 0x13;
        text.text[11] = 0xff;
        volume = DAT_801b24e4.bgm_volume;
        for (i = 0x59; i != -1; i--) {
            if (volume > 0) {
                volume--;
            }
            func_8001eb3c(7, &text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
            SsSeqSetVol(DAT_801b09fc.seq_id, volume, volume);
        }
    }
    return result;
}

ADDRESS(0x8001e710, 0x324)
s32 func_8001e710(void)
{
    struct DIRENTRY dir[15];
    MenuList list;
    MenuEntry entries[8];
    u32 experience[8];
    u8 levels[8];
    s32 numbers[8];
    MenuText text[3];
    s32 status;
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    s32 error;

    confirmed = 0;
    result = MENU_NONE;
    if (DAT_8009c384 != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqPause(DAT_801b09fc.seq_id);
    }
    func_80019ab4();
    func_8002090c(text);
    func_8001eb3c(6, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    func_80028034();
    for (i = 4; i != -1; i--) {
        func_800281c4(dir, &status);
        if (status != 0) {
            break;
        }
    }
    n = func_8001ea34(dir, entries->text, experience, levels, numbers);
    func_80027688(&list, 3, 0);
    list.rows = 5;
    list.y = 0x73;
    list.count = n;
    list.entries = entries;
    list.counts = levels;
    list.values = experience;
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 6, 8, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = numbers[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        func_800222fc(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            func_8002792c(0xc);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025468(&list, 8);
            func_800270f8();
        }
    }
    if (result != -1) {
        func_80020dec(text);
        func_8001eb3c(6, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
        error = func_8002860c(result);
        if (error != 0) {
            func_80020e6c(text, error);
            func_8001eb3c(6, text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            func_80027a40();
            func_80027a70();
            func_80027a40();
            result = -1;
        } else {
            result = 0;
        }
    }
    func_80028090();
    if (DAT_8009c384 != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqReplay(DAT_801b09fc.seq_id);
    }
    return result;
}

ADDRESS(0x8001ea34, 0x108)
s32 func_8001ea34(struct DIRENTRY *dir, u8 *p, u32 *experience, u8 *levels, s32 *numbers)
{
    s32 n = 0;
    s32 i = 0;
    s32 points;
    s32 level;
    s32 number;

    for (; i < 15; i++) {
        if (func_80028398(dir->name, &points, &level, &number) == 0) {
            *p++ = 3;
            *p++ = 0;
            *p++ = 0x13;
            *p++ = 0;
            *p++ = number + 32;
            *p = 0xff;
            p += 19;
            n++;
            *experience++ = points;
            *levels++ = level;
            *numbers++ = number;
        }
        dir++;
    }
    return n;
}

ADDRESS(0x8001eb3c, 0x148)
void func_8001eb3c(s32 message, MenuText *lines, s32 count, s32 x, s32 y, s32 w, s32 h, s32 back)
{
    s32 i;
    s32 j;
    MenuText *title = &DAT_8007e660[message + 28];
    MenuText *line;

    for (i = 0; i < 2; i++) {
        func_80026fe4();
        line = lines;
        if (message < 8) {
            func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], title);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], title);
        }
        for (j = 0; j < count; j++) {
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], line);
            line++;
        }
        func_80026acc(x, y, w, h);
        if (back == 1) {
            func_800269c0(0x80, 0xd6, DAT_8009c258);
            func_80026570(0x8a, 0xd6, "return", 0, 0);
        }
        func_800270f8();
    }
}

ADDRESS(0x8001ec84, 0x380)
void func_8001ec84(void)
{
    u8 levels[3];
    s32 row = 0;
    s32 result = MENU_NONE;
    s32 last = 2;
    u32 pad;
    s32 i;

    levels[0] = DAT_801b24e4.se_volume;
    if (DAT_801b24e4.bgm_volume == 0) {
        levels[1] = 0;
    } else {
        levels[1] = DAT_801b24e4.bgm_volume + 1;
    }
    levels[2] = DAT_801b24e4.unkfd + 192;
    for (;;) {
        if (result != MENU_NONE) {
            func_80027a40();
            break;
        }
        func_800279d8();
        pad = func_800279a4();
        if (pad & PADLup) {
            DAT_8009c268 = 0;
            func_8002792c(0xc);
            if (row == 0) {
                row = last;
            } else {
                row--;
            }
        } else if (pad & PADLdown) {
            DAT_8009c268 = 0;
            func_8002792c(0xc);
            if (row == last) {
                row = 0;
            } else {
                row++;
            }
        } else if (pad & PADLright) {
            levels[row] += 3;
            if (levels[row] > 128) {
                levels[row] = 128;
            }
        } else if (pad & PADLleft) {
            if (levels[row] >= 3) {
                levels[row] -= 3;
            } else if (levels[row] != 0) {
                levels[row] = 0;
            }
        } else if (pad & DAT_8009c250) {
            func_8002792c(0xe);
            result = -1;
        }
        if ((pad & PADselect) || (pad & PADstart)) {
            DAT_8009c248 = 0;
            VSync(0);
        }
        if ((pad & PADLright) || (pad & PADLleft)) {
            if (row == 0) {
                DAT_801b24e4.se_volume = levels[0];
                func_80015790(0xc, 0x80, 0x80, 0);
                DAT_801b24e4.se_volume = levels[0];
            } else if (row == 1) {
                if (DAT_8009c384 == 0) {
                    DAT_801b24e4.se_volume = levels[1];
                    func_80015790(0xc, 0x80, 0x80, 0);
                    DAT_801b24e4.se_volume = levels[0];
                } else {
                    if (levels[1] == 0) {
                        i = 0;
                    } else if (levels[1] >= 128) {
                        i = 0x7f;
                    } else {
                        i = levels[1] - 1;
                    }
                    SsSeqSetVol(DAT_801b09fc.seq_id, i, i);
                }
            } else {
                func_8002792c(0xc);
            }
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(4, 3, row, 1);
            func_80024a78(levels);
            func_800270f8();
        }
    }
    DAT_801b24e4.se_volume = levels[0];
    if (levels[1] == 0) {
        DAT_801b24e4.bgm_volume = levels[1];
    } else {
        DAT_801b24e4.bgm_volume = levels[1] - 1;
    }
    DAT_801b24e4.unkfd = levels[2] + 64;
    if (DAT_801b24e4.unk100 == 1) {
        DAT_8009c270 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c274 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c278 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
    }
}

ADDRESS(0x8001f004, 0x270)
void func_8001f004(void)
{
    u8 options[5];
    s32 row = 0;
    s32 result = MENU_NONE;
    s32 last = 4;
    u32 pad;
    s32 i;

    options[0] = DAT_801b24e4.unkf9;
    options[1] = DAT_801b24e4.unkfa;
    options[2] = DAT_801b24e4.unkfb;
    options[3] = DAT_801b24e4.unkfc;
    options[4] = DAT_801b24e4.unk100;
    for (;;) {
        if (result != MENU_NONE) {
            func_80027a40();
            break;
        }
        func_800279d8();
        pad = func_800279a4();
        if (pad & PADLup) {
            DAT_8009c268 = 0;
            func_8002792c(0xc);
            if (row == 0) {
                row = last;
            } else {
                row--;
            }
        } else if (pad & PADLdown) {
            DAT_8009c268 = 0;
            func_8002792c(0xc);
            if (row == last) {
                row = 0;
            } else {
                row++;
            }
        } else if ((pad & DAT_8009c24c) || (pad & PADLright) || (pad & PADLleft)) {
            func_8002792c(0xd);
            if (row >= 2) {
                options[row] = !options[row];
            } else if ((pad & DAT_8009c24c) || (pad & PADLright)) {
                if (options[row] == 0) {
                    options[row] = 2;
                } else {
                    options[row]--;
                }
            } else {
                if (options[row] == 2) {
                    options[row] = 0;
                } else {
                    options[row]++;
                }
            }
        } else if (pad & DAT_8009c250) {
            func_8002792c(0xe);
            result = -1;
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(5, 5, row, 1);
            func_800246bc(options);
            func_800270f8();
        }
    }
    DAT_801b24e4.unkf9 = options[0];
    DAT_801b24e4.unkfa = options[1];
    DAT_801b24e4.unkfb = options[2];
    DAT_801b24e4.unkfc = options[3];
    DAT_801b24e4.unk100 = options[4];
    func_8001f274();
}

ADDRESS(0x8001f274, 0xa8)
void func_8001f274(void)
{
    s32 shade;

    if (DAT_801b24e4.unk100 == 0) {
        if (DAT_801b24e4.unkfd > 128) {
            shade = 0x30 - (DAT_801b24e4.unkfd - 128) / 2;
        } else {
            shade = 0x30;
        }
        DAT_8009c26c = 0x57;
        DAT_8009c270 = shade;
        DAT_8009c274 = shade;
        DAT_8009c278 = shade;
        DAT_8009c27c = 0x80;
    } else {
        DAT_8009c26c = 0x78;
        DAT_8009c27c = 0x30;
        DAT_8009c270 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c274 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c278 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
    }
}

ADDRESS(0x8001f31c, 0x1a4)
void func_8001f31c(void)
{
    u8 style = DAT_801b24e4.unkfe;
    u8 layout = DAT_801b24e4.unkff;
    s32 row = 0;
    s32 result = MENU_NONE;
    u32 pad;
    s32 i;

    for (;;) {
        if (result != MENU_NONE) {
            func_80027a40();
            break;
        }
        func_800279d8();
        pad = func_800279a4();
        if (pad & DAT_8009c250) {
            func_8002792c(0xe);
            result = -1;
        } else if ((pad & PADLup) || (pad & PADLdown)) {
            func_8002792c(0xc);
            row = row == 0;
        } else if (pad & PADLleft) {
            func_8002792c(0xc);
            if (row == 0) {
                if (style != 0) {
                    style--;
                }
            } else if (layout != 0) {
                layout--;
            }
        } else if (pad & PADLright) {
            func_8002792c(0xc);
            if (row == 0) {
                style++;
                if (style >= 4) {
                    style = 3;
                }
            } else {
                layout++;
                if (layout >= 7) {
                    layout = 6;
                }
            }
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80023fd4(style, layout, row);
            func_800270f8();
        }
    }
    DAT_801b24e4.unkfe = style;
    DAT_801b24e4.unkff = layout;
    func_8001f4c0();
}

ADDRESS(0x8001f4c0, 0x3f4)
void func_8001f4c0(void)
{
    switch (DAT_801b24e4.unkfe) {
    case 0:
        DAT_80081868[4] = PADRup;
        DAT_80081868[6] = PADRleft;
        DAT_80081868[7] = PADRright;
        DAT_80081868[5] = PADRdown;
        break;
    case 1:
        DAT_80081868[4] = PADRleft;
        DAT_80081868[6] = PADRup;
        DAT_80081868[7] = PADRright;
        DAT_80081868[5] = PADRdown;
        break;
    case 2:
        DAT_80081868[4] = PADRup;
        DAT_80081868[6] = PADRleft;
        DAT_80081868[7] = PADRdown;
        DAT_80081868[5] = PADRright;
        break;
    case 3:
        DAT_80081868[4] = PADRleft;
        DAT_80081868[6] = PADRup;
        DAT_80081868[7] = PADRdown;
        DAT_80081868[5] = PADRright;
        break;
    }
    switch (DAT_801b24e4.unkff) {
    case 0:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADL1;
        DAT_80081868[9] = PADL2;
        DAT_80081868[10] = PADR1;
        DAT_80081868[11] = PADR2;
        break;
    case 1:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADR1;
        DAT_80081868[9] = PADL1;
        DAT_80081868[10] = PADR2;
        DAT_80081868[11] = PADL2;
        break;
    case 2:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADL1;
        DAT_80081868[9] = PADR1;
        DAT_80081868[10] = PADL2;
        DAT_80081868[11] = PADR2;
        break;
    case 3:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADL1;
        DAT_80081868[3] = PADR1;
        DAT_80081868[8] = PADLleft;
        DAT_80081868[9] = PADL2;
        DAT_80081868[10] = PADLright;
        DAT_80081868[11] = PADR2;
        break;
    case 4:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADR1;
        DAT_80081868[3] = PADR2;
        DAT_80081868[8] = PADLleft;
        DAT_80081868[9] = PADL1;
        DAT_80081868[10] = PADLright;
        DAT_80081868[11] = PADL2;
        break;
    case 5:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADL1;
        DAT_80081868[3] = PADL2;
        DAT_80081868[8] = PADLleft;
        DAT_80081868[9] = PADR1;
        DAT_80081868[10] = PADLright;
        DAT_80081868[11] = PADR2;
        break;
    case 6:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADL2;
        DAT_80081868[9] = PADR1;
        DAT_80081868[10] = PADR2;
        DAT_80081868[11] = PADL1;
        break;
    }
    if (DAT_801b24e4.unkfe < 2) {
        DAT_8009c24c = PADRright;
        DAT_8009c250 = PADRdown;
        DAT_8009c254 = 1;
        DAT_8009c258 = 3;
    } else {
        DAT_8009c24c = PADRdown;
        DAT_8009c250 = PADRright;
        DAT_8009c254 = 3;
        DAT_8009c258 = 1;
    }
}

ADDRESS(0x8001f8b4, 0x1ac)
s32 func_8001f8b4(void)
{
    s32 choice = 1;
    s32 result = MENU_NONE;
    u8 *mailbox = MAILBOX_BYTE_8;
    MenuText labels[2];
    u32 pad;
    s32 i;

    labels[0].x = 0xab;
    labels[0].y = 0xa2;
    labels[0].text[0] = 0x18;
    labels[0].text[1] = 4;
    labels[0].text[2] = 0x12;
    labels[0].text[3] = 0xff;
    labels[1].x = 0xe5;
    labels[1].y = 0xa2;
    labels[1].text[0] = 0x0d;
    labels[1].text[1] = 0x0e;
    labels[1].text[2] = 0xff;
    for (;;) {
        if (result != MENU_NONE) {
            func_80027a40();
            return result;
        }
        func_800279d8();
        pad = func_800279a4();
        if ((pad & PADLright) || (pad & PADLleft) || (pad & PADLup) || (pad & PADLdown)) {
            DAT_8009c268 = 0;
            func_8002792c(0xc);
            choice = choice == 0;
        } else if (pad & DAT_8009c24c) {
            DAT_8009c268 = 0;
            func_8002792c(0xd);
            if (choice == 0) {
                *mailbox = 1;
                result = 0;
            } else {
                result = -1;
            }
        } else if (pad & DAT_8009c250) {
            DAT_8009c268 = 0;
            func_8002792c(0xe);
            result = -1;
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(3, 5, 4, 0);
            func_80025b24(&labels[0], &labels[1], choice);
            func_800270f8();
        }
    }
}

ADDRESS(0x8001fa60, 0x1fc)
s32 func_8001fa60(void)
{
    s32 result = -1;
    u8 *mailbox = MAILBOX_BYTE_A;
    struct DIRENTRY dir[15];
    MenuText text[4];
    s32 status;
    s32 i;
    s32 found;

    func_80027198();
    func_8002090c(text);
    func_8001eb3c(9, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    func_80019ab4();
    func_80028034();
    for (i = 9; i != -1; i--) {
        found = func_800280d4();
        if (found == 0) {
            break;
        }
    }
    if (found != 0 && found != 2) {
        func_8001fe9c(text);
        func_8001eb3c(9, text, 2, 0x3c, 0x57, 0xc6, 0x42, 1);
    } else {
        found = func_800281c4(dir, &status);
        if (found == 1 && status == 0) {
            found = func_800281c4(dir, &status);
        }
        if (status != 0) {
            goto loaded;
        }
        if (found != 1) {
            goto done;
        }
        func_8001ff94(text);
        func_8001eb3c(9, text, 2, 0x46, 0x57, 0xb2, 0x42, found);
    }
    func_80027a40();
    func_80027a70();
    func_80027a40();
done:
    func_80028090();
    func_80027310(0);
    return -1;
loaded:
    if (*mailbox == 1) {
        func_8002792c(0xc);
        func_80027a40();
        result = func_8001fc5c(dir);
    }
    func_80028090();
    func_80027310(0);
    return result;
}

ADDRESS(0x8001fc5c, 0x240)
s32 func_8001fc5c(struct DIRENTRY *dir)
{
    MenuList list;
    MenuEntry entries[8];
    u32 experience[8];
    u8 levels[8];
    s32 numbers[8];
    MenuText text[3];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    s32 error;

    result = MENU_NONE;
    confirmed = 0;
    n = func_8001ea34(dir, entries->text, experience, levels, numbers);
    func_80027688(&list, 3, 6);
    list.rows = 5;
    list.y = 0x73;
    list.count = n;
    list.entries = entries;
    list.counts = levels;
    list.values = experience;
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 6, 8, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = numbers[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        func_800222fc(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            func_8002792c(0xc);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025468(&list, 8);
            func_800270f8();
        }
    }
    if (result != -1) {
        func_80020dec(text);
        func_8001eb3c(9, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
        error = func_8002860c(result);
        if (error != 0) {
            func_80020e6c(text, error);
            func_8001eb3c(9, text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            func_80027a40();
            func_80027a70();
            func_80027a40();
        }
    }
    return result;
}

ADDRESS(0x8001fe9c, 0xf8)
void func_8001fe9c(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy(lines->text, &DAT_8007f230[25], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy(lines->text, &DAT_8007f230[26], sizeof(MenuEntry));
}

ADDRESS(0x8001ff94, 0xf8)
void func_8001ff94(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy(lines->text, &DAT_8007f230[28], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy(lines->text, &DAT_8007f230[29], sizeof(MenuEntry));
}

ADDRESS(0x8002008c, 0x33c)
void func_8002008c(void)
{
    struct DIRENTRY dir[15];
    MenuList list;
    MenuEntry entries[8];
    u32 experience[8];
    u8 levels[8];
    s32 numbers[8];
    MenuText text[2];
    s32 status;
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    s32 found;

    confirmed = 0;
    result = MENU_NONE;
    func_80027198();
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqPause(DAT_801b09fc.seq_id);
    }
    func_80019ab4();
    func_8002090c(text);
    func_80020fe8(text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    func_80028034();
    for (i = 9; i != -1; i--) {
        if (func_800280d4() == 0) {
            break;
        }
    }
    found = func_800281c4(dir, &status);
    if (found == 1 && status == 0) {
        found = func_800281c4(dir, &status);
    }
    n = func_8001ea34(dir, entries->text, experience, levels, numbers);
    if (found == 0 || status == 0) {
        entries[n].text[0] = 0x0d;
        entries[n].text[1] = 4;
        entries[n].text[2] = 0x16;
        entries[n].text[3] = 0xff;
        experience[n] = -1;
        levels[n] = 0xff;
        numbers[n] = 0xff;
        n++;
    }
    func_80027688(&list, 3, 5);
    list.rows = 5;
    list.y = 0x73;
    list.entries = entries;
    list.counts = levels;
    list.count = n;
    list.values = experience;
    func_8002792c(0xc);
    func_80027a40();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 7, 9, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = list.cursor;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        func_800222fc(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            func_8002792c(0xc);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025468(&list, 9);
            func_800270f8();
        }
    }
    if (result != -1) {
        func_800203c8(numbers[result]);
    }
    func_80028090();
    func_80027310(0);
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqReplay(DAT_801b09fc.seq_id);
    }
}

ADDRESS(0x800203c8, 0x198)
void func_800203c8(s32 number)
{
    MenuText text[4];
    s32 status;

    status = func_800280d4();
    if (status != 0) {
        if (status != 2) {
            if (status == 1) {
                func_8002098c(text);
            } else {
                func_80020a84(text);
            }
            func_80020fe8(text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            func_80027a40();
            func_80027a70();
            func_80027a40();
            return;
        }
        if (func_80020560() != 0) {
            func_80020b7c(text);
            func_80020fe8(text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            func_80027a40();
            func_80027a70();
            func_80027a40();
            return;
        }
        func_800285dc();
    }
    func_80020d6c(text);
    func_80020fe8(text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    status = func_80028750(number);
    if (status == 0) {
        return;
    }
    if (status == 1) {
        func_80020a84(text);
        func_80020fe8(text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
    } else {
        func_80020c74(text);
        func_80020fe8(text, 2, 0x3c, 0x57, 0xc6, 0x42, 1);
    }
    func_80027a40();
    func_80027a70();
    func_80027a40();
}

ADDRESS(0x80020560, 0x3ac)
s32 func_80020560(void)
{
    s32 selected = 1;
    MenuText labels[2];
    MenuText lines[5];
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    chosen = -1;
    confirmed = 0;
    labels[0].text[0] = 0x18;
    labels[0].text[1] = 4;
    labels[0].text[2] = 0x12;
    labels[0].text[3] = 0xff;
    labels[1].text[0] = 0x0d;
    labels[1].text[1] = 0x0e;
    labels[1].text[2] = 0xff;
    lines[0].x = 0x4c;
    labels[0].x = DAT_8007e660[10].x;
    labels[0].y = DAT_8007e660[10].y;
    labels[1].x = DAT_8007e660[11].x;
    labels[1].y = DAT_8007e660[11].y;
    lines[0].y = 0x7a;
    memcpy(lines[0].text, &DAT_8007f230[36], sizeof(MenuEntry));
    lines[1].x = 0x4c;
    lines[1].y = 0x89;
    memcpy(lines[1].text, &DAT_8007f230[30], sizeof(MenuEntry));
    lines[2].x = 0x4c;
    lines[2].y = 0x98;
    memcpy(lines[2].text, &DAT_8007f230[31], sizeof(MenuEntry));
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            func_80027a40();
        }
        switch (chosen) {
        case 0:
            result = 0;
            break;
        case 1:
            result = -1;
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = func_800221e8(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025b24(&labels[0], &labels[1], selected);
            func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], &DAT_8007e660[33]);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &DAT_8007e660[33]);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &lines[0]);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &lines[1]);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &lines[2]);
            func_80026acc(0x32, 0x68, 0xdc, 0x51);
            func_800269c0(0x5f, 0xd6, DAT_8009c254);
            func_80026570(0x69, 0xd6, "select", 0, 0);
            func_800269c0(0xa0, 0xd6, DAT_8009c258);
            func_80026570(0xaa, 0xd6, "return", 0, 0);
            func_800270f8();
        }
    }
    return result;
}

ADDRESS(0x8002090c, 0x80)
void func_8002090c(MenuText *line)
{
    line->x = 0x4c;
    line->y = 0x70;
    memcpy(line->text, &DAT_8007f230[24], sizeof(MenuEntry));
}

ADDRESS(0x8002098c, 0xf8)
void func_8002098c(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy(lines->text, &DAT_8007f230[36], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy(lines->text, &DAT_8007f230[38], sizeof(MenuEntry));
}

ADDRESS(0x80020a84, 0xf8)
void func_80020a84(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy(lines->text, &DAT_8007f230[33], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy(lines->text, &DAT_8007f230[27], sizeof(MenuEntry));
}

ADDRESS(0x80020b7c, 0xf8)
void func_80020b7c(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy(lines->text, &DAT_8007f230[32], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy(lines->text, &DAT_8007f230[39], sizeof(MenuEntry));
}

ADDRESS(0x80020c74, 0xf8)
void func_80020c74(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy(lines->text, &DAT_8007f230[40], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy(lines->text, &DAT_8007f230[41], sizeof(MenuEntry));
}

ADDRESS(0x80020d6c, 0x80)
void func_80020d6c(MenuText *line)
{
    line->x = 0x4c;
    line->y = 0x70;
    memcpy(line->text, &DAT_8007f230[10], sizeof(MenuEntry));
}

ADDRESS(0x80020dec, 0x80)
void func_80020dec(MenuText *line)
{
    line->x = 0x4c;
    line->y = 0x70;
    memcpy(line->text, &DAT_8007f230[37], sizeof(MenuEntry));
}

ADDRESS(0x80020e6c, 0x17c)
void func_80020e6c(MenuText *lines, s32 error)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    if (error == 1) {
        memcpy(lines->text, &DAT_8007f230[34], sizeof(MenuEntry));
    } else {
        memcpy(lines->text, &DAT_8007f230[35], sizeof(MenuEntry));
    }
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy(lines->text, &DAT_8007f230[27], sizeof(MenuEntry));
}

ADDRESS(0x80020fe8, 0x12c)
void func_80020fe8(MenuText *lines, s32 count, s32 x, s32 y, s32 w, s32 h, s32 back)
{
    s32 i;
    s32 j;
    MenuText *line;

    for (i = 0; i < 2; i++) {
        func_80026fe4();
        line = lines;
        func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], &DAT_8007e660[33]);
        func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &DAT_8007e660[33]);
        for (j = 0; j < count; j++) {
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], line);
            line++;
        }
        func_80026acc(x, y, w, h);
        if (back == 1) {
            func_800269c0(0x80, 0xd6, DAT_8009c258);
            func_80026570(0x8a, 0xd6, "return", 0, 0);
        }
        func_800270f8();
    }
}

ADDRESS(0x80021114, 0x184)
void func_80021114(s32 shop)
{
    s32 selected = 0;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    func_80027198();
    for (i = 0; i < 2; i++) {
        func_80026fe4();
        func_800252f4(6, 2, selected, 0);
        func_800270f8();
    }
    func_8002792c(0xc);
    func_80027a40();
    func_80027bb4();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            func_80027a40();
        }
        switch (chosen) {
        case 0:
            func_80021298(shop);
            break;
        case 1:
            func_80021724(shop);
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = func_800221e8(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(6, 2, selected, 0);
            func_800270f8();
        }
    }
    func_80027c8c();
    func_80027310(0);
}

ADDRESS(0x80021298, 0x40c)
void func_80021298(s32 shop)
{
    u32 pad = 0;
    MenuList list;
    MenuEntry entries[152];
    u8 counts[152];
    u32 prices[152];
    u8 ids[152];
    s32 confirmed = 0;
    s32 result = MENU_NONE;
    s32 mode;
    u8 cursors[5] = {0, 0xb, 7, 2, 0};
    s32 kind;
    u8 *bag = DAT_800c85e8[0];
    s32 n;
    s32 i;
    u8 item;

    kind = 0xa;
    if (shop == 0) {
        kind = 0xd;
    }
    mode = shop != 0;
    n = func_8001aea4(DAT_80080718[shop], entries, counts, ids, 0, 0x95);
    func_800216a4(bag, counts, prices, ids, 0, n, shop);
    func_80027688(&list, 6, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    list.values = prices;
    func_80027728(&list, cursors[shop]);
    func_80027a9c(ids[list.cursor]);
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 3, kind, item);
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
            func_80024f88(&list, 2, item);
        }
        pad = func_800222fc(&list, ids, &confirmed, &result, mode);
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (shop == 0) {
                if ((u32)DAT_801b24e4.unk50 < list.values[list.cursor] || bag[item] + 1 >= 100) {
                    goto refuse;
                }
            } else if ((u32)DAT_801b24e4.unk50 < list.values[list.cursor] * DAT_8009c25c
                       || bag[item] + DAT_8009c25c >= 100) {
            refuse:
                func_8002792c(0xe);
                confirmed = 0;
                goto draw;
            }
            func_8002792c(0xd);
        }
    draw:
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, kind);
            func_800270f8();
        }
    }
    if (result != -1) {
        if (shop == 0) {
            DAT_8009c25c = 1;
        }
        DAT_801b24e4.unk50 -= list.values[list.cursor] * DAT_8009c25c;
        bag[result] += DAT_8009c25c;
        if (shop == 0 && result != 0x6b) {
            DAT_80080718[0][result] -= 1;
        }
    }
}

ADDRESS(0x800216a4, 0x80)
void func_800216a4(u8 *bag, u8 *counts, u32 *prices, u8 *ids, s32 first, s32 end, s32 shop)
{
    for (; first < end; first++) {
        *counts++ = bag[*ids];
        *prices++ = DAT_80080a9c[shop][*ids];
        ids++;
    }
}

ADDRESS(0x80021724, 0x328)
void func_80021724(s32 shop)
{
    static u8 cursor = 0;
    MenuList list;
    MenuEntry entries[152];
    u8 counts[152];
    u32 prices[152];
    u8 ids[152];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u32 pad;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    n = func_8001af88(DAT_800c85e8[0], entries, counts, ids, 0, 0x95);
    func_80021a4c(prices, ids, 0, n, shop);
    func_80027688(&list, 6, 1);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    list.values = prices;
    func_80027728(&list, cursor);
    pad = 0;
    if (list.count != 0) {
        func_80027a9c(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = func_80024c70(&list, 4, 0xb, item);
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
        pad = func_800222fc(&list, ids, &confirmed, &result, 2);
        cursor = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (counts[list.cursor] < DAT_8009c25c) {
                func_8002792c(0xe);
                confirmed = 0;
            } else {
                func_8002792c(0xd);
            }
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, 0xb);
            func_800270f8();
        }
    }
    if (result != -1) {
        DAT_801b24e4.unk50 += list.values[list.cursor] * DAT_8009c25c;
        DAT_800c85e8[0][result] -= DAT_8009c25c;
        DAT_80080718[0][result] += DAT_8009c25c;
        if (DAT_80080718[0][result] >= 100) {
            DAT_80080718[0][result] = 99;
        }
    }
}

ADDRESS(0x80021a4c, 0x60)
void func_80021a4c(u32 *prices, u8 *ids, s32 first, s32 end, s32 shop)
{
    for (; first < end; first++) {
        *prices++ = DAT_800811a4[shop][*ids];
        ids++;
    }
}

ADDRESS(0x80021aac, 0x2e8)
s32 func_80021aac(void)
{
    MenuList list;
    MenuEntry entries[60];
    u8 counts[64];
    u8 ids[64];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u8 item;

    confirmed = 0;
    result = MENU_NONE;
    func_80027198();
    n = func_8001b150(DAT_800c85e8[0], entries, counts, ids, 0, 0x95);
    func_80027688(&list, 8, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    func_80027bb4();
    item = ids[list.cursor];
    if (list.count != 0) {
        func_80027a9c(item);
    }
    for (i = 0; i < 2; i++) {
        func_80026fe4();
        if (list.count != 0) {
            func_80025be8(item);
        }
        func_80025468(&list, 0xc);
        func_800270f8();
    }
    func_8002792c(0xc);
    func_80027a40();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            result = item;
        }
        if (result != MENU_NONE) {
            break;
        }
        func_800222fc(&list, ids, &confirmed, &result, 0);
        item = ids[list.cursor];
        if (confirmed == 1) {
            func_8002792c(0xd);
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list.count != 0) {
                func_80025be8(item);
            }
            func_80025468(&list, 0xc);
            func_800270f8();
        }
    }
    func_80027c8c();
    if (result != -1) {
        DAT_80081780[result] = 4;
    }
    switch (result) {
    case 2:
        result = 300;
        break;
    case 8:
        result = 301;
        break;
    case 10:
        result = 302;
        break;
    case 32:
        result = 303;
        break;
    case 33:
        result = 304;
        break;
    case 38:
        result = 305;
        break;
    case 39:
        result = 306;
        break;
    case 41:
        result = 307;
        break;
    case 56:
        result = 308;
        break;
    case 58:
        result = 309;
        break;
    case 82:
        result = 310;
        break;
    case 89:
        result = 311;
        break;
    case 90:
        result = 312;
        break;
    case 91:
        result = 313;
        break;
    case 92:
        result = 314;
        break;
    case 126:
        result = 315;
        break;
    case 134:
        result = 316;
        break;
    case 140:
        result = 317;
        break;
    case 143:
        result = 318;
        break;
    }
    func_80027310(0);
    return result;
}

ADDRESS(0x80021d94, 0x278)
s32 func_80021d94(void)
{
    MenuList list;
    MenuEntry entries[20];
    u8 counts[24];
    u8 ids[24];
    s32 confirmed;
    s32 result;
    u8 saved;
    u8 item;
    s32 n;
    s32 i;

    confirmed = 0;
    result = MENU_NONE;
    func_80027198();
    saved = DAT_801b24e4.unkfb;
    DAT_801b24e4.unkfb = 0;
    n = func_8001aea4(DAT_80080718[5], entries, counts, ids, 0, 0x95);
    for (i = 0; i < 5; i++) {
        counts[i] = DAT_800c85e8[0][ids[i]] + DAT_800c85e8[1][ids[i]];
    }
    func_80027688(&list, 8, 1);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    for (i = 0; i < 2; i++) {
        func_80026fe4();
        func_80025468(&list, 0xc);
        func_800270f8();
    }
    func_8002792c(0xc);
    func_80027a40();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            func_80027a40();
        }
        if (confirmed == 1) {
            func_8002792c(0xd);
            result = func_80024c70(&list, 9, 0xc, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        func_800222fc(&list, 0, &confirmed, &result, 0);
        item = ids[list.cursor];
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025468(&list, 0xc);
            func_800270f8();
        }
    }
    if (result != -1) {
        if (DAT_800c85e8[0][0x7c] != 0) {
            DAT_800c85e8[0][0x7c]--;
        } else if (DAT_800c85e8[1][0x7c] != 0) {
            DAT_800c85e8[1][0x7c]--;
        }
    }
    DAT_801b24e4.unkfb = saved;
    func_80027310(0);
    return result;
}

ADDRESS(0x8002200c, 0x1dc)
s32 func_8002200c(s32 mode)
{
    s32 selected = 0;
    s32 paid;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    paid = -1;
    func_80027198();
    mode &= 0xf;
    for (i = 0; i < 2; i++) {
        func_80026fe4();
        func_800252f4(7, 2, selected, 0);
        if (mode == 0) {
            func_80025e80();
        }
        func_800270f8();
    }
    func_8002792c(0xc);
    func_80027a40();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            func_80027a40();
        }
        if (chosen == 0) {
            if (mode == 0) {
                if ((u32)DAT_801b24e4.unk50 >= 150) {
                    DAT_801b24e4.unk50 -= 150;
                    paid = 0;
                    break;
                }
                func_8002792c(0xe);
                chosen = -1;
                result = MENU_NONE;
            } else {
                paid = 0;
                break;
            }
        }
        if (chosen == 1 || result != MENU_NONE) {
            break;
        }
        selected = func_800221e8(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_800252f4(7, 2, selected, 0);
            if (mode == 0) {
                func_80025e80();
            }
            func_800270f8();
        }
    }
    func_80027310(0);
    return paid;
}

ADDRESS(0x800221e8, 0x114)
s32 func_800221e8(s32 selected, s32 last, s32 *chosen, s32 *confirmed, s32 *result)
{
    u32 pad;

    *chosen = -1;
    *confirmed = 0;
    func_800279d8();
    pad = func_800279a4();
    if (pad & PADLup) {
        DAT_8009c268 = 0;
        func_8002792c(0xc);
        if (selected == 0) {
            selected = last;
        } else {
            selected--;
        }
    } else if (pad & PADLdown) {
        DAT_8009c268 = 0;
        func_8002792c(0xc);
        if (selected == last) {
            selected = 0;
        } else {
            selected++;
        }
    } else if (pad & DAT_8009c24c) {
        func_8002792c(0xd);
        *confirmed = 1;
        *chosen = selected;
    } else if (pad & DAT_8009c250) {
        func_8002792c(0xe);
        *result = -1;
    }
    return selected;
}

ADDRESS(0x800222fc, 0x4f0)
u32 func_800222fc(MenuList *list, u8 *ids, s32 *confirmed, s32 *result, s32 mode)
{
    u32 price;
    u8 count;
    u32 pad;
    s32 next;

    if (mode == 1) {
        price = list->values[list->cursor];
    }
    if (mode != 0) {
        count = list->counts[list->cursor];
    }
    *confirmed = 0;
    func_800279d8();
    pad = func_800279a4();
    if (list->count == 0) {
        if (pad != 0) {
            func_8002792c(0xe);
            *result = -1;
        }
    } else if (pad & PADLup) {
        func_8002792c(0xc);
        if (list->cursor != 0) {
            list->cursor--;
            if (list->row == 0) {
                list->first--;
            } else {
                list->row--;
            }
        } else {
            list->cursor = list->count - 1;
            if (list->count < list->rows) {
                list->first = 0;
                list->row = list->count - 1;
            } else {
                list->first = list->count - list->rows;
                list->row = list->rows - 1;
            }
        }
        if (ids != 0) {
            func_80027a9c(ids[list->cursor]);
        }
    } else if (pad & PADLdown) {
        func_8002792c(0xc);
        if (list->cursor < list->count - 1) {
            list->cursor++;
            if (list->row == list->rows - 1) {
                list->first++;
            } else {
                list->row++;
            }
        } else {
            list->cursor = 0;
            list->first = 0;
            list->row = 0;
        }
        if (ids != 0) {
            func_80027a9c(ids[list->cursor]);
        }
    } else if (pad & PADLright) {
        if (mode == 1) {
            if ((u32)DAT_801b24e4.unk50 < (DAT_8009c25c + 1) * price
                || (next = count + 1) + DAT_8009c25c >= 100) {
                goto refuse;
            }
        } else if (mode == 2) {
            if (count < DAT_8009c25c + 1) {
                goto refuse;
            }
        } else {
            goto debug;
        }
        func_8002792c(0xc);
        DAT_8009c25c++;
        goto debug;
    refuse:
        func_8002792c(0xe);
    } else if (pad & PADLleft) {
        if (mode != 0 && DAT_8009c25c >= 2) {
            func_8002792c(0xc);
            DAT_8009c25c--;
        }
    } else if (pad & DAT_8009c24c) {
        *confirmed = 1;
    } else if (pad & DAT_8009c250) {
        func_8002792c(0xe);
        *result = -1;
    }
debug:
    if (pad & PADselect) {
        if (pad & PADR1) {
            DAT_8009c39c.vx += 32;
        }
        if (pad & PADR2) {
            DAT_8009c39c.vx -= 32;
        }
        if (pad & PADL1) {
            DAT_8009c39c.vz += 32;
        }
        if (pad & PADL2) {
            DAT_8009c39c.vz -= 32;
        }
        if (pad & PADRup) {
            DAT_8009c3a4++;
        }
        if (pad & PADRleft) {
            DAT_8009c3a4--;
        }
    }
    if (pad & PADstart) {
        if (pad & PADR1) {
            DAT_8009c394.vx += 32;
        }
        if (pad & PADR2) {
            DAT_8009c394.vx -= 32;
        }
        if (pad & PADL1) {
            DAT_8009c394.vy += 32;
        }
        if (pad & PADL2) {
            DAT_8009c394.vy -= 32;
        }
        if (pad & PADRup) {
            DAT_8009c394.vz += 32;
        }
        if ((pad & PADRleft) && DAT_8009c394.vz > 640) {
            DAT_8009c394.vz -= 32;
        }
    }
    if ((pad & PADselect) || (pad & PADstart)) {
        DAT_8009c248 = 0;
        VSync(2);
    }
    return pad;
}

ADDRESS(0x800227ec, 0xb14)
void func_800227ec(void)
{
    MenuText label;
    MenuText number;
    s32 dy;

    label.x = 0xa3;
    label.y = 0x24;
    memcpy(label.text, &DAT_8007f230[12], 20);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.experience, 7, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.y += 17;
    memcpy(label.text, &DAT_8007f230[13], 20);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y += 17;
    func_800277c0(DAT_801b24e4.level, 7, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 7;
    label.text[1] = 0xf;
    label.text[2] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 63;
    number.y += 17;
    func_800277c0(DAT_801b24e4.hp, 4, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.text[0] = 0x14;
    number.text[1] = 0xff;
    number.x += 28;
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.x += 7;
    func_800277c0(DAT_801b24e4.hp_max, 4, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 0xc;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 63;
    number.y += 17;
    func_800277c0(DAT_801b24e4.unk1c, 4, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.text[0] = 0x14;
    number.text[1] = 0xff;
    number.x += 28;
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.x += 7;
    func_800277c0(DAT_801b24e4.unk1a, 4, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 0xf;
    label.text[1] = 0x16;
    label.text[2] = 0x11;
    label.text[3] = 0x3b;
    label.text[4] = 0x16;
    label.text[5] = 8;
    label.text[6] = 0x12;
    label.text[7] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 70;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk40, 5, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.x = label.x + 98;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk4c + 1, 4, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 0x3b;
    label.text[1] = 0xff;
    label.x += 105;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.x = 0xa3;
    label.text[0] = 0xc;
    label.text[1] = 0;
    label.text[2] = 6;
    label.text[3] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.x += 35;
    func_80027414();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7d04;
    func_80027494(label.x, label.y - 1, 14, 14, 0, 0xc0, 14, 14);
    func_80027530(0x1e);
    number.x = label.x;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk42[4], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x += 49;
    func_80027414();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7d44;
    func_80027494(label.x, label.y - 1, 14, 14, 0x10, 0xc0, 14, 14);
    func_80027530(0x1e);
    number.x = label.x;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk42[0], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa3;
    label.y += 15;
    label.x += 35;
    func_80027414();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7d84;
    func_80027494(label.x, label.y - 1, 14, 14, 0x20, 0xc0, 14, 14);
    func_80027530(0x1e);
    number.x = label.x;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk42[1], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x += 49;
    func_80027414();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7dc4;
    func_80027494(label.x, label.y - 1, 14, 14, 0x30, 0xc0, 14, 14);
    func_80027530(0x1e);
    number.x = label.x;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk42[3], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa3;
    label.y += 15;
    label.x += 35;
    func_80027414();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7e04;
    func_80027494(label.x, label.y - 1, 14, 14, 0x40, 0xc0, 14, 14);
    func_80027530(0x1e);
    number.x = label.x;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk42[2], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa3;
    label.y += 17;
    memcpy(label.text, &DAT_8007f230[16], 12);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    dy = 17;
    number.x = label.x + 56;
    number.y += 17;
    if (DAT_801b24e4.unk84 != 0) {
        memcpy(number.text, &DAT_8007f230[17], 12);
    } else if (DAT_801b24e4.unk7a != 0) {
        memcpy(number.text, &DAT_8007f230[18], 12);
    } else if (DAT_801b24e4.unk82 != 0) {
        memcpy(number.text, &DAT_8007f230[19], 12);
    } else if (DAT_801b24e4.unk78 != 0) {
        memcpy(number.text, &DAT_8007f230[20], 12);
    } else if (DAT_801b24e4.unk7e != 0) {
        memcpy(number.text, &DAT_8007f230[21], 12);
    } else {
        memcpy(number.text, &DAT_8007f230[22], 12);
    }
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &number);
    label.y += dy;
    memcpy(label.text, &DAT_8007f230[23], 10);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y += dy;
    func_800277c0(DAT_801b24e4.unk50, 7, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    func_80026acc(0x9b, 0x1a, 0x8d, 0xb4);
}

ADDRESS(0x80023300, 0x9e8)
void func_80023300(void)
{
    MenuText label;
    MenuText number;

    label.x = 0x1e;
    label.y = 0x22;
    memcpy(label.text, &DAT_8007f230[14], 10);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.text[0] = 0x7f;
    label.text[1] = 0x7f;
    label.text[2] = 0x12;
    label.text[3] = 0xb;
    label.text[4] = 0;
    label.text[5] = 0x12;
    label.text[6] = 0x7;
    label.text[7] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk54[0], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x1;
    label.text[3] = 0xb;
    label.text[4] = 0xe;
    label.text[5] = 0x16;
    label.text[6] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk54[1], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x12;
    label.text[3] = 0x13;
    label.text[4] = 0;
    label.text[5] = 0x1;
    label.text[6] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk54[2], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x7;
    label.text[3] = 0xe;
    label.text[4] = 0xb;
    label.text[5] = 0x18;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk54[3], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x5;
    label.text[3] = 0x8;
    label.text[4] = 0x11;
    label.text[5] = 0x4;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk54[4], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x4;
    label.text[3] = 0;
    label.text[4] = 0x11;
    label.text[5] = 0x13;
    label.text[6] = 0x7;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk54[5], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0x8;
    label.text[4] = 0xd;
    label.text[5] = 0x3;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk54[6], 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0;
    label.text[4] = 0x13;
    label.text[5] = 0x4;
    label.text[6] = 0x11;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 16;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk62, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa4;
    label.y = 0x22;
    memcpy(label.text, &DAT_8007f230[15], 12);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.text[0] = 0x7f;
    label.text[1] = 0x7f;
    label.text[2] = 0x12;
    label.text[3] = 0xb;
    label.text[4] = 0;
    label.text[5] = 0x12;
    label.text[6] = 0x7;
    label.text[7] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk66, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x1;
    label.text[3] = 0xb;
    label.text[4] = 0xe;
    label.text[5] = 0x16;
    label.text[6] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk68, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x12;
    label.text[3] = 0x13;
    label.text[4] = 0;
    label.text[5] = 0x1;
    label.text[6] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk6a, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0xf;
    label.text[3] = 0xe;
    label.text[4] = 0x8;
    label.text[5] = 0x12;
    label.text[6] = 0xe;
    label.text[7] = 0xd;
    label.text[8] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk76, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x3;
    label.text[3] = 0;
    label.text[4] = 0x11;
    label.text[5] = 0xa;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk6c, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x5;
    label.text[3] = 0x8;
    label.text[4] = 0x11;
    label.text[5] = 0x4;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk6e, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x4;
    label.text[3] = 0;
    label.text[4] = 0x11;
    label.text[5] = 0x13;
    label.text[6] = 0x7;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk70, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0x8;
    label.text[4] = 0xd;
    label.text[5] = 0x3;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk72, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0;
    label.text[4] = 0x13;
    label.text[5] = 0x4;
    label.text[6] = 0x11;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 17;
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk74, 6, 0, 0, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    func_80026acc(0x14, 0x1a, 0x114, 0xb4);
    func_800269c0(0x80, 0xd6, DAT_8009c258);
    func_80026570(0x8a, 0xd6, "return", 0, 0);
}

ADDRESS(0x80023ce8, 0x2ec)
void func_80023ce8(s32 page)
{
    char names[48][12] = {
        "christy", "lyn", "", "gullick", "jens", "rene", "michael", "janan", "mark", "calmathar",
        "garnabus", "ungrund", "zul", "varde", "jack", "airon", "marcus", "jamie", "yvette", "toni",
        "priscilla", "marilyn", "ed", "robert", "krone", "jane", "john", "olivier", "sal", "giant",
        "light", "vallad", "leon", "", "", "orladin", "", "", "joe", "franz", "tim", "james",
        "alexander", "jean", "seath", "", "", "",
    };

    func_80026acc(0x1f, 0x20, 0x102, 0xb0);
    func_800269c0(0x7d, 0xbe, 8);
    func_80026570(0x8c, 0xbf, "talk", 1, 0);
    func_800269c0(0xb2, 0xbe, 9);
    func_80026570(0xc1, 0xbf, "character", 1, 0);
    func_800269c0(0x80, 0xd6, DAT_8009c258);
    func_80026570(0x8a, 0xd6, "return", 0, 0);
    if (page != -1) {
        func_80027414();
        DAT_8009c374->tpage = 0x1f;
        DAT_8009c374->clut = 0x7806;
        func_80027494(0x2c, 0x24, 0xf8, 0xc8, 0, 0, 0xf8, 0xc8);
        func_80027530(0x1e);
        if (DAT_801b24e4.unk100 == 1) {
            func_80027414();
            setRGB0(DAT_8009c374, 0x10, 0x10, 0x10);
            DAT_8009c374->tpage = 0x1f;
            DAT_8009c374->clut = 0x7806;
            func_80027494(0x2d, 0x25, 0xf8, 0xc8, 0, 0, 0xf8, 0xc8);
            func_80027530(0x1f);
        }
        func_80027414();
        setRGB0(DAT_8009c374, 0x60, 0x60, 0x60);
        DAT_8009c374->tpage = GetTPage(1, 1, 0x3c0, 0x100);
        DAT_8009c374->clut = 0x7f80;
        func_80027494(0x28, 0x98, 0x40, 0x2f, 0, 0xd0, 0x40, 0x2f);
        func_80027530(0x1e);
        func_80026570(0x6b, 0x9a, names[page], 1, 0);
    }
}

ADDRESS(0x80023fd4, 0x6e8)
void func_80023fd4(u8 style, u8 layout, s32 row)
{
    s32 order[6];
    char actions[4][8] = {"attack", "event", "magic", "status"};
    char moves[9][16] = {"move{left", "view{up", "move{right", "view{down", "forward{back", "turn", "move",
                         "turn{left", "turn{right"};
    char types[7][8] = {"type{a", "type{b", "type{c", "type{d", "type{e", "type{f", "type{g"};
    s32 i;
    s32 y;
    s32 ny;
    s32 bright;
    s32 blue;

    switch (style) {
    case 0:
        order[0] = 0;
        order[1] = 1;
        order[2] = 2;
        order[3] = 3;
        break;
    case 1:
        order[0] = 2;
        order[1] = 1;
        order[2] = 0;
        order[3] = 3;
        break;
    case 2:
        order[0] = 0;
        order[1] = 3;
        order[2] = 2;
        order[3] = 1;
        break;
    case 3:
        order[0] = 2;
        order[1] = 3;
        order[2] = 0;
        order[3] = 1;
        break;
    }
    for (i = 0, y = 0x32, ny = 0x31; i < 4; i++) {
        func_800269c0(0x89, ny, i);
        func_80026570(0x9b, y, actions[order[i]], 1, 0);
        y += 14;
        ny += 14;
    }
    switch (layout) {
    case 0:
        order[0] = 0;
        order[1] = 1;
        order[2] = 2;
        order[3] = 3;
        order[4] = 4;
        order[5] = 5;
        break;
    case 1:
        order[0] = 1;
        order[1] = 3;
        order[2] = 0;
        order[3] = 2;
        order[4] = 4;
        order[5] = 5;
        break;
    case 2:
        order[0] = 0;
        order[1] = 2;
        order[2] = 1;
        order[3] = 3;
        order[4] = 4;
        order[5] = 5;
        break;
    case 3:
        order[0] = 7;
        order[1] = 1;
        order[2] = 8;
        order[3] = 3;
        order[4] = 4;
        order[5] = 6;
        break;
    case 4:
        order[0] = 1;
        order[1] = 3;
        order[2] = 7;
        order[3] = 8;
        order[4] = 4;
        order[5] = 6;
        break;
    case 5:
        order[0] = 7;
        order[1] = 8;
        order[2] = 1;
        order[3] = 3;
        order[4] = 4;
        order[5] = 6;
        break;
    case 6:
        order[0] = 3;
        order[1] = 0;
        order[2] = 1;
        order[3] = 2;
        order[4] = 4;
        order[5] = 5;
        break;
    }
    for (i = 4, y = 0x70, ny = 0x6f; i < 10; i++) {
        func_800269c0(0x89, ny, i);
        func_80026570(0x9b, y, moves[order[i - 4]], 1, 0);
        y += 14;
        ny += 14;
    }
    func_80026570(0x32, 0x32, "action", 1, 0);
    func_80026570(0x3a, 0x40, types[style], 1, 0);
    func_80026570(0x32, 0x70, "move", 1, 0);
    func_80026570(0x3a, 0x7e, types[layout], 1, 0);
    if (DAT_8009c260 < 48) {
        bright = DAT_8009c260 + DAT_8009c26c;
        blue = DAT_8009c260 / 2 + DAT_8009c26c;
    } else {
        bright = DAT_8009c26c + 95 - DAT_8009c260;
        blue = DAT_8009c26c + 47 - DAT_8009c260 / 2;
    }
    func_80027414();
    setRGB0(DAT_8009c374, bright, bright, blue);
    DAT_8009c374->tpage = DAT_8007e564[11].unk0;
    DAT_8009c374->clut = DAT_8007e564[11].unk2;
    if (row == 0) {
        func_80027494(0x37, 0x3c, 0x35, 0xf, DAT_8007e564[11].unk4, DAT_8007e564[11].unk6,
                      DAT_8007e564[11].unk8, DAT_8007e564[11].unka);
    } else {
        func_80027494(0x37, 0x7a, 0x35, 0xf, DAT_8007e564[11].unk4, DAT_8007e564[11].unk6,
                      DAT_8007e564[11].unk8, DAT_8007e564[11].unka);
    }
    SetSemiTrans(DAT_8009c374, 1);
    func_80027530(0x31);
    func_80026acc(0x1f, 0x20, 0x102, 0xb0);
    func_800269c0(0x5b, 0xd6, 9);
    func_80026570(0x69, 0xd6, "select", 0, 0);
    func_800269c0(0xa0, 0xd6, DAT_8009c258);
    func_80026570(0xaa, 0xd6, "return", 0, 0);
}

ADDRESS(0x800246bc, 0x3bc)
void func_800246bc(u8 *options)
{
    MenuText t[3];
    s32 i;

    t[0].x = 0xab;
    t[0].y = 0x3a;
    t[0].text[0] = 0;
    t[0].text[1] = 0xff;
    t[1].x = 0xc7;
    t[1].y = 0x3a;
    t[1].text[0] = 1;
    t[1].text[1] = 0xff;
    t[2].x = 0xe3;
    t[2].y = 0x3a;
    t[2].text[0] = 2;
    t[2].text[1] = 0xff;
    for (i = 0; i < 2; i++) {
        if (*options == 2) {
            func_80026020(&DAT_8007e564[8], &t[0], DAT_8009c264);
            t[0].x += 4;
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            func_80025f38(&DAT_8007e564[8], &t[1]);
            t[1].x += 4;
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
            func_80025f38(&DAT_8007e564[8], &t[2]);
            t[2].x += 4;
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[2]);
        } else if (*options == 1) {
            func_80025f38(&DAT_8007e564[8], &t[0]);
            t[0].x += 4;
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            func_80026020(&DAT_8007e564[8], &t[1], DAT_8009c264);
            t[1].x += 4;
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
            func_80025f38(&DAT_8007e564[8], &t[2]);
            t[2].x += 4;
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[2]);
        } else {
            func_80025f38(&DAT_8007e564[8], &t[0]);
            t[0].x += 4;
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            func_80025f38(&DAT_8007e564[8], &t[1]);
            t[1].x += 4;
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
            func_80026020(&DAT_8007e564[8], &t[2], DAT_8009c264);
            t[2].x += 4;
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &t[2]);
        }
        options++;
        t[0].x = 0xab;
        t[1].x = 0xc7;
        t[2].x = 0xe3;
        t[0].y += 26;
        t[1].y += 26;
        t[2].y += 26;
    }
    t[0].x = 0xab;
    t[0].text[0] = 0x7f;
    t[0].text[1] = 0x7f;
    t[0].text[2] = 0xe;
    t[0].text[3] = 0xd;
    t[0].text[4] = 0xff;
    t[1].x = 0xe5;
    t[1].text[0] = 0x7f;
    t[1].text[1] = 0x7f;
    t[1].text[2] = 0xe;
    t[1].text[3] = 5;
    t[1].text[4] = 5;
    t[1].text[5] = 0xff;
    for (i = 0; i < 3; i++) {
        if (i == 2) {
            t[0].text[0] = 2;
            t[0].text[1] = 0xb;
            t[0].text[2] = 4;
            t[0].text[3] = 0;
            t[0].text[4] = 0x11;
            t[0].text[5] = 0xff;
            t[1].text[0] = 0x12;
            t[1].text[1] = 0xe;
            t[1].text[2] = 0xb;
            t[1].text[3] = 8;
            t[1].text[4] = 3;
            t[1].text[5] = 0xff;
        }
        if (*options == 1) {
            func_80026020(&DAT_8007e564[4], &t[0], DAT_8009c264);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            func_80025f38(&DAT_8007e564[4], &t[1]);
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
        } else {
            func_80025f38(&DAT_8007e564[4], &t[0]);
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            func_80026020(&DAT_8007e564[4], &t[1], DAT_8009c264);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
        }
        options++;
        t[0].y += 26;
        t[1].y += 26;
        t[2].y += 26;
    }
}

ADDRESS(0x80024a78, 0x1f8)
void func_80024a78(u8 *levels)
{
    s32 i;
    s32 y;
    s32 bar_y;

    func_80027610(0x32, DAT_8007e564[10].unk0);
    for (i = 0, y = 0x3c, bar_y = 0x34; i < 3; i++) {
        func_80027560();
        setRGB0(DAT_8009c378, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
        DAT_8009c378->clut = DAT_8007e564[10].unk2;
        DAT_8009c378->x0 = 0xa5;
        DAT_8009c378->y0 = bar_y;
        DAT_8009c378->w = DAT_8007e564[10].unk8;
        DAT_8009c378->h = DAT_8007e564[10].unka;
        DAT_8009c378->u0 = DAT_8007e564[10].unk4;
        DAT_8009c378->v0 = DAT_8007e564[10].unk6;
        SetSemiTrans(DAT_8009c378, DAT_801b24e4.unk100);
        func_800275e0(0x32);
        func_80027560();
        setRGB0(DAT_8009c378, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
        DAT_8009c378->clut = DAT_8007e564[9].unk2;
        DAT_8009c378->x0 = *levels++ * 96 / 128 + 172;
        DAT_8009c378->y0 = y;
        DAT_8009c378->w = DAT_8007e564[9].unk8;
        DAT_8009c378->h = DAT_8007e564[9].unka;
        DAT_8009c378->u0 = DAT_8007e564[9].unk4 + DAT_801b24e4.unk100 * 16;
        DAT_8009c378->v0 = DAT_8007e564[9].unk6;
        func_800275e0(0x31);
        y += 26;
        bar_y += 26;
    }
}

ADDRESS(0x80024c70, 0x318)
s32 func_80024c70(MenuList *list, s32 kind, s32 flags, u8 item)
{
    MenuText labels[2];
    s32 choice;
    s32 result;
    s32 pad;
    s32 i;

    labels[0].x = DAT_8007e660[10].x;
    labels[0].y = DAT_8007e660[10].y;
    labels[1].x = DAT_8007e660[11].x;
    labels[1].y = DAT_8007e660[11].y;
    choice = 0;
    result = MENU_NONE;
    flags |= 0x8000;
    if (kind == 0) {
        labels[0].text[0] = 0x14;
        labels[0].text[1] = 0x12;
        labels[0].text[2] = 4;
        labels[0].text[3] = 0xff;
    } else if (kind == 1) {
        labels[0].text[0] = 0x12;
        labels[0].text[1] = 0x13;
        labels[0].text[2] = 0xe;
        labels[0].text[3] = 0x11;
        labels[0].text[4] = 4;
        labels[0].text[5] = 0xff;
    } else if (kind == 2 || kind == 3 || kind == 4 || kind == 6 || kind == 7 || kind == 8) {
        labels[0].text[0] = 0x18;
        labels[0].text[1] = 4;
        labels[0].text[2] = 0x12;
        labels[0].text[3] = 0xff;
    } else if (kind == 5) {
        labels[0].text[0] = 0x14;
        labels[0].text[1] = 0x12;
        labels[0].text[2] = 4;
        labels[0].text[3] = 0xff;
    } else if (kind == 9) {
        labels[0].text[0] = 0xc;
        labels[0].text[1] = 0;
        labels[0].text[2] = 0xa;
        labels[0].text[3] = 4;
        labels[0].text[4] = 8;
        labels[0].text[5] = 0x13;
        labels[0].text[6] = 0xff;
    } else {
        labels[0].text[0] = 0xc;
        labels[0].text[1] = 0xe;
        labels[0].text[2] = 0x15;
        labels[0].text[3] = 4;
        labels[0].text[4] = 0xff;
    }
    if (kind == 2 || kind == 3 || kind == 4 || kind == 6 || kind == 7 || kind == 8) {
        labels[1].text[0] = 0xd;
        labels[1].text[1] = 0xe;
        labels[1].text[2] = 0xff;
    } else {
        labels[1].text[0] = 2;
        labels[1].text[1] = 0;
        labels[1].text[2] = 0xd;
        labels[1].text[3] = 2;
        labels[1].text[4] = 4;
        labels[1].text[5] = 0xb;
        labels[1].text[6] = 0xff;
    }
    while (result == MENU_NONE) {
        func_800279d8();
        pad = func_800279a4();
        if ((pad & 0x1000) || (pad & 0x4000)) {
            DAT_8009c268 = 0;
            func_8002792c(0xc);
            if (choice == 0) {
                choice = 1;
            } else {
                choice = 0;
            }
        } else if (pad & DAT_8009c24c) {
            func_8002792c(0xd);
            result = -choice;
        } else if (pad & DAT_8009c250) {
            func_8002792c(0xe);
            result = MENU_CANCEL;
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            func_80025be8(item);
            func_80025468(list, flags);
            func_80025b24(&labels[0], &labels[1], choice);
            func_800270f8();
        }
    }
    func_80027a40();
    return result;
}

ADDRESS(0x80024f88, 0x36c)
void func_80024f88(MenuList *list, s32 mode, u8 item)
{
    RECT rect;
    s32 result;
    s32 entry;
    s32 size;
    s32 saved;
    u8 *buffer;
    s32 i;

    result = MENU_NONE;
    func_8002792c(0xd);
    if (mode != 1) {
        func_80027e60();
    }
    rect.x = 0x3c0;
    rect.y = 0x100;
    rect.w = 0x40;
    rect.h = 0xfe;
    ClearImage(&rect, 0, 0, 0);
    if (mode == 0) {
        if (DAT_801b24e4.unk4c < DAT_80081780[item]) {
            entry = item + 540;
        } else {
            entry = item + 390;
        }
    } else if (mode == 1) {
        entry = item + 690;
    } else if (mode == 2) {
        entry = item + 390;
    } else {
        entry = item + 390;
        DAT_80081780[item] = 4;
    }
    saved = DAT_8009c230;
    size = func_80019cc4(6, entry, 0);
    DAT_8009c234 = (size + 2047) >> 11;
    buffer = func_800194f0(size);
    DAT_8009c380 = buffer;
    func_80027d88(6, entry);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    func_80043b38(buffer);
    DrawSync(0);
    func_80019518(buffer);
    DAT_8009c230 = saved;
    func_80027a40();
    while (result == MENU_NONE) {
        func_800279d8();
        if (func_800279a4() & DAT_8009c250) {
            func_8002792c(0xe);
            result = MENU_CANCEL;
        }
        for (i = 0; i < 2; i++) {
            func_80026fe4();
            if (list->unk0 != 0) {
                func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], (MenuText *)list);
                func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], (MenuText *)list);
            }
            if (mode != 1) {
                func_80025be8(item);
            }
            func_80026acc(0x1b, 0x70, 0x10a, 0x5f);
            func_80027414();
            DAT_8009c374->tpage = 0x1f;
            DAT_8009c374->clut = 0x7806;
            func_80027494(0x20, 0x76, 0xfe, 0x50, 0, 0, 0xfe, 0x50);
            func_80027530(0x1d);
            func_80027414();
            setRGB0(DAT_8009c374, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
            DAT_8009c374->tpage = 0x1f;
            DAT_8009c374->clut = 0x7806;
            func_80027494(0x21, 0x77, 0xfe, 0x50, 0, 0, 0xfe, 0x50);
            func_80027530(0x1e);
            func_800269c0(0x80, 0xd6, DAT_8009c258);
            func_80026570(0x8a, 0xd6, "return", 0, 0);
            func_800270f8();
        }
    }
    func_80027a40();
}

ADDRESS(0x800252f4, 0x174)
void func_800252f4(s32 page, s32 count, s32 selected, s32 flags)
{
    MenuText *title;
    MenuText *line;
    s32 i;

    title = &DAT_8007e660[page * 9];
    line = title + 1;
    if (title->x != 0) {
        func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], title);
        func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], title);
    }
    for (i = 0; i < count; i++) {
        if (i == selected) {
            func_80026020(&DAT_8007e564[MENU_FONT_FRAME], line, DAT_8009c260);
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], line);
        } else {
            func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], line);
            func_80026158(&DAT_8007e564[MENU_FONT_TEXT], line);
        }
        line++;
    }
    if (flags == 0) {
        func_800269c0(0x5f, 0xd6, DAT_8009c254);
    } else if (flags == 1) {
        func_800269c0(0x5b, 0xd6, 9);
    }
    func_80026570(0x69, 0xd6, "select", 0, 0);
    func_800269c0(0xa0, 0xd6, DAT_8009c258);
    func_80026570(0xaa, 0xd6, "return", 0, 0);
}

ADDRESS(0x80025468, 0x6bc)
void func_80025468(MenuList *list, s32 flags)
{
    MenuText text;
    u8 *names;
    u8 *second;
    u32 *values;
    u8 *counts;
    s32 hide;
    s32 stock;
    s32 i;
    s32 j;
    s32 bright;
    s32 blue;

    hide = flags & 0x8000;
    names = (u8 *)list->entries;
    second = (u8 *)list->unk28;
    values = list->values;
    counts = list->counts;
    flags &= 0xfff;
    if (list->unk0 != 0) {
        func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], (MenuText *)list);
        func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], (MenuText *)list);
    }
    names += list->first * list->unk23;
    second += list->first * 24;
    values += list->first;
    counts += list->first;
    stock = flags == 8 || flags == 9;
    for (i = 0; i < list->rows; i++) {
        if (i >= list->count) {
            break;
        }
        text.x = list->x + 13;
        text.y = list->y + 8 + i * 16;
        for (j = 0; j < list->unk23; j++) {
            text.text[j] = *names++;
        }
        func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &text);
        if (flags == 3) {
            text.x += 84;
            for (j = 0; j < 24; j++) {
                text.text[j] = *second++;
            }
            text.text[j] = 0xff;
            func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &text);
        }
        if (flags >= 10 && flags < 16 && flags != 12) {
            text.x += 161;
            func_800277c0(*values++, 6, 0, 2, text.text);
            func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
        }
        if (flags == 2 || flags == 4) {
            if (*values++ != -1) {
                text.x += 206;
                func_800277c0(values[-1], 3, 0, 3, text.text);
                func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
        if (stock) {
            if (*values++ != -1) {
                text.x += 115;
                func_800277c0(values[-1], 7, 0, 4, text.text);
                func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
            if (*counts++ != 0xff) {
                text.x += 91;
                func_800277c0(counts[-1], 3, 0, 5, text.text);
                func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
        if (!(flags == 2 || flags == 3 || flags == 4 || stock || flags == 16)) {
            if (*counts++ != 0xff) {
                text.x = list->x + 233;
                func_800277c0(counts[-1], 2, 0, 1, text.text);
                func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
        if (flags == 16) {
            if (*counts++ != 0xff) {
                text.x += 220;
                func_800277c0(counts[-1], 2, 0, 1, text.text);
                func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
            if (*values++ != -1) {
                text.x += 206;
                func_800277c0(values[-1], 3, 0, 3, text.text);
                func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
    }
    func_80026acc(list->x, list->y, 0x10a, list->rows * 16 + 12);
    for (i = 0; i < list->rows; i++) {
        if (i == list->row) {
            if (DAT_8009c260 < 48) {
                bright = DAT_8009c260 + DAT_8009c26c;
                blue = DAT_8009c260 / 2 + DAT_8009c26c;
            } else {
                bright = DAT_8009c26c + 95 - DAT_8009c260;
                blue = DAT_8009c26c + 47 - DAT_8009c260 / 2;
            }
            func_80027414();
            setRGB0(DAT_8009c374, bright, bright, blue);
            DAT_8009c374->tpage = DAT_8007e564[11].unk0;
            DAT_8009c374->clut = DAT_8007e564[11].unk2;
            func_80027494(list->x + 6, list->y + i * 16 + 5, DAT_8007e564[11].unk8, DAT_8007e564[11].unka,
                          DAT_8007e564[11].unk4, DAT_8007e564[11].unk6, DAT_8007e564[11].unk8,
                          DAT_8007e564[11].unka);
            SetSemiTrans(DAT_8009c374, 1);
            func_80027530(0x31);
        }
    }
    if (flags == 7) {
        func_80025db8(0);
    }
    if (flags == 10 || flags == 11 || flags == 14) {
        func_80025db8(1);
    }
    if (flags == 13) {
        func_80025e80();
    }
    if (flags == 3 || flags == 8 || flags == 9 || flags == 12 || hide != 0) {
        func_800269c0(0x5f, 0xd6, DAT_8009c254);
        func_80026570(0x69, 0xd6, "select", 0, 0);
        func_800269c0(0xa0, 0xd6, DAT_8009c258);
        func_80026570(0xaa, 0xd6, "return", 0, 0);
    } else {
        func_800269c0(0x42, 0xd6, DAT_8009c254);
        func_80026570(0x4c, 0xd6, "select", 0, 0);
        func_800269c0(0x83, 0xd6, 0);
        func_80026570(0x8d, 0xd6, "guide", 0, 0);
        func_800269c0(0xbc, 0xd6, DAT_8009c258);
        func_80026570(0xc6, 0xd6, "return", 0, 0);
    }
}

ADDRESS(0x80025b24, 0xc4)
void func_80025b24(MenuText *yes, MenuText *no, s32 choice)
{
    if (choice == 0) {
        func_80026020(&DAT_8007e564[4], yes, DAT_8009c260);
        func_80025f38(&DAT_8007e564[4], no);
        func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], yes);
        func_80026158(&DAT_8007e564[MENU_FONT_TEXT], no);
    } else {
        func_80025f38(&DAT_8007e564[4], yes);
        func_80026020(&DAT_8007e564[4], no, DAT_8009c260);
        func_80026158(&DAT_8007e564[MENU_FONT_TEXT], yes);
        func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], no);
    }
}

ADDRESS(0x80025be8, 0x1d0)
void func_80025be8(u8 item)
{
    MATRIX m;
    MATRIX light;
    MATRIX lights;
    MATRIX color;

    if (DAT_801b24e4.unkfb != 0 && item != 0xff && DAT_8009c230 != 1) {
        if (DAT_8009c23c != -1 && DAT_8009c23c != 0xff) {
            func_80027ac0();
            return;
        }
        if (DAT_8009c240 != 0) {
            CdControl(CdlPause, 0, 0);
            if (func_80019b68(DAT_8009c37c, DAT_8009c240) != 0) {
                DAT_8009c244 = -1;
            }
            DAT_8009c240 = 0;
        }
        if (DAT_8009c244 == -1) {
            if (DAT_8009c238 == 0) {
                func_80035c44(3, DAT_8009c37c);
                DAT_8009c238 = 1;
            }
            m.t[0] = DAT_8009c394.vx;
            m.t[1] = DAT_8009c394.vy;
            m.t[2] = DAT_8009c394.vz;
            DAT_8009c39c.vx &= 0xfff;
            DAT_8009c39c.vy += DAT_8009c3a4;
            DAT_8009c39c.vy &= 0xfff;
            DAT_8009c39c.vz &= 0xfff;
            RotMatrix(&DAT_8009c39c, &m);
            light.m[0][0] = 0x800;
            light.m[0][1] = 0x800;
            light.m[0][2] = -0x800;
            light.m[1][0] = 0;
            light.m[1][1] = 0;
            light.m[1][2] = -0x800;
            light.m[2][0] = 0x1000;
            light.m[2][1] = 0;
            light.m[2][2] = -0x800;
            MulMatrix0(&light, &m, &lights);
            SetLightMatrix(&lights);
            color.m[0][0] = 0xc00;
            color.m[0][1] = 0xc00;
            color.m[0][2] = 0xc00;
            color.m[1][0] = 0xc00;
            color.m[1][1] = 0xc00;
            color.m[1][2] = 0xc00;
            color.m[2][0] = 0xc00;
            color.m[2][1] = 0xc00;
            color.m[2][2] = 0xc00;
            SetColorMatrix(&color);
            SetRotMatrix(&m);
            SetTransMatrix(&m);
            func_8004290c();
        }
    }
}

ADDRESS(0x80025db8, 0xc8)
void func_80025db8(s32 big)
{
    MenuText label;
    MenuText number;

    if (big) {
        func_80025e80();
    }
    label.x = 0xb7;
    if (big == 0) {
        label.y = 0x2e;
    } else {
        label.y = 0x4a;
    }
    label.text[0] = 0x10;
    label.text[1] = 0x13;
    label.text[2] = 0x18;
    label.text[3] = 0xff;
    func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], &label);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 84;
    number.y = label.y;
    func_800277c0(DAT_8009c25c, 2, 0, 1, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
}

ADDRESS(0x80025e80, 0xb8)
void func_80025e80(void)
{
    MenuText label;
    MenuText number;

    label.x = 0xb7;
    label.y = 0x2e;
    label.text[0] = 6;
    label.text[1] = 0xe;
    label.text[2] = 0xb;
    label.text[3] = 3;
    label.text[4] = 0xff;
    func_80025f38(&DAT_8007e564[MENU_FONT_FRAME], &label);
    func_800261dc(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 49;
    number.y = label.y;
    func_800277c0(DAT_801b24e4.unk50, 7, 0, 2, number.text);
    func_8002636c(&DAT_8007e564[MENU_FONT_NUMBER], &number);
}

ADDRESS(0x80025f38, 0xe8)
void func_80025f38(MenuFont *frame, MenuText *text)
{
    func_80027414();
    setRGB0(DAT_8009c374, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
    DAT_8009c374->tpage = frame->unk0;
    DAT_8009c374->clut = frame->unk2;
    func_80027494(text->x - 6, text->y - 6, frame->unk8, frame->unka, frame->unk4, frame->unk6, frame->unk8,
                  frame->unka);
    SetSemiTrans(DAT_8009c374, DAT_801b24e4.unk100);
    func_80027530(0x32);
}

ADDRESS(0x80026020, 0x138)
void func_80026020(MenuFont *frame, MenuText *text, s32 shade)
{
    s32 bright;
    s32 blue;

    if (shade < 48) {
        bright = shade + DAT_8009c26c;
        blue = shade / 2 + DAT_8009c26c;
    } else {
        bright = 95 - shade + DAT_8009c26c;
        blue = 47 - shade / 2 + DAT_8009c26c;
    }
    func_80027414();
    setRGB0(DAT_8009c374, bright, bright, blue);
    DAT_8009c374->tpage = frame->unk0;
    DAT_8009c374->clut = frame->unk2;
    func_80027494(text->x - 6, text->y - 6, frame->unk8, frame->unka, frame->unk4, frame->unk6, frame->unk8,
                  frame->unka);
    SetSemiTrans(DAT_8009c374, DAT_801b24e4.unk100);
    func_80027530(0x32);
}

ADDRESS(0x80026158, 0x84)
void func_80026158(MenuFont *font, MenuText *text)
{
    s32 r;
    s32 g;
    s32 b;

    r = DAT_8009c270;
    g = DAT_8009c274;
    b = DAT_8009c278;
    if (DAT_801b24e4.unk100 == 0) {
        DAT_8009c270 = 0x47;
        DAT_8009c274 = 0x47;
        DAT_8009c278 = 0x57;
    } else {
        DAT_8009c270 = 0x57;
        DAT_8009c274 = 0x57;
    }
    func_800261dc(font, text);
    DAT_8009c270 = r;
    DAT_8009c274 = g;
    DAT_8009c278 = b;
}

ADDRESS(0x800261dc, 0x190)
void func_800261dc(MenuFont *font, MenuText *text)
{
    u8 *c;
    s32 dx;
    s32 u;
    s32 v;

    c = text->text;
    func_80027610(3, font->unk0);
    for (dx = 0; *c != 0xff; dx += 7) {
        func_80027560();
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx - 1;
        DAT_8009c378->y0 = text->y - 1;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        u = (*c & 0xf) * 8;
        v = (*c >> 4) * 15;
        DAT_8009c378->u0 = u;
        DAT_8009c378->v0 = v;
        func_800275e0(2);
        c++;
        func_80027560();
        setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx;
        DAT_8009c378->y0 = text->y;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        DAT_8009c378->u0 = u;
        DAT_8009c378->v0 = v;
        func_800275e0(3);
    }
}

ADDRESS(0x8002636c, 0x204)
void func_8002636c(MenuFont *font, MenuText *text)
{
    u8 *c;
    s32 dx;

    c = text->text;
    func_80027610(3, font->unk0);
    for (dx = 0; *c != 0xff; dx += 7) {
        func_80027560();
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx - 1;
        DAT_8009c378->y0 = text->y - 1;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        if (*c < 11) {
            DAT_8009c378->u0 = font->unk4;
            DAT_8009c378->v0 = *c * 15;
        } else {
            DAT_8009c378->u0 = font->unk4 + 7;
            DAT_8009c378->v0 = (*c - 11) * 15;
        }
        func_800275e0(2);
        func_80027560();
        setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx;
        DAT_8009c378->y0 = text->y;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        if (*c < 11) {
            DAT_8009c378->u0 = font->unk4;
            DAT_8009c378->v0 = *c * 15;
        } else {
            DAT_8009c378->u0 = font->unk4 + 7;
            DAT_8009c378->v0 = (*c - 11) * 15;
        }
        func_800275e0(3);
        c++;
    }
}

ADDRESS(0x80026570, 0x224)
void func_80026570(s32 x, s32 y, char *text, s32 shadow, s32 row)
{
    MenuFont *font;
    s32 shade;
    s32 v;

    font = &DAT_8007e564[2];
    v = row != 0 ? 16 : 0;
    func_80027610(6, font->unk0);
    shade = 0;
    if (DAT_801b24e4.unkfd > 128) {
        shade = (DAT_801b24e4.unkfd - 128) / 2;
    }
    shade += 112;
    while (*text != 0) {
        func_80027560();
        if (shadow == 0) {
            setRGB0(DAT_8009c378, shade, shade, shade);
        }
        DAT_8009c378->clut = font->unk2;
        if (shadow == 0) {
            DAT_8009c378->x0 = x;
            DAT_8009c378->y0 = y;
        } else {
            DAT_8009c378->x0 = x - 1;
            DAT_8009c378->y0 = y - 1;
        }
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        DAT_8009c378->u0 = font->unk4 + (*text - 'a') * font->unk8;
        DAT_8009c378->v0 = font->unk6 + v;
        func_800275e0(5);
        if (shadow != 0) {
            func_80027560();
            setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
            DAT_8009c378->clut = font->unk2;
            DAT_8009c378->x0 = x;
            DAT_8009c378->y0 = y;
            DAT_8009c378->w = font->unk8;
            DAT_8009c378->h = font->unka;
            DAT_8009c378->u0 = font->unk4 + (*text - 'a') * font->unk8;
            DAT_8009c378->v0 = font->unk6 + v;
            func_800275e0(6);
        }
        text++;
        x += 8;
    }
}

ADDRESS(0x80026794, 0x22c)
void func_80026794(MenuText *text, s32 shadow)
{
    MenuFont *font;
    u8 *c;
    s32 shade;
    s32 dx;

    font = &DAT_8007e564[2];
    func_80027610(6, font->unk0);
    shade = 0;
    if (DAT_801b24e4.unkfd > 128) {
        shade = (DAT_801b24e4.unkfd - 128) / 2;
    }
    c = text->text;
    for (dx = 0; *c != 0xff; dx += 7) {
        func_80027560();
        if (shadow == 0) {
            setRGB0(DAT_8009c378, shade + 112, shade + 112, shade + 112);
        }
        DAT_8009c378->clut = font->unk2;
        if (shadow == 0) {
            DAT_8009c378->x0 = text->x + dx;
            DAT_8009c378->y0 = text->y;
        } else {
            DAT_8009c378->x0 = text->x + dx - 1;
            DAT_8009c378->y0 = text->y - 1;
        }
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        DAT_8009c378->u0 = font->unk4 + *c * 8;
        DAT_8009c378->v0 = font->unk6 + 8;
        func_800275e0(5);
        if (shadow != 0) {
            func_80027560();
            setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
            DAT_8009c378->clut = font->unk2;
            DAT_8009c378->x0 = text->x + dx;
            DAT_8009c378->y0 = text->y;
            DAT_8009c378->w = font->unk8;
            DAT_8009c378->h = font->unka;
            DAT_8009c378->u0 = font->unk4 + *c * 8;
            DAT_8009c378->v0 = font->unk6 + 8;
            func_800275e0(6);
        }
        c++;
    }
}

ADDRESS(0x800269c0, 0x10c)
void func_800269c0(s32 x, s32 y, s32 value)
{
    s32 shade;

    func_80027610(6, DAT_8007e564[6].unk0);
    shade = 0;
    func_80027560();
    if (DAT_801b24e4.unkfd > 128) {
        shade = (DAT_801b24e4.unkfd - 128) / 2;
    }
    setRGB0(DAT_8009c378, shade + 112, shade + 112, shade + 112);
    DAT_8009c378->clut = DAT_8007e564[6].unk2;
    DAT_8009c378->x0 = x;
    DAT_8009c378->y0 = y;
    DAT_8009c378->w = DAT_8007e564[6].unk8;
    DAT_8009c378->h = DAT_8007e564[6].unka;
    DAT_8009c378->u0 = DAT_8007e564[6].unk4 + value * 16;
    DAT_8009c378->v0 = DAT_8007e564[6].unk6;
    func_800275e0(6);
}

ADDRESS(0x80026acc, 0x380)
void func_80026acc(s32 x0, s32 y0, s32 w, s32 h)
{
    MenuFont *f;
    s32 x;
    s32 y;
    s32 i;
    s32 k;
    s32 ww;
    s32 hh;
    s32 v;
    s32 wrem;
    s32 hrem;
    s32 done;
    s32 end;

    x = x0;
    y = y0;
    wrem = w - 66;
    hrem = h - 66;
    done = 0;
    for (i = 0, k = 9; i < 6; i++, k++) {
        if (i == 3) {
            k += 3;
            x = x0;
            if (hrem < 0) {
                y += 33;
            } else {
                y = y + 33 + hrem;
            }
            wrem = w - 66;
        }
        f = &DAT_8007e564[k + 3];
        if (i >= 3 && hrem < 0) {
            hh = h - 33;
            v = f->unk6 + 66 - h;
        } else {
            hh = f->unka;
            v = f->unk6;
        }
        ww = f->unk8;
        for (;;) {
            func_80027414();
            setRGB0(DAT_8009c374, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
            SetSemiTrans(DAT_8009c374, DAT_801b24e4.unk100);
            DAT_8009c374->tpage = f->unk0;
            DAT_8009c374->clut = f->unk2;
            func_80027494(x, y, ww, hh, f->unk4, v, ww, hh);
            x += ww;
            func_80027530(0x32);
            if (i == 1 || i == 4) {
                wrem -= ww;
                if (wrem == 0) {
                    break;
                }
                if (wrem < ww) {
                    ww = wrem;
                }
            } else {
                break;
            }
        }
    }
    y = y0 + 33;
    x = x0;
    i = 11;
    while (!done) {
        i++;
        f = &DAT_8007e564[i + 3];
        hh = f->unka;
        ww = f->unk8;
        end = 0;
        if (hrem < hh) {
            hh = hrem;
        }
        wrem = w - 66;
        do {
            func_80027414();
            setRGB0(DAT_8009c374, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
            SetSemiTrans(DAT_8009c374, DAT_801b24e4.unk100);
            DAT_8009c374->tpage = f->unk0;
            DAT_8009c374->clut = f->unk2;
            func_80027494(x, y, ww, hh, f->unk4, f->unk6, ww, hh);
            x += ww;
            func_80027530(0x32);
            if (i != 13) {
                break;
            }
            wrem -= ww;
            if (wrem == 0) {
                end = 1;
            } else if (wrem < ww) {
                ww = wrem;
            }
        } while (!end);
        if (i == 14) {
            i = 11;
            y += 28;
            x = x0;
            hrem -= hh;
            if (hrem == 0) {
                done = 1;
            }
        }
    }
}

ADDRESS(0x80026e4c, 0x198)
void func_80026e4c(s32 x, s32 y, s32 w, s32 h)
{
    func_80027414();
    setRGB0(DAT_8009c374, 0x40, 0x40, 0x40);
    SetSemiTrans(DAT_8009c374, DAT_801b24e4.unk100);
    DAT_8009c374->tpage = 0x1c;
    DAT_8009c374->clut = 0x7b05;
    func_80027494(x, y, w, h, 0, 0x30, 0x80, 0xc8);
    func_80027530(0x32);
    func_80027414();
    setRGB0(DAT_8009c374, 0x40, 0x40, 0x40);
    SetSemiTrans(DAT_8009c374, 1);
    DAT_8009c374->tpage = GetTPage(0, 1, 0x300, 0x100);
    DAT_8009c374->clut = 0x7b05;
    func_80027494(x, y, w, h, 0, 0x30, 0x80, 0xc8);
    func_80027530(0x31);
}

ADDRESS(0x80026fe4, 0x114)
void func_80026fe4(void)
{
    DAT_80199154.frame = DAT_80199154.frame == 0;
    DAT_80199154.pool = &DAT_80199154.pools[DAT_80199154.frame];
    DAT_80199154.ot = DAT_80199154.ot_buffer[DAT_80199154.frame];
    ClearOTagR(DAT_80199154.ot, OT_LENGTH);
    DAT_80199154.pool->next = DAT_80199154.pool->base;
    if (DAT_80199154.frame == 1) {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0xf0;
    } else {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0;
    }
    if (++DAT_8009c260 == 0x60) {
        DAT_8009c260 = 0;
    }
    if (DAT_8009c268 == 0) {
        DAT_8009c260 = 0x30;
        DAT_8009c268 = -1;
    }
    if (++DAT_8009c264 == 0x60) {
        DAT_8009c264 = 0;
    }
}

ADDRESS(0x800270f8, 0xa0)
void func_800270f8(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&DAT_80199154.disp[DAT_80199154.frame]);
    PutDrawEnv(&DAT_80199154.draw[DAT_80199154.frame]);
    LoadImage(&DAT_8009c38c, (u_long *)DAT_8009c388);
    DrawOTag(DAT_80199154.ot + OT_LENGTH - 1);
}

ADDRESS(0x80027198, 0x178)
void func_80027198(void)
{
    RECT rect;

    func_800438e0();
    DAT_80199154.draw[0].isbg = 0;
    DAT_80199154.draw[0].dfe = 0;
    DAT_80199154.draw[1].isbg = 0;
    DAT_80199154.draw[1].dfe = 0;
    DAT_8009c404 = DAT_80199154.pools[0];
    DAT_8009c410 = DAT_80199154.pools[1];
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x7400;
    DAT_8009c388 = DAT_80199154.pools[1].end = DAT_80199154.pools[1].base + 0x7400;
    if (DAT_80199154.frame == 1) {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0xf0;
    } else {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0;
    }
    DAT_8009c38c.w = 0x140;
    DAT_8009c38c.h = 0xf0;
    StoreImage(&DAT_8009c38c, (u_long *)DAT_8009c388);
    DrawSync(0);
    DAT_8009c384 = DAT_801b24e4.bgm_volume;
    func_8001f274();
    if (DAT_801b24e4.unkfe < 2) {
        DAT_8009c24c = 0x20;
        DAT_8009c250 = 0x40;
        DAT_8009c254 = 1;
        DAT_8009c258 = 3;
    } else {
        DAT_8009c24c = 0x40;
        DAT_8009c250 = 0x20;
        DAT_8009c254 = 3;
        DAT_8009c258 = 1;
    }
}

ADDRESS(0x80027310, 0x104)
void func_80027310(s32 mode)
{
    DAT_80199154.pools[0] = DAT_8009c404;
    DAT_80199154.pools[1] = DAT_8009c410;
    DAT_80199154.draw[0].isbg = 1;
    DAT_80199154.draw[0].dfe = 1;
    DAT_80199154.draw[1].isbg = 1;
    DAT_80199154.draw[1].dfe = 1;
    if (mode == 1) {
        func_800152f8();
    } else if ((DAT_801b24e4.bgm_volume != 0 && DAT_8009c384 == 0) ||
               (DAT_801b24e4.bgm_volume == 0 && DAT_8009c384 != 0)) {
        if (DAT_801b24e4.bgm_volume == 0) {
            func_800152f8();
        } else {
            func_80015240();
        }
    }
}

ADDRESS(0x80027414, 0x80)
void func_80027414(void)
{
    DAT_8009c374 = (POLY_FT4 *)DAT_80199154.pool->next;
    SetPolyFT4(DAT_8009c374);
    setRGB0(DAT_8009c374, DAT_8009c270, DAT_8009c274, DAT_8009c278);
    DAT_80199154.pool->next += sizeof(POLY_FT4);
}

ADDRESS(0x80027494, 0x9c)
void func_80027494(s16 x, s16 y, s16 w, s16 h, s16 u, s16 v, s16 uw, s16 vh)
{
    setXY4(DAT_8009c374, x, y, x + w, y, x, y + h, x + w, y + h);
    setUV4(DAT_8009c374, u, v, u + uw, v, u, v + vh, u + uw, v + vh);
}

ADDRESS(0x80027530, 0x30)
void func_80027530(s32 depth)
{
    AddPrim(DAT_80199154.ot + depth, DAT_8009c374);
}

ADDRESS(0x80027560, 0x80)
void func_80027560(void)
{
    DAT_8009c378 = (SPRT *)DAT_80199154.pool->next;
    SetSprt(DAT_8009c378);
    setRGB0(DAT_8009c378, DAT_8009c270, DAT_8009c274, DAT_8009c278);
    DAT_80199154.pool->next += sizeof(SPRT);
}

ADDRESS(0x800275e0, 0x30)
void func_800275e0(s32 depth)
{
    AddPrim(DAT_80199154.ot + depth, DAT_8009c378);
}

ADDRESS(0x80027610, 0x78)
void func_80027610(s32 depth, s16 tpage)
{
    DR_MODE *mode;

    mode = (DR_MODE *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(DR_MODE);
    SetDrawMode(mode, 0, 0, tpage, 0);
    AddPrim(DAT_80199154.ot + depth + 1, mode);
}

ADDRESS(0x80027688, 0xa0)
void func_80027688(MenuList *list, s32 page, s32 button)
{
    u8 *src;
    u8 *dst;
    s32 i;

    dst = list->title;
    src = DAT_8007e660[page * 9 + button + 1].text;
    list->unk0 = 0x1f;
    list->unk2 = 0x20;
    for (i = 0; i < 16; i++) {
        *dst++ = *src++;
    }
    list->x = 0x1b;
    list->y = 0x93;
    list->count = 0;
    list->rows = 3;
    list->first = 0;
    list->cursor = 0;
    list->row = 0;
    list->unk23 = 0x18;
    DAT_8009c25c = 1;
}

ADDRESS(0x80027728, 0x98)
void func_80027728(MenuList *list, u8 cursor)
{
    if (list->count != 0) {
        if (cursor > list->count - 1) {
            cursor = list->count - 1;
        }
        list->cursor = cursor;
        if (list->count - list->cursor < list->rows) {
            if (list->count < list->rows) {
                list->first = 0;
                list->row = list->cursor;
            } else {
                list->first = list->count - list->rows;
                list->row = list->cursor - list->first;
            }
        } else {
            list->first = cursor;
            list->row = 0;
        }
    }
}

ADDRESS(0x800277c0, 0x16c)
void func_800277c0(s32 value, s32 digits, s32 arg2, s32 arg3, u8 *out)
{
    s32 i;
    s32 fill;

    i = 0;
    if (arg3 == 1 || arg3 == 2) {
        digits += 1;
    } else if (arg3 == 3 || arg3 == 5) {
        digits += 2;
    } else if (arg3 == 4) {
        digits += 3;
    }
    fill = arg2 == 0 ? 10 : 0;
    for (i = 0; i < digits; i++) {
        out[i] = fill;
    }
    out[digits] = 0xff;
    if (arg3 == 1) {
        out[0] = 0x13;
    } else if (arg3 == 2) {
        out[digits - 1] = 0xd;
        digits--;
    } else if (arg3 == 3) {
        out[0] = 0xf;
        out[1] = 0x10;
    } else if (arg3 == 4) {
        out[0] = 0xc;
        out[1] = 0x12;
        out[2] = 0x10;
    } else if (arg3 == 5) {
        out[0] = 0xe;
        out[1] = 0x11;
    }
    for (i = digits - 1; i >= 0; i--) {
        out[i] = value % 10;
        value /= 10;
        if (value == 0) {
            i = -1;
        }
    }
}

ADDRESS(0x8002792c, 0x78)
void func_8002792c(s32 se)
{
    if (se == 12 || se == 13 || se == 14) {
        func_80015790(se, 0x38, 0x38, 0);
    } else if (se == 9) {
        func_80015790(9, 0, 0x48, 0);
        VSync(0);
        VSync(0);
        func_80015790(9, 0x48, 0, 0);
    }
}

ADDRESS(0x800279a4, 0x34)
s32 func_800279a4(void)
{
    u16 pad;

    pad = PadRead(1);
    if (pad != 0) {
        DAT_8009c248 = 1;
    }
    return pad;
}

ADDRESS(0x800279d8, 0x68)
void func_800279d8(void)
{
    s32 i;

    i = 0;
    if (DAT_8009c248 == 1) {
        DAT_8009c248 = 0;
        while ((u16)PadRead(1) != 0) {
            if (i++ < 8) {
                VSync(0);
            } else {
                DAT_8009c260 = 0;
                break;
            }
        }
    }
}

ADDRESS(0x80027a40, 0x30)
void func_80027a40(void)
{
    DAT_8009c248 = 0;
    while ((u16)PadRead(1) != 0) {
    }
}

ADDRESS(0x80027a70, 0x2c)
void func_80027a70(void)
{
    while ((u16)PadRead(1) == 0) {
    }
}

ADDRESS(0x80027a9c, 0x24)
void func_80027a9c(u8 item)
{
    DAT_8009c25c = 1;
    if (DAT_801b24e4.unkfb != 0) {
        DAT_8009c23c = item;
    }
}

ADDRESS(0x80027ac0, 0x9c)
void func_80027ac0(void)
{
    u32 size;

    func_80027b5c();
    size = func_80019cc4(6, DAT_8009c23c, 0);
    DAT_8009c234 = (size + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = func_800194f0(size);
    DAT_8009c240 = func_80027d88(6, DAT_8009c23c);
    DAT_8009c230 = 1;
    DAT_8009c244 = DAT_8009c23c;
    DAT_8009c23c = -1;
    DAT_8009c394.vz = 1000;
    DAT_8009c394.vx = 0;
    DAT_8009c394.vy = 0;
    DAT_8009c39c.vx = 0;
    DAT_8009c39c.vy = 0;
    DAT_8009c39c.vz = 0;
    DAT_8009c3a4 = 4;
}

ADDRESS(0x80027b5c, 0x58)
void func_80027b5c(void)
{
    if (DAT_8009c238 == 1) {
        func_80019518(DAT_80199154.unk10148[3]);
    } else if (DAT_8009c230 != 0) {
        func_80019518(DAT_8009c37c);
    }
    DAT_8009c230 = 0;
    DAT_8009c238 = 0;
}

ADDRESS(0x80027bb4, 0xd8)
void func_80027bb4(void)
{
    func_80019ab4();
    EnterCriticalSection();
    DisableEvent(DAT_801c12e4.error_event);
    DisableEvent(DAT_801c12e4.complete_event);
    DisableEvent(DAT_801c12e4.ready_event);
    CloseEvent(DAT_801c12e4.ready_event);
    DAT_801c12e4.ready_event = OpenEvent(HwCdRom, EvSpDR, EvMdINTR, func_80027ea8);
    EnableEvent(DAT_801c12e4.ready_event);
    EnableEvent(DAT_801c12e4.error_event);
    CdReadyCallback(func_80027ea8);
    ExitCriticalSection();
    VSync(4);
    VSync(4);
}

ADDRESS(0x80027c8c, 0xfc)
void func_80027c8c(void)
{
    func_80027e60();
    func_80027b5c();
    DAT_8009c23c = -1;
    EnterCriticalSection();
    DisableEvent(DAT_801c12e4.error_event);
    DisableEvent(DAT_801c12e4.ready_event);
    CloseEvent(DAT_801c12e4.ready_event);
    DAT_801c12e4.ready_event = OpenEvent(HwCdRom, EvSpDR, EvMdINTR, func_80019848);
    EnableEvent(DAT_801c12e4.error_event);
    EnableEvent(DAT_801c12e4.complete_event);
    EnableEvent(DAT_801c12e4.ready_event);
    CdReadyCallback(func_80019848);
    ExitCriticalSection();
    CdControl(CdlPause, 0, 0);
    VSync(4);
    VSync(4);
}

ADDRESS(0x80027d88, 0xd8)
s32 func_80027d88(u16 file, u16 entry)
{
    CdlLOC pos;
    u8 result[8];
    u8 param[8];
    CdFile *f;
    s32 size;

    f = &DAT_801c12e4.files[file];
    size = f->sectors[entry + 1] - f->sectors[entry];
    func_80019a74(&f->extent.pos, f->sectors[entry], &pos);
    CdControl(CdlPause, 0, 0);
    param[0] = CdlModeSpeed;
    do {
        CdControl(CdlSetmode, param, 0);
    } while (CdSync(0, result) != CdlComplete);
    do {
        CdControl(CdlReadN, (u8 *)&pos, 0);
    } while (CdSync(0, result) != CdlComplete);
    return size;
}

ADDRESS(0x80027e60, 0x48)
void func_80027e60(void)
{
    if (DAT_8009c230 == 1) {
        do {
            VSync(2);
        } while (DAT_8009c230 == 1);
    }
}

ADDRESS(0x80027ea8, 0x84)
void func_80027ea8(u8 status, u8 *data)
{
    u8 result[8];

    if (DAT_8009c230 == 1 && DAT_8009c234 > 0) {
        if (CdReady(1, result) == CdlDiskError) {
            DAT_8009c234 = 0;
            DAT_8009c230 = 3;
        } else {
            CdGetSector(DAT_8009c380, 0x200);
            DAT_8009c380 += 2048;
            if (--DAT_8009c234 == 0) {
                DAT_8009c230 = 2;
            }
        }
    }
}
