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

    rot.vx = cur_avatar_status.unk124.vx - cur_avatar_status.unkb0->rotation_offset_x;
    rot.vy = cur_avatar_status.unk124.vy - cur_avatar_status.unkb0->rotation_offset_y;
    rot.vz = cur_avatar_status.unk124.vz + cur_avatar_status.unkb0->rotation_offset_z;
    animation_sample_vertex(0x20, cur_avatar_status.unkca, cur_avatar_status.unkc0, vertex, &v);
    v.vx -= cur_avatar_status.unkb0->position_offset_x;
    v.vy += cur_avatar_status.unkb0->position_offset_y;
    v.vz -= cur_avatar_status.unkb0->position_offset_z;
    vector_rotate_yxz(&rot, &v, out);
    out->vx += cur_avatar_status.pos.vx;
    out->vz += cur_avatar_status.pos.vz;
    y = out->vy - 1600;
    out->vy = y + (cur_avatar_status.unk16c + cur_avatar_status.pos.vy + cur_avatar_status.unk170);
}

/*
 * Whether the weapon's spell of hand (0: WeaponRecord.unk3, 1: unk4) may be cast:
 * the spell must be learned; weapon 2 and 13..17 always may, others need
 * unk40 and every unk42 at 50 or more.
 */
ADDRESS(0x8002cfe0, 0x150)
s32 player_meets_weapon_magic_requirement(s32 hand)
{
    s32 n = 5;
    s32 low;

    if (cur_avatar_status.unkcb != 0xff) {
        if (hand == 0 && cur_avatar_status.unkb0->unk0[3] != 0xff
            && effect_status.unk0[cur_avatar_status.unkb0->unk0[3]].menu_available == 0) {
            return 0;
        }
        if (hand == 1 && cur_avatar_status.unkb0->alternate_magic_id != 0xff
            && effect_status.unk0[cur_avatar_status.unkb0->alternate_magic_id].menu_available == 0) {
            return 0;
        }
    }
    switch (cur_avatar_status.unkcb) {
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
        if (cur_avatar_status.unk42[n] < low) {
            low = cur_avatar_status.unk42[n];
        }
    }
    return cur_avatar_status.unk40 >= 50 && low >= 50;
}

/* Casts spell id (MagicRecord) along dir when unk1c covers its cost; pay takes the cost. */
ADDRESS(0x8002d130, 0x170)
void player_dispatch_weapon_magic(s32 id, s32 pay, s32 dir)
{
    VECTOR unused;
    MagicRecord *r = &effect_status.unk0[id];

    if (cur_avatar_status.unk1c < r->mp_cost) {
        return;
    }
    cur_avatar_status.unkac = r;
    if (pay) {
        cur_avatar_status.unk22 = 0;
        cur_avatar_status.unk10 = 0;
        cur_avatar_status.unk1c -= r->mp_cost;
        if (id == 0x36) {
            audio_play_sound(0x63, 0x6e);
        }
    }
    switch (id) {
    case 0x42:
        dir <<= 9;
        cur_avatar_status.unk14c.vx = rcos(dir) >> 3;
        cur_avatar_status.unk14c.vy = rsin(dir) >> 3;
        cur_avatar_status.unk14c.vz = 600;
        player_dispatch_magic_effect(id);
        break;
    case 0x47:
        audio_play_sound(0x61, 0x6e);
        cur_avatar_status.unk14c.vx = 0;
        cur_avatar_status.unk14c.vy = dir * 300 - 300;
        cur_avatar_status.unk14c.vz = 400;
        player_dispatch_magic_effect(id);
        break;
    default:
        cur_avatar_status.unk14c.vx = 200;
        cur_avatar_status.unk14c.vy = 200;
        cur_avatar_status.unk14c.vz = 400;
        player_dispatch_magic_effect(id);
        break;
    }
}

