#ifndef GAME_MENU_H
#define GAME_MENU_H

/*
 * GAME.EXE in-game menus (src/game/menu.c, WIP names): the pause screen,
 * the menu's button page and the pages it opens. The helpers the pages call
 * (frame begin/present, text, windows, list input) lie past band E and are
 * declared from their call sites only.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/kernel.h>

/* Results of the menu pages (anything else is an item or action id). */
#define MENU_NONE (-99)
#define MENU_CANCEL (-1)

/*
 * The menu's glyph sets and window frame (12-byte records from 0x8007e564):
 * [0] digits, [1] text, [5] the frame func_80025f38 draws.
 */
typedef struct MenuFont {
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
    s16 unk8;
    s16 unka;
} MenuFont;

#define MENU_FONT_NUMBER 0
#define MENU_FONT_TEXT 1
#define MENU_FONT_FRAME 5

/* A line of menu text: screen position and glyphs (0xff ends the line). */
typedef struct MenuText {
    s16 x;
    s16 y;
    u8 text[24];
} MenuText;

/* A map room: its rectangle on the area map (u8 x, y, w, h). */
typedef struct MapRect {
    u8 x;
    u8 y;
    u8 w;
    u8 h;
} MapRect;

/* One row of a menu list: the item name record copied out of DAT_8007f620. */
typedef struct MenuEntry {
    u8 text[24];
} MenuEntry;

/* A menu list window (func_80027688 sets it up from a button page title). */
typedef struct MenuList {
    s16 unk0;
    s16 unk2;
    u8 title[16];   /* +0x04 */
    u8 unk14[8];
    u8 x;           /* +0x1c */
    u8 y;
    u8 count;       /* +0x1e: rows in the list */
    u8 rows;        /* +0x1f: rows shown */
    u8 first;       /* +0x20: first row shown */
    u8 cursor;      /* +0x21: highlighted row */
    u8 row;         /* +0x22: highlighted row on screen */
    u8 unk23;
    MenuEntry *entries; /* +0x24 */
    MenuEntry *unk28; /* +0x28: a second column of names */
    u8 *counts;     /* +0x2c */
    u32 *values;    /* +0x30 */
    u8 unk34[4];
} MenuList;

/* menu.c */
void func_8001a634(void);
s32 func_8001a774(void);
void func_8001a9dc(void);
s32 func_8001aa84(void);
s32 func_8001aea4(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last);
s32 func_8001af88(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last);
s32 func_8001b150(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last);
void func_8001b254(s32 item);
void func_8001b4f8(void);
void func_8001b564(void);
void func_8001b594(void);
void func_8001b5c4(u8 item);
void func_8001b88c(s32 item);
void func_8001babc(void);
void func_8001bd48(void);
void func_8001bea8(void);
s32 func_8001c030(void);

