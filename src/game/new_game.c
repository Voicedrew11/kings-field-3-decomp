/*
 * GAME.EXE: clearing the script flags, the block arena and its saved
 * offsets, and the item counts with the starting items, and the ending
 * (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/memory.h>
#include <game/cd.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/render.h>
#include <game/player.h>
#include <game/object44.h>
#include <game/scene.h>
#include <game/misc.h>

ADDRESS(0x8005ea64, 0xbc)
void event_state_initialize(void)
{
    u16 *off;
    s32 i;

    repeat_store_word((u32 *)cur_evt_state.flags, 0, 0x40);
    repeat_store_word((u32 *)cur_evt_state.arena, 0, 0x1400);
    memory_arena_initialize_blocks((void *)cur_evt_state.arena, 0x5000);
    off = cur_evt_state.saved_offsets;
    for (i = 31; i != -1; i--) {
        *off++ = 0xffff;
    }
    repeat_store_word((u32 *)event_item_counters_table, 0, 0x4b);
    event_item_counters_table[0][0] = 1;
    event_item_counters_table[0][42] = 1;
    event_item_counters_table[0][104] = 2;
    event_item_counters_table[0][105] = 1;
}

/*
 * The ending once FDAT slot 1 has run: with ending_state 1 and ending_variant set, fades the
 * music out, moves the player to start position 13 and plays movie 7, then
 * loads areas 9 and 11; with ending_variant clear, fades out, stops the sequence,
 * plays movie 8 and leaves the game loop (main_end_flag = 4).
 */
ADDRESS(0x8005eb20, 0x338)
void game_update_ending(void)
{
    s32 n;
    s32 volume;

    asset_status.handlers->update();
    if (cur_evt_state.ending_state != 1) {
        return;
    }
    cur_evt_state.ending_state = 2;
    if (cur_evt_state.ending_variant != 0) {
        player_warp_to_destination(13);
        n = cur_avatar_status.bgm_volume >> 1;
        volume = cur_avatar_status.bgm_volume;
        while (--n != -1) {
            volume -= 2;
            if (volume < 0) {
                SsSeqSetVol(sound_status.seq_id, 0, 0);
            } else {
                SsSeqSetVol(sound_status.seq_id, volume, volume);
            }
            DrawSync(0);
            VSync(0);
        }
        cd_request_wait_idle();
        do {
            cd_request_yield();
            resource_advance_transition();
        } while ((s16)asset_status.loading == 1);
        movie_state.movie_id = 7;
        movie_state.current_movie = &movie_state.movies[7];
        movie_play();
        movie_state.stream_status = -1;
        movie_state.state = 0;
        movie_state.movie_id = 0xff;
        player_reset_reaction_state();
        SsSetMVol(127, 127);
        resource_request_transition(0xff, 0xff, 0xff, 9, 11, 0x7f, 0x7f, 0x7f);
        MoveImage(&scene_display_sys.disp[scene_display_sys.frame].disp,
                  scene_display_sys.draw[scene_display_sys.frame].clip.x,
                  scene_display_sys.draw[scene_display_sys.frame].clip.y);
        DrawSync(0);
        render_frames_with_color_overlay(1, 0, 0, 0);
        cd_request_wait_idle();
    } else {
        n = cur_avatar_status.bgm_volume >> 1;
        volume = cur_avatar_status.bgm_volume;
        while (--n != -1) {
            volume -= 2;
            if (volume < 0) {
                SsSeqSetVol(sound_status.seq_id, 0, 0);
            } else {
                SsSeqSetVol(sound_status.seq_id, volume, volume);
            }
            DrawSync(0);
            VSync(0);
        }
        SsSetMVol(0, 0);
        if (sound_status.seq_playing == 1) {
            SsSeqSetVol(sound_status.seq_id, 0, 0);
            SsSeqStop(sound_status.seq_id);
            SsSeqClose(sound_status.seq_id);
            sound_status.seq_playing = 0;
        }
        render_frames_with_color_overlay(1, 0, 0x1000, 0x100);
        movie_state.movie_id = 8;
        movie_state.current_movie = &movie_state.movies[8];
        movie_play();
        movie_state.stream_status = -1;
        movie_state.state = 0;
        movie_state.movie_id = 0xff;
        main_end_flag = 4;
    }
}
