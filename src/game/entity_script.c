/*
 * GAME.EXE: Object88 script selection, wake and sleep (WIP module): the
 * best script record for the current record, script record lookup, the
 * animation pick, the per-record wake/sleep test and a charge curve.
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

/* Starts the best-scoring script record of the current record. */
ADDRESS(0x8004bf1c, 0x100)
void actor_select_best_target(s32 dist)
{
    Object88 *obj;
    Object78 *def;
    u8 **scripts;
    u8 *script;
    u8 *best;
    s32 best_score;
    s32 score;
    s32 i;

    obj = actor_status.current;
    def = actor_status.def;
    if (obj->unkf == 0xf0 || obj->unkf == 0) {
        return;
    }
    scripts = def->scripts;
    best_score = -2;
    best = NULL;
    for (i = 15; i != -1; i--) {
        script = *scripts++;
        if (script == NULL) {
            break;
        }
        score = actor_score_target_candidate(script, dist);
        if (best_score < score) {
            best_score = score;
            best = script;
        }
    }
    if (best == NULL) {
        return;
    }
    if (best != obj->unk60 || obj->unkf == 0xff) {
        actor_set_target(obj, best);
    }
}

ADDRESS(0x8004c01c, 0x4c)
void actor_select_target_for_player_distance(void)
{
    actor_select_best_target(fixed_vector2_length(actor_status.current->pos.vx - cur_avatar_status.pos.vx,
                                                  actor_status.current->pos.vz - cur_avatar_status.pos.vz));
}

/* The script record of the given id, or 0. */
ADDRESS(0x8004c068, 0x48)
u8 *actor_find_target_of_type(Object78 *def, u8 id)
{
    u8 **scripts;
    u8 *script;
    s32 i;

    scripts = def->scripts;
    for (i = 15; i != -1; i--) {
        script = *scripts++;
        if (script == NULL) {
            break;
        }
        if (script[0] == id) {
            return script;
        }
    }
    return NULL;
}

ADDRESS(0x8004c0b0, 0x54)
void actor_select_target_type_in_own_group(Object88 *obj, u8 id)
{
    actor_set_target(obj, actor_find_target_of_type(&actor_status.defs[obj->unk2], id));
}

ADDRESS(0x8004c104, 0x2c)
void actor_reset_target_and_reselect(void)
{
    actor_status.current->unkf = 0xff;
    actor_select_target_for_player_distance();
}

ADDRESS(0x8004c130, 0x54)
void actor_set_animation(u8 anim)
{
    Object88 *obj;

    if (anim == 0xff) {
        return;
    }
    obj = actor_status.current;
    if (actor_animation_shift_enabled != 0 && actor_animation_shift_target == obj) {
        anim += 2;
    }
    obj->unkc = anim;
    obj->unk18 = 0;
}

ADDRESS(0x8004c184, 0x6c)
void actor_set_animation_if_changed(u8 anim)
{
    Object88 *obj;

    if (anim == 0xff) {
        return;
    }
    obj = actor_status.current;
    if (obj->unkc == anim) {
        return;
    }
    if (actor_animation_shift_enabled != 0 && actor_animation_shift_target == obj) {
        anim += 2;
    }
    obj->unkc = anim;
    obj->unk18 = 0;
}

