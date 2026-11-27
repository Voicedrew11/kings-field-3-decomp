/*
 * GAME.EXE: an Actor's talk script (event_target_stream_execute), a MapObject spawn
 * helper, the player's move to one of 14 start positions (areas loaded
 * through resource_request_transition) and the item use dispatcher (event_scene_command_dispatch) (WIP
 * module). The .rdata is player_warp_to_destination's local table initializer, then the
 * jump tables of event_target_stream_execute and event_scene_command_dispatch: GCC 2.6 emits the
 * initializers of a TU ahead of its jump tables, so the three share a TU.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/cd.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/scene.h>
#include <game/image.h>
#include <game/sound.h>

RODATA(0x800130f0, 0x1a4)

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
            ((void (*)(Actor *, u8))asset_status.handlers[FDAT_SLOT_TALK_EVENT])(obj, *p);
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
    ((void (*)(VECTOR *, SVECTOR *, s32))asset_status.handlers[FDAT_SLOT_USE_ITEM])(pos, angle, id);
    if (cur_evt_state.interaction_handled == 0) {
        notify_enqueue(10);
    }
    player_clear_motion();
}
