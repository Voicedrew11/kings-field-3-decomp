/*
 * END.EXE's movie player: plays one of the three closing movies (\OP\M4.S,
 * M5.S, M6.S) up to its last frame. The str* functions follow the shape of
 * Sony's movie playback sample (DECENV, strSetDefDecEnv, strInit,
 * strCallback, strNextVlc, strNext, strSync, strKickCD), with a 16-bit
 * image, one slice buffer and a letterboxed first movie.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/press.h>
#include "end.h"

/* Polling bound of the stream waits. */
#define WAIT_TIME 0x800000
/* Ring buffer length in sectors. */
#define RING_SIZE 32

static void strSetDefDecEnv(int movie);
static void strInit(CdlLOC *loc);
static void strCallback(void);
static int strNextVlc(void);
static u_long *strNext(DECENV *dec);
static void strSync(DECENV *dec);
static void strKickCD(CdlLOC *loc);

RODATA(0x80011000, 0x48)

/*
 * Fixed small and large statics: they start .sbss and .bss, ahead of the
 * MALLOC.OBJ and SDK statics and of every COMMON request. Nothing reads
 * or writes the first two words.
 */
DATA(0x80037498, 0x4, ".sbss")
static int movie_unused_0;
DATA(0x8003749c, 0x4, ".sbss")
static int movie_unused_1;
DATA(0x800374a0, 0x4, ".sbss")
static int frame_no;

DATA(0x800374dc, 0x50000, ".bss")
static u_long vlc_buffer[2][320 / 2 * 256];
DATA(0x800874dc, 0x3c00, ".bss")
static u_long image_buffer[2][16 * 240 / 2];
DATA(0x8008b0dc, 0x10000, ".bss")
static u_long ring_buffer[RING_SIZE * 2048 / 4];
DATA(0x8009b0dc, 0x30, ".bss")
static DECENV dec;

ADDRESS(0x80011d14, 0x250)
void play_movie(int movie)
{
    int frame = 0;
    int no;
    CdlFILE file;
    int end_frame[3] = {3380, 942, 1841};
    char names[3][20] = {"\\OP\\M4.S;1", "\\OP\\M5.S;1", "\\OP\\M6.S;1"};

    no = movie;
    if (CdSearchFile(&file, names[no]) == 0)
        return;
    SsSetMVol(127, 127);
    SsSetSerialVol(SS_SERIAL_A, 127, 127);
    strSetDefDecEnv(no);
    strInit(&file.pos);
    strNextVlc();
    cdb = db;
    frame_no = 0;
    for (;;) {
        swap_buffers();
        DecDCTin(dec.vlcbuf[dec.vlcid], 2);
        DrawSync(0);
        DecDCTout(dec.imgbuf, dec.slice.w * dec.slice.h / 2);
        strNextVlc();
        strSync(&dec);
        flip_buffers();
        if (frame_no > end_frame[no])
            break;
        if (++frame > end_frame[no])
            break;
    }
    StUnSetRing();
    CdControlB(CdlPause, 0, 0);
}

ADDRESS(0x80011f64, 0x128)
static void strSetDefDecEnv(int movie)
{
    dec.vlcbuf[0] = vlc_buffer[0];
    dec.vlcbuf[1] = vlc_buffer[1];
    dec.vlcid = 0;
    dec.imgbuf = image_buffer[0];
    dec.rectid = 0;
    dec.isdone = 0;
    if (movie) {
        setRECT(&dec.rect[0], 0, 0, 320, 240);
        setRECT(&dec.rect[1], 0, 240, 320, 240);
        setRECT(&dec.slice, 0, 0, 16, 240);
    } else {
        setRECT(&dec.rect[0], 0, 32, 320, 176);
        setRECT(&dec.rect[1], 0, 272, 320, 176);
        setRECT(&dec.slice, 0, 32, 16, 176);
    }
}

ADDRESS(0x8001208c, 0x70)
static void strInit(CdlLOC *loc)
{
    DecDCTReset(0);
    DecDCToutCallback(strCallback);
    StSetRing(ring_buffer, RING_SIZE);
    StSetStream(0, 1, 0x0fffffff, 0, 0);
    strKickCD(loc);
}

ADDRESS(0x800120fc, 0x124)
static void strCallback(void)
{
    LoadImage(&dec.slice, dec.imgbuf);
    dec.slice.x += dec.slice.w;
    if (dec.slice.x < dec.rect[dec.rectid].x + dec.rect[dec.rectid].w) {
        DrawSync(0);
        DecDCTout(dec.imgbuf, dec.slice.w * dec.slice.h / 2);
    } else {
        dec.isdone = 1;
        dec.rectid = dec.rectid ? 0 : 1;
        dec.slice.x = dec.rect[dec.rectid].x;
        dec.slice.y = dec.rect[dec.rectid].y;
    }
}

ADDRESS(0x80012220, 0x94)
static int strNextVlc(void)
{
    int cnt = WAIT_TIME;
    u_long *next;

    while ((next = strNext(&dec)) == 0) {
        if (--cnt == 0)
            return -1;
    }
    dec.vlcid = dec.vlcid ? 0 : 1;
    DecDCTvlc(next, dec.vlcbuf[dec.vlcid]);
    StFreeRing(next);
    return 0;
}

ADDRESS(0x800122b4, 0x94)
static u_long *strNext(DECENV *dec)
{
    u_long *addr;
    StHEADER *sector;
    int cnt = WAIT_TIME;

    while (StGetNext(&addr, (u_long **)&sector)) {
        if (--cnt == 0)
            return 0;
    }
    if (addr[0] != sector->dummy1 || addr[1] != sector->dummy2) {
        StFreeRing(addr);
        return 0;
    }
    frame_no = sector->frameCount;
    return addr;
}

ADDRESS(0x80012348, 0x78)
static void strSync(DECENV *dec)
{
    u_long cnt = WAIT_TIME;

    while (dec->isdone == 0) {
        if (--cnt == 0) {
            dec->isdone = 1;
            dec->rectid = dec->rectid ? 0 : 1;
            dec->slice.x = dec->rect[dec->rectid].x;
            dec->slice.y = dec->rect[dec->rectid].y;
        }
    }
    dec->isdone = 0;
}

ADDRESS(0x800123c0, 0x4c)
static void strKickCD(CdlLOC *loc)
{
    while (CdControl(CdlSeekL, (u_char *)loc, 0) == 0)
        ;
    while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0)
        ;
}
