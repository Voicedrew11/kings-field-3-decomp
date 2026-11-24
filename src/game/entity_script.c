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
void func_8004bf1c(s32 dist)
{
    Object88 *obj;
    Object78 *def;
    u8 **scripts;
    u8 *script;
    u8 *best;
    s32 best_score;
    s32 score;
    s32 i;

    obj = DAT_80185da8.current;
    def = DAT_80185da8.def;
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
        score = func_8004b984(script, dist);
        if (best_score < score) {
            best_score = score;
            best = script;
        }
    }
    if (best == NULL) {
        return;
    }
    if (best != obj->unk60 || obj->unkf == 0xff) {
        func_8004b94c(obj, best);
    }
}

ADDRESS(0x8004c01c, 0x4c)
void func_8004c01c(void)
{
    func_8004bf1c(func_80016c08(DAT_80185da8.current->pos.vx - DAT_801b24e4.pos.vx,
                                DAT_80185da8.current->pos.vz - DAT_801b24e4.pos.vz));
}

/* The script record of the given id, or 0. */
ADDRESS(0x8004c068, 0x48)
u8 *func_8004c068(Object78 *def, u8 id)
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
void func_8004c0b0(Object88 *obj, u8 id)
{
    func_8004b94c(obj, func_8004c068(&DAT_80185da8.defs[obj->unk2], id));
}

ADDRESS(0x8004c104, 0x2c)
void func_8004c104(void)
{
    DAT_80185da8.current->unkf = 0xff;
    func_8004c01c();
}

ADDRESS(0x8004c130, 0x54)
void func_8004c130(u8 anim)
{
    Object88 *obj;

    if (anim == 0xff) {
        return;
    }
    obj = DAT_80185da8.current;
    if (DAT_80199150 != 0 && DAT_80199148 == obj) {
        anim += 2;
    }
    obj->unkc = anim;
    obj->unk18 = 0;
}

ADDRESS(0x8004c184, 0x6c)
void func_8004c184(u8 anim)
{
    Object88 *obj;

    if (anim == 0xff) {
        return;
    }
    obj = DAT_80185da8.current;
    if (obj->unkc == anim) {
        return;
    }
    if (DAT_80199150 != 0 && DAT_80199148 == obj) {
        anim += 2;
    }
    obj->unkc = anim;
    obj->unk18 = 0;
}