/*
 * Weapon swing and bow update (once per frame): unkc0 runs the swing of the
 * held weapon (WeaponRecord, unkb0) from 0 to 4096; the bows (27, 28) spend an
 * arrow (event_item_counters_table) and aim it, the other weapons cast the weapon spell,
 * play the swing sound and probe for an Actor along the swing.
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
    WeaponRecord *w;
    Object4c *arrow;
    Actor *obj;
    ActorGroup *def;
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

    w = cur_avatar_status.unkb0;
    switch (cur_avatar_status.unkcb) {
    case 27:
    case 28:
        if (cur_avatar_status.unkc0 != -1) {
        switch (cur_avatar_status.unkca) {
        case 0:
            if (cur_avatar_status.unkc0 == 0) {
                switch (cur_avatar_status.unkcb) {
                case 27:
                    id = 0x51;
                    item = 0x93;
                    break;
                case 28:
                    id = 0x52;
                    item = 0x94;
                    break;
                }
                count = event_item_counters_table[0][item];
                if (count != 0) {
                    event_item_counters_table[0][item] = --count;
                    arrow = effect_construct_record(10, 0x42, id, &cur_avatar_status.pos, 0,
                                                    &cur_avatar_status.unk124);
                    cur_avatar_status.unkbc = arrow;
                    if (arrow != NULL) {
                        arrow->unk7 = 99;
                    }
                    if (cur_avatar_status.unkcb == 28) {
                        d = (rand() * 5 >> 15) + 3;
                        if (d >= cur_avatar_status.hp) {
                            cur_avatar_status.hp = 0;
                            player_death_begin(NULL);
                        } else {
                            cur_avatar_status.hp -= d;
                        }
                    }
                } else {
                    cur_avatar_status.unkbc = NULL;
                }
            }
            cur_avatar_status.unkc0 += w->attack_phase_step;
            if (cur_avatar_status.unkc0 >= 4095) {
                cur_avatar_status.unkc0 = 0xfff;
            }
            if (cur_avatar_status.unkc0 >= w->normal_attack_end_phase) {
                if (cur_avatar_status.unk1e == 0) {
                    audio_play_sound(0x60, 0x6e);
                }
                cur_avatar_status.unk1e = (cur_avatar_status.unkc0 - w->normal_attack_end_phase) * 5000 / (0xfff - w->normal_attack_end_phase);
            } else {
                cur_avatar_status.unk1e = 0;
            }
            arrow = cur_avatar_status.unkbc;
            if (arrow != NULL) {
                rot.vx = cur_avatar_status.unk124.vx - w->rotation_offset_x;
                rot.vy = cur_avatar_status.unk124.vy - w->rotation_offset_y;
                rot.vz = cur_avatar_status.unk124.vz + w->rotation_offset_z;
                animation_sample_vertex(0x20, cur_avatar_status.unkca, cur_avatar_status.unkc0,
                                        w->initial_vertex_index, &v);
                v.vx -= w->position_offset_x;
                v.vy += w->position_offset_y;
                v.vz -= w->position_offset_z;
                vector_rotate_yxz(&rot, &v, &a);
                arrow->pos.vx = cur_avatar_status.pos.vx + a.vx;
                arrow->pos.vz = cur_avatar_status.pos.vz + a.vz;
                arrow->pos.vy = cur_avatar_status.pos.vy + a.vy + cur_avatar_status.unk16c + cur_avatar_status.unk170 - 1600;
                animation_sample_vertex(0x20, cur_avatar_status.unkca, cur_avatar_status.unkc0,
                                        w->final_vertex_index, &v);
                v.vx -= w->position_offset_x;
                v.vy += w->position_offset_y;
                v.vz -= w->position_offset_z;
                vector_rotate_yxz(&rot, &v, &b);
                vector_displacement_to_pitch_yaw(a.vx - b.vx, a.vy - b.vy, a.vz - b.vz, &arrow->angle);
            }
            if (cur_avatar_status.unk178 & action_button_masks[4]) {
                break;
            }
            if (arrow != NULL) {
                arrow->unk7 = 0;
                pitch_yaw_to_forward_vector(&arrow->angle, &arrow->vel);
                vector3s_scale_shift12(cur_avatar_status.unk1e * 900 / 5000, &arrow->vel);
                arrow->unk10 = 50;
                arrow->arg.h.unk44 = arrow->pos.vy;
            }
            cur_avatar_status.unkca = 1;
            cur_avatar_status.unkc0 = 0;
            break;
        case 1:
            cur_avatar_status.unkc0 += 400;
            if (cur_avatar_status.unkc0 >= 4095) {
                cur_avatar_status.unkc0 = -1;
                cur_avatar_status.unk1e = 0;
            }
            break;
        }
        } else {
            cur_avatar_status.unk1e = 0;
            cur_avatar_status.unkf = 0;
        }
        break;
    case 0xff:
        break;
    default:
        if (cur_avatar_status.unkc0 != -1) {
            s32 scale;

            div = 0;
            if (cur_avatar_status.unkca == 0) {
                speed = w->attack_phase_step;
                hit = w->normal_attack_end_phase;
                limit = w->normal_attack_sound_phase;
                add = 0;
            } else {
                speed = w->alternate_attack_phase_step;
                hit = w->alternate_attack_end_phase;
                div = w->magic_phase_step;
                limit = w->alternate_attack_sound_end_phase;
                add = w->alternate_attack_sound_phase_step;
            }
            cur_avatar_status.unkc0 += speed;
            if (cur_avatar_status.unkca == 0 && w->unk0[3] != 0xff && cur_avatar_status.unkcf != 0
                && player_meets_weapon_magic_requirement(0)
                && (cur_avatar_status.unk178 & action_button_masks[6])) {
                if (cur_avatar_status.unkc0 >= w->magic_window_start && cur_avatar_status.unkc0 <= w->magic_window_end) {
                    if (cur_avatar_status.unkce != 0) {
                        player_dispatch_weapon_magic(w->unk0[3], cur_avatar_status.unkce == w->magic_shots,
                                                     cur_avatar_status.unkce);
                        cur_avatar_status.unkce--;
                    }
                } else {
                    cur_avatar_status.unkce = 0;
                }
            }
            if (cur_avatar_status.unkc0 >= cur_avatar_status.unkc4 && cur_avatar_status.unkc0 < cur_avatar_status.unkc4 + speed) {
                if (w->unk0[0] != 0xff) {
                    audio_play_sound(w->unk0[0], 0x50);
                }
                if (cur_avatar_status.unkc4 < limit) {
                    cur_avatar_status.unkc4 += add;
                } else {
                    cur_avatar_status.unkc4 = 5000;
                }
            }
            if (cur_avatar_status.unkc0 >= cur_avatar_status.unkc2 && cur_avatar_status.unkc0 < cur_avatar_status.unkc2 + speed) {
                if (cur_avatar_status.unkcb == 0x21 && cur_avatar_status.unk22 == 5000) {
                    cur_avatar_status.unk14c.vx = -200;
                    cur_avatar_status.unk14c.vy = 200;
                    cur_avatar_status.unk14c.vz = 400;
                    player_probe_view_target_and_vectors(900, &probe, &dir, &dist);
                    effect_construct_record(10, 0x42, 0, &probe, &dir);
                }
                cur_avatar_status.unkd0 = 0;
                if (cur_avatar_status.unkca == 1) {
                    if (cur_avatar_status.unkcb == 13 && (cur_avatar_status.unk178 & action_button_masks[6])) {
                        cur_avatar_status.unkd0 = 1;
                        cur_avatar_status.unkc0 -= speed;
                        break;
                    }
                    player_dispatch_weapon_magic(w->alternate_magic_id, cur_avatar_status.unkc0 >= hit,
                                                 (cur_avatar_status.unkc0 - w->alternate_attack_window_start) / div);
                }
                if (cur_avatar_status.unkc0 >= hit) {
                    target = &from;
                    scale = cur_avatar_status.unk20;
                    cur_avatar_status.unkc2 = 5000;
                    cur_avatar_status.unkf = 10;
                    cur_avatar_status.unk1e = 0;
                    from.vx = cur_avatar_status.pos.vx;
                    from.vy = cur_avatar_status.pos.vy - 1000;
                    from.vz = cur_avatar_status.pos.vz;
                } else {
                    target = NULL;
                    scale = cur_avatar_status.unk20 >> 2;
                    cur_avatar_status.unkc2 += div;
                }
                v.vx = 0;
                v.vy = 0;
                v.vz = w->attack_reach;
                rot.vx = -cur_avatar_status.unk12c.vx;
                rot.vy = cur_avatar_status.unk12c.vy;
                rot.vz = 0;
                vector_rotate_yxz(&rot, &v, &step);
                step.vy = (3600 - w->attack_reach) * step.vy / 1800;
                step.vx /= 4;
                step.vy /= 4;
                step.vz /= 4;
                probe.vx = cur_avatar_status.pos.vx;
                probe.vy = cur_avatar_status.pos.vy - 800;
                probe.vz = cur_avatar_status.pos.vz;
                for (n = 3; n != -1; n--) {
                    probe.vx += step.vx;
                    probe.vy += step.vy;
                    probe.vz += step.vz;
                    found = actor_find_overlap_excluding_target_type3(probe.vx, probe.vy, probe.vz, 400, 600);
                    if (found != -1) {
                        obj = &actor_status.objects[found];
                        def = &actor_status.groups[obj->group_index];
                        angle = vector_xz_to_angle(obj->pos.vx - cur_avatar_status.pos.vx,
                                                   obj->pos.vz - cur_avatar_status.pos.vz);
                        if (angle_within_tolerance(cur_avatar_status.unk124.vy, angle,
                                                   def->player_facing_tolerance)
                            && angle_within_tolerance(obj->angle.vy, angle + 2048,
                                                      def->actor_facing_tolerance)) {
                            actor_apply_magic_to_actor(found, cur_avatar_status.unk40,
                                                       cur_avatar_status.unk54[0], cur_avatar_status.unk54[1],
                                                       cur_avatar_status.unk54[2], cur_avatar_status.unk54[3],
                                                       cur_avatar_status.unk54[4], cur_avatar_status.unk54[5],
                                                       cur_avatar_status.unk54[6], cur_avatar_status.unk62,
                                                       scale, 80, NULL, target);
                            break;
                        }
                    }
                }
            }
            if (cur_avatar_status.unkc0 >= 4096) {
                cur_avatar_status.unkc0 = -1;
                cur_avatar_status.unkce = 0;
            }
        } else if (!(cur_avatar_status.unk178 & action_button_masks[4])) {
            if (cur_avatar_status.unkf == 0) {
                weight = w->unk0[1];
                if (cur_avatar_status.unkf3 == 0x42) {
                    weight -= weight >> 2;
                }
                if (cur_avatar_status.unkf3 == 0x44) {
                    weight += weight >> 1;
                }
                if (cur_avatar_status.unkf3 == 0x46) {
                    weight >>= 1;
                }
                cur_avatar_status.unk1e += player_charge_gain_for_rank(cur_avatar_status.unk40, weight) * 2;
                if (cur_avatar_status.unk1e > 5000) {
                    cur_avatar_status.unk1e = 5000;
                }
            } else {
                cur_avatar_status.unkf--;
            }
        }
        break;
    }
}
