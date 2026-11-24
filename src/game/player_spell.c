/*
 * GAME.EXE: starting a spell (same WIP module as src/game/player.c, between
 * the unreconstructed func_8002d2a0 and func_8002e3f8).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/render.h>
#include <game/data.h>
#include <game/player.h>

RODATA(0x80011920, 0x154)

/*
 * Starts spell id (Record18): checks and pays its unk1c cost, then sets the
 * repeat count unk106, the interval unk107 and the direction unk14c that
 * func_8002fe1c casts it with.
 */
ADDRESS(0x8002deec, 0x50c)
void func_8002deec(s32 id)
{
    SVECTOR unused;
    Record18 *r;
    s32 cost;

    if (DAT_801b24e4.unk105 != 0xff) {
        return;
    }
    if (id == 0xff) {
        return;
    }
    r = &DAT_801b77ec.unk0[id];
    cost = r->unk16;
    if (DAT_801b24e4.unkf0 == 0x26) {
        cost <<= 1;
    }
    if (DAT_801b24e4.unkf1 == 0x2e) {
        cost >>= 1;
    }
    if (DAT_801b24e4.unk1c < cost) {
        func_80041eec(9);
        return;
    }
    if (DAT_801b24e4.unkcb == 0x1f) {
        return;
    }
    switch (id) {
    case 26:
        if (DAT_801b24e4.unk86 != 0) {
            return;
        }
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 27:
        if (DAT_801b24e4.unk88 != 0) {
            return;
        }
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 28:
        if (DAT_801b24e4.unk8a != 0) {
            return;
        }
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 25:
        if (DAT_801b24e4.unk78 == 0) {
            return;
        }
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 23:
        if (DAT_801b24e4.hp == DAT_801b24e4.hp_max) {
            return;
        }
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 29:
        if (DAT_801b24e4.unk8e != 0) {
            return;
        }
        DAT_801b24e4.unk8e = 900;
        DAT_801b24e4.unk1c -= cost;
        func_8002a10c(r);
        return;
    case 30:
        if (DAT_801b24e4.hp == DAT_801b24e4.hp_max && DAT_801b24e4.unk7a == 0 && DAT_801b24e4.unk7e == 0
            && DAT_801b24e4.unk82 == 0 && DAT_801b24e4.unk84 == 0) {
            return;
        }
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    }
    if (DAT_801b24e4.unk22 < 5000) {
        return;
    }
    switch (id) {
    case 23:
    case 25:
    case 26:
    case 27:
    case 28:
    case 30:
        func_8002a10c(r);
        break;
    }
    DAT_801b24e4.unk14c.vx = -200;
    DAT_801b24e4.unk14c.vy = 200;
    DAT_801b24e4.unk14c.vz = 400;
    DAT_801b24e4.unk22 = 0;
    DAT_801b24e4.unkf = 0;
    DAT_801b24e4.unk105 = id;
    DAT_801b24e4.unk108 = 1;
    DAT_801b24e4.unk1c -= cost;
    switch (id) {
    case 4:
        DAT_801b24e4.unk14c.vy = -512;
        DAT_801b24e4.unk14c.vx = 0;
        DAT_801b24e4.unk14c.vz = 2000;
        /* fall through */
    case 0:
    case 5:
    case 6:
    case 12:
    case 15:
    case 16:
    case 19:
    case 21:
    case 29:
    case 54:
    case 55:
    case 58:
    case 60:
    case 62:
    case 68:
    case 69:
    case 70:
    case 72:
    case 73:
    case 75:
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 76:
        DAT_801b24e4.unk14c.vz = 600;
        DAT_801b24e4.unk14c.vx = 0;
        DAT_801b24e4.unk14c.vy = 0;
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 1:
        DAT_801b24e4.unk14c.vx = -2000;
        DAT_801b24e4.unk14c.vy = 1600;
        DAT_801b24e4.unk14c.vz = 1000;
        DAT_801b24e4.unk106 = 5;
        DAT_801b24e4.unk107 = 3;
        break;
    case 56:
        DAT_801b24e4.unk14c.vy = 500;
        DAT_801b24e4.unk106 = 3;
        DAT_801b24e4.unk107 = 1;
        break;
    case 13:
        DAT_801b24e4.unk14c.vy = 500;
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 64:
        DAT_801b24e4.unk106 = 3;
        DAT_801b24e4.unk107 = 1;
        break;
    case 66:
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 9:
        DAT_801b24e4.unk14c.vy = -400;
        DAT_801b24e4.unk14c.vz = 400;
        DAT_801b24e4.unk14c.vx = 0;
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 17:
        DAT_801b24e4.unk14c.vz = 600;
        DAT_801b24e4.unk14c.vy = -600;
        DAT_801b24e4.unk14c.vx = 0;
        DAT_801b24e4.unk106 = 1;
        DAT_801b24e4.unk107 = 1;
        break;
    case 20:
    case 71:
        DAT_801b24e4.unk14c.vx = -200;
        DAT_801b24e4.unk106 = 5;
        DAT_801b24e4.unk107 = 2;
        break;
    case 2:
        DAT_801b24e4.unk106 = 3;
        DAT_801b24e4.unk107 = 1;
        break;
    case 11:
        DAT_801b24e4.unk106 = 12;
        DAT_801b24e4.unk107 = 1;
        break;
    case 7:
        DAT_801b24e4.unk106 = 2;
        DAT_801b24e4.unk107 = 1;
        DAT_801b24e4.unk14c.vz = 0;
        DAT_801b24e4.unk14c.vy = 0;
        DAT_801b24e4.unk14c.vx = 0;
        break;
    case 8:
        DAT_801b24e4.unk106 = 4;
        DAT_801b24e4.unk107 = 2;
        break;
    }
    DAT_801b24e4.unkac = r;
}
