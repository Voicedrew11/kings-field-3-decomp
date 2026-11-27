#ifndef GAME_MAP_PATCH_H
#define GAME_MAP_PATCH_H

/*
 * GAME.EXE map cell patches (src/game/map_patch.c, WIP).
 */
#include <kf/lib/types.h>
#include <sys/types.h>

/* A cell patch record (10 bytes; lists end with bytes[0][0] == 0xff): per
 * variant the own and other layer's collision_shape_id, then their object_index, and the cell
 * offset from the patch origin before the turn. */
typedef struct MapCellPattern {
    u8 variant[2][4];
    s8 dx;
    s8 dz;
} MapCellPattern;

/* Patch lists map_object_initialize_from_placements applies (.data, owner unit unknown): [def * 2 + side] of
 * four records each, and five lists of three. */
extern MapCellPattern map_object_cell_patterns[2][4];
extern MapCellPattern map_object_door_shape_pattern[3];
extern MapCellPattern map_object_door3_object_pattern[3];
extern MapCellPattern map_object_door4_object_pattern[3];
extern MapCellPattern map_object_door5_shape_pattern[3];
extern MapCellPattern map_object_door5_object_pattern[3];

void map_cell_apply_rotated_pattern(u8 area, s32 x, s32 z, s32 angle, MapCellPattern *rec, s32 index,
                                    s32 flags);
void map_cell_copy_rotated_fields(u8 flags, s32 sx, s32 sz, s32 dx, s32 dz, s32 w, s32 h, s32 angle,
                                  s32 mask);

#endif
