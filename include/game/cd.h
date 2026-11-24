#ifndef GAME_CD_H
#define GAME_CD_H

/*
 * GAME.EXE CD read requests (WIP). func_8001967c resets a request and walks
 * an array of 0x2c-byte requests that ends at DAT_801c1724; the sound module
 * uses the two requests DAT_801c1724 and DAT_801c1728 point to. Only the
 * fields band A touches are named.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBCD.H>

struct SoundSlot;

typedef struct CdRequest {
    u8 unk0;                /* +0x00: 0x30 while the sound read is current */
    u8 unk1;                /* +0x01 */
    u8 unk2[2];
    s16 bank;               /* +0x04: SoundState.banks index */
    struct SoundSlot *slot; /* +0x08 */
    s32 spu_addr;           /* +0x0c */
    u8 state;               /* +0x10 */
    u8 unk11[3];
    CdlLOC pos;             /* +0x14 */
    CdlLOC retry_pos;       /* +0x18 */
    u8 *buffer;             /* +0x1c */
    s32 count;              /* +0x20 */
    u8 unk24[8];
} CdRequest;

extern CdRequest *DAT_801c1724;
extern CdRequest *DAT_801c1728;
extern u8 DAT_801c17a4[];

void func_8001967c(CdRequest *request);
void func_80019a74(CdlLOC *pos, s32 count, CdlLOC *out);
void func_80019b1c(CdRequest *request);
s32 func_80019b68(u8 *buffer, s32 sectors);
/* The callee truncates file and sector to 16 bits; callers pass ints unconverted. */
void func_80019df0(s32 file, s32 sector, u8 *buffer, void (*callback)(CdRequest *request));

#endif
