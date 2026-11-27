/*
 * GAME.EXE: KF2's event_pose_interpolate.c from
 * scene_position_from_camera_offset to event_world_state_restore_slot: a point
 * relative to the player and MapObject position/angle interpolation, Actor
 * script record scanning, an Actor's talk script (event_target_stream_execute),
 * a MapObject spawn helper, the player's move to one of 14 start positions, the
 * item use dispatcher (event_scene_command_dispatch), the item counts, a timed
 * screen tint fade and the item pick-up shown in front of the viewer, clearing
 * the script flags and the event arena, the ending, and the event arena's 32
 * saved pointers with an area's saved object states. One TU: the .rdata is
 * player_warp_to_destination's local table initializer, then the jump tables of
 * event_target_stream_execute, event_scene_command_dispatch,
 * event_world_state_save_slot and event_world_state_restore_slot (GCC 2.6 emits
 * a TU's initializers ahead of its jump tables, and the 4-byte pad at
 * 0x80013294 aligns event_world_state_save_slot's table inside the same
 * .rdata). KF2's file also holds the memory card payload, which is a -G0 TU
 * here (src/game/card_payload.c).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/data.h>
#include <game/misc.h>
#include <game/render.h>
#include <game/funcs.h>
#include <game/cd.h>
#include <game/actor.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/map_object.h>
#include <game/scene.h>
#include <game/image.h>
#include <game/sound.h>
#include <game/memory.h>
#include <psyq/libc.h>
#include <stdarg.h>

/* (x, y + dy, z + dz) turned by (-pitch, yaw, 0), relative to the player's eye. */
ADDRESS(0x8005be28, 0xb8)
void scene_position_from_camera_offset(s32 x, s32 y, s32 z, s32 pitch, s32 yaw, s32 dy, s32 dz, VECTOR *out)
{
    SVECTOR v;
    SVECTOR rotation;

    v.vx = x;
    rotation.vx = -pitch;
    rotation.vz = 0;
    v.vy = y + dy;
    v.vz = z + dz;
    rotation.vy = yaw;
    vector_rotate_yxz(&rotation, &v, out);
    out->vx += cur_avatar_status.pos.vx;
    out->vy += cur_avatar_status.pos.vy + dy - 1600;
    out->vz += cur_avatar_status.pos.vz;
}

/* Moves obj between two positions and two angles by t (either pair may be NULL). */
ADDRESS(0x8005bee0, 0xe4)
void scene_pose_interpolate(MapObject *obj, VECTOR *from, VECTOR *to, SVECTOR *afrom, SVECTOR *ato, s32 t)
{
    if (from != NULL) {
        obj->pos.vx = fixed_lerp_q12(from->vx, to->vx, t);
        obj->pos.vy = fixed_lerp_q12(from->vy, to->vy, t);
        obj->pos.vz = fixed_lerp_q12(from->vz, to->vz, t);
    }
    if (afrom != NULL) {
        obj->angle.vx = angle_lerp_shortest_q12(afrom->vx, ato->vx, t);
        obj->angle.vz = angle_lerp_shortest_q12(afrom->vz, ato->vz, t);
        obj->angle.vy = angle_lerp_shortest_q12(afrom->vy, ato->vy, t);
    }
}

ADDRESS(0x8005bfc4, 0x88)
void scene_position_interpolate(MapObject *obj, VECTOR *from, VECTOR *to, s32 t)
{
    if (from != NULL) {
        obj->pos.vx = fixed_lerp_q12(from->vx, to->vx, t);
        obj->pos.vy = fixed_lerp_q12(from->vy, to->vy, t);
        obj->pos.vz = fixed_lerp_q12(from->vz, to->vz, t);
    }
}

ADDRESS(0x8005c04c, 0x88)
void scene_angles_interpolate(MapObject *obj, SVECTOR *afrom, SVECTOR *ato, s32 t)
{
    if (afrom != NULL) {
        obj->angle.vx = angle_lerp_shortest_q12(afrom->vx, ato->vx, t);
        obj->angle.vz = angle_lerp_shortest_q12(afrom->vz, ato->vz, t);
        obj->angle.vy = angle_lerp_shortest_q12(afrom->vy, ato->vy, t);
    }
}

/* Sets obj's animation and time, then steps the time by step until it is within step / 2 of target. */
ADDRESS(0x8005c0d4, 0xb8)
void actor_animation_seek_phase(Actor *obj, u8 anim, u16 time, s32 target, u16 step)
{
    u32 s;
    s16 half;
    s16 low;

    if (step != 0) {
        s = step & 0xfffe;
        half = s >> 1;
        low = target - half;
        obj->animation_id = anim;
        obj->animation_phase = time;
        while (!angle_within_tolerance(obj->animation_phase, low, half)) {
            obj->animation_phase = (s + obj->animation_phase) & 0xfff;
            render_game_frame(NULL, NULL);
        }
        obj->animation_phase = low & 0xfff;
        render_game_frame(NULL, NULL);
    }
}

