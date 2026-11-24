/*
 * GAME.EXE: the menu's magic and equipment pages (WIP module, a slice of
 * the menu translation unit after the area warp picker). The menu's $gp
 * globals are defined in game.menu; this slice repeats them as tentative
 * definitions because GCC 2.6 reaches only a translation unit's own small
 * globals through $gp (an extern is reached absolutely).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/libc.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>
#include <LIBCD.H>
#include <LIBSND.H>
#include <game/sound.h>
#include <game/data.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/menu.h>
#include <game/funcs.h>

RODATA(0x80011460, 0x84)

/* PSX.EXE's mailbox byte beside its next-program byte (see src/game/main.c). */
#define MAILBOX_BYTE_8 ((u8 *)0x800102f8)
#define MAILBOX_BYTE_A ((u8 *)0x800102fa)

u8 DAT_8009c22d;
u8 DAT_8009c22e;
u8 DAT_8009c22f;
s32 DAT_8009c250;
s32 DAT_8009c25c;
s32 DAT_8009c258;
u8 DAT_8009c384;
s32 DAT_8009c24c;
s32 DAT_8009c254;
s32 DAT_8009c26c;
s32 DAT_8009c27c;
s32 DAT_8009c248;
s32 DAT_8009c268;
s32 DAT_8009c270;
s32 DAT_8009c274;
s32 DAT_8009c278;

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
    slots[2] = DAT_801b24e4.unkf0[2];
    slots[3] = DAT_801b24e4.unkf0[0];
    slots[4] = DAT_801b24e4.unkf0[1];
    slots[5] = DAT_801b24e4.unkf0[3];
    slots[6] = DAT_801b24e4.unkf0[4];
    slots[7] = DAT_801b24e4.unkf0[5];
    slots[8] = DAT_801b24e4.unkf0[6];
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
        equipped = DAT_801b24e4.unkf0[2];
        last = 0x3e;
        break;
    case 3:
        first = 0x22;
        equipped = DAT_801b24e4.unkf0[0];
        last = 0x29;
        break;
    case 4:
        first = 0x2a;
        equipped = DAT_801b24e4.unkf0[1];
        last = 0x32;
        break;
    case 5:
        first = 0x3f;
        equipped = DAT_801b24e4.unkf0[3];
        last = 0x47;
        break;
    case 6:
        first = 0x48;
        equipped = DAT_801b24e4.unkf0[4];
        last = 0x50;
        break;
    case 7:
    case 8:
        if (slot == 7) {
            equipped = DAT_801b24e4.unkf0[5];
        } else {
            equipped = DAT_801b24e4.unkf0[6];
        }
        if (slot == 7) {
            other = DAT_801b24e4.unkf0[6];
        } else {
            other = DAT_801b24e4.unkf0[5];
        }
        first = 0x51;
        if (other != 0xff) {
            DAT_800c85e8[other]--;
        }
        last = 0x5e;
        break;
    }
    n = func_8001aea4(DAT_800c85e8, entries, counts, ids, first, last);
    memcpy(&entries[n], &DAT_8007f230[11], 10);
    counts[n] = 0xff;
    ids[n] = 0xff;
    n++;
    func_80027688(&list, 0, 2);
    list.count = n;
    n--;
    list.entries = entries;
    list.counts = counts;
    for (i = 0; i < n; i++) {
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
        DAT_800c85e8[other]++;
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
    n--;
    list.entries = entries;
    list.values = values;
    list.y = 0x83;
    for (i = 0; i < n; i++) {
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

    confirmed = 0;
    result = MENU_NONE;
    for (; i < 88; i++) {
        counts[i] = 0xff;
        values[i] = -1;
        ids[i] = 0xff;
        spells[i] = 0xff;
    }
    saved = DAT_800c85e8[0x85];
    DAT_800c85e8[0x85] = 0;
    i = func_8001aea4(DAT_800c85e8, entries, counts, ids, 0x65, 0x92);
    DAT_800c85e8[0x85] = saved;
    i += func_8001c6c0(DAT_801b77ec.unk0, &entries[i], &values[i], &spells[i], 0, 0x1e);
    memcpy(&entries[i], &DAT_8007f230[11], 12);
    i++;
    func_80027688(&list, 0, 2);
    list.count = i;
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
    n = func_8001af88(DAT_800c85e8, entries, counts, ids, 0, 0x95);
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
            if (counts[list.cursor] < DAT_8009c25c || DAT_800c85e8[item + 150] + DAT_8009c25c >= 100) {
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
        DAT_800c85e8[result] -= DAT_8009c25c;
        DAT_800c85e8[result + 150] += DAT_8009c25c;
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
    n = func_8001aea4(&DAT_800c85e8[150], entries, counts, ids, 0, 0x95);
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
            if (counts[list.cursor] < DAT_8009c25c || DAT_800c85e8[item] + DAT_8009c25c >= 100) {
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
        DAT_800c85e8[result] += DAT_8009c25c;
        DAT_800c85e8[result + 150] -= DAT_8009c25c;
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
    char dir[600];
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
s32 func_8001ea34(char *dir, u8 *p, u32 *experience, u8 *levels, s32 *numbers)
{
    s32 n = 0;
    s32 i = 0;
    s32 points;
    s32 level;
    s32 number;

    for (; i < 15; i++) {
        if (func_80028398(dir, &points, &level, &number) == 0) {
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
        dir += 40;
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
    char dir[600];
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
s32 func_8001fc5c(char *dir)
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
    char dir[600];
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
