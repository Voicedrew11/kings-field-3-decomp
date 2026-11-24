/*
 * GAME.EXE: Object88 searches and animation time (WIP module): the
 * player-in-reach test, the best record in a cone, the first record a point
 * touches, the current-record pointers, and the animation time steps.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>

/*
 * Hits every active record within reach of pos (mode: 0x8000 sphere, 0x8001
 * sphere above its base, 0x8002 sphere outside the cone either side of the
 * yaw kept in pos->pad, otherwise func_80016ec8's own mode) with
 * func_8004c668, its strength falling off to falloff at the edge.
 */
ADDRESS(0x8004ceac, 0x3a8)
void func_8004ceac(VECTOR *pos, s32 near, s32 reach, s32 mode, u16 falloff, u16 a5, u16 a6, u16 a7,
                   u16 a8, u16 a9, u16 a10, u16 a11, u16 a12, u16 a13, u16 flags, u16 a15,
                   Record18 *rec)
{
    Object88 *obj;
    VECTOR *target;
    s32 dist;
    s32 angle;
    s32 side;
    u16 scale;
    s16 i;

    target = pos;
    if (flags & 0x8000) {
        target = NULL;
    }
    flags &= 0x7fff;
    obj = DAT_80185da8.objects;
    for (i = 0; i < 200; i++, obj++) {
        if (obj->unk9 != 1 || obj == DAT_80185da8.current) {
            continue;
        }
        if (mode == 0x8000) {
            dist = func_80016fb4(pos, reach, &obj->pos, obj->unk1c, obj->unk1e);
        } else if (mode == 0x8001) {
            if (pos->vy < obj->pos.vy - obj->unk1e) {
                dist = -9999999;
            } else {
                dist = func_80016fb4(pos, reach, &obj->pos, obj->unk1c, obj->unk1e);
            }
        } else if (mode == 0x8002) {
            dist = func_80016fb4(pos, reach, &obj->pos, obj->unk1c, obj->unk1e);
            angle = (s16)ratan2(obj->pos.vz - pos->vz, obj->pos.vx - pos->vx);
            side = func_80016a2c(angle, pos->pad + 1024, 0x40);
            if (side == 1 && func_80016a2c(angle, pos->pad - 1024, 0x40) == side) {
                dist = -9999999;
            }
        } else {
            dist = func_80016ec8(&obj->pos, pos->vx, pos->vy, pos->vz, reach, obj->unk1e, mode);
            if (dist == -1) {
                dist = -9999999;
            }
        }
        if (dist < near) {
            continue;
        }
        if (falloff != 0x1000) {
            scale = (dist << 12) / reach;
            scale = (u32)(scale * (0x1000 - falloff)) >> 12;
            scale = 0x1000 - scale;
            scale = flags * scale >> 12;
        } else {
            scale = flags;
        }
        func_8004c668(i, a5, a6, a7, a8, a9, a10, a11, a12, a13, scale, a15, rec, target);
    }
}

/* Fires (func_8002ab18) at the player when the player is within reach and the cone. */
ADDRESS(0x8004d254, 0x170)
s32 func_8004d254(s32 near, s32 range, s32 height, s32 cone, u16 a4, u16 a5, u16 a6, u16 a7)
{
    s32 offset = height << 5;
    Object88 *obj = DAT_80185da8.current;
    VECTOR *target;
    VECTOR pos;
    s32 dist;
    s32 angle;

    near <<= 6;
    range <<= 6;
    cone <<= 4;
    pos.vx = obj->pos.vx;
    pos.vy = obj->pos.vy - offset;
    pos.vz = obj->pos.vz;
    dist = func_80016ec8(&pos, DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vy, DAT_801b24e4.pos.vz, range, 0, 0x6a4);
    if (dist == -1 || dist < near) {
        return 0;
    }

    target = &DAT_801b24e4.pos;
    angle = func_80016ab8(target->vx - pos.vx, target->vz - pos.vz);
    if (!func_80016a2c(obj->angle.vy, angle, cone)) {
        return 0;
    }

    func_8002ab18(a4, a5, a6, a7, 0, 0, 0, 0, 0, 0x1000, 10, &pos);
    return 1;
}

