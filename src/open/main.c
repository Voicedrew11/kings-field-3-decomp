/*
 * OPEN.EXE: the boot logo, attract movies and title screen. main plays the
 * logo movie (skipped when PSX.EXE's mailbox byte holds 1), loads OP.D (the
 * title pictures and sound bank), fades the title in over the streamed
 * \OP\M2.S music and lets the player pick NEW GAME or CONTINUE (CONTINUE
 * starts selected when the memory card holds a save). Idle time runs the
 * attract movies. The choice goes to 0x800102fa, a new game plays the
 * intro movie, and OPEN returns to PSX.EXE with 1 (GAME.EXE) as the next
 * program.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include "open.h"

/* PSX.EXE's next-program byte and the mailbox bytes after it (see src/psx/main.c). */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800102f0)
#define MAILBOX_BYTE_8 ((u8 *)0x800102f8)
#define MAILBOX_BYTE_A ((u8 *)0x800102fa)
/* PSX.EXE's program table index of GAME.EXE. */
#define PROGRAM_GAME 1

/* The heap: 0x80100000 up to 0x801f8000, below the 0x801ffff0 stack. */
#define HEAP_BASE ((u_long *)0x80100000)
#define HEAP_SIZE 0xf8000

/* OP.D: TIM pictures, then the sound bank header and body. */
#define TITLE_DATA_SIZE 0x32000
#define TITLE_VH_OFFSET 0x1f240
#define TITLE_VH_SIZE 0xc20 /* the sound bank header ahead of its body */

/* Frames the title waits for a choice before the logo movie runs again. */
#define TITLE_TIMEOUT 376

static void fade_out_title(int selection);
static void load_title_data(void);
static void init_graphics(void);
static void init_sound(void);
static void load_title_assets(void);
static void start_title_music(void);
static int draw_title_logo(int mode);
static int draw_press_start(int mode);
static int draw_menu(int mode);
static int read_menu_pad(int *selection, int saves);
static void begin_sprite(void);
static void add_sprite(int depth);
static int load_file(u_long *addr, char *name);
static void load_tims(u_long *addr);
static void play_sound(short vab, short prog, short tone, short note, short voll, short volr);

RODATA(0x80011000, 0x13)
SDATA(0x80040414, 0xb)

DATA(0x800403f4, 0x4, ".sdata")
u8 *next_program = NEXT_PROGRAM_BYTE;
DATA(0x800403f8, 0x4, ".sdata")
int logo_level = 0;
DATA(0x800403fc, 0x4, ".sdata")
int logo_state = 0;
DATA(0x80040400, 0x4, ".sdata")
int press_level = 0;
DATA(0x80040404, 0x4, ".sdata")
int press_state = 0;
DATA(0x80040408, 0x4, ".sdata")
int menu_level = 0;
DATA(0x8004040c, 0x4, ".sdata")
int menu_state = 0;
DATA(0x80040410, 0x4, ".sdata")
u_long old_pad = 0;

/*
 * Exported COMMON: PSYLINK places these after every fixed .sbss/.bss
 * contribution in symbol-hash bucket order, so their WIP spellings keep the
 * retail order.
 */
DATA(0x80040488, 0x2, ".sbss")
short vab_id;
DATA(0x8004048c, 0x4, ".sbss")
u_char *vh_addr;
DATA(0x80040490, 0x4, ".sbss")
DB *cdb;
DATA(0x80040494, 0x4, ".sbss")
u_long *data_buf;
DATA(0x80040498, 0x1, ".sbss")
u_char fade;
DATA(0x8004049c, 0x4, ".sbss")
POLY_FT4 *prim_next;
DATA(0x800a6f00, 0x8e8, ".bss")
DB db[2];
DATA(0x800a7804, 0x2000, ".bss")
u_long prim_buffer[2][0x400];

