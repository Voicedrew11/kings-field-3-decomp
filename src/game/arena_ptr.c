/*
 * GAME.EXE: the DAT_801ba984 block arena's 32 saved pointers (WIP module):
 * pointers to and from arena offsets, moving the blocks' owner pointers by
 * a delta, and saving and restoring an area's object states in its block
 * (func_8005efd4/func_8005f444). The .rdata is their two jump tables.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/memory.h>
#include <game/data.h>
#include <game/misc.h>
#include <game/object44.h>

RODATA(0x80013298, 0x3e8)

/* out[i] = the arena address of saved offset i, NULL for 0xffff. */
ADDRESS(0x8005ee58, 0x50)
void func_8005ee58(u8 **out)
{
    u16 *off;
    u16 o;
    s32 i;

    off = DAT_801ba984.unk5104;
    for (i = 31; i != -1; i--) {
        o = *off++;
        if (o == 0xffff) {
            *out = NULL;
        } else {
            *out = DAT_801ba984.unk104 + o;
        }
        out++;
    }
}

/* Adds delta to the owner pointer of every used block. */
ADDRESS(0x8005eea8, 0x70)
void func_8005eea8(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)DAT_801ba984.unk104;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner + delta);
        }
        block = HEAP_NEXT(block);
    }
}

/* Saves in[i] as an arena offset, 0xffff for NULL. */
ADDRESS(0x8005ef18, 0x4c)
void func_8005ef18(u8 **in)
{
    u16 *off;
    u8 *p;
    s32 i;

    off = DAT_801ba984.unk5104;
    for (i = 31; i != -1; i--) {
        p = *in++;
        if (p == NULL) {
            *off = 0xffff;
        } else {
            *off = p - DAT_801ba984.unk104;
        }
        off++;
    }
}

/* Subtracts delta from the owner pointer of every used block. */
ADDRESS(0x8005ef64, 0x70)
void func_8005ef64(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)DAT_801ba984.unk104;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner - delta);
        }
        block = HEAP_NEXT(block);
    }
}

/*
 * Saves area's state in its saved block: the live Object88 records, the talk
 * script positions and each Object44 record by its Object44Def type.
 */
ADDRESS(0x8005efd4, 0x470)
void func_8005efd4(s32 area)
{
    u8 *ptrs[32];
    u8 buf[0xc00];
    Object88 *obj;
    Object78 *def;
    Object44 *rec;
    u8 *script;
    u8 *p;
    u8 *block;
    s32 size;
    s32 i;

    p = buf;
    obj = DAT_80185da8.objects;
    for (i = 0; i < 200; i++) {
        if (obj->unk0 != 0xff && obj->unk0 == 1) {
            *p++ = i;
            if (obj->unk9 == 3) {
                *p = 3;
            } else {
                *p = 0;
            }
            p++;
        }
        obj++;
    }
    *p++ = 0xff;
    def = DAT_80185da8.defs;
    for (i = 0; i < 40; i++) {
        if (def->unk0 == 0xff) {
            break;
        }
        script = def->scripts[0];
        def++;
        if (script != NULL && script[0] == 0x70) {
            *p++ = i;
            *p++ = script[16];
            *p++ = script[19];
        }
    }
    *p++ = 0xff;
    rec = DAT_8018fb3c.objects;
    for (i = 0; i < 396; i++) {
        if (rec->unk6 == 0xff) {
            *p++ = 0xff;
        } else {
            switch (DAT_8018fb3c.defs[rec->unk6].unk0) {
            case 0x40:
                switch (rec->unk4) {
                case 0x60:
                    *p++ = 0xf0;
                    *p++ = rec->unk6;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    *p++ = rec->angle.vy >> 4;
                    break;
                case 0x61:
                    *p++ = 0xf1;
                    *p++ = rec->unk6;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    break;
                case 0x62:
                    *p++ = 0xf2;
                    *p++ = rec->unk6;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    *p++ = rec->arg.h.unk3a >> 2;
                    break;
                case 0x70:
                    *p++ = 0xf3;
                    *p++ = rec->unk6;
                    *p++ = rec->arg.b.unk38;
                    break;
                default:
                    *p++ = 0xfd;
                    *p++ = rec->arg.b.unk38;
                    break;
                }
                break;
            case 0x53:
                if (rec->arg.b.unk38 < 2) {
                    break;
                }
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x17:
            case 0x1b:
            case 0x51:
            case 0x52:
            case 0x54:
            case 0x55:
            case 0x57:
            case 0x58:
            case 0x5a:
            case 0x5f:
            case 0xa0:
            case 0xa1:
            case 0xa3:
            case 0xa4:
            case 0xa6:
            case 0xe8:
                *p++ = 0xfd;
                *p++ = rec->arg.b.unk38;
                break;
            case 0x0f:
            case 0x11:
            case 0x18:
            case 0x1a:
            case 0xa7:
                *p++ = 0xf4;
                *p++ = rec->arg.b.unk38;
                *p++ = rec->arg.b.unk39;
                break;
            case 0x56:
                *p++ = 0xf5;
                *p++ = rec->arg.b.unk39;
                break;
            default:
                *p++ = 0xfe;
                break;
            }
        }
        rec++;
    }
    func_8005ee58(ptrs);
    func_8005eea8((s32)ptrs);
    block = ptrs[area];
    if (block != NULL) {
        func_80019454(block);
    }
    size = (p - buf + 3) & ~3;
    block = func_8001939c((HeapBlock *)DAT_801ba984.unk104, size, &ptrs[area]);
    if (block != NULL) {
        func_80018f5c((u32 *)block, (u32 *)buf, size >> 2);
        func_8005ef18(ptrs);
        func_8005ef64((s32)ptrs);
    }
}

