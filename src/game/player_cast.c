/*
 * GAME.EXE: player state, continued (same WIP module as src/game/player.c,
 * contiguous with it and with src/game/player_spell.c): the weapon tip
 * position, the spell checks and casts, and the weapon swing update.
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
#include <game/object4c.h>
#include <game/entity.h>
#include <psyq/libc.h>
#include <kf/lib/null.h>

/* World position of vertex of the weapon model (no callers). */
ADDRESS(0x8002cea8, 0x138)
void player_sample_weapon_world_vertex(s32 vertex, VECTOR *out)
{
    SVECTOR v;
    SVECTOR rot;
    s32 y;

    rot.vx = DAT_801b24e4.unk124.vx - DAT_801b24e4.unkb0->unk3c;
    rot.vy = DAT_801b24e4.unk124.vy - DAT_801b24e4.unkb0->unk3e;
    rot.vz = DAT_801b24e4.unk124.vz + DAT_801b24e4.unkb0->unk40;
    animation_sample_vertex(0x20, DAT_801b24e4.unkca, DAT_801b24e4.unkc0, vertex, &v);
    v.vx -= DAT_801b24e4.unkb0->unk34;
    v.vy += DAT_801b24e4.unkb0->unk36;
    v.vz -= DAT_801b24e4.unkb0->unk38;
    vector_rotate_yxz(&rot, &v, out);
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
s32 player_meets_weapon_magic_requirement(s32 hand)
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
void player_dispatch_weapon_magic(s32 id, s32 pay, s32 dir)
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
            audio_play_sound(0x63, 0x6e);
        }
    }
    switch (id) {
    case 0x42:
        dir <<= 9;
        DAT_801b24e4.unk14c.vx = rcos(dir) >> 3;
        DAT_801b24e4.unk14c.vy = rsin(dir) >> 3;
        DAT_801b24e4.unk14c.vz = 600;
        player_dispatch_magic_effect(id);
        break;
    case 0x47:
        audio_play_sound(0x61, 0x6e);
        DAT_801b24e4.unk14c.vx = 0;
        DAT_801b24e4.unk14c.vy = dir * 300 - 300;
        DAT_801b24e4.unk14c.vz = 400;
        player_dispatch_magic_effect(id);
        break;
    default:
        DAT_801b24e4.unk14c.vx = 200;
        DAT_801b24e4.unk14c.vy = 200;
        DAT_801b24e4.unk14c.vz = 400;
        player_dispatch_magic_effect(id);
        break;
    }
}

/*
 * Weapon swing and bow update (once per frame): unkc0 runs the swing of the
 * held weapon (Record44, unkb0) from 0 to 4096; the bows (27, 28) spend an
 * arrow (DAT_800c85e8) and aim it, the other weapons cast the weapon spell,
 * play the swing sound and probe for an Object88 along the swing.
 */
