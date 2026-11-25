/*
 * GAME.EXE: memory card saves (WIP module), in the shape of Sony's memory
 * card sample that OPEN.EXE's card checks also follow: four BIOS card
 * events, a test that slot 1 holds a usable card, the save list, reading a
 * save's experience and level back from its title, loading and saving.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/kernel.h>
#include <psyq/libc.h>
#include <psyq/convert.h>
#include <game/card.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/scene.h>

#define SAVE_SLOTS 15

/* Shift-JIS digits are 0x82 0x4f ('0') and up; 0x81 0x40 is a space. */
#define SJIS_DIGIT_LEAD 0x82
#define SJIS_DIGIT_ZERO 0x4f
#define SJIS_SPACE 0x4081

RODATA(0x8001173c, 0x22)
SDATA(0x8009c2c4, 0xb)

/* The product code every save file name starts with. */
DATA(0x80081818, 0xd, ".data")
static char DAT_80081818[] = "BASLUS-00255";

/* The save title, before func_80028b70 writes the digits into it. */
DATA(0x80081828, 0x3f, ".data")
static char DAT_80081828[] = "\x82\x6a\x82\x68\x82\x6d\x82\x66\x81\x66\x82\x72\x81\x40\x82\x65"
                      "\x82\x68\x82\x64\x82\x6b\x82\x63\x81\x40\x82\x51\x81\x7c\x81\x40"
                      "\x81\x40\x82\x64\x82\x77\x82\x6f\x81\x40\x81\x40\x81\x40\x81\x40"
                      "\x81\x40\x81\x40\x81\x40\x82\x6b\x82\x75\x81\x40\x81\x40";

/* The save number last loaded or saved. */
DATA(0x8009c2c0, 0x1, ".sdata")
u8 DAT_8009c2c0 = 0;

/* BIOS card events: I/O end, timeout, new card, error. */
DATA(0x8009c3a8, 0x4, ".sbss")
static long DAT_8009c3a8;
DATA(0x8009c3ac, 0x4, ".sbss")
static long DAT_8009c3ac;
DATA(0x8009c3b0, 0x4, ".sbss")
static long DAT_8009c3b0;
DATA(0x8009c3b4, 0x4, ".sbss")
static long DAT_8009c3b4;
/* The sample's hardware-event handles; nothing opens them here. */
DATA(0x8009c3b8, 0x4, ".sbss")
static long DAT_8009c3b8;
DATA(0x8009c3bc, 0x4, ".sbss")
static long DAT_8009c3bc;
DATA(0x8009c3c0, 0x4, ".sbss")
static long DAT_8009c3c0;
DATA(0x8009c3c4, 0x4, ".sbss")
static long DAT_8009c3c4;
/* The 0x6000-byte file buffer func_80028034 allocates. */
DATA(0x8009c3c8, 0x4, ".sbss")
static u8 *DAT_8009c3c8;

ADDRESS(0x80027f2c, 0xb8)
void func_80027f2c(void)
{
    DAT_8009c3a8 = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, 0);
    DAT_8009c3ac = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, 0);
    DAT_8009c3b0 = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, 0);
    DAT_8009c3b4 = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, 0);
    EnableEvent(DAT_8009c3a8);
    EnableEvent(DAT_8009c3ac);
    EnableEvent(DAT_8009c3b0);
    EnableEvent(DAT_8009c3b4);
}

ADDRESS(0x80027fe4, 0x50)
void func_80027fe4(void)
{
    CloseEvent(DAT_8009c3a8);
    CloseEvent(DAT_8009c3ac);
    CloseEvent(DAT_8009c3b0);
    CloseEvent(DAT_8009c3b4);
    StopCARD();
}

ADDRESS(0x80028034, 0x5c)
void func_80028034(void)
{
    EnterCriticalSection();
    StartCARD();
    ChangeClearPAD(0);
    _bu_init();
    DAT_8009c3c8 = (u8 *)func_800194f0(CARD_FILE_SIZE);
    if (DAT_8009c3c8 == NULL) {
        exit(1);
    }
    ExitCriticalSection();
}

ADDRESS(0x80028090, 0x44)
void func_80028090(void)
{
    func_80019518((void *)DAT_8009c3c8);
    EnterCriticalSection();
    StopCARD();
    PadInit(0);
    ExitCriticalSection();
}

/*
 * 0 when slot 1 holds a card a file can be created on, 2 when it cannot,
 * else the failing event (1 timeout, 3 error).
 */
