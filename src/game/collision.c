/*
 * GAME.EXE: collision test front ends (WIP module, the tail of the module of
 * func_800324f0 and the shape interpreter func_8003260c). They pick the map
 * cell, run the interpreter and the record searches, and leave their results
 * in DAT_801d4464 (+0x12000..+0x12050).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/player.h>
#include <game/entity.h>
#include <game/collision.h>
#include <game/object44.h>

/* Tests a shape at (x, y, z) after locating the point 1280 above it; returns the floor. */
ADDRESS(0x80033b10, 0x7c)
s32 func_80033b10(s32 x, s32 y, s32 z, s32 radius, s32 shape)
{
    func_800324f0(x, y - 1280, z);
    func_8003260c(x, y, z, radius, shape);
    return DAT_801d4464.unk12010;
}

/* Floor height at (x, z) on the map cell's layer for area (2 the upper one). */
ADDRESS(0x80033b8c, 0xc0)
s32 func_80033b8c(u8 area, s32 x, s32 z, s32 radius, s32 height)
{
    MapCell *cell;

    cell = &DAT_801d4464.cells[z >> 11][x >> 11];
    if (area == 2) {
        DAT_801d4464.unk1200a = 5;
        DAT_801d4464.unk1200c = -cell->layer[1].unk1 << 7;
    } else {
        DAT_801d4464.unk1200a = 0;
        DAT_801d4464.unk1200c = -cell->layer[0].unk1 << 7;
    }
    DAT_801d4464.unk12000 = cell;
    func_8003260c(x, DAT_801d4464.unk1200c, z, radius, height);
    return DAT_801d4464.unk12010;
}

/* Adds count to the occupant count of every map cell a circle at (x, z) touches. */
ADDRESS(0x80033c4c, 0xbc)
void func_80033c4c(s32 x, s32 z, s32 radius, s32 count)
{
    s32 x0;
    s32 z0;
    s32 nx;
    s32 nz;
    s32 i;
    u32 cx;
    s32 cz;
    MapCell *row;
    MapCell *cell;

    radius += 2048;
    x0 = (x - radius) >> 11;
    nx = ((x + radius) >> 11) - x0;
    z0 = (z - radius) >> 11;
    nz = ((z + radius) >> 11) - z0;
    row = &DAT_801d4464.cells[z0][x0];
    cz = z0;
    do {
        cell = row;
        row += 80;
        if ((u32)cz < 80) {
            cx = x0;
            i = nx;
            do {
                if (cx < 80) {
                    cell->layer[0].unk2 = count * 4 + cell->layer[0].unk2;
                }
                cx++;
                cell++;
            } while (--i != -1);
        }
        cz++;
    } while (--nz != -1);
}

/* Tests a shape at (x, y, z) after locating the point half its height above it. */
ADDRESS(0x80033d08, 0x7c)
s32 func_80033d08(s32 x, s32 y, s32 z, s32 radius, s32 shape)
{
    func_800324f0(x, y - (shape & 0xfffffff) / 2, z);
    return func_8003260c(x, y, z, radius, shape);
}

/* Copies the position and size of what the last func_80033f38 test hit. */
ADDRESS(0x80033d84, 0x1b4)
void func_80033d84(void)
{
    Object88 *obj;
    Object44 *carrier;
    Object44Def *def;

    if (DAT_801d4464.unk12024 & 0x80) {
        DAT_801d4464.unk1203c = DAT_801b24e4.pos;
        DAT_801d4464.unk1204c = 800;
        DAT_801d4464.unk1204e = 1700;
    } else if (DAT_801d4464.unk12028 != -1) {
        obj = &DAT_80185da8.objects[DAT_801d4464.unk12028];
        DAT_801d4464.unk1203c = obj->pos;
        DAT_801d4464.unk1204c = obj->unk1c;
        DAT_801d4464.unk1204e = obj->unk1e;
    } else if (DAT_801d4464.unk1202c != -1) {
        carrier = &DAT_8018fb3c.objects[DAT_801d4464.unk1202c];
        def = &DAT_8018fb3c.defs[carrier->unk6];
        DAT_801d4464.unk1203c = carrier->pos;
        if (carrier->unk3 & 0x10) {
            DAT_801d4464.unk1204c = def->unk4 * carrier->arg.b.unk38 >> 7;
        } else if (carrier->unk3 & 0x4) {
            DAT_801d4464.unk12050 = def->arg.h.unke;
            DAT_801d4464.unk1204c = def->arg.h.unk10;
        } else {
            DAT_801d4464.unk1204c = def->unk4;
        }
        DAT_801d4464.unk1204e = def->unk8;
    }
}

/*
 * Tests a shape at (x, y, z) against what mask selects: bit 0 the map, bit 1
 * the map's pits, bit 4 or 6 the Object88 records, bit 5 the Object44 records,
 * bit 7 the player. Returns the bits that hit. mask is a u16: callers pass
 * DAT_80185da8.unk9d04 (a word) with lhu.
 */
ADDRESS(0x80033f38, 0x248)
s32 func_80033f38(s32 x, s32 y, s32 z, s32 radius, s32 shape, u16 mask)
{
    s32 hits;

    hits = 0;
    if (mask & 1) {
        hits = func_80033d08(x, y, z, radius, shape);
        if ((mask & 2) && (DAT_801d4464.unk12004[4] & 0x40)) {
            DAT_801d4464.unk12010 = -100000;
            hits |= 1;
        }
    } else {
        DAT_801d4464.unk12000 = &DAT_801d4464.cells[z >> 11][x >> 11];
    }
    shape &= 0xfffffff;
    if (DAT_801d4464.unk12000->layer[0].unk2 & 0xfc) {
        if (mask & 0x10) {
            DAT_801d4464.unk12028 = func_8004d644(x, y, z, radius, shape);
            if (DAT_801d4464.unk12028 != -1) {
                hits |= 0x10;
            }
        } else {
            if (mask & 0x40) {
                DAT_801d4464.unk12028 = func_8004d838(x, y, z, radius, shape);
                if (DAT_801d4464.unk12028 != -1) {
                    hits |= 0x10;
                }
            }
            DAT_801d4464.unk12028 = -1;
        }
        if (mask & 0x20) {
            DAT_801d4464.unk1202c = func_80045ac8(x, y, z, radius, shape);
            if (DAT_801d4464.unk1202c != -1) {
                hits |= 0x20;
            }
        } else {
            DAT_801d4464.unk1202c = -1;
        }
        if (mask & 0x80) {
            if (func_80028e48(x, y, z, radius, shape) != -1) {
                hits |= 0x80;
            }
        }
    } else {
        DAT_801d4464.unk12028 = -1;
        DAT_801d4464.unk1202c = -1;
    }
    DAT_801d4464.unk12024 = hits;
    return hits;
}
