/*
 * OPEN.EXE's memory card checks, in the shape of Sony's memory card sample:
 * four BIOS card events, a test that slot 1 holds a usable card, and a scan
 * that counts King's Field saves (BASLUS-00255 plus a save number).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <psyq/sdk.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <psyq/convert.h>
#include "open.h"

#define SAVE_SLOTS 15

static int card_wait_event(void);
static void card_clear_events(void);

RODATA(0x8001108c, 0x1a)
SDATA(0x80040428, 0xb)

/* The product code every save file name starts with. */
DATA(0x8002c1bc, 0xd, ".data")
static char save_prefix[] = "BASLUS-00255";

/* BIOS card events: I/O end, timeout, new card, error. */
DATA(0x80040438, 0x4, ".sbss")
static long ev0;
DATA(0x8004043c, 0x4, ".sbss")
static long ev1;
DATA(0x80040440, 0x4, ".sbss")
static long ev2;
DATA(0x80040444, 0x4, ".sbss")
static long ev3;
/* The sample's hardware-event handles; nothing opens them here. */
DATA(0x80040448, 0x4, ".sbss")
static long ev10;
DATA(0x8004044c, 0x4, ".sbss")
static long ev11;
DATA(0x80040450, 0x4, ".sbss")
static long ev12;
DATA(0x80040454, 0x4, ".sbss")
static long ev13;

ADDRESS(0x80014014, 0xc0)
void card_init(void)
{
    InitCARD(1);
    ev0 = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, 0);
    ev1 = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    ev2 = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, 0);
    ev3 = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, 0);
    EnableEvent(ev0);
    EnableEvent(ev1);
    EnableEvent(ev2);
    EnableEvent(ev3);
}

ADDRESS(0x800140d4, 0x50)
void card_close(void)
{
    CloseEvent(ev0);
    CloseEvent(ev1);
    CloseEvent(ev2);
    CloseEvent(ev3);
    StopCARD();
}

ADDRESS(0x80014124, 0x28)
void card_start(void)
{
    StartCARD();
    _bu_init();
}

ADDRESS(0x8001414c, 0x28)
void card_stop(void)
{
    StopCARD();
    PadInit(0);
}

/*
 * 0 when slot 1 holds a card a file can be created on, 2 when it cannot,
 * else the failing event (1 timeout, 3 error).
 */
ADDRESS(0x80014174, 0xf0)
int card_check_writable(void)
{
    char name[] = "bu00:BASLUS-00255TEMP    ";
    long fd;
    int event;

    card_clear_events();
    _card_info(0);
    event = card_wait_event();
    if (event != 2 && event != 0)
        return event;
    fd = open(name, O_CREAT);
    close(fd);
    bios_delete(name);
    return (fd == -1) * 2;
}

/*
 * Lists slot 1 into dir, counting the saves in *count, and sorts the saves
 * by their number into dir's first entries. 1 when the card's files fill
 * more than 13 blocks.
 */
ADDRESS(0x80014264, 0x1d4)
int card_list_saves(struct DIRENTRY *dir, int *count)
{
    struct DIRENTRY sorted[SAVE_SLOTS];
    long used = 0;
    char number[2] = " ";
    struct DIRENTRY *top;
    int i;
    int slot;

    top = dir;
    memset((u_char *)top, 0, sizeof(sorted));
    *count = 0;
    if (firstfile("bu00:*", top) == top) {
        do {
            used += dir->size;
            if (strncmp(dir->name, save_prefix, 12) == 0)
                (*count)++;
            dir++;
        } while (nextfile(dir) == dir);
    }
    dir = top;
    memset((u_char *)sorted, 0, sizeof(sorted));
    for (i = 0; i < SAVE_SLOTS; dir++, i++) {
        if (strncmp(dir->name, save_prefix, 12) == 0) {
            number[0] = dir->name[12];
            slot = atoi(number) - 1;
            memcpy((void *)&sorted[slot], (const void *)dir, sizeof(*dir));
        }
    }
    memcpy((void *)top, (const void *)sorted, sizeof(sorted));
    return used > 0x1a000;
}

ADDRESS(0x80014438, 0x74)
static int card_wait_event(void)
{
    for (;;) {
        if (TestEvent(ev0) == 1)
            return 0;
        if (TestEvent(ev1) == 1)
            return 1;
        if (TestEvent(ev2) == 1)
            return 2;
        if (TestEvent(ev3) == 1)
            return 3;
    }
}

ADDRESS(0x800144ac, 0x48)
static void card_clear_events(void)
{
    TestEvent(ev0);
    TestEvent(ev1);
    TestEvent(ev2);
    TestEvent(ev3);
}
