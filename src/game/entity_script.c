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
    func_8004bf1c(func_80016c08(DAT_80185da8.current->pos.vx - DAT_801b24e4.x,
                                DAT_80185da8.current->pos.vz - DAT_801b24e4.z));
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
    Object88 *obj;
    Object78 *def;
    u8 kind;
    s32 dist;
    u8 chance;
    u8 *script;

    obj = DAT_80185da8.current;
    def = DAT_80185da8.def;
    kind = obj->unk0;
    switch (obj->unk9) {
    case 0:
        dist = func_80016ec8(&obj->pos, DAT_801b24e4.x, 0xffff, DAT_801b24e4.z, (def->unka + 1) << 11, 0, 0);
        if (dist == -1) {
            break;
        }
        if (kind == 3 || kind == 4) {
            if (DAT_80185da8.target->unk9 != 1) {
                break;
            }
            func_8004b868(DAT_80185da8.current);
            func_8004bf1c(dist);
            break;
        }
        if (kind == 2) {
            chance = obj->unka;
            if (chance != 0xff && chance < rand() >> 4) {
                break;
            }
        } else if (dist < def->unka << 11 && DAT_801b24e4.unke == 0) {
            goto wake;
        } else if (kind != 1) {
            if (kind != 0) {
                goto wake;
            }
            chance = obj->unka;
            if (chance == 0 || chance < rand() >> 7) {
                goto wake;
            }
        }
        if (func_8004d644(obj->pos.vx, obj->pos.vy, obj->pos.vz, def->unk12, def->unk14) == -1) {
            func_8004b868(DAT_80185da8.current);
            if ((script = func_8004c068(def, 0x15)) != NULL || (script = func_8004c068(def, 6)) != NULL ||
                (script = func_8004c068(def, 0x1a)) != NULL) {
                func_8004b94c(obj, script);
                break;
            }
            func_8004bf1c(dist);
            break;
        }
    wake:
        if (kind != 2) {
            obj->unk9 = 2;
        }
        break;
    case 1:
        if (func_80016ec8(&obj->pos, DAT_801b24e4.x, 0xffff, DAT_801b24e4.z, def->unkb << 11, 0, 0) != -1) {
            break;
        }
        func_80033c4c(obj->pos.vx, obj->pos.vz, obj->unk1c, -1);
        obj->unk9 = 0;
        func_8004b560(obj);
        break;
    case 2:
        if (kind == 3 || kind == 4) {
            if (DAT_80185da8.target->unk9 == 1) {
                break;
            }
            obj->unk9 = 0;
            func_8004b560(obj);
            break;
        }
        if (func_80016ec8(&obj->pos, DAT_801b24e4.x, 0xffff, DAT_801b24e4.z, def->unkb << 11, 0, 0) != -1) {
            break;
        }
        obj->unk9 = 0;
        func_8004b560(obj);
        break;
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
