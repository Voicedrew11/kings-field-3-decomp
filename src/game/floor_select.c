/*
 * GAME.EXE: map cell selection (same WIP module as src/game/player.c,
 * after the unreconstructed func_80030fcc; func_8003260c follows).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/funcs.h>

/* The map cell outside the 0x28000 square map. */
DATA(0x8008190c, 0xa, ".data")
static MapCell DAT_8008190c = { { { 0xff, 0x00, 0x00, 0x3f, 0x00 }, { 0xff, 0x00, 0x00, 0x3f, 0x00 } } };

/*
 * Selects the map cell under x/z (2048 units a cell) and the layer of it
 * (0 or 5 bytes in) whose height suits y; returns that floor height.
 */
ADDRESS(0x800324f0, 0x11c)
s32 func_800324f0(s32 x, s32 y, s32 z)
{
    MapCell *cell;
    u16 h;

    if (x >= 0 && x <= 0x27fff && z >= 0 && z <= 0x27fff) {
        cell = &DAT_801d4464.cells[z >> 11][x >> 11];
        h = -(y >> 7);
        if (cell->layer[0].unk1 > cell->layer[1].unk1) {
            if (h >= cell->layer[0].unk1 || cell->layer[1].unk1 == 0) {
            lower:
                DAT_801d4464.unk1200a = 0;
                DAT_801d4464.unk1200c = -cell->layer[0].unk1 << 7;
            } else {
            upper:
                DAT_801d4464.unk1200a = 5;
                DAT_801d4464.unk1200c = -cell->layer[1].unk1 << 7;
            }
        } else {
            if (h < cell->layer[1].unk1 && cell->layer[0].unk1 != 0) {
                goto lower;
            }
            goto upper;
        }
    } else {
        cell = &DAT_8008190c;
        DAT_801d4464.unk1200a = 0;
        DAT_801d4464.unk1200c = 0;
    }
    DAT_801d4464.unk12000 = cell;
    return DAT_801d4464.unk1200c;
}
