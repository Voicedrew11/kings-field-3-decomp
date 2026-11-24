/*
 * GAME.EXE: player state, continued (same WIP module as src/game/player.c,
 * between the unreconstructed func_8002c30c and func_8002d2a0): the weapon
 * tip position and the spell checks and casts.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/render.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/player.h>
#include <kf/lib/null.h>

/* World position of vertex of the weapon model (no callers). */
ADDRESS(0x8002cea8, 0x138)
void func_8002cea8(s32 vertex, VECTOR *out)
{
    SVECTOR v;
    SVECTOR rot;
    s32 y;

    rot.vx = DAT_801b24e4.unk124.vx - DAT_801b24e4.unkb0->unk3c;
    rot.vy = DAT_801b24e4.unk124.vy - DAT_801b24e4.unkb0->unk3e;
    rot.vz = DAT_801b24e4.unk124.vz + DAT_801b24e4.unkb0->unk40;
    func_8004352c(0x20, DAT_801b24e4.unkca, DAT_801b24e4.unkc0, vertex, &v);
    v.vx -= DAT_801b24e4.unkb0->unk34;
    v.vy += DAT_801b24e4.unkb0->unk36;
    v.vz -= DAT_801b24e4.unkb0->unk38;
    func_800168a8(&rot, &v, out);
    out->vx += DAT_801b24e4.pos.vx;
    out->vz += DAT_801b24e4.pos.vz;
    y = out->vy - 1600;
    out->vy = y + (DAT_801b24e4.unk16c + DAT_801b24e4.pos.vy + DAT_801b24e4.unk170);
}

/*
 * Whether the weapon's spell of hand (0: Record44.unk3, 1: unk4) may be cast:
 * the spell must be learned; weapon 2 and 13..17 always may, others need
 * unk40 and every unk42 at 50 or more.
 */
ADDRESS(0x8002cfe0, 0x150)
s32 func_8002cfe0(s32 hand)
{
    s32 n = 5;
    s32 low;

    if (DAT_801b24e4.unkcb != 0xff) {
        if (hand == 0 && DAT_801b24e4.unkb0->unk0[3] != 0xff
            && DAT_801b77ec.unk0[DAT_801b24e4.unkb0->unk0[3]].unk0 == 0) {
            return 0;
        }
        if (hand == 1 && DAT_801b24e4.unkb0->unk4 != 0xff
            && DAT_801b77ec.unk0[DAT_801b24e4.unkb0->unk4].unk0 == 0) {
            return 0;
        }
    }
    switch (DAT_801b24e4.unkcb) {
    case 2:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
        return 1;
    }
    low = 999;
    while (--n != -1) {
        if (DAT_801b24e4.unk42[n] < low) {
            low = DAT_801b24e4.unk42[n];
        }
    }
    return DAT_801b24e4.unk40 >= 50 && low >= 50;
}

/* Casts spell id (Record18) along dir when unk1c covers its cost; pay takes the cost. */
ADDRESS(0x8002d130, 0x170)
void func_8002d130(s32 id, s32 pay, s32 dir)
{
    VECTOR unused;
    Record18 *r = &DAT_801b77ec.unk0[id];

    if (DAT_801b24e4.unk1c < r->unk16) {
        return;
    }
    DAT_801b24e4.unkac = r;
    if (pay) {
        DAT_801b24e4.unk22 = 0;
        DAT_801b24e4.unk10 = 0;
        DAT_801b24e4.unk1c -= r->unk16;
        if (id == 0x36) {
            func_8001576c(0x63, 0x6e);
        }
    }
    switch (id) {
    case 0x42:
        dir <<= 9;
        DAT_801b24e4.unk14c.vx = rcos(dir) >> 3;
        DAT_801b24e4.unk14c.vy = rsin(dir) >> 3;
        DAT_801b24e4.unk14c.vz = 600;
        func_8002c30c(id);
        break;
    case 0x47:
        func_8001576c(0x61, 0x6e);
        DAT_801b24e4.unk14c.vx = 0;
        DAT_801b24e4.unk14c.vy = dir * 300 - 300;
        DAT_801b24e4.unk14c.vz = 400;
        func_8002c30c(id);
        break;
    default:
        DAT_801b24e4.unk14c.vx = 200;
        DAT_801b24e4.unk14c.vy = 200;
        DAT_801b24e4.unk14c.vz = 400;
        func_8002c30c(id);
        break;
    }
}