ADDRESS(0x80011e14, 0x5b8)
int main(void)
{
    int logo_done = 0;
    int frames = 0;
    u8 *mailbox = MAILBOX_BYTE_8;
    u8 *choice = MAILBOX_BYTE_A;
    int loaded = 0;
    int attract = 0;
    int pressed;
    int menu_done;
    int i;
    struct DIRENTRY dir[15];
    int saves;
    int selection = 0;

    ResetCallback();
    InitHeap(HEAP_BASE, HEAP_SIZE);
    CdInit();
    PadInit(0);
    ExitCriticalSection();
    card_init();
    init_sound();
    init_graphics();
    cdb = &db[0];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    cdb = &db[1];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    if (*mailbox != 1) {
        if (play_movie(MOVIE_BOOT_LOGO) == 1) {
            attract = 1;
        } else {
        attract_movie:
            if (attract == 0) {
                play_movie(MOVIE_ATTRACT_A);
                attract = 1;
            } else {
                play_movie(MOVIE_ATTRACT_B);
                attract = 0;
            }
        }
    }
title:
    if (!loaded) {
        loaded = 1;
        load_title_data();
        load_title_assets();
    }
    selection = 0;
    card_start();
    SsSetMVol(127, 127);
    start_title_music();
    for (;;) {
        swap_buffers();
        frames++;
        if (!logo_done) {
            if (draw_title_logo(0) == 1)
                logo_done = 1;
        } else {
            draw_title_logo(1);
        }
        if (logo_done == 1) {
            pressed = draw_press_start(0);
            menu_done = draw_menu(2);
            if (pressed == 1 && menu_done == 1)
                break;
        }
        if (PadRead(1))
            break;
        flip_buffers();
    }
    if (card_check_writable() == 0)
        card_list_saves(dir, &saves);
    else
        saves = 0;
    card_stop();
    if (saves)
        selection = 1;
    while (PadRead(1))
        ;
    for (;;) {
        swap_buffers();
        frames++;
        draw_title_logo(1);
        draw_press_start(1);
        if (read_menu_pad(&selection, saves) == 1) {
            *choice = selection;
            fade_out_title(selection);
            break;
        }
        if (frames >= TITLE_TIMEOUT) {
            logo_done = 0;
            fade_out_title(selection);
            frames = 0;
            draw_title_logo(2);
            draw_press_start(2);
            draw_menu(3);
            if (play_movie(MOVIE_LOGO) == 1)
                goto title;
            goto attract_movie;
        }
        draw_menu(selection);
        flip_buffers();
    }
    if (*choice == 0) {
        i = 30;
        while (i--)
            VSync(0);
        play_movie(MOVIE_NEW_GAME);
    }
    *next_program = PROGRAM_GAME;
    *mailbox = 0;
    SetDefDrawEnv(&db[0].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&db[1].draw, 0, 240, 320, 240);
    SetDefDispEnv(&db[0].disp, 0, 240, 320, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, 320, 240);
    db[0].draw.isbg = db[1].draw.isbg = 1;
    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    i = 0;
    do {
        swap_buffers();
        i++;
        begin_sprite();
        prim_next->tpage = GetTPage(0, 1, 960, 0);
        prim_next->clut = getClut(960, 200);
        setXYWH(prim_next, 100, 30, 120, 120);
        setUVWH(prim_next, 0, 0, 120, 120);
        add_sprite(50);
        begin_sprite();
        prim_next->tpage = GetTPage(0, 1, 960, 0);
        prim_next->clut = getClut(960, 201);
        setXYWH(prim_next, 80, 160, 160, 24);
        setUVWH(prim_next, 0, 120, 160, 24);
        add_sprite(50);
        flip_buffers();
    } while (i < 2);
    card_close();
    SsSetMVol(0, 0);
    SsEnd();
    PadStop();
    ResetGraph(1);
}

/* Fades the title out over 48 frames, the music with it, and pauses the CD. */
ADDRESS(0x800123cc, 0xc8)
static void fade_out_title(int selection)
{
    int level = 0;
    int vol = 0;

    do {
        swap_buffers();
        fade = level;
        draw_title_logo(1);
        draw_press_start(1);
        draw_menu(selection);
        flip_buffers();
        if (level == 32)
            selection = 4;
        if (vol < 127)
            SsSetSerialVol(SS_SERIAL_A, 127 - vol, 127 - vol);
        level += 2;
        vol += 6;
    } while (level < 96);
    SsSetSerialVol(SS_SERIAL_A, 0, 0);
    CdControlB(CdlPause, 0, 0);
}

