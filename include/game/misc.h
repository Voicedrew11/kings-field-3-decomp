#ifndef GAME_MISC_H
#define GAME_MISC_H

/*
 * GAME.EXE functions after the Object4c modules (src/game/misc_5bd34.c,
 * object44_move.c, ...; WIP names and module edges).
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <game/data.h>

void audio_play_sound_16(void);
void audio_play_sound_at_volume_100(s32 se);
s32 collision_probe_forward_floor(VECTOR *pos, SVECTOR *angle);
void func_8005be18(void);
void func_8005be20(void);
void scene_position_from_camera_offset(s32 x, s32 y, s32 z, s32 pitch, s32 yaw, s32 dy, s32 dz, VECTOR *out);
void scene_pose_interpolate(Object44 *obj, VECTOR *from, VECTOR *to, SVECTOR *afrom, SVECTOR *ato, s32 t);
void scene_position_interpolate(Object44 *obj, VECTOR *from, VECTOR *to, s32 t);
void scene_angles_interpolate(Object44 *obj, SVECTOR *afrom, SVECTOR *ato, s32 t);
void actor_animation_seek_phase(Object88 *obj, u8 anim, u16 time, s32 target, u16 step);
u8 event_target_stream_find_marker(u8 *script, u8 id);
void event_target_stream_execute(Object88 *obj);
u8 *event_target_stream_resolve_cursor(Object88 *obj);
void event_spawn_effect_object(Object44 *obj, s32 id);

/* A start position of player_warp_to_destination's table: areas, map cell and height, yaw. */
typedef struct Record08 {
    u8 area;        /* resource_request_transition's third area (0xff: none) */
    u8 area3;
    u8 area4;
    u8 x;           /* map cell x */
    u8 z;           /* map cell z */
    u8 y;           /* height / 128 */
    u16 yaw;        /* negated */
} Record08;

void player_warp_to_destination(s32 n);
s32 game_counter_is_nonzero(s32 id);
s32 game_counter_decrement(s32 id);
s32 game_counter_increment(s32 id);
void color_overlay_transition(s32 step, s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1);
void func_8005da94(void);
void event_state_initialize(void);
void event_saved_offsets_decode(u8 **out);
void event_arena_owner_pointers_add_delta(s32 delta);
void event_saved_offsets_encode(u8 **in);
void event_arena_owner_pointers_subtract_delta(s32 delta);

void conversation_records_initialize(void);
void conversation_record_mark_page(u8 id, u8 slot);
u16 conversation_record_page_entry(u8 n, u8 slot);
u8 conversation_records_count_characters(void);
u8 conversation_record_count_pages(u8 n);
u8 conversation_records_nth_character(u8 n);
u8 conversation_records_find_character(u8 id);
void movie_play_stream(void);
void strSetDefDecEnv(void);
void strInit(CdlLOC *loc);
void strCallback(void);
s32 strNextVlc(void);
u_long *strNext(DECENV *dec);
void strSync(DECENV *dec);
void strKickCD(CdlLOC *loc);
void movie_swap_display_buffer(void);
void movie_present_frame(void);
void movie_state_reset(void);
void movie_play(void);

/* Game loop exit word (defined in src/game/main.c). */
extern s32 DAT_8009c3f8;

#endif
