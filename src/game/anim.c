/*
 * GAME.EXE: model animation (WIP module): key frame lookup, delta-coded
 * vertex poses blended by the sub-frame, and the twelve cached pose slots.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <MALLOC.H>
#include <RAND.H>
#include <game/data.h>
#include <game/render.h>

/* A delta run marker: the next word is a count instead of a vertex. */
#define DELTA_RUN (-32768)

ADDRESS(0x80042b70, 0x38)
void func_80042b70(u16 id)
{
    u8 *file;

    file = DAT_80199154.models[id];
    DAT_80199154.vertices = file + ((ModelFile *)file)->vertices;
}

ADDRESS(0x80042ba8, 0xbc)
void func_80042ba8(u16 id, u8 *file)
{
    u16 count;
    u32 size;

    count = *(u16 *)file;
    file += 4;
    while (count--) {
        size = *(u32 *)file;
        if (size >= 5) {
            DAT_80199154.models[id] = file;
            func_80042b70(id);
            func_8003591c(DAT_80199154.vertices);
        }
        file += size;
        id++;
    }
}

ADDRESS(0x80042c64, 0x48)
void func_80042c64(u16 id, u8 *file)
{
    DAT_80199154.models[id] = file;
    func_80042b70(id);
    func_8003591c(DAT_80199154.vertices);
}

ADDRESS(0x80042cac, 0xc4)
AnimKey *func_80042cac(ModelFile *model, s32 anim, s32 time, s32 *index, s32 *frac)
{
    s32 key;
    s32 end;
    s32 start;
    s32 i;
    s32 *keys;
    AnimKey *frame;
    u32 length;
    s32 t;
    u16 *list;

    key = 0;
    end = 0;
    list = (u16 *)((u8 *)model + ((s32 *)((u8 *)model + model->anims))[anim]);
    keys = (s32 *)(list + 2);
    start = 0;
    i = *list;
    while (--i != -1) {
        frame = (AnimKey *)((u8 *)model + *keys++);
        length = frame->length;
        end += length;
        if (time < end) {
            t = ((time - start) << 12) / length;
            if (frame->reverse != 0) {
                t = 0x1000 - t;
            }
            goto done;
        }
        start = end;
        key++;
    }
    key--;
    t = 0x1000;
done:
    *index = key;
    *frac = t;
    return frame;
}

ADDRESS(0x80042d70, 0xc4)
void func_80042d70(SVECTOR *pose, SVECTOR *base, s16 *deltas)
{
    s32 i;
    s32 j;
    s16 v;

    i = *deltas++;
    while (--i != -1) {
        v = *deltas++;
        if (v == DELTA_RUN) {
            j = *deltas++;
            while (--j != -1) {
                pose->vx = base->vx;
                pose->vy = base->vy;
                pose->vz = base->vz;
                pose++;
                base++;
            }
        } else {
            pose->vx = v;
            pose->vy = *deltas++;
            base++;
            pose->vz = *deltas++;
            pose++;
        }
    }
}

ADDRESS(0x80042e34, 0x7c)
void func_80042e34(SVECTOR *pose, s16 *deltas)
{
    s32 i;
    s16 v;

    i = *deltas++;
    while (--i != -1) {
        v = *deltas++;
        if (v == DELTA_RUN) {
            pose += *deltas++;
        } else {
            pose->vx = v;
            pose->vy = *deltas++;
            pose->vz = *deltas++;
            pose++;
        }
    }
}

ADDRESS(0x80042eb0, 0x2bc)
void func_80042eb0(SVECTOR *pose, s16 *deltas, s32 t)
{
    MATRIX m;
    VECTOR scale;
    s32 i;
    s32 j;
    s32 n;
    s16 v;
    s16 *d;
    SVECTOR *out;

    scale.vx = scale.vy = scale.vz = t;
    i = *deltas++;
    n = 0;
    out = pose;
    d = m.m[0];
    while (--i != -1) {
        v = *deltas++;
        if (v == DELTA_RUN) {
            pose += *deltas++;
            if (n != 0) {
                ScaleMatrix(&m, &scale);
                d = m.m[0];
                while (--n != -1) {
                    out->vx += *d++;
                    out->vy += *d++;
                    out->vz += *d++;
                    out++;
                }
                n = 0;
                d = m.m[0];
            }
            out = pose;
        } else {
            *d++ = v - pose->vx;
            *d++ = *deltas++ - pose->vy;
            *d++ = *deltas++ - pose->vz;
            pose++;
            if (n == 2) {
                ScaleMatrix(&m, &scale);
                out[0].vx += m.m[0][0];
                out[0].vy += m.m[0][1];
                out[0].vz += m.m[0][2];
                out[1].vx += m.m[1][0];
                out[1].vy += m.m[1][1];
                out[1].vz += m.m[1][2];
                out[2].vx += m.m[2][0];
                out[2].vy += m.m[2][1];
                out[2].vz += m.m[2][2];
                n = 0;
                d = m.m[0];
                out = pose;
            } else {
                n++;
            }
        }
    }
    if (n != 0) {
        ScaleMatrix(&m, &scale);
        d = m.m[0];
        while (--n != -1) {
            out->vx += *d++;
            out->vy += *d++;
            out->vz += *d++;
            out++;
        }
    }
}