ADDRESS(0x800280d4, 0xf0)
s32 func_800280d4(void)
{
    char name[] = "bu00:BASLUS-00255TEMP    ";
    long fd;
    s32 event;

    func_80028d0c();
    _card_info(0);
    event = func_80028c98();
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
 * more than 12 blocks.
 */
ADDRESS(0x800281c4, 0x1d4)
s32 func_800281c4(struct DIRENTRY *dir, s32 *count)
{
    struct DIRENTRY sorted[SAVE_SLOTS];
    long used = 0;
    char number[2] = " ";
    struct DIRENTRY *top;
    s32 i;
    s32 slot;

    top = dir;
    memset((u_char *)top, 0, sizeof(sorted));
    *count = 0;
    if (firstfile("bu00:*", top) == top) {
        do {
            used += dir->size;
            if (strncmp(dir->name, DAT_80081818, 12) == 0)
                (*count)++;
            dir++;
        } while (nextfile(dir) == dir);
    }
    dir = top;
    memset((u_char *)sorted, 0, sizeof(sorted));
    for (i = 0; i < SAVE_SLOTS; dir++, i++) {
        if (strncmp(dir->name, DAT_80081818, 12) == 0) {
            number[0] = dir->name[12];
            slot = atoi(number) - 1;
            memcpy((void *)&sorted[slot], (const void *)dir, sizeof(*dir));
        }
    }
    memcpy((void *)top, (const void *)sorted, sizeof(sorted));
    return used > 0x18000;
}

/*
 * Reads the experience, level and save number back from the title of the
 * save file name; 1 when it is not a save or cannot be read.
 */
ADDRESS(0x80028398, 0x244)
s32 func_80028398(char *name, s32 *experience, s32 *level, s32 *number)
{
    SaveHeader header;
    char path[40] = "bu00:";
    char digit[2] = " ";
    s16 ch;
    long fd;
    s32 i;
    s32 scale;

    if (strncmp(name, DAT_80081818, 12) != 0)
        return 1;
    strcat(path, name);
    fd = open(path, O_RDONLY);
    if (fd == -1 || read(fd, (void *)&header, sizeof(header)) != sizeof(header))
        return 1;
    close(fd);
    *experience = 0;
    for (scale = 100000, i = 0; i < 6; i++) {
        memcpy((void *)&ch, (const void *)&header.title[40 + i * 2], 2);
        if (ch != SJIS_SPACE) {
            ch = (ch >> 8) - SJIS_DIGIT_ZERO;
            *experience += ch * scale;
        }
        scale /= 10;
    }
    *level = 0;
    for (scale = 10, i = 0; i < 2; i++) {
        memcpy((void *)&ch, (const void *)&header.title[58 + i * 2], 2);
        if (ch != SJIS_SPACE) {
            ch = (ch >> 8) - SJIS_DIGIT_ZERO;
            *level += ch * scale;
        }
        scale /= 10;
    }
    digit[0] = name[12];
    *number = atoi(digit);
    return 0;
}

/* 1 when formatting the card in slot 1 fails. */
ADDRESS(0x800285dc, 0x30)
s32 func_800285dc(void)
{
    return format("bu00:") != 1;
}

/*
 * Loads save number into the game state (three tries): 0, or 1 when the
 * file cannot be read, 2 when its checksum is wrong.
 */
ADDRESS(0x8002860c, 0x144)
s32 func_8002860c(s32 number)
{
    char path[40] = "bu00:";
    s32 tries = 0;
    long fd;
    s32 result;
    u8 *buffer;

    do {
        func_80019538();
        func_80018358();
    } while ((s16)DAT_8018fad4.loading != 0);
retry:
    strcat(path, DAT_80081818);
    path[17] = number + '0';
    path[18] = '\0';
    fd = open(path, O_RDONLY);
    if (fd == -1 || read(fd, (void *)DAT_8009c3c8, CARD_FILE_SIZE) != CARD_FILE_SIZE) {
        result = 1;
    } else {
        close(fd);
        buffer = DAT_8009c3c8;
        if (((SaveHeader *)buffer)->checksum != func_80028c74(buffer + CARD_DATA)) {
            result = 2;
        } else {
            func_8005ffd0(DAT_8009c3c8 + CARD_DATA);
            DAT_8009c2c0 = number;
            return 0;
        }
    }
    if (tries < 2) {
        tries++;
        goto retry;
    }
    return result;
}

/*
 * Saves the game state as save number, or as the first free number when
 * there is no such save yet (four tries, each read back and checked): 0,
 * 1 when it cannot be written, 2 when the card is full.
 */
ADDRESS(0x80028750, 0x420)
s32 func_80028750(s32 number)
{
    struct DIRENTRY dir[SAVE_SLOTS];
    SaveHeader header;
    char path[40] = "bu00:";
    s32 used[SAVE_SLOTS];
    RECT rect;
    char digit[10] = " ";
    s32 tries = 0;
    s32 count;
    s32 full;
    s32 found;
    s32 i;
    s32 n;
    u32 sum;
    long fd;

    memset((u_char *)used, 0, sizeof(used));
    memset((u_char *)dir, 0, sizeof(dir));
    full = func_800281c4(dir, &count);
    found = 0;
    for (i = 0; i < SAVE_SLOTS; i++) {
        if (strncmp(dir[i].name, DAT_80081818, 12) == 0) {
            digit[0] = dir[i].name[12];
            n = atoi(digit);
            used[n - 1] = 1;
            if (n == number)
                found = 1;
        }
    }
    if (!found) {
        if (full == 1)
            return 2;
        for (i = 0; i < SAVE_SLOTS; i++) {
            if (used[i] == 0) {
                number = i + 1;
                i = SAVE_SLOTS;
            }
        }
    }
retry:
    strcat(path, DAT_80081818);
    path[17] = number + '0';
    header.magic[0] = 'S';
    header.magic[1] = 'C';
    header.type = 0x13;
    path[18] = '\0';
    header.blocks = 3;
    strcpy(header.title, DAT_80081828);
    func_80028b70(&header, number);
    rect.x = 80;
    rect.y = number + 492;
    rect.w = 16;
    rect.h = 1;
    StoreImage(&rect, (u_long *)header.clut);
    rect.x = 804;
    rect.y = number * 16 + 288;
    rect.w = 4;
    rect.h = 16;
    StoreImage(&rect, (u_long *)header.icon[0]);
    rect.x = 808;
    StoreImage(&rect, (u_long *)header.icon[1]);
    rect.x = 812;
    StoreImage(&rect, (u_long *)header.icon[2]);
    memset(DAT_8009c3c8, 0, CARD_FILE_SIZE);
    func_8005f7bc(DAT_8009c3c8 + CARD_DATA);
    sum = header.checksum = func_80028c74(DAT_8009c3c8 + CARD_DATA);
    memcpy((void *)DAT_8009c3c8, (const void *)&header, sizeof(header));
    if (!found) {
        fd = open(path, O_CREAT | (3 << 16));
        if (fd == -1)
            return 1;
        close(fd);
        found = 1;
    }
    fd = open(path, O_WRONLY);
    if (fd != -1) {
        if (write(fd, (const void *)DAT_8009c3c8, CARD_FILE_SIZE) == CARD_FILE_SIZE) {
            close(fd);
            fd = open(path, O_RDONLY);
            if (fd != -1) {
                if (read(fd, (void *)DAT_8009c3c8, CARD_FILE_SIZE) == CARD_FILE_SIZE) {
                    close(fd);
                    if (sum == func_80028c74(DAT_8009c3c8 + CARD_DATA)) {
                        DAT_8009c2c0 = number;
                        return 0;
                    }
                }
            }
        }
    }
    if (tries < 3) {
        tries++;
        goto retry;
    }
    return 1;
}

/* Writes the save number, experience and level into the title as digits. */
ADDRESS(0x80028b70, 0x104)
void func_80028b70(SaveHeader *header, s32 number)
{
    s32 i;
    s32 d;
    s32 e;
    s32 experience = DAT_801b24e4.experience;
    s32 level = DAT_801b24e4.level;

    header->title[30] = SJIS_DIGIT_LEAD;
    header->title[31] = number + SJIS_DIGIT_ZERO;
    for (i = 0; i < 6; i++) {
        d = experience % 10 + SJIS_DIGIT_ZERO;
        header->title[(25 - i) * 2 + 1] = d;
        header->title[(25 - i) * 2] = SJIS_DIGIT_LEAD;
        experience /= 10;
        if (experience == 0)
            i = 6;
    }
    for (i = 0; i < 2; i++) {
        e = level % 10 + SJIS_DIGIT_ZERO;
        header->title[(30 - i) * 2 + 1] = e;
        header->title[(30 - i) * 2] = SJIS_DIGIT_LEAD;
        level /= 10;
        if (level == 0)
            i = 2;
    }
}

ADDRESS(0x80028c74, 0x24)
u32 func_80028c74(u8 *data)
{
    u32 sum = 0;
    s32 i;

    i = CARD_DATA_SIZE - 1;
    do {
        i--;
        sum += *data++;
    } while (i >= 0);
    return sum;
}

ADDRESS(0x80028c98, 0x74)
s32 func_80028c98(void)
{
    for (;;) {
        if (TestEvent(DAT_8009c3a8) == 1)
            return 0;
        if (TestEvent(DAT_8009c3ac) == 1)
            return 1;
        if (TestEvent(DAT_8009c3b0) == 1)
            return 2;
        if (TestEvent(DAT_8009c3b4) == 1)
            return 3;
    }
}

ADDRESS(0x80028d0c, 0x48)
void func_80028d0c(void)
{
    TestEvent(DAT_8009c3a8);
    TestEvent(DAT_8009c3ac);
    TestEvent(DAT_8009c3b0);
    TestEvent(DAT_8009c3b4);
}