/* Offset after the 0xf2 marker for id, or script[16] at the 0xff end. */
ADDRESS(0x8005c18c, 0x5c)
u8 event_target_stream_find_marker(u8 *script, u8 id)
{
    u8 *p;

    p = script + 20;
    for (;;) {
        switch (*p++) {
        case 0xff:
            return script[16];
        case 0xf2:
            if (*p == id) {
                return p - 19 - script;
            }
            p++;
            break;
        }
    }
}

/* Entry point of obj's first script record: the first 0xf1 entry whose flag matches, else the saved one. */
ADDRESS(0x8005c1e8, 0x120)
u8 *event_target_stream_resolve_cursor(Actor *obj)
{
    u8 *script;
    u8 *p;
    u8 n;

    script = actor_status.groups[obj->group_index].targets[0];
    p = script + 20;
    for (;;) {
        switch (*p) {
        case 0xfe:
            p++;
            if (script[16] == 0) {
                script[16] = p - (script + 20);
                return p;
            }
        found:
            return script + (script[16] + 20);
        case 0xf1:
            if (cur_evt_state.flags[p[1]] == p[2]) {
                n = event_target_stream_find_marker(script, p[3]);
                if (script[16] < n) {
                    script[16] = n;
                }
                cur_evt_state.stream_actor_definition_id = obj->definition_id;
                script[19] = 0;
                goto found;
            }
            p += 4;
            break;
        }
    }
}

RODATA(0x800130f0, 0x590)

/*
 * Runs obj's talk script record (id 0x70) from its entry point up to the
 * 0xff end, then its end action.
 *
 * Not exact (one instruction out of place): in the first
 * actor_animation_seek_phase(obj, script[1], ...) call retail sets `li
 * a3,0xfff` before the argument loads and leaves the `lhu` next to the
 * `jal`; here sched2 moves `li a3` into that load-use gap. Retail's sched2
 * still had the copy of script[1] into a1 at that point, which needs the
 * byte in a pseudo that combine could not merge into its extension. Only a
 * volatile read of that one byte reproduces it, and the same byte is read
 * non-volatile three lines earlier, so the plain read stays. gcc 2.6.3
 * gives the same code as 2.6.0 here.
 */
ADDRESS(0x8005c308, 0x4e0)
void event_target_stream_execute(Actor *obj)
{
    ActorGroup *def;
    u8 *script;
    u8 *p;
    s32 repeat;
    s32 posed;
    u8 anim;
    s32 n;
    u8 back;

    def = &actor_status.groups[obj->group_index];
    script = def->targets[0];
    if (script == NULL || script[0] != 0x70) {
        return;
    }
    posed = 0;
    repeat = 0;
    if (script[16] == 0) {
        cur_evt_state.stream_actor_definition_id = obj->definition_id;
    }
    p = event_target_stream_resolve_cursor(obj);
    if (cur_evt_state.stream_actor_definition_id != obj->definition_id && script[19] == 1) {
        while (*p++ != 0xf0)
            ;
        p++;
        script[16] = p - (script + 20);
        script[19] = 0;
    }
    for (;;) {
        switch (*p) {
        case 0xf0:
            script[19] = 1;
        case 0xf8:
            back = p[1];
            script[16] -= back;
            p -= back;
            continue;
        case 0xf9:
            if (cur_evt_state.flags[p[1]] == p[2]) {
                script[16] = event_target_stream_find_marker(script, p[3]);
                p = script + (script[16] + 20);
            } else {
                script[16] += 4;
                p += 4;
            }
            continue;
        case 0xf2:
            script[16] += 2;
            p += 2;
            continue;
        case 0xf4:
            script[16]++;
            p++;
            asset_status.handlers->talk_event(obj, *p);
            break;
        case 0xf5:
            script[16]++;
            p++;
            repeat = *p;
            break;
        case 0xf6:
            cur_evt_state.stream_actor_definition_id = obj->definition_id;
            p++;
            script[19] = 0;
            script[16]++;
            continue;
        case 0xf7:
            cur_evt_state.flags[p[1]] = p[2];
            p += 2;
            script[16] += 2;
            break;
        case 0xf3:
            break;
        case 0xff:
            goto end;
        default:
            if (!posed && script[1] != 0xff) {
                anim = obj->animation_id;
                posed = 1;
                if (obj->animation_phase != 0) {
                    actor_animation_seek_phase(obj, obj->animation_id, obj->animation_phase, 0,
                                               obj->animation_step);
                }
                actor_animation_seek_phase(obj, script[1], 0, 0xfff, *(u16 *)(script + 8));
            }
            if (def->definition_id != 0x2b) {
                conversation_record_mark_page(def->definition_id, *p);
            }
            render_game_frame(NULL, NULL);
            menu_show_transition_image(7, *(u16 *)(script + 12) + *p);
            if ((cur_avatar_status.pad_buttons & action_button_masks[7]) && script[18] == 0xff) {
                repeat++;
            }
            break;
        }
        script[16]++;
        p++;
        if (repeat == 0) {
            break;
        }
        repeat--;
    }
end:
    switch (script[18] & 0xf0) {
    case 0x00:
        player_render_frame_and_release_pool();
        menu_item_buy_sell_controller(script[18] & 0xf);
        break;
    case 0x10:
    case 0x20:
        player_render_frame_and_release_pool();
        break;
    case 0x30:
        if (cur_avatar_status.wisdom == 4) {
            player_render_frame_and_release_pool();
            n = menu_intellectual_choose_item();
            if (n != -1) {
                render_game_frame(NULL, NULL);
                menu_show_transition_image(6, n + 488);
            }
        }
        break;
    case 0x40:
        player_render_frame_and_release_pool();
        if (menu_prompt_stay(script[18] & 0xf) != -1) {
            movie_state.state = 1;
            movie_state.movie_id = script[18] & 3;
        }
        break;
    }
    cur_evt_state.stream_actor_definition_id = obj->definition_id;
    if (posed && script[17] != 0xff) {
        actor_animation_seek_phase(obj, script[17], 0, 0xfff, *(u16 *)(script + 14));
        obj->animation_id = anim;
    }
    player_clear_motion();
    cur_evt_state.interaction_handled = 1;
}

