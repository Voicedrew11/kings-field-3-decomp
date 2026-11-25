/*
 * GAME.EXE: player state, continued (same WIP module as src/game/player.c;
 * split from it at the unreconstructed player_dispatch_magic_effect): the unk101 modes and
 * their parameters, hp/unk1c changes and the per-frame effect timers.
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
#include <game/menu.h>
#include <game/entity.h>
#include <game/misc.h>
#include <game/scene.h>
#include <kf/lib/null.h>

RODATA(0x80011a78, 0x44)

/* The pad bits of the sequence player_update_actions_and_charge follows, 0xffff ended (rebuilt
 * every frame). */
DATA(0x8009c3cc, 0x8, ".sbss")
static u16 player_magic_attack_masks[4];

/* A fall from 20480 above the floor at speed 400 or more is death. */
ADDRESS(0x8002ecbc, 0x60)
void player_check_fall_death(void)
{
    if (cur_avatar_status.unk172 >= 400 && map_world.collision_floor_height - cur_avatar_status.pos.vy > 20480) {
        player_death_begin(NULL);
        cur_avatar_status.unk109 = 1;
    }
}

/* The landing sound, louder with the fall speed. */
ADDRESS(0x8002ed1c, 0x44)
void player_play_landing_sound(s32 speed)
{
    if (speed >= 320) {
        speed -= 320;
        if (speed > 896) {
            speed = 0x380;
        }
        audio_play_sound(7, (speed >> 3) + 32);
    }
}

