/*
 * GAME.EXE: starting a spell and the wall-sliding mover (same WIP module as
 * src/game/player.c, after the unreconstructed player_update_weapon_attack; contiguous
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
 * Starts spell id (MagicRecord): checks and pays its unk1c cost, then sets the
 * repeat count unk106, the interval unk107 and the direction unk14c that
 * player_update_actions_and_charge casts it with.
 */
ADDRESS(0x8002deec, 0x50c)
void player_select_magic_action(s32 id)
{
    SVECTOR unused;
    MagicRecord *r;
    s32 cost;

    if (cur_avatar_status.unk105 != 0xff) {
        return;
    }
    if (id == 0xff) {
        return;
    }
    r = &effect_status.unk0[id];
    cost = r->mp_cost;
    if (cur_avatar_status.unkf0 == 0x26) {
        cost <<= 1;
    }
    if (cur_avatar_status.unkf1 == 0x2e) {
        cost >>= 1;
    }
    if (cur_avatar_status.unk1c < cost) {
        notify_enqueue(9);
        return;
    }
    if (cur_avatar_status.unkcb == 0x1f) {
        return;
    }
    switch (id) {
    case 26:
        if (cur_avatar_status.unk86 != 0) {
            return;
        }
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 27:
        if (cur_avatar_status.unk88 != 0) {
            return;
        }
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 28:
        if (cur_avatar_status.unk8a != 0) {
            return;
        }
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 25:
        if (cur_avatar_status.unk78 == 0) {
            return;
        }
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 23:
        if (cur_avatar_status.hp == cur_avatar_status.hp_max) {
            return;
        }
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 29:
        if (cur_avatar_status.unk8e != 0) {
            return;
        }
        cur_avatar_status.unk8e = 900;
        cur_avatar_status.unk1c -= cost;
        player_increment_magic_training(r);
        return;
    case 30:
        if (cur_avatar_status.hp == cur_avatar_status.hp_max && cur_avatar_status.unk7a == 0 && cur_avatar_status.unk7e == 0
            && cur_avatar_status.unk82 == 0 && cur_avatar_status.unk84 == 0) {
            return;
        }
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    }
    if (cur_avatar_status.unk22 < 5000) {
        return;
    }
    switch (id) {
    case 23:
    case 25:
    case 26:
    case 27:
    case 28:
    case 30:
        player_increment_magic_training(r);
        break;
    }
    cur_avatar_status.unk14c.vx = -200;
    cur_avatar_status.unk14c.vy = 200;
    cur_avatar_status.unk14c.vz = 400;
    cur_avatar_status.unk22 = 0;
    cur_avatar_status.unkf = 0;
    cur_avatar_status.unk105 = id;
    cur_avatar_status.unk108 = 1;
    cur_avatar_status.unk1c -= cost;
    switch (id) {
    case 4:
        cur_avatar_status.unk14c.vy = -512;
        cur_avatar_status.unk14c.vx = 0;
        cur_avatar_status.unk14c.vz = 2000;
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
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 76:
        cur_avatar_status.unk14c.vz = 600;
        cur_avatar_status.unk14c.vx = 0;
        cur_avatar_status.unk14c.vy = 0;
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 1:
        cur_avatar_status.unk14c.vx = -2000;
        cur_avatar_status.unk14c.vy = 1600;
        cur_avatar_status.unk14c.vz = 1000;
        cur_avatar_status.unk106 = 5;
        cur_avatar_status.unk107 = 3;
        break;
    case 56:
        cur_avatar_status.unk14c.vy = 500;
        cur_avatar_status.unk106 = 3;
        cur_avatar_status.unk107 = 1;
        break;
    case 13:
        cur_avatar_status.unk14c.vy = 500;
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 64:
        cur_avatar_status.unk106 = 3;
        cur_avatar_status.unk107 = 1;
        break;
    case 66:
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 9:
        cur_avatar_status.unk14c.vy = -400;
        cur_avatar_status.unk14c.vz = 400;
        cur_avatar_status.unk14c.vx = 0;
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 17:
        cur_avatar_status.unk14c.vz = 600;
        cur_avatar_status.unk14c.vy = -600;
        cur_avatar_status.unk14c.vx = 0;
        cur_avatar_status.unk106 = 1;
        cur_avatar_status.unk107 = 1;
        break;
    case 20:
    case 71:
        cur_avatar_status.unk14c.vx = -200;
        cur_avatar_status.unk106 = 5;
        cur_avatar_status.unk107 = 2;
        break;
    case 2:
        cur_avatar_status.unk106 = 3;
        cur_avatar_status.unk107 = 1;
        break;
    case 11:
        cur_avatar_status.unk106 = 12;
        cur_avatar_status.unk107 = 1;
        break;
    case 7:
        cur_avatar_status.unk106 = 2;
        cur_avatar_status.unk107 = 1;
        cur_avatar_status.unk14c.vz = 0;
        cur_avatar_status.unk14c.vy = 0;
        cur_avatar_status.unk14c.vx = 0;
        break;
    case 8:
        cur_avatar_status.unk106 = 4;
        cur_avatar_status.unk107 = 2;
        break;
    }
    cur_avatar_status.unkac = r;
}