ADDRESS(0x8002d2a0, 0xc4c)
void player_update_weapon_attack(void)
{
    SVECTOR v;
    SVECTOR rot;
    VECTOR a;
    VECTOR step;
    VECTOR from;
    VECTOR b;
    VECTOR probe;
    SVECTOR dir;
    s32 dist;
    Record44 *w;
    Object4c *arrow;
    Object88 *obj;
    Object78 *def;
    VECTOR *target;
    s32 speed;
    s32 hit;
    s32 div;
    s32 limit;
    s32 add;
    u8 id;
    s32 item;
    s32 count;
    s32 d;
    s32 weight;
    s32 n;
    s32 found;
    s32 angle;

    w = DAT_801b24e4.unkb0;
    switch (DAT_801b24e4.unkcb) {
    case 27:
    case 28:
        if (DAT_801b24e4.unkc0 != -1) {
        switch (DAT_801b24e4.unkca) {
        case 0:
            if (DAT_801b24e4.unkc0 == 0) {
                switch (DAT_801b24e4.unkcb) {
                case 27:
                    id = 0x51;
                    item = 0x93;
                    break;
                case 28:
                    id = 0x52;
                    item = 0x94;
                    break;
                }
                count = DAT_800c85e8[0][item];
                if (count != 0) {
                    DAT_800c85e8[0][item] = --count;
                    arrow = effect_construct_record(10, 0x42, id, &DAT_801b24e4.pos, 0, &DAT_801b24e4.unk124);
                    DAT_801b24e4.unkbc = arrow;
                    if (arrow != NULL) {
                        arrow->unk7 = 99;
                    }
                    if (DAT_801b24e4.unkcb == 28) {
                        d = (rand() * 5 >> 15) + 3;
                        if (d >= DAT_801b24e4.hp) {
                            DAT_801b24e4.hp = 0;
                            player_death_begin(NULL);
                        } else {
                            DAT_801b24e4.hp -= d;
                        }
                    }
                } else {
                    DAT_801b24e4.unkbc = NULL;
                }
            }
            DAT_801b24e4.unkc0 += w->unk1c;
            if (DAT_801b24e4.unkc0 >= 4095) {
                DAT_801b24e4.unkc0 = 0xfff;
            }
            if (DAT_801b24e4.unkc0 >= w->unk1e) {
                if (DAT_801b24e4.unk1e == 0) {
                    audio_play_sound(0x60, 0x6e);
                }
                DAT_801b24e4.unk1e = (DAT_801b24e4.unkc0 - w->unk1e) * 5000 / (0xfff - w->unk1e);
            } else {
                DAT_801b24e4.unk1e = 0;
            }
            arrow = DAT_801b24e4.unkbc;
            if (arrow != NULL) {
                rot.vx = DAT_801b24e4.unk124.vx - w->unk3c;
                rot.vy = DAT_801b24e4.unk124.vy - w->unk3e;
                rot.vz = DAT_801b24e4.unk124.vz + w->unk40;
                animation_sample_vertex(0x20, DAT_801b24e4.unkca, DAT_801b24e4.unkc0, w->unk3a, &v);
                v.vx -= w->unk34;
                v.vy += w->unk36;
                v.vz -= w->unk38;
                vector_rotate_yxz(&rot, &v, &a);
                arrow->pos.vx = DAT_801b24e4.pos.vx + a.vx;
                arrow->pos.vz = DAT_801b24e4.pos.vz + a.vz;
                arrow->pos.vy = DAT_801b24e4.pos.vy + a.vy + DAT_801b24e4.unk16c + DAT_801b24e4.unk170 - 1600;
                animation_sample_vertex(0x20, DAT_801b24e4.unkca, DAT_801b24e4.unkc0, w->unk42, &v);
                v.vx -= w->unk34;
                v.vy += w->unk36;
                v.vz -= w->unk38;
                vector_rotate_yxz(&rot, &v, &b);
                vector_displacement_to_pitch_yaw(a.vx - b.vx, a.vy - b.vy, a.vz - b.vz, &arrow->angle);
            }
            if (DAT_801b24e4.unk178 & DAT_80081868[4]) {
                break;
            }
            if (arrow != NULL) {
                arrow->unk7 = 0;
                pitch_yaw_to_forward_vector(&arrow->angle, &arrow->vel);
                vector3s_scale_shift12(DAT_801b24e4.unk1e * 900 / 5000, &arrow->vel);
                arrow->unk10 = 50;
                arrow->arg.h.unk44 = arrow->pos.vy;
            }
            DAT_801b24e4.unkca = 1;
            DAT_801b24e4.unkc0 = 0;
            break;
        case 1:
            DAT_801b24e4.unkc0 += 400;
            if (DAT_801b24e4.unkc0 >= 4095) {
                DAT_801b24e4.unkc0 = -1;
                DAT_801b24e4.unk1e = 0;
            }
            break;
        }
        } else {
            DAT_801b24e4.unk1e = 0;
            DAT_801b24e4.unkf = 0;
        }
        break;
    case 0xff:
        break;
    default:
        if (DAT_801b24e4.unkc0 != -1) {
            s32 scale;

            div = 0;
            if (DAT_801b24e4.unkca == 0) {
                speed = w->unk1c;
                hit = w->unk1e;
                limit = w->unk2c;
                add = 0;
            } else {
                speed = w->unk24;
                hit = w->unk28;
                div = w->unk2a;
                limit = w->unk30;
                add = w->unk32;
            }
            DAT_801b24e4.unkc0 += speed;
            if (DAT_801b24e4.unkca == 0 && w->unk0[3] != 0xff && DAT_801b24e4.unkcf != 0 && player_meets_weapon_magic_requirement(0)
                && (DAT_801b24e4.unk178 & DAT_80081868[6])) {
                if (DAT_801b24e4.unkc0 >= w->unk20 && DAT_801b24e4.unkc0 <= w->unk22) {
                    if (DAT_801b24e4.unkce != 0) {
                        player_dispatch_weapon_magic(w->unk0[3], DAT_801b24e4.unkce == w->unk5,
                                                     DAT_801b24e4.unkce);
                        DAT_801b24e4.unkce--;
                    }
                } else {
                    DAT_801b24e4.unkce = 0;
                }
            }
            if (DAT_801b24e4.unkc0 >= DAT_801b24e4.unkc4 && DAT_801b24e4.unkc0 < DAT_801b24e4.unkc4 + speed) {
                if (w->unk0[0] != 0xff) {
                    audio_play_sound(w->unk0[0], 0x50);
                }
                if (DAT_801b24e4.unkc4 < limit) {
                    DAT_801b24e4.unkc4 += add;
                } else {
                    DAT_801b24e4.unkc4 = 5000;
                }
            }
            if (DAT_801b24e4.unkc0 >= DAT_801b24e4.unkc2 && DAT_801b24e4.unkc0 < DAT_801b24e4.unkc2 + speed) {
                if (DAT_801b24e4.unkcb == 0x21 && DAT_801b24e4.unk22 == 5000) {
                    DAT_801b24e4.unk14c.vx = -200;
                    DAT_801b24e4.unk14c.vy = 200;
                    DAT_801b24e4.unk14c.vz = 400;
                    player_probe_view_target_and_vectors(900, &probe, &dir, &dist);
                    effect_construct_record(10, 0x42, 0, &probe, &dir);
                }
                DAT_801b24e4.unkd0 = 0;
                if (DAT_801b24e4.unkca == 1) {
                    if (DAT_801b24e4.unkcb == 13 && (DAT_801b24e4.unk178 & DAT_80081868[6])) {
                        DAT_801b24e4.unkd0 = 1;
                        DAT_801b24e4.unkc0 -= speed;
                        break;
                    }
                    player_dispatch_weapon_magic(w->unk4, DAT_801b24e4.unkc0 >= hit,
                                                 (DAT_801b24e4.unkc0 - w->unk26) / div);
                }
                if (DAT_801b24e4.unkc0 >= hit) {
                    target = &from;
                    scale = DAT_801b24e4.unk20;
                    DAT_801b24e4.unkc2 = 5000;
                    DAT_801b24e4.unkf = 10;
                    DAT_801b24e4.unk1e = 0;
                    from.vx = DAT_801b24e4.pos.vx;
                    from.vy = DAT_801b24e4.pos.vy - 1000;
                    from.vz = DAT_801b24e4.pos.vz;
                } else {
                    target = NULL;
                    scale = DAT_801b24e4.unk20 >> 2;
                    DAT_801b24e4.unkc2 += div;
                }
                v.vx = 0;
                v.vy = 0;
                v.vz = w->unk1a;
                rot.vx = -DAT_801b24e4.unk12c.vx;
                rot.vy = DAT_801b24e4.unk12c.vy;
                rot.vz = 0;
                vector_rotate_yxz(&rot, &v, &step);
                step.vy = (3600 - w->unk1a) * step.vy / 1800;
                step.vx /= 4;
                step.vy /= 4;
                step.vz /= 4;
                probe.vx = DAT_801b24e4.pos.vx;
                probe.vy = DAT_801b24e4.pos.vy - 800;
                probe.vz = DAT_801b24e4.pos.vz;
                for (n = 3; n != -1; n--) {
                    probe.vx += step.vx;
                    probe.vy += step.vy;
                    probe.vz += step.vz;
                    found = actor_find_overlap_excluding_target_type3(probe.vx, probe.vy, probe.vz, 400, 600);
                    if (found != -1) {
                        obj = &DAT_80185da8.objects[found];
                        def = &DAT_80185da8.defs[obj->unk2];
                        angle = vector_xz_to_angle(obj->pos.vx - DAT_801b24e4.pos.vx,
                                                   obj->pos.vz - DAT_801b24e4.pos.vz);
                        if (angle_within_tolerance(DAT_801b24e4.unk124.vy, angle, def->unk16)
                            && angle_within_tolerance(obj->angle.vy, angle + 2048, def->unk18)) {
                            actor_apply_magic_to_actor(found, DAT_801b24e4.unk40, DAT_801b24e4.unk54[0],
                                                       DAT_801b24e4.unk54[1], DAT_801b24e4.unk54[2],
                                                       DAT_801b24e4.unk54[3], DAT_801b24e4.unk54[4],
                                                       DAT_801b24e4.unk54[5], DAT_801b24e4.unk54[6],
                                                       DAT_801b24e4.unk62, scale, 80, NULL, target);
                            break;
                        }
                    }
                }
            }
            if (DAT_801b24e4.unkc0 >= 4096) {
                DAT_801b24e4.unkc0 = -1;
                DAT_801b24e4.unkce = 0;
            }
        } else if (!(DAT_801b24e4.unk178 & DAT_80081868[4])) {
            if (DAT_801b24e4.unkf == 0) {
                weight = w->unk0[1];
                if (DAT_801b24e4.unkf3 == 0x42) {
                    weight -= weight >> 2;
                }
                if (DAT_801b24e4.unkf3 == 0x44) {
                    weight += weight >> 1;
                }
                if (DAT_801b24e4.unkf3 == 0x46) {
                    weight >>= 1;
                }
                DAT_801b24e4.unk1e += player_charge_gain_for_rank(DAT_801b24e4.unk40, weight) * 2;
                if (DAT_801b24e4.unk1e > 5000) {
                    DAT_801b24e4.unk1e = 5000;
                }
            } else {
                DAT_801b24e4.unkf--;
            }
        }
        break;
    }
}
