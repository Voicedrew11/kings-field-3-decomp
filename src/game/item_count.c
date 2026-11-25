/*
 * GAME.EXE: item counts (WIP module): the two counts per id in
 * event_item_counters_table, a timed screen tint fade and a brightening loop, and the
 * item pick-up shown in front of the viewer.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/image.h>
#include <game/data.h>
#include <game/render.h>
#include <game/player.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/entity.h>
#include <game/cd.h>
#include <game/menu.h>
#include <game/memory.h>
#include <psyq/libc.h>
#include <stdarg.h>

/* Non-zero when either count of id is. */
ADDRESS(0x8005d7bc, 0x3c)
s32 game_counter_is_nonzero(s32 id)
{
    if (event_item_counters_table[0][id] == 0) {
        return event_item_counters_table[1][id] != 0;
    }
    return 1;
}

/* Takes one of id (first count first); 1 when there is none. */
ADDRESS(0x8005d7f8, 0xa0)
s32 game_counter_decrement(s32 id)
{
    if (event_item_counters_table[0][id] != 0) {
        event_item_counters_table[0][id]--;
    } else if (event_item_counters_table[1][id] != 0) {
        event_item_counters_table[1][id]--;
    } else {
        return 1;
    }
    asset_status.handlers[11]();
    return 0;
}

/* Adds one of id (each count holds 99); 1 and message 11 when both are full. */
ADDRESS(0x8005d898, 0xb0)
s32 game_counter_increment(s32 id)
{
    if (event_item_counters_table[0][id] < 99) {
        event_item_counters_table[0][id]++;
    } else if (event_item_counters_table[1][id] < 99) {
        event_item_counters_table[1][id]++;
    } else {
        notify_enqueue(11);
        return 1;
    }
    asset_status.handlers[6]();
    return 0;
}

/* Fades the screen tint from (r0, g0, b0) to (r1, g1, b1), step per frame out of 4096. */
ADDRESS(0x8005d948, 0x14c)
void color_overlay_transition(s32 step, s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1)
{
    s32 t;
    s32 r;
    s32 g;

    t = 0;
    do {
        light_reset_records();
        render_reset_color_overlays();
        light_refresh_record_rotations();
        r = fixed_lerp_q12(r0, r1, t);
        g = fixed_lerp_q12(g0, g1, t);
        accumulate_color_overlay(r, g, fixed_lerp_q12(b0, b1, t), 0x800, 1);
        t += step;
        player_update_status_visuals();
        render_game_frame(NULL, NULL);
    } while (t < 4096);
    light_reset_records();
    render_reset_color_overlays();
    light_refresh_record_rotations();
    accumulate_color_overlay(r1, g1, b1, 0x800, 1);
    player_update_status_visuals();
    render_game_frame(NULL, NULL);
}

/* Brightens the screen over 33 frames, then sets main_end_flag to 1. */
ADDRESS(0x8005da94, 0x9c)
void func_8005da94(void)
{
    VECTOR pos;
    SVECTOR angle;
    s32 i;
    s32 v;

    i = 0;
    for (;;) {
        v = i * i >> 16;
        if (v >= 256) {
            v = 255;
        }
        render_set_color_overlay(0x82, v, v, v);
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
        if (i >= 4096) {
            break;
        }
        i += 128;
    }
    main_end_flag = 1;
}

/*
 * Picks up obj: lifts it in front of the viewer (levelling the view pitch)
 * and spins it until a button press; the pick-up button (action_button_masks[7]) or
 * a forced id adds it to the item counts, else it turns back and returns to
 * its place. obj NULL spawns a record for the def id given after it.
 * pad doubles as the taken flag after the press. This module passes
 * angle_lerp_shortest_q12 both angles narrowed to s16 (an s16 prototype in its view;
 * the callee narrows them again).
 */
