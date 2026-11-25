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

/* DAT_8009c3f8 values that end the game loop. */
#define EXIT_TITLE 2
#define EXIT_ENDING_A 3
#define EXIT_ENDING_B 4


SDATA(0x8009c214, 0x8)

/* Read by the game loop; other modules set it to end the game. */
DATA(0x8009c3f8, 0x4, ".sbss")
s32 DAT_8009c3f8;

void game_main_loop(void);
void game_shutdown(void);

ADDRESS(0x80014b48, 0x8c)
int main(void)
{
    DAT_800136e8[5] = ((DAT_800136e8[4] - DAT_800136e8[2] + 2047) & ~2047) + DAT_800136e8[2];
    InitHeap((unsigned long *)DAT_800136e8[5], HEAP_END - DAT_800136e8[5]);
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

    repeat_store_word((u32 *)&DAT_8018fad4, 0, sizeof(DAT_8018fad4) / 4);
    repeat_store_word((u32 *)&DAT_80199154, 0, sizeof(DAT_80199154) / 4);
    repeat_store_word((u32 *)&DAT_801b24e4, 0, sizeof(DAT_801b24e4) / 4);
    repeat_store_word((u32 *)&DAT_801ba984, 0, sizeof(DAT_801ba984) / 4);
    repeat_store_word((u32 *)&DAT_801d4464, 0, sizeof(DAT_801d4464) / 4);
    repeat_store_word((u32 *)&DAT_8018fb3c, 0, sizeof(DAT_8018fb3c) / 4);
    repeat_store_word((u32 *)&DAT_80185da8, 0, sizeof(DAT_80185da8) / 4);
    repeat_store_word((u32 *)&DAT_801b77ec, 0, sizeof(DAT_801b77ec) / 4);
    repeat_store_word((u32 *)&DAT_801b09fc, 0, sizeof(DAT_801b09fc) / 4);
    repeat_store_word((u32 *)&DAT_801e7edc, 0, sizeof(DAT_801e7edc) / 4);
    repeat_store_word((u32 *)DAT_801b594c, 0, sizeof(DAT_801b594c) / 4);
    repeat_store_word((u32 *)DAT_801b56cc, 0, sizeof(DAT_801b56cc) / 4);
    cd_initialize();
    func_80035394();
    func_8003c9dc();
    audio_initialize_runtime();
    func_8004485c();
    func_8004b8fc();
    func_8005bbf4();
    resource_initialize_game_assets();
    func_80061918();
    EnterCriticalSection();
    DecDCTReset(0);
    ExitCriticalSection();
    func_8005ea64();
    func_80060800();
    func_80027f2c();
    func_800341e8();
    func_80034180();
    func_80034300();
    func_80030e14();
    SsSetMVol(127, 127);
    if (menu_card_browser() != -1) {
        func_80028f90();
    } else {
        continued = 1;
    }
    resource_run_initial_transition();
    if (continued && DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqPause(DAT_801b09fc.seq_id);
    }
    func_8002b760();
    func_800350fc(&rect, 0, 1, 1);
    memory_arena_initialize_blocks((void *)DAT_800c8954, sizeof(DAT_800c8954));
    if (continued == 1) {
        DAT_801e7edc.unk384 = 3;
        DAT_801e7edc.unk380 = &DAT_801e7edc.movies[3];
        func_80061bc4();
        DAT_801e7edc.unk372 = -1;
        DAT_801e7edc.unk370 = 0;
        DAT_801e7edc.unk384 = 0xff;
        DAT_801ba984.unkd = 1;
        SsSetMVol(127, 127);
        if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == continued) {
            SsSeqReplay(DAT_801b09fc.seq_id);
        }
    }
    func_80046c00(0x82, 0x1000, 0x1000, 0);
    cd_request_wait_idle();
    func_80046c00(0x82, 0x1000, 0, -128);
    DAT_8009c3f8 = 0;
    for (;;) {
        func_800341e8();
        func_80034180();
        func_80047010();
        func_80030fcc();
        func_80052e5c();
        func_8005bc50();
        DAT_801b24e4.unke = 0;
        func_8005eb20();
        resource_advance_transition();
        func_80061940();
        func_8002b330(&pos, &angle);
        audio_update_listener(&pos, &angle);
        func_80034300();
        cd_request_service_stream();
        cd_request_service_vab();
        func_800422b8(&pos, &angle);
        exit = DAT_8009c3f8;
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