/* The active record best inside the cone from pos (yaw, pitch); its distance in *dist. */
ADDRESS(0x8004d3c4, 0x280)
Object88 *func_8004d3c4(VECTOR *pos, s16 yaw, s16 pitch, s32 range, s32 max_yaw, s32 max_pitch,
                        s32 *dist, s32 jitter)
{
    Object88 *obj;
    Object88 *best;
    SVECTOR angle;
    s32 best_score;
    s32 best_dist;
    s32 d;
    s32 r;
    s32 score;
    u16 i;

    best_score = 30000;
    best_dist = -1;
    best = NULL;
    obj = DAT_80185da8.objects;
    i = 200;
    while (i--) {
        if (obj->unk9 == 1 && obj->unke != 3 && obj != DAT_80185da8.current) {
            r = range;
            if (obj->unk28 & 0x20000) {
                r <<= 1;
            }
            d = func_80016ec8(&obj->pos, pos->vx, 0xffff, pos->vz, r, 0, 0);
            if (d != -1) {
                func_80016c90(obj->pos.vx - pos->vx, obj->pos.vy - pos->vy, obj->pos.vz - pos->vz, &angle);
                angle.vy = (angle.vy - yaw) & 0xfff;
                if (angle.vy >= 2048) {
                    angle.vy = 0x1000 - angle.vy;
                }
                angle.vx = (angle.vx - pitch) & 0xfff;
                if (angle.vx >= 2048) {
                    angle.vx = 0x1000 - angle.vx;
                }
                if ((obj->unk28 & 0x20000) && jitter >= 0) {
                    d >>= 2;
                    angle.vx -= 512;
                    angle.vy -= 512;
                } else if (angle.vy > max_yaw || angle.vx > max_pitch) {
                    goto next;
                }
                score = angle.vy + angle.vx + (d >> 7);
                if (jitter > 0) {
                    score += (rand() * jitter) >> 15;
                }
                if (score < best_score) {
                    best_score = score;
                    best = obj;
                    best_dist = d;
                }
            }
        }
    next:
        obj++;
    }
    *dist = best_dist;
    return best;
}

/* The first active record (not current) whose reach holds the point; -1 for none. */
ADDRESS(0x8004d644, 0x1f4)
s32 func_8004d644(s32 x, s32 y, s32 z, s32 radius, s32 reach_up)
{
    Object88 *obj;
    Object78 *def;
    VECTOR pos;
    s32 i;

    obj = DAT_80185da8.objects;
    for (i = 0; i < 200; i++, obj++) {
        if (obj->unk9 != 1) {
            continue;
        }
        if (obj->unke == 3) {
            continue;
        }
        if (DAT_80185da8.unk9d00 & obj->unk28) {
            continue;
        }
        if (obj == DAT_80185da8.current) {
            continue;
        }
        if (obj->unk28 & 0x10) {
            if (obj->unk22 == DAT_80185da8.unk9d18) {
                continue;
            }
            pos.vx = obj->pos.vx;
            pos.vz = obj->pos.vz;
            pos.vy = obj->unk26 + obj->pos.vy;
            if (func_80016ec8(&pos, x, y, z, obj->unk1c + radius, obj->unk1e, reach_up) != -1) {
                return i;
            }
        } else if (obj->unk28 & 0x400000) {
            def = &DAT_80185da8.defs[obj->unk2];
            func_8004ede0(obj, def->unk0 == 0x6a ? 0x46 : 0, &pos);
            pos.vx += obj->pos.vx;
            pos.vz += obj->pos.vz;
            pos.vy += obj->pos.vy;
            if (func_80016ec8(&pos, x, y, z, obj->unk1c + radius, obj->unk1e, reach_up) != -1) {
                return i;
            }
        } else if (func_80016ec8(&obj->pos, x, y, z, obj->unk1c + radius, obj->unk1e, reach_up) != -1) {
            return i;
        }
    }
    return -1;
}

