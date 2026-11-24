/*
 * GAME.EXE: floor tile selection (same WIP module as src/game/player.c,
 * after the unreconstructed func_80030fcc; func_8003260c follows).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/funcs.h>

/* The tile record outside the 0x28000 square map. */
DATA(0x8008190c, 0xa, ".data")
static u8 DAT_8008190c[10] = { 0xff, 0x00, 0x00, 0x3f, 0x00, 0xff, 0x00, 0x00, 0x3f, 0x00 };

/*
 * Selects the 10-byte tile record of DAT_801d4464 under x/z (2048 units a
 * tile) and the half of it (0 or 5) whose height suits y; returns its height.
 */
ADDRESS(0x800324f0, 0x11c)
s32 func_800324f0(s32 x, s32 y, s32 z)
{
    u8 *tile;
    u16 h;

    if (x >= 0 && x <= 0x27fff && z >= 0 && z <= 0x27fff) {
        tile = &DAT_801d4464.unk0[(z >> 11) * 800 + (x >> 11) * 10];
        h = -(y >> 7);
        if (tile[1] > tile[6]) {
            if (h >= tile[1] || tile[6] == 0) {
            lower:
                DAT_801d4464.unk1200a = 0;
                DAT_801d4464.unk1200c = -tile[1] << 7;
            } else {
            upper:
                DAT_801d4464.unk1200a = 5;
                DAT_801d4464.unk1200c = -tile[6] << 7;
            }
        } else {
            if (h < tile[6] && tile[1] != 0) {
                goto lower;
            }
            goto upper;
        }
    } else {
        tile = DAT_8008190c;
        DAT_801d4464.unk1200a = 0;
        DAT_801d4464.unk1200c = 0;
    }
    DAT_801d4464.unk12000 = tile;
    return DAT_801d4464.unk1200c;
}