/* Spawns MapObject 0x17c, links obj to it (its index - 124) and returns it. */
ADDRESS(0x8005c7e8, 0x7c)
MapObject *event_spawn_effect_object(MapObject *obj, s32 id)
{
    MapObject *made;

    made = map_object_effect_pool_acquire(0x17c, 0x10, -1);
    obj->arg.b.unk39 = made - map_objects_slots.objects - 124;
    made->object_id = id;
    made->arg.b.unk38 = 0;
    return made;
}

/* Loads start position n's areas and puts the player there (12: none). */
ADDRESS(0x8005c864, 0x37c)
void player_warp_to_destination(s32 n)
{
    WarpDestination table[14] = {
        {0, 0, 0, 10, 56, 100, 2048},
        {1, 4, 5, 32, 14, 100, 0},
        {2, 2, 2, 57, 36, 100, 0},
        {3, 16, 18, 7, 31, 100, 0},
        {4, 13, 15, 56, 13, 100, 2048},
        {5, 4, 5, 29, 31, 100, 0},
        {6, 17, 19, 34, 67, 100, 2048},
        {7, 7, 9, 19, 29, 104, 0},
        {255, 255, 255, 10, 56, 100, 0},
        {9, 5, 7, 39, 56, 100, 2048},
        {10, 6, 8, 37, 51, 204, 2048},
        {11, 6, 8, 58, 34, 100, 0},
        {27, 9, 11, 40, 35, 100, 0},
        {24, 9, 11, 7, 70, 200, 0},
    };
    WarpDestination *rec;
    s32 yaw;
    s32 z;

    rec = &table[n];
    if (n < 12 || n == 13) {
        if (rec->area == 0xff) {
            return;
        }
        render_frames_with_color_overlay(1, 0, 0x1000, 0x100);
        actor_disable_type3_transition_actors();
    }
    do {
        cd_request_yield();
        resource_advance_transition();
    } while ((s16)asset_status.loading != 0);
    if (rec->area != asset_status.active_resource_ids[0]) {
        resource_request_transition(rec->area, rec->area, rec->area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
    } else {
        resource_request_transition(0xff, 0xff, rec->area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
        cur_avatar_status.force_actor_lifecycle_refresh = 1;
    }
    do {
        cd_request_yield();
        resource_advance_transition();
    } while ((s16)asset_status.loading != 0);
    cur_avatar_status.reaction_mode = 0;
    cur_avatar_status.view_rotation_offset_x = cur_avatar_status.view_rotation_offset_y =
        cur_avatar_status.view_rotation_offset_z = 0;
    cur_avatar_status.camera_rotation = cur_avatar_status.camera_rotation_target;
    cur_avatar_status.pos.vx = (rec->x << 11) + 1024;
    z = rec->z << 11;
    cur_avatar_status.pos.vz = z + 1024;
    cur_avatar_status.pos.vy = -(rec->y << 7);
    yaw = -rec->yaw;
    cur_avatar_status.camera_rotation.vy = cur_avatar_status.camera_rotation_target.vy = yaw;
    if (n == 13) {
        cur_avatar_status.camera_rotation.vx = cur_avatar_status.camera_rotation_target.vx =
            cur_avatar_status.camera_rotation.vz =
            cur_avatar_status.camera_rotation_target.vz = 0;
        cur_avatar_status.pos.vz = z + 2000;
    } else {
        render_frames_with_color_overlay(1, 0x1000, 0x1000, 0);
        cd_request_wait_idle();
        render_frames_with_color_overlay(1, 0x1000, 0, -256);
        render_set_color_overlay(0xff, 0, 0, 0);
    }
    if (n != 13) {
        resource_request_transition(0xff, 0xff, 0xff, rec->vab_id, rec->sequence_id, 0x7f, 0x7f, 0x7f);
    }
    if (n < 12) {
        cur_avatar_status.warp_arrived = 1;
        cur_evt_state.interaction_handled = 1;
        player_recalculate_combat_stats();
    }
    cur_avatar_status.reaction_rotation_offset_x = cur_avatar_status.reaction_rotation_offset_y =
        cur_avatar_status.reaction_rotation_offset_z = 0;
}

/* Uses item id at pos/angle (stat raises, objects near the player, start-position moves),
 * then passes it to the loaded FDAT module's handler 2; message 10 when nothing took it. */
ADDRESS(0x8005cbe0, 0xbdc)
void event_scene_command_dispatch(VECTOR *pos, SVECTOR *angle, s32 id)
{
    VECTOR unused[2]; /* the frame keeps 32 unused bytes here */
    Actor *o88;
    MapObject *obj;
    MapObject *obj2;
    Actor *o88b;
    MapObject *obj2b;
    Actor *target;
    MapObject *made;
    s32 dist;
    s32 i;
    s32 element;
    s32 flag;
    s32 n;
    s32 k;
    s32 j;

    cur_evt_state.interaction_handled = 0;
    switch (id) {
    case 113:
        element = 0;
        goto raise;
    case 114:
        element = 1;
        goto raise;
    case 115:
        element = 2;
        goto raise;
    case 116:
        element = 3;
        goto raise;
    case 117:
        element = 4;
    raise:
        game_counter_decrement(id);
        for (j = 2; j != -1; j--) {
            cur_avatar_status.base_magic[element]++;
            if (cur_avatar_status.base_magic[element] >= 1000) {
                cur_avatar_status.base_magic[element] = 999;
            } else {
                player_learn_element_magic(element);
                player_recalculate_combat_stats();
                if (cur_evt_state.interaction_handled == 0) {
                    cur_evt_state.interaction_handled = 1;
                    notify_enqueue(element + 18);
                }
            }
        }
        break;
    case 110:
        game_counter_decrement(id);
        for (i = 4; i != -1; i--) {
            cur_avatar_status.base_magic[i]++;
            if (cur_avatar_status.base_magic[i] >= 1000) {
                cur_avatar_status.base_magic[i] = 999;
            } else {
                player_learn_element_magic(i);
                player_recalculate_combat_stats();
                if (cur_evt_state.interaction_handled == 0) {
                    cur_evt_state.interaction_handled = 1;
                    notify_enqueue(13);
                }
            }
        }
        break;
    case 111:
        for (j = 2; j != -1; j--) {
            cur_avatar_status.base_physical_power++;
            if (cur_avatar_status.base_physical_power >= 1000) {
                cur_avatar_status.base_physical_power = 999;
            } else {
                player_recalculate_combat_stats();
                if (cur_evt_state.interaction_handled == 0) {
                    cur_evt_state.interaction_handled = 1;
                    notify_enqueue(1);
                }
            }
        }
        break;
    case 112:
        cur_avatar_status.wisdom++;
        if (cur_avatar_status.wisdom >= 5) {
            cur_avatar_status.wisdom = 4;
            break;
        }
        player_recalculate_combat_stats();
        if (cur_evt_state.interaction_handled == 0) {
            cur_evt_state.interaction_handled = 1;
            notify_enqueue(2);
        }
        break;
    case 128:
        if (asset_status.active_resource_ids[0] == 6) {
            k = 0;
            for (;;) {
                k = map_object_find_interaction_target(k, pos, 800, 1700, angle->vy, 512);
                if (k == -1) {
                    break;
                }
                if (k == 0x5b) {
                    cur_evt_state.interaction_handled = 1;
                    game_counter_decrement(0x80);
                    obj2 = &map_objects_slots.objects[0x5b];
                    i = 16;
                    o88 = &actor_status.objects[3];
                    o88->lifecycle = 0;
                    o88->slot_state = 2;
                    while (o88->lifecycle == 0) {
                        actor_update_frame();
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    cd_request_wait_idle();
                    o88->render_depth_offset = -1;
                    obj2->lighting_blend = 0x400;
                    obj2->render_mode = 5;
                    o88->lighting_blend = 0x1000;
                    o88->render_mode = 5;
                    while (--i != -1) {
                        if (o88->render_depth_offset > 100) {
                            o88->render_depth_offset -= 8;
                            obj2->scale.vx += 8;
                            obj2->scale.vz += 8;
                            obj2->lighting_blend += 256;
                            o88->lighting_blend -= 256;
                        } else {
                            o88->render_depth_offset = 100;
                        }
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    o88->lighting_blend = 0x800;
                    o88->render_mode = 0;
                    o88->slot_state = 1;
                    obj2->object_id = 0xff;
                    cur_evt_state.unke[0x19] = 1;
                    break;
                }
                if (k == 0x53) {
                    cur_evt_state.interaction_handled = 1;
                    game_counter_decrement(0x80);
                    o88b = &actor_status.objects[58];
                    o88b->lifecycle = 0;
                    o88b->slot_state = 2;
                    actor_select_target_type_in_own_group(o88b, 0x15);
                    obj2b = &map_objects_slots.objects[0x53];
                    i = 16;
                    while (o88b->lifecycle == 0) {
                        actor_update_frame();
                        o88b->animation_phase = 0;
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    cd_request_wait_idle();
                    o88b->render_depth_offset = -1;
                    obj2b->lighting_blend = 0x400;
                    obj2b->render_mode = 5;
                    o88b->lighting_blend = 0x1000;
                    o88b->render_mode = 5;
                    while (--i != -1) {
                        if (o88b->render_depth_offset > 0) {
                            o88b->render_depth_offset -= 8;
                            obj2b->scale.vx += 8;
                            obj2b->scale.vz += 8;
                            obj2b->lighting_blend += 256;
                            o88b->lighting_blend -= 256;
                        } else {
                            o88b->render_depth_offset = 0;
                        }
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    o88b->lighting_blend = 0x800;
                    o88b->render_mode = 0;
                    o88b->slot_state = 1;
                    obj2b->object_id = 0xff;
                    cur_evt_state.unk2c[0] = 1;
                    break;
                }
                k++;
            }
        }
        if (cur_evt_state.interaction_handled == 0) {
            game_counter_decrement(id);
            cur_avatar_status.magic_boost_timer = 900;
            cur_evt_state.interaction_handled = 1;
            player_recalculate_combat_stats();
        }
        break;
    case 135:
    case 137:
    case 138:
    case 139:
    case 140:
    case 142:
    case 144:
        i = 0;
        for (;;) {
            i = map_object_find_interaction_target(i, pos, 800, 1700, angle->vy, 512);
            if (i == -1) {
                goto done;
            }
            switch (map_object_check_and_consume_marker(&map_objects_slots.objects[i], id)) {
            case 0:
                break;
            case 1:
                audio_play_sound_16();
                cur_evt_state.interaction_handled = 1;
                goto done;
            case 3:
                notify_enqueue(7);
                cur_evt_state.interaction_handled = 1;
                goto done;
            case 4:
                notify_enqueue(7);
                cur_evt_state.interaction_handled = 1;
                goto done;
            }
            i++;
        }
    case 141:
        i = map_object_find_interaction_target(0, pos, 800, 1700, angle->vy, 512);
        if (i == -1) {
            break;
        }
        made = &map_objects_slots.objects[i];
        if (made->object_id == 0x101) {
            if (made->arg.b.unk38 == 0xff) {
                made->arg.b.unk38 = id;
                made->action_state = 0;
                cur_evt_state.interaction_handled = 1;
                game_counter_decrement(id);
                event_spawn_effect_object(made, id);
            }
        } else if (map_object_check_and_consume_marker(made, id) == 3) {
            notify_enqueue(7);
            cur_evt_state.interaction_handled = 1;
        }
        break;
    case 136:
        i = 0;
        for (;;) {
            i = map_object_find_interaction_target(i, pos, 800, 1700, angle->vy, 512);
            if (i == -1) {
                break;
            }
            obj = &map_objects_slots.objects[i];
            if (obj->object_id == 0xa2) {
                if (obj->arg.b.unk38 == 0x88) {
                    obj->arg.b.unk38 = 0xfe;
                    cur_evt_state.interaction_handled = 1;
                    game_counter_decrement(id);
                }
            } else if (obj->object_id == 0x102) {
                if (obj->arg.b.unk38 == 0xff) {
                    MapObject *made;

                    obj->arg.b.unk38 = id;
                    obj->action_state = 0;
                    cur_evt_state.interaction_handled = 1;
                    game_counter_decrement(id);
                    made = map_object_effect_pool_acquire(350, 10, map_objects_slots.scatter_pool_sequence);
                    map_object_reset(made);
                    made->arg.h.unk3c = map_objects_slots.scatter_pool_sequence++;
                    made->object_id = id;
                    made->arg.b.unk38 = 0;
                    obj->arg.b.unk39 = made - map_objects_slots.objects - 94;
                }
            } else if (map_object_check_and_consume_marker(obj, id) == 3) {
                notify_enqueue(7);
                cur_evt_state.interaction_handled = 1;
            }
            i++;
        }
        break;
    case 118:
        i = map_object_find_interaction_target(0, pos, 800, 1700, angle->vy, 512);
        if (i == -1) {
            break;
        }
        if (map_objects_slots.objects[i].object_id == 0xfe) {
            if (game_counter_increment(0x77) == 0) {
                notify_enqueue(12);
                game_counter_decrement(id);
            }
            cur_evt_state.interaction_handled = 1;
        } else if (map_objects_slots.objects[i].object_id == 0xfb) {
            if (game_counter_increment(0x78) == 0) {
                notify_enqueue(12);
                game_counter_decrement(id);
            }
            cur_evt_state.interaction_handled = 1;
        }
        break;
    case 130:
        k = 1;
        goto start;
    case 131:
        k = 6;
        goto start;
    case 132:
        k = 9;
        goto start;
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
        k = id - 150;
        if ((u32)k >= 12) {
            break;
        }
    start:
        if (asset_status.active_resource_ids[0] == 25 || asset_status.active_resource_ids[0] == 27) {
            k = 0;
            break;
        }
        if ((s16)asset_status.loading == 1
            && (asset_status.requested_resource_ids[0] == 25
                || asset_status.requested_resource_ids[0] == 27)) {
            break;
        }
        player_warp_to_destination(k);
        break;
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 119:
    case 120:
        cur_evt_state.interaction_handled = 1;
        break;
    case 101:
        flag = 2;
        if (cur_avatar_status.map_layer_offset == 0) {
            flag = 1;
        }
        target = actor_find_best_in_cone(pos, angle->vy, angle->vx, 8000, 500, 500, &dist, -1);
        if (target != NULL && target->layer_mask == flag) {
            menu_show_transition_image(6, target->definition_id + 270);
            cur_evt_state.interaction_handled = 1;
        } else {
            n = map_object_find_interaction_target(0, pos, 800, 1700, angle->vy, 512);
            if (n != -1 && map_objects_slots.objects[n].layer_mask != 0 && n == 11
                && asset_status.active_resource_ids[0] == 17) {
                menu_show_transition_image(6, 305);
                cur_evt_state.interaction_handled = 1;
            }
        }
        if (cur_evt_state.interaction_handled == 0) {
            obj = map_objects_slots.objects;
            for (i = 395; i != -1; i--, obj++) {
                if (map_objects_slots.defs[obj->object_id].action == 0xe2 && obj->arg.b.unk40 == flag
                    && player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                                       obj->arg.b.unk39,
                                                       0x8000) != 0) {
                    menu_show_transition_image(6, obj->arg.h.unk3a + 810);
                    cur_evt_state.interaction_handled = 1;
                    break;
                }
            }
        }
        break;
    }
done:
    asset_status.handlers->use_item(pos, angle, id);
    if (cur_evt_state.interaction_handled == 0) {
        notify_enqueue(10);
    }
    player_clear_motion();
}

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
    asset_status.handlers->on_item_removed();
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
    asset_status.handlers->on_item_added();
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
                event_item_counters_table[0][0x94] = event_item_counters_table[0][0x94] < 95
                    ? event_item_counters_table[0][0x94] + 4 : 99;
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
    asset_status.handlers->interact(&p, angle);
}

ADDRESS(0x8005ea64, 0xbc)
void event_state_initialize(void)
{
    u16 *off;
    s32 i;

    repeat_store_word((u32 *)cur_evt_state.flags, 0, 0x40);
    repeat_store_word((u32 *)cur_evt_state.arena, 0, 0x1400);
    memory_arena_initialize_blocks((void *)cur_evt_state.arena, 0x5000);
    off = cur_evt_state.saved_offsets;
    for (i = 31; i != -1; i--) {
        *off++ = 0xffff;
    }
    repeat_store_word((u32 *)event_item_counters_table, 0, 0x4b);
    event_item_counters_table[0][0] = 1;
    event_item_counters_table[0][42] = 1;
    event_item_counters_table[0][104] = 2;
    event_item_counters_table[0][105] = 1;
}

/*
 * The ending once FDAT slot 1 has run: with ending_state 1 and ending_variant set, fades the
 * music out, moves the player to start position 13 and plays movie 7, then
 * loads areas 9 and 11; with ending_variant clear, fades out, stops the sequence,
 * plays movie 8 and leaves the game loop (main_end_flag = 4).
 */
ADDRESS(0x8005eb20, 0x338)
void game_update_ending(void)
{
    s32 n;
    s32 volume;

    asset_status.handlers->update();
    if (cur_evt_state.ending_state != 1) {
        return;
    }
    cur_evt_state.ending_state = 2;
    if (cur_evt_state.ending_variant != 0) {
        player_warp_to_destination(13);
        n = cur_avatar_status.bgm_volume >> 1;
        volume = cur_avatar_status.bgm_volume;
        while (--n != -1) {
            volume -= 2;
            if (volume < 0) {
                SsSeqSetVol(sound_status.seq_id, 0, 0);
            } else {
                SsSeqSetVol(sound_status.seq_id, volume, volume);
            }
            DrawSync(0);
            VSync(0);
        }
        cd_request_wait_idle();
        do {
            cd_request_yield();
            resource_advance_transition();
        } while ((s16)asset_status.loading == 1);
        movie_state.movie_id = 7;
        movie_state.current_movie = &movie_state.movies[7];
        movie_play();
        movie_state.stream_status = -1;
        movie_state.state = 0;
        movie_state.movie_id = 0xff;
        player_reset_reaction_state();
        SsSetMVol(127, 127);
        resource_request_transition(0xff, 0xff, 0xff, 9, 11, 0x7f, 0x7f, 0x7f);
        MoveImage(&scene_display_sys.disp[scene_display_sys.frame].disp,
                  scene_display_sys.draw[scene_display_sys.frame].clip.x,
                  scene_display_sys.draw[scene_display_sys.frame].clip.y);
        DrawSync(0);
        render_frames_with_color_overlay(1, 0, 0, 0);
        cd_request_wait_idle();
    } else {
        n = cur_avatar_status.bgm_volume >> 1;
        volume = cur_avatar_status.bgm_volume;
        while (--n != -1) {
            volume -= 2;
            if (volume < 0) {
                SsSeqSetVol(sound_status.seq_id, 0, 0);
            } else {
                SsSeqSetVol(sound_status.seq_id, volume, volume);
            }
            DrawSync(0);
            VSync(0);
        }
        SsSetMVol(0, 0);
        if (sound_status.seq_playing == 1) {
            SsSeqSetVol(sound_status.seq_id, 0, 0);
            SsSeqStop(sound_status.seq_id);
            SsSeqClose(sound_status.seq_id);
            sound_status.seq_playing = 0;
        }
        render_frames_with_color_overlay(1, 0, 0x1000, 0x100);
        movie_state.movie_id = 8;
        movie_state.current_movie = &movie_state.movies[8];
        movie_play();
        movie_state.stream_status = -1;
        movie_state.state = 0;
        movie_state.movie_id = 0xff;
        main_end_flag = 4;
    }
}

/* out[i] = the arena address of saved offset i, NULL for 0xffff. */
ADDRESS(0x8005ee58, 0x50)
void event_saved_offsets_decode(u8 **out)
{
    u16 *off;
    u16 o;
    s32 i;

    off = cur_evt_state.saved_offsets;
    for (i = 31; i != -1; i--) {
        o = *off++;
        if (o == 0xffff) {
            *out = NULL;
        } else {
            *out = cur_evt_state.arena + o;
        }
        out++;
    }
}

/* Adds delta to the owner pointer of every used block. */
ADDRESS(0x8005eea8, 0x70)
void event_arena_owner_pointers_add_delta(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)cur_evt_state.arena;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner + delta);
        }
        block = HEAP_NEXT(block);
    }
}

