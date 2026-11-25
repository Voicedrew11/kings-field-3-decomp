#ifndef GAME_SCENE_H
#define GAME_SCENE_H

/*
 * GAME.EXE area set-up (src/game/scene.c, WIP names): start-up loading of
 * the archive index tables and the first FDAT.T entry, the area load loop,
 * and the shift that moves every object when the area origin changes.
 */
#include <kf/lib/types.h>
#include <game/data.h>

/* The loaded FDAT module's handler table and the FDAT window base. */
extern u32 DAT_8009c3fc;
extern u32 DAT_8009c400[];

/* Handler table installed while an FDAT module loads: 32 copies of the empty
 * resource_noop_callback (.data, owner unit unknown). */
extern FdatHandler DAT_8007e4e4[32];

void resource_initialize_game_assets(void);
void resource_run_initial_transition(void);
void translate_active_world_positions(s32 dx, s32 dy, s32 dz);
void resource_request_transition(u8 area0, u8 area1, u8 area2, u8 area3, u8 area4, s8 shift0, s8 shift1,
                                 s8 shift2);
void resource_transition_set_phase_1(void);
void resource_transition_set_phase_2(void);
void resource_transition_set_phase_3(void);
void resource_transition_set_phase_4(void);
void resource_transition_set_phase_6(void);
void resource_advance_transition(void);

#endif