ADDRESS(0x8005db30, 0x7a0)
void event_map_object_interact(MapObject *obj, ...)
{
    va_list args;
    SVECTOR angle_to;
    SVECTOR angle_from;
    VECTOR to;
    VECTOR from;
    u8 *model;
    MapObjectDef *def;
    MapObject *link;
    MapObject *driver;
    s32 id;
    s32 pitch;
    s32 start;
    s16 spin;
    s16 held;
    s32 model_id;
    s32 t;
    u32 pad;
    u32 prev;
    s32 n;

    id = -1;
    if (obj == NULL) {
        va_start(args, obj);
        id = va_arg(args, s32);
        va_end(args);
        obj = map_object_effect_pool_acquire(350, 10, map_objects_slots.scatter_pool_sequence);
        map_object_reset(obj);
        obj->object_id = id;
        obj->layer_mask = 2;
        obj->action = 0xff;
        obj->angle.vz = 0;
        obj->angle.vy = 0;
        obj->angle.vx = 0;
        obj->arg.b.unk38 = 0xff;
        obj->pos = cur_avatar_status.pos;
        obj->pos.vx += rcos(scene_display_sys.view_rotation.vy - 3072) * 1024 >> 12;
        obj->pos.vz += rsin(scene_display_sys.view_rotation.vy - 3072) * 1024 >> 12;
        obj->pos.vy -= 1024;
    }
    def = &map_objects_slots.defs[obj->object_id];
    spin = obj->render_depth_offset;
    if (obj->arg.b.unk38 != 0xff) {
        return;
    }
    if (def->kind == 0x20) {
        notify_enqueue(25, obj->arg.h.unk3a);
        obj->object_id = 0xff;
        cur_avatar_status.gold += obj->arg.h.unk3a;
        return;
    }
    if (obj->action == 0x70 && obj->object_id == 0x86) {
        driver = &map_objects_slots.objects[obj->arg.h.unk40];
        link = &map_objects_slots.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->action == 2 && link->action_state != 0) {
            return;
        }
    } else if (obj->action == 0x70 && obj->object_id == 0x8d) {
        driver = &map_objects_slots.objects[obj->arg.h.unk40];
        link = &map_objects_slots.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->action == 2 && link->action_state != 0) {
            return;
        }
    } else if (obj->action == 0x70 && obj->object_id == 0x88) {
        driver = &map_objects_slots.objects[obj->arg.h.unk40];
        link = &map_objects_slots.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->action == 2 && link->action_state != 0) {
            return;
        }
    } else if (obj->object_id == 0x85 && asset_status.active_resource_ids[0] == 7) {
        if (!angle_within_tolerance(cur_avatar_status.camera_rotation_target.vy + 2048, obj->angle.vy, 256)) {
            return;
        }
    }
    pitch = cur_avatar_status.camera_rotation.vx & 0xfff;
    start = pitch;
    if (pitch < 2048) {
        if (pitch > 0) {
            pitch = 0;
        }
    } else if (pitch < 4095) {
        pitch = 4095;
    }
    prev = PadRead(1);
    held = def->arg.sh.held_depth_offset;
    angle_from = obj->angle;
    from = obj->pos;
    scene_position_from_camera_offset(0, 500, 1500, pitch, cur_avatar_status.camera_rotation.vy,
                                      def->arg.sh.height_offset, def->arg.sh.depth_offset, &to);
    angle_to.vz = 0;
    angle_to.vx = 0;
    angle_to.vy = (cur_avatar_status.camera_rotation.vy + 2048) & 0xfff;
    model_id = map_object_asset_index(obj->object_id);
    resource_registry_get(model_id, &model);
    while (model == NULL) {
        cd_request_service_vab();
        cd_request_service_stream();
        render_game_frame(NULL, &cur_avatar_status.camera_rotation);
        resource_registry_get(model_id, &model);
    }
    for (t = 0; t <= 4096; t += 512) {
        pad = PadRead(1);
        if (prev == 0 && pad != 0) {
            goto pressed;
        }
        prev = pad;
        scene_pose_interpolate(obj, &from, &to, &angle_from, &angle_to, t);
        obj->render_depth_offset = value_approach(spin, held, t >> 4);
        cur_avatar_status.camera_rotation.vx = angle_lerp_shortest_q12((s16)start, (s16)pitch, t);
        cd_request_service_vab();
        cd_request_service_stream();
        render_game_frame(NULL, &cur_avatar_status.camera_rotation);
    }
    obj->render_depth_offset = held;
    for (;;) {
        pad = PadRead(1);
        if (prev == 0 && pad != 0) {
            break;
        }
        prev = pad;
        obj->angle.vy += 64;
        cd_request_service_vab();
        cd_request_service_stream();
        render_game_frame(NULL, &cur_avatar_status.camera_rotation);
    }
