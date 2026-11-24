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
    u16 unk1e;
    s16 unk20;
    u16 unk22;
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
    s32 unka4;
    s32 unka8;
    s32 unkac;
    struct Record44 *unkb0; /* +0xb0: &DAT_801d37a4[unkcb] */
    u8 *unkb4;      /* +0xb4 */
    s32 unkb8;
    s32 unkbc;
    s16 unkc0;
    u16 unkc2;
    u16 unkc4;
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
    struct Record20 *unkd4; /* +0xd4..+0xec: &DAT_801d4464.unk11c14[] of unkf0..unkf6 */
    struct Record20 *unkd8;
    struct Record20 *unkdc;
    struct Record20 *unke0;
    struct Record20 *unke4;
    struct Record20 *unke8;
    struct Record20 *unkec;
    u8 unkf0;       /* +0xf0..+0xf6: Record20 indices, 0xff for none */
    u8 unkf1;
    u8 unkf2;
    u8 unkf3;
    u8 unkf4;
    u8 unkf5;
    u8 unkf6;
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
    VECTOR pos;     /* +0x10c */
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
    SVECTOR unk14c; /* +0x14c: func_8002c164 rotates it by unk124 */
    s32 unk154;
    s32 unk158;
    s32 unk15c;
    u16 unk160;
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
    s32 unk0;
    u8 unk4[9];
    u8 unkd;        /* +0x0d: the game loop sets it to 1 on a continued game */
    u8 unke[0x1d];
    u8 unk2b;
    u8 unk2c[5];
    u8 unk31;
    u8 unk32[0x33];
    u8 unk65;
    u8 unk66[0xb];
    u8 unk71[5];    /* +0x71: per magic element 0..3; element 4 needs none */
    u8 unk76[9];
    u8 unk7f[5];    /* +0x7f: per magic element */
    u8 unk84[0x50c0];
} Data801ba984;

/* 0x20-byte records func_800293e4 adds to the player's halfwords +0x66..+0x76. */
typedef struct Record20 {
    u8 unk0[2];
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u16 unk8;
    u16 unka;
    u16 unkc;
    u16 unke;
    u16 unk10;
    u16 unk12;
    u8 unk14[0xc];
} Record20;

typedef struct Data801d4464 {
    u8 unk0[0x11c14];
    Record20 unk11c14[31]; /* +0x11c14: WIP count */
    u8 unk11ff4[0xc];
    u8 unk12000[0xa];
    u16 unk1200a;   /* +0x1200a: copied to SoundState.unk24 with the listener */
    u8 unk1200c[0xc];
    s32 unk12018[3]; /* +0x12018: heights func_80028d54 measures the player against */
    u8 unk12024[0x30];
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

typedef struct Object88 {
    u8 unk0;        /* +0x00: 0xff when free */
    u8 unk1[0x2b];
    s32 x;          /* +0x2c */
    s32 y;
    s32 z;
    u8 unk38[0x50];
} Object88;

/* 120-byte records func_8002c164 hands to func_8004c068 (WIP). */
typedef struct Record78 {
    u8 unk0[0x78];
} Record78;

typedef struct Data80185da8 {
    Object88 objects[200];
    Record78 unk6a40[108];
    u8 unk9ce0[0x48];
    struct Object2c *unk9d28; /* +0x9d28: func_8002c164 keeps its last object here */
} Data80185da8;

/* What func_8004d3c4 returns to func_8002c164 (WIP, referenced prefix). */
typedef struct Object2c {
    u8 unk0[2];
    u8 unk2;        /* +0x02: Data80185da8.unk6a40 index */
    u8 unk3[0x25];
    u32 unk28;      /* +0x28: flags; 0x100 skips func_8004b94c */
} Object2c;

typedef struct Data80199154 {
    u8 unk0[0x1015c];
    u8 *unk1015c[0x1dd3]; /* +0x1015c: arena blocks; func_800290d8 marks 32 of them cached */
} Data80199154;

/* 0x18-byte records at the start of DAT_801b77ec (WIP). */
typedef struct Record18 {
    u8 unk0;        /* +0x00: func_80029f1c sets it to 1 */
    u8 unk1;
    u8 unk2;        /* +0x02: SoundState.effects[unk2 + 144], 0xff for none */
    u8 unk3[0x15];
} Record18;

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

typedef struct Data801b77ec {
    Record18 unk0[96];
    Object4c objects[128];
    u8 unk2f00[0xc];
} Data801b77ec;

/* 12-byte level records (DAT_8009f114, one per level). */
typedef struct Record0c {
    u16 unk0;       /* new Data801b24e4.unk16 */
    u16 unk2;       /* new Data801b24e4.unk1a */
    u16 unk4;       /* added to Data801b24e4.unk32 */
    u16 unk6;
    s32 unk8;       /* experience for the next level */
} Record0c;

/* 0x44-byte records func_80029500 adds to the player's unk54[]/unk62 (WIP). */
typedef struct Record44 {
    u8 unk0[5];
    u8 unk5;
    u16 unk6[8];
    u8 unk16[8];
    u16 unk1e;
    u8 unk20[6];
    u16 unk26;
    u8 unk28[4];
    u16 unk2c;
    u16 unk2e;
    u8 unk30[0x14];
} Record44;

/* One entry of a loaded FDAT module's handler table. */
typedef void (*FdatHandler)(void);

/* Area state func_80017664 and func_8001796c set up (WIP). */
typedef struct Data8018fad4 {
    u16 loading;    /* +0x00: func_8001796c steps func_80018358 until it clears */
    s16 step;       /* +0x02: the CD callbacks func_800182e0.. set it (band C: volatile) */
    u8 area[5];     /* +0x04: 99 asks for a reload, 0xff keeps a part */
    u8 unk9;
    u8 unka[2];
    FdatHandler *handlers; /* +0x0c: the loaded FDAT module's handler table */
    u8 previous[5]; /* +0x10 */
    u8 unk15[4];
    s8 shift[3];    /* +0x19: 0x7f is no shift */
    u8 unk1c[0x4c];
} Data8018fad4;

/*
 * FDAT window words in the load image (owner unit unknown): [2] the window
 * base, [4] the end of the loaded module, [5] the heap base main stores.
 */
extern u32 DAT_800136e8[6];

/* Arena func_8001937c formats with a 0x4d800-byte free block. */
extern u8 DAT_800c8954[0x4d800];

extern Data80185da8 DAT_80185da8;
extern Data8018fad4 DAT_8018fad4;
extern u32 DAT_8018fb3c[0x221a];
extern Data80199154 DAT_80199154;
extern Data801b24e4 DAT_801b24e4;
extern u32 DAT_801b56cc[0xa0];
extern u32 DAT_801b594c[0xc8];
extern Data801b77ec DAT_801b77ec;
extern Data801ba984 DAT_801ba984;
extern Data801d4464 DAT_801d4464;
extern Record0c DAT_8009f114[100];
/* What func_8002b64c points Data801b24e4.unkb4 at (WIP extent). */
extern u8 DAT_8009f5c4[];
extern Record44 DAT_801d37a4[48];
/* Written by func_80029500 (band C models it as CdState.unkc). */
extern s32 DAT_801c12f0;
extern Data801e7edc DAT_801e7edc;

#endif
