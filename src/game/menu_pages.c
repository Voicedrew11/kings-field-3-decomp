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
#include <game/data.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/menu.h>

RODATA(0x80011460, 0x50)

u8 DAT_8009c22d;
u8 DAT_8009c22e;
u8 DAT_8009c22f;
s32 DAT_8009c250;
s32 DAT_8009c25c;

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
