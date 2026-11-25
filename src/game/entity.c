/*
 * GAME.EXE: Object88 records (WIP module): allocation, placement on the
 * map from the record's cell and offsets, set-up from its Object78 record,
 * script record selection, and the searches over all 200 records.
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

/* A free record among the last ten. */
ADDRESS(0x8004b524, 0x3c)
Object88 *func_8004b524(void)
{
    Object88 *obj;
    s32 i;

    obj = &DAT_80185da8.objects[190];
    for (i = 9; i != -1; i--) {
        if (obj->unk0 == OBJECT88_FREE) {
            return obj;
        }
        obj++;
    }
    return NULL;
}

/* Places the record in its map cell and on the floor. */
ADDRESS(0x8004b560, 0xc4)
void func_8004b560(Object88 *obj)
{
    obj->pos.vx = (obj->unk8 << 11) + obj->unk24;
    obj->pos.vz = (obj->unk7 << 11) + obj->unk22;
    obj->pos.vy = collision_sample_map_layer_height(obj->unk6, obj->pos.vx, obj->pos.vz, obj->unk1c,
                                                    obj->unk1e);
    if (obj->pos.vy >= 0) {
        obj->pos.vy = 0;
    }
    if (obj->unk28 & 0x400) {
        obj->pos.vy = DAT_801d4464.unk1200c;
    }
    if (!(obj->unk28 & 0x10)) {
        obj->pos.vy += obj->unk26;
    }
}

ADDRESS(0x8004b624, 0x74)
void func_8004b624(Object88 *obj)
{
    Object78 *def;

    def = &DAT_80185da8.defs[obj->unk2];
    obj->unk1 = def->unk0;
    obj->unk1a = def->unk1a;
    obj->unk1c = def->unk12;
    obj->unk1e = def->unk14;
    obj->unk28 = def->unk34;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = def->unk32;
}

ADDRESS(0x8004b698, 0xd8)
void func_8004b698(Object88 *obj)
{
    func_8004b624(obj);
    obj->unk9 = 1;
    obj->unkc = 0;
    obj->unk18 = 0;
    obj->unk11 = 0;
    obj->unkd = 0;
    obj->unke = 0;
    obj->unkf = 0xff;
    obj->unk60 = NULL;
    if (!(obj->unk5 & 1)) {
        obj->angle.vy = rand() >> 3;
    }
    obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
    obj->unk58 = 0;
    if (obj->unk1 < 50) {
        obj->unk14 = 0x3e;
    } else {
        obj->unk14 = 0x37;
    }
    obj->unk16 = 0x800;
    if (obj->unk28 & 0x80) {
        obj->unk13 |= 7;
    } else {
        obj->unk13 = 0;
    }
    map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->unk1c, 1);
}

ADDRESS(0x8004b770, 0x24)
void func_8004b770(Object88 *obj)
{
    obj->unk9 = 3;
    func_8004b560(obj);
}

ADDRESS(0x8004b794, 0xd4)
void func_8004b794(void)
{
    Object88 *obj;
    s32 i;

    obj = DAT_80185da8.objects;
    for (i = 199; i != -1; i--) {
        if (obj->unk0 == 1 && obj->unk9 == 1 && obj->unke == 3 && obj->unkf == 0xf0) {
            if (obj->unk70 != 0 || obj->unk18 > 0x800) {
                ((void (*)(Object88 *))DAT_8018fad4.handlers[19])(obj);
                func_8004b770(obj);
            }
        }
        obj++;
    }
}

ADDRESS(0x8004b868, 0x5c)
void func_8004b868(Object88 *obj)
{
    obj->angle.vx = obj->angle.vz = 0;
    obj->unk3 = obj->unk6;
    obj->angle.vy = obj->unk20;
    func_8004b560(obj);
    func_8004b698(obj);
    if (obj->unk0 == 3) {
        obj->unk3 = 0;
    }
}

ADDRESS(0x8004b8c4, 0x38)
void func_8004b8c4(u16 index)
{
    func_8004b868(&DAT_80185da8.objects[index]);
}

ADDRESS(0x8004b8fc, 0x50)
void func_8004b8fc(void)
{
    Object88 *obj;
    u16 i;

    DAT_80185da8.unk9d24 = 0;
    obj = DAT_80185da8.objects;
    i = 0;
    while (i < 200) {
        i++;
        obj->unk0 = OBJECT88_FREE;
        obj->unk9 = 0;
        obj->unk5c = 0;
        obj++;
    }
    DAT_80185da8.unk9d00 = 0;
}

/* Starts a script record (0 stops the script). */
ADDRESS(0x8004b94c, 0x38)
void func_8004b94c(Object88 *obj, u8 *script)
{
    obj->unk10 = obj->unke;
    if (script != NULL) {
        obj->unk60 = script;
        obj->unke = script[0];
        obj->unkf = 0;
    } else {
        obj->unk60 = NULL;
        obj->unke = 0xff;
        obj->unkf = 0xff;
    }
}
