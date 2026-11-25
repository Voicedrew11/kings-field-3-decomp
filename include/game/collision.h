#ifndef GAME_COLLISION_H
#define GAME_COLLISION_H

/*
 * GAME.EXE collision tests (src/game/collision.c, WIP). A shape word carries
 * the height in its low 28 bits; results land in DAT_801d4464 from +0x12000
 * (cell, floor height, hit bits and indices, the hit point and size).
 */
#include <kf/lib/types.h>
#include <sys/types.h>

s32 collision_probe_floor_height(s32 x, s32 y, s32 z, s32 radius, s32 shape);
s32 collision_sample_map_layer_height(u8 area, s32 x, s32 z, s32 radius, s32 height);
void map_cell_add_layer_occupancy(s32 x, s32 z, s32 radius, s32 count);
s32 collision_query_shapes_with_layer_sample(s32 x, s32 y, s32 z, s32 radius, s32 shape);
void collision_cache_load_hit_bounds(void);
s32 collision_query_world(s32 x, s32 y, s32 z, s32 radius, s32 shape, u16 mask);

/* Callees outside the module; signatures from these call sites. */
s32 collision_evaluate_shape_records(s32 x, s32 y, s32 z, s32 radius, s32 shape);

#endif