/* As func_8004d644, for every active record that is not of kind 0x6e. */
ADDRESS(0x8004d838, 0x1f4)
s32 func_8004d838(s32 x, s32 y, s32 z, s32 radius, s32 reach_up)
{
    Object88 *obj;
    Object78 *def;
    VECTOR pos;
    s32 i;

    obj = DAT_80185da8.objects;
    for (i = 0; i < 200; i++, obj++) {
        if (obj->unk9 != 1) {
            continue;
        }
        if (DAT_80185da8.unk9d00 & obj->unk28) {
            continue;
        }
        if (obj->unk1 == 0x6e) {
            continue;
        }
        if (obj == DAT_80185da8.current) {
            continue;
        }
        if (obj->unk28 & 0x10) {
            if (obj->unk22 == DAT_80185da8.unk9d18) {
                continue;
            }
            pos.vx = obj->pos.vx;
            pos.vz = obj->pos.vz;
            pos.vy = obj->unk26 + obj->pos.vy;
            if (func_80016ec8(&pos, x, y, z, obj->unk1c + radius, obj->unk1e, reach_up) != -1) {
                return i;
            }
        } else if (obj->unk28 & 0x400000) {
            def = &DAT_80185da8.defs[obj->unk2];
            func_8004ede0(obj, def->unk0 == 0x6a ? 0x46 : 0, &pos);
            pos.vx += obj->pos.vx;
            pos.vz += obj->pos.vz;
            pos.vy += obj->pos.vy;
            if (func_80016ec8(&pos, x, y, z, obj->unk1c + radius, obj->unk1e, reach_up) != -1) {
                return i;
            }
        } else if (func_80016ec8(&obj->pos, x, y, z, obj->unk1c + radius, obj->unk1e, reach_up) != -1) {
            return i;
        }
    }
    return -1;
}

/* Makes obj the current record (0 clears the pointers). */
ADDRESS(0x8004da2c, 0xdc)
void func_8004da2c(Object88 *obj)
{
    if (obj != NULL) {
        DAT_80185da8.current = obj;
        DAT_80185da8.def = &DAT_80185da8.defs[obj->unk2];
        DAT_80185da8.unk9d1c = obj->unk2;
        if (DAT_80185da8.def->unk34 & 0x10) {
            DAT_80185da8.target = &DAT_80185da8.objects[obj->unk22];
            DAT_80185da8.target_def = &DAT_80185da8.defs[DAT_80185da8.target->unk2];
        }
    } else {
        DAT_80185da8.current = NULL;
        DAT_80185da8.def = NULL;
        DAT_80185da8.unk9d1c = 0;
        DAT_80185da8.target = NULL;
        DAT_80185da8.target_def = NULL;
    }
}

/* Steps the animation time, wrapping. */
ADDRESS(0x8004db08, 0x34)
void func_8004db08(Object88 *obj, s16 step)
{
    if (step < 0) {
        obj->unk66 = -step;
    } else {
        obj->unk66 = step;
    }
    obj->unk18 = (step + obj->unk18) & 0xfff;
}

/* Steps the animation time, holding at either end. */
ADDRESS(0x8004db3c, 0x5c)
void func_8004db3c(Object88 *obj, s16 step)
{
    s16 time;

    if (step < 0) {
        obj->unk66 = -step;
    } else {
        obj->unk66 = step;
    }
    obj->unk18 = time = step + obj->unk18;
    if (time >= 0x1000) {
        obj->unk18 = 0xfff;
    } else if (time < 0) {
        obj->unk18 = 0;
    }
}

/* Whether the last time step passed time. */
ADDRESS(0x8004db98, 0x30)
s32 func_8004db98(Object88 *obj, u16 time)
{
    s32 passed;

    return time < obj->unk18 && time >= obj->unk18 - obj->unk66;
}

/*
 * Moves the current record horizontally by vel, sliding along walls and
 * turning off slopes as mode allows (bits 0-1 slide, 2 stop at a drop, 3
 * write the final step back, 4 and 5 more wall rules). Returns the hit bits.
 */
