/*
 * GAME.EXE: main and the game loop (WIP module). main sets up the heap above
 * the FDAT window, CD, pads and memory cards and runs the loop; the loop
 * clears and initialises the game state, runs one frame per pass until a
 * module sets the exit word, then shuts down and tells PSX.EXE which program
 * comes next. Its unit is -G8: the loop's exit word is $gp-relative and the
 * clear RECT is an .sdata literal.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/press.h>
#include <psyq/kernel.h>
#include <MALLOC.H>
#include <game/sound.h>
#include <game/data.h>
#include <game/memory.h>
#include <game/image.h>
#include <game/scene.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/object4c.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/funcs.h>
#include <game/card.h>
#include <game/player.h>

/*
 * PSX.EXE's next-program byte (0 OPEN, 1 GAME, 2 END) and the mailbox byte
 * beside it, both just past PSX.EXE's bss (see src/psx/main.c).
 */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800102f0)
#define MAILBOX_BYTE_8 ((u8 *)0x800102f8)
#define PROGRAM_OPEN 0
#define PROGRAM_END 2

/* Top of the heap main hands to InitHeap. */
#define HEAP_END 0x801fc000

/* main_end_flag values that end the game loop. */
#define EXIT_TITLE 2
#define EXIT_ENDING_A 3
#define EXIT_ENDING_B 4


SDATA(0x8009c214, 0x8)

/* Read by the game loop; other modules set it to end the game. */
DATA(0x8009c3f8, 0x4, ".sbss")
s32 main_end_flag;

void game_main_loop(void);
void game_shutdown(void);

ADDRESS(0x80014b48, 0x8c)
int main(void)
{
    fdat_window_bounds[5] = ((fdat_window_bounds[4] - fdat_window_bounds[2] + 2047) & ~2047) + fdat_window_bounds[2];
    InitHeap((unsigned long *)fdat_window_bounds[5], HEAP_END - fdat_window_bounds[5]);
    CdInit();
    PadInit(0);
    InitCARD(1);
    ChangeClearPAD(0);
    ExitCriticalSection();
    game_main_loop();
}

ADDRESS(0x80014bd4, 0x490)
void game_main_loop(void)
{
    RECT rect = {1016, 96, 32, 32};
    VECTOR pos;
    SVECTOR angle;
    s32 continued = 0;
    s32 exit;

    repeat_store_word((u32 *)&asset_status, 0, sizeof(asset_status) / 4);
    repeat_store_word((u32 *)&scene_display_sys, 0, sizeof(scene_display_sys) / 4);
    repeat_store_word((u32 *)&cur_avatar_status, 0, sizeof(cur_avatar_status) / 4);
    repeat_store_word((u32 *)&cur_evt_state, 0, sizeof(cur_evt_state) / 4);
    repeat_store_word((u32 *)&map_world, 0, sizeof(map_world) / 4);
    repeat_store_word((u32 *)&map_objects_slots, 0, sizeof(map_objects_slots) / 4);
    repeat_store_word((u32 *)&actor_status, 0, sizeof(actor_status) / 4);
    repeat_store_word((u32 *)&effect_status, 0, sizeof(effect_status) / 4);
    repeat_store_word((u32 *)&sound_status, 0, sizeof(sound_status) / 4);
    repeat_store_word((u32 *)&movie_state, 0, sizeof(movie_state) / 4);
    repeat_store_word((u32 *)map_visited_cells, 0, sizeof(map_visited_cells) / 4);
    repeat_store_word((u32 *)map_revealed_rects, 0, sizeof(map_revealed_rects) / 4);
    cd_initialize();
    display_initialize();
    loading_screen_show();
    audio_initialize_runtime();
    map_object_pool_reset();
    actor_pool_clear();
    effect_pool_reset();
    resource_initialize_game_assets();
    movie_state_reset();
    EnterCriticalSection();
    DecDCTReset(0);
    ExitCriticalSection();
    event_state_initialize();
    conversation_records_initialize();
    memory_card_initialize();
    light_reset_records();
    render_reset_color_overlays();
    light_refresh_record_rotations();
    player_update_status_visuals();
    SsSetMVol(127, 127);
    if (menu_card_browser() != -1) {
        player_restore_equipment_effects();
    } else {
        continued = 1;
    }
    resource_run_initial_transition();
    if (continued && cur_avatar_status.bgm_volume != 0 && sound_status.seq_playing == 1) {
        SsSeqPause(sound_status.seq_id);
    }
    player_sync_position_to_map();
    vram_scroll_start(&rect, 0, 1, 1);
    memory_arena_initialize_blocks((void *)resource_space, sizeof(resource_space));
    if (continued == 1) {
        movie_state.movie_id = 3;
        movie_state.current_movie = &movie_state.movies[3];
        movie_play();
        movie_state.stream_status = -1;
        movie_state.state = 0;
        movie_state.movie_id = 0xff;
        cur_evt_state.unkd = 1;
        SsSetMVol(127, 127);
        if (cur_avatar_status.bgm_volume != 0 && sound_status.seq_playing == continued) {
            SsSeqReplay(sound_status.seq_id);
        }
    }
    render_frames_with_color_overlay(0x82, 0x1000, 0x1000, 0);
    cd_request_wait_idle();
    render_frames_with_color_overlay(0x82, 0x1000, 0, -128);
    main_end_flag = 0;
    for (;;) {
        light_reset_records();
        render_reset_color_overlays();
        map_object_update_actions();
        player_update_frame();
        actor_update_frame();
        effect_pool_sweep();
        cur_avatar_status.force_actor_lifecycle_refresh = 0;
        game_update_ending();
        resource_advance_transition();
        movie_update_state();
        player_get_camera_pose(&pos, &angle);
        audio_update_listener(&pos, &angle);
        light_refresh_record_rotations();
        cd_request_service_stream();
        cd_request_service_vab();
        render_game_frame(&pos, &angle);
        exit = main_end_flag;
        if (exit == 0) {
            continue;
        }
        switch (exit) {
        case EXIT_ENDING_A:
            game_shutdown();
            *NEXT_PROGRAM_BYTE = PROGRAM_END;
            *MAILBOX_BYTE_8 = 2;
            return;
        case EXIT_ENDING_B:
            game_shutdown();
            *NEXT_PROGRAM_BYTE = PROGRAM_END;
            *MAILBOX_BYTE_8 = 3;
            return;
        case EXIT_TITLE:
            game_shutdown();
            *NEXT_PROGRAM_BYTE = PROGRAM_OPEN;
            return;
        }
    }
}

ADDRESS(0x80015064, 0x38)
void game_shutdown(void)
{
    audio_shutdown();
    cd_close_events();
    PadStop();
    ResetGraph(3);
}