ADDRESS(0x80012494, 0x6c)
static void load_title_data(void)
{
    char name[10] = "OP.D";

    data_buf = (u_long *)malloc(TITLE_DATA_SIZE);
    while (load_file(data_buf, name))
        ;
}

ADDRESS(0x80012500, 0x144)
static void init_graphics(void)
{
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();
    SetGeomOffset(320, 120);
    SetGeomScreen(320);
    SetBackColor(0, 0, 0);
    SetDefDrawEnv(&db[0].draw, 0, 0, 640, 240);
    SetDefDrawEnv(&db[1].draw, 0, 240, 640, 240);
    SetDefDispEnv(&db[0].disp, 0, 240, 640, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, 640, 240);
    db[0].draw.dtd = db[1].draw.dtd = 1;
    db[0].draw.isbg = db[1].draw.isbg = 1;
    setRGB0(&db[0].draw, 0, 0, 0);
    setRGB0(&db[1].draw, 0, 0, 0);
    SetDispMask(1);
    db[0].prim = prim_buffer[0];
    db[1].prim = prim_buffer[1];
}

ADDRESS(0x80012644, 0x68)
static void init_sound(void)
{
    SsInit();
    SsSetTickMode(SS_NOTICK);
    SsUtSetReverbType(SS_REV_TYPE_HALL);
    SsSetMVol(0, 0);
    SsUtReverbOn();
    SsUtSetReverbDepth(48, 48);
    SsStart2();
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
}

ADDRESS(0x800126ac, 0x90)
static void load_title_assets(void)
{
    u_char *p = (u_char *)data_buf;

    load_tims((u_long *)p);
    p += TITLE_VH_OFFSET;
    vh_addr = p;
    p += TITLE_VH_SIZE;
    vab_id = SsVabOpenHead(vh_addr, -1);
    if (vab_id != -1 && SsVabTransBody(p, vab_id) == vab_id)
        SsVabTransCompleted(SS_WAIT_COMPLETED);
    DrawSync(0);
}

ADDRESS(0x8001273c, 0x60)
static void start_title_music(void)
{
    CdlFILE file;

    SsSetSerialVol(SS_SERIAL_A, 127, 127);
    CdSearchFile(&file, "\\OP\\M2.S;1");
    while (CdControl(CdlSeekL, (u_char *)&file.pos, 0) == 0)
        ;
    while (CdRead2(CdlModeSpeed | CdlModeRT) == 0)
        ;
}

/* The title picture and logo: mode 0 fades in, 1 holds, 2 resets. */
ADDRESS(0x8001279c, 0x370)
static int draw_title_logo(int mode)
{
    int done = 0;

    if (mode == 1) {
        logo_state = mode;
        logo_level = 96;
    } else if (mode == 2) {
        logo_level = 0;
        logo_state = 0;
        return 0;
    }
    begin_sprite();
    setRGB0(prim_next, 64, 64, 64);
    prim_next->tpage = GetTPage(1, 0, 640, 0);
    prim_next->clut = getClut(0, 480);
    setXYWH(prim_next, 0, 0, 640, 240);
    setUVWH(prim_next, 0, 0, 255, 240);
    add_sprite(200);
    if (logo_state == 0) {
        logo_level += 4;
        if (logo_level >= 144)
            logo_state = 1;
    } else if (logo_level > 96) {
        logo_level -= 4;
    } else {
        done = 1;
    }
    begin_sprite();
    prim_next->tpage = GetTPage(1, 1, 640, 256);
    prim_next->clut = getClut(0, 481);
    SetSemiTrans((void *)prim_next, 1);
    setRGB0(prim_next, logo_level, logo_level, logo_level);
    setXYWH(prim_next, 64, 44, 256, 89);
    setUVWH(prim_next, 0, 0, 255, 89);
    add_sprite(100);
    begin_sprite();
    prim_next->tpage = GetTPage(1, 1, 640, 256);
    prim_next->clut = getClut(0, 482);
    SetSemiTrans((void *)prim_next, 1);
    setRGB0(prim_next, logo_level, logo_level, logo_level);
    setXYWH(prim_next, 320, 44, 256, 89);
    setUVWH(prim_next, 0, 128, 255, 89);
    add_sprite(100);
    return done;
}

