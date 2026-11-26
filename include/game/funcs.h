#ifndef GAME_FUNCS_H
#define GAME_FUNCS_H

/*
 * GAME.EXE functions the reconstructed units call that are not reconstructed
 * yet, plus two unnamed SDK-band members (sdk-unmatched-version). Signatures
 * follow the reconstructed call sites only.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

s32 menu_card_browser(void);
void player_update_frame(void);
s32 collision_sample_map_cell_layer(s32 x, s32 y, s32 z);
void map_object_update_actions(void);
void game_update_ending(void);
void event_world_state_save_slot(s32 area);
void event_world_state_restore_slot(s32 area);
void card_payload_capture_game_state(u8 *data);
void card_payload_restore_game_state(u8 *data);
void movie_update_state(void);

/* LIBSND-shaped SDK members: SsVabOpenHeadSticky(addr, vab_id, spu_addr) and
 * SsUtKeyOn(vab, prog, tone, note, fine, voll, volr) by their call shape only. */
short func_80068f7c(unsigned char *addr, short vab_id, unsigned long spu_addr);
short func_8006dce8(short vab_id, short program, short tone, short note, short fine,
                    short left, short right);

#endif
