#ifndef GAME_ENTITY_H
#define GAME_ENTITY_H

/*
 * GAME.EXE Actor records (src/game/entity*.c, WIP names): the 200
 * records of actor_status made from its ActorGroup records, their placement
 * on the map, the script records each runs, their movement and the searches
 * over them. actor_status.current and .def name the record being updated.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/player.h>
#include <game/collision.h>

/* A script record's fields past the id and chances depend on the id (WIP view). */
#define SCRIPT_U16(script, offset) (*(u16 *)((script) + (offset)))
#define SCRIPT_S16(script, offset) (*(s16 *)((script) + (offset)))

/* Script 5's two byte flags where the other scripts keep their phase (Actor.unk70). */
#define ACTOR_B70(obj) (((u8 *)&(obj)->target_phase)[0])
#define ACTOR_B71(obj) (((u8 *)&(obj)->target_phase)[1])

/* Actor.unk0 of a free record. */
#define ACTOR_FREE 0xff

/* The shape word the collision tests take for an Actor record. */
#define ACTOR_SHAPE(obj) ((obj)->collision_height | (((obj)->flags & 0xc000) << 16))

/* A placement record of an area: one per Actor record (actor_load_records). */
typedef struct ActorPlacement {
    u8 slot_state;  /* Actor.unk0, 0xff for none */
    u8 group_index; /* ActorGroup record index */
    u8 placement_flags;
    u8 cell_z;      /* map cell z */
    u8 cell_x;      /* map cell x */
    u8 spawn_chance;
    u8 death_drop_object_id;
    u8 home_layer_mask; /* area */
    u16 unk8;       /* yaw, negated */
    s16 unka;       /* z offset */
    s16 unkc;       /* x offset; -1 takes ActorGroup.unk1a */
    s16 vertical_anchor_offset; /* height; -1 takes ActorGroup.unk1c */
} ActorPlacement;

extern Actor *actor_animation_shift_target;
extern u8 actor_animation_shift_enabled;

Actor *actor_pool_find_free(void);
void actor_set_home_position(Actor *obj);
void actor_copy_group_defaults(Actor *obj);
void actor_initialize_from_group(Actor *obj);
void actor_set_lifecycle_and_home_position(Actor *obj);
void actor_disable_type3_transition_actors(void);
void actor_prepare_and_initialize(Actor *obj);
void actor_prepare_and_initialize_by_index(u16 index);
void actor_pool_clear(void);
void actor_set_target(Actor *obj, u8 *script);
s32 actor_score_target_candidate(u8 *script, s32 dist);
void actor_select_best_target(s32 dist);
void actor_select_target_for_player_distance(void);
u8 *actor_find_target_of_type(ActorGroup *def, u8 id);
void actor_select_target_type_in_own_group(Actor *obj, u8 id);
void actor_reset_target_and_reselect(void);
void actor_set_animation(u8 anim);
void actor_set_animation_if_changed(u8 anim);
void actor_update_lifecycle_for_player_range(void);
void actor_retarget_or_disable_group_members(u16 kind);
s32 actor_magic_component_curve(s32 a, s32 b, s32 c);
void actor_apply_magic_to_actor(u16 index, u16 a1, u16 a2, u16 a3, u16 a4, u16 a5, u16 a6, u16 a7, u16 a8,
                                u16 a9, u16 a10, u16 a11, MagicRecord *rec, VECTOR *target);
void actor_apply_area_magic(VECTOR *pos, s32 near, s32 reach, s32 mode, u16 falloff, u16 a5, u16 a6, u16 a7,
                            u16 a8, u16 a9, u16 a10, u16 a11, u16 a12, u16 a13, u16 flags, u16 a15,
                            MagicRecord *rec);
s32 actor_try_damage_player_in_cone(s32 near, s32 range, s32 height, s32 cone, u16 a4, u16 a5, u16 a6,
                                    u16 a7);
Actor *actor_find_best_in_cone(VECTOR *pos, s16 yaw, s16 pitch, s32 range, s32 max_yaw, s32 max_pitch,
                               s32 *dist, s32 jitter);
s32 actor_find_overlap_excluding_target_type3(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
s32 actor_find_overlap(s32 x, s32 y, s32 z, s32 radius, s32 reach_up);
void actor_bind_current(Actor *obj);
void actor_advance_animation_wrapped(Actor *obj, s16 step);
#ifdef FDAT_MODULE
/*
 * The FDAT modules call it without this prototype: FDAT_077/083 pass a u16
 * script field zero-extended (lhu) where GAME.EXE's callers narrow to s16.
 */
void actor_advance_animation_clamped();
#else
void actor_advance_animation_clamped(Actor *obj, s16 step);
#endif
s32 actor_animation_crossed_phase(Actor *obj, u16 time);
s32 actor_move_horizontal_with_collision(SVECTOR *vel, s32 mode);
s32 actor_move_with_collision(SVECTOR *vel);
s32 actor_start_ballistic_motion(s32 high, s32 x, s32 y, s32 z, s32 gravity, s32 speed);
void actor_suspend_vertical_motion(void);
void actor_update_vertical_motion(void);
s32 actor_damp_horizontal_motion(s32 speed, s32 mode);
s32 actor_move_along_heading(s32 angle, s32 scale_x, s32 scale_z, s32 mode);
s32 func_8004e8ec(s32 angle, s32 target);
s32 func_8004e914(s32 angle, s32 target);
/* target travels as int (actor_update_behavior); the callee narrows it to s16 for its angle tests. */
void actor_turn_toward_angle(Actor *obj, int target, s32 max, s32 accel);
s32 actor_turn_and_move_along_heading(s16 target, s32 scale_x, s32 max, s32 scale_z, s32 accel, s32 mode);
s32 actor_turn_and_move_toward_point(s32 x, s32 z, s32 scale_x, s32 max, s16 face, s32 scale_z, s32 accel,
                                     s32 mode);
s32 actor_move_along_euler_angles(SVECTOR *angle, s32 scale_a, s32 scale_b, s32 mode);
s32 actor_turn_and_move_along_euler_angles(SVECTOR *angle, s32 scale_a, s32 max, s32 scale_b, s32 accel,
                                           s32 mode);
s32 actor_sample_rotated_animation_vertex(Actor *obj, s32 vertex, VECTOR *out);
VECTOR *actor_resolve_group_position(Actor *obj, VECTOR *out);
void actor_update_motion_animation(s32 idle, s32 left, s32 right, s32 up, s32 down, s32 speed);
s32 actor_compute_target_direction(Actor *obj, VECTOR *from, s32 speed, VECTOR *target, SVECTOR *out,
                                   s32 pitch, s16 turn, s32 steps);
void actor_dispatch_group_effect(s32 id, s32 kind, s32 vertex, ...);
s32 actor_sound_note_offset(Actor *obj);
void actor_play_target_sound(Actor *obj);
void actor_update_behavior(void);
void actor_update_frame(void);
void actor_fixup_group_targets(void);
void actor_load_records(ActorPlacement *place);

#endif