/* PRESS START BUTTON: mode 0 fades in, 1 holds, 2 resets. */
ADDRESS(0x80012b0c, 0x394)
static int draw_press_start(int mode)
{
    int done = 0;

    if (mode == 1) {
        press_state = mode;
        press_level = 96;
    } else if (mode == 2) {
        press_level = 0;
        press_state = 0;
        return 0;
    }
    if (press_state == 0) {
        press_level += 4;
        if (press_level >= 128)
            press_state = 1;
    } else if (press_level > 96) {
        press_level -= 4;
    } else {
        done = 1;
    }
    begin_sprite();
    prim_next->tpage = GetTPage(0, 1, 896, 0);
    prim_next->clut = getClut(0, 500);
    SetSemiTrans((void *)prim_next, 1);
    setRGB0(prim_next, press_level, press_level, press_level);
    setXYWH(prim_next, 124, 154, 255, 12);
    setUVWH(prim_next, 0, 0, 255, 12);
    add_sprite(100);
    begin_sprite();
    prim_next->tpage = GetTPage(0, 1, 896, 0);
    prim_next->clut = getClut(0, 500);
    SetSemiTrans((void *)prim_next, 1);
    setRGB0(prim_next, press_level, press_level, press_level);
    setXYWH(prim_next, 124, 168, 255, 12);
    setUVWH(prim_next, 0, 24, 255, 12);
    add_sprite(100);
    begin_sprite();
    prim_next->tpage = GetTPage(0, 1, 896, 0);
    prim_next->clut = getClut(0, 500);
    SetSemiTrans((void *)prim_next, 1);
    setRGB0(prim_next, press_level, press_level, press_level);
    setXYWH(prim_next, 380, 168, 144, 12);
    setUVWH(prim_next, 0, 40, 144, 12);
    add_sprite(100);
    return done;
}

/*
 * NEW GAME / CONTINUE: mode 0 or 1 pulses that item and dims the other,
 * 2 pulses both, 3 resets, 4 holds both at the dim level.
 */
ADDRESS(0x80012ea0, 0x30c)
static int draw_menu(int mode)
{
    int done = 0;

    if (mode == 3) {
        menu_level = 0;
        menu_state = 0;
        return 0;
    }
    if (mode == 4) {
        menu_level = 56;
    } else if (menu_state == 0) {
        menu_level += 4;
        if (menu_level >= 128)
            menu_state = 1;
    } else if (menu_level > 56) {
        menu_level -= 4;
    } else {
        menu_state = 0;
        done = 1;
    }
    begin_sprite();
    prim_next->tpage = GetTPage(0, 1, 896, 0);
    prim_next->clut = getClut(0, 500);
    SetSemiTrans((void *)prim_next, 1);
    if (mode != 1) {
        setRGB0(prim_next, menu_level, menu_level, menu_level);
    } else {
        setRGB0(prim_next, 56, 56, 56);
    }
    setXYWH(prim_next, 272, 190, 96, 8);
    setUVWH(prim_next, 0, 16, 96, 8);
    add_sprite(100);
    begin_sprite();
    prim_next->tpage = GetTPage(0, 1, 896, 0);
    prim_next->clut = getClut(0, 500);
    SetSemiTrans((void *)prim_next, 1);
    if (mode != 0) {
        setRGB0(prim_next, menu_level, menu_level, menu_level);
    } else {
        setRGB0(prim_next, 56, 56, 56);
    }
    setXYWH(prim_next, 272, 202, 96, 8);
    setUVWH(prim_next, 96, 16, 96, 8);
    add_sprite(100);
    return done;
}

