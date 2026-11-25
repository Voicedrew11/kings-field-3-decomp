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
#include <psyq/cd.h>

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
#define CD_REQUEST_IMAGE 0x40  /* VRAM image read (cd_request_service_stream) */

/* Called with the buffer (CD_REQUEST_READ) or with the request; stored
 * untyped and cast to one of the two below where it is called. */
typedef void (*CdCallback)();
struct CdRequest;
typedef void (*CdBufferCallback)(u8 *buffer);
typedef void (*CdRequestCallback)(struct CdRequest *request);

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

/* Where an archive file lies; cd_extent_read_into reads from it like a CdlFILE. */
typedef struct CdExtent {
    CdlLOC pos;
    u32 size;
} CdExtent;

typedef struct CdArchive {
    u16 *sectors;           /* +0x0: first sector of each entry, plus the end */
    CdExtent extent;        /* +0x4 */
} CdArchive;

typedef struct CdState {
    s32 vsync_event;            /* +0x000: root counter 3 interrupt */
    u32 frames;                 /* +0x004: VSyncs since start-up */
    CD_VSYNC_SHARED u32 frame_gate; /* +0x008: VSyncs since the last cd_wait_four_vsyncs */
    u32 unkc;                   /* +0x00c: counts VSyncs; player_recalculate_combat_stats stores the
                                 * player's +0xa0 in it; nothing reads it yet */
    s32 error_event;            /* +0x010: HwCdRom EvSpERROR */
    s32 complete_event;         /* +0x014: HwCdRom EvSpCOMP */
    s32 unk18;
    s32 ready_event;            /* +0x01c: HwCdRom EvSpDR */
    CdRequest requests[24];     /* +0x020 */
    CdRequest *write;           /* +0x440: next request to fill */
    CdRequest *read;            /* +0x444: request being read */
    CdArchive archives[10];     /* +0x448 */
} CdState;

extern CdState cdrom_ctl;
extern u8 stream_work_buffer[];

void cd_request_yield(void);
void cd_wait_four_vsyncs(void);
void cd_request_advance(CdRequest *request);
void cd_location_add(CdlLOC *pos, s32 offset, CdlLOC *out);
void cd_request_wait_idle(void);
/* The CdReadyCallback that runs the request queue (the menu puts it back,
 * menu_cd_restore_data_ready_handler). */
void cd_data_ready_handler(void);
void cd_request_wait_done(CdRequest *request);
/* Word checksum of a sector read: nonzero when the last word disagrees. */
s32 cd_sectors_corrupt(u8 *buffer, s32 sectors);
s32 cd_archive_entry_extent(s32 file, s32 entry, CdlLOC *pos);
/* 16-bit file and entry numbers: cd_archive_queue_sector_callback_read/df0 take ints and narrow them
 * at entry (callers pass ints unconverted); cd_archive_read_chunked takes u16 itself,
 * since the int-and-local form swaps its file/entry registers. */
void cd_archive_queue_read(s32 file, s32 entry, u8 *buffer, CdCallback callback);
void cd_archive_queue_sector_callback_read(int file, int entry, u8 *buffer, CdCallback callback);
void cd_archive_queue_stream_read(int file, int entry, u8 *buffer, CdCallback callback);
void cd_archive_read_chunked(u16 file, u16 entry, u8 *buffer, CdCallback callback);
s32 cd_archive_entry_size(int file, int entry);
void cd_report_error(s32 fatal);
void cd_read_sectors(CdlLOC *pos, u8 *buffer, s32 sectors);
u8 *cd_extent_load(CdlFILE *file);
void cd_extent_read_into(u8 *buffer, CdExtent *extent, u32 bytes);
void cd_archive_read(int file, int entry, u8 *buffer);
u8 *cd_file_load(char *name);
s32 cd_file_load_into(u8 *buffer, char *name, u32 bytes);
void cd_archive_open(int file, char *name);
void cd_initialize(void);
void cd_close_events(void);

#endif
