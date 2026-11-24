/*
 * GAME.EXE: the in-game movie player's Sony movie-sample stream functions
 * (WIP module; src/open/movie.c and src/end/movie.c are the same sample): the
 * decode environment lives in DAT_801e7edc.dec, the ring buffer is
 * DAT_801c17a4, and the display swaps through DAT_80199154. The decode
 * set-up func_80061364 (not exact) leads; the player func_80060d20 precedes
 * it. func_80061940 steps the movie state; its jump table is this unit's .rdata
 * (after func_80060d20's file names, so the TU starts at or before it).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <psyq/press.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/render.h>
#include <game/misc.h>
#include <game/player.h>
#include <game/memory.h>
#include <game/cd.h>

RODATA(0x80013680, 0x64)

/* Polling bound of the stream waits. */
#define WAIT_TIME 0x800000
/* Ring buffer length in sectors. */
#define RING_SIZE 32
/* CdRead2 mode: CdlModeStream | CdlModeSpeed | CdlModeRT as the 3.4 library spells it. */
#define STREAM_MODE (0x100 | CdlModeSpeed | CdlModeRT)

/*
 * Plays movie DAT_801e7edc.unk380: releases the area's model blocks, takes
 * three arena blocks for the VLC and slice buffers, shrinks the packet pools
 * to 0x7400 bytes and streams the movie's STR file (a random DRM file for
 * file numbers from 16) until its last frame, fading the music back in.
 */
ADDRESS(0x80060d20, 0x644)
void func_80060d20(void)
{
    CdlFILE file;
    u_char param[8];
    char name[64];
    s32 i;
    s32 j;
    u8 *p;
    s32 vol; /* never initialized: retail ramps whatever s4 held */
    s32 n;

    func_80019ab4();
    func_8002b6f0();
    for (i = 127, j = 128; i != -1; i--, j++) {
        if ((p = DAT_80199154.models[j]) != NULL) {
            while (func_8001947c(p) == 3)
                ;
            func_80019474(p, 1);
        }
    }
    for (i = 299, j = 256; i != -1; i--, j++) {
        if ((p = DAT_80199154.models[j]) != NULL) {
            while (func_8001947c(p) == 3)
                ;
            func_80019474(p, 1);
        }
    }
    for (i = 31, j = 556 + DAT_8018fad4.area[0] * 32; i != -1; i--, j++) {
        if ((p = DAT_80199154.models[j]) != NULL) {
            while (func_8001947c(p) == 3)
                ;
            func_80019474(p, 1);
        }
    }
    DrawSync(0);
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x7400;
    DAT_80199154.pools[1].end = DAT_80199154.pools[1].base + 0x7400;
    if ((p = DAT_80199154.models[0x629]) == NULL) {
        if ((p = func_8001939c((HeapBlock *)DAT_800c8954, 0x10000, &DAT_80199154.models[0x629])) == NULL) {
            return;
        }
        func_80019474(p, 2);
        func_8001949c(p, 0x629);
    } else if (func_8001947c(p) == 1) {
        func_80019474(p, 2);
    }
    DAT_801e7edc.unk374[0] = (u_long *)p;
    if ((p = DAT_80199154.models[0x62a]) == NULL) {
        if ((p = func_8001939c((HeapBlock *)DAT_800c8954, 0x10000, &DAT_80199154.models[0x62a])) == NULL) {
            return;
        }
        func_80019474(p, 2);
        func_8001949c(p, 0x62a);
    } else if (func_8001947c(p) == 1) {
        func_80019474(p, 2);
    }
    DAT_801e7edc.unk374[1] = (u_long *)p;
    if ((p = DAT_80199154.models[0x62b]) == NULL) {
        if ((p = func_8001939c((HeapBlock *)DAT_800c8954, 0x3c00, &DAT_80199154.models[0x62b])) == NULL) {
            return;
        }
        func_80019474(p, 2);
        func_8001949c(p, 0x62b);
    } else if (func_8001947c(p) == 1) {
        func_80019474(p, 2);
    }
    DAT_801e7edc.unk37c = (u_long *)p;
    if (DAT_801e7edc.unk380->file < 16) {
        strcpy(name, "\\STR\\SXX.S;1");
        name[6] = DAT_801e7edc.unk380->file / 10 + '0';
        name[7] = DAT_801e7edc.unk380->file % 10 + '0';
        CdSearchFile(&file, name);
    } else {
        n = rand() * 18 >> 15;
        if (n >= 18) {
            n = 17;
        }
        strcpy(name, "\\DRM\\DXX.S;1");
        name[6] = n / 10 + '0';
        name[7] = n % 10 + '0';
        CdSearchFile(&file, name);
    }
    SsSetMVol(0, 0);
    SsSeqSetVol(DAT_801b09fc.seq_id, 0, 0);
    SsSetSerialVol(SS_SERIAL_A, DAT_801b24e4.bgm_volume, DAT_801b24e4.bgm_volume);
    func_80061364();
    func_800614d4(&file.pos);
    func_80061650();
    for (;;) {
        func_80061878();
        DecDCTin(DAT_801e7edc.dec.vlcbuf[DAT_801e7edc.dec.vlcid], 2);
        DrawSync(0);
        DecDCTout(DAT_801e7edc.dec.imgbuf, DAT_801e7edc.dec.slice.w * DAT_801e7edc.dec.slice.h / 2);
        func_80061650();
        func_8006179c(&DAT_801e7edc.dec);
        func_80061894();
        if (DAT_801e7edc.unk372 == 99) {
            break;
        }
        if (DAT_801e7edc.unk380->last < DAT_801e7edc.unk388) {
            break;
        }
        if (vol < 127) {
            SsSetMVol(vol, vol);
            vol += 8;
        } else if (vol != 127) {
            vol = 127;
            SsSetMVol(127, 127);
        }
    }
    DrawSync(0);
    SsSetMVol(0, 0);
    SsSetSerialVol(SS_SERIAL_A, 0, 0);
    SsSeqSetVol(DAT_801b09fc.seq_id, DAT_801b24e4.bgm_volume, DAT_801b24e4.bgm_volume);
    param[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, param, 0);
    EnterCriticalSection();
    DecDCToutCallback(0);
    StUnSetRing();
    CdDataCallback(0);
    CdReadyCallback(0);
    ExitCriticalSection();
    CdControlB(CdlPause, 0, 0);
    func_80019474(DAT_80199154.models[0x629], 1);
    func_80019474(DAT_80199154.models[0x62a], 1);
    func_80019474(DAT_80199154.models[0x62b], 1);
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x1a000;
    DAT_80199154.pools[1].end = DAT_80199154.pools[1].base + 0x1a000;
}

