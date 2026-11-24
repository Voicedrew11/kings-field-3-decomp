#ifndef GAME_CD_H
#define GAME_CD_H

/*
 * GAME.EXE CD and VSync state (src/game/cd.c, WIP names): the root-counter
 * frame counters, the CD event ids, a ring of 24 0x2c-byte CD read requests
 * with its write and read pointers, and the table of archive files whose
 * sector index the loader keeps. One aggregate: the module reaches the
 * requests from the VSync event's base address and compares the write
 * pointer with &requests[24] as one address.
 *
 * Only the owning module sees the VSync counter the frame gate polls as
 * volatile: a volatile member changes how GCC 2.6 reaches every member of
 * the aggregate, and the sound module's accesses to the same object show
 * none of that, so its declaration had no volatile member.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBCD.H>

#ifdef GAME_CD_MODULE
#define CD_VSYNC_SHARED volatile
#else
#define CD_VSYNC_SHARED
#endif

struct SoundSlot;

/* CdRequest.kind values. */
#define CD_REQUEST_FREE 0
#define CD_REQUEST_READ 0x10   /* checksummed read, then callback(buffer) */
#define CD_REQUEST_STREAM 0x20 /* CdRead2 stream, callback(request) per sector */
#define CD_REQUEST_SOUND 0x30  /* sound bank read (src/game/sound.c) */
#define CD_REQUEST_IMAGE 0x40  /* VRAM image read (func_80018cd0) */

/* Called with the buffer (CD_REQUEST_READ) or with the request. */
typedef void (*CdCallback)();

/* CdRequest.u for a sound bank read. */
typedef struct CdSoundRequest {
    s16 bank;               /* SoundState.banks index */
    s16 pad;
    struct SoundSlot *slot;
    s32 spu_addr;
} CdSoundRequest;

/* CdRequest.u for a VRAM image read. */
typedef struct CdImageRequest {
    RECT rect;              /* VRAM destination of the next part */
    u16 sectors;            /* whole image */
    s16 remaining;          /* sectors still to read after this part */
} CdImageRequest;

typedef union CdRequestUnion {
    CdSoundRequest sound;
    CdImageRequest image;
} CdRequestUnion;

typedef struct CdRequest {
    u8 kind;                /* +0x00: CD_REQUEST_* */
    u8 issued;              /* +0x01: 1 once CdRead/CdRead2 accepted the read */
    u8 unk2[2];
    CdRequestUnion u;       /* +0x04 */
    u8 state;               /* +0x10 */
    u8 unk11[3];
    CdlLOC pos;             /* +0x14 */
    CdlLOC retry_pos;       /* +0x18 */
    u8 *buffer;             /* +0x1c */
    s32 count;              /* +0x20: sectors */
    CdCallback callback;    /* +0x24 */
    u8 unk28[4];
} CdRequest;

/* Where an archive file lies; func_8001a118 reads from it like a CdlFILE. */
typedef struct CdExtent {
    CdlLOC pos;
    u32 size;
} CdExtent;

typedef struct CdFile {
    u16 *sectors;           /* +0x0: first sector of each entry, plus the end */
    CdExtent extent;        /* +0x4 */
} CdFile;

typedef struct CdState {
    s32 vsync_event;            /* +0x000: root counter 3 interrupt */
    u32 frames;                 /* +0x004: VSyncs since start-up */
    CD_VSYNC_SHARED u32 frame_gate; /* +0x008: VSyncs since the last func_80019614 */
    u32 unkc;                   /* +0x00c: counts VSyncs; nothing reads it yet */
    s32 error_event;            /* +0x010: HwCdRom EvSpERROR */
    s32 complete_event;         /* +0x014: HwCdRom EvSpCOMP */
    s32 unk18;
    s32 ready_event;            /* +0x01c: HwCdRom EvSpDR */
    CdRequest requests[24];     /* +0x020 */
    CdRequest *write;           /* +0x440: next request to fill */
    CdRequest *read;            /* +0x444: request being read */
    CdFile files[10];           /* +0x448 */
} CdState;

extern CdState DAT_801c12e4;
extern u8 DAT_801c17a4[];

void func_80019538(void);
void func_80019614(void);
void func_8001967c(CdRequest *request);
void func_80019a74(CdlLOC *pos, s32 offset, CdlLOC *out);
void func_80019ab4(void);
void func_80019b1c(CdRequest *request);
/* Word checksum of a sector read: nonzero when the last word disagrees. */
s32 func_80019b68(u8 *buffer, s32 sectors);
s32 func_80019cc4(s32 file, s32 entry, CdlLOC *pos);
/* These definitions are old-style with 16-bit file and entry numbers: the
 * callee truncates them and callers pass ints unconverted. */
void func_80019d48(s32 file, s32 entry, u8 *buffer, CdCallback callback);
void func_80019d98(int file, int entry, u8 *buffer, CdCallback callback);
void func_80019df0(int file, int entry, u8 *buffer, CdCallback callback);
void func_80019e48(int file, int entry, u8 *buffer, CdCallback callback);
s32 func_80019f14(int file, int entry);
void func_80019f58(s32 fatal);
void func_80019f60(CdlLOC *pos, u8 *buffer, s32 sectors);
u8 *func_8001a0bc(CdlFILE *file);
void func_8001a118(u8 *buffer, CdExtent *extent, u32 bytes);
void func_8001a154(int file, int entry, u8 *buffer);
u8 *func_8001a1e8(char *name);
s32 func_8001a298(u8 *buffer, char *name, u32 bytes);
void func_8001a34c(int file, char *name);
void func_8001a438(void);
void func_8001a5c0(void);

#endif
