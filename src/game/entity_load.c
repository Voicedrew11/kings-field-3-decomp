/*
 * GAME.EXE: Actor per-frame update and area set-up (WIP module): the
 * loop over all records, the script pointers of the loaded ActorGroup
 * records, and the records made from an area's placement records.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/data.h>
#include <game/entity.h>

/* Updates every record: wake/sleep, script pick, its script, attachment. */
ADDRESS(0x80052e5c, 0x228)
void actor_update_frame(void)
{
    Actor *obj;
    Object44 *carrier;

    obj = actor_status.objects;
    actor_status.active_actor_count = 0;
    actor_status.current_index = 0;
    do {
        if (obj->slot_state != ACTOR_FREE) {
            actor_bind_current(obj);
            if ((actor_status.actor_update_frame_count & 3) == (actor_status.current_index & 3) || cur_avatar_status.unke != 0 ||
                cur_avatar_status.unk101 == 1) {
                actor_update_lifecycle_for_player_range();
            }
            if (obj->lifecycle == 1) {
                if ((actor_status.actor_update_frame_count & 3) == (actor_status.active_actor_count & 3)) {
                    actor_select_target_for_player_distance();
                }
                actor_update_behavior();
                actor_status.active_actor_count++;
            }
            if (actor_status.current_group->initial_actor_flags & 0x10000) {
                carrier = &map_objects_slots.objects[obj->unk22];
                obj->pos.vx = carrier->pos.vx;
                obj->pos.vy = carrier->pos.vy - 500;
                obj->pos.vz = carrier->pos.vz;
                if (obj->lifecycle == 1) {
                    obj->angle.vy = vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                       cur_avatar_status.pos.vz - obj->pos.vz);
                }
            }
            if (obj->definition_id == 0x6e) {
                if (cur_avatar_status.unkf5 != 0x5c && cur_avatar_status.unkf6 != 0x5c) {
                    obj->layer_mask = 0;
                } else if (cur_avatar_status.unk1c == 0) {
                    obj->layer_mask = 0;
                }
            }
        }
        obj++;
    } while (++actor_status.current_index < 200);
    actor_status.actor_update_frame_count++;
    actor_bind_current(NULL);
}

/* Turns the loaded ActorGroup records' script offsets into pointers (-1: none). */
ADDRESS(0x80053084, 0x74)
void actor_fixup_group_targets(void)
{
    ActorGroup *def;
    u8 **scripts;
    s32 i;
    s32 j;

    def = actor_status.groups;
    for (i = 0; i < 40; i++, def++) {
        if (def->definition_id == 0xff) {
            return;
        }
        scripts = def->targets;
        for (j = 0; j < 16; j++, scripts++) {
            if ((s32)*scripts == -1) {
                *scripts = NULL;
            } else {
                *scripts = actor_status.target_candidate_blob + (s32)*scripts;
            }
        }
    }
}

/* Makes the 200 records from an area's placement records. */
ADDRESS(0x800530f8, 0x1dc)
void actor_load_records(ActorPlacement *place)
{
    Actor *obj;
    ActorGroup *def;
    u16 i;

    obj = actor_status.objects;
    i = 200;
    while (i--) {
        obj->slot_state = place->slot_state;
        if (obj->slot_state != ACTOR_FREE) {
            obj->group_index = place->group_index;
            obj->unk4 = 0;
            obj->placement_flags = place->placement_flags;
            obj->home_layer_mask = place->home_layer_mask;
            obj->home_cell_z = place->cell_z;
            obj->home_cell_x = place->cell_x;
            obj->spawn_chance = place->spawn_chance;
            obj->death_drop_object_id = place->death_drop_object_id;
            obj->unk20 = -place->unk8 & 0xfff;
            obj->unk22 = place->unka;
            obj->unk24 = place->unkc;
            obj->vertical_anchor_offset = place->vertical_anchor_offset;
            obj->lifecycle = 0;
            obj->target_type = 0;
            obj->target_action_state = 0xff;
            obj->target = NULL;
            def = &actor_status.groups[obj->group_index];
            actor_copy_group_defaults(obj);
            actor_set_home_position(obj);
            obj->render_depth_offset = def->render_depth_offset;
            if (obj->flags & 0x10) {
                if (obj->slot_state == 3) {
                    if (obj->unk24 == -1) {
                        obj->unk24 = def->unk1a;
                    }
                    if (obj->vertical_anchor_offset == -1) {
                        obj->vertical_anchor_offset = def->default_vertical_anchor_offset;
                    }
                } else {
                    obj->slot_state = 4;
                    obj->vertical_anchor_offset = 0;
                }
            }
        } else {
            obj->lifecycle = 0;
        }
        place++;
        obj++;
    }
}