pressed:
    if ((pad & action_button_masks[7]) || id != -1) {
        if (game_counter_increment(obj->object_id) == 0) {
            switch (obj->object_id) {
            case 0x93:
                n = (rand() * 6 >> 15) + 4;
                if (event_item_counters_table[0][0x93] < 99 - n) {
                    event_item_counters_table[0][0x93] += n;
                } else {
                    event_item_counters_table[0][0x93] = 99;
                }
                break;
            case 0x94:
                event_item_counters_table[0][0x94] = event_item_counters_table[0][0x94] < 95 ? event_item_counters_table[0][0x94] + 4 : 99;
                break;
            }
            pad = 1;
            scene_position_from_camera_offset(-500, 500, 0, pitch, cur_avatar_status.camera_rotation.vy, 0, 0,
                                              &from);
            angle_from = obj->angle;
            spin = 0;
            goto back;
        }
        if (id != -1) {
            obj->object_id = 0xff;
            player_clear_motion();
            return;
        }
    }
    pad = 0;
    for (;;) {
        if (angle_within_tolerance(obj->angle.vy, (cur_avatar_status.camera_rotation.vy + 2048) & 0xfff,
                                   128)) {
            obj->angle.vy = (cur_avatar_status.camera_rotation.vy + 2048) & 0xfff;
            break;
        }
        obj->angle.vy = (obj->angle.vy + 256) & 0xfff;
        render_game_frame(NULL, &cur_avatar_status.camera_rotation);
    }
back:
    angle_to = obj->angle;
    pitch = cur_avatar_status.camera_rotation.vx;
    to = obj->pos;
    for (t = 4096; t >= 0; t -= 512) {
        scene_pose_interpolate(obj, &from, &to, &angle_from, &angle_to, t);
        cur_avatar_status.camera_rotation.vx = angle_lerp_shortest_q12((s16)start, (s16)pitch, t);
        obj->render_depth_offset = value_approach(spin, held, t >> 4);
        render_game_frame(NULL, &cur_avatar_status.camera_rotation);
    }
    if (pad) {
        obj->object_id = 0xff;
    }
    player_clear_motion();
}

/*
 * The action button at pos facing angle: the Actor within reach in front
 * gets event_target_stream_execute, then each MapObject record
 * map_object_find_interaction_target finds in front acts by the type of its def
 * (doors and locks show why they stay shut or open, items are picked up, the
 * healing and save points fade the screen), and the loaded FDAT module's
 * handler 0 gets the point last. The full restore (type 0x12) clamps both
 * values like the heal (type 0x16) does.
 */