ADDRESS(0x8004316c, 0x7c)
s16 *func_8004316c(s16 *deltas, s32 index)
{
    s32 k;
    s32 i;

    k = 0;
    i = *deltas++;
    while (--i != -1) {
        if (*deltas == DELTA_RUN) {
            deltas++;
            k += *deltas++;
            if (index < k) {
                return NULL;
            }
        } else {
            if (k == index) {
                return deltas;
            }
            k++;
            deltas += 3;
        }
    }
    return NULL;
}

ADDRESS(0x800431e8, 0x344)
s32 func_800431e8(AnimSlot **owner, s32 id, s32 anim, s32 time, s32 count)
{
    AnimSlot *slot;
    s32 index;
    s32 frac;
    ModelFile *model;
    AnimKey *key;
    s32 *deltas;
    u8 *data;
    u16 n;
    u16 *ids;
    u32 *src;
    u32 *dst;
    ScratchPad *scratch;
    s32 i;
    u16 c;

    slot = *owner;
    model = (ModelFile *)DAT_80199154.models[id];
    scratch = SCRATCH;
    if (model->unk4 == 0) {
        if (slot != NULL) {
            func_80043894(slot);
        }
        data = DAT_80199154.models[id];
        data += ((ModelFile *)data)->vertices;
        scratch->model = data;
        scratch->part = ((ModelData *)data)->parts;
        scratch->vertices = *(s32 *)(data + 12) + 12 + data;
        return 1;
    }
    if (slot == NULL) {
        func_800439b0(&slot);
        if (slot == NULL) {
            return 0;
        }
    init:
        slot->model = id;
        slot->owner = owner;
        while ((slot->pose = func_800194b0(count * 8)) == NULL) {
            func_800438e0();
        }
        *owner = slot;
    } else if (slot->model != id) {
        func_80043894(slot);
        slot->anim = 0xff;
        goto init;
    }
    key = func_80042cac(model, anim, time, &index, &frac);
    if (slot->anim != anim || slot->key != index) {
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = *(s32 *)(scratch->model + 12) + 12 + scratch->model;
        deltas = (s32 *)((u8 *)model + model->deltas);
        n = key->count;
        if (n != 0) {
            ids = key->ids;
            n--;
            func_80042d70(slot->pose, (SVECTOR *)scratch->vertices,
                          (s16 *)((u8 *)model + deltas[key->first]));
            while (n != 0) {
                i = *ids++;
                n--;
                func_80042e34(slot->pose, (s16 *)((u8 *)model + deltas[i]));
            }
        } else {
            src = (u32 *)scratch->vertices;
            dst = (u32 *)slot->pose;
            c = count;
            do {
                *dst++ = *src++;
                *dst++ = *src++;
            } while (--c != 0);
        }
        slot->base = deltas[key->base];
    }
    src = (u32 *)slot->pose;
    dst = (u32 *)DAT_80199154.pose;
    slot->anim = anim;
    slot->key = index;
    c = count;
    do {
        *dst++ = *src++;
        *dst++ = *src++;
    } while (--c != 0);
    func_80042eb0(DAT_80199154.pose, (s16 *)((u8 *)model + slot->base), frac);
    scratch->vertices = (u8 *)DAT_80199154.pose;
    slot->state = 2;
}

