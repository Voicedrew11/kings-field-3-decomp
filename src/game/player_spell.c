/*
 * GAME.EXE: starting a spell and the wall-sliding mover (same WIP module as
 * src/game/player.c, after the unreconstructed func_8002d2a0; contiguous
 * with src/game/player_action.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/render.h>
#include <game/collision.h>
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

/*
 * Moves the player speed units along angle (4096ths of a turn), retrying
 * around what func_80033f38 hit: the push of the last test, up to six turns
 * along a pushing Object44 (unk3 bit 2), around a wall point, shorter steps,
 * one axis at a time, and along the slope of the hit record. Returns 1 when
 * the player moved freely; unk11c keeps the last step tried.
 */
ADDRESS(0x8002e3f8, 0x8c4)
s32 func_8002e3f8(s32 angle, s32 speed)
{
    VECTOR p;
    Object44 *obj;
    s32 dx;
    s32 dz;
    s32 hit;
    s32 blocked;
    s32 tries;
    s32 lz;
    s32 step;
    s32 a;
    s32 r;
    s32 dx0;
    s32 dz0;
    s32 pushed;
    s32 cornered;
    s32 stepped;
    s32 walled;
    s32 moved;
    s16 rel;

    dx = rcos(angle) * speed >> 12;
    dz = rsin(angle) * speed >> 12;
    tries = 0;
    moved = 0;
    walled = 0;
    stepped = 0;
    cornered = 0;
    pushed = 0;
    dx0 = dx;
    dz0 = dz;
retry:
    p.vx = dx + DAT_801b24e4.pos.vx;
    p.vz = dz + DAT_801b24e4.pos.vz;
    hit = func_80033f38(p.vx, DAT_801b24e4.pos.vy, p.vz, 800, 1700, 0x31);
    if (hit == 0) {
    free:
        if (DAT_801d4464.unk12034 >= 64) {
            s32 px;
            s32 pz;

            px = rcos(-DAT_801d4464.unk12038 - 1024) * DAT_801d4464.unk12034 >> 12;
            pz = rsin(-DAT_801d4464.unk12038 - 1024) * DAT_801d4464.unk12034 >> 12;
            if (!pushed) {
                pushed = 1;
                dx += px;
                dz += pz;
                goto retry;
            }
        }
        moved = 1;
        DAT_801b24e4.pos.vx = p.vx;
        DAT_801b24e4.pos.vz = p.vz;
        DAT_801b24e4.unk160 = DAT_801d4464.unk1200a;
        goto done;
    }
    blocked = 0;
    if (!(hit & ~0x207)) {
        if (DAT_801d4464.unk12010 + 1280 >= DAT_801b24e4.pos.vy && DAT_801b24e4.unk101 == 0
            && DAT_801d4464.unk12014 - DAT_801d4464.unk12010 < -1700) {
            goto free;
        }
        if (!(hit & 2)) {
            blocked = 1;
        }
    }
    if (DAT_801d4464.unk1202c != -1) {
        obj = &DAT_8018fb3c.objects[DAT_801d4464.unk1202c];
        if (obj->unk3 & 4) {
            tries++;
            lz = speed;
            if (lz < 0) {
                lz = -lz;
            }
            if (tries == 1) {
                s32 to;

                to = ratan2(dz, dx) - 1024;
                rel = to - obj->angle.vy;
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (lz != 0) {
                    dz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1024) >> 12) - (lz * rsin(obj->angle.vy + 1024) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1024) >> 12) + (lz * rcos(obj->angle.vy + 1024) >> 12);
                goto retry;
            } else if (tries == 2) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (dz != 0) {
                    lz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1024) >> 12) - (lz * rsin(obj->angle.vy + 1024) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1024) >> 12) + (lz * rcos(obj->angle.vy + 1024) >> 12);
                goto retry;
            } else if (tries == 3) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (lz != 0) {
                    dz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1000) >> 12) - (lz * rsin(obj->angle.vy + 1000) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1000) >> 12) + (lz * rcos(obj->angle.vy + 1000) >> 12);
                goto retry;
            } else if (tries == 4) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (dz != 0) {
                    lz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1000) >> 12) - (lz * rsin(obj->angle.vy + 1000) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1000) >> 12) + (lz * rcos(obj->angle.vy + 1000) >> 12);
                goto retry;
            } else if (tries == 5) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (lz != 0) {
                    dz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1048) >> 12) - (lz * rsin(obj->angle.vy + 1048) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1048) >> 12) + (lz * rcos(obj->angle.vy + 1048) >> 12);
                goto retry;
            } else if (tries == 6) {
                dz = lz * rcos(rel) >> 12;
                lz = lz * rsin(rel) >> 12;
                if (dz != 0) {
                    lz = 0;
                }
                dx = (dz * rcos(obj->angle.vy + 1048) >> 12) - (lz * rsin(obj->angle.vy + 1048) >> 12);
                dz = (dz * rsin(obj->angle.vy + 1048) >> 12) + (lz * rcos(obj->angle.vy + 1048) >> 12);
                goto retry;
            }
        }
    }
    if (hit & 0x30) {
        s32 wx;
        s32 wz;

        if (++walled == 2) {
            goto done;
        }
        func_80033d84();
        wx = DAT_801d4464.unk1203c.vx - DAT_801b24e4.pos.vx;
        wz = DAT_801d4464.unk1203c.vz - DAT_801b24e4.pos.vz;
        a = ratan2(dz - wz, dx - wx);
        r = DAT_801d4464.unk1204c + 880;
        dx = rcos(a) * r / 4096;
        dz = rsin(a) * r / 4096;
        p.vx = dx + DAT_801d4464.unk1203c.vx;
        dx = p.vx - DAT_801b24e4.pos.vx;
        p.vz = dz + DAT_801d4464.unk1203c.vz;
        dz = p.vz - DAT_801b24e4.pos.vz;
        goto retry;
    }
    if (!stepped) {
        for (step = speed - 22; step >= 0; step -= 22) {
            p.vx = DAT_801b24e4.pos.vx + rcos(angle) * step / 4096;
            p.vz = DAT_801b24e4.pos.vz + rsin(angle) * step / 4096;
            if (func_80033f38(p.vx, DAT_801b24e4.pos.vy, p.vz, 800, 1700, 0x31) == 0) {
                DAT_801b24e4.pos.vx = p.vx;
                DAT_801b24e4.pos.vz = p.vz;
                break;
            }
        }
        stepped = 1;
    }
    if (blocked || (hit & 1)) {
    slide:
        if (dx != 0) {
            dx = 0;
            goto retry;
        }
        if (dz != 0) {
            dx = dx0;
            dz = 0;
            goto retry;
        }
    }
    if (hit & 2) {
        s32 slope;

        if (cornered) {
            goto slide;
        }
        slope = DAT_801d4464.unk12004[2] & 3;
        if (slope == 0 || slope == 2) {
            dz = (dx0 + dz0) >> 1;
            dx = dz;
            cornered = 1;
            goto retry;
        }
        dx = (dx0 - dz0) >> 1;
        dz = -dx;
        cornered = 1;
        goto retry;
    }
    moved = 0;
done:
    DAT_801b24e4.unk11c.vx = dx;
    DAT_801b24e4.unk11c.vz = dz;
    return moved;
}
