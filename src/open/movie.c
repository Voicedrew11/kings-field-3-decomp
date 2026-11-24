/*
 * OPEN.EXE's movie player: plays one of the five opening movies up to its
 * last frame or until a button is pressed, fading the CD audio in and out,
 * then restores the 640-wide title display. The str* functions are END.EXE's
 * (src/end/movie.c) Sony movie-sample player, bar the CdRead2 mode.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/press.h>
#include "open.h"

/* Polling bound of the stream waits. */
#define WAIT_TIME 0x800000
/* Ring buffer length in sectors. */
#define RING_SIZE 32
/*
 * strKickCD's CdRead2 mode 0x1c0 is END.EXE's CdlModeStream | CdlModeSpeed |
 * CdlModeRT with the streaming bit alone: OPEN was built against Library
 * 3.4, whose LIBCD.H is not preserved; the 3.3 header spells CdlModeStream
 * 0x120.
 */
#define STREAM_MODE (0x100 | CdlModeSpeed | CdlModeRT)

static void strSetDefDecEnv(int movie);
static void strInit(CdlLOC *loc);
static void strCallback(void);
static int strNextVlc(void);
static u_long *strNext(DECENV *dec);
static void strSync(DECENV *dec);
static void strKickCD(CdlLOC *loc);

RODATA(0x80011014, 0x78)

/* Nothing reads or writes these two words. */
DATA(0x80040420, 0x4, ".sdata")
int movie_unused_0 = 0;
DATA(0x80040424, 0x4, ".sdata")
int movie_unused_1 = 0;
DATA(0x80040434, 0x4, ".sbss")
static int frame_no;

DATA(0x800404a0, 0x50000, ".bss")
static u_long vlc_buffer[2][320 / 2 * 256];
DATA(0x800904a0, 0x3c00, ".bss")
static u_long image_buffer[2][16 * 240 / 2];
DATA(0x800940a0, 0x10000, ".bss")
static u_long ring_buffer[RING_SIZE * 2048 / 4];
DATA(0x800a40a0, 0x30, ".bss")
static DECENV dec;

/* 0 when the movie ran out, 1 when START stopped it, 2 for another button. */
ADDRESS(0x800136d8, 0x494)
int play_movie(int movie)
{
    int frame = 0;
    int result = 0;
    CdlFILE file;
    u_char unused[8]; /* an 8-byte frame slot nothing touches */
    int end_frame[5] = {3837, 1376, 2102, 137, 130};
    char names[5][20] = {
        "\\OP\\M0.S;1", "\\OP\\M1.S;1", "\\OP\\M3.S;1", "\\OP\\L0.S;1", "\\OP\\L1.S;1",
    };
    int vol;
    u_long pad;

    /* Old-C valueless return: the caller sees CdSearchFile's NULL left in v0. */
    if (CdSearchFile(&file, names[movie]) == 0)
        return;
    SsSetMVol(127, 127);
    for (vol = 0; vol < 127; vol++) {
        SsSetSerialVol(SS_SERIAL_A, vol, vol);
        if ((vol & 7) == 0)
            VSync(0);
    }
    strSetDefDecEnv(movie);
    strInit(&file.pos);
    strNextVlc();
    SetDefDrawEnv(&db[0].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&db[1].draw, 0, 240, 320, 240);
    SetDefDispEnv(&db[0].disp, 0, 240, 320, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, 320, 240);
    cdb = db;
    for (;;) {
        swap_buffers();
        DecDCTin(dec.vlcbuf[dec.vlcid], 2);
        DrawSync(0);
        DecDCTout(dec.imgbuf, dec.slice.w * dec.slice.h / 2);
        strNextVlc();
        strSync(&dec);
        flip_buffers();
        pad = PadRead(1);
        if (pad & PADstart) {
            result = 1;
            break;
        }
        if (pad & PADRup || pad & PADRleft || pad & PADRright || pad & PADRdown) {
            result = 2;
            break;
        }
        if (frame_no > end_frame[movie])
            break;
        if (++frame > end_frame[movie])
            break;
    }
    if (frame_no < end_frame[movie] || movie == 0) {
        for (vol = 127; vol > 0; vol--) {
            SsSetSerialVol(SS_SERIAL_A, vol, vol);
            if ((vol & 1) == 0)
                VSync(0);
        }
    } else {
        SsSetSerialVol(SS_SERIAL_A, 0, 0);
    }
    StUnSetRing();
    CdControlB(CdlPause, 0, 0);
    while (PadRead(1))
        ;
    db[0].draw.isbg = db[1].draw.isbg = 1;
    VSync(0);
    cdb = &db[0];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    VSync(0);
    cdb = &db[1];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    VSync(0);
    SetDefDrawEnv(&db[0].draw, 0, 0, 640, 240);
    SetDefDrawEnv(&db[1].draw, 0, 240, 640, 240);
    SetDefDispEnv(&db[0].disp, 0, 240, 640, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, 640, 240);
    db[0].draw.isbg = db[1].draw.isbg = 1;
    return result;
}

ADDRESS(0x80013b6c, 0x128)
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

ADDRESS(0x80013c94, 0x70)
static void strInit(CdlLOC *loc)
{
    DecDCTReset(0);
    DecDCToutCallback(strCallback);
    StSetRing(ring_buffer, RING_SIZE);
    StSetStream(0, 1, 0x0fffffff, 0, 0);
    strKickCD(loc);
}

ADDRESS(0x80013d04, 0x124)
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

ADDRESS(0x80013e28, 0x94)
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

ADDRESS(0x80013ebc, 0x94)
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

ADDRESS(0x80013f50, 0x78)
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

ADDRESS(0x80013fc8, 0x4c)
static void strKickCD(CdlLOC *loc)
{
    while (CdControl(CdlSeekL, (u_char *)loc, 0) == 0)
        ;
    while (CdRead2(STREAM_MODE) == 0)
        ;
}
