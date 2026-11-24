#ifndef GAME_DATA_H
#define GAME_DATA_H

/*
 * GAME.EXE globals band A references outside the sound and CD headers (WIP),
 * mostly the BSS blocks the game loop clears word by word at start-up. Each
 * extent is the word count main passes to func_80018fbc; whether a block is
 * one object or a run of several is not yet known, so the names stay
 * address-derived and only the fields band A touches are named.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

typedef struct Data801b24e4 {
    s32 experience; /* +0x00: the save title shows it as six digits */
    s32 unk4;
    s32 unk8;
    u8 level;       /* +0x0c: the save title shows it as two digits */
    u8 unkd;
    u8 unke;        /* +0x0e: cleared every frame by the game loop */
    u8 unkf;
    u8 unk10;
    u8 unk11;
    s16 unk12;
    s16 unk14;
    u16 unk16;      /* +0x16: 0..999, from the level table */
    u16 unk18;      /* +0x18: 0 is death (func_8002a6a0) */
    u16 unk1a;      /* +0x1a: 0..999, from the level table */
    u16 unk1c;
    s16 unk1e;
    s16 unk20;
    s16 unk22;
    u16 unk24;      /* +0x24: counts up to 100 for a point of unk32 */
    u16 unk26[5];   /* +0x26: count up to 400 for a point of unk34[n] */
    s16 unk30;
    u16 unk32;      /* +0x32: func_80029500 copies it to unk40 */
    u16 unk34[5];   /* +0x34: func_80029500 copies them to unk42 */
    s16 unk3e;
    u16 unk40;      /* +0x40: 0..999 */
    u16 unk42[5];   /* +0x42: 0..999 each */
    s16 unk4c;
    u8 unk4e[0x2];
    s32 unk50;
    u16 unk54[7];   /* +0x54: func_80029500 rebuilds them; never above 0x7fff */
    u16 unk62;
    u8 unk64[0x2];
    u16 unk66;      /* +0x66..+0x76: func_80029500 rebuilds them from Record20s */
    u16 unk68;
    u16 unk6a;
    u16 unk6c;
    u16 unk6e;
    u16 unk70;
    u16 unk72;
    u16 unk74;
    u16 unk76;
    s16 unk78;
    s16 unk7a;
    s16 unk7c;
    s16 unk7e;
    s16 unk80;
    s16 unk82;
    s16 unk84;
    s16 unk86;
    s16 unk88;
    s16 unk8a;
    s16 unk8c;
    s16 unk8e;
    s16 unk90;
    s16 unk92;
    s16 unk94;
    u8 unk96[0x6];
    s32 unk9c;
    s32 unka0;
    u32 play_minutes; /* +0xa4: the VSync handler adds one every 3600 VSyncs */
    s32 unka8;
    s32 unkac;
    s32 unkb0;
    s32 unkb4;
    s32 unkb8;
    s32 unkbc;
    s16 unkc0;
    s16 unkc2;
    s16 unkc4;
    u8 unkc6;
    u8 unkc7;
    u8 unkc8;
    u8 unkc9;
    u8 unkca;
    u8 unkcb;
    u8 unkcc;
    u8 unkcd[0x1];
    u8 unkce;
    u8 unkcf;
    u8 unkd0;
    u8 unkd1[0x3];
    s32 unkd4;
    s32 unkd8;
    s32 unkdc;
    s32 unke0;
    s32 unke4;
    s32 unke8;
    s32 unkec;
    u8 unkf0[7];    /* +0xf0: equipped item ids (func_8001af88); [5] and [6] may match */
    u8 se_volume;   /* +0xf7: 0 mutes sound effects; scales them by n/128 */
    u8 bgm_volume;  /* +0xf8: 0 keeps the sequence off */
    u8 unkf9;
    u8 unkfa;
    u8 unkfb;
    u8 unkfc;
    u8 unkfd;
    u8 unkfe;
    u8 unkff;
    u8 unk100;
    u8 unk101;
    u8 unk102[0x2];
    u8 unk104;
    u8 unk105;
    u8 unk106;
    u8 unk107;
    u8 unk108;
    u8 unk109;
    u8 unk10a;
    u8 unk10b[0x1];
    s32 x;          /* +0x10c: func_80017aa4 moves it with the objects */
    s32 y;
    s32 z;
    s32 unk118;
    s16 unk11c;
    s16 unk11e;
    s16 unk120;
    u8 unk122[0x2];
    SVECTOR unk124; /* +0x124 */
    SVECTOR unk12c; /* +0x12c */
    s16 unk134;
    s16 unk136;
    s16 unk138;
    s16 unk13a;
    s16 unk13c;
    s16 unk13e;
    s16 unk140;
    u8 unk142[0x2];
    s16 unk144;
    s16 unk146;
    s16 unk148;
    u8 unk14a[0x2];
    s16 unk14c;
    s16 unk14e;
    s16 unk150;
    u8 unk152[0x2];
    s32 unk154;
    s32 unk158;
    s32 unk15c;
    s16 unk160;
    s16 unk162;
    s16 unk164;
    s16 unk166;
    s16 unk168;
    s16 unk16a;
    s16 unk16c;
    s16 unk16e;
    s16 unk170;
    s16 unk172;
    s16 unk174;
    s16 unk176;
    s16 unk178;
    s16 unk17a;
    u8 unk17c[0x4];
    s32 unk180;
    s32 unk184;
    s16 unk188;
    s16 unk18a;
    s16 unk18c;
    s16 unk18e;
    s16 unk190;
    s16 unk192;
    s16 unk194;
    u8 unk196[0x6];
} Data801b24e4;

