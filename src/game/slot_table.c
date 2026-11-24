/*
 * GAME.EXE: the slot tables of DAT_801983a8 (WIP module): building the
 * Record06 records and their runs of slot bytes from the copied Record04
 * table, marking a slot used, and the searches and counts over the runs.
 * func_80060d20 (the in-game movie player) follows.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/data.h>
#include <game/misc.h>

/* Builds the Record06 records and an empty run of count slots for each, then marks slots 0-3 of id 0x20. */
ADDRESS(0x80060800, 0xec)
void func_80060800(void)
{
    Record04 *src;
    Record06 *dst;
    u8 *p;
    s32 i;
    s32 n;

    p = DAT_801983a8.unk1e0;
    dst = DAT_801983a8.unkc0;
    src = DAT_801983a8.unk0;
    for (i = 47; i != -1; i--) {
        if (src->id == 0xff) {
            break;
        }
        dst->id = src->id;
        dst->unk2 = src->unk2;
        dst->offset = p - DAT_801983a8.unk1e0;
        n = src->count;
        while (--n != -1) {
            *p++ = 0xff;
        }
        *p++ = 0xff;
        dst++;
        src++;
    }
    dst->id = 0xff;
    func_800608ec(0x20, 0);
    func_800608ec(0x20, 1);
    func_800608ec(0x20, 2);
    func_800608ec(0x20, 3);
}

/* Marks slot of id's run used (3) and frees (0) the 0xff slots before it. */
ADDRESS(0x800608ec, 0x124)
void func_800608ec(u8 id, u8 slot)
{
    Record06 *t;
    u8 *p;
    s32 n;
    u8 k;
    u8 c;

    t = DAT_801983a8.unkc0;
    n = 48;
    if (id == 1 || id == 2) {
        id = 1;
    }
    if (id == 7 || id == 0x25) {
        id = 7;
    }
    if (id == 0xf || id == 0x24) {
        id = 0xf;
    }
    if (id == 0x20 || id == 0x21 || id == 0x22) {
        id = 0x20;
    }
    while (--n != -1) {
        if (t->id == 0xff) {
            return;
        }
        if (t->id == id) {
            p = DAT_801983a8.unk1e0 + t->offset + slot;
            *p = 3;
            if (slot == 0) {
                return;
            }
            k = 0;
            c = slot - 1;
            do {
                k++;
                if (p[-k] == 0xff) {
                    p[-k] = 0;
                }
            } while (c--);
            return;
        }
        t++;
    }
}

/* Index in the run of the n-th id's slot after slot (0xfffd/0xfffe/0xffff: none). */
ADDRESS(0x80060a10, 0xfc)
u16 func_80060a10(u8 n, u8 slot)
{
    Record06 *t;
    u8 *p;

    t = &DAT_801983a8.unkc0[func_80060cd0(func_80060c54(n))];
    if (t->id == 0xff) {
        return 0xfffe;
    }
    if (t->offset != 0xffff) {
        p = DAT_801983a8.unk1e0 + t->offset;
        if (*p != 0xff) {
            do {
                if ((*p & 2) && (*p & 1)) {
                    if (slot == 0) {
                        return t->unk2 + (p - DAT_801983a8.unk1e0 - t->offset);
                    }
                } else if (slot == 0) {
                    return 0xfffd;
                }
                p++;
                slot--;
            } while (*p != 0xff);
            return 0xffff;
        }
    }
    return 0xffff;
}

/* Number of runs with a used slot. */
ADDRESS(0x80060b0c, 0x9c)
u8 func_80060b0c(void)
{
    Record06 *t;
    u8 *p;
    u8 n;

    n = 0;
    for (t = DAT_801983a8.unkc0; t->id != 0xff; t++) {
        if (t->offset == 0xffff) {
            continue;
        }
        p = DAT_801983a8.unk1e0 + t->offset;
        while (*p++ != 0xff) {
            if ((*p & 2) && (*p & 1)) {
                n++;
                break;
            }
        }
    }
    return n;
}

/* Used slots in the n-th id's run. */
ADDRESS(0x80060ba8, 0xac)
u8 func_80060ba8(u8 n)
{
    Record06 *t;
    u8 *p;
    u8 count;

    count = 0;
    t = &DAT_801983a8.unkc0[func_80060cd0(func_80060c54(n))];
    if (t->id != 0xff && t->offset != 0xffff) {
        p = DAT_801983a8.unk1e0 + t->offset;
        while (*p++ != 0xff) {
            if ((*p & 2) && (*p & 1)) {
                count++;
            }
        }
    }
    return count;
}

/* Id of the n-th record whose run is in use, 0xff for none. */
ADDRESS(0x80060c54, 0x7c)
u8 func_80060c54(u8 n)
{
    Record06 *t;
    u8 *p;
    u8 none;

    none = 0xff;
    for (t = DAT_801983a8.unkc0; t->id != 0xff; t++) {
        if (t->offset != 0xffff) {
            p = DAT_801983a8.unk1e0 + t->offset;
            if (*p != 0xff) {
                if (n == 0) {
                    return t->id;
                }
                n--;
            }
        }
    }
    return none;
}

/* Index of id's record, 0xff for none. */
ADDRESS(0x80060cd0, 0x50)
u8 func_80060cd0(u8 id)
{
    Record06 *t;
    u8 i;

    i = 0;
    for (t = DAT_801983a8.unkc0; t->id != 0xff; t++) {
        if (t->id == id) {
            return i;
        }
        i++;
    }
    return 0xff;
}