ADDRESS(0x8004dbc8, 0x4cc)
s32 func_8004dbc8(SVECTOR *vel, s32 mode)
{
    Object88 *obj = DAT_80185da8.current;
    s32 dx;
    s32 dz;
    s32 dx0;
    s32 dz0;
    VECTOR p;
    s32 hit;
    s32 slid;
    s32 cornered;
    s32 turns;
    s32 hits = 0;

    if (vel->vx == 0 && vel->vz == 0) {
        return 0;
    }
    turns = 0;
    cornered = 0;
    slid = 0;
    dx0 = dx = vel->vx;
    dz0 = dz = vel->vz;

retry:
    p.vx = obj->pos.vx + dx;
    p.vz = obj->pos.vz + dz;
    hit = func_80033f38(p.vx, obj->pos.vy, p.vz, obj->unk1c, OBJECT88_SHAPE(obj), DAT_80185da8.unk9d04);
    if (hit == 0) {
    clear:
        if (obj->unkd == 0 && (mode & 0x24)) {
            s32 floor = DAT_801d4464.unk12010;
            s32 step_x;
            s32 step_z;

            if (dx == 0) {
                step_x = 0;
            } else if (dx > 0) {
                step_x = obj->unk1c * 2;
            } else {
                step_x = -obj->unk1c * 2;
            }
            if (dz == 0) {
                step_z = 0;
            } else if (dz > 0) {
                step_z = obj->unk1c * 2;
            } else {
                step_z = -obj->unk1c * 2;
            }
            func_80033d08(obj->pos.vx, obj->pos.vy, obj->pos.vz + step_z, obj->unk1c, OBJECT88_SHAPE(obj));
            if (floor < DAT_801d4464.unk12010) {
                floor = DAT_801d4464.unk12010;
            }
            func_80033d08(obj->pos.vx + step_x, obj->pos.vy, obj->pos.vz, obj->unk1c, OBJECT88_SHAPE(obj));
            if (floor < DAT_801d4464.unk12010) {
                floor = DAT_801d4464.unk12010;
            }
            if (obj->pos.vy < floor - 1100) {
                hits |= 0x100;
                if (mode & 4) {
                    goto blocked;
                }
            }
        }
        obj->pos.vx = p.vx;
        obj->pos.vz = p.vz;
        goto finish;
    }

    hits |= hit;
    if (!(mode & 3)) {
        goto finish;
    }
    if (hit & 0xb0) {
        s32 angle;
        s32 heading;
        s32 len;

        turns++;
        if (turns == 2) {
            goto finish;
        }
        func_80033d84();
        angle = func_80016ab8(DAT_801d4464.unk1203c.vx - obj->pos.vx, DAT_801d4464.unk1203c.vz - obj->pos.vz);
        heading = func_80016ab8(dx, dz);
        if (func_80016a68(heading, angle)) {
            angle = (angle + 1024) & 0xfff;
        } else {
            angle = (angle - 1024) & 0xfff;
        }
        len = SquareRoot0(dx * dx + dz * dz);
        dx = -(rsin(angle) * len) >> 13;
        dz = (rcos(angle) * len) >> 13;
        goto retry;
    }
    if (!(mode & 0x10)) {
        if (obj->unk28 & 0x4000) {
            goto blocked;
        }
        if (!(hit & ~0x205)) {
            if (obj->pos.vy <= DAT_801d4464.unk12010 + 800) {
                goto clear;
            }
            goto blocked;
        }
    }
    if (!(hit & 1)) {
        goto diagonal;
    }

blocked:
    if (slid) {
        dz = 0;
        dx = 0;
        goto finish;
    }
    if (dx != 0) {
        dx = 0;
        if (mode & 1) {
            s32 len = SquareRoot0(dx0 * dx0 + dz0 * dz0);

            dz = len;
            if (dz0 <= 0) {
                dz = -dz;
            }
        } else {
            dz = dz0;
        }
        goto retry;
    }
    if (dz != 0) {
        dz = 0;
        if (mode & 1) {
            s32 len = SquareRoot0(dx0 * dx0 + dz0 * dz0);

            dx = len;
            if (dx0 <= 0) {
                dx = -dx;
            }
        } else {
            dz = dz0;
        }
        slid = 1;
        goto retry;
    }
diagonal:
    if (!(hit & 2)) {
        goto finish;
    }
    if (cornered) {
        goto blocked;
    }
    switch (DAT_801d4464.unk12004[2] & 3) {
    case 0:
    case 2:
        dx = (dx0 + dz0) >> 1;
        dz = dx;
        break;
    default:
        dx = (dx0 - dz0) >> 1;
        dz = -dx;
        break;
    }
    cornered = 1;
    goto retry;

finish:
    if (mode & 8) {
        vel->vx = dx;
        vel->vz = dz;
    }
    return hits;
}
