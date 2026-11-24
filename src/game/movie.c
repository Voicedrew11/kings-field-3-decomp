/*
 * GAME.EXE: the in-game movie player's Sony movie-sample stream functions
 * (WIP module; src/open/movie.c and src/end/movie.c are the same sample): the
 * decode environment lives in DAT_801e7edc.dec, the ring buffer is
 * DAT_801c17a4, and the display swaps through DAT_80199154. The player
 * func_80060d20 and the decode set-up func_80061364 (not exact) precede them
 * and the switch func_80061940 follows.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <psyq/press.h>
#include <psyq/kernel.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/render.h>
#include <game/misc.h>

/* Polling bound of the stream waits. */
#define WAIT_TIME 0x800000
/* Ring buffer length in sectors. */
#define RING_SIZE 32
/* CdRead2 mode: CdlModeStream | CdlModeSpeed | CdlModeRT as the 3.4 library spells it. */
#define STREAM_MODE (0x100 | CdlModeSpeed | CdlModeRT)

ADDRESS(0x800614d4, 0x7c)
void func_800614d4(CdlLOC *loc)
{
    DAT_801e7edc.unk372 = 0;
    EnterCriticalSection();
    DecDCToutCallback(func_80061550);
    StSetRing((u_long *)DAT_801c17a4, RING_SIZE);
    StSetStream(0, 1, 0x0fffffff, 0, 0);
    ExitCriticalSection();
    func_8006182c(loc);
}

/* MDEC output callback: loads the slice and decodes the next one or flips rect. */
ADDRESS(0x80061550, 0x100)
void func_80061550(void)
{
    DECENV *dec = &DAT_801e7edc.dec;
    RECT *rect;

    LoadImage(&dec->slice, dec->imgbuf);
    dec->slice.x += dec->slice.w;
    rect = dec->rect + dec->rectid;
    if (dec->slice.x < rect->x + rect->w) {
        DrawSync(0);
        DecDCTout(dec->imgbuf, dec->slice.w * dec->slice.h / 2);
    } else {
        dec->isdone = 1;
        dec->rectid = dec->rectid ? 0 : 1;
        rect = dec->rect + dec->rectid;
        dec->slice.x = rect->x;
        dec->slice.y = rect->y;
    }
}

ADDRESS(0x80061650, 0x8c)
s32 func_80061650(void)
{
    int cnt = WAIT_TIME;
    DECENV *dec = &DAT_801e7edc.dec;
    u_long *next;

    while ((next = func_800616dc(dec)) == 0) {
        if (--cnt == 0) {
            return -1;
        }
    }
    dec->vlcid = dec->vlcid ? 0 : 1;
    DecDCTvlc(next, dec->vlcbuf[dec->vlcid]);
    StFreeRing(next);
    return 0;
}

/* Next movie sector; sets unk372 to 99 past the movie's last frame. */
ADDRESS(0x800616dc, 0xc0)
u_long *func_800616dc(DECENV *dec)
{
    u_long *addr;
    StHEADER *sector;
    int cnt = WAIT_TIME;

    while (StGetNext(&addr, (u_long **)&sector)) {
        if (--cnt == 0) {
            return 0;
        }
    }
    if (addr[0] != sector->dummy1 || addr[1] != sector->dummy2) {
        StFreeRing(addr);
        return 0;
    }
    DAT_801e7edc.unk388 = sector->frameCount;
    if (((u16 *)DAT_801e7edc.unk380)[1] < sector->frameCount) {
        DAT_801e7edc.unk372 = 99;
    }
    return addr;
}

ADDRESS(0x8006179c, 0x90)
void func_8006179c(DECENV *dec)
{
    volatile u_long cnt = WAIT_TIME;
    RECT *rect;

    while (dec->isdone == 0) {
        if (--cnt == 0) {
            dec->isdone = 1;
            dec->rectid = dec->rectid ? 0 : 1;
            rect = dec->rect + dec->rectid;
            dec->slice.x = rect->x;
            dec->slice.y = rect->y;
        }
    }
    dec->isdone = 0;
}

ADDRESS(0x8006182c, 0x4c)
void func_8006182c(CdlLOC *loc)
{
    while (CdControl(CdlSeekL, (u_char *)loc, 0) == 0)
        ;
    while (CdRead2(STREAM_MODE) == 0)
        ;
}

/* Flips the display buffer index. */
ADDRESS(0x80061878, 0x1c)
void func_80061878(void)
{
    DAT_80199154.frame = DAT_80199154.frame ? 0 : 1;
}

/* Shows the current display buffer. */
ADDRESS(0x80061894, 0x84)
void func_80061894(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&DAT_80199154.disp[DAT_80199154.frame]);
    PutDrawEnv(&DAT_80199154.draw[DAT_80199154.frame]);
    SetDispMask(1);
}

/* No movie. */
ADDRESS(0x80061918, 0x28)
void func_80061918(void)
{
    DAT_801e7edc.unk372 = -1;
    DAT_801e7edc.unk370 = 0;
    DAT_801e7edc.unk384 = 0xff;
}