/* Sets up the decode environment for a 320x240 movie drawn into the display buffer not shown. */
ADDRESS(0x80061364, 0x170)
void func_80061364(void)
{
    DECENV *dec = &DAT_801e7edc.dec;

    dec->vlcbuf[0] = DAT_801e7edc.unk374[0];
    dec->vlcbuf[1] = DAT_801e7edc.unk374[1];
    dec->vlcid = 0;
    dec->imgbuf = DAT_801e7edc.unk37c;
    dec->rectid = 0;
    dec->isdone = 0;
    setRECT(&dec->rect[DAT_80199154.frame & 1], 0, 0, 320, 240);
    setRECT(&dec->rect[(DAT_80199154.frame + 1) & 1], 0, 240, 320, 240);
    if (DAT_80199154.frame) {
        setRECT(&DAT_801e7edc.dec.slice, 0, 240, 16, 240);
    } else {
        setRECT(&DAT_801e7edc.dec.slice, 0, 0, 16, 240);
    }
}

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
    if (DAT_801e7edc.unk380->last < sector->frameCount) {
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

/* Steps the in-game movie state (unk370) of movie unk384. */
ADDRESS(0x80061940, 0x284)
void func_80061940(void)
{
    u8 id;
    u8 flag;

    if (DAT_801e7edc.unk384 == 0xff) {
        return;
    }
    id = DAT_801e7edc.unk384;
    DAT_801e7edc.unk380 = &DAT_801e7edc.movies[id];
    switch (DAT_801e7edc.unk370) {
    case 0:
        break;
    case 1:
        if (DAT_801e7edc.unk380->flag != 0xff) {
            flag = DAT_801e7edc.unk380->flag;
            if (flag & 0x80) {
                if (DAT_801ba984.unk4[flag & 0x7f] == 0) {
                    DAT_801e7edc.unk370 = 0x10;
                    break;
                }
            } else if (DAT_801ba984.unk4[flag & 0x7f] != 0) {
                DAT_801e7edc.unk370 = 0x10;
                break;
            }
        }
        DAT_801e7edc.unk370 = 2;
        func_80030810(id);
        break;
    case 2:
        if (DAT_801e7edc.unk372 == 1) {
            DAT_801e7edc.unk370 = 4;
        }
        DAT_801e7edc.unk372 = 0;
        break;
    case 4:
        DAT_8018fad4.handlers[10](id);
        func_80060d20();
        if (DAT_801ba984.unk61 == 1) {
            DAT_801ba984.unk61 = 2;
            DAT_801e7edc.unk372 = 1;
            DAT_801e7edc.unk370 = 0;
            DAT_801e7edc.unk384 = 0xff;
            func_8003078c();
            MoveImage(&DAT_80199154.disp[DAT_80199154.frame].disp, DAT_80199154.draw[DAT_80199154.frame].clip.x,
                      DAT_80199154.draw[DAT_80199154.frame].clip.y);
            DrawSync(0);
            func_8005c864(12);
        }
        DAT_801e7edc.unk370 = 8;
        func_80030864(id);
        break;
    case 8:
        if (DAT_801e7edc.unk372 == 1) {
            DAT_801e7edc.unk370 = 0;
            DAT_8018fad4.handlers[7](id);
        }
        DAT_801e7edc.unk372 = 0;
        break;
    case 16:
        DAT_801e7edc.unk370 = 1;
        break;
    }
}