/* Wakes, sleeps and retargets the current record by its distance to the player. */
ADDRESS(0x8004c1f0, 0x33c)
void func_8004c1f0(void)
{
    Object88 *obj = DAT_80185da8.current;
    Object78 *def = DAT_80185da8.def;
    u8 kind = obj->unk0;
    s32 dist;

    switch (obj->unk9) {
    case 0:
        dist = func_80016ec8(&obj->pos, DAT_801b24e4.pos.vx, 0xffff, DAT_801b24e4.pos.vz, (def->unka + 1) << 11, 0, 0);
        if (dist == -1) {
            return;
        }
        if (kind == 3 || kind == 4) {
            if (DAT_80185da8.target->unk9 != 1) {
                return;
            }
            func_8004b868(DAT_80185da8.current);
            func_8004bf1c(dist);
            return;
        }
        if (kind == 2) {
            u8 chance = obj->unka;
            if (chance != 0xff && chance < rand() >> 4) {
                return;
            }
        } else {
            if (dist < def->unka << 11 && DAT_801b24e4.unke == 0) {
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
        if (func_8004d644(obj->pos.vx, obj->pos.vy, obj->pos.vz, def->unk12, def->unk14) != -1) {
            if (kind != 2) {
                obj->unk9 = 2;
            }
            return;
        }
        func_8004b868(DAT_80185da8.current);
        {
            u8 *script = func_8004c068(def, 0x15);
            if (script == NULL) {
                script = func_8004c068(def, 6);
            }
            if (script == NULL) {
                script = func_8004c068(def, 0x1a);
            }
            if (script != NULL) {
                func_8004b94c(obj, script);
                return;
            }
        }
        func_8004bf1c(dist);
        return;
    wake:
        if (kind != 2) {
            obj->unk9 = 2;
        }
        return;
    case 1:
        dist = func_80016ec8(&obj->pos, DAT_801b24e4.pos.vx, 0xffff, DAT_801b24e4.pos.vz, def->unkb << 11, 0, 0);
        if (dist != -1) {
            return;
        }
        func_80033c4c(obj->pos.vx, obj->pos.vz, obj->unk1c, -1);
        obj->unk9 = 0;
        func_8004b560(obj);
        return;
    case 2:
        if (kind == 3 || kind == 4) {
            if (DAT_80185da8.target->unk9 == 1) {
                return;
            }
        } else {
            dist = func_80016ec8(&obj->pos, DAT_801b24e4.pos.vx, 0xffff, DAT_801b24e4.pos.vz, def->unkb << 11, 0, 0);
            if (dist != -1) {
                return;
            }
        }
        obj->unk9 = 0;
        func_8004b560(obj);
        return;
    }
}

/* Restarts or puts back every live record made from record kind. */
ADDRESS(0x8004c52c, 0xc0)
void func_8004c52c(u16 kind)
{
    Object88 *obj;
    s16 i;

    obj = DAT_80185da8.objects;
    for (i = 199; i != -1; i--) {
        if (obj->unk0 != OBJECT88_FREE && obj->unk2 == kind) {
            if (obj->unk9 == 1) {
                func_8004c0b0(obj, 3);
            } else {
                func_8004b770(obj);
            }
        }
        obj++;
    }
}

ADDRESS(0x8004c5ec, 0x7c)
s32 func_8004c5ec(s32 a, s32 b, s32 c)
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
 * resistances (func_8004c5ec each), strength a10 and flags a11 (bits 6-7
 * the attacker kind, bits 4-5 the hit kind); FDAT slot 18 adjusts the
 * damage. Starts the record's hurt or death script and pushes it away from
 * target (NULL: no push).
 */
ADDRESS(0x8004c668, 0x844)
void func_8004c668(u16 index, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6, u16 a7, u16 a8,
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

    obj = &DAT_80185da8.objects[index];
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
        obj = &DAT_80185da8.objects[obj->unk22];
    }
    def = &DAT_80185da8.defs[obj->unk2];
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
    dmg = func_8004c5ec(a1, a2, def->unk20[0]);
    dmg += func_8004c5ec(a1, a3, def->unk20[1]);
    dmg += func_8004c5ec(a1, a4, def->unk20[2]);
    dmg += func_8004c5ec(a1, a5, def->unk20[3]);
    dmg += func_8004c5ec(a1, a6, def->unk20[4]);
    dmg += func_8004c5ec(a1, a7, def->unk20[5]);
    dmg += func_8004c5ec(a1, a8, def->unk20[6]);
    dmg += func_8004c5ec(a1, a9, def->unk20[7]);
    if (dmg > 0x68db7) {
        dmg = 0x68db7;
    }
    dmg = (dmg * a10 / 5000 + 128) >> 4;
    DAT_8018fad4.handlers[18](obj, &dmg, a2, a3, a4, a5, a6, a7, a8, a9);
    if (dmg == 0) {
        return;
    }
    if (obj->unk1a != 0 && (kind & 0x40)) {
        if (mode & 0x20) {
            func_8002a10c(rec);
        } else if ((mode & 0x10) && a10 >= 2500) {
            func_8002a074();
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
        dodge = func_8004c068(def, 0x16);
        if (dodge != NULL &&
            func_80016a2c(obj->angle.vy, func_80016ab8(obj->pos.vx - target->vx, obj->pos.vz - target->vz), 0x200)) {
            func_8004b94c(obj, dodge);
            goto push;
        }
    }
    if ((kind & 0x40) && (mode & 0x10)) {
        switch (DAT_801b24e4.unkcb) {
        case 0x1f:
            func_80030c4c((rand() * 9 >> 15) + 1);
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
            func_8002a310(def->unk1e);
        }
        if (obj->unk1 < 43) {
            if (DAT_801ba984.unk7d == 0) {
                DAT_801ba984.unk7d = 1;
            }
            DAT_801ba984.unk7e++;
        }
        hp = 0;
        func_8004c0b0(obj, 3);
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
                        func_8004b94c(obj, script);
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
        link = &DAT_80185da8.objects[obj->unk22];
        mass = (DAT_80185da8.defs + link->unk2)->unk1[1];
    } else {
        mass = def->unk1[1];
    }
    if (target != NULL && mass < 240) {
        func_80016c90(obj->pos.vx - target->vx, obj->pos.vy - (obj->unk1e >> 1) - target->vy,
                      obj->pos.vz - target->vz, &dir);
        func_800167cc(&dir, &obj->vel);
        dmg = (SquareRoot0(SquareRoot0(dmg << 11)) << 10) / mass;
        dmg = (dmg << 5) / mass;
        if (dmg > 512) {
            dmg = 512;
        }
        func_80016928(dmg, &obj->vel);
        obj->vel.vx >>= 3;
        obj->vel.vz >>= 3;
        obj->vel.vy >>= 6;
        if (obj->unk28 & 0x10) {
            link->vel.vx = obj->vel.vx;
            link->vel.vy = obj->vel.vy;
            link->vel.vz = obj->vel.vz;
            if (link->unke != 3) {
                func_8004c0b0(link, 2);
            }
        }
    }
    obj->unkd = 0x10;
}