/* Vertical movement per unk104 fall state: steps up and down, falls, jumps and landings. */
ADDRESS(0x8002ed60, 0x5c0)
void player_update_vertical_motion(void)
{
    s32 y;
    s32 hit;
    s32 d;
    s32 v;
    s32 speed;
    s32 bob;

    collision_probe_floor_height(cur_avatar_status.pos.vx, cur_avatar_status.pos.vy, cur_avatar_status.pos.vz,
                                 800, 1700);
    cur_avatar_status.unk11c.vy = 0;
    switch (cur_avatar_status.unk104) {
    case 0x10:
        player_check_fall_death();
        cur_avatar_status.pos.vy += cur_avatar_status.unk172;
        cur_avatar_status.unk11c.vy = cur_avatar_status.unk172;
        cur_avatar_status.unk172 += 80;
        if (cur_avatar_status.pos.vy > map_world.collision_floor_height + 100) {
            cur_avatar_status.pos.vy = map_world.collision_floor_height;
            cur_avatar_status.unk104 = 0;
        }
        break;
    case 0x20:
        player_check_fall_death();
        cur_avatar_status.pos.vy += cur_avatar_status.unk172;
        if (cur_avatar_status.pos.vy <= map_world.collision_floor_height || cur_avatar_status.unk172 >= 0) {
            if (cur_avatar_status.pos.vy - cur_avatar_status.unk172 > map_world.collision_floor_height) {
                cur_avatar_status.pos.vy = map_world.collision_floor_height;
            }
            cur_avatar_status.unk104 = 0;
        }
        cur_avatar_status.unk11c.vy = cur_avatar_status.unk172;
        cur_avatar_status.unk172 += 10;
        break;
    case 0x40:
        player_check_fall_death();
        y = cur_avatar_status.unk172 + cur_avatar_status.pos.vy;
        hit = collision_query_world(cur_avatar_status.pos.vx, y, cur_avatar_status.pos.vz, 800, 1700, 0x31);
        if (hit == 0) {
            cur_avatar_status.pos.vy = y;
            cur_avatar_status.unk11c.vy = cur_avatar_status.unk172;
            cur_avatar_status.unk172 += 40;
            cur_avatar_status.unk144 = cur_avatar_status.unk172 >> 1;
            break;
        }
        if (cur_avatar_status.unk172 < 0) {
            cur_avatar_status.unk172 = 0;
            break;
        }
        player_play_landing_sound(cur_avatar_status.unk172);
        if (cur_avatar_status.unk172 >= 480) {
            v = cur_avatar_status.unk172 * cur_avatar_status.unk172 >> 12;
            player_apply_damage_reaction(NULL, v * v * v / 118000, 0);
        }
        if (hit & 4) {
            cur_avatar_status.pos.vy = map_world.collision_floor_height;
            if (cur_avatar_status.unk172 >= 512) {
                cur_avatar_status.pos.vy = map_world.collision_floor_height - (cur_avatar_status.unk172 >> 1);
            }
        } else {
            collision_cache_load_hit_bounds();
            y = map_world.collision_hit_position.vy - map_world.collision_hit_height - 1;
            hit = collision_query_world(cur_avatar_status.pos.vx, y, cur_avatar_status.pos.vz, 800, 1700,
                                        0x31);
            if (hit == 0) {
                cur_avatar_status.pos.vy = y;
            }
        }
        cur_avatar_status.unk170 = 1;
        cur_avatar_status.unk104 = 0x50;
        /* fall through */
    case 0x50:
        if (cur_avatar_status.unk170 > 0) {
            cur_avatar_status.unk170 += cur_avatar_status.unk172 >> 2;
        }
        cur_avatar_status.unk172 -= 300;
        if (cur_avatar_status.unk144 > 0) {
            if (cur_avatar_status.unk172 > 0) {
                cur_avatar_status.unk144 += 10;
            } else {
                cur_avatar_status.unk144 -= 30;
            }
        }
        if (cur_avatar_status.unk170 <= 0 && cur_avatar_status.unk144 <= 0) {
            cur_avatar_status.unk104 = 0;
            cur_avatar_status.unk172 = 0;
            cur_avatar_status.unk170 = 0;
            cur_avatar_status.unk144 = 0;
        }
        /* fall through */
    case 0:
        d = map_world.collision_floor_height - cur_avatar_status.pos.vy;
        if (d < 0) {
            if (d >= -256) {
                if (d < -128) {
                    cur_avatar_status.pos.vy -= 128;
                } else {
                    cur_avatar_status.pos.vy = map_world.collision_floor_height;
                }
                break;
            }
            if (d >= -512) {
                cur_avatar_status.pos.vy -= 256;
                break;
            }
            if (d >= -1024) {
                cur_avatar_status.pos.vy -= 512;
                break;
            }
            speed = cur_avatar_status.unk166;
            cur_avatar_status.unk104 = 0x20;
            cur_avatar_status.unk172 = speed > 200 ? -300 : -150;
        } else {
            if (d <= 0) {
                break;
            }
            hit = collision_query_world(cur_avatar_status.pos.vx, cur_avatar_status.pos.vy + 1,
                                        cur_avatar_status.pos.vz, 800, 1700, 0x31);
            if (hit != 0) {
                break;
            }
            if (d <= 256) {
                if (d > 128) {
                    cur_avatar_status.pos.vy += 128;
                } else {
                    cur_avatar_status.pos.vy = map_world.collision_floor_height;
                }
                break;
            }
            if (d <= 512) {
                cur_avatar_status.pos.vy += 256;
                break;
            }
            cur_avatar_status.unk104 = d <= 1024 ? 0x10 : 0x40;
            cur_avatar_status.unk172 = 40;
        }
        cur_avatar_status.unk170 = 0;
        cur_avatar_status.unk144 = 0;
        break;
    }
    if (cur_avatar_status.unk104 == 0) {
        if (cur_avatar_status.unkfc != 0) {
            cur_avatar_status.unk16e = (cur_avatar_status.unk16e + cur_avatar_status.unk166) & 0xfff;
            bob = rsin(cur_avatar_status.unk16e) >> 5;
            if (bob < 0) {
                bob = -bob;
            }
            cur_avatar_status.unk16c = bob - (bob >> 2);
        } else {
            cur_avatar_status.unk16c = 0;
        }
    }
    player_update_collision_bounds();
}