/*
 * Moves the player speed units along angle (4096ths of a turn), retrying
 * around what collision_query_world hit: the push of the last test, up to six turns
 * along a pushing MapObject (unk3 bit 2), around a wall point, shorter steps,
 * one axis at a time, and along the slope of the hit record. Returns 1 when
 * the player moved freely; unk11c keeps the last step tried.
 */
ADDRESS(0x8002e3f8, 0x8c4)
s32 player_move_horizontal(s32 angle, s32 speed)
{
    VECTOR p;
    MapObject *obj;
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
    p.vx = dx + cur_avatar_status.pos.vx;
    p.vz = dz + cur_avatar_status.pos.vz;
    hit = collision_query_world(p.vx, cur_avatar_status.pos.vy, p.vz, 800, 1700, 0x31);
    if (hit == 0) {
    free:
        if (map_world.collision_slope_push >= 64) {
            s32 px;
            s32 pz;

            px = rcos(-map_world.collision_slope_angle - 1024) * map_world.collision_slope_push >> 12;
            pz = rsin(-map_world.collision_slope_angle - 1024) * map_world.collision_slope_push >> 12;
            if (!pushed) {
                pushed = 1;
                dx += px;
                dz += pz;
                goto retry;
            }
        }
        moved = 1;
        cur_avatar_status.pos.vx = p.vx;
        cur_avatar_status.pos.vz = p.vz;
        cur_avatar_status.unk160 = map_world.collision_layer_offset;
        goto done;
    }
    blocked = 0;
    if (!(hit & ~0x207)) {
        if (map_world.collision_floor_height + 1280 >= cur_avatar_status.pos.vy && cur_avatar_status.unk101 == 0
            && map_world.collision_height_limit - map_world.collision_floor_height < -1700) {
            goto free;
        }
        if (!(hit & 2)) {
            blocked = 1;
        }
    }
    if (map_world.collision_object_index != -1) {
        obj = &map_objects_slots.objects[map_world.collision_object_index];
        if (obj->flags & 4) {
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
        collision_cache_load_hit_bounds();
        wx = map_world.collision_hit_position.vx - cur_avatar_status.pos.vx;
        wz = map_world.collision_hit_position.vz - cur_avatar_status.pos.vz;
        a = ratan2(dz - wz, dx - wx);
        r = map_world.collision_hit_radius + 880;
        dx = rcos(a) * r / 4096;
        dz = rsin(a) * r / 4096;
        p.vx = dx + map_world.collision_hit_position.vx;
        dx = p.vx - cur_avatar_status.pos.vx;
        p.vz = dz + map_world.collision_hit_position.vz;
        dz = p.vz - cur_avatar_status.pos.vz;
        goto retry;
    }
    if (!stepped) {
        for (step = speed - 22; step >= 0; step -= 22) {
            p.vx = cur_avatar_status.pos.vx + rcos(angle) * step / 4096;
            p.vz = cur_avatar_status.pos.vz + rsin(angle) * step / 4096;
            if (collision_query_world(p.vx, cur_avatar_status.pos.vy, p.vz, 800, 1700, 0x31) == 0) {
                cur_avatar_status.pos.vx = p.vx;
                cur_avatar_status.pos.vz = p.vz;
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
        slope = map_world.collision_layer[2] & 3;
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
    cur_avatar_status.unk11c.vx = dx;
    cur_avatar_status.unk11c.vz = dz;
    return moved;
}
