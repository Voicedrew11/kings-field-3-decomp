/*
 * GAME.EXE: object drawing (WIP module): model ids, resident and streamed
 * model files, the map cell grid around the viewer, the sound banks of the
 * objects in view and the per-frame object drawer.
 */
/* func_80040ae4 reads Data8018fad4.step as volatile (see game/data.h). */
#define GAME_RENDER_MODULE
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <game/sound.h>
#include <game/entity.h>
#include <game/object44.h>
#include <INLINE.H>
#include <GTEMAC.H>

/*
 * Draws model file id (an object's model) turned by rotation and placed 100
 * units down in view space: lit by light record light_id, posed by
 * func_800431e8 (the base pose when no slot is free) and drawn by
 * func_80039428 at depth offset depth.
 * Not exact only in the read of DAT_8009c2e0: retail reads it $gp-relative,
 * which ASPSX emits only for .sdata defined in the same file (hud.c).
 * K&R definition: depth is u16 here, int at the calls (func_80040ae4).
 */
ADDRESS(0x800400ac, 0x4bc)
void func_800400ac(id, rotation, owner, anim, time, shade, light_id, depth)
    u16 id;
    SVECTOR *rotation;
    AnimSlot **owner;
    u16 anim;
    u16 time;
    s32 shade;
    s32 light_id;
    u16 depth;
{
    MATRIX m;
    ScratchPad *scratch;
    Record6c *light;
    u8 *file;
    u8 *model;
    s32 i;
    s32 offset;

    m.t[0] = 0;
    m.t[1] = 100;
    m.t[2] = 0;
    func_800166f4(rotation, &m);
    scratch = SCRATCH;
    gte_MulMatrix0(&DAT_80199154.unk159f8, &m, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    light = &DAT_80199154.unk15da8[light_id & 0x7f];
    gte_SetColorMatrix(&light->unk50);
    gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
    gte_SetLightMatrix(&light->unk0[0]);
    file = DAT_80199154.models[id];
    model = file + ((ModelFile *)file)->vertices;
    SCRATCH->ot = DAT_80199154.ot;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->model = model;
    SCRATCH->next = DAT_80199154.pool->next;
    SCRATCH->end = DAT_80199154.pool->end;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->draw.count.clut = 0;
    *(u32 *)&SCRATCH->rgb = *(u32 *)&DAT_8009c2e0;
    if (func_800431e8(owner, id, anim, time, ((ModelData *)model)->parts[0].unk4) == 0) {
        u8 *vertex;
        ScreenVertex *screen;

        SCRATCH->part = ((ModelData *)SCRATCH->model)->parts;
        offset = SCRATCH->part->vertices + 12;
        vertex = SCRATCH->vertices = (u8 *)(offset + (u32)SCRATCH->model);
        screen = SCRATCH->screen;
        i = ((ModelData *)model)->parts[0].unk4;
        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rtps();
            gte_stsxy(&screen->sxy);
            gte_stszotz(&screen->z);
            screen->fog = 0;
            screen++;
            vertex += 8;
        }
    } else {
        u8 *vertex;
        ScreenVertex *screen;

        i = ((ModelData *)model)->parts[0].unk4;
        screen = SCRATCH->screen;
        vertex = SCRATCH->vertices;
        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rtps();
            gte_stsxy(&screen->sxy);
            gte_stszotz(&screen->z);
            screen->fog = 0;
            screen++;
            vertex += 8;
        }
    }
    func_80039428(0, depth, shade);
    DAT_80199154.pool->next = scratch->next;
    DAT_80199154.unk159bc = scratch->draw.count.unk4;
    DAT_80199154.ot_count = scratch->draw.count.ot_count;
}

ADDRESS(0x80040568, 0x80)
u16 func_80040568(u16 id)
{
    if (id < 300) {
        return id + 256;
    }
    if ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.unk17 == 1 && DAT_8018fad4.previous[0] != 0xff) {
        return id + (DAT_8018fad4.previous[0] * 32 + 256);
    }
    return id + (DAT_8018fad4.area[0] * 32 + 256);
}

ADDRESS(0x800405e8, 0x70)
void func_800405e8(u16 id, u8 **out)
{
    u8 *model;
    u8 state;

    model = DAT_80199154.models[id];
    *out = model;
    if (id < 104) {
        return;
    }
    if (model != NULL) {
        state = func_8001947c(model);
        if (state == 1 || state == 2) {
            return;
        }
    }
    *out = NULL;
}