/* Mode 0x10 flight: moves the player along unk188.knock.dir, sliding on walls; 1 when it lands. */
ADDRESS(0x8002f320, 0x2a0)
s32 player_move_reaction_with_collision(void)
{
    VECTOR p;
    s32 hit;
    s32 friction;
    s32 d;

    p.vx = cur_avatar_status.pos.vx + cur_avatar_status.unk188.knock.dir.vx;
    p.vy = cur_avatar_status.pos.vy + cur_avatar_status.unk188.knock.dir.vy;
    p.vz = cur_avatar_status.pos.vz + cur_avatar_status.unk188.knock.dir.vz;
    if (collision_query_world(p.vx, p.vy, p.vz, 800, 1700, 0x31) == 0) {
    moved:
        if (cur_avatar_status.unk188.knock.dir.vy >= 300 && map_world.collision_floor_height - cur_avatar_status.pos.vy > 20480) {
            player_death_begin(NULL);
            cur_avatar_status.unk109 = 1;
        }
        cur_avatar_status.pos.vx = p.vx;
        cur_avatar_status.pos.vy = p.vy;
        cur_avatar_status.pos.vz = p.vz;
        cur_avatar_status.unk188.knock.dir.vy += 32;
    } else {
        p.vy = cur_avatar_status.pos.vy;
        hit = collision_query_world(p.vx, p.vy, p.vz, 800, 1700, 0x31);
        if (hit == 0) {
            cur_avatar_status.unk188.knock.dir.vy = 1;
            hit = collision_query_world(p.vx, p.vy, p.vz, 800, 1700, 0x31);
            if (hit == 0) {
                friction = 0x20;
            slide:
                d = fixed_vector2_length(cur_avatar_status.unk188.knock.dir.vx,
                                         cur_avatar_status.unk188.knock.dir.vz);
                if (d <= friction) {
                    cur_avatar_status.unk188.knock.dir.vz = 0;
                    cur_avatar_status.unk188.knock.dir.vx = 0;
                    goto stop;
                }
                cur_avatar_status.unk188.knock.dir.vx = cur_avatar_status.unk188.knock.dir.vx * (d - friction) / d;
                cur_avatar_status.unk188.knock.dir.vz = cur_avatar_status.unk188.knock.dir.vz * (d - friction) / d;
                goto moved;
            }
        }
        if ((hit & ~0x205) || map_world.collision_floor_height + 256 < cur_avatar_status.pos.vy) {
            return 1;
        }
        p.vy = map_world.collision_floor_height;
        friction = 0x38;
        goto slide;
    stop:
        return 1;
    }
    player_update_collision_bounds();
    return 0;
}

/* Turning (unk168 into the yaw unk12c.vy) and looking up and down (unk16a into the pitch). */
ADDRESS(0x8002f5c0, 0x3fc)
void player_update_camera_rotation(void)
{
    s16 pitch;
    s32 limit;

    if (cur_avatar_status.unk178 & action_button_masks[2]) {
        cur_avatar_status.unk168 += cur_avatar_status.unk184 >> 2;
        if (cur_avatar_status.unk168 > cur_avatar_status.unk184) {
            cur_avatar_status.unk168 = cur_avatar_status.unk184;
        }
    } else if (cur_avatar_status.unk178 & action_button_masks[3]) {
        cur_avatar_status.unk168 -= cur_avatar_status.unk184 >> 2;
        if (cur_avatar_status.unk168 < -cur_avatar_status.unk184) {
            cur_avatar_status.unk168 = -cur_avatar_status.unk184;
        }
    } else if (cur_avatar_status.unk168 > 0) {
        cur_avatar_status.unk168 -= cur_avatar_status.unk184 >> 2;
        if (cur_avatar_status.unk168 < 0) {
            cur_avatar_status.unk168 = 0;
        }
    } else if (cur_avatar_status.unk168 < 0) {
        cur_avatar_status.unk168 += cur_avatar_status.unk184 >> 2;
        if (cur_avatar_status.unk168 > 0) {
            cur_avatar_status.unk168 = 0;
        }
    }
    cur_avatar_status.unk12c.vy = (cur_avatar_status.unk12c.vy + cur_avatar_status.unk168) & 0xfff;
    if (cur_avatar_status.unk178 & action_button_masks[11]) {
        if (cur_avatar_status.unk178 & action_button_masks[9]) {
            pitch = cur_avatar_status.unk12c.vx;
            if (pitch > 2048) {
                limit = (0x1000 - pitch) >> 2;
                cur_avatar_status.unk16a += 8;
                if (cur_avatar_status.unk16a > limit) {
                    cur_avatar_status.unk16a = limit;
                    if (limit == 0) {
                        cur_avatar_status.unk16a = 0x1000 - cur_avatar_status.unk12c.vx;
                    }
                }
            } else if (pitch < 2048) {
                cur_avatar_status.unk16a -= 8;
                if (cur_avatar_status.unk16a < -(pitch >> 2)) {
                    cur_avatar_status.unk16a = -(pitch >> 2);
                    if (cur_avatar_status.unk16a == 0) {
                        cur_avatar_status.unk16a = -cur_avatar_status.unk12c.vx;
                    }
                }
            }
        } else {
            cur_avatar_status.unk16a += 3;
            if (cur_avatar_status.unk16a > 32) {
                cur_avatar_status.unk16a = 32;
            }
        }
    } else if (cur_avatar_status.unk178 & action_button_masks[9]) {
        cur_avatar_status.unk16a -= 3;
        if (cur_avatar_status.unk16a < -32) {
            cur_avatar_status.unk16a = -32;
        }
    } else if (cur_avatar_status.unk16a > 0) {
        cur_avatar_status.unk16a -= 3;
        if (cur_avatar_status.unk16a < 0) {
            cur_avatar_status.unk16a = 0;
        }
    } else if (cur_avatar_status.unk16a < 0) {
        cur_avatar_status.unk16a += 3;
        if (cur_avatar_status.unk16a > 0) {
            cur_avatar_status.unk16a = 0;
        }
    }
    if (cur_avatar_status.unk16a > 0) {
        cur_avatar_status.unk12c.vx = (cur_avatar_status.unk12c.vx + cur_avatar_status.unk16a) & 0xfff;
        if (angle_signed_greater_than(cur_avatar_status.unk12c.vx, 700)) {
            cur_avatar_status.unk12c.vx = 700;
        }
    } else if (cur_avatar_status.unk16a < 0) {
        cur_avatar_status.unk12c.vx = (cur_avatar_status.unk12c.vx + cur_avatar_status.unk16a) & 0xfff;
        if (angle_signed_greater_than(0xd44, cur_avatar_status.unk12c.vx)) {
            cur_avatar_status.unk12c.vx = 0xd44;
        }
    }
}