/* Wakes, sleeps and retargets the current record by its distance to the player. */
ADDRESS(0x8004c1f0, 0x33c)
void actor_update_lifecycle_for_player_range(void)
{
    Object88 *obj = actor_status.current;
    Object78 *def = actor_status.def;
    u8 kind = obj->unk0;
    s32 dist;

    switch (obj->unk9) {
    case 0:
        dist = vector_distance_to_point(&obj->pos, cur_avatar_status.pos.vx, 0xffff, cur_avatar_status.pos.vz,
                                        (def->unka + 1) << 11, 0, 0);
        if (dist == -1) {
            return;
        }
        if (kind == 3 || kind == 4) {
            if (actor_status.target->unk9 != 1) {
                return;
            }
            actor_prepare_and_initialize(actor_status.current);
            actor_select_best_target(dist);
            return;
        }
        if (kind == 2) {
            u8 chance = obj->unka;
            if (chance != 0xff && chance < rand() >> 4) {
                return;
            }
        } else {
            if (dist < def->unka << 11 && cur_avatar_status.unke == 0) {
                goto wake;
            }
            if (kind != 1) {
                if (kind != 0) {
                    goto wake;
                }
                if (obj->unka == 0 || obj->unka < rand() >> 7) {
                    goto wake;
                }
            }
        }
        if (actor_find_overlap_excluding_target_type3(obj->pos.vx, obj->pos.vy, obj->pos.vz, def->unk12,
                                                      def->unk14) != -1) {
            if (kind != 2) {
                obj->unk9 = 2;
            }
            return;
        }
        actor_prepare_and_initialize(actor_status.current);
        {
            u8 *script = actor_find_target_of_type(def, 0x15);
            if (script == NULL) {
                script = actor_find_target_of_type(def, 6);
            }
            if (script == NULL) {
                script = actor_find_target_of_type(def, 0x1a);
            }
            if (script != NULL) {
                actor_set_target(obj, script);
                return;
            }
        }
        actor_select_best_target(dist);
        return;
    wake:
        if (kind != 2) {
            obj->unk9 = 2;
        }
        return;
    case 1:
        dist = vector_distance_to_point(&obj->pos, cur_avatar_status.pos.vx, 0xffff, cur_avatar_status.pos.vz,
                                        def->unkb << 11, 0, 0);
        if (dist != -1) {
            return;
        }
        map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->unk1c, -1);
        obj->unk9 = 0;
        actor_set_home_position(obj);
        return;
    case 2:
        if (kind == 3 || kind == 4) {
            if (actor_status.target->unk9 == 1) {
                return;
            }
        } else {
            dist = vector_distance_to_point(&obj->pos, cur_avatar_status.pos.vx, 0xffff,
                                            cur_avatar_status.pos.vz, def->unkb << 11, 0, 0);
            if (dist != -1) {
                return;
            }
        }
        obj->unk9 = 0;
        actor_set_home_position(obj);
        return;
    }
}

/* Restarts or puts back every live record made from record kind. */
ADDRESS(0x8004c52c, 0xc0)
void actor_retarget_or_disable_group_members(u16 kind)
{
    Object88 *obj;
    s16 i;

    obj = actor_status.objects;
    for (i = 199; i != -1; i--) {
        if (obj->unk0 != OBJECT88_FREE && obj->unk2 == kind) {
            if (obj->unk9 == 1) {
                actor_select_target_type_in_own_group(obj, 3);
            } else {
                actor_set_lifecycle_and_home_position(obj);
            }
        }
        obj++;
    }
}

ADDRESS(0x8004c5ec, 0x7c)
s32 actor_magic_component_curve(s32 a, s32 b, s32 c)
{
    s32 d;

    a <<= 4;
    b <<= 4;
    c <<= 4;
    if (b == 0) {
        return 0;
    }
    b += a;
    d = b - c;
    if (d < 0) {
        d = 0;
    }
    if (c == 0) {
        c = 16;
    }
    return d + (b * b) / (c * 2);
}

/*
 * Hits record index: attack stats a2..a9 of power a1 against the record's
 * resistances (actor_magic_component_curve each), strength a10 and flags a11 (bits 6-7
 * the attacker kind, bits 4-5 the hit kind); FDAT slot 18 adjusts the
 * damage. Starts the record's hurt or death script and pushes it away from
 * target (NULL: no push).
 */
