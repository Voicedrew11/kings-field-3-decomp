#ifndef GAME_OBJECT44_H
#define GAME_OBJECT44_H

/*
 * GAME.EXE MapObject records (src/game/object44.c, WIP): the 396 records of
 * map_objects_slots.objects and their MapObjectDef table.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/data.h>

/* One record of an area's MapObject placement table (map_object_initialize_from_placements reads 350). */
typedef struct MapObjectPlacement {
    u8 layer_mask;  /* +0x00: MapObject.unk0 (area) */
    u8 cell_z;      /* +0x01: map cell z */
    u8 cell_x;      /* +0x02: map cell x */
    u8 unk3;
    u16 object_id;  /* +0x04: MapObjectDef index, 0xffff for none */
    u16 rotation_y; /* +0x06: yaw, negated */
    s16 local_z;    /* +0x08: z offset in the cell */
    s16 local_x;    /* +0x0a: x offset in the cell; kind 0xe0: 0xff or a target cell x */
    s16 height;     /* +0x0c: height */
    u8 unke[2];
    u8 arg_bytes[8]; /* +0x10: copied to MapObject.arg from +0x38; [0] the scale, 0x80 = 1 */
} MapObjectPlacement;

void map_object_play_spatial_sound(MapObject *obj, s32 se);
void map_object_pool_reset(void);
void map_object_reset(MapObject *obj);
void map_object_set_property(s32 id, s32 mode, ...);
void map_object_set_cell_marker(MapObject *obj, s32 mode, u8 value);
void map_object_refresh_cell_markers(s32 mode);
void map_object_initialize_from_placements(MapObjectPlacement *place);
s32 map_object_find_collision_at_point(s32 x, s32 y, s32 z, s32 radius, s32 shape);
/* angle and range travel as int (FDAT callers); the callee narrows them to s16 at entry. */
s32 map_object_find_interaction_target(s32 start, VECTOR *pos, s32 height, s32 reach_up, int angle,
                                       int range);
void map_object_start_action_if_idle(MapObject *obj, u8 kind);
MapObject *map_object_effect_pool_acquire(s32 first, s32 count, s32 stamp);
void map_object_spawn_effect(u8 which, u8 def_id, VECTOR *pos, s32 dy);
void map_object_spawn_scattered_effect(u16 target, VECTOR *pos, s32 dy);
void map_object_apply_marker_signal(u8 id);
s32 map_object_check_and_consume_marker(MapObject *obj, s32 id);
void map_object_process_marker_all(u8 id);
void map_object_sample_world_vertex(MapObject *obj, s32 vertex, VECTOR *out);
s32 player_camera_within_map_region(s32 x, s32 z, s32 w, s32 h, s32 y);
s32 map_object_step_offset_motion(MapObject *src, MapObject *dst, SVECTOR *from, SVECTOR *to, s32 fade,
                                  s32 steps);
/* K&R definition: mode is u8 in the callee, int at the call (player_update_frame passes an s16). */
void render_frames_with_color_overlay(int mode, s32 level, s32 end, s32 step);
void translate_player_effects_and_actors(s32 dx, s32 dy, s32 dz);
s16 map_object_compute_aim_vector(MapObject *obj, VECTOR *from, s32 dist, VECTOR *to, SVECTOR *out, s16 pitch,
                                  s16 rate, s32 count);

#endif