/* Walking (unk164) and strafing (unk162), then the move along the yaw. */
ADDRESS(0x8002f9bc, 0x420)
void player_update_horizontal_motion(void)
{
    s16 side;
    s16 ahead;
    s32 sx;
    s32 sz;
    s16 len;

    if (cur_avatar_status.unk178 & action_button_masks[0]) {
        ahead = cur_avatar_status.unk164 + (cur_avatar_status.unk180 >> 2);
        if (ahead > cur_avatar_status.unk180) {
            cur_avatar_status.unk164 = cur_avatar_status.unk180;
        } else {
            cur_avatar_status.unk164 = ahead;
        }
    } else if (cur_avatar_status.unk178 & action_button_masks[1]) {
        ahead = cur_avatar_status.unk164 - (cur_avatar_status.unk180 >> 2);
        if (ahead >= -cur_avatar_status.unk180) {
            cur_avatar_status.unk164 = ahead;
        } else {
            cur_avatar_status.unk164 = -cur_avatar_status.unk180;
        }
    } else if (cur_avatar_status.unk164 > 0) {
        cur_avatar_status.unk164 -= cur_avatar_status.unk180 >> 3;
        if (cur_avatar_status.unk164 < 0) {
            cur_avatar_status.unk164 = 0;
        }
    } else if (cur_avatar_status.unk164 < 0) {
        cur_avatar_status.unk164 += cur_avatar_status.unk180 >> 3;
        if (cur_avatar_status.unk164 > 0) {
            cur_avatar_status.unk164 = 0;
        }
    }
    if (cur_avatar_status.unk178 & action_button_masks[10]) {
        side = cur_avatar_status.unk162 + (cur_avatar_status.unk180 >> 2);
        if (side > cur_avatar_status.unk180) {
            cur_avatar_status.unk162 = cur_avatar_status.unk180;
        } else {
            cur_avatar_status.unk162 = side;
        }
    } else if (cur_avatar_status.unk178 & action_button_masks[8]) {
        side = cur_avatar_status.unk162 - (cur_avatar_status.unk180 >> 2);
        if (side >= -cur_avatar_status.unk180) {
            cur_avatar_status.unk162 = side;
        } else {
            cur_avatar_status.unk162 = -cur_avatar_status.unk180;
        }
    } else if (cur_avatar_status.unk162 > 0) {
        cur_avatar_status.unk162 -= cur_avatar_status.unk180 >> 2;
        if (cur_avatar_status.unk162 < 0) {
            cur_avatar_status.unk162 = 0;
        }
    } else if (cur_avatar_status.unk162 < 0) {
        cur_avatar_status.unk162 += cur_avatar_status.unk180 >> 2;
        if (cur_avatar_status.unk162 > 0) {
            cur_avatar_status.unk162 = 0;
        }
    }
    sx = cur_avatar_status.unk162;
    sx *= sx;
    sz = cur_avatar_status.unk164;
    sz *= sz;
    len = SquareRoot0(sx + sz);
    if (len == 0) {
        ahead = 0;
        side = 0;
    } else {
        side = sx / len;
        if (cur_avatar_status.unk162 < 0) {
            side = -side;
        }
        ahead = sz / len;
        if (cur_avatar_status.unk164 < 0) {
            ahead = -ahead;
        }
    }
    cur_avatar_status.unk166 = SquareRoot0(side * side + ahead * ahead);
    player_move_horizontal((cur_avatar_status.unk12c.vy + 1024) & 0xfff, ahead);
    player_move_horizontal(cur_avatar_status.unk12c.vy & 0xfff, side);
}

