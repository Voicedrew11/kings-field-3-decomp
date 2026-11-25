#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

/*
 * GAME.EXE player state functions (src/game/player.c, WIP module) over the
 * player record DAT_801b24e4 (game/data.h). Signatures follow the call sites
 * reconstructed so far.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>

void player_update_collision_bounds(void);
s32 player_distance_to_point_with_margin(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
void player_reset_view(void);
void player_restore_equipment_effects(void);
int resource_mark_area_models_reclaimable(int part);
void player_reload_map_resources(s32 area0, s32 area1, s32 area2, s32 area3, s32 area4, s32 part);
s32 player_charge_gain_for_rank(s32 magic, s32 weight);
void player_add_equipment_bonuses(s32 id);
void player_recalculate_combat_stats(void);
void player_learn_element_magic(u32 element);
void player_increment_physical_power_training(void);
void player_increment_magic_training(struct Record18 *spell);
void player_grow_weapon_with_experience(void);
void player_add_experience(int amount);
s32 player_calculate_damage_component(s32 power, s32 defense, s32 attack);
void player_adjust_hp_unclamped(s32 amount);
void player_apply_damage_reaction(VECTOR *from, s32 damage, s32 flags);
void player_cap_status_components(s32 flags);
void player_apply_damage(u16 slash, u16 blow, u16 stab, u16 type, u16 element0, u16 element1, u16 element2,
                         u16 element3, u16 element4, u16 scale, u16 knockback, VECTOR *from);
void player_apply_radial_damage(VECTOR *pos, s32 reach, s32 radius, s32 mode, u16 falloff, u16 slash,
                                u16 blow, u16 stab, u16 type, u16 element0, u16 element1, u16 element2,
                                u16 element3, u16 element4, s32 flags, u16 knockback);
void player_get_camera_pose(VECTOR *pos, SVECTOR *angle);
void player_reset_status(void);
void player_initialize_state(void);
void game_initialize_session(void);
void player_clear_motion(void);
void player_sync_position_to_map(void);
s32 player_distance_to_point_in_cone(VECTOR *pos, s32 angle, s32 radius, s32 range);
s32 player_distance_to_point(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
void player_set_primary_magic_shortcut_id(u8 id);
void player_set_secondary_magic_shortcut_id(u8 id);
void player_set_secondary_item_shortcut_id(u8 id);
void player_set_equipment_slot(u8 id, u8 slot);
void player_equip_weapon(u8 id);
void player_begin_weapon_attack(s32 hand);
struct Object88 *player_probe_view_target_and_vectors(s32 speed, VECTOR *offset, SVECTOR *dir, s32 *dist);

/* Pad bit per action (WIP), defined in player.c; [7] toggles a setting in
 * Data801b24e4.unk178 (menu_fade_transition). */
extern u16 DAT_80081868[14];

void player_dispatch_magic_effect(s32 id, ...);

/*
 * The same WIP module after player_dispatch_magic_effect: src/game/player_cast.c,
 * player_spell.c and player_action.c (player_update_weapon_attack is not exact
 * yet).
 */
void player_sample_weapon_world_vertex(s32 vertex, VECTOR *out);
s32 player_meets_weapon_magic_requirement(s32 hand);
void player_dispatch_weapon_magic(s32 id, s32 pay, s32 dir);
void player_update_weapon_attack(void);
void player_select_magic_action(s32 id);
s32 player_move_horizontal(s32 angle, s32 speed);
void player_check_fall_death(void);
void player_play_landing_sound(s32 speed);
void player_update_vertical_motion(void);
s32 player_move_reaction_with_collision(void);
void player_update_camera_rotation(void);
void player_update_horizontal_motion(void);
s32 item_id_is_consumable(s32 id);
void player_update_actions_and_charge(void);
void player_update_reaction_rotation_offsets(void);
void player_render_frame_and_release_pool(void);
void player_handle_interaction_and_menu(void);
void player_reset_reaction_state(void);
void player_begin_movie_playback(u8 id);
void player_begin_movie_approach(u8 id);
void player_begin_movie_return(u8 id);
void player_begin_rotation_reaction(SVECTOR *dir);
void func_800308f4(void);
void player_begin_moving_damage_reaction(SVECTOR *dir, SVECTOR *push, s16 lift);
void player_begin_rotation_only_damage_reaction(SVECTOR *dir, SVECTOR *push, s16 lift);
void player_death_begin(SVECTOR *dir);
void player_begin_actor_overlap_bob(void);
s16 status_phase_step_scaled(s16 *count, s16 *timer, s32 range, s32 scale);
void player_adjust_hp(s32 amount);
void player_adjust_hp_on_weapon_hit(s32 amount);
void player_adjust_mp(s32 amount);
void player_apply_equipment_hp_tick(struct Record20 *r);
void player_apply_equipment_mp_tick(struct Record20 *r);
void player_update_status_visuals(void);

/* Callees outside the module (no definition reconstructed yet). */
void func_8005cbe0(VECTOR *pos, SVECTOR *angle, s32 id);
void func_8005e2d0(VECTOR *pos, SVECTOR *angle);

#endif