ADDRESS(0x8004352c, 0x29c)
s32 func_8004352c(u16 id, s32 anim, s32 time, s32 vertex, SVECTOR *out)
{
    u8 *loaded;
    ModelFile *file;
    u8 *data;
    SVECTOR *vertices;

    func_800405e8(id, &loaded);
    file = (ModelFile *)loaded;
    if (file == NULL) {
        out->vx = out->vy = out->vz = 0;
        return 1;
    }
    data = (u8 *)file + file->vertices;
    if (anim >= 128) {
    single:
        vertices = (SVECTOR *)(data + ((((ModelData *)data)->parts + (anim & 0x7f))->vertices + 12));
        *out = vertices[vertex];
    } else if (file->unk4 == 0) {
        anim = 0;
        goto single;
    } else {
        SVECTOR v;
        s32 index;
        s32 frac;
        AnimKey *key;
        s32 *deltas;
        u16 *ids;
        s32 n;
        s16 *d;

        vertices = (SVECTOR *)(data + (((ModelData *)data)->parts[0].vertices + 12));
        key = func_80042cac(file, anim, time, &index, &frac);
        v = vertices[vertex];
        deltas = (s32 *)((u8 *)file + file->deltas);
        ids = &key->first;
        n = key->count;
        while (--n != -1) {
            d = func_8004316c((s16 *)((u8 *)file + deltas[*ids++]), vertex);
            if (d != NULL) {
                v.vx = *d++;
                v.vy = *d++;
                v.vz = *d;
            }
        }
        d = func_8004316c((s16 *)((u8 *)file + deltas[key->base]), vertex);
        if (d != NULL) {
            v.vx = (((*d++ - v.vx) * frac) >> 12) + v.vx;
            v.vy = (((*d++ - v.vy) * frac) >> 12) + v.vy;
            v.vz = (((*d - v.vz) * frac) >> 12) + v.vz;
        }
        *out = v;
    }
    return 0;
}

ADDRESS(0x800437c8, 0x60)
s32 func_800437c8(s32 id, s32 part)
{
    u8 *file;
    ModelData *data;

    file = DAT_80199154.models[id];
    if (file == NULL) {
        return 0;
    }
    data = (ModelData *)(file + ((ModelFile *)file)->vertices);
    return part < 128 ? data->parts[0].unk4 : (data->parts + (part & 0x7f))->unk4;
}

ADDRESS(0x80043828, 0x30)
void func_80043828(void)
{
    AnimSlot *slot;
    u16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        slot->state = 0;
        slot->pose = NULL;
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043858, 0x3c)
void func_80043858(void)
{
    AnimSlot *slot;
    u16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        if (slot->state != 0) {
            slot->state = 1;
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043894, 0x4c)
void func_80043894(AnimSlot *slot)
{
    slot->state = 0;
    *slot->owner = NULL;
    if (slot->pose != NULL) {
        free(slot->pose);
        slot->pose = NULL;
    }
}

ADDRESS(0x800438e0, 0x60)
void func_800438e0(void)
{
    AnimSlot *slot;
    s16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        if (slot->state != 0) {
            func_80043894(slot);
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043940, 0x70)
void func_80043940(void)
{
    AnimSlot *slot;
    u16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        if (slot->state == 1) {
            func_80043894(slot);
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x800439b0, 0x58)
void func_800439b0(AnimSlot **out)
{
    u16 n;

    *out = DAT_80199154.slots;
    n = 12;
    do {
        if ((*out)->state == 0) {
            (*out)->anim = 0xff;
            return;
        }
        (*out)++;
    } while (--n != 0);
    *out = NULL;
}

ADDRESS(0x80043a08, 0x130)
void func_80043a08(EffectSpawn *spawn)
{
    Effect *effect;
    s32 i;

    effect = DAT_80182968;
    for (i = 127; i != -1; i--) {
        if (spawn->id != 0xffff) {
            effect->id = spawn->id;
            effect->unk2 = spawn->unk4;
            effect->unk3 = spawn->unk2;
            effect->unk4 = spawn->unk3;
            effect->pos.vx = (spawn->cell_x << 11) + spawn->dx;
            effect->pos.vz = (spawn->cell_z << 11) + spawn->dz;
            effect->pos.vy = func_80033b8c(effect->unk2, effect->pos.vx, effect->pos.vz, 0, 0)
                + spawn->dy;
            effect->unk5 = (rand() * effect->unk3) >> 15;
        } else {
            effect->id = 0xffff;
        }
        effect++;
        spawn++;
    }
}

ADDRESS(0x80043b38, 0x80)
void func_80043b38(u8 *file)
{
    TIM_IMAGE tim;
    u_long unused[128];

    OpenTIM((u_long *)file);
    while (ReadTIM(&tim) != NULL) {
        if (tim.paddr != NULL) {
            LoadImage(tim.prect, tim.paddr);
            DrawSync(0);
        }
        if (tim.caddr != NULL) {
            LoadImage(tim.crect, tim.caddr);
            DrawSync(0);
        }
    }
}