ADDRESS(0x8004c668, 0x844)
void actor_apply_magic_to_actor(u16 index, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6, u16 a7, u16 a8,
                                u16 a9, u16 a10, u16 a11, Record18 *rec, VECTOR *target)
{
    Object88 *obj;
    Object88 *link;
    Object78 *def;
    u8 **scripts;
    u8 *script;
    u8 *dodge;
    SVECTOR dir;
    s32 dmg;
    s32 hp;
    s32 mass;
    s32 kind;
    s32 mode;
    s32 i;
    u8 chance;

    obj = &actor_status.objects[index];
    if (obj->unk1 == 0x6e) {
        return;
    }
    if (obj->unk1 == 1) {
        return;
    }
    if (obj->unk1 == 2) {
        return;
    }
    if (obj->unk1 == 0x1d) {
        return;
    }
    if (obj->unk1 == 0x16) {
        return;
    }
    if (obj->unk0 == 3) {
        obj = &actor_status.objects[obj->unk22];
    }
    def = &actor_status.defs[obj->unk2];
    if (obj->unke == 3 && obj->unk18 >= 1548) {
        return;
    }
    if (obj->unke == 0x15) {
        if (obj->unk70 == 0) {
            obj->unk70 = 2;
        }
        return;
    }
    if (obj->unke == 6 && obj->unk70 == 0) {
        obj->unk70 = 2;
    }
    if (obj->unke == 0x1a) {
        return;
    }
    kind = a11 & 0xc0;
    mode = a11 & 0x30;
    if (kind == 0x80 && (obj->unk28 & 0x100000)) {
        return;
    }
    dmg = actor_magic_component_curve(a1, a2, def->unk20[0]);
    dmg += actor_magic_component_curve(a1, a3, def->unk20[1]);
    dmg += actor_magic_component_curve(a1, a4, def->unk20[2]);
    dmg += actor_magic_component_curve(a1, a5, def->unk20[3]);
    dmg += actor_magic_component_curve(a1, a6, def->unk20[4]);
    dmg += actor_magic_component_curve(a1, a7, def->unk20[5]);
    dmg += actor_magic_component_curve(a1, a8, def->unk20[6]);
    dmg += actor_magic_component_curve(a1, a9, def->unk20[7]);
    if (dmg > 0x68db7) {
        dmg = 0x68db7;
    }
    dmg = (dmg * a10 / 5000 + 128) >> 4;
    ((void (*)(Object88 *, s32 *, u16, u16, u16, u16, u16, u16, u16, u16))
         asset_status.handlers[18])(obj, &dmg, a2, a3, a4, a5, a6, a7, a8, a9);
    if (dmg == 0) {
        return;
    }
    if (obj->unk1a != 0 && (kind & 0x40)) {
        if (mode & 0x20) {
            player_increment_magic_training(rec);
        } else if ((mode & 0x10) && a10 >= 2500) {
            player_increment_physical_power_training();
        }
    }
    if (mode & 0x10) {
        if (obj->unke == 0x13 && obj->unk70 == 0x10) {
            goto push;
        }
    } else if (mode & 0x20) {
        if (obj->unk28 & 0x40000) {
            goto push;
        }
        dodge = actor_find_target_of_type(def, 0x16);
        if (dodge != NULL &&
            angle_within_tolerance(obj->angle.vy, vector_xz_to_angle(obj->pos.vx - target->vx,
                                                                     obj->pos.vz - target->vz), 0x200)) {
            actor_set_target(obj, dodge);
            goto push;
        }
    }
    if ((kind & 0x40) && (mode & 0x10)) {
        switch (cur_avatar_status.unkcb) {
        case 0x1f:
            player_adjust_hp_on_weapon_hit((rand() * 9 >> 15) + 1);
            break;
        case 0x20:
            if (obj->unk1 == 59 || obj->unk1 == 60 || obj->unk1 == 61 || obj->unk1 == 63 || obj->unk1 == 66 ||
                obj->unk1 == 67 || obj->unk1 == 79 || obj->unk1 == 88 || obj->unk1 == 89 || obj->unk1 == 91) {
                if (obj->unk1a != 0) {
                    dmg = obj->unk1a;
                }
            }
            break;
        }
    }
    hp = obj->unk1a - dmg;
    scripts = def->scripts;
    if (hp <= 0) {
        if (obj->unk1a != 0 && kind == 0x40) {
            player_add_experience(def->unk1e);
        }
        if (obj->unk1 < 43) {
            if (cur_evt_state.unk7d == 0) {
                cur_evt_state.unk7d = 1;
            }
            cur_evt_state.unk7e++;
        }
        hp = 0;
        actor_select_target_type_in_own_group(obj, 3);
    } else {
        for (i = 15; i != -1; i--) {
            script = *scripts++;
            if (script == NULL) {
                break;
            }
            if (script[0] == 2) {
                if (dmg >= SCRIPT_U16(script, 12)) {
                    chance = script[2];
                    if (chance == 0xff || rand() >> 7 < chance) {
                        actor_set_target(obj, script);
                        break;
                    }
                } else {
                    obj->unk28 |= 0x200000;
                    obj->unk84 = 0;
                }
            }
        }
    }
    obj->unk1a = hp;
push:
    if (obj->unk28 & 0x10) {
        link = &actor_status.objects[obj->unk22];
        mass = (actor_status.defs + link->unk2)->unk1[1];
    } else {
        mass = def->unk1[1];
    }
    if (target != NULL && mass < 240) {
        vector_displacement_to_pitch_yaw(obj->pos.vx - target->vx,
                                         obj->pos.vy - (obj->unk1e >> 1) - target->vy,
                                         obj->pos.vz - target->vz, &dir);
        pitch_yaw_to_forward_vector(&dir, &obj->vel);
        dmg = (SquareRoot0(SquareRoot0(dmg << 11)) << 10) / mass;
        dmg = (dmg << 5) / mass;
        if (dmg > 512) {
            dmg = 512;
        }
        vector3s_scale_shift12(dmg, &obj->vel);
        obj->vel.vx >>= 3;
        obj->vel.vz >>= 3;
        obj->vel.vy >>= 6;
        if (obj->unk28 & 0x10) {
            link->vel.vx = obj->vel.vx;
            link->vel.vy = obj->vel.vy;
            link->vel.vz = obj->vel.vz;
            if (link->unke != 3) {
                actor_select_target_type_in_own_group(link, 2);
            }
        }
    }
    obj->unkd = 0x10;
}
