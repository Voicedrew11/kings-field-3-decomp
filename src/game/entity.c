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
Object88 *actor_pool_find_free(void)
{
    Object88 *obj;
    s32 i;

    obj = &actor_status.objects[190];
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
void actor_set_home_position(Object88 *obj)
{
    obj->pos.vx = (obj->unk8 << 11) + obj->unk24;
    obj->pos.vz = (obj->unk7 << 11) + obj->unk22;
    obj->pos.vy = collision_sample_map_layer_height(obj->unk6, obj->pos.vx, obj->pos.vz, obj->unk1c,
                                                    obj->unk1e);
    if (obj->pos.vy >= 0) {
        obj->pos.vy = 0;
    }
    if (obj->unk28 & 0x400) {
        obj->pos.vy = map_world.unk1200c;
    }
    if (!(obj->unk28 & 0x10)) {
        obj->pos.vy += obj->unk26;
    }
}

ADDRESS(0x8004b624, 0x74)
void actor_copy_group_defaults(Object88 *obj)
{
    Object78 *def;

    def = &actor_status.defs[obj->unk2];
    obj->unk1 = def->unk0;
    obj->unk1a = def->unk1a;
    obj->unk1c = def->unk12;
    obj->unk1e = def->unk14;
    obj->unk28 = def->unk34;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = def->unk32;
}

ADDRESS(0x8004b698, 0xd8)
void actor_initialize_from_group(Object88 *obj)
{
    actor_copy_group_defaults(obj);
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
void actor_set_lifecycle_and_home_position(Object88 *obj)
{
    obj->unk9 = 3;
    actor_set_home_position(obj);
}

ADDRESS(0x8004b794, 0xd4)
void actor_disable_type3_transition_actors(void)
{
    Object88 *obj;
    s32 i;

    obj = actor_status.objects;
    for (i = 199; i != -1; i--) {
        if (obj->unk0 == 1 && obj->unk9 == 1 && obj->unke == 3 && obj->unkf == 0xf0) {
            if (obj->unk70 != 0 || obj->unk18 > 0x800) {
                ((void (*)(Object88 *))asset_status.handlers[19])(obj);
                actor_set_lifecycle_and_home_position(obj);
            }
        }
        obj++;
    }
}

ADDRESS(0x8004b868, 0x5c)
void actor_prepare_and_initialize(Object88 *obj)
{
    obj->angle.vx = obj->angle.vz = 0;
    obj->unk3 = obj->unk6;
    obj->angle.vy = obj->unk20;
    actor_set_home_position(obj);
    actor_initialize_from_group(obj);
    if (obj->unk0 == 3) {
        obj->unk3 = 0;
    }
}

ADDRESS(0x8004b8c4, 0x38)
void actor_prepare_and_initialize_by_index(u16 index)
{
    actor_prepare_and_initialize(&actor_status.objects[index]);
}

ADDRESS(0x8004b8fc, 0x50)
void actor_pool_clear(void)
{
    Object88 *obj;
    u16 i;

    actor_status.unk9d24 = 0;
    obj = actor_status.objects;
    i = 0;
    while (i < 200) {
        i++;
        obj->unk0 = OBJECT88_FREE;
        obj->unk9 = 0;
        obj->unk5c = 0;
        obj++;
    }
    actor_status.unk9d00 = 0;
}

/* Starts a script record (0 stops the script). */
ADDRESS(0x8004b94c, 0x38)
void actor_set_target(Object88 *obj, u8 *script)
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