/* Load-complete callback for a streamed model file. */
ADDRESS(0x80040658, 0x3c)
void func_80040658(u8 *file)
{
    func_8003591c(file + *(s32 *)(file + 8));
    func_80019474(file, 2);
}

ADDRESS(0x80040694, 0x74)
u8 func_80040694(VECTOR *pos)
{
    s32 row;
    s32 col;

    row = (pos->vz >> 11) + DAT_80199154.grid_z;
    if (row >= 0 && row < 25) {
        col = (pos->vx >> 11) + DAT_80199154.grid_x;
        if (col >= 0 && col < 25) {
            return DAT_80199154.grid[row][col];
        }
        return 0;
    }
    return 0;
}

ADDRESS(0x80040708, 0xc4)
u8 func_80040708(VECTOR *pos, s32 radius)
{
    u8 cells;
    s32 row;
    s32 col0;
    s32 col;
    s32 i;
    s32 j;
    u8 *line;
    u8 *cell;

    cells = 0;
    i = radius * 2;
    row = (pos->vz >> 11) + DAT_80199154.grid_z - radius;
    col0 = (pos->vx >> 11) + DAT_80199154.grid_x - radius;
    line = DAT_80199154.grid[row];
    do {
        if (row >= 0 && (u32)row < 25) {
                j = radius * 2;
                col = col0;
                cell = &line[col];
                do {
                    if (col >= 0 && (u32)col < 25) {
                        cells |= *cell;
                    }
                    cell++;
                    col++;
                } while (--j != -1);
        }
        line += 25;
        row++;
    } while (--i != -1);
    return cells;
}

ADDRESS(0x800407cc, 0x64)
s32 func_800407cc(VECTOR *pos, s32 range_x, s32 range_z)
{
    s32 inside;
    s32 x;
    s32 z;

    inside = 0;
    z = pos->vz >> 11;
    if (z - range_z <= DAT_80199154.unk15b14 && z + range_z >= DAT_80199154.unk15b14) {
        x = pos->vx >> 11;
        if (x - range_x <= DAT_80199154.unk15b10) {
            inside = x + range_x >= DAT_80199154.unk15b10;
        }
    }
    return inside;
}

ADDRESS(0x80040830, 0xa0)
void func_80040830(s32 file, s32 entry, s32 id)
{
    u8 *model;

    model = func_8001939c((HeapBlock *)DAT_800c8954, func_80019f14(file, entry), &DAT_80199154.models[id]);
    if (model != NULL) {
        func_80019474(model, 3);
        func_8001949c(model, id);
        func_80019d48(file, entry, model, func_80040658);
    }
}

ADDRESS(0x800408d0, 0xf8)
void func_800408d0(s32 file, s32 entry, s32 bank, s32 count)
{
    u8 *wanted;
    s32 end;
    SoundSlot *slot;

    wanted = SCRATCH->u.load.sound_wanted;
    for (end = count + entry; entry < end; entry++, bank++) {
        slot = DAT_801b09fc.banks[bank].slot;
        if (*wanted++ != 0) {
            if (slot == NULL) {
                func_80015ce0(file, entry, bank);
                continue;
            }
            if (slot->state == 2) {
                slot->state = 1;
            }
        } else if (slot != NULL && slot->state == 1) {
            slot->state = 2;
        }
    }
}

ADDRESS(0x800409c8, 0x11c)
void func_800409c8(s32 file, s32 entry, s32 id, s32 count)
{
    u8 *wanted;
    s32 end;
    u8 *model;

    wanted = SCRATCH->u.load.model_wanted;
    for (end = count + entry; entry < end; entry++, id++) {
        if (*wanted++ != 0) {
            model = DAT_80199154.models[id];
            if (model == NULL) {
                func_80040830(file, entry, id);
                continue;
            }
            if (func_8001947c(model) == 1) {
                func_80019474(model, 2);
            }
        } else {
            model = DAT_80199154.models[id];
            if (model != NULL && func_8001947c(model) != 3) {
                func_80019474(model, 1);
            }
        }
    }
}

/*
 * Draws the objects in view once a frame and keeps their files loaded: the
 * Object88 records (models from 128, sound banks from 51), the Object44
 * records (models from 256 and the area bank, sound banks from 147; kind
 * 0x1f plays its sound by the player's distance), the Object4c records and
 * the placed effects. The passes flag the model ids and sound banks in use
 * and func_800409c8/func_800408d0 load the missing ones and release the rest.
 */
