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

    repeat_store_word((u32 *)DAT_801ba984.unk4, 0, 0x40);
    repeat_store_word((u32 *)DAT_801ba984.unk104, 0, 0x1400);
    memory_arena_initialize_blocks((void *)DAT_801ba984.unk104, 0x5000);
    off = DAT_801ba984.unk5104;
    for (i = 31; i != -1; i--) {
        *off++ = 0xffff;
    }
    repeat_store_word((u32 *)DAT_800c85e8, 0, 0x4b);
    DAT_800c85e8[0][0] = 1;
    DAT_800c85e8[0][42] = 1;
    DAT_800c85e8[0][104] = 2;
    DAT_800c85e8[0][105] = 1;
}

/*
 * The ending once FDAT slot 1 has run: with unk69 1 and unk6a set, fades the
 * music out, moves the player to start position 13 and plays movie 7, then
 * loads areas 9 and 11; with unk6a clear, fades out, stops the sequence,
 * plays movie 8 and leaves the game loop (DAT_8009c3f8 = 4).
 */
ADDRESS(0x8005eb20, 0x338)
void game_update_ending(void)
{
    s32 n;
    s32 volume;

    DAT_8018fad4.handlers[1]();
    if (DAT_801ba984.unk69 != 1) {
        return;
    }
    DAT_801ba984.unk69 = 2;
    if (DAT_801ba984.unk6a != 0) {
        player_warp_to_destination(13);
        n = DAT_801b24e4.bgm_volume >> 1;
        volume = DAT_801b24e4.bgm_volume;
        while (--n != -1) {
            volume -= 2;
            if (volume < 0) {
                SsSeqSetVol(DAT_801b09fc.seq_id, 0, 0);
            } else {
                SsSeqSetVol(DAT_801b09fc.seq_id, volume, volume);
            }
            DrawSync(0);
            VSync(0);
        }
        cd_request_wait_idle();
        do {
            cd_request_yield();
            resource_advance_transition();
        } while ((s16)DAT_8018fad4.loading == 1);
        DAT_801e7edc.unk384 = 7;
        DAT_801e7edc.unk380 = &DAT_801e7edc.movies[7];
        movie_play();
        DAT_801e7edc.unk372 = -1;
        DAT_801e7edc.unk370 = 0;
        DAT_801e7edc.unk384 = 0xff;
        player_reset_reaction_state();
        SsSetMVol(127, 127);
        resource_request_transition(0xff, 0xff, 0xff, 9, 11, 0x7f, 0x7f, 0x7f);
        MoveImage(&DAT_80199154.disp[DAT_80199154.frame].disp, DAT_80199154.draw[DAT_80199154.frame].clip.x,
                  DAT_80199154.draw[DAT_80199154.frame].clip.y);
        DrawSync(0);
        render_frames_with_color_overlay(1, 0, 0, 0);
        cd_request_wait_idle();
    } else {
        n = DAT_801b24e4.bgm_volume >> 1;
        volume = DAT_801b24e4.bgm_volume;
        while (--n != -1) {
            volume -= 2;
            if (volume < 0) {
                SsSeqSetVol(DAT_801b09fc.seq_id, 0, 0);
            } else {
                SsSeqSetVol(DAT_801b09fc.seq_id, volume, volume);
            }
            DrawSync(0);
            VSync(0);
        }
        SsSetMVol(0, 0);
        if (DAT_801b09fc.seq_playing == 1) {
            SsSeqSetVol(DAT_801b09fc.seq_id, 0, 0);
            SsSeqStop(DAT_801b09fc.seq_id);
            SsSeqClose(DAT_801b09fc.seq_id);
            DAT_801b09fc.seq_playing = 0;
        }
        render_frames_with_color_overlay(1, 0, 0x1000, 0x100);
        DAT_801e7edc.unk384 = 8;
        DAT_801e7edc.unk380 = &DAT_801e7edc.movies[8];
        movie_play();
        DAT_801e7edc.unk372 = -1;
        DAT_801e7edc.unk370 = 0;
        DAT_801e7edc.unk384 = 0xff;
        DAT_8009c3f8 = 4;
    }
}
