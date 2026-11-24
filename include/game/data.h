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
    u8 unkf[0xe8];
    u8 se_volume;   /* +0xf7: 0 mutes sound effects; scales them by n/128 */
    u8 bgm_volume;  /* +0xf8: 0 keeps the sequence off */
    u8 unkf9[0xa3];
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
 * FDAT window words in the load image (owner unit unknown): [2] the window
 * base, [4] the end of the loaded module, [5] the heap base main stores.
 */
extern u32 DAT_800136e8[6];

/* Arena func_8001937c formats with a 0x4d800-byte free block. */
extern u8 DAT_800c8954[0x4d800];

extern u32 DAT_80185da8[0x274b];
extern u32 DAT_8018fad4[0x1a];
extern u32 DAT_8018fb3c[0x221a];
extern u32 DAT_80199154[0x5e2a];
extern Data801b24e4 DAT_801b24e4;
extern u32 DAT_801b56cc[0xa0];
extern u32 DAT_801b594c[0xc8];
extern u32 DAT_801b77ec[0xbc3];
extern Data801ba984 DAT_801ba984;
extern Data801d4464 DAT_801d4464;
extern Data801e7edc DAT_801e7edc;

#endif