/* Saves in[i] as an arena offset, 0xffff for NULL. */
ADDRESS(0x8005ef18, 0x4c)
void event_saved_offsets_encode(u8 **in)
{
    u16 *off;
    u8 *p;
    s32 i;

    off = cur_evt_state.saved_offsets;
    for (i = 31; i != -1; i--) {
        p = *in++;
        if (p == NULL) {
            *off = 0xffff;
        } else {
            *off = p - cur_evt_state.arena;
        }
        off++;
    }
}

/* Subtracts delta from the owner pointer of every used block. */
ADDRESS(0x8005ef64, 0x70)
void event_arena_owner_pointers_subtract_delta(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)cur_evt_state.arena;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner - delta);
        }
        block = HEAP_NEXT(block);
    }
}

/*
 * Saves area's state in its saved block: the live Actor records, the talk
 * script positions and each MapObject record by its MapObjectDef type.
 */
ADDRESS(0x8005efd4, 0x470)
void event_world_state_save_slot(s32 area)
{
    u8 *ptrs[32];
    u8 buf[0xc00];
    Actor *obj;
    ActorGroup *def;
    MapObject *rec;
    u8 *script;
    u8 *p;
    u8 *block;
    s32 size;
    s32 i;

    p = buf;
    obj = actor_status.objects;
    for (i = 0; i < 200; i++) {
        if (obj->slot_state != 0xff && obj->slot_state == 1) {
            *p++ = i;
            if (obj->lifecycle == 3) {
                *p = 3;
            } else {
                *p = 0;
            }
            p++;
        }
        obj++;
    }
    *p++ = 0xff;
    def = actor_status.groups;
    for (i = 0; i < 40; i++) {
        if (def->definition_id == 0xff) {
            break;
        }
        script = def->targets[0];
        def++;
        if (script != NULL && script[0] == 0x70) {
            *p++ = i;
            *p++ = script[16];
            *p++ = script[19];
        }
    }
    *p++ = 0xff;
    rec = map_objects_slots.objects;
    for (i = 0; i < 396; i++) {
        if (rec->object_id == 0xff) {
            *p++ = 0xff;
        } else {
            switch (map_objects_slots.defs[rec->object_id].action) {
            case 0x40:
                switch (rec->action) {
                case 0x60:
                    *p++ = 0xf0;
                    *p++ = rec->object_id;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    *p++ = rec->angle.vy >> 4;
                    break;
                case 0x61:
                    *p++ = 0xf1;
                    *p++ = rec->object_id;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    break;
                case 0x62:
                    *p++ = 0xf2;
                    *p++ = rec->object_id;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    *p++ = rec->arg.h.unk3a >> 2;
                    break;
                case 0x70:
                    *p++ = 0xf3;
                    *p++ = rec->object_id;
                    *p++ = rec->arg.b.unk38;
                    break;
                default:
                    *p++ = 0xfd;
                    *p++ = rec->arg.b.unk38;
                    break;
                }
                break;
            case 0x53:
                if (rec->arg.b.unk38 < 2) {
                    break;
                }
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x17:
            case 0x1b:
            case 0x51:
            case 0x52:
            case 0x54:
            case 0x55:
            case 0x57:
            case 0x58:
            case 0x5a:
            case 0x5f:
            case 0xa0:
            case 0xa1:
            case 0xa3:
            case 0xa4:
            case 0xa6:
            case 0xe8:
                *p++ = 0xfd;
                *p++ = rec->arg.b.unk38;
                break;
            case 0x0f:
            case 0x11:
            case 0x18:
            case 0x1a:
            case 0xa7:
                *p++ = 0xf4;
                *p++ = rec->arg.b.unk38;
                *p++ = rec->arg.b.unk39;
                break;
            case 0x56:
                *p++ = 0xf5;
                *p++ = rec->arg.b.unk39;
                break;
            default:
                *p++ = 0xfe;
                break;
            }
        }
        rec++;
    }
    event_saved_offsets_decode(ptrs);
    event_arena_owner_pointers_add_delta((s32)ptrs);
    block = ptrs[area];
    if (block != NULL) {
        memory_block_release(block);
    }
    size = (p - buf + 3) & ~3;
    block = memory_arena_allocate_block((HeapBlock *)cur_evt_state.arena, size, &ptrs[area]);
    if (block != NULL) {
        resource_copy_words((u32 *)block, (u32 *)buf, size >> 2);
        event_saved_offsets_encode(ptrs);
        event_arena_owner_pointers_subtract_delta((s32)ptrs);
    }
}