ADDRESS(0x8005e2d0, 0x794)
void event_world_dispatch_interaction(VECTOR *pos, SVECTOR *angle)
{
    VECTOR p;
    Actor *o88;
    MapObject *obj;
    MapObjectDef *def;
    MapObject *link;
    s32 i;

    p.vx = pos->vx;
    p.vy = pos->vy + 500;
    p.vz = pos->vz;
    cur_evt_state.interaction_handled = 0;
    i = actor_find_overlap_excluding_target_type3(p.vx, p.vy, p.vz, 1400, 3200);
    if (i != -1) {
        o88 = &actor_status.objects[i];
        if (angle_within_tolerance(angle->vy, vector_xz_to_angle(o88->pos.vx - p.vx, o88->pos.vz - p.vz),
                                   300)) {
            event_target_stream_execute(o88);
        }
    }
    for (i = 0; (i = map_object_find_interaction_target(i, &p, 800, 2500, angle->vy, 512)) != -1; i++) {
        obj = &map_objects_slots.objects[i];
        def = &map_objects_slots.defs[obj->object_id];
        cur_evt_state.interaction_handled = 1;
        switch (def->action) {
        case 0xa5:
        case 0xff:
            if (obj->object_id == 0xf8) {
                notify_enqueue(0x1d);
            } else if (obj->object_id == 0xf9) {
                notify_enqueue(0x1c);
            }
            break;
        case 0x40:
            event_map_object_interact(obj);
            if (obj->object_id == 0xff) {
                goto done;
            }
            break;
        case 0x53:
            if (obj->arg.b.unk3d[1] == 0) {
                break;
            }
            if (obj->action_state != 0) {
                break;
            }
            if (obj->arg.b.unk3d[0] == 0xff) {
                obj->action_state = 1;
            }
            break;
        case 0x57:
            if (obj->action_state == 0) {
                obj->action_state = 1;
            }
            break;
        case 3:
        case 4:
        case 5:
            if (obj->action_state != 0) {
                break;
            }
            if (obj->arg.b.unk38 == 0xff) {
                obj->action_state = 1;
            } else {
                notify_enqueue(6);
            }
            break;
        case 0:
        case 1:
        case 2:
        case 0x1b:
            if (obj->action_state != 0) {
                break;
            }
            switch (obj->arg.b.unk38) {
            case 0x98:
            case 0xf1:
                notify_enqueue(5);
                break;
            case 0x96:
            case 0xf0:
                notify_enqueue(0x17);
                break;
            case 0x9a:
            case 0xf2:
                notify_enqueue(0xf);
                break;
            case 0xfd:
            case 0xfe:
                if (obj->object_id != 0xae) {
                    obj->action_state = 1;
                    break;
                }
                notify_enqueue(6);
                break;
            case 0xff:
                obj->action_state = 1;
                break;
            default:
                notify_enqueue(6);
                break;
            }
            break;
        case 0x51:
            if (obj->action_state != 0) {
                break;
            }
            if (obj->arg.b.unk38 == 0xff) {
                obj->action_state = 1;
            }
            break;
        case 0x52:
            if (obj->action_state != 0) {
                break;
            }
            if (obj->arg.b.unk38 == 0xff) {
                obj->action_state = 1;
            }
            break;
        case 6:
        case 8:
            if (!angle_within_tolerance(angle->vy, obj->angle.vy + 2048, 0x155)) {
                break;
            }
            /* fall through */
        case 7:
        case 0x17:
            switch (obj->arg.b.unk38) {
            case 0xf1:
                notify_enqueue(5);
                break;
            case 0xf0:
                notify_enqueue(0x17);
                break;
            case 0xfe:
                if (obj->arg.h.unk3a == 0xffff) {
                missing:
                    notify_enqueue(0x1a);
                } else {
                    link = &map_objects_slots.objects[obj->arg.h.unk3a];
                    if (link->object_id == 0xff) {
                        goto missing;
                    }
                }
                break;
            case 0xff:
                obj->arg.b.unk38 = 0xfe;
                break;
            default:
                notify_enqueue(6);
                break;
            }
            break;
        case 0xd:
        case 0x14:
            if (obj->arg.b.unk3d[1] == 0) {
                break;
            }
            if (obj->arg.b.unk3d[0] == 0 && !angle_within_tolerance(angle->vy, obj->angle.vy + 2048, 0x155)) {
                break;
            }
            menu_show_transition_image(6, obj->arg.h.unk38 + 150);
            break;
        case 0x12:
            if (!angle_within_tolerance(angle->vy, vector_xz_to_angle(obj->pos.vx - p.vx, obj->pos.vz - p.vz),
                                        600)) {
                break;
            }
            if (!angle_within_tolerance(angle->vy + 2048, obj->angle.vy + 1024, 1024)) {
                break;
            }
            color_overlay_transition(512, 0, 0, 0, 128, 200, 255);
            cur_avatar_status.hp = cur_avatar_status.hp_max;
            if (cur_avatar_status.hp_max < cur_avatar_status.hp) {
                cur_avatar_status.hp = cur_avatar_status.hp_max;
            }
            cur_avatar_status.mp = cur_avatar_status.mp_max;
            if (cur_avatar_status.mp_max < cur_avatar_status.mp) {
                cur_avatar_status.mp = cur_avatar_status.mp_max;
            }
            player_cap_status_components(7);
            color_overlay_transition(512, 128, 200, 255, 0, 0, 0);
            player_clear_motion();
            break;
        case 0x16:
            if (!angle_within_tolerance(angle->vy, vector_xz_to_angle(obj->pos.vx - p.vx, obj->pos.vz - p.vz),
                                        600)) {
                break;
            }
            if (!angle_within_tolerance(angle->vy + 2048, obj->angle.vy + 1024, 1024)) {
                break;
            }
            color_overlay_transition(512, 0, 0, 0, 128, 128, 200);
            cur_avatar_status.hp = cur_avatar_status.hp_max;
            if (cur_avatar_status.hp_max < cur_avatar_status.hp) {
                cur_avatar_status.hp = cur_avatar_status.hp_max;
            }
            color_overlay_transition(512, 128, 128, 200, 0, 0, 0);
            player_clear_motion();
            break;
        case 0xe:
            if ((s16)asset_status.loading != 1) {
                cd_request_wait_idle();
                map_reveal_visited_rects();
                repeat_store_word((u32 *)map_visited_cells, 0, 200);
                event_world_state_save_slot(asset_status.active_resource_ids[0]);
                player_render_frame_and_release_pool();
                menu_card_save_browser();
            }
            break;
        }
    }
done:
    ((void (*)(VECTOR *, SVECTOR *))asset_status.handlers[0])(&p, angle);
}