ADDRESS(0x8002fddc, 0x40)
s32 item_id_is_consumable(s32 id)
{
    switch (id) {
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 119:
    case 120:
        return 1;
    }
    return 0;
}

ADDRESS(0x8002fe1c, 0x660)
void player_update_actions_and_charge(void)
{
    s32 id;
    s32 sum;
    s32 i;
    s32 weight;
    u16 mask;
    u16 *next;
    s32 flag;

    player_magic_attack_masks[3] = 0xffff;
    player_magic_attack_masks[0] = action_button_masks[7];
    player_magic_attack_masks[1] = action_button_masks[4];
    player_magic_attack_masks[2] = action_button_masks[6];
    if (cur_avatar_status.unkce == 0 && (cur_avatar_status.unk178 & action_button_masks[6])
        && !(cur_avatar_status.unk17a & action_button_masks[6]) && cur_avatar_status.unkc6 == 0) {
        player_select_magic_action(cur_avatar_status.unkc7);
    }
    if ((cur_avatar_status.unk178 & action_button_masks[12]) && !(cur_avatar_status.unk17a & action_button_masks[12])) {
        if (cur_avatar_status.unkc8 != 0xff) {
            player_select_magic_action(cur_avatar_status.unkc8);
        }
        if (cur_avatar_status.unkc9 != 0xff) {
            if (event_item_counters_table[0][cur_avatar_status.unkc9] != 0) {
                if (item_id_is_consumable(cur_avatar_status.unkc9)) {
                    menu_apply_item_effect(cur_avatar_status.unkc9);
                }
                event_scene_command_dispatch(&cur_avatar_status.pos, &cur_avatar_status.unk12c,
                                             cur_avatar_status.unkc9);
            } else {
                notify_enqueue(10);
            }
        }
    }
    if (cur_avatar_status.unk105 != 0xff && --cur_avatar_status.unk108 == 0) {
        player_dispatch_magic_effect(cur_avatar_status.unk105);
        if (--cur_avatar_status.unk106 == 0) {
            cur_avatar_status.unk105 = 0xff;
        } else {
            cur_avatar_status.unk108 = cur_avatar_status.unk107;
        }
    }
    if ((cur_avatar_status.unk178 & action_button_masks[7]) && (cur_avatar_status.unk17a & action_button_masks[7])) {
        id = cur_avatar_status.unkf4;
        if (id != 0x4c) {
            if (cur_avatar_status.unk11 != 0) {
                cur_avatar_status.unk11--;
            }
            cur_avatar_status.unkf = 1;
            cur_avatar_status.unk1e -= 500;
            if ((s16)cur_avatar_status.unk1e <= 0) {
                cur_avatar_status.unk1e = 0;
                if (id == 0x4d || id == 0x4f) {
                    flag = 1;
                } else {
                    flag = 0x3c;
                }
                cur_avatar_status.unkf = flag;
            }
            cur_avatar_status.unk22 -= 500;
            cur_avatar_status.unk10 = 1;
            if ((s16)cur_avatar_status.unk22 <= 0) {
                cur_avatar_status.unk22 = 0;
                if (cur_avatar_status.unkf4 != 0x4d && cur_avatar_status.unkf4 != 0x4f) {
                    cur_avatar_status.unk10 = 0x3c;
                } else {
                    cur_avatar_status.unk10 = 0;
                }
            }
            cur_avatar_status.unk14 -= 128;
            if (cur_avatar_status.unk14 < 1024) {
                cur_avatar_status.unk14 = 0x400;
            }
            return;
        }
    }
    sum = 0;
    for (i = 4; i != -1; i--) {
        sum += cur_avatar_status.unk42[i];
    }
    sum /= 5;
    if (cur_avatar_status.unk10 == 0) {
        weight = 0;
        if (cur_avatar_status.unkac != NULL) {
            weight = cur_avatar_status.unkac->charge_rate;
            if (cur_avatar_status.unkf0 == 0x25) {
                weight -= weight >> 2;
            }
            if (cur_avatar_status.unkf1 == 0x30) {
                weight <<= 1;
            }
        }
        cur_avatar_status.unk22 += player_charge_gain_for_rank(sum, weight);
    } else {
        cur_avatar_status.unk10--;
    }
    if (cur_avatar_status.unk22 > 5000) {
        cur_avatar_status.unk22 = 5000;
    }
    cur_avatar_status.unk14 += 128;
    if (cur_avatar_status.unk14 > 4096) {
        cur_avatar_status.unk14 = 0x1000;
    }
    cur_avatar_status.unk11 = 1;
    if (cur_avatar_status.unkc6 != 0) {
        cur_avatar_status.unkc6--;
    } else {
        cur_avatar_status.unka8 = player_magic_attack_masks;
    }
    mask = action_button_masks[7] | action_button_masks[4] | action_button_masks[6];
    if ((cur_avatar_status.unk178 & mask) && !(cur_avatar_status.unk17a & mask)) {
        if (cur_avatar_status.unkb0->alternate_attack_phase_step != 0 && player_meets_weapon_magic_requirement(1)
            && cur_avatar_status.unkb0->alternate_magic_id != 0xff
            && (cur_avatar_status.unk178 & *cur_avatar_status.unka8)
            && (cur_avatar_status.unka8 != player_magic_attack_masks
                || (cur_avatar_status.unk1e == 5000 && cur_avatar_status.unk22 == cur_avatar_status.unk1e))) {
            next = cur_avatar_status.unka8;
            cur_avatar_status.unka8 = next + 1;
            if (next[1] == 0xffff) {
                player_begin_weapon_attack(1);
                return;
            }
            cur_avatar_status.unkc6 = 20;
        } else {
            cur_avatar_status.unkc6 = 0;
        }
    }
    if (cur_avatar_status.unkf == 0 && cur_avatar_status.unkc6 == 0 && (cur_avatar_status.unk178 & action_button_masks[4])
        && !(cur_avatar_status.unk17a & action_button_masks[4])) {
        player_begin_weapon_attack(0);
    }
}