/* Restores area's state from its saved block (event_world_state_save_slot's format). */
ADDRESS(0x8005f444, 0x378)
void event_world_state_restore_slot(s32 area)
{
    u8 *ptrs[32];
    MapObject *rec;
    u8 *script;
    u8 *p;
    s32 i;
    s32 x;
    s32 z;
    s16 y;

    event_saved_offsets_decode(ptrs);
    p = ptrs[area];
    if (p == NULL) {
        return;
    }
    for (;;) {
        s32 n = *p++;

        if (n == 0xff) {
            break;
        }
        (actor_status.objects + n)->lifecycle = *p++;
    }
    for (;;) {
        s32 n = *p++;

        if (n == 0xff) {
            break;
        }
        script = (actor_status.groups + n)->targets[0];
        script[16] = *p++;
        script[19] = *p++;
    }
    rec = map_objects_slots.objects;
    for (i = 0; i < 396; i++) {
        switch (*p++) {
        case 0xff:
            rec->object_id = 0xff;
            break;
        case 0xf4:
            rec->arg.b.unk38 = *p++;
        case 0xf5:
            rec->arg.b.unk39 = *p++;
            break;
        case 0xf0:
            map_object_reset(rec);
            rec->action = 0x60;
            rec->object_id = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            rec->angle.vy = *p++ << 4;
            rec->angle.vz = 0x400;
        place:
            rec->pos.vx = x << 2;
            rec->pos.vz = z << 2;
            rec->pos.vy = y;
            rec->action_state = 99;
            collision_sample_map_cell_layer(rec->pos.vx, rec->pos.vy, rec->pos.vz);
            rec->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
            rec->arg.b.unk38 = 0xff;
            break;
        case 0xf1:
            map_object_reset(rec);
            rec->action = 0x61;
            rec->object_id = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            goto place;
        case 0xf2:
            map_object_reset(rec);
            rec->action = 0x62;
            rec->object_id = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            rec->arg.h.unk3a = (*p++ << 2) + (rand() >> 13);
            if (rec->object_id == 0x79) {
                rec->angle.vx = 0x400;
            }
            goto place;
        case 0xf3:
            map_object_reset(rec);
            rec->action = 0x70;
            rec->object_id = *p++;
        case 0xfd:
            rec->arg.b.unk38 = *p++;
            break;
        }
        rec++;
    }
}
