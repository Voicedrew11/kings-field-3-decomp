/*
 * GAME.EXE: Actor records (WIP module): allocation, placement on the
 * map from the record's cell and offsets, set-up from its ActorGroup record,
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
Actor *actor_pool_find_free(void)
{
    Actor *obj;
    s32 i;

    obj = &actor_status.objects[190];
    for (i = 9; i != -1; i--) {
        if (obj->slot_state == ACTOR_FREE) {
            return obj;
        }
        obj++;
    }
    return NULL;
}

/* Places the record in its map cell and on the floor. */
ADDRESS(0x8004b560, 0xc4)
void actor_set_home_position(Actor *obj)
{
    obj->pos.vx = (obj->home_cell_x << 11) + obj->unk24;
    obj->pos.vz = (obj->home_cell_z << 11) + obj->unk22;
    obj->pos.vy = collision_sample_map_layer_height(obj->home_layer_mask, obj->pos.vx, obj->pos.vz,
                                                    obj->collision_radius, obj->collision_height);
    if (obj->pos.vy >= 0) {
        obj->pos.vy = 0;
    }
    if (obj->flags & 0x400) {
        obj->pos.vy = map_world.unk1200c;
    }
    if (!(obj->flags & 0x10)) {
        obj->pos.vy += obj->vertical_anchor_offset;
    }
}

ADDRESS(0x8004b624, 0x74)
void actor_copy_group_defaults(Actor *obj)
{
    ActorGroup *def;

    def = &actor_status.groups[obj->group_index];
    obj->definition_id = def->definition_id;
    obj->health = def->unk1a;
    obj->collision_radius = def->collision_radius;
    obj->collision_height = def->collision_height;
    obj->flags = def->initial_actor_flags;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = def->initial_model_scale_q12;
}

ADDRESS(0x8004b698, 0xd8)
void actor_initialize_from_group(Actor *obj)
{
    actor_copy_group_defaults(obj);
    obj->lifecycle = 1;
    obj->animation_id = 0;
    obj->animation_phase = 0;
    obj->unk11 = 0;
    obj->vertical_motion_state = 0;
    obj->target_type = 0;
    obj->target_action_state = 0xff;
    obj->target = NULL;
    if (!(obj->placement_flags & 1)) {
        obj->angle.vy = rand() >> 3;
    }
    obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
    obj->turn_rate = 0;
    if (obj->definition_id < 50) {
        obj->lighting_override_index = 0x3e;
    } else {
        obj->lighting_override_index = 0x37;
    }
    obj->lighting_blend = 0x800;
    if (obj->flags & 0x80) {
        obj->render_mode |= 7;
    } else {
        obj->render_mode = 0;
    }
    map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->collision_radius, 1);
}

ADDRESS(0x8004b770, 0x24)
void actor_set_lifecycle_and_home_position(Actor *obj)
{
    obj->lifecycle = 3;
    actor_set_home_position(obj);
}

ADDRESS(0x8004b794, 0xd4)
void actor_disable_type3_transition_actors(void)
{
    Actor *obj;
    s32 i;

    obj = actor_status.objects;
    for (i = 199; i != -1; i--) {
        if (obj->slot_state == 1 && obj->lifecycle == 1 && obj->target_type == 3 && obj->target_action_state == 0xf0) {
            if (obj->target_phase != 0 || obj->animation_phase > 0x800) {
                ((void (*)(Actor *))asset_status.handlers[19])(obj);
                actor_set_lifecycle_and_home_position(obj);
            }
        }
        obj++;
    }
}

ADDRESS(0x8004b868, 0x5c)
void actor_prepare_and_initialize(Actor *obj)
{
    obj->angle.vx = obj->angle.vz = 0;
    obj->layer_mask = obj->home_layer_mask;
    obj->angle.vy = obj->unk20;
    actor_set_home_position(obj);
    actor_initialize_from_group(obj);
    if (obj->slot_state == 3) {
        obj->layer_mask = 0;
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
    Actor *obj;
    u16 i;

    actor_status.actor_update_frame_count = 0;
    obj = actor_status.objects;
    i = 0;
    while (i < 200) {
        i++;
        obj->slot_state = ACTOR_FREE;
        obj->lifecycle = 0;
        obj->animation_cache = 0;
        obj++;
    }
    actor_status.actor_overlap_exclusion_flags = 0;
}

/* Starts a script record (0 stops the script). */
ADDRESS(0x8004b94c, 0x38)
void actor_set_target(Actor *obj, u8 *script)
{
    obj->previous_target_type = obj->target_type;
    if (script != NULL) {
        obj->target = script;
        obj->target_type = script[0];
        obj->target_action_state = 0;
    } else {
        obj->target = NULL;
        obj->target_type = 0xff;
        obj->target_action_state = 0xff;
    }
}
