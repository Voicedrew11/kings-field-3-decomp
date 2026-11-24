#ifndef GAME_CARD_H
#define GAME_CARD_H

/*
 * GAME.EXE memory card saves (src/game/card.c, WIP module). A save file is
 * BASLUS-00255 plus the save number (1 to 15) on slot 1 and holds 0x6000
 * bytes: the BIOS file header (SaveHeader), the checksum of the data at
 * +0x200, and 0x5c00 data bytes at CARD_DATA.
 */
#include <kf/lib/types.h>
#include <psyq/kernel.h>

#define CARD_FILE_SIZE 0x6000
#define CARD_DATA 0x400
#define CARD_DATA_SIZE 0x5c00

/* The memory card file header the BIOS file browser reads. */
typedef struct SaveHeader {
    char magic[2];      /* +0x000: "SC" */
    u8 type;            /* +0x002: 0x13, a three-frame icon */
    u8 blocks;          /* +0x003 */
    char title[64];     /* +0x004: Shift-JIS; the save number, experience and level are digits in it */
    u8 unk44[0x1c];
    u16 clut[16];       /* +0x060 */
    u8 icon[3][128];    /* +0x080: 16x16 4-bit frames */
    u32 checksum;       /* +0x200: byte sum of the data */
    u8 unk204[0x7c];
} SaveHeader;

void func_80027f2c(void);
void func_80027fe4(void);
void func_80028034(void);
void func_80028090(void);
s32 func_800280d4(void);
s32 func_800281c4(struct DIRENTRY *dir, s32 *count);
s32 func_80028398(char *name, s32 *experience, s32 *level, s32 *number);
s32 func_800285dc(void);
s32 func_8002860c(s32 number);
s32 func_80028750(s32 number);
void func_80028b70(SaveHeader *header, s32 number);
u32 func_80028c74(u8 *data);
s32 func_80028c98(void);
void func_80028d0c(void);

#endif