ADDRESS(0x80040ae4, 0xff0)
void func_80040ae4(void)
{
    VECTOR pos;
    u8 *model;
    ScratchPad *scratch;
    Object88 *obj;
    Object78 *def;
    Object44 *rec;
    Object44Def *rec_def;
    Object4c *obj4c;
    Effect *effect;
    VECTOR *at;
    s32 i;
    s32 now;
    s32 near;
    s32 dist;
    s32 range;
    s32 volume;
    SoundEffect *sound;
    u16 id;
    u8 mask;
    u8 cells;
    u8 flags;

    scratch = SCRATCH;
    SCRATCH->u.load.model_wanted = SCRATCH->u.load.models;
    SCRATCH->u.load.sound_wanted = SCRATCH->u.load.sounds;
    SCRATCH->ot = DAT_80199154.ot;
    SCRATCH->ot2 = DAT_80199154.ot2;
    func_80018fbc((u32 *)SCRATCH->u.load.models, 0, 0x20);
    func_80018fbc((u32 *)SCRATCH->u.load.sounds, 0, 0x18);
    SCRATCH->draw.count.clut = 0;
    obj = DAT_80185da8.objects;
    for (i = 199; i != -1; i--, obj++) {
        if (obj->unk9 != 1) {
            continue;
        }
        if (obj->unk28 & 0x2000) {
            mask = obj->unk3 | 0x10;
        } else {
            mask = obj->unk3;
        }
        if (!(obj->unk28 & 0x80000)) {
            if (!(func_80040694(&obj->pos) & mask)) {
                continue;
            }
        } else {
            if (!(func_80040708(&obj->pos, 3) & obj->unk3)) {
                continue;
            }
        }
        func_800405e8(obj->unk1 + 128, &model);
        if (model != NULL) {
            at = func_8004eee0(obj, &pos);
            if (obj->unk28 & 0x20) {
                scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
                func_8003e34c(obj->unk3, obj->unk1 + 128, &obj->pos, &scratch->u.load.angle,
                              &obj->scale, &obj->unk5c, &DAT_8007e4c4, obj->unkc, obj->unk18,
                              obj->unk14, obj->unk16, obj->unk13, obj->unk15);
            } else {
                scratch->u.load.angle.vx = obj->angle.vx;
                scratch->u.load.angle.vy = obj->angle.vy + 2048;
                scratch->u.load.angle.vz = obj->angle.vz;
                func_8003e34c(obj->unk3, obj->unk1 + 128, at, &scratch->u.load.angle,
                              &obj->scale, &obj->unk5c, &DAT_80199154.unk159f8, obj->unkc,
                              obj->unk18, obj->unk14, obj->unk16, obj->unk13, obj->unk15);
            }
        }
        def = &DAT_80185da8.defs[obj->unk2];
        scratch->u.load.sounds[def->unk7[0]] = 1;
        scratch->u.load.sounds[def->unk7[1]] = 1;
        scratch->u.load.models[obj->unk1] = 1;
    }
    func_800409c8(0, 0, 0x80, 0x80);
    func_800408d0(2, 0x91, 0x33, 0x60);
    now = DAT_801c12e4.frames;
    func_80018fbc((u32 *)scratch->u.load.models, 0, 0x53);
    func_80018fbc((u32 *)scratch->u.load.sounds, 0, 8);
    if (DAT_80199154.unk15996 == 0) {
        DAT_8018fad4.unk20 = NULL;
    }
    scratch->draw.count.clut = 0;
    rec = DAT_8018fb3c.objects;
    for (i = 395; i != -1; i--, rec++) {
        if (rec->unk6 == 0xff) {
            continue;
        }
        rec->unk3 &= 0x7f;
        switch (rec->unk4) {
        case 0xe5:
        case 0xe9:
            break;
        case 0xf2:
            if (func_800407cc(&rec->pos, rec->arg.m.unk38, rec->arg.m.unk39) == 0) {
                break;
            }
            scratch->u.load.models[rec->unk6] = 1;
            id = func_80040568(rec->unk6);
            func_800405e8(id, &model);
            if (model == NULL) {
                break;
            }
            scratch->u.load.angle.vx = rec->angle.vx;
            scratch->u.load.angle.vy = rec->angle.vy + 2048;
            scratch->u.load.angle.vz = rec->angle.vz;
            pos.vx = ((rec->arg.m.unk3a << 11) + rec->pos.vx) >> 1;
            pos.vz = ((rec->arg.m.unk3c << 11) + rec->pos.vz) >> 1;
            pos.vy = (rec->pos.vy - (rec->arg.m.unk3e << 11)) >> 1;
            func_8003f304(rec->unk0, id, &pos, &scratch->u.load.angle, &rec->scale,
                          &rec->arg.m.unk34, &DAT_80199154.unk159f8, rec->unk1, rec->unka,
                          rec->unk5, rec->unk10, rec->unk2, rec->unke);
            rec->unk3 |= 0x80;
            break;
        case 0xf0:
            if (DAT_80199154.unk15996 == 0) {
                break;
            }
            if (func_800407cc(&rec->pos, rec->arg.v.unk38, rec->arg.v.unk39) == 0) {
                break;
            }
            id = rec->unk6 - 221;
            func_800405e8(id, &model);
            if (model != NULL) {
                func_800400ac(id, &rec->angle, &rec->arg.v.unk34, rec->unk1, rec->unka,
                              rec->arg.v.unk3c, rec->arg.v.unk3b, 0x1fff - rec->arg.v.unk3a);
                rec->unk3 |= 0x80;
            }
            DAT_8018fad4.unk24 = *rec;
            DAT_8018fad4.unk20 = &DAT_8018fad4.unk24;
            break;
        case 0x1f:
            if (func_80046884(rec->pos.vx >> 11, rec->pos.vz >> 11, rec->arg.s.unk38,
                              rec->arg.s.unk39, 0x8000) != 0) {
                sound = &DAT_801b09fc.effects[rec->arg.s.unk3a + 496];
                if (sound->bank >= 147 && sound->bank < 179) {
                    scratch->u.load.sounds[sound->bank - 147] = 1;
                }
                if (rec->arg.s.unk40 - now < 0) {
                    rec->arg.s.unk40 = now + rec->arg.s.unk3e * 6;
                    range = rec->arg.s.unk38 << 10;
                    dist = DAT_801b24e4.pos.vx - (range + rec->pos.vx);
                    if (dist < 0) {
                        dist = -dist;
                    }
                    near = range - dist;
                    range = rec->arg.s.unk39 << 10;
                    dist = DAT_801b24e4.pos.vz - (range + rec->pos.vz);
                    if (dist < 0) {
                        dist = -dist;
                    }
                    dist = range - dist;
                    if (dist < near) {
                        near = dist;
                    }
                    range = rec->arg.s.unk3c << 11;
                    if (near >= range) {
                        volume = rec->arg.s.unk3b;
                    } else {
                        if (range == 0) {
                            break;
                        }
                        volume = rec->arg.s.unk3b * near / range;
                    }
                    if (rec->arg.s.unk3d & 1) {
                        dist = DAT_801b24e4.pos.vy - rec->pos.vy;
                        if (dist < 0) {
                            dist = -dist;
                        }
                        volume -= (rec->arg.s.unk3b * dist) >> 13;
                    }
                    if (volume >= 20) {
                        func_8001576c(rec->arg.s.unk3a + 496, volume);
                    }
                }
            } else {
                rec->arg.s.unk40 = now + rec->arg.s.unk3e * 6;
            }
            break;
        default:
            if (!(rec->unk3 & 2) || (rec->unk3 & 8)) {
                cells = func_80040694(&rec->pos);
                if (!(cells & rec->unk0) && !(rec->unk3 & 8)) {
                    break;
                }
                rec_def = &DAT_8018fb3c.defs[rec->unk6];
            } else {
                rec_def = &DAT_8018fb3c.defs[rec->unk6];
                cells = func_80040708(&rec->pos, rec_def->unkc);
                if (!(cells & rec->unk0) && !(rec->unk3 & 8)) {
                    break;
                }
            }
            scratch->u.load.models[rec->unk6] = 1;
            scratch->u.load.sounds[rec_def->unk2] = 1;
            id = func_80040568(rec->unk6);
            func_800405e8(id, &model);
            if (model == NULL) {
                break;
            }
            scratch->u.load.angle.vx = rec->angle.vx;
            scratch->u.load.angle.vy = rec->angle.vy + 2048;
            scratch->u.load.angle.vz = rec->angle.vz;
            flags = rec->unk2;
            if (rec->unk3 & 1) {
                if (cells & 4) {
                    flags |= 0x40;
                } else {
                    flags &= 0xbf;
                }
            }
            if (rec->unk3 & 0x40) {
                scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
                func_8003e34c(rec->unk0, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                              &rec->arg.m.unk34, &DAT_80199154.unk15ab8, rec->unk1, rec->unka,
                              rec->unk5, rec->unk10, flags, rec->unke);
            } else if (rec->unk3 & 8) {
                func_8003f304(rec->unk0, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                              &rec->arg.m.unk34, &DAT_80199154.unk159f8, rec->unk1, rec->unka,
                              rec->unk5, rec->unk10, flags, rec->unke);
            } else {
                func_8003e34c(rec->unk0, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                              &rec->arg.m.unk34, &DAT_80199154.unk159f8, rec->unk1, rec->unka,
                              rec->unk5, rec->unk10, flags, rec->unke);
            }
            rec->unk3 |= 0x80;
            break;
        }
    }
    scratch->u.load.models[265] = 0;
    scratch->u.load.models[264] = 0;
    func_800409c8(0, 0x80, 0x100, 0x118);
    scratch->u.load.model_wanted = &scratch->u.load.models[300];
    if ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.unk17 == 1
        && DAT_8018fad4.previous[0] != 0xff) {
        func_800409c8(1, DAT_8018fad4.previous[0] * 32, DAT_8018fad4.previous[0] * 32 + 556, 32);
    } else {
        func_800409c8(1, DAT_8018fad4.area[0] * 32, DAT_8018fad4.area[0] * 32 + 556, 32);
    }
    func_800408d0(2, 0xf1, 0x93, 0x20);
    if ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.step > 0
        && (rec = DAT_8018fad4.unk20) != NULL) {
        id = rec->unk6 - 221;
        func_800400ac(id, &rec->angle, &rec->arg.v.unk34, rec->unk1, rec->unka,
                      rec->arg.v.unk3c, rec->arg.v.unk3b, 0x1fff - rec->arg.v.unk3a);
        rec->unk3 |= 0x80;
    }
    obj4c = DAT_801b77ec.objects;
    for (i = 127; i != -1; i--, obj4c++) {
        if (obj4c->unk0 == 0xff) {
            continue;
        }
        if ((obj4c->unk8 & 3) == 0) {
            continue;
        }
        if ((obj4c->unk8 & 3) != 2 && !(func_80040694(&obj4c->pos) & obj4c->unka)) {
            continue;
        }
        scratch->draw.count.clut = obj4c->unke;
        switch (obj4c->unk8 & 0xc) {
        case 0:
            scratch->u.load.angle.vx = obj4c->angle.vx;
            scratch->u.load.angle.vy = obj4c->angle.vy + 2048;
            scratch->u.load.angle.vz = obj4c->angle.vz;
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &scratch->u.load.angle,
                          &obj4c->scale, &obj4c->unk40, &DAT_80199154.unk159f8, obj4c->unk4,
                          obj4c->unk14, obj4c->unkc, obj4c->unk12, obj4c->unk9, -60);
            break;
        case 4:
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &obj4c->angle,
                          &obj4c->scale, &obj4c->unk40, &DAT_8007e4c4, obj4c->unk4,
                          obj4c->unk14, obj4c->unkc, obj4c->unk12, obj4c->unk9, -60);
            break;
        case 8:
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &obj4c->angle,
                          &obj4c->scale, &obj4c->unk40, &DAT_80199154.unk15ab8, obj4c->unk4,
                          obj4c->unk14, obj4c->unkc, obj4c->unk12, obj4c->unk9, -60);
            break;
        case 0xc:
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &obj4c->angle,
                          &obj4c->scale, &obj4c->unk40, NULL, obj4c->unk4, obj4c->unk14,
                          obj4c->unkc, obj4c->unk12, obj4c->unk9, 20);
            break;
        }
    }
    effect = DAT_80182968;
    scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
    scratch->draw.count.clut = 0;
    for (i = 127; i != -1; i--, effect++) {
        if (effect->id == 0xffff) {
            continue;
        }
        if (effect->id & 0x8000) {
            func_8003f304(effect->unk2, (effect->id & 0x7fff) + 40, &effect->pos,
                          &scratch->u.load.angle, NULL, NULL, &DAT_8007e4c4, effect->unk5 + 128, 0,
                          0x3c, 0x1000, 7, 0);
        } else if (func_80040694(&effect->pos) & effect->unk2) {
            func_8003e34c(effect->unk2, effect->id + 40, &effect->pos, &scratch->u.load.angle,
                          NULL, NULL, &DAT_80199154.unk15ab8, effect->unk5 + 128, 0, 0x36, 0x1000,
                          5, 0);
        }
        if (effect->unk4 != 0 && DAT_80182964 % effect->unk4 == 0) {
            effect->unk5++;
            if (effect->unk5 >= effect->unk3) {
                effect->unk5 = 0;
            }
        }
    }
    DAT_80182964++;
}
