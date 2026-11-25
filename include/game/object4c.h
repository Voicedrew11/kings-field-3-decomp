#ifndef GAME_OBJECT4C_H
#define GAME_OBJECT4C_H

/*
 * GAME.EXE Object4c records (src/game/object4c*.c, WIP names): the 128
 * records of effect_status.objects, made by effect_construct_record from an id that
 * indexes the Record18 records at the start of effect_status, then moved,
 * tested for hits and drawn. effect_status.unk2f04 names the record being
 * updated and unk2f00 its Record18 record.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/player.h>

/* Object4c.unk0 of a free record. */
#define OBJECT4C_FREE 0xff

s32 effect_play_spatial_sound(Object4c *obj, s32 se);
s32 effect_probe_collision_by_type(VECTOR *pos, s32 radius, s32 height);
/* Its u16/u8 parameters arrive as int and are narrowed at entry. */
void effect_dispatch_magic_impact(s32 flags, s32 kind, Record18 *rec, int a3, int a4, int a5, int a6, int a7,
                                  int a8, int a9, int a10, int a11, int a12, int a13, int a14,
                                  VECTOR *target);
u16 effect_magic_power(Object4c *obj);
void effect_apply_current_magic(s32 flags, s32 a1, VECTOR *target);
void effect_apply_current_magic_backstep(s32 flags);
void effect_sample_rotated_vertex(Object4c *obj, s32 vertex, VECTOR *out, SVECTOR *scale);
void effect_sample_world_vertex(Object4c *obj, s32 vertex, VECTOR *out, SVECTOR *scale);
void effect_apply_radial_magic_damage(VECTOR *pos, s32 reach, s32 radius, s32 mode, s32 falloff, s32 flags);
Object4c *effect_pool_find_free(void);
void effect_pool_initialize_scaled_world(Object4c *obj, s32 a1, s32 scale, s32 a3);
void effect_pool_initialize_scaled(Object4c *obj, s32 a1, s32 scale, s32 a3);
void effect_pool_initialize_fixed(Object4c *obj, s32 a1, s32 a2, s32 a3);
/*
 * kind, flags and id are u8 in the callee (they arrive as int); callers pass
 * 5 to 12 arguments and it reads those after vel through &vel.
 */
Object4c *effect_construct_record(int kind, int flags, int id, VECTOR *pos, SVECTOR *vel, ...);
void effect_rotate_scale_offset_y(SVECTOR *dir, VECTOR *out, s32 yaw, s32 scale);
s32 effect_move_probe(s32 scale, s32 max, s32 radius, s32 height, SVECTOR *accel);
s32 effect_aim_and_move(s32 max, s32 scale, s32 turn, s32 radius, s32 height, s32 range, s32 near_scale,
                        s32 cone);
s32 effect_target_motion(VECTOR *target, s32 max, s32 scale, s32 slow, s32 near, s32 radius, s32 height);
void effect_scale_step(s32 rate, s32 limit, s32 grow, s32 falloff, s32 mode);
void effect_spawn_zero_direction(Object4c *obj, s32 vertex);
s32 effect_spawn_at_lower_bound(VECTOR *pos, s32 a1, s32 a2, s32 height);
/* Reads up to six more arguments after a7 through &a7 (see its definition). */
void effect_spawn_motion(Object4c *obj, s32 vertex, s32 mode, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7);
s32 effect_scatter_lower_bound(VECTOR *pos, s32 count, s32 spread, s32 a3, s32 a4, s32 range);
s32 effect_collision_step(s32 radius, s32 height, s32 spin);
void effect_collision_backtrack(void);
void effect_collision_backtrack_half(void);
void effect_update_dispatch(void);
void effect_pool_reset(void);
void magic_load_records(u32 *src);
void effect_pool_sweep(void);

extern SVECTOR effect_zero_direction;
extern Object4cTrail effect_trails_table[4][24];

#endif
