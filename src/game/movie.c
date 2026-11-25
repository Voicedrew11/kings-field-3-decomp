/*
 * GAME.EXE: the in-game movie player's Sony movie-sample stream functions
 * (WIP module; src/open/movie.c and src/end/movie.c are the same sample): the
 * decode environment lives in movie_state.dec, the ring buffer is
 * stream_work_buffer, and the display swaps through scene_display_sys. The decode
 * set-up strSetDefDecEnv (not exact) leads; the player movie_play_stream precedes
 * it. movie_update_state steps the movie state; its jump table is this unit's .rdata
 * (after movie_play_stream's file names, so the TU starts at or before it).
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
 * Plays movie movie_state.current_movie: releases the area's model blocks, takes
 * three arena blocks for the VLC and slice buffers, shrinks the packet pools
 * to 0x7400 bytes and streams the movie's STR file (a random DRM file for
 * file numbers from 16) until its last frame, fading the music back in.
 */
ADDRESS(0x80060d20, 0x644)
void movie_play_stream(void)
{
    CdlFILE file;
    u_char param[8];
    char name[64];
    s32 i;
    s32 j;
    u8 *p;
    s32 vol; /* never initialized: retail ramps whatever s4 held */
    s32 n;

    cd_request_wait_idle();
    player_clear_motion();
    for (i = 127, j = 128; i != -1; i--, j++) {
        if ((p = scene_display_sys.models[j]) != NULL) {
            while (memory_block_kind(p) == 3)
                ;
            memory_block_set_kind(p, 1);
        }
    }
    for (i = 299, j = 256; i != -1; i--, j++) {
        if ((p = scene_display_sys.models[j]) != NULL) {
            while (memory_block_kind(p) == 3)
                ;
            memory_block_set_kind(p, 1);
        }
    }
    for (i = 31, j = 556 + asset_status.active_resource_ids[0] * 32; i != -1; i--, j++) {
        if ((p = scene_display_sys.models[j]) != NULL) {
            while (memory_block_kind(p) == 3)
                ;
            memory_block_set_kind(p, 1);
        }
    }
    DrawSync(0);
    scene_display_sys.pools[1].base = scene_display_sys.pools[0].end = scene_display_sys.pools[0].base
        + 0x7400;
    scene_display_sys.pools[1].end = scene_display_sys.pools[1].base + 0x7400;
    if ((p = scene_display_sys.models[0x629]) == NULL) {
        if ((p = memory_arena_allocate_block((HeapBlock *)resource_space, 0x10000,
                                             &scene_display_sys.models[0x629])) == NULL) {
            return;
        }
        memory_block_set_kind(p, 2);
        memory_block_set_tag(p, 0x629);
    } else if (memory_block_kind(p) == 1) {
        memory_block_set_kind(p, 2);
    }
    movie_state.vlc_buffers[0] = (u_long *)p;
    if ((p = scene_display_sys.models[0x62a]) == NULL) {
        if ((p = memory_arena_allocate_block((HeapBlock *)resource_space, 0x10000,
                                             &scene_display_sys.models[0x62a])) == NULL) {
            return;
        }
        memory_block_set_kind(p, 2);
        memory_block_set_tag(p, 0x62a);
    } else if (memory_block_kind(p) == 1) {
        memory_block_set_kind(p, 2);
    }
    movie_state.vlc_buffers[1] = (u_long *)p;
    if ((p = scene_display_sys.models[0x62b]) == NULL) {
        if ((p = memory_arena_allocate_block((HeapBlock *)resource_space, 0x3c00,
                                             &scene_display_sys.models[0x62b])) == NULL) {
            return;
        }
        memory_block_set_kind(p, 2);
        memory_block_set_tag(p, 0x62b);
    } else if (memory_block_kind(p) == 1) {
        memory_block_set_kind(p, 2);
    }
    movie_state.slice_buffer = (u_long *)p;
    if (movie_state.current_movie->file < 16) {
        strcpy(name, "\\STR\\SXX.S;1");
        name[6] = movie_state.current_movie->file / 10 + '0';
        name[7] = movie_state.current_movie->file % 10 + '0';
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
    SsSeqSetVol(sound_status.seq_id, 0, 0);
    SsSetSerialVol(SS_SERIAL_A, cur_avatar_status.bgm_volume, cur_avatar_status.bgm_volume);
    strSetDefDecEnv();
    strInit(&file.pos);
    strNextVlc();
    for (;;) {
        movie_swap_display_buffer();
        DecDCTin(movie_state.dec.vlcbuf[movie_state.dec.vlcid], 2);
        DrawSync(0);
        DecDCTout(movie_state.dec.imgbuf, movie_state.dec.slice.w * movie_state.dec.slice.h / 2);
        strNextVlc();
        strSync(&movie_state.dec);
        movie_present_frame();
        if (movie_state.stream_status == 99) {
            break;
        }
        if (movie_state.current_movie->last_frame < movie_state.stream_frame) {
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
    SsSeqSetVol(sound_status.seq_id, cur_avatar_status.bgm_volume, cur_avatar_status.bgm_volume);
    param[0] = CdlModeSpeed;
    CdControlB(CdlSetmode, param, 0);
    EnterCriticalSection();
    DecDCToutCallback(0);
    StUnSetRing();
    CdDataCallback(0);
    CdReadyCallback(0);
    ExitCriticalSection();
    CdControlB(CdlPause, 0, 0);
    memory_block_set_kind(scene_display_sys.models[0x629], 1);
    memory_block_set_kind(scene_display_sys.models[0x62a], 1);
    memory_block_set_kind(scene_display_sys.models[0x62b], 1);
    scene_display_sys.pools[1].base = scene_display_sys.pools[0].end = scene_display_sys.pools[0].base
        + 0x1a000;
    scene_display_sys.pools[1].end = scene_display_sys.pools[1].base + 0x1a000;
}

/* Sets up the decode environment for a 320x240 movie drawn into the display buffer not shown. */
ADDRESS(0x80061364, 0x170)
void strSetDefDecEnv(void)
{
    u_long **buf = movie_state.vlc_buffers;
    DECENV *dec = &movie_state.dec;

    dec->vlcbuf[0] = buf[0];
    dec->vlcbuf[1] = buf[1];
    dec->vlcid = 0;
    dec->imgbuf = movie_state.slice_buffer;
    dec->rectid = 0;
    dec->isdone = 0;
    setRECT(&dec->rect[scene_display_sys.frame & 1], 0, 0, 320, 240);
    setRECT(&dec->rect[(scene_display_sys.frame + 1) & 1], 0, 240, 320, 240);
    if (scene_display_sys.frame) {
        setRECT(&movie_state.dec.slice, 0, 240, 16, 240);
    } else {
        setRECT(&movie_state.dec.slice, 0, 0, 16, 240);
    }
}

ADDRESS(0x800614d4, 0x7c)
void strInit(CdlLOC *loc)
{
    movie_state.stream_status = 0;
    EnterCriticalSection();
    DecDCToutCallback(strCallback);
    StSetRing((u_long *)stream_work_buffer, RING_SIZE);
    StSetStream(0, 1, 0x0fffffff, 0, 0);
    ExitCriticalSection();
    strKickCD(loc);
}

/* MDEC output callback: loads the slice and decodes the next one or flips rect. */
ADDRESS(0x80061550, 0x100)
void strCallback(void)
{
    DECENV *dec = &movie_state.dec;
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
s32 strNextVlc(void)
{
    int cnt = WAIT_TIME;
    DECENV *dec = &movie_state.dec;
    u_long *next;

    while ((next = strNext(dec)) == 0) {
        if (--cnt == 0) {
            return -1;
        }
    }
    dec->vlcid = dec->vlcid ? 0 : 1;
    DecDCTvlc(next, dec->vlcbuf[dec->vlcid]);
    StFreeRing(next);
    return 0;
}

/* Next movie sector; sets stream_status to 99 past the movie's last frame. */
ADDRESS(0x800616dc, 0xc0)
u_long *strNext(DECENV *dec)
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
    movie_state.stream_frame = sector->frameCount;
    if (movie_state.current_movie->last_frame < sector->frameCount) {
        movie_state.stream_status = 99;
    }
    return addr;
}

ADDRESS(0x8006179c, 0x90)
void strSync(DECENV *dec)
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
void strKickCD(CdlLOC *loc)
{
    while (CdControl(CdlSeekL, (u_char *)loc, 0) == 0)
        ;
    while (CdRead2(STREAM_MODE) == 0)
        ;
}

/* Flips the display buffer index. */
ADDRESS(0x80061878, 0x1c)
void movie_swap_display_buffer(void)
{
    scene_display_sys.frame = scene_display_sys.frame ? 0 : 1;
}

/* Shows the current display buffer. */
ADDRESS(0x80061894, 0x84)
void movie_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&scene_display_sys.disp[scene_display_sys.frame]);
    PutDrawEnv(&scene_display_sys.draw[scene_display_sys.frame]);
    SetDispMask(1);
}

/* No movie. */
ADDRESS(0x80061918, 0x28)
void movie_state_reset(void)
{
    movie_state.stream_status = -1;
    movie_state.state = 0;
    movie_state.movie_id = 0xff;
}

/* Steps the in-game movie state (state) of movie movie_id. */
ADDRESS(0x80061940, 0x284)
void movie_update_state(void)
{
    u8 id;
    u8 flag;

    if (movie_state.movie_id == 0xff) {
        return;
    }
    id = movie_state.movie_id;
    movie_state.current_movie = &movie_state.movies[id];
    switch (movie_state.state) {
    case 0:
        break;
    case 1:
        if (movie_state.current_movie->skip_flag != 0xff) {
            flag = movie_state.current_movie->skip_flag;
            if (flag & 0x80) {
                if (cur_evt_state.flags[flag & 0x7f] == 0) {
                    movie_state.state = 0x10;
                    break;
                }
            } else if (cur_evt_state.flags[flag & 0x7f] != 0) {
                movie_state.state = 0x10;
                break;
            }
        }
        movie_state.state = 2;
        player_begin_movie_approach(id);
        break;
    case 2:
        if (movie_state.stream_status == 1) {
            movie_state.state = 4;
        }
        movie_state.stream_status = 0;
        break;
    case 4:
        ((void (*)(u8))asset_status.handlers[FDAT_SLOT_ON_MOVIE_START])(id);
        movie_play_stream();
        if (cur_evt_state.unk61 == 1) {
            cur_evt_state.unk61 = 2;
            movie_state.stream_status = 1;
            movie_state.state = 0;
            movie_state.movie_id = 0xff;
            player_reset_reaction_state();
            MoveImage(&scene_display_sys.disp[scene_display_sys.frame].disp,
                      scene_display_sys.draw[scene_display_sys.frame].clip.x,
                      scene_display_sys.draw[scene_display_sys.frame].clip.y);
            DrawSync(0);
            player_warp_to_destination(12);
        }
        movie_state.state = 8;
        player_begin_movie_return(id);
        break;
    case 8:
        if (movie_state.stream_status == 1) {
            movie_state.state = 0;
            ((void (*)(u8))asset_status.handlers[FDAT_SLOT_ON_MOVIE_END])(id);
        }
        movie_state.stream_status = 0;
        break;
    case 16:
        movie_state.state = 1;
        break;
    }
}
