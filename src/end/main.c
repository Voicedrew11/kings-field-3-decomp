/*
 * END.EXE: the ending. main sets up the display and sound, plays the closing
 * movies (src/end/movie.c) and stops, leaving 0 (OPEN.EXE) in PSX.EXE's
 * next-program byte. When the byte after it holds 2, two more movies and a
 * one-second pause come before the last one.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include "end.h"

/*
 * PSX.EXE's next-program byte and the mailbox byte at 0x800102f8 that
 * PSX.EXE clears at boot (see src/psx/main.c). Both are spelled as numbers:
 * main builds them with lui/ori, which a symbol reference would not get.
 */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800102f0)
#define MAILBOX_BYTE_8 ((u8 *)0x800102f8)
/* PSX.EXE's program table index of OPEN.EXE. */
#define PROGRAM_OPEN 0

/* The heap: 0x80100000 up to 0x801f8000, below the 0x801ffff0 stack. */
#define HEAP_BASE ((u_long *)0x80100000)
#define HEAP_SIZE 0xf8000

static void init_graphics(void);
static void init_sound(void);

DATA(0x800374d4, 0x4, ".sbss")
DB *cdb;
DATA(0x800374d8, 0x4, ".sbss")
u_long *prim_next;
DATA(0x8009df0c, 0x8e8, ".bss")
DB db[2];
DATA(0x8009e804, 0x2000, ".bss")
u_long prim_buffer[2][0x400];

ADDRESS(0x800119b8, 0x128)
int main(void)
{
    u8 *next_program = NEXT_PROGRAM_BYTE;
    u8 *mailbox = MAILBOX_BYTE_8;
    int i;

    ResetCallback();
    InitHeap(HEAP_BASE, HEAP_SIZE);
    CdInit();
    PadInit(0);
    ExitCriticalSection();
    init_sound();
    init_graphics();
    cdb = &db[0];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    cdb = &db[1];
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    if (*mailbox == 2) {
        play_movie(0);
        play_movie(1);
        i = 60;
        while (i--)
            VSync(0);
    }
    play_movie(2);
    *next_program = PROGRAM_OPEN;
    *mailbox = 0;
    PadStop();
    CdControl(CdlStop, 0, 0);
    for (;;)
        ;
}

ADDRESS(0x80011ae0, 0x144)
static void init_graphics(void)
{
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();
    SetGeomOffset(160, 120);
    SetGeomScreen(320);
    SetBackColor(0, 0, 0);
    SetDefDrawEnv(&db[0].draw, 0, 0, 320, 240);
    SetDefDrawEnv(&db[1].draw, 0, 240, 320, 240);
    SetDefDispEnv(&db[0].disp, 0, 240, 320, 240);
    SetDefDispEnv(&db[1].disp, 0, 0, 320, 240);
    db[0].draw.dtd = db[1].draw.dtd = 1;
    db[0].draw.isbg = db[1].draw.isbg = 1;
    setRGB0(&db[0].draw, 0, 0, 0);
    setRGB0(&db[1].draw, 0, 0, 0);
    SetDispMask(1);
    db[0].prim = prim_buffer[0];
    db[1].prim = prim_buffer[1];
}

ADDRESS(0x80011c24, 0x3c)
static void init_sound(void)
{
    SsInit();
    SsSetMVol(0, 0);
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
}

ADDRESS(0x80011c60, 0x50)
void swap_buffers(void)
{
    cdb = (cdb == &db[0]) ? &db[1] : &db[0];
    ClearOTag(cdb->ot, OT_LENGTH);
    prim_next = cdb->prim;
}

ADDRESS(0x80011cb0, 0x4c)
void flip_buffers(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&cdb->draw);
    PutDispEnv(&cdb->disp);
    DrawOTag(cdb->ot);
}