typedef struct Data801ba984 {
    u8 unk0[0xd];
    u8 unkd;        /* +0x0d: the game loop sets it to 1 on a continued game */
    u8 unke[0x5136];
} Data801ba984;

typedef struct Data801d4464 {
    u8 unk0[0x1200a];
    u16 unk1200a;   /* +0x1200a: copied to SoundState.unk24 with the listener */
    u8 unk1200c[0x48];
} Data801d4464;

typedef struct Data801e7edc {
    u8 unk0[0x9c];
    u8 unk9c[0x2d4];
    s16 unk370;
    s16 unk372;
    u8 unk374[0xc];
    u8 *unk380;
    u8 unk384;
    u8 unk385[7];
} Data801e7edc;

/*
 * Object records func_80017aa4 moves when the area origin shifts (WIP: named
 * by size; 0xff/0xffff in the first field marks a free record).
 */
typedef struct Object18 {
    u16 unk0;       /* +0x00: 0xffff when free */
    u8 unk2[6];
    s32 x;          /* +0x08 */
    s32 y;
    s32 z;
    u8 unk14[4];
} Object18;

typedef struct Object44 {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u16 unk6;       /* +0x06: 0xff when free */
    u8 unk8[0xc];
    s32 x;          /* +0x14 */
    s32 y;
    s32 z;
    u8 unk20[0x24];
} Object44;

typedef struct Object4c {
    u8 unk0;        /* +0x00: 0xff when free */
    u8 unk1[7];
    u8 unk8;        /* +0x08: both of bits 2 and 3 set keeps it in place */
    u8 unk9[0xf];
    s32 x;          /* +0x18 */
    s32 y;
    s32 z;
    u8 unk24[0x28];
} Object4c;

typedef struct Object88 {
    u8 unk0;        /* +0x00: 0xff when free */
    u8 unk1[0x2b];
    s32 x;          /* +0x2c */
    s32 y;
    s32 z;
    u8 unk38[0x50];
} Object88;

typedef struct Data80185da8 {
    Object88 objects[200];
    u8 unk6a40[0x32ec];
} Data80185da8;

/* One entry of a loaded FDAT module's handler table. */
typedef void (*FdatHandler)(void);

/*
 * Area state func_80017664 and func_8001796c set up (WIP). Only the scene
 * module sees the step the CD callbacks set as volatile: the menu module's
 * accesses to the same object show none of that (a volatile member changes
 * how GCC 2.6 reaches every member of the aggregate; see game/cd.h).
 */
#ifdef GAME_SCENE_MODULE
#define SCENE_STEP_SHARED volatile
#else
#define SCENE_STEP_SHARED
#endif

typedef struct Data8018fad4 {
    u16 loading;    /* +0x00: func_8001796c steps func_80018358 until it clears */
    SCENE_STEP_SHARED s16 step; /* +0x02: the CD callbacks func_800182e0.. set it */
    u8 area[5];     /* +0x04: 99 asks for a reload, 0xff keeps a part */
    u8 unk9;
    u8 unka[2];
    FdatHandler *handlers; /* +0x0c: the loaded FDAT module's handler table */
    u8 previous[5]; /* +0x10 */
    u8 unk15[4];
    s8 shift[3];    /* +0x19: 0x7f is no shift */
    u8 unk1c[0x4c];
} Data8018fad4;

typedef struct Data8018fb3c {
    u8 unk0[0x1f20];
    Object44 objects[0x18c];
    u8 unk8850[0x18];
} Data8018fb3c;

/* 0x18-byte records at the start of DAT_801b77ec (WIP; the spells func_8001c6c0 lists). */
typedef struct Record18 {
    u8 unk0;        /* +0x00: 1 when the entry is listed */
    u8 unk1;
    u8 unk2;
    u8 unk3[0x13];
    u16 unk16;      /* +0x16: the value func_8001c6c0 lists with it */
} Record18;

typedef struct Data801b77ec {
    Record18 unk0[96];
    Object4c objects[128];
    u8 unk2f00[0xc];
} Data801b77ec;

/*
 * FDAT window words in the load image (owner unit unknown): [2] the window
 * base, [4] the end of the loaded module, [5] the heap base main stores.
 */
extern u32 DAT_800136e8[6];

/* Arena func_8001937c formats with a 0x4d800-byte free block. */
extern u8 DAT_800c8954[0x4d800];

extern Object18 DAT_80182968[128];
extern Data80185da8 DAT_80185da8;
extern Data8018fad4 DAT_8018fad4;
extern Data8018fb3c DAT_8018fb3c;
extern u32 DAT_80199154[0x5e2a];
extern Data801b24e4 DAT_801b24e4;
/* Area map bits: rooms seen per area (32 areas x 160 bits) and the 80 x 80 grid of
 * visited cells func_8001bea8 marks rooms from. */
extern u8 DAT_801b56cc[32][20];
extern u8 DAT_801b594c[80][10];
extern Data801b77ec DAT_801b77ec;
extern Data801ba984 DAT_801ba984;
extern Data801d4464 DAT_801d4464;
extern Data801e7edc DAT_801e7edc;

/* Tables func_80017664 copies out of the first FDAT.T entry (WIP extents). */
extern u32 DAT_80081c8c[];
extern u32 DAT_8009f114[];
extern u32 DAT_800aedc4[];
extern u32 DAT_80116154[];
extern u32 DAT_801983a8[];
extern u32 DAT_801d37a4[];
extern u32 DAT_801e64b8[];
extern u8 DAT_8014f164[];

#endif