ADDRESS(0x8003047c, 0xec)
void player_update_reaction_rotation_offsets(void)
{
    cur_avatar_status.unk188.knock.push.vx =
        angle_velocity_step(0, cur_avatar_status.unk134, cur_avatar_status.unk188.knock.push.vx, 8, 4);
    cur_avatar_status.unk188.knock.push.vy =
        angle_velocity_step(0, cur_avatar_status.unk136, cur_avatar_status.unk188.knock.push.vy, 8, 4);
    cur_avatar_status.unk188.knock.push.vz =
        angle_velocity_step(0, cur_avatar_status.unk138, cur_avatar_status.unk188.knock.push.vz, 8, 4);
    cur_avatar_status.unk134 += cur_avatar_status.unk188.knock.push.vx;
    cur_avatar_status.unk136 += cur_avatar_status.unk188.knock.push.vy;
    cur_avatar_status.unk138 += cur_avatar_status.unk188.knock.push.vz;
}

ADDRESS(0x80030568, 0x70)
void player_render_frame_and_release_pool(void)
{
    u8 f9 = cur_avatar_status.unkf9;
    u8 fa = cur_avatar_status.unkfa;

    cur_avatar_status.unkf9 = 0;
    cur_avatar_status.unkfa = 0;
    render_game_frame(NULL, NULL);
    cur_avatar_status.unkf9 = f9;
    cur_avatar_status.unkfa = fa;
    pool_release_all();
}

ADDRESS(0x800305d8, 0x1b4)
void player_handle_interaction_and_menu(void)
{
    s32 result;

    if (cur_avatar_status.unk101 == 0x11) {
        return;
    }
    if ((cur_avatar_status.unk178 & action_button_masks[7]) && !(cur_avatar_status.unk17a & action_button_masks[7])) {
        event_world_dispatch_interaction(&cur_avatar_status.pos, &cur_avatar_status.unk12c);
    }
    if ((cur_avatar_status.unk178 & action_button_masks[5]) && !(cur_avatar_status.unk17a & action_button_masks[5])
        && cur_avatar_status.unkc0 == -1) {
        player_render_frame_and_release_pool();
        do {
            cd_request_yield();
            resource_advance_transition();
        } while ((s16)asset_status.loading == 1 && asset_status.unk17 == 0);
        map_reveal_visited_rects();
        resource_mark_area_models_reclaimable(asset_status.area[0]);
        result = menu_run_root_controller();
        if (result >= 0) {
            render_game_frame(NULL, NULL);
            event_scene_command_dispatch(&cur_avatar_status.pos, &cur_avatar_status.unk12c, result);
        } else if (result == -2) {
            main_end_flag = 2;
        } else if (result == -3) {
            player_restore_equipment_effects();
            player_reload_map_resources(asset_status.area[0], asset_status.area[0], asset_status.area[0],
                                        asset_status.area[3], asset_status.area[4], 0xff);
        }
        player_clear_motion();
    }
}

