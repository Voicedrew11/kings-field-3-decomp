/*
 * GAME.EXE: the player's frame and map cell selection (same WIP module as
 * src/game/player.c, after src/game/player_action.c; the collision TU starts
 * at collision_evaluate_shape_records).
 */
/* player_update_frame polls asset_status.step, which the CD callbacks set
 * asynchronously (game/data.h SCENE_STEP_SHARED). */
#define GAME_FLOOR_SELECT_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/render.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/entity.h>
#include <game/misc.h>
#include <game/memory.h>
#include <game/object44.h>
#include <kf/lib/null.h>

RODATA(0x80011ac0, 0x4c)

/* The map cell outside the 0x28000 square map. */
DATA(0x8008190c, 0xa, ".data")
static MapCell collision_default_cell = { { { 0xff, 0x00, 0x00, 0x3f, 0x00 }, { 0xff, 0x00, 0x00, 0x3f, 0x00 } } };

/*
 * The player's frame: marks the map cell visited, reads the pad, steps the
 * walk speeds (unk180/unk184), runs the unk101 mode, then the per-frame
 * timers, the view angle sums, the Actor hit test, player_update_weapon_attack and the
 * effects of the weapon and the EquipmentRecord slots.
 */
ADDRESS(0x80030fcc, 0x1524)
void player_update_frame(void)
{
    MovieRec *rec;
    MovieRec *movie;
    s32 t;
    s32 u;
    s32 v;
    s32 hit;
    s32 i;
    s32 cz;
    u8 c;
    s32 part;
    Object4c *o4c;

    actor_status.actor_overlap_exclusion_flags = 4;
    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, -1);
    if ((s16)asset_status.loading == 0
        || ((s16)asset_status.loading == 1 && asset_status.step != 0 && asset_status.step != 1
            && asset_status.step != 2 && asset_status.step != 3 && asset_status.step != 0xf0)) {
        cz = cur_avatar_status.pos.vz >> 11;
        map_visited_cells[cur_avatar_status.pos.vx >> 11][cz >> 3] |= 1 << (7 - cz % 8);
    }
    player_update_status_visuals();
    cur_avatar_status.unk178 = PadRead(1);
    if (cur_avatar_status.unk178 & action_button_masks[12]) {
        cd_report_error(3);
    }
    if (cur_avatar_status.unk178 & action_button_masks[13]) {
        menu_run_pause_screen();
    }
    cur_avatar_status.unk180 = 200;
    cur_avatar_status.unk184 = 32;
    if (!(cur_avatar_status.unk178 & (action_button_masks[0] | action_button_masks[1]))) {
        cur_avatar_status.unk184 = 40;
    }
    if (cur_avatar_status.unk82 != 0) {
        if (cur_avatar_status.unk82 < 64) {
            cur_avatar_status.unk12 += 100;
            if (cur_avatar_status.unk12 > 0) {
                cur_avatar_status.unk12 = 0;
            }
        } else {
            cur_avatar_status.unk12 -= 100;
            if (cur_avatar_status.unk12 < -3300) {
                cur_avatar_status.unk12 = -3300;
            }
        }
        cur_avatar_status.unk184 >>= 1;
        cur_avatar_status.unk82--;
    } else if (cur_avatar_status.unk11 == 0) {
        cur_avatar_status.unk12 += 800;
        if (cur_avatar_status.unk12 > 2800) {
            cur_avatar_status.unk12 = 2800;
        }
    } else {
        cur_avatar_status.unk12 -= 800;
        if (cur_avatar_status.unk12 < 0) {
            cur_avatar_status.unk12 = 0;
        }
    }
    cur_avatar_status.unk180 += cur_avatar_status.unk12 * cur_avatar_status.unk180 >> 12;
    cur_avatar_status.unk184 += cur_avatar_status.unk12 * cur_avatar_status.unk184 >> 13;
    if (cur_avatar_status.unk84 != 0) {
        cur_avatar_status.unk180 = 0;
        cur_avatar_status.unk184 = 0;
        cur_avatar_status.unk84--;
        cur_avatar_status.unk162 >>= 1;
        cur_avatar_status.unk164 >>= 1;
        if ((cur_avatar_status.unk84 & 3) == 3) {
            accumulate_color_overlay(0x3c, 0x1e, 0, 0xc00, 1);
        }
    }
    if (cur_avatar_status.unkd0 != 0) {
        cur_avatar_status.unk180 >>= 1;
        cur_avatar_status.unk184 >>= 1;
    }
    switch (cur_avatar_status.unk101) {
    case 0:
    case 4:
        player_update_actions_and_charge();
        player_update_camera_rotation();
        player_update_horizontal_motion();
        player_update_vertical_motion();
        player_handle_interaction_and_menu();
        break;
    case 8:
        movie = &movie_state.movies[cur_avatar_status.unk188.turn.id];
        movie_update_state();
        if (movie->unk22 == 0x82) {
            render_frames_with_color_overlay(0x82, 0x1000, 0x1000, 0);
            cd_request_wait_idle();
            render_frames_with_color_overlay(movie->unk22, 0x1000, 0, -256);
            render_set_color_overlay(0xff, 0, 0, 0);
        } else if (movie->unk22 == 1) {
            MoveImage(&scene_display_sys.disp[scene_display_sys.frame].disp,
                      scene_display_sys.draw[scene_display_sys.frame].clip.x,
                      scene_display_sys.draw[scene_display_sys.frame].clip.y);
            DrawSync(0);
            render_frames_with_color_overlay(1, 0x1000, 0x1000, 0);
            cd_request_wait_idle();
            render_frames_with_color_overlay(movie->unk22, 0x1000, 0, -256);
        } else {
            MoveImage(&scene_display_sys.disp[scene_display_sys.frame].disp,
                      scene_display_sys.draw[scene_display_sys.frame].clip.x,
                      scene_display_sys.draw[scene_display_sys.frame].clip.y);
            DrawSync(0);
            render_frames_with_color_overlay(1, 0, 0, 0);
            cd_request_wait_idle();
        }
        break;
    case 7:
        rec = &movie_state.movies[cur_avatar_status.unk188.turn.id];
        t = cur_avatar_status.unk188.turn.unk1 << 7;
        if (rec->unkc != -1) {
            cur_avatar_status.pos.vx = fixed_lerp_q12(cur_avatar_status.pos.vx, rec->unkc, t + 1);
            cur_avatar_status.pos.vz = fixed_lerp_q12(cur_avatar_status.pos.vz, rec->unk14, t + 1);
        }
        if (rec->unk10 != -1) {
            cur_avatar_status.pos.vy = fixed_lerp_q12(cur_avatar_status.pos.vy, rec->unk10, t + 1);
        }
        if (rec->unk4 != -1) {
            cur_avatar_status.unk13c = angle_lerp_shortest_q12(0, rec->unk4, t + 1);
            cur_avatar_status.unk12c.vx = angle_lerp_shortest_q12(cur_avatar_status.unk188.turn.angle.vx, 0,
                                                                  t + 1);
        }
        if (rec->unk6 != -1) {
            cur_avatar_status.unk13e = 0;
            cur_avatar_status.unk12c.vy = angle_lerp_shortest_q12(cur_avatar_status.unk188.turn.angle.vy,
                                                                  rec->unk6, t + 1);
        }
        if (rec->unk8 != -1) {
            cur_avatar_status.unk140 = angle_lerp_shortest_q12(0, rec->unk8, t + 1);
            cur_avatar_status.unk12c.vz = angle_lerp_shortest_q12(cur_avatar_status.unk188.turn.angle.vz, 0,
                                                                  t + 1);
        }
        if (0x7f - cur_avatar_status.unk188.turn.unk1 * 4 < 0) {
            SsSetMVol(0, 0);
        } else {
            SsSetMVol(0x7f - cur_avatar_status.unk188.turn.unk1 * 4,
                      0x7f - cur_avatar_status.unk188.turn.unk1 * 4);
        }
        if (cur_avatar_status.unk188.turn.unk1++ >= 32) {
            if (cur_avatar_status.bgm_volume != 0 && sound_status.seq_playing == 1) {
                SsSeqPause(sound_status.seq_id);
            }
            player_begin_movie_playback(cur_avatar_status.unk188.turn.id);
            if (rec->unk4 != -1) {
                cur_avatar_status.unk12c.vx = rec->unk4;
                cur_avatar_status.unk13c = 0;
            }
            if (rec->unk6 != -1) {
                cur_avatar_status.unk12c.vy = rec->unk6;
                cur_avatar_status.unk13e = 0;
            }
            if (rec->unk8 != -1) {
                cur_avatar_status.unk12c.vz = rec->unk8;
                cur_avatar_status.unk140 = 0;
            }
            movie_state.unk372 = 1;
            if (rec->unka != 0) {
                render_frames_with_color_overlay(rec->unk22, 0, 0x1000, 0x100);
            }
        }
        break;
    case 9:
        rec = &movie_state.movies[cur_avatar_status.unk188.turn.id];
        u = cur_avatar_status.unk188.turn.unk1 << 7;
        if (rec->unk24 != -1) {
            cur_avatar_status.pos.vx = fixed_lerp_q12(cur_avatar_status.pos.vx, rec->unk24, u);
            cur_avatar_status.pos.vz = fixed_lerp_q12(cur_avatar_status.pos.vz, rec->unk2c, u);
        }
        if (rec->unk28 != -1) {
            cur_avatar_status.pos.vy = fixed_lerp_q12(cur_avatar_status.pos.vy, rec->unk28, u);
        }
        if (rec->unk1c != -1) {
            cur_avatar_status.unk13c = angle_lerp_shortest_q12(0, rec->unk1c, u);
            cur_avatar_status.unk12c.vx = angle_lerp_shortest_q12(cur_avatar_status.unk188.turn.angle.vx, 0,
                                                                  u);
        }
        if (rec->unk1e != -1) {
            cur_avatar_status.unk13e = 0;
            cur_avatar_status.unk12c.vy = angle_lerp_shortest_q12(cur_avatar_status.unk188.turn.angle.vy,
                                                                  rec->unk1e, u);
        }
        if (rec->unk20 != -1) {
            cur_avatar_status.unk140 = angle_lerp_shortest_q12(0, rec->unk20, u);
            cur_avatar_status.unk12c.vz = angle_lerp_shortest_q12(cur_avatar_status.unk188.turn.angle.vz, 0,
                                                                  u);
        }
        if (cur_avatar_status.unk188.turn.unk1 == 0 && cur_avatar_status.bgm_volume != 0
            && sound_status.seq_playing == 1) {
            SsSeqReplay(sound_status.seq_id);
        }
        if (cur_avatar_status.unk188.turn.unk1 * 4 < 0x7f) {
            SsSetMVol(cur_avatar_status.unk188.turn.unk1 * 4, cur_avatar_status.unk188.turn.unk1 * 4);
        } else {
            SsSetMVol(0x7f, 0x7f);
        }
        if (cur_avatar_status.unk188.turn.unk1++ >= 32) {
            player_reset_reaction_state();
            if (rec->unk1c != -1) {
                cur_avatar_status.unk12c.vx = rec->unk1c;
                cur_avatar_status.unk13c = 0;
            }
            if (rec->unk1e != -1) {
                cur_avatar_status.unk12c.vy = rec->unk1e;
                cur_avatar_status.unk13e = 0;
            }
            if (rec->unk20 != -1) {
                cur_avatar_status.unk12c.vz = rec->unk20;
                cur_avatar_status.unk140 = 0;
            }
            movie_state.unk372 = 1;
        }
        break;
    case 3:
        if (player_move_reaction_with_collision()) {
            player_reset_reaction_state();
        }
        player_handle_interaction_and_menu();
        break;
    case 5:
        player_update_actions_and_charge();
        player_update_camera_rotation();
        player_update_horizontal_motion();
        cur_avatar_status.unk16c = 0;
        player_update_vertical_motion();
        cur_avatar_status.unk16c += (rcos(cur_avatar_status.unk188.count) >> 4) - 256;
        cur_avatar_status.unk140 = rsin(cur_avatar_status.unk188.count) >> 6;
        cur_avatar_status.unk188.count = (cur_avatar_status.unk188.count + 128) & 0xfff;
        if (cur_avatar_status.unk188.count == 0) {
            cur_avatar_status.unk140 = 0;
            player_reset_reaction_state();
        }
        player_handle_interaction_and_menu();
        break;
    case 0x10:
        player_update_actions_and_charge();
        player_update_camera_rotation();
        player_update_horizontal_motion();
        cur_avatar_status.unk188.knock.dir.vy = 1;
        player_move_reaction_with_collision();
        player_update_reaction_rotation_offsets();
        /* unk134 and unk136 are tested as one word (lw). */
        if (*(s32 *)&cur_avatar_status.unk134 == 0 && cur_avatar_status.unk138 == 0) {
            player_reset_reaction_state();
        }
        player_update_vertical_motion();
        player_handle_interaction_and_menu();
        break;
    case 0x12:
        player_update_reaction_rotation_offsets();
        if (*(s32 *)&cur_avatar_status.unk134 == 0 && cur_avatar_status.unk138 == 0) {
            player_reset_reaction_state();
        }
        player_handle_interaction_and_menu();
        break;
    case 0x11:
        cur_avatar_status.hp = 0;
        cur_avatar_status.unk188.knock.push.vx =
            angle_velocity_step(-1024, cur_avatar_status.unk134, cur_avatar_status.unk188.knock.push.vx, 8,
                                4);
        cur_avatar_status.unk134 += cur_avatar_status.unk188.knock.push.vx * 3 >> 1;
        if (cur_avatar_status.unk16c < 1500) {
            cur_avatar_status.unk16c += 500;
        } else {
            cur_avatar_status.unk16c = 1500;
        }
        player_move_reaction_with_collision();
        if (++cur_avatar_status.unk13a >= 32) {
            if (cur_avatar_status.unk13a < 65) {
                c = fixed_lerp_q12(0, 0xff, (cur_avatar_status.unk13a - 32) << 7);
                render_set_color_overlay(0x82, c, c, c);
            } else {
                o4c = effect_status.objects;
                for (i = 128; i != 0; i--) {
                    o4c->unk0 = 0xff;
                    o4c++;
                }
                part = 0xff;
                if (game_counter_is_nonzero(0x6b)) {
                    actor_disable_type3_transition_actors();
                    game_counter_decrement(0x6b);
                    cur_avatar_status.unk12c.vy = 0x2d5;
                    cur_avatar_status.pos.vy = -14976;
                    cur_avatar_status.pos.vz = 0xc000;
                    cur_avatar_status.unk12c.vx = 0;
                    cur_avatar_status.unk12c.vz = 0;
                    cur_avatar_status.pos.vx = 0x1c800;
                    cur_avatar_status.unk160 = 5;
                    map_reveal_visited_rects();
                    repeat_store_word((u32 *)map_visited_cells, 0, 200);
                    event_world_state_save_slot(asset_status.area[0]);
                    player_reset_status();
                    player_recalculate_combat_stats();
                } else {
                    player_initialize_state();
                    player_recalculate_combat_stats();
                    movie_state_reset();
                    event_state_initialize();
                    cur_evt_state.unkd = 1;
                    conversation_records_initialize();
                    repeat_store_word((u32 *)map_visited_cells, 0, 200);
                }
                player_reload_map_resources(0, 0, 0, 0, 0, part);
            }
        }
        break;
    }
    cur_avatar_status.unk17a = cur_avatar_status.unk178;
    if (cur_avatar_status.unk78 != 0) {
        if (cur_avatar_status.unk78 % 30 == 0) {
            cur_avatar_status.unk174 = 2400;
            cur_avatar_status.unk176 = 60;
            player_adjust_hp_unclamped(-1);
        }
        cur_avatar_status.unk78--;
    }
    if (cur_avatar_status.unk86 != 0) {
        if (--cur_avatar_status.unk86 == 0) {
            player_recalculate_combat_stats();
        }
    }
    if (cur_avatar_status.unk88 != 0) {
        if (--cur_avatar_status.unk88 == 0) {
            player_recalculate_combat_stats();
        }
    }
    if (cur_avatar_status.unk8a != 0) {
        if (--cur_avatar_status.unk8a == 0) {
            player_recalculate_combat_stats();
        }
    }
    if (cur_avatar_status.unk92 != 0) {
        cur_avatar_status.unk1c = cur_avatar_status.unk1a;
        if (--cur_avatar_status.unk92 == 0) {
            notify_enqueue(0x1b);
        }
    }
    if (cur_avatar_status.unk94 != 0) {
        if (--cur_avatar_status.unk94 == 0) {
            player_recalculate_combat_stats();
            notify_enqueue(0x1b);
        }
    }
    cur_avatar_status.unk124.vx = cur_avatar_status.unk12c.vx + cur_avatar_status.unk134 + cur_avatar_status.unk13c + cur_avatar_status.unk144;
    cur_avatar_status.unk124.vy = cur_avatar_status.unk12c.vy + cur_avatar_status.unk136 + cur_avatar_status.unk13e + cur_avatar_status.unk146;
    cur_avatar_status.unk124.vz = cur_avatar_status.unk12c.vz + cur_avatar_status.unk138 + cur_avatar_status.unk140 + cur_avatar_status.unk148;
    map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, 1);
    actor_status.actor_overlap_exclusion_flags = 0;
    hit = actor_find_overlap_excluding_target_type3(cur_avatar_status.pos.vx, cur_avatar_status.pos.vy,
                                                    cur_avatar_status.pos.vz, 1, 0x6a4);
    if (hit != -1 && (actor_status.objects[hit].flags & 8)) {
        player_begin_actor_overlap_bob();
    }
    player_update_weapon_attack();
    if (cur_avatar_status.unkcb != 0xff) {
        if (cur_avatar_status.unkb0->hp_regen_interval != 0 && cur_avatar_status.unk9c % cur_avatar_status.unkb0->hp_regen_interval == 0) {
            player_adjust_hp(1);
        }
        if (cur_avatar_status.unkb0->mp_regen_interval != 0 && cur_avatar_status.unk9c % cur_avatar_status.unkb0->mp_regen_interval == 0) {
            player_adjust_mp(1);
        }
        if (cur_avatar_status.unkf2 == 0x35 && scene_display_sys.unk15996 != 0 && cur_avatar_status.unk9c % 900 == 0) {
            player_adjust_mp(1);
        }
    }
    if (cur_avatar_status.unkf0 != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.unkd4);
        player_apply_equipment_mp_tick(cur_avatar_status.unkd4);
    }
    if (cur_avatar_status.unkf1 != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.unkd8);
        player_apply_equipment_mp_tick(cur_avatar_status.unkd8);
    }
    if (cur_avatar_status.unkf2 != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.unkdc);
        player_apply_equipment_mp_tick(cur_avatar_status.unkdc);
    }
    if (cur_avatar_status.unkf3 != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.unke0);
        player_apply_equipment_mp_tick(cur_avatar_status.unke0);
    }
    if (cur_avatar_status.unkf4 != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.unke4);
        player_apply_equipment_mp_tick(cur_avatar_status.unke4);
    }
    if (cur_avatar_status.unkf5 != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.unke8);
        player_apply_equipment_mp_tick(cur_avatar_status.unke8);
    }
    if (cur_avatar_status.unkf6 != 0xff) {
        player_apply_equipment_hp_tick(cur_avatar_status.unkec);
        player_apply_equipment_mp_tick(cur_avatar_status.unkec);
    }
    if (cur_avatar_status.unkcb == 0x16 && cur_avatar_status.unk9c % 150 == 0 && (rand() >> 12) == 0) {
        cur_avatar_status.unk84 = 200;
    }
    if (cur_avatar_status.unkf2 == 0x3a) {
        light_interpolate_color_and_fog(10, 10, 10, 0xd05, 6000, 0x800);
    }
    if ((cur_avatar_status.unkf5 == 0x52 || cur_avatar_status.unkf6 == 0x52) && (rand() >> 7) == 0) {
        cur_avatar_status.unk78 = 900;
    }
    cur_avatar_status.unk9c++;
}