/* 1 when circle, cross or START is newly pressed; up/down toggle the choice. */
ADDRESS(0x800131ac, 0x148)
static int read_menu_pad(int *selection, int saves)
{
    int chosen = 0;
    u_long pad;

    pad = PadRead(1);
    if ((pad & PADRright && !(old_pad & PADRright))
        || (pad & PADRdown && !(old_pad & PADRdown))
        || (pad & PADstart && !(old_pad & PADstart))) {
        play_sound(vab_id, 0, 0, 58, 112, 112);
        chosen = 1;
    }
    if ((pad & PADLup && !(old_pad & PADLup)) || (pad & PADLdown && !(old_pad & PADLdown))) {
        play_sound(vab_id, 0, 0, 90, 80, 80);
        if (saves)
            *selection = *selection == 0;
    }
    old_pad = pad;
    return chosen;
}

ADDRESS(0x800132f4, 0x48)
static void begin_sprite(void)
{
    SetPolyFT4(prim_next);
    setRGB0(prim_next, 96, 96, 96);
}

/* Darkens the sprite by the fade level, links it into the table and steps on. */
ADDRESS(0x8001333c, 0x8c)
static void add_sprite(int depth)
{
    u_char c;

    if (fade) {
        c = prim_next->r0;
        if (c < fade)
            c = 0;
        else
            c -= fade;
        setRGB0(prim_next, c, c, c);
    }
    AddPrim((void *)&cdb->ot[depth], (void *)prim_next);
    prim_next++;
}

ADDRESS(0x800133c8, 0x50)
void swap_buffers(void)
{
    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    ClearOTagR(cdb->ot, OT_LENGTH);
    fade = 0;
    prim_next = (POLY_FT4 *)cdb->prim;
}

ADDRESS(0x80013418, 0x4c)
void flip_buffers(void)
{
    DrawSync(0);
    VSync(2);
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    DrawOTag(&cdb->ot[OT_LENGTH - 1]);
}

/* Reads \OP\<name>;1 whole into addr: 0 when read, 1 if absent, 3 on a read error. */
ADDRESS(0x80013464, 0x158)
static int load_file(u_long *addr, char *name)
{
    char path[40];
    CdlFILE file;
    CdlLOC start;
    u_char mode;
    int sectors;

    start.minute = 0;
    start.second = 2;
    start.sector = 0;
    start.track = 0;
    strcpy(path, "\\OP\\");
    strcat(path, name);
    strcat(path, ";1");
    if (CdSearchFile(&file, path) == 0)
        return 1;
    mode = CdlModeSpeed;
    sectors = (file.size + 2047) / 2048;
    CdControl(CdlSetmode, &mode, 0);
    CdControl(CdlReadN, (u_char *)&file.pos, 0);
    while (sectors--) {
        if (CdReady(0, 0) != CdlDataReady) {
            CdControl(CdlSetloc, (u_char *)&start, 0);
            CdControl(CdlPause, 0, 0);
            return 3;
        }
        CdGetSector((void *)addr, 512);
        addr += 512;
    }
    CdControl(CdlSetloc, (u_char *)&start, 0);
    CdControl(CdlPause, 0, 0);
    return 0;
}

ADDRESS(0x800135bc, 0x70)
static void load_tims(u_long *addr)
{
    TIM_IMAGE tim;

    OpenTIM(addr);
    while (ReadTIM(&tim)) {
        if (tim.caddr)
            LoadImage(tim.crect, tim.caddr);
        if (tim.paddr)
            LoadImage(tim.prect, tim.paddr);
    }
}

ADDRESS(0x8001362c, 0x94)
static void play_sound(short vab, short prog, short tone, short note, short voll, short volr)
{
    if (prog || tone || note) {
        SsUtKeyOn(vab, prog, tone, note, 0, voll, volr);
        SsSeqCalledTbyT();
    }
}