/* helpers past band E (call-site signatures) */
s32 func_8001c48c(void);
s32 func_8001c6c0(Record18 *spells, MenuEntry *entries, u32 *values, u8 *ids, s32 first, s32 last);
void func_8001c7a8(void);
void func_8001c9c0(MenuEntry *out);
void func_8001cbb8(s32 slot);
void func_8001d0c4(void);
void func_8001d3a0(void);
void func_8001d784(void);
void func_8001d8a4(void);
void func_8001d944(void);
s32 func_8001dbc4(s32 index, s32 book, u8 *out);
void func_8001dcd0(s32 book, s32 page);
void func_8001dd94(void);
void func_8001dee4(void);
void func_8001e1b4(void);
s32 func_8001e484(void);
s32 func_8001e710(void);
s32 func_8001ea34(struct DIRENTRY *dir, u8 *text, u32 *experience, u8 *levels, s32 *numbers);
void func_8001eb3c(s32 message, MenuText *lines, s32 count, s32 x, s32 y, s32 w, s32 h, s32 back);
void func_8001ec84(void);
void func_8001f004(void);
void func_8001f274(void);
void func_8001f31c(void);
void func_8001f4c0(void);
s32 func_8001fc5c(struct DIRENTRY *dir);
void func_8001fe9c(MenuText *lines);
void func_8001ff94(MenuText *lines);
s32 func_8001f8b4(void);
void func_8002090c(MenuText *line);
void func_8002098c(MenuText *lines);
void func_80020a84(MenuText *lines);
void func_80020b7c(MenuText *lines);
void func_80020c74(MenuText *lines);
void func_80020d6c(MenuText *line);
void func_80020dec(MenuText *line);
void func_80020e6c(MenuText *lines, s32 error);
void func_8002008c(void);
void func_800203c8(s32 number);
s32 func_80020560(void);
void func_80020fe8(MenuText *lines, s32 count, s32 x, s32 y, s32 w, s32 h, s32 back);
void func_80021114(s32 shop);
void func_80021298(s32 shop);
void func_800216a4(u8 *bag, u8 *counts, u32 *prices, u8 *ids, s32 first, s32 end, s32 shop);
void func_80021724(s32 shop);
void func_80021a4c(u32 *prices, u8 *ids, s32 first, s32 end, s32 shop);
s32 func_80021aac(void);
s32 func_80021d94(void);
s32 func_8002200c(s32 mode);
void func_80025e80(void);
s32 func_800221e8(s32 selected, s32 last, s32 *chosen, s32 *confirmed, s32 *result);
void func_800227ec(void);
void func_800252f4(s32 page, s32 count, s32 selected, s32 flags);
void func_80025f38(MenuFont *frame, MenuText *text);
void func_800261dc(MenuFont *font, MenuText *text);
void func_8002636c(MenuFont *font, MenuText *text);
void func_80026acc(s32 x, s32 y, s32 w, s32 h);
void func_80026570(s32 x, s32 y, char *text, s32 a3, s32 a4);
void func_80026fe4(void);
void func_800270f8(void);
void func_80027198(void);
void func_80027310(s32 a0);
void func_8002792c(s32 se);
s32 func_800279a4(void);
void func_80027a40(void);
void func_8002deec(s32 a0);
void func_80023300(void);
void func_80024a78(u8 *levels);
void func_800246bc(u8 *options);
void func_80023fd4(u8 style, u8 layout, s32 row);
void func_80025b24(MenuText *yes, MenuText *no, s32 choice);
void func_80023ce8(s32 page);
s32 func_80024c70(MenuList *list, s32 kind, s32 flags, u8 item);
void func_80024f88(MenuList *list, s32 a1, u8 item);
u32 func_800222fc(MenuList *list, u8 *ids, s32 *confirmed, s32 *result, s32 a4);
void func_80025be8(u8 item);
void func_80025468(MenuList *list, s32 flags);
void func_80027688(MenuList *list, s32 page, s32 button);
void func_80027728(MenuList *list, u8 cursor);
void func_80027a9c(u8 item);
void func_80027b5c(void);
void func_80027bb4(void);
void func_80027c8c(void);
void func_80027e60(void);
void func_80027414(void);
void func_80027494(s16 x, s16 y, s16 w, s16 h, s16 u, s16 v, s16 uw, s16 vh);
void func_80027530(s32 depth);
void func_800269c0(s32 x, s32 y, s32 value);
void func_80026e4c(s32 x, s32 y, s32 w, s32 h);
void func_80025db8(s32 big);
void func_80026794(MenuText *text, s32 shadow);
void func_80027a70(void);
void func_800279d8(void);
s32 func_80027d88(u16 file, u16 entry);
void func_80027ea8(u8 status, u8 *data);
void func_80027560(void);
void func_800275e0(s32 depth);
void func_80027610(s32 depth, s16 tpage);
void func_80027ac0(void);
/* model code past the menu (call-site signatures) */
void func_8004290c(void);
void func_80026020(MenuFont *font, MenuText *text, s32 shade);
void func_80026158(MenuFont *font, MenuText *text);

extern MenuFont DAT_8007e564[];
/* The menu's text records (MenuText, 0x1c bytes): button page titles from [0],
 * yes/no positions [10] and [11], message titles from [28]. */
extern MenuText DAT_8007e660[];
extern MenuEntry DAT_8007f620[];
extern MenuEntry DAT_8007f230[10];
extern MenuEntry DAT_80080430[];
extern u8 DAT_80081780[];
/* Shop stock counts per shop and item id, buy prices and sell prices. */
extern u8 DAT_80080718[6][150];
extern u16 DAT_80080a9c[6][150];
extern u16 DAT_800811a4[5][150];
extern MapRect DAT_801ba6fc[160];

/* The two frame packet pools (DAT_80199154.pools) as the menu saved them on
 * opening (func_80027198); func_80027310 puts them back. */
extern PrimPool DAT_8009c404;
extern PrimPool DAT_8009c410;

/* menu .sdata */
extern u8 DAT_8009c22c;
extern s32 DAT_8009c230;
extern s32 DAT_8009c234;
extern s32 DAT_8009c24c;
extern s32 DAT_8009c250;
extern s32 DAT_8009c254;
extern s32 DAT_8009c248;
extern s32 DAT_8009c268;
extern s32 DAT_8009c26c;
extern s32 DAT_8009c270;
extern s32 DAT_8009c274;
extern s32 DAT_8009c278;
extern s32 DAT_8009c27c;
extern s32 DAT_8009c258;
extern s32 DAT_8009c25c;
extern s32 DAT_8009c260;
extern s32 DAT_8009c238;
extern s32 DAT_8009c23c;
extern s32 DAT_8009c240;
extern s32 DAT_8009c244;
extern s32 DAT_8009c264;
extern s32 DAT_8009c274;
extern s32 DAT_8009c278;

/* The menu .sbss words (0x8009c374..0x8009c3a7) are statics of menu.c. */

#endif