/*
 * Selects the map cell under x/z (2048 units a cell) and the layer of it
 * (0 or 5 bytes in) whose height suits y; returns that floor height.
 */
ADDRESS(0x800324f0, 0x11c)
s32 collision_sample_map_cell_layer(s32 x, s32 y, s32 z)
{
    MapCell *cell;
    u16 h;

    if (x >= 0 && x <= 0x27fff && z >= 0 && z <= 0x27fff) {
        cell = &map_world.cells[z >> 11][x >> 11];
        h = -(y >> 7);
        if (cell->layer[0].elevation > cell->layer[1].elevation) {
            if (h >= cell->layer[0].elevation || cell->layer[1].elevation == 0) {
            lower:
                map_world.collision_layer_offset = 0;
                map_world.collision_layer_height = -cell->layer[0].elevation << 7;
            } else {
            upper:
                map_world.collision_layer_offset = 5;
                map_world.collision_layer_height = -cell->layer[1].elevation << 7;
            }
        } else {
            if (h < cell->layer[1].elevation && cell->layer[0].elevation != 0) {
                goto lower;
            }
            goto upper;
        }
    } else {
        cell = &collision_default_cell;
        map_world.collision_layer_offset = 0;
        map_world.collision_layer_height = 0;
    }
    map_world.collision_cell = cell;
    return map_world.collision_layer_height;
}
