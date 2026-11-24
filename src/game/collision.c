/*
 * GAME.EXE: the collision shape interpreter func_8003260c and its front ends
 * (WIP module after src/game/floor_select.c). They pick the map cell, run the
 * interpreter and the record searches, and leave their results in
 * DAT_801d4464 (+0x12000..+0x12050).
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

RODATA(0x80011b0c, 0xc4)

/*
 * Runs the collision shape records of the map cell layer in unk12004 for a
 * body at (x, y, z) of radius and shape (bits 0-27 its height, bits 28-31
 * select the base floor and limit tests of record 0x18): floors, ceilings,
 * walls, stairs and ramps lower unk12010 (floor) and set unk12014 (limit),
 * and steep ones push through unk12034/unk12038. Record 0x40 switches once
 * to the cell's other layer. Returns the hit bits (4 floor, 8 limit, 1 and 2
 * walls).
 */
ADDRESS(0x8003260c, 0x1504)
s32 func_8003260c(s32 x, s32 y, s32 z, s32 radius, s32 shape)
{
    s32 hits;
    s32 locked;
    s32 unused; /* never set: ceiling records are always tested */
    s32 second;
    u8 *layer;
    s16 *p;
    s32 count;
    s32 top;
    s32 *heights; /* DAT_801d4464.unk1200c, unk12010 and unk12014 */
    s32 cell_x;
    s32 cell_z;
    s16 local_x; /* 16-bit copies for the stair, ramp and diagonal tests */
    s16 local_z;
    s32 flags;
    s32 bit; /* hit bits a wall record reports */
    s32 limit;
    s32 floor;
    s32 rot;
    s16 *wall;
    s16 *slope;

    hits = 0;
    locked = 0;
    unused = 0;
    second = 0;
    DAT_801d4464.unk12010 = 100000;
    DAT_801d4464.unk12014 = DAT_801d4464.unk1200c - 40000;
    DAT_801d4464.unk12018[0] = 100000;
    DAT_801d4464.unk12018[1] = 100000;
    DAT_801d4464.unk12018[2] = 100000;
    DAT_801d4464.unk12034 = 0;
    DAT_801d4464.unk12038 = 0;
    layer = (u8 *)DAT_801d4464.unk12000 + DAT_801d4464.unk1200a;
    DAT_801d4464.unk12004 = layer;
    flags = shape & 0xf0000000;
    shape &= 0x0fffffff;
again:
    p = (s16 *)(DAT_801d4464.unk10000 + ((u16 *)DAT_801d4464.unk10000)[layer[3]]);
    radius = radius * *p++ >> 12;
    top = y - shape;
    count = *p++;
    while (--count != -1) {
        heights = &DAT_801d4464.unk1200c;
        cell_x = x & 0x7ff;
        cell_z = z & 0x7ff;
        local_x = x & 0x7ff;
        local_z = z & 0x7ff;
        switch (*p++) {
        case 0x10: {
            s16 *q;
            s32 f;

            q = p++;
            if (!locked) {
                f = q[0] + heights[0];
                if (f < heights[1]) {
                    heights[1] = f;
                }
                if (heights[1] < y) {
                    hits |= 4;
                }
            }
            break;
        }
        case 0x11: {
            s16 *q;
            s32 lo;

            q = p;
            p += 2;
            if (!unused) {
                lo = q[0] + heights[0];
                if (top < lo) {
                    floor = q[1] + heights[0];
                    if (floor < top) {
                        hits |= 8;
                    } else {
                        if (floor < heights[1]) {
                            heights[1] = floor;
                        }
                        heights[2] = -100000;
                        break;
                    }
                }
                DAT_801d4464.unk12014 = lo;
            }
            break;
        }
        case 0x20:
            wall = p;
            p += 4;
            bit = 5;
            switch ((layer[2] + (u16)wall[3]) & 3) {
            case 0:
                if (wall[0] + radius - cell_x < 0) {
                    break;
                }
            wall:
                limit = wall[1];
                limit += heights[0];
                floor = wall[2] + heights[0];
                if (top < limit) {
                    if (floor < heights[1]) {
                        heights[1] = floor;
                        if (y - floor > 0) {
                            hits |= bit;
                        }
                    }
                } else {
                    heights[2] = limit;
                }
                break;
            case 1:
                if (cell_z - (0x800 - wall[0] - radius) >= 0) {
                    goto wall;
                }
                break;
            case 2:
                if (cell_x - (0x800 - wall[0] - radius) >= 0) {
                    goto wall;
                }
                break;
            case 3:
                if (wall[0] + radius - cell_z >= 0) {
                    goto wall;
                }
                break;
            }
            break;
        case 0x21:
            wall = p;
            p += 4;
            bit = 5;
            rot = (layer[2] + (u16)wall[3]) & 3;
            switch (rot) {
            case 0:
                if (wall[0] + radius < cell_x && cell_z < 0x800 - wall[0] - radius) {
                    break;
                }
                goto wall;
            case 1:
                if (cell_z < 0x800 - wall[0] - radius && cell_x < 0x800 - wall[0] - radius) {
                    break;
                }
                goto wall;
            case 2:
                if (cell_x < 0x800 - wall[0] - radius && wall[0] + radius < cell_z) {
                    break;
                }
                goto wall;
            case 3:
                if (wall[0] + radius < cell_z && wall[0] + radius < cell_x) {
                    break;
                }
                goto wall;
            }
            break;
        case 0x22:
            wall = p;
            p += 4;
            bit = 5;
            rot = (layer[2] + (u16)wall[3]) & 3;
            switch (rot) {
            case 0:
                if (wall[0] + radius < cell_x || cell_z < 0x800 - wall[0] - radius) {
                    break;
                }
                goto wall;
            case 1:
                if (cell_z < 0x800 - wall[0] - radius || cell_x < 0x800 - wall[0] - radius) {
                    break;
                }
                goto wall;
            case 2:
                if (cell_x < 0x800 - wall[0] - radius || wall[0] + radius < cell_z) {
                    break;
                }
                goto wall;
            case 3:
                if (wall[0] + radius < cell_z || wall[0] + radius < cell_x) {
                    break;
                }
                goto wall;
            }
            break;
        case 0x23: {
            s32 reach;

            wall = p;
            p += 4;
            bit = 6;
            rot = (layer[2] + (u16)wall[3]) & 3;
            switch (rot) {
            case 0:
                reach = radius - 0x800;
                if (local_x - local_z <= wall[0] + reach) {
                    goto wall;
                }
                break;
            case 1:
                reach = radius - 0x1000;
                if (-local_x - local_z <= wall[0] + reach) {
                    goto wall;
                }
                break;
            case 2:
                reach = radius - 0x800;
                if (local_z - local_x <= wall[0] + reach) {
                    goto wall;
                }
                break;
            case 3:
                if (local_x + local_z <= wall[0] + radius) {
                    goto wall;
                }
                break;
            }
            break;
        }
        case 0x24: {
            s32 d1;
            s32 d2;

            wall = p;
            p += 5;
            bit = 5;
            rot = (layer[2] + (u16)wall[4]) & 3;
            switch (rot) {
            case 0:
                d1 = wall[1] + radius - cell_x;
                d2 = cell_x - (wall[0] - radius);
                if (d1 < 0 || d2 < 0) {
                    break;
                }
            wall24:
                limit = wall[2];
                limit += heights[0];
                floor = wall[3] + heights[0];
                if (top < limit) {
                    if (floor < heights[1]) {
                        heights[1] = floor;
                        if (y - floor > 0) {
                            hits |= bit;
                        }
                    }
                } else {
                    heights[2] = limit;
                }
                break;
            case 1:
                d1 = 0x800 - wall[0] + radius - cell_z;
                d2 = cell_z - (0x800 - wall[1] - radius);
                if (d1 >= 0 && d2 >= 0) {
                    goto wall24;
                }
                break;
            case 2:
                d1 = 0x800 - wall[0] + radius - cell_x;
                d2 = cell_x - (0x800 - wall[1] - radius);
                if (d1 >= 0 && d2 >= 0) {
                    goto wall24;
                }
                break;
            case 3:
                d1 = wall[1] + radius - cell_z;
                d2 = cell_z - (wall[0] - radius);
                if (d1 >= 0 && d2 >= 0) {
                    goto wall24;
                }
                break;
            }
            break;
        }
        case 0x25: {
            wall = p;
            p += 7;
            bit = 5;
            rot = (layer[2] + (u16)wall[6]) & 3;
            switch (rot) {
            case 0:
                if ((wall[1] + radius < cell_x && cell_z < wall[2] - radius) || cell_x < wall[0] - radius
                    || wall[3] + radius < cell_z) {
                    break;
                }
            wall25:
                limit = wall[4];
                limit += heights[0];
                floor = wall[5] + heights[0];
                if (top < limit) {
                    if (floor < heights[1]) {
                        heights[1] = floor;
                        if (y - floor > 0) {
                            hits |= bit;
                        }
                    }
                } else {
                    heights[2] = limit;
                }
                break;
            case 1:
                if ((0x800 - wall[1] - radius <= cell_z || wall[2] - radius <= cell_x)
                    && cell_z <= 0x800 - wall[0] + radius && cell_x <= wall[3] + radius) {
                    goto wall25;
                }
                break;
            case 2:
                if ((0x800 - wall[1] - radius <= cell_x || cell_z <= 0x800 - wall[2] + radius)
                    && cell_x <= 0x800 - wall[0] + radius && 0x800 - wall[3] - radius <= cell_z) {
                    goto wall25;
                }
                break;
            case 3:
                if ((cell_z <= wall[1] + radius || cell_x <= 0x800 - wall[2] + radius)
                    && wall[0] - radius <= cell_z && 0x800 - wall[3] - radius <= cell_x) {
                    goto wall25;
                }
                break;
            }
            break;
        }
        case 0x30: {
            s32 n;
            s32 f;

            slope = p;
            p += 6;
            rot = (layer[2] + (u16)slope[3]) & 3;
            switch (rot) {
            case 0:
                if (local_x < slope[1] - radius || slope[2] + radius < local_x) {
                    break;
                }
                n = cell_z / slope[5];
            slope30:
                f = slope[0] + heights[0] - (n + 1) * slope[4];
                if (f < heights[1]) {
                    heights[1] = f;
                    if (f < y) {
                        hits |= 4;
                    }
                }
                if (slope[5] < 64) {
                    DAT_801d4464.unk12034 = (slope[4] << 7) / slope[5];
                    DAT_801d4464.unk12038 = rot << 10;
                }
                break;
            case 1:
                if (0x800 - slope[1] + radius < local_z || local_z < 0x800 - slope[2] - radius) {
                    break;
                }
                n = cell_x / slope[5];
                goto slope30;
            case 2:
                if (0x800 - slope[1] + radius < local_x || local_x < 0x800 - slope[2] - radius) {
                    break;
                }
                n = (0x800 - cell_z) / slope[5];
                goto slope30;
            case 3:
                if (local_z < slope[1] - radius || slope[2] + radius < local_z) {
                    break;
                }
                n = (0x800 - cell_x) / slope[5];
                goto slope30;
            }
            break;
        }
        case 0x33: {
            s16 *q;
            s16 a;
            s16 b;
            s32 f;

            q = p;
            p += 4;
            rot = (layer[2] + (u16)q[1]) & 3;
            switch (rot) {
            case 0:
                a = cell_x;
                b = cell_z;
            slope33:
                if (a < b) {
                    b = a;
                }
                f = q[0] + heights[0] - (b / q[3] + 1) * q[2];
                if (f < heights[1]) {
                    heights[1] = f;
                    if (f < y) {
                        hits |= 4;
                    }
                }
                if (q[3] < 64) {
                    DAT_801d4464.unk12034 = (q[2] << 7) / q[3];
                    DAT_801d4464.unk12038 = (rot << 10) + 512;
                }
                break;
            case 1:
                a = cell_x;
                b = 0x800 - cell_z;
                goto slope33;
            case 2:
                a = 0x800 - cell_x;
                b = 0x800 - cell_z;
                goto slope33;
            case 3:
                a = 0x800 - cell_x;
                b = cell_z;
                goto slope33;
            }
            break;
        }
        case 0x34: {
            s16 *q;
            s16 a;
            s16 b;
            s32 f;

            q = p;
            p += 4;
            rot = (layer[2] + (u16)q[1]) & 3;
            switch (rot) {
            case 0:
                a = cell_x;
                b = cell_z;
            slope34:
                if (a < b) {
                    a = b;
                }
                f = q[0] + heights[0] - (a / q[3] + 1) * q[2];
                if (f < heights[1]) {
                    heights[1] = f;
                    if (f < y) {
                        hits |= 4;
                    }
                }
                if (q[3] < 64) {
                    DAT_801d4464.unk12034 = (q[2] << 7) / q[3];
                    DAT_801d4464.unk12038 = (rot << 10) + 512;
                }
                break;
            case 1:
                a = cell_x;
                b = 0x800 - cell_z;
                goto slope34;
            case 2:
                a = 0x800 - cell_x;
                b = 0x800 - cell_z;
                goto slope34;
            case 3:
                a = 0x800 - cell_x;
                b = cell_z;
                goto slope34;
            }
            break;
        }
        case 0x32: {
            s16 *q;
            s32 n;
            s32 f;

            q = p;
            p += 6;
            rot = (layer[2] + (u16)q[3]) & 3;
            switch (rot) {
            case 0:
                n = local_z + (0x800 - local_x);
            ramp:
                if (n < q[1]) {
                    break;
                }
                if (q[2] < n) {
                    n = q[2];
                }
                n = (n - q[1]) / q[5];
                f = q[0] + heights[0] - n * q[4];
                if (f < heights[1]) {
                    heights[1] = f;
                    if (f < y) {
                        hits |= 4;
                    }
                }
                if ((q[1] < n || n < q[2]) && q[5] < 64) {
                    DAT_801d4464.unk12034 = (q[4] << 7) / q[5];
                    DAT_801d4464.unk12038 = (rot << 10) - 512;
                }
                break;
            case 1:
                n = local_x + local_z;
                goto ramp;
            case 2:
                n = local_x + (0x800 - local_z);
                goto ramp;
            case 3:
                n = (0x800 - local_x) + (0x800 - local_z);
                goto ramp;
            }
            break;
        }
        case 0x35: {
            s32 n;
            s32 lo;
            s32 f;

            slope = p;
            p += 6;
            rot = (layer[2] + (u16)slope[3]) & 3;
            switch (rot) {
            case 0:
                if (local_x < slope[1] - radius || slope[2] + radius < local_x) {
                    break;
                }
                n = cell_z / slope[5];
            slope35:
                lo = slope[0] + heights[0];
                f = lo - (n + 1) * slope[4];
                if (top < lo) {
                    if (f < heights[1]) {
                        heights[1] = f;
                        if (f < y) {
                            hits |= 4;
                        }
                    }
                    if (slope[5] < 64) {
                        DAT_801d4464.unk12034 = (slope[4] << 7) / slope[5];
                        DAT_801d4464.unk12038 = rot << 10;
                    }
                } else {
                    heights[2] = lo;
                }
                break;
            case 1:
                if (0x800 - slope[1] + radius < local_z || local_z < 0x800 - slope[2] - radius) {
                    break;
                }
                n = cell_x / slope[5];
                goto slope35;
            case 2:
                if (0x800 - slope[1] + radius < local_x || local_x < 0x800 - slope[2] - radius) {
                    break;
                }
                n = (0x800 - cell_z) / slope[5];
                goto slope35;
            case 3:
                if (local_z < slope[1] - radius || slope[2] + radius < local_z) {
                    break;
                }
                n = (0x800 - cell_x) / slope[5];
                goto slope35;
            }
            break;
        }
        case 0x31:
            if (hits & 1) {
                s16 distance;
                s16 x_distance;

                wall = p;
                p += 5;
                distance = cell_z;
                x_distance = cell_x;
                rot = (layer[2] + (u16)wall[4]) & 3;
                switch (rot) {
                case 0:
                ledge:
                    if (distance < wall[0] + radius || wall[1] - radius < distance) {
                        break;
                    }
                    DAT_801d4464.unk12010 = wall[2] + DAT_801d4464.unk1200c;
                    DAT_801d4464.unk12014 = wall[3] + DAT_801d4464.unk1200c;
                    if (top < DAT_801d4464.unk12014) {
                        hits |= 8;
                    }
                    if (DAT_801d4464.unk12010 >= y) {
                        hits &= ~0x205;
                    }
                    break;
                case 1:
                    distance = x_distance;
                    goto ledge;
                case 2:
                    distance = 0x800 - cell_z;
                    goto ledge;
                case 3:
                    distance = 0x800 - cell_x;
                    goto ledge;
                }
            }
            break;
        case 0x40:
            if (second) {
                return hits;
            }
            {
                u16 *offset = &DAT_801d4464.unk1200a;

                *offset = *offset == 0 ? 5 : 0;
                layer = (u8 *)DAT_801d4464.unk12000 + *offset;
            }
            second = 1;
            DAT_801d4464.unk1200c = -layer[1] << 7;
            goto again;
        case 0x18: {
            s16 *q;
            s32 f;

            q = p++;
            DAT_801d4464.unk12018[0] = f = q[0] + DAT_801d4464.unk1200c;
            if (flags & 0x80000000) {
                DAT_801d4464.unk12010 = f;
                if (f < y) {
                    hits |= 4;
                    locked = 1;
                }
            }
            if (flags & 0x40000000) {
                DAT_801d4464.unk12014 = f;
                if (f >= top) {
                    hits |= 8;
                }
            }
            break;
        }
        case 0x19: {
            s16 *q;

            q = p++;
            DAT_801d4464.unk12018[1] = q[0] + DAT_801d4464.unk1200c;
            break;
        }
        case 0x17: {
            s16 *q;

            q = p++;
            DAT_801d4464.unk12018[2] = q[0] + DAT_801d4464.unk1200c;
            break;
        }
        }
    }
    return hits;
}

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