ADDRESS(0x8003078c, 0x68)
void player_reset_reaction_state(void)
{
    cur_avatar_status.unk101 = 0;
    cur_avatar_status.unk164 = 0;
    cur_avatar_status.unk162 = 0;
    cur_avatar_status.unk138 = 0;
    cur_avatar_status.unk136 = 0;
    cur_avatar_status.unk134 = 0;
    cur_avatar_status.unk140 = 0;
    cur_avatar_status.unk13e = 0;
    cur_avatar_status.unk13c = 0;
    cur_avatar_status.unk148 = 0;
    cur_avatar_status.unk146 = 0;
    cur_avatar_status.unk144 = 0;
}

ADDRESS(0x800307f4, 0x1c)
void player_begin_movie_playback(u8 id)
{
    cur_avatar_status.unk101 = 8;
    cur_avatar_status.unk188.turn.id = id;
}

ADDRESS(0x80030810, 0x54)
void player_begin_movie_approach(u8 id)
{
    cur_avatar_status.unk101 = 7;
    cur_avatar_status.unk188.turn.id = id;
    cur_avatar_status.unk188.turn.unk1 = 0;
    cur_avatar_status.unk188.turn.angle = cur_avatar_status.unk12c;
}

ADDRESS(0x80030864, 0x54)
void player_begin_movie_return(u8 id)
{
    cur_avatar_status.unk101 = 9;
    cur_avatar_status.unk188.turn.id = id;
    cur_avatar_status.unk188.turn.unk1 = 0;
    cur_avatar_status.unk188.turn.angle = cur_avatar_status.unk12c;
}

ADDRESS(0x800308b8, 0x3c)
void player_begin_rotation_reaction(SVECTOR *dir)
{
    cur_avatar_status.unk101 = 3;
    cur_avatar_status.unk188.knock.dir = *dir;
}

ADDRESS(0x800308f4, 0x14)
void func_800308f4(void)
{
    cur_avatar_status.unk101 = 4;
}

ADDRESS(0x80030908, 0xc0)
void player_begin_moving_damage_reaction(SVECTOR *dir, SVECTOR *push, s16 lift)
{
    if (cur_avatar_status.unk101 == 7 || cur_avatar_status.unk101 == 8 || cur_avatar_status.unk101 == 9) {
        return;
    }
    cur_avatar_status.unk101 = 0x10;
    cur_avatar_status.unk188.knock.dir = *dir;
    cur_avatar_status.unk188.knock.push = *push;
    cur_avatar_status.unk174 = 3500;
    cur_avatar_status.unk176 = lift;
    cur_avatar_status.unk104 = 0x40;
    cur_avatar_status.unk172 = cur_avatar_status.unk188.knock.dir.vy;
}

ADDRESS(0x800309c8, 0xa4)
void player_begin_rotation_only_damage_reaction(SVECTOR *dir, SVECTOR *push, s16 lift)
{
    if (cur_avatar_status.unk101 == 7 || cur_avatar_status.unk101 == 8 || cur_avatar_status.unk101 == 9) {
        return;
    }
    cur_avatar_status.unk101 = 0x12;
    cur_avatar_status.unk188.knock.dir = *dir;
    cur_avatar_status.unk188.knock.push = *push;
    cur_avatar_status.unk174 = 3500;
    cur_avatar_status.unk176 = lift;
}

/* Death: mode 0x11, knocked along dir (NULL: in place). */
ADDRESS(0x80030a6c, 0x8c)
void player_death_begin(SVECTOR *dir)
{
    if (cur_avatar_status.unk101 == 0x11) {
        return;
    }
    cur_avatar_status.unk101 = 0x11;
    audio_play_sound(6, 0x6e);
    if (dir != NULL) {
        cur_avatar_status.unk188.knock.dir = *dir;
    }
    cur_avatar_status.unk188.knock.push.vx = 0;
    cur_avatar_status.unk13a = 0;
}

ADDRESS(0x80030af8, 0x2c)
void player_begin_actor_overlap_bob(void)
{
    if (cur_avatar_status.unk101 == 0) {
        cur_avatar_status.unk101 = 5;
        cur_avatar_status.unk188.count = 0;
    }
}

