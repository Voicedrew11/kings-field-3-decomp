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

typedef struct Data801b24e4 {
    u8 unk0[0xe];
    u8 unke;        /* +0x0e: cleared every frame by the game loop */
    u8 unkf[0x95];
    u32 play_minutes; /* +0xa4: the VSync handler adds one every 3600 VSyncs */
    u8 unka8[0x4f];
    u8 se_volume;   /* +0xf7: 0 mutes sound effects; scales them by n/128 */
    u8 bgm_volume;  /* +0xf8: 0 keeps the sequence off */
    u8 unkf9[0x13];
    s32 x;          /* +0x10c: func_80017aa4 moves it with the objects */
    s32 y;
    s32 z;
    u8 unk118[0x84];
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

/* Area state func_80017664 and func_8001796c set up (WIP). */
typedef struct Data8018fad4 {
    u16 loading;    /* +0x00: func_8001796c steps func_80018358 until it clears */
    volatile s16 step; /* +0x02: the CD callbacks func_800182e0.. set it */
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

typedef struct Data801b77ec {
    u8 unk0[0x900];
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
extern u32 DAT_801b56cc[0xa0];
extern u32 DAT_801b594c[0xc8];
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