/* Restores area's state from its saved block (func_8005efd4's format). */
ADDRESS(0x8005f444, 0x378)
void func_8005f444(s32 area)
{
    u8 *ptrs[32];
    Object44 *rec;
    u8 *script;
    u8 *p;
    s32 i;
    s32 x;
    s32 z;
    s16 y;

    func_8005ee58(ptrs);
    p = ptrs[area];
    if (p == NULL) {
        return;
    }
    for (;;) {
        s32 n = *p++;

        if (n == 0xff) {
            break;
        }
        (DAT_80185da8.objects + n)->unk9 = *p++;
    }
    for (;;) {
        s32 n = *p++;

        if (n == 0xff) {
            break;
        }
        script = (DAT_80185da8.defs + n)->scripts[0];
        script[16] = *p++;
        script[19] = *p++;
    }
    rec = DAT_8018fb3c.objects;
    for (i = 0; i < 396; i++) {
        switch (*p++) {
        case 0xff:
            rec->unk6 = 0xff;
            break;
        case 0xf4:
            rec->arg.b.unk38 = *p++;
        case 0xf5:
            rec->arg.b.unk39 = *p++;
            break;
        case 0xf0:
            func_800448b8(rec);
            rec->unk4 = 0x60;
            rec->unk6 = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            rec->angle.vy = *p++ << 4;
            rec->angle.vz = 0x400;
        place:
            rec->pos.vx = x << 2;
            rec->pos.vz = z << 2;
            rec->pos.vy = y;
            rec->unk8 = 99;
            func_800324f0(rec->pos.vx, rec->pos.vy, rec->pos.vz);
            rec->unk0 = DAT_801d4464.unk1200a == 0 ? 1 : 2;
            rec->arg.b.unk38 = 0xff;
            break;
        case 0xf1:
            func_800448b8(rec);
            rec->unk4 = 0x61;
            rec->unk6 = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            goto place;
        case 0xf2:
            func_800448b8(rec);
            rec->unk4 = 0x62;
            rec->unk6 = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            rec->arg.h.unk3a = (*p++ << 2) + (rand() >> 13);
            if (rec->unk6 == 0x79) {
                rec->angle.vx = 0x400;
            }
            goto place;
        case 0xf3:
            func_800448b8(rec);
            rec->unk4 = 0x70;
            rec->unk6 = *p++;
        case 0xfd:
            rec->arg.b.unk38 = *p++;
            break;
        }
        rec++;
    }
}