/*
 * Steps an effect counter towards *timer (up while below it, else down with
 * the timer cleared); returns count scaled to scale over range frames, -1 when
 * both are 0.
 */
ADDRESS(0x80030b24, 0xbc)
s16 status_phase_step_scaled(s16 *count, s16 *timer, s32 range, s32 scale)
{
    s16 v;
    s16 n = *count;
    s16 t = 0;

    if (timer != NULL) {
        t = *timer;
    }
    if (n != 0 || t != 0) {
        if (n < t) {
            n++;
        } else {
            t = 0;
            n--;
        }
        if (n < range) {
            v = n * scale / range;
        } else {
            v = scale;
        }
        *count = n;
        if (timer != NULL) {
            *timer = t;
        }
        return v;
    }
    return -1;
}

ADDRESS(0x80030be0, 0x6c)
void player_adjust_hp(s32 amount)
{
    s32 hp = cur_avatar_status.hp;

    hp += amount;
    if (hp <= 0) {
        cur_avatar_status.hp = 0;
        player_death_begin(NULL);
    } else if (hp > cur_avatar_status.hp_max) {
        cur_avatar_status.hp = cur_avatar_status.hp_max;
    } else {
        cur_avatar_status.hp = hp;
    }
}

ADDRESS(0x80030c4c, 0x20)
void player_adjust_hp_on_weapon_hit(s32 amount)
{
    player_adjust_hp(amount);
}

ADDRESS(0x80030c6c, 0x50)
void player_adjust_mp(s32 amount)
{
    s32 v = cur_avatar_status.unk1c;

    v += amount;
    if (v <= 0) {
        cur_avatar_status.unk1c = 0;
    } else if (v > cur_avatar_status.unk1a) {
        cur_avatar_status.unk1c = cur_avatar_status.unk1a;
    } else {
        cur_avatar_status.unk1c = v;
    }
}

ADDRESS(0x80030cbc, 0xac)
void player_apply_equipment_hp_tick(EquipmentRecord *r)
{
    if (r->hp_regen_interval != 0 && cur_avatar_status.unk9c % r->hp_regen_interval == 0) {
        player_adjust_hp(1);
    }
    if (r->hp_drain_interval != 0 && cur_avatar_status.unk9c % r->hp_drain_interval == 0) {
        player_adjust_hp(-1);
    }
}

ADDRESS(0x80030d68, 0xac)
void player_apply_equipment_mp_tick(EquipmentRecord *r)
{
    if (r->mp_regen_interval != 0 && cur_avatar_status.unk9c % r->mp_regen_interval == 0) {
        player_adjust_mp(1);
    }
    if (r->mp_drain_interval != 0 && cur_avatar_status.unk9c % r->mp_drain_interval == 0) {
        player_adjust_mp(-1);
    }
}

ADDRESS(0x80030e14, 0x1b8)
void player_update_status_visuals(void)
{
    s16 v;
    s16 t;

    v = status_phase_step_scaled(&cur_avatar_status.unk7e, &cur_avatar_status.unk80, 0x40, 0xc00);
    if (v != -1) {
        light_interpolate_color_and_fog(10, 10, 10, 5000, 6000, v);
        set_accumulated_color_overlay(0x80, 0x80, 0x80, v >> 2, 2);
    }
    v = status_phase_step_scaled(&cur_avatar_status.unk8c, &cur_avatar_status.unk8e, 0x40, 0xe10);
    if (v != -1) {
        light_interpolate_color_and_fog(0xdc, 0xdc, 0xa0, 0xffff, 18000, v);
    }
    if (cur_avatar_status.unk174 != 0) {
        cur_avatar_status.unk174 -= cur_avatar_status.unk176;
        if (cur_avatar_status.unk174 < 0) {
            cur_avatar_status.unk174 = 0;
        }
        if (cur_avatar_status.unk174 >= 4096) {
            t = 4096;
        } else {
            t = cur_avatar_status.unk174;
        }
        accumulate_color_overlay(0x3c, 0, 0, t, 1);
    }
    v = status_phase_step_scaled(&cur_avatar_status.unk7a, &cur_avatar_status.unk7c, 0x40, 0xc00);
    if (v != -1) {
        if (cur_avatar_status.unk7a == 0) {
            player_recalculate_combat_stats();
        }
        accumulate_color_overlay(0x1e, 0x1e, 10, v, 1);
    }
}
