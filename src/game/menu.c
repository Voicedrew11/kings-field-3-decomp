/*
 * GAME.EXE: the in-game menu (WIP module, 0x8001a634..0x80027f2c): the pause
 * screen, the button page and the pages it opens (items, equipment, magic,
 * status, books, map, options, saves, shops), their list and yes/no input,
 * the item preview read from ITEM.T, and the menu's own drawing helpers
 * (frame begin/present, glyph text, windows) and CD read callback. Its unit
 * is -G8: the menu state words are $gp-relative .sdata globals and .sbss
 * statics the unit defines, short labels are .sdata literals, and the name
 * tables and jump tables are its .rdata.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/libc.h>
#include <psyq/kernel.h>
#include <game/sound.h>
#include <game/data.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/render.h>
#include <game/card.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/misc.h>
#include <game/funcs.h>

RODATA(0x800110b0, 0x68c)
SDATA(0x8009c280, 0x3e)

/* PSX.EXE's mailbox byte beside its next-program byte (see src/game/main.c). */
#define MAILBOX_BYTE_8 ((u8 *)0x800102f8)
#define MAILBOX_BYTE_A ((u8 *)0x800102fa)

/* Saved list cursors of four menu lists. */
DATA(0x8009c22c, 0x1, ".sdata")
u8 DAT_8009c22c = 0;
DATA(0x8009c22d, 0x1, ".sdata")
u8 DAT_8009c22d = 0;
DATA(0x8009c22e, 0x1, ".sdata")
u8 DAT_8009c22e = 0;
DATA(0x8009c22f, 0x1, ".sdata")
u8 DAT_8009c22f = 0;

/* Menu state words (WIP). */
DATA(0x8009c230, 0x4, ".sdata")
s32 DAT_8009c230 = 0;
DATA(0x8009c234, 0x4, ".sdata")
s32 DAT_8009c234 = 0;
DATA(0x8009c238, 0x4, ".sdata")
s32 DAT_8009c238 = 0;
DATA(0x8009c23c, 0x4, ".sdata")
s32 DAT_8009c23c = -1;
DATA(0x8009c240, 0x4, ".sdata")
s32 DAT_8009c240 = 0;
DATA(0x8009c244, 0x4, ".sdata")
s32 DAT_8009c244 = 0;
DATA(0x8009c248, 0x4, ".sdata")
s32 DAT_8009c248 = 0;
DATA(0x8009c24c, 0x4, ".sdata")
s32 DAT_8009c24c = 0x20;
DATA(0x8009c250, 0x4, ".sdata")
s32 DAT_8009c250 = 0x40;
DATA(0x8009c254, 0x4, ".sdata")
s32 DAT_8009c254 = 1;
DATA(0x8009c258, 0x4, ".sdata")
s32 DAT_8009c258 = 3;
DATA(0x8009c25c, 0x4, ".sdata")
s32 DAT_8009c25c = 1;
DATA(0x8009c260, 0x4, ".sdata")
s32 DAT_8009c260 = 0;
DATA(0x8009c264, 0x4, ".sdata")
s32 DAT_8009c264 = 0;
DATA(0x8009c268, 0x4, ".sdata")
s32 DAT_8009c268 = 0;
DATA(0x8009c26c, 0x4, ".sdata")
s32 DAT_8009c26c = 0x57;
DATA(0x8009c270, 0x4, ".sdata")
s32 DAT_8009c270 = 0x30;
DATA(0x8009c274, 0x4, ".sdata")
s32 DAT_8009c274 = 0x30;
DATA(0x8009c278, 0x4, ".sdata")
s32 DAT_8009c278 = 0x30;
DATA(0x8009c27c, 0x4, ".sdata")
s32 DAT_8009c27c = 0x80;

/* Menu primitives, the loaded picture and the screen copy (WIP). */
DATA(0x8009c374, 0x4, ".sbss")
static POLY_FT4 *DAT_8009c374;
DATA(0x8009c378, 0x4, ".sbss")
static SPRT *DAT_8009c378;
DATA(0x8009c37c, 0x4, ".sbss")
static u8 *DAT_8009c37c;
DATA(0x8009c380, 0x4, ".sbss")
static u8 *DAT_8009c380;
DATA(0x8009c384, 0x1, ".sbss")
static u8 DAT_8009c384;
/* The screen copy the menu draws over (menu_enter_display_state stores it, the
 * present loads it back each frame) and where it lies in VRAM. */
DATA(0x8009c388, 0x4, ".sbss")
static u8 *DAT_8009c388;
DATA(0x8009c38c, 0x8, ".sbss")
static RECT DAT_8009c38c;
/* The rotating item preview: its translation (vz the distance), its rotation
 * and the spin added to rotation.vy each frame (debug pad edits them). */
DATA(0x8009c394, 0x8, ".sbss")
static SVECTOR DAT_8009c394;
DATA(0x8009c39c, 0x8, ".sbss")
static SVECTOR DAT_8009c39c;
DATA(0x8009c3a4, 0x4, ".sbss")
static s32 DAT_8009c3a4;

ADDRESS(0x8001a634, 0x140)
void menu_run_pause_screen(void)
{
    MenuText text;
    s32 i;

    menu_enter_display_state();
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqPause(DAT_801b09fc.seq_id);
    }
    menu_play_sound_cue(0xc);
    input_wait_release();
    text.x = 0x6a;
    text.y = 0x71;
    text.text[0] = 0xff;
    do {
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], &text);
            menu_draw_ascii_string(0x8c, 0x74, "pause", 1, 0);
            menu_present_frame();
        }
    } while (!(input_read_mark_active() & DAT_80081868[13]));
    menu_play_sound_cue(0xc);
    input_wait_release();
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqReplay(DAT_801b09fc.seq_id);
    }
    menu_exit_display_state(0);
}

ADDRESS(0x8001a774, 0x268)
s32 menu_run_root_controller(void)
{
    s32 selected;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;
    u32 pad;

    selected = 0;
    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    menu_enter_display_state();
    for (i = 0; i < 2; i++) {
        menu_frame_begin();
        menu_draw_player_status();
        menu_draw_window(0, 6, selected, 0);
        menu_present_frame();
    }
    menu_play_sound_cue(0xc);
    input_wait_release();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            input_wait_release();
        }
        switch (chosen) {
        case 0:
            result = menu_item_selection_controller();
            if (result == MENU_CANCEL) {
                result = MENU_NONE;
            }
            break;
        case 1:
            result = menu_choose_magic_action();
            if (result == MENU_CANCEL) {
                result = MENU_NONE;
            }
            break;
        case 2:
            menu_equipment_list_controller();
            break;
        case 3:
            menu_status_records_controller();
            break;
        case 4:
            menu_storage_controller();
            break;
        case 5:
            result = menu_run_system_choice();
            if (result == MENU_CANCEL) {
                result = MENU_NONE;
            }
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = menu_poll_choice_input(selected, 5, &chosen, &confirmed, &result);
        pad = PadRead(1);
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if ((pad & PADR1) && (pad & PADL1)) {
                menu_draw_location_number();
            }
            menu_draw_player_status();
            menu_draw_window(0, 6, selected, 0);
            menu_present_frame();
        }
    }
    if (result == -2 || result == -3) {
        menu_exit_display_state(1);
    } else {
        menu_exit_display_state(0);
    }
    if (result != -1 && result != -2 && result != -3 && (result & 0x1000)) {
        player_select_magic_action(result & 0xfff);
        result = -1;
    }
    return result;
}
ADDRESS(0x8001a9dc, 0xa8)
void menu_draw_location_number(void)
{
    MenuText text;
    s32 x = DAT_801b24e4.pos.vx >> 11;
    s32 z = DAT_801b24e4.pos.vz >> 11;

    text.x = 0xf7;
    text.y = 0xd2;
    menu_format_number(DAT_8018fad4.unk9 * 10000 + x * 100 + z,
                       6, 1, 0, text.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
}

ADDRESS(0x8001aa84, 0x420)
s32 menu_item_selection_controller(void)
{
    MenuList list;
    MenuEntry entries[60];
    u8 counts[64];
    u8 ids[64];
    s32 confirmed;
    s32 result;
    s32 i;
    u32 pad;
    u8 item;
    s32 picked;
    s32 n;

    result = MENU_NONE;
    confirmed = 0;
    n = menu_collect_masked_item_rows(DAT_800c85e8[0], entries, counts, ids, 0x5f, 0x95);
    menu_list_init(&list, 0, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    menu_list_set_cursor(&list, DAT_8009c22c);
    menu_cd_install_data_ready_handler();
    pad = 0;
    if (list.count != 0) {
        menu_request_item_model(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 0, 1, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            menu_show_item_guide(&list, 0, item);
        }
        pad = menu_update_list_input(&list, ids, &confirmed, &result, 0);
        DAT_8009c22c = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
            if (item == 95 || item == 96 || item == 97 || item == 98 || item == 99 || item == 100) {
                menu_wait_async_read();
                menu_release_item_model();
                if (item == 0x60) {
                    menu_show_area_map();
                } else {
                    menu_show_item_picture(item);
                }
                menu_play_sound_cue(0xe);
                menu_request_item_model(item);
                confirmed = 0;
            } else if (item == 0x85) {
                if (DAT_8018fad4.unk9 != 0x19 && DAT_8018fad4.unk9 != 0x1b
                    && (DAT_8018fad4.unk9 != 7
                        || !(DAT_801b24e4.pos.vx / 2048 >= 62 && DAT_801b24e4.pos.vx / 2048 <= 70
                             && DAT_801b24e4.pos.vz / 2048 < 12))) {
                    menu_wait_async_read();
                    menu_release_item_model();
                    picked = menu_choose_warp_destination();
                    if (picked == -1) {
                        menu_request_item_model(item);
                        confirmed = 0;
                    } else {
                        result = picked + 150;
                        break;
                    }
                }
            }
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, 1);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
    if (result >= 104 && result <= 120) {
        if (DAT_8018fad4.unk9 == 2
            && DAT_801b24e4.pos.vx / 2048 >= 10 && DAT_801b24e4.pos.vx / 2048 <= 13
            && DAT_801b24e4.pos.vz / 2048 >= 38 && DAT_801b24e4.pos.vz / 2048 <= 42) {
            if (!(result == 104 || result == 107 || result == 108 || result == 109)) {
                menu_apply_item_effect(result);
            }
        } else {
            menu_apply_item_effect(result);
        }
    }
    return result;
}

ADDRESS(0x8001aea4, 0xe4)
s32 menu_collect_masked_item_rows(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last)
{
    s32 n = 0;
    MenuEntry *src = &DAT_8007f620[first];
    u8 *count = &bag[first];
    s32 i;

    last++;
    for (i = first; i < last; count++, i++, src++) {
        if (*count != 0) {
            *entries++ = *src;
            *counts++ = *count;
            n++;
            *ids++ = i;
        }
    }
    return n;
}

ADDRESS(0x8001af88, 0x1c8)
s32 menu_collect_available_item_rows(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last)
{
    s32 n = 0;
    MenuEntry *src = &DAT_8007f620[first];
    u8 *count = &bag[first];
    s32 i;

    last++;
    for (i = first; i < last; count++, i++, src++) {
        if (*count != 0) {
            *counts = *count;
            if (i == DAT_801b24e4.unkcb || i == DAT_801b24e4.unkf0 || i == DAT_801b24e4.unkf1
                || i == DAT_801b24e4.unkf2 || i == DAT_801b24e4.unkf3
                || i == DAT_801b24e4.unkf4 || i == DAT_801b24e4.unkf5
                || i == DAT_801b24e4.unkf6 || i == DAT_801b24e4.unkc9) {
                (*counts)--;
                if (i == DAT_801b24e4.unkf5 && i == DAT_801b24e4.unkf6) {
                    (*counts)--;
                }
            }
            if (*counts != 0) {
                *entries = *src;
                *ids = i;
                entries++;
                counts++;
                ids++;
                n++;
            }
        }
    }
    return n;
}

ADDRESS(0x8001b150, 0x104)
s32 menu_collect_guide_locked_item_rows(u8 *bag, MenuEntry *entries, u8 *counts, u8 *ids, s32 first, s32 last)
{
    s32 n = 0;
    MenuEntry *src = &DAT_8007f620[first];
    u8 *count = &bag[first];
    s32 i;

    last++;
    for (i = first; i < last; count++, i++, src++) {
        if (*count != 0 && DAT_80081780[i] == 5) {
            *entries++ = *src;
            *counts++ = *count;
            n++;
            *ids++ = i;
        }
    }
    return n;
}

ADDRESS(0x8001b254, 0x2a4)
void menu_apply_item_effect(s32 item)
{
    if (item == 110 || item == 113 || item == 114 || item == 115 || item == 116 || item == 117
        || item == 118) {
        return;
    }
    if (DAT_800c85e8[0][item] == 0) {
        return;
    }
    if (item == 104) {
        DAT_801b24e4.hp += 50;
    } else if (item == 105) {
        DAT_801b24e4.hp += 15;
        DAT_801b24e4.unk78 = 0;
    } else if (item == 106) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
    } else if (item == 107) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
        DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
        player_clear_and_cap_status_effects();
    } else if (item == 108) {
        if (DAT_801b24e4.unk84 > 0) {
            DAT_801b24e4.unk84 = 0;
        }
        if (DAT_801b24e4.unk82 > 64) {
            DAT_801b24e4.unk82 = 64;
        }
        player_cap_curse_strength();
        player_cap_darkness_phase();
    } else if (item == 109) {
        DAT_801b24e4.unk1c += 40;
    } else if (item == 111) {
    } else if (item == 112) {
    } else if (item == 119) {
        DAT_801b24e4.hp += 100;
    } else if (item == 120) {
        DAT_801b24e4.hp += 200;
        DAT_801b24e4.unk1c += 100;
        player_clear_and_cap_status_effects();
    }
    if (DAT_801b24e4.hp > DAT_801b24e4.hp_max) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
    }
    if (DAT_801b24e4.unk1c > DAT_801b24e4.unk1a) {
        DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
    }
    DAT_800c85e8[0][item]--;
    if (item == 119 || item == 120) {
        DAT_800c85e8[0][118]++;
    }
    menu_play_sound_cue(9);
}

ADDRESS(0x8001b4f8, 0x6c)
void player_clear_and_cap_status_effects(void)
{
    if (DAT_801b24e4.unk84 > 0) {
        DAT_801b24e4.unk84 = 0;
    }
    if (DAT_801b24e4.unk82 > 64) {
        DAT_801b24e4.unk82 = 64;
    }
    DAT_801b24e4.unk78 = 0;
    player_cap_curse_strength();
    player_cap_darkness_phase();
}

ADDRESS(0x8001b564, 0x30)
void player_cap_darkness_phase(void)
{
    if (DAT_801b24e4.unk7e > 64) {
        DAT_801b24e4.unk7e = 64;
        DAT_801b24e4.unk80 = 0;
    }
}

ADDRESS(0x8001b594, 0x30)
void player_cap_curse_strength(void)
{
    if (DAT_801b24e4.unk7a > 64) {
        DAT_801b24e4.unk7a = 64;
        DAT_801b24e4.unk7c = 0;
    }
}

/*
 * Not exact: the map-marker argument loads (area, pos.vz) are scheduled and
 * allocated differently. The two-frame repeat is a goto loop: as a nested
 * loop its uses would be weighted by depth and the hoisted address would
 * outrank the 0x66666667 constant and the item copy ($s2/$s3/$s4).
 */
ADDRESS(0x8001b5c4, 0x2c8)
void menu_show_item_picture(u8 item)
{
    s32 n;
    s32 i;
    s32 bytes;
    u8 area;

    if (item == 0x5f) {
        n = 0x2f1;
    } else if (item == 0x63) {
        n = 0x2f2;
    } else {
        n = 0x2f3;
    }
    bytes = cd_archive_entry_extent(6, n, 0);
    DAT_8009c234 = (bytes + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = (u8 *)memory_allocate(bytes);
    cd_archive_start_async_read(6, n);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    i = 0;
    tim_upload_images(DAT_8009c37c);
    DrawSync(0);
    memory_free((void *)DAT_8009c37c);
    DAT_8009c230 = 0;
    input_wait_release();
    do {
    again:
        menu_frame_begin();
        menu_draw_item_picture(item);
        if (DAT_8018fad4.unk9 < 12 && item == 0x5f) {
            primitive_buffer_begin_poly_ft4();
            if (DAT_8009c260 < 48) {
                n = DAT_8009c260;
            } else {
                n = 0x60 - DAT_8009c260;
            }
            setRGB0(DAT_8009c374, n - 128, n - 128, n - 128);
            DAT_8009c374->tpage = 0x1e;
            DAT_8009c374->clut = 0x7e45;
            n = ((DAT_801b24e4.unk124.vy & 0xfff) + 256) >> 9;
            if (n == 8) {
                n = 0;
            }
            area = DAT_8018fad4.unk9;
            primitive_set_poly_ft4_rect((area & 3) * 56 + DAT_801b24e4.pos.vx / 2560 + 40,
                                        0xbc - DAT_801b24e4.pos.vz / 2560 - (area >> 2) * 56,
                                        15, 15, n * 16 + 128, 0x90, 15, 15);
            primitive_buffer_commit_poly_ft4(0x1c);
        }
        menu_draw_button_icon(0x80, 0xd6, DAT_8009c258);
        menu_draw_ascii_string(0x8a, 0xd6, "return", 0, 0);
        menu_present_frame();
        i++;
        if (i < 2) {
            goto again;
        }
    } while (!input_read_mark_active());
    input_wait_press();
    input_wait_release();
}

ADDRESS(0x8001b88c, 0x230)
void menu_draw_item_picture(s32 item)
{
    s32 shade = 0;

    primitive_buffer_begin_poly_ft4();
    if (DAT_801b24e4.unkfd > 128) {
        shade = (DAT_801b24e4.unkfd - 128) / 4;
    }
    setRGB0(DAT_8009c374, shade + 64, shade + 64, shade + 64);
    SetSemiTrans((void *)DAT_8009c374, DAT_801b24e4.unk100);
    DAT_8009c374->tpage = GetTPage(0, 0, 0x3c0, 0x100);
    DAT_8009c374->clut = 0x7806;
    if (item == 0x5f) {
        primitive_set_poly_ft4_rect(0x2b, 0x17, 0xea, 0xb2, 0, 0, 0xea, 0xb2);
    } else if (item == 0x63) {
        primitive_set_poly_ft4_rect(0x50, 0x16, 0xa0, 0xb4, 0, 0, 0xa0, 0xb4);
    } else {
        primitive_set_poly_ft4_rect(0x46, 0x16, 0xb4, 0xb4, 0, 0, 0xb4, 0xb4);
    }
    primitive_buffer_commit_poly_ft4(0x1e);
    primitive_buffer_begin_poly_ft4();
    SetSemiTrans((void *)DAT_8009c374, 1);
    setRGB0(DAT_8009c374, shade + 64, shade + 64, shade + 64);
    DAT_8009c374->tpage = GetTPage(0, 1, 0x3c0, 0x100);
    DAT_8009c374->clut = 0x7806;
    if (item == 0x5f) {
        primitive_set_poly_ft4_rect(0x2b, 0x17, 0xea, 0xb2, 0, 0, 0xea, 0xb2);
    } else if (item == 0x63) {
        primitive_set_poly_ft4_rect(0x50, 0x16, 0xa0, 0xb4, 0, 0, 0xa0, 0xb4);
    } else {
        primitive_set_poly_ft4_rect(0x46, 0x16, 0xb4, 0xb4, 0, 0, 0xb4, 0xb4);
    }
    primitive_buffer_commit_poly_ft4(0x1d);
}

ADDRESS(0x8001babc, 0x28c)
void menu_show_area_map(void)
{
    s32 n;
    s32 i;
    s32 bytes;
    s32 size = 15;

    n = DAT_8018fad4.unk9 + 721;
    bytes = cd_archive_entry_extent(6, n, 0);
    DAT_8009c234 = (bytes + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = (u8 *)memory_allocate(bytes);
    cd_archive_start_async_read(6, n);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    i = 0;
    tim_upload_images(DAT_8009c37c);
    DrawSync(0);
    memory_free((void *)DAT_8009c37c);
    DAT_8009c230 = 0;
    map_reveal_visited_rects();
    input_wait_release();
    do {
        do {
            menu_frame_begin();
            menu_draw_revealed_map_rects();
            primitive_buffer_begin_poly_ft4();
            if (DAT_8009c260 < 48) {
                n = DAT_8009c260;
            } else {
                n = 0x60 - DAT_8009c260;
            }
            setRGB0(DAT_8009c374, n - 128, n - 128, n - 128);
            DAT_8009c374->tpage = 0x1e;
            DAT_8009c374->clut = 0x7e45;
            n = ((DAT_801b24e4.unk124.vy & 0xfff) + 256) >> 9;
            if (n == 8) {
                n = 0;
            }
            if (DAT_8018fad4.unk9 == 0x19 || DAT_8018fad4.unk9 == 0x1b) {
                primitive_set_poly_ft4_rect(0x98, 0x70, 15, 15, n * 16 + 128, 0x90, size, size);
            } else {
                primitive_set_poly_ft4_rect(DAT_801b24e4.pos.vx / 1024 + 72,
                                            0xbc - DAT_801b24e4.pos.vz / 1024, 15, 15, n * 16 + 128, 0x90,
                                            size, size);
            }
            primitive_buffer_commit_poly_ft4(0x1d);
            menu_draw_backdrop(0x32, 0x1a, 0xdc, 0xb4);
            menu_draw_button_icon(0x80, 0xd6, DAT_8009c258);
            menu_draw_ascii_string(0x8a, 0xd6, "return", 0, 0);
            menu_present_frame();
            i++;
        } while (i < 2);
    } while (!input_read_mark_active());
    input_wait_press();
    input_wait_release();
}

ADDRESS(0x8001bd48, 0x160)
void menu_draw_revealed_map_rects(void)
{
    s32 i;
    MapRect *rect;
    s32 bit;

    for (i = 159; i != -1; i--) {
        rect = DAT_801ba6fc + i;
        if (DAT_801b56cc[DAT_8018fad4.unk9][i / 8] & (bit = 1 << (7 - i % 8))) {
            primitive_buffer_begin_poly_ft4();
            setRGB0(DAT_8009c374, 0x7f, 0x7f, 0x7f);
            DAT_8009c374->tpage = GetTPage(0, 0, 0x3c0, 0x100);
            DAT_8009c374->clut = 0x7806;
            primitive_set_poly_ft4_rect(rect->x + 80, 0xc4 - rect->y - rect->h, rect->w, rect->h,
                                        rect->x, 0xa0 - rect->y - rect->h, rect->w, rect->h);
            primitive_buffer_commit_poly_ft4(0x1e);
        }
    }
}

ADDRESS(0x8001bea8, 0x188)
void map_reveal_visited_rects(void)
{
    s32 i;
    s32 x;
    s32 y;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    MapRect *rect;
    s32 bit;

    for (i = 159; i != -1; i--) {
        rect = DAT_801ba6fc + i;
        if (!(DAT_801b56cc[DAT_8018fad4.unk9][i / 8] & (bit = 1 << (7 - i % 8)))) {
            x0 = rect->x / 2;
            y0 = rect->y / 2;
            x1 = x0 + rect->w / 2;
            y1 = y0 + rect->h / 2;
            for (x = x0; x < x1; x++) {
                for (y = y0; y < y1; y++) {
                    if (DAT_801b594c[x][y / 8] & (bit = 1 << (7 - y % 8))) {
                        DAT_801b56cc[DAT_8018fad4.unk9][i / 8] |= 1 << (7 - i % 8);
                        x = x1;
                        y = y1;
                    }
                }
            }
        }
    }
}

ADDRESS(0x8001c030, 0x45c)
s32 menu_choose_warp_destination(void)
{
    s32 selected = 0;
    s32 xs[12] = {10, 32, 56, 7, 56, 29, 34, 19, 0, 39, 37, 58};
    s32 ys[12] = {56, 14, 35, 31, 13, 31, 67, 29, 0, 56, 51, 34};
    s32 i;
    s32 bytes;
    s32 n;
    u32 pad;

    if (DAT_8018fad4.unk9 < 12 && DAT_8018fad4.unk9 != 8) {
        selected = DAT_8018fad4.unk9;
    }
    bytes = cd_archive_entry_extent(6, 0x2f1, 0);
    DAT_8009c234 = (bytes + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = (u8 *)memory_allocate(bytes);
    cd_archive_start_async_read(6, 0x2f1);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    tim_upload_images(DAT_8009c37c);
    DrawSync(0);
    memory_free((void *)DAT_8009c37c);
    DAT_8009c230 = 0;
    input_wait_release();
    for (;;) {
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_item_picture(0x5f);
            primitive_buffer_begin_poly_ft4();
            if (DAT_8009c260 < 48) {
                n = DAT_8009c260;
            } else {
                n = 0x60 - DAT_8009c260;
            }
            setRGB0(DAT_8009c374, n - 96, n - 96, n / 2 - 96);
            DAT_8009c374->tpage = 0x1e;
            DAT_8009c374->clut = 0x7d85;
            primitive_set_poly_ft4_rect(selected % 4 * 56 + 40 + xs[selected] * 4 / 5,
                                        0xbc - selected / 4 * 56 - ys[selected] * 4 / 5, 16, 16, 0xa0, 0x80,
                                        16, 16);
            primitive_buffer_commit_poly_ft4(0x1c);
            menu_draw_button_icon(0x5f, 0xd6, DAT_8009c254);
            menu_draw_ascii_string(0x69, 0xd6, "select", 0, 0);
            menu_draw_button_icon(0xa0, 0xd6, DAT_8009c258);
            menu_draw_ascii_string(0xaa, 0xd6, "return", 0, 0);
            menu_present_frame();
        }
        input_wait_brief_release();
        pad = input_read_mark_active();
        if (pad == PADLup) {
            menu_play_sound_cue(0xc);
            selected += 4;
            if (selected == 8) {
                selected = 0;
            } else if (selected >= 12) {
                selected -= 12;
            }
        } else if (pad == PADLdown) {
            menu_play_sound_cue(0xc);
            selected -= 4;
            if (selected == -4) {
                selected = 4;
            } else if (selected < 0) {
                selected += 12;
            }
        } else if (pad == PADLright) {
            menu_play_sound_cue(0xc);
            if (selected == 3 || selected == 7) {
                selected -= 3;
            } else if (selected == 11) {
                selected = 9;
            } else {
                selected++;
            }
        } else if (pad == PADLleft) {
            menu_play_sound_cue(0xc);
            if (selected == 0 || selected == 4) {
                selected += 3;
            } else if (selected == 9) {
                selected = 11;
            } else {
                selected--;
            }
        } else if (pad == DAT_8009c24c) {
            menu_play_sound_cue(0xd);
            break;
        } else if (pad == DAT_8009c250) {
            selected = -1;
            menu_play_sound_cue(0xe);
            break;
        }
    }
    input_wait_press();
    input_wait_release();
    return selected;
}

ADDRESS(0x8001c48c, 0x234)
s32 menu_choose_magic_action(void)
{
    u32 pad = 0;
    MenuList list;
    MenuEntry entries[20];
    u32 values[20];
    u8 ids[24];
    s32 confirmed;
    s32 result;
    s32 i;
    s32 n;

    result = MENU_NONE;
    confirmed = 0;
    n = menu_collect_available_magic_rows(DAT_801b77ec.unk0, entries, values, ids, 0x17, 0x1e);
    menu_list_init(&list, 0, 1);
    list.rows = 4;
    list.count = n;
    list.entries = entries;
    list.values = values;
    list.y = 0x83;
    menu_list_set_cursor(&list, DAT_8009c22d);
    menu_cd_install_data_ready_handler();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 0, 2, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = ids[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            menu_show_item_guide(&list, 1, ids[list.cursor]);
        }
        pad = menu_update_list_input(&list, 0, &confirmed, &result, 0);
        DAT_8009c22d = list.cursor;
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_render_list(&list, 2);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
    if (result != -1) {
        result |= 0x1000;
    }
    return result;
}

ADDRESS(0x8001c6c0, 0xe8)
s32 menu_collect_available_magic_rows(Record18 *spells, MenuEntry *entries, u32 *values, u8 *ids, s32 first,
                                      s32 last)
{
    MenuEntry *src = &DAT_80080430[first];
    Record18 *spell = &spells[first];
    s32 n = 0;
    s32 i;

    last++;
    for (i = first; i < last; spell++, i++, src++) {
        if (spell->unk0 == 1) {
            *entries++ = *src;
            *values++ = spell->unk16;
            n++;
            *ids++ = i;
        }
    }
    return n;
}

ADDRESS(0x8001c7a8, 0x218)
void menu_equipment_list_controller(void)
{
    MenuList list;
    MenuEntry labels[10];
    MenuEntry names[10];
    s32 confirmed;
    s32 result;
    s32 i;

    confirmed = 0;
    result = MENU_NONE;
    memcpy((void *)labels, (const void *)DAT_8007f230, sizeof(labels));
    menu_build_equipped_label_rows(names);
    menu_list_init(&list, 0, 2);
    list.count = 10;
    list.rows = 10;
    list.entries = labels;
    list.y = 0x23;
    list.unk28 = names;
    list.unk0 = 0;
    list.unk23 = 0x18;
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            if (list.cursor == 1) {
                menu_choose_primary_magic_shortcut();
            } else if (list.cursor == 9) {
                menu_item_magic_controller();
            } else {
                menu_equipment_category_controller(list.cursor);
            }
            menu_build_equipped_label_rows(names);
        }
        if (result != MENU_NONE) {
            break;
        }
        menu_update_list_input(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_render_list(&list, 3);
            menu_present_frame();
        }
    }
}

ADDRESS(0x8001c9c0, 0x1f8)
void menu_build_equipped_label_rows(MenuEntry *out)
{
    u8 slots[10];
    s32 i;
    u8 *slot;

    slots[0] = DAT_801b24e4.unkcb;
    slots[1] = DAT_801b24e4.unkc7;
    slots[2] = DAT_801b24e4.unkf2;
    slots[3] = DAT_801b24e4.unkf0;
    slots[4] = DAT_801b24e4.unkf1;
    slots[5] = DAT_801b24e4.unkf3;
    slots[6] = DAT_801b24e4.unkf4;
    slots[7] = DAT_801b24e4.unkf5;
    slots[8] = DAT_801b24e4.unkf6;
    if (DAT_801b24e4.unkc8 == 0xff) {
        slots[9] = DAT_801b24e4.unkc9;
    } else {
        slots[9] = DAT_801b24e4.unkc8;
    }
    slot = slots;
    for (i = 0; i < 10; i++) {
        if (*slot != 0xff) {
            if (i == 1 || (i == 9 && DAT_801b24e4.unkc8 != 0xff)) {
                *out = DAT_80080430[*slot];
            } else {
                *out = DAT_8007f620[*slot];
            }
        } else {
            out->text[0] = 0xff;
        }
        out++;
        slot++;
    }
}

ADDRESS(0x8001cbb8, 0x50c)
void menu_equipment_category_controller(s32 slot)
{
    u32 pad = 0;
    u8 equipped = 0;
    MenuList list;
    MenuEntry entries[40];
    u8 counts[40];
    u8 ids[40];
    s32 confirmed;
    s32 result;
    u8 other;
    u8 cursor = 0;
    s32 first;
    s32 last;
    s32 n;
    s32 i;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    switch (slot) {
    case 0:
        first = 0;
        equipped = DAT_801b24e4.unkcb;
        last = 0x21;
        break;
    case 2:
        first = 0x33;
        equipped = DAT_801b24e4.unkf2;
        last = 0x3e;
        break;
    case 3:
        first = 0x22;
        equipped = DAT_801b24e4.unkf0;
        last = 0x29;
        break;
    case 4:
        first = 0x2a;
        equipped = DAT_801b24e4.unkf1;
        last = 0x32;
        break;
    case 5:
        first = 0x3f;
        equipped = DAT_801b24e4.unkf3;
        last = 0x47;
        break;
    case 6:
        first = 0x48;
        equipped = DAT_801b24e4.unkf4;
        last = 0x50;
        break;
    case 7:
    case 8:
        if (slot == 7) {
            equipped = DAT_801b24e4.unkf5;
        } else {
            equipped = DAT_801b24e4.unkf6;
        }
        if (slot == 7) {
            other = DAT_801b24e4.unkf6;
        } else {
            other = DAT_801b24e4.unkf5;
        }
        first = 0x51;
        if (other != 0xff) {
            DAT_800c85e8[0][other]--;
        }
        last = 0x5e;
        break;
    }
    n = menu_collect_masked_item_rows(DAT_800c85e8[0], entries, counts, ids, first, last);
    memcpy((void *)&entries[n], (const void *)&DAT_8007f230[11], 10);
    counts[n] = 0xff;
    ids[n] = 0xff;
    n++;
    menu_list_init(&list, 0, 2);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    for (i = 0; i < n - 1; i++) {
        if (ids[i] == equipped) {
            cursor = i;
            i = 40;
        }
    }
    menu_list_set_cursor(&list, cursor);
    menu_cd_install_data_ready_handler();
    if (list.count >= 2) {
        menu_request_item_model(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 5, 5, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && item != 0xff && !(pad & PADselect) && !(pad & PADstart)
            && (pad & PADRup)) {
            menu_show_item_guide(&list, 0, item);
        }
        pad = menu_update_list_input(&list, ids, &confirmed, &result, 0);
        item = ids[list.cursor];
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, 5);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
    if (result != -1) {
        switch (slot) {
        case 0:
            player_equip_weapon(result);
            break;
        case 2:
            player_set_equipment_slot(result, 4);
            break;
        case 3:
            player_set_equipment_slot(result, 0);
            break;
        case 4:
            player_set_equipment_slot(result, 1);
            break;
        case 5:
            player_set_equipment_slot(result, 2);
            break;
        case 6:
            player_set_equipment_slot(result, 3);
            break;
        case 7:
            player_set_equipment_slot(result, 5);
            break;
        case 8:
            player_set_equipment_slot(result, 6);
            break;
        }
    }
    if ((slot == 7 || slot == 8) && other != 0xff) {
        DAT_800c85e8[0][other]++;
    }
}

ADDRESS(0x8001d0c4, 0x2dc)
void menu_choose_primary_magic_shortcut(void)
{
    u32 pad = 0;
    MenuList list;
    MenuEntry entries[32];
    u32 values[32];
    u8 ids[32];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    n = menu_collect_available_magic_rows(DAT_801b77ec.unk0, entries, values, ids, 0, 0x16);
    memcpy((void *)&entries[n], (const void *)&DAT_8007f230[11], 12);
    values[n] = -1;
    ids[n] = 0xff;
    n++;
    menu_list_init(&list, 0, 2);
    list.rows = 4;
    list.count = n;
    list.entries = entries;
    list.values = values;
    list.y = 0x83;
    for (i = 0; i < n - 1; i++) {
        if (ids[i] == DAT_801b24e4.unkc7) {
            menu_list_set_cursor(&list, i);
            i = 32;
        }
    }
    menu_cd_install_data_ready_handler();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 5, 4, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = ids[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && ids[list.cursor] != 0xff && !(pad & PADselect) && !(pad & PADstart)
            && (pad & PADRup)) {
            menu_show_item_guide(&list, 1, ids[list.cursor]);
        }
        pad = menu_update_list_input(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_render_list(&list, 4);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
    if (result != -1) {
        player_set_primary_magic_shortcut_id(result);
    }
}

ADDRESS(0x8001d3a0, 0x3e4)
void menu_item_magic_controller(void)
{
    u32 pad = 0;
    s32 i = 0;
    MenuList list;
    MenuEntry entries[88];
    u8 counts[88];
    u32 values[88];
    u8 ids[88];
    u8 spells[88];
    s32 confirmed;
    s32 result;
    u8 saved;
    u8 item;
    s32 n;

    confirmed = 0;
    result = MENU_NONE;
    for (; i < 88; i++) {
        counts[i] = 0xff;
        values[i] = -1;
        ids[i] = 0xff;
        spells[i] = 0xff;
    }
    saved = DAT_800c85e8[0][0x85];
    DAT_800c85e8[0][0x85] = 0;
    n = menu_collect_masked_item_rows(DAT_800c85e8[0], entries, counts, ids, 0x65, 0x92);
    DAT_800c85e8[0][0x85] = saved;
    n += menu_collect_available_magic_rows(DAT_801b77ec.unk0, &entries[n], &values[n], &spells[n], 0, 0x1e);
    memcpy((void *)&entries[n], (const void *)&DAT_8007f230[11], 12);
    n++;
    menu_list_init(&list, 0, 2);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    list.values = values;
    if (DAT_801b24e4.unkc8 != 0xff || DAT_801b24e4.unkc9 != 0xff) {
        for (i = 0; i < 88; i++) {
            if (DAT_801b24e4.unkc8 == 0xff) {
                if (ids[i] == DAT_801b24e4.unkc9) {
                    menu_list_set_cursor(&list, i);
                    i = 88;
                }
            } else if (spells[i] == DAT_801b24e4.unkc8) {
                menu_list_set_cursor(&list, i);
                i = 88;
            }
        }
    }
    menu_cd_install_data_ready_handler();
    if (list.count >= 2) {
        menu_request_item_model(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 5, 0x10, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = list.cursor;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            if (item != 0xff) {
                menu_show_item_guide(&list, 0, item);
            } else if (spells[list.cursor] != 0xff) {
                menu_show_item_guide(&list, 1, spells[list.cursor]);
            }
        }
        pad = menu_update_list_input(&list, ids, &confirmed, &result, 0);
        item = ids[list.cursor];
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, 0x10);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
    if (result != -1) {
        player_set_secondary_magic_shortcut_id(spells[result]);
        player_set_secondary_item_shortcut_id(ids[result]);
    }
}

ADDRESS(0x8001d784, 0x120)
void menu_status_records_controller(void)
{
    MenuList list;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 selected = 0;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            input_wait_release();
        }
        switch (chosen) {
        case 0:
            menu_show_combat_attributes();
            break;
        case 1:
            menu_conversation_records_controller();
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = menu_poll_choice_input(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(1, 2, selected, 0);
            menu_present_frame();
        }
    }
}

ADDRESS(0x8001d8a4, 0xa0)
void menu_show_combat_attributes(void)
{
    s32 result = MENU_NONE;
    s32 i;

    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            break;
        }
        input_wait_brief_release();
        if (input_read_mark_active()) {
            menu_play_sound_cue(0xe);
            result = -1;
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_combat_attributes();
            menu_present_frame();
        }
    }
}

ADDRESS(0x8001d944, 0x280)
void menu_conversation_records_controller(void)
{
    s32 result = MENU_NONE;
    s32 page = 0;
    s32 row = 0;
    u8 books[64];
    u8 pages[256];
    s32 count;
    s32 n;
    s32 i;
    s32 j;
    u32 pad;

    count = conversation_records_count_characters();
    i = count;
    j = 0;
    while (--i != -1) {
        books[j] = conversation_records_nth_character(j);
        j++;
    }
    if (count == 0) {
        for (i = 0; i < 3; i++) {
            menu_frame_begin();
            menu_draw_conversation_page(-1);
            menu_present_frame();
        }
        while (!input_read_mark_active()) {
        }
        menu_play_sound_cue(0xe);
        input_wait_release();
        return;
    }
    j = page;
    n = menu_conversation_load_character(books[page], page, pages);
    menu_conversation_load_page(page, pages[row]);
    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            return;
        }
        input_wait_brief_release();
        pad = input_read_mark_active();
        if (pad & DAT_8009c250) {
            menu_play_sound_cue(0xe);
            result = -1;
        } else if (pad & PADLup) {
            menu_play_sound_cue(0xc);
            if (row > 0) {
                row--;
                menu_conversation_load_page(page, pages[row]);
            }
        } else if (pad & PADLdown) {
            menu_play_sound_cue(0xc);
            if (row < n - 1) {
                row++;
                menu_conversation_load_page(page, pages[row]);
            }
        } else if (pad & PADLleft) {
            menu_play_sound_cue(0xc);
            if (page > 0) {
                page--;
                n = menu_conversation_load_character(books[page], page, pages);
                row = 0;
                menu_conversation_load_page(page, pages[0]);
            }
        } else if (pad & PADLright) {
            menu_play_sound_cue(0xc);
            if (page < count - 1) {
                page++;
                n = menu_conversation_load_character(books[page], page, pages);
                row = 0;
                menu_conversation_load_page(page, pages[0]);
            }
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_conversation_page(books[page]);
            menu_present_frame();
        }
    }
}

ADDRESS(0x8001dbc4, 0x10c)
s32 menu_conversation_load_character(s32 index, s32 book, u8 *out)
{
    u16 entry = index + 840;
    u8 *buffer;
    s32 n;
    s32 i;
    u16 page;

    buffer = (u8 *)memory_allocate(cd_archive_entry_extent(8, entry, 0));
    cd_archive_read(8, entry, buffer);
    tim_upload_images(buffer);
    DrawSync(0);
    memory_free((void *)buffer);
    n = 0;
    for (i = 0; i < 256; i++) {
        page = conversation_record_page_entry(book, i);
        if (page != 0xffff && page != 0xfffe && page != 0xfffd) {
            *out++ = i;
            n++;
        }
        if (page == 0xffff) {
            i = 256;
        }
    }
    return n;
}

ADDRESS(0x8001dcd0, 0xc4)
void menu_conversation_load_page(s32 book, s32 page)
{
    RECT rect;
    u16 entry;
    u8 *buffer;

    setRECT(&rect, 0x3c0, 0x100, 0x40, 0xc8);
    ClearImage(&rect, 0, 0, 0);
    entry = conversation_record_page_entry(book, page);
    if (entry <= 0xf000) {
        buffer = (u8 *)memory_allocate(cd_archive_entry_extent(8, entry, 0));
        cd_archive_read(8, entry, buffer);
        tim_upload_images(buffer);
        DrawSync(0);
        memory_free((void *)buffer);
    }
}

ADDRESS(0x8001dd94, 0x150)
void menu_storage_controller(void)
{
    s32 selected = 0;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    menu_cd_install_data_ready_handler();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            input_wait_release();
        }
        switch (chosen) {
        case 0:
            menu_storage_store_controller();
            break;
        case 1:
            menu_storage_take_out_controller();
            break;
        }
        if (chosen != -1 && result == MENU_CANCEL) {
            result = MENU_NONE;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = menu_poll_choice_input(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(2, 2, selected, 0);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
}

ADDRESS(0x8001dee4, 0x2d0)
void menu_storage_store_controller(void)
{
    MenuList list;
    MenuEntry entries[152];
    u8 counts[152];
    u8 ids[152];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u32 pad;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    n = menu_collect_available_item_rows(DAT_800c85e8[0], entries, counts, ids, 0, 0x95);
    menu_list_init(&list, 2, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    menu_list_set_cursor(&list, DAT_8009c22e);
    pad = 0;
    if (list.count != 0) {
        menu_request_item_model(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 1, 7, ids[list.cursor]);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            menu_show_item_guide(&list, 0, item);
        }
        pad = menu_update_list_input(&list, ids, &confirmed, &result, 2);
        DAT_8009c22e = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (counts[list.cursor] < DAT_8009c25c || DAT_800c85e8[1][item] + DAT_8009c25c >= 100) {
                menu_play_sound_cue(0xe);
                confirmed = 0;
            } else {
                menu_play_sound_cue(0xd);
            }
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, 7);
            menu_present_frame();
        }
    }
    if (result != -1) {
        DAT_800c85e8[0][result] -= DAT_8009c25c;
        DAT_800c85e8[1][result] += DAT_8009c25c;
    }
}

ADDRESS(0x8001e1b4, 0x2d0)
void menu_storage_take_out_controller(void)
{
    MenuList list;
    MenuEntry entries[76];
    u8 counts[152];
    u8 ids[152];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u32 pad;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    n = menu_collect_masked_item_rows(DAT_800c85e8[1], entries, counts, ids, 0, 0x95);
    menu_list_init(&list, 2, 1);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    menu_list_set_cursor(&list, DAT_8009c22f);
    pad = 0;
    if (list.count != 0) {
        menu_request_item_model(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 10, 7, ids[list.cursor]);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            menu_show_item_guide(&list, 0, item);
        }
        pad = menu_update_list_input(&list, ids, &confirmed, &result, 2);
        DAT_8009c22f = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (counts[list.cursor] < DAT_8009c25c || DAT_800c85e8[0][item] + DAT_8009c25c >= 100) {
                menu_play_sound_cue(0xe);
                confirmed = 0;
            } else {
                menu_play_sound_cue(0xd);
            }
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, 7);
            menu_present_frame();
        }
    }
    if (result != -1) {
        DAT_800c85e8[0][result] += DAT_8009c25c;
        DAT_800c85e8[1][result] -= DAT_8009c25c;
    }
}

ADDRESS(0x8001e484, 0x28c)
s32 menu_run_system_choice(void)
{
    MenuText text;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 selected = 0;
    s32 i;
    s32 volume;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            input_wait_release();
        }
        switch (chosen) {
        case 0:
            result = menu_card_load_browser();
            if (result == 0) {
                result = -3;
            }
            break;
        case 1:
            menu_volume_brightness_controller();
            break;
        case 2:
            menu_display_options_controller();
            break;
        case 3:
            menu_button_config_controller();
            break;
        case 4:
            result = menu_prompt_quit_game();
            if (result == 0) {
                result = -2;
            }
            break;
        }
        if (chosen != -1 && result == MENU_CANCEL) {
            result = MENU_NONE;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = menu_poll_choice_input(selected, 4, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(3, 5, selected, 0);
            menu_present_frame();
        }
    }
    if (result == -2) {
        u32 status;

        CdControl(CdlStop, 0, (u8 *)&status);
        text.x = 0x78;
        text.y = 0x70;
        text.text[0] = 0x0f;
        text.text[1] = 0x0b;
        text.text[2] = 4;
        text.text[3] = 0;
        text.text[4] = 0x12;
        text.text[5] = 4;
        text.text[6] = 0x7f;
        text.text[7] = 0x16;
        text.text[8] = 0;
        text.text[9] = 8;
        text.text[10] = 0x13;
        text.text[11] = 0xff;
        volume = DAT_801b24e4.bgm_volume;
        for (i = 0x59; i != -1; i--) {
            if (volume > 0) {
                volume--;
            }
            menu_show_dialog_panel(7, &text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
            SsSeqSetVol(DAT_801b09fc.seq_id, volume, volume);
        }
    }
    return result;
}

ADDRESS(0x8001e710, 0x324)
s32 menu_card_load_browser(void)
{
    struct DIRENTRY dir[15];
    MenuList list;
    MenuEntry entries[8];
    u32 experience[8];
    u8 levels[8];
    s32 numbers[8];
    MenuText text[3];
    s32 status;
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    s32 error;

    confirmed = 0;
    result = MENU_NONE;
    if (DAT_8009c384 != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqPause(DAT_801b09fc.seq_id);
    }
    cd_request_wait_idle();
    menu_prepare_card_browser_rows(text);
    menu_show_dialog_panel(6, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    memory_card_start();
    for (i = 4; i != -1; i--) {
        memory_card_scan_save_entries(dir, &status);
        if (status != 0) {
            break;
        }
    }
    n = menu_card_build_slot_rows(dir, entries->text, experience, levels, numbers);
    menu_list_init(&list, 3, 0);
    list.rows = 5;
    list.y = 0x73;
    list.count = n;
    list.entries = entries;
    list.counts = levels;
    list.values = experience;
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 6, 8, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = numbers[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        menu_update_list_input(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            menu_play_sound_cue(0xc);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_render_list(&list, 8);
            menu_present_frame();
        }
    }
    if (result != -1) {
        menu_prepare_card_read_row(text);
        menu_show_dialog_panel(6, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
        error = memory_card_read_slot(result);
        if (error != 0) {
            menu_prepare_card_read_failure_rows(text, error);
            menu_show_dialog_panel(6, text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            input_wait_release();
            input_wait_press();
            input_wait_release();
            result = -1;
        } else {
            result = 0;
        }
    }
    memory_card_stop();
    if (DAT_8009c384 != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqReplay(DAT_801b09fc.seq_id);
    }
    return result;
}

ADDRESS(0x8001ea34, 0x108)
s32 menu_card_build_slot_rows(struct DIRENTRY *dir, u8 *p, u32 *experience, u8 *levels, s32 *numbers)
{
    s32 n = 0;
    s32 i = 0;
    s32 points;
    s32 level;
    s32 number;

    for (; i < 15; i++) {
        if (memory_card_read_slot_summary(dir->name, &points, &level, &number) == 0) {
            *p++ = 3;
            *p++ = 0;
            *p++ = 0x13;
            *p++ = 0;
            *p++ = number + 32;
            *p = 0xff;
            p += 19;
            n++;
            *experience++ = points;
            *levels++ = level;
            *numbers++ = number;
        }
        dir++;
    }
    return n;
}

ADDRESS(0x8001eb3c, 0x148)
void menu_show_dialog_panel(s32 message, MenuText *lines, s32 count, s32 x, s32 y, s32 w, s32 h, s32 back)
{
    s32 i;
    s32 j;
    MenuText *line;

    for (i = 0; i < 2; i++) {
        menu_frame_begin();
        if (message < 8) {
            menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], &DAT_8007e660[message + 28]);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &DAT_8007e660[message + 28]);
        }
        line = lines;
        for (j = 0; j < count; j++) {
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], line);
            line++;
        }
        menu_draw_nine_slice_panel(x, y, w, h);
        if (back == 1) {
            menu_draw_button_icon(0x80, 0xd6, DAT_8009c258);
            menu_draw_ascii_string(0x8a, 0xd6, "return", 0, 0);
        }
        menu_present_frame();
    }
}

ADDRESS(0x8001ec84, 0x380)
void menu_volume_brightness_controller(void)
{
    u8 levels[3];
    s32 row = 0;
    s32 result = MENU_NONE;
    s32 last = 2;
    u32 pad;
    s32 i;

    levels[0] = DAT_801b24e4.se_volume;
    if (DAT_801b24e4.bgm_volume == 0) {
        levels[1] = 0;
    } else {
        levels[1] = DAT_801b24e4.bgm_volume + 1;
    }
    levels[2] = DAT_801b24e4.unkfd + 192;
    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            break;
        }
        input_wait_brief_release();
        pad = input_read_mark_active();
        if (pad & PADLup) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xc);
            if (row == 0) {
                row = last;
            } else {
                row--;
            }
        } else if (pad & PADLdown) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xc);
            if (row == last) {
                row = 0;
            } else {
                row++;
            }
        } else if (pad & PADLright) {
            levels[row] += 3;
            if (levels[row] > 128) {
                levels[row] = 128;
            }
        } else if (pad & PADLleft) {
            if (levels[row] >= 3) {
                levels[row] -= 3;
            } else if (levels[row] != 0) {
                levels[row] = 0;
            }
        } else if (pad & DAT_8009c250) {
            menu_play_sound_cue(0xe);
            result = -1;
        }
        if ((pad & PADselect) || (pad & PADstart)) {
            DAT_8009c248 = 0;
            VSync(0);
        }
        if ((pad & PADLright) || (pad & PADLleft)) {
            if (row == 0) {
                DAT_801b24e4.se_volume = levels[0];
                audio_key_on(0xc, 0x80, 0x80, 0);
                DAT_801b24e4.se_volume = levels[0];
            } else if (row == 1) {
                if (DAT_8009c384 == 0) {
                    DAT_801b24e4.se_volume = levels[1];
                    audio_key_on(0xc, 0x80, 0x80, 0);
                    DAT_801b24e4.se_volume = levels[0];
                } else {
                    if (levels[1] == 0) {
                        i = 0;
                    } else if (levels[1] >= 128) {
                        i = 0x7f;
                    } else {
                        i = levels[1] - 1;
                    }
                    SsSeqSetVol(DAT_801b09fc.seq_id, i, i);
                }
            } else {
                menu_play_sound_cue(0xc);
            }
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(4, 3, row, 1);
            menu_draw_volume_brightness_sliders(levels);
            menu_present_frame();
        }
    }
    DAT_801b24e4.se_volume = levels[0];
    if (levels[1] == 0) {
        DAT_801b24e4.bgm_volume = levels[1];
    } else {
        DAT_801b24e4.bgm_volume = levels[1] - 1;
    }
    DAT_801b24e4.unkfd = levels[2] + 64;
    if (DAT_801b24e4.unk100 == 1) {
        DAT_8009c270 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c274 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c278 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
    }
}

ADDRESS(0x8001f004, 0x270)
void menu_display_options_controller(void)
{
    u8 options[5];
    s32 row = 0;
    s32 result = MENU_NONE;
    s32 last = 4;
    u32 pad;
    s32 i;

    options[0] = DAT_801b24e4.unkf9;
    options[1] = DAT_801b24e4.unkfa;
    options[2] = DAT_801b24e4.unkfb;
    options[3] = DAT_801b24e4.unkfc;
    options[4] = DAT_801b24e4.unk100;
    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            break;
        }
        input_wait_brief_release();
        pad = input_read_mark_active();
        if (pad & PADLup) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xc);
            if (row == 0) {
                row = last;
            } else {
                row--;
            }
        } else if (pad & PADLdown) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xc);
            if (row == last) {
                row = 0;
            } else {
                row++;
            }
        } else if ((pad & DAT_8009c24c) || (pad & PADLright) || (pad & PADLleft)) {
            menu_play_sound_cue(0xd);
            if (row >= 2) {
                options[row] = !options[row];
            } else if ((pad & DAT_8009c24c) || (pad & PADLright)) {
                if (options[row] == 0) {
                    options[row] = 2;
                } else {
                    options[row]--;
                }
            } else {
                if (options[row] == 2) {
                    options[row] = 0;
                } else {
                    options[row]++;
                }
            }
        } else if (pad & DAT_8009c250) {
            menu_play_sound_cue(0xe);
            result = -1;
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(5, 5, row, 1);
            menu_draw_options_rows(options);
            menu_present_frame();
        }
    }
    DAT_801b24e4.unkf9 = options[0];
    DAT_801b24e4.unkfa = options[1];
    DAT_801b24e4.unkfb = options[2];
    DAT_801b24e4.unkfc = options[3];
    DAT_801b24e4.unk100 = options[4];
    menu_apply_display_colors();
}

ADDRESS(0x8001f274, 0xa8)
void menu_apply_display_colors(void)
{
    s32 shade;

    if (DAT_801b24e4.unk100 == 0) {
        if (DAT_801b24e4.unkfd > 128) {
            shade = 0x30 - (DAT_801b24e4.unkfd - 128) / 2;
        } else {
            shade = 0x30;
        }
        DAT_8009c26c = 0x57;
        DAT_8009c270 = shade;
        DAT_8009c274 = shade;
        DAT_8009c278 = shade;
        DAT_8009c27c = 0x80;
    } else {
        DAT_8009c26c = 0x78;
        DAT_8009c27c = 0x30;
        DAT_8009c270 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c274 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
        DAT_8009c278 = (DAT_801b24e4.unkfd - 128) / 2 + 128;
    }
}

ADDRESS(0x8001f31c, 0x1a4)
void menu_button_config_controller(void)
{
    u8 style = DAT_801b24e4.unkfe;
    u8 layout = DAT_801b24e4.unkff;
    s32 row = 0;
    s32 result = MENU_NONE;
    u32 pad;
    s32 i;

    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            break;
        }
        input_wait_brief_release();
        pad = input_read_mark_active();
        if (pad & DAT_8009c250) {
            menu_play_sound_cue(0xe);
            result = -1;
        } else if ((pad & PADLup) || (pad & PADLdown)) {
            menu_play_sound_cue(0xc);
            row = row == 0;
        } else if (pad & PADLleft) {
            menu_play_sound_cue(0xc);
            if (row == 0) {
                if (style != 0) {
                    style--;
                }
            } else if (layout != 0) {
                layout--;
            }
        } else if (pad & PADLright) {
            menu_play_sound_cue(0xc);
            if (row == 0) {
                style++;
                if (style >= 4) {
                    style = 3;
                }
            } else {
                layout++;
                if (layout >= 7) {
                    layout = 6;
                }
            }
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_button_config(style, layout, row);
            menu_present_frame();
        }
    }
    DAT_801b24e4.unkfe = style;
    DAT_801b24e4.unkff = layout;
    menu_apply_button_config();
}

ADDRESS(0x8001f4c0, 0x3f4)
void menu_apply_button_config(void)
{
    switch (DAT_801b24e4.unkfe) {
    case 0:
        DAT_80081868[4] = PADRup;
        DAT_80081868[6] = PADRleft;
        DAT_80081868[7] = PADRright;
        DAT_80081868[5] = PADRdown;
        break;
    case 1:
        DAT_80081868[4] = PADRleft;
        DAT_80081868[6] = PADRup;
        DAT_80081868[7] = PADRright;
        DAT_80081868[5] = PADRdown;
        break;
    case 2:
        DAT_80081868[4] = PADRup;
        DAT_80081868[6] = PADRleft;
        DAT_80081868[7] = PADRdown;
        DAT_80081868[5] = PADRright;
        break;
    case 3:
        DAT_80081868[4] = PADRleft;
        DAT_80081868[6] = PADRup;
        DAT_80081868[7] = PADRdown;
        DAT_80081868[5] = PADRright;
        break;
    }
    switch (DAT_801b24e4.unkff) {
    case 0:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADL1;
        DAT_80081868[9] = PADL2;
        DAT_80081868[10] = PADR1;
        DAT_80081868[11] = PADR2;
        break;
    case 1:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADR1;
        DAT_80081868[9] = PADL1;
        DAT_80081868[10] = PADR2;
        DAT_80081868[11] = PADL2;
        break;
    case 2:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADL1;
        DAT_80081868[9] = PADR1;
        DAT_80081868[10] = PADL2;
        DAT_80081868[11] = PADR2;
        break;
    case 3:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADL1;
        DAT_80081868[3] = PADR1;
        DAT_80081868[8] = PADLleft;
        DAT_80081868[9] = PADL2;
        DAT_80081868[10] = PADLright;
        DAT_80081868[11] = PADR2;
        break;
    case 4:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADR1;
        DAT_80081868[3] = PADR2;
        DAT_80081868[8] = PADLleft;
        DAT_80081868[9] = PADL1;
        DAT_80081868[10] = PADLright;
        DAT_80081868[11] = PADL2;
        break;
    case 5:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADL1;
        DAT_80081868[3] = PADL2;
        DAT_80081868[8] = PADLleft;
        DAT_80081868[9] = PADR1;
        DAT_80081868[10] = PADLright;
        DAT_80081868[11] = PADR2;
        break;
    case 6:
        DAT_80081868[0] = PADLup;
        DAT_80081868[1] = PADLdown;
        DAT_80081868[2] = PADLleft;
        DAT_80081868[3] = PADLright;
        DAT_80081868[8] = PADL2;
        DAT_80081868[9] = PADR1;
        DAT_80081868[10] = PADR2;
        DAT_80081868[11] = PADL1;
        break;
    }
    if (DAT_801b24e4.unkfe < 2) {
        DAT_8009c24c = PADRright;
        DAT_8009c250 = PADRdown;
        DAT_8009c254 = 1;
        DAT_8009c258 = 3;
    } else {
        DAT_8009c24c = PADRdown;
        DAT_8009c250 = PADRright;
        DAT_8009c254 = 3;
        DAT_8009c258 = 1;
    }
}

ADDRESS(0x8001f8b4, 0x1ac)
s32 menu_prompt_quit_game(void)
{
    s32 choice = 1;
    s32 result = MENU_NONE;
    u8 *mailbox = MAILBOX_BYTE_8;
    MenuText labels[2];
    u32 pad;
    s32 i;

    labels[0].x = 0xab;
    labels[0].y = 0xa2;
    labels[0].text[0] = 0x18;
    labels[0].text[1] = 4;
    labels[0].text[2] = 0x12;
    labels[0].text[3] = 0xff;
    labels[1].x = 0xe5;
    labels[1].y = 0xa2;
    labels[1].text[0] = 0x0d;
    labels[1].text[1] = 0x0e;
    labels[1].text[2] = 0xff;
    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            return result;
        }
        input_wait_brief_release();
        pad = input_read_mark_active();
        if ((pad & PADLright) || (pad & PADLleft) || (pad & PADLup) || (pad & PADLdown)) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xc);
            choice = choice == 0;
        } else if (pad & DAT_8009c24c) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xd);
            if (choice == 0) {
                *mailbox = 1;
                result = 0;
            } else {
                result = -1;
            }
        } else if (pad & DAT_8009c250) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xe);
            result = -1;
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(3, 5, 4, 0);
            menu_draw_two_option(&labels[0], &labels[1], choice);
            menu_present_frame();
        }
    }
}

ADDRESS(0x8001fa60, 0x1fc)
s32 menu_card_browser(void)
{
    s32 result = -1;
    u8 *mailbox = MAILBOX_BYTE_A;
    struct DIRENTRY dir[15];
    MenuText text[4];
    s32 status;
    s32 i;
    s32 found;

    menu_enter_display_state();
    menu_prepare_card_browser_rows(text);
    menu_show_dialog_panel(9, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    cd_request_wait_idle();
    memory_card_start();
    for (i = 9; i != -1; i--) {
        found = memory_card_probe_temporary_file();
        if (found == 0) {
            break;
        }
    }
    if (found != 0 && found != 2) {
        menu_build_card_probe_error_rows(text);
        menu_show_dialog_panel(9, text, 2, 0x3c, 0x57, 0xc6, 0x42, 1);
    } else {
        found = memory_card_scan_save_entries(dir, &status);
        if (found == 1 && status == 0) {
            found = memory_card_scan_save_entries(dir, &status);
        }
        if (status != 0) {
            goto loaded;
        }
        if (found != 1) {
            goto done;
        }
        menu_build_card_full_rows(text);
        menu_show_dialog_panel(9, text, 2, 0x46, 0x57, 0xb2, 0x42, found);
    }
    input_wait_release();
    input_wait_press();
    input_wait_release();
done:
    memory_card_stop();
    menu_exit_display_state(0);
    return -1;
loaded:
    if (*mailbox == 1) {
        menu_play_sound_cue(0xc);
        input_wait_release();
        result = menu_card_load_slot_browser(dir);
    }
    memory_card_stop();
    menu_exit_display_state(0);
    return result;
}

ADDRESS(0x8001fc5c, 0x240)
s32 menu_card_load_slot_browser(struct DIRENTRY *dir)
{
    MenuList list;
    MenuEntry entries[8];
    u32 experience[8];
    u8 levels[8];
    s32 numbers[8];
    MenuText text[3];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    s32 error;

    result = MENU_NONE;
    confirmed = 0;
    n = menu_card_build_slot_rows(dir, entries->text, experience, levels, numbers);
    menu_list_init(&list, 3, 6);
    list.rows = 5;
    list.y = 0x73;
    list.count = n;
    list.entries = entries;
    list.counts = levels;
    list.values = experience;
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 6, 8, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = numbers[list.cursor];
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        menu_update_list_input(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            menu_play_sound_cue(0xc);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_render_list(&list, 8);
            menu_present_frame();
        }
    }
    if (result != -1) {
        menu_prepare_card_read_row(text);
        menu_show_dialog_panel(9, text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
        error = memory_card_read_slot(result);
        if (error != 0) {
            menu_prepare_card_read_failure_rows(text, error);
            menu_show_dialog_panel(9, text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            input_wait_release();
            input_wait_press();
            input_wait_release();
        }
    }
    return result;
}

ADDRESS(0x8001fe9c, 0xf8)
void menu_build_card_probe_error_rows(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[25], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[26], sizeof(MenuEntry));
}

ADDRESS(0x8001ff94, 0xf8)
void menu_build_card_full_rows(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[28], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[29], sizeof(MenuEntry));
}

ADDRESS(0x8002008c, 0x33c)
void menu_card_save_browser(void)
{
    struct DIRENTRY dir[15];
    MenuList list;
    MenuEntry entries[8];
    u32 experience[8];
    u8 levels[8];
    s32 numbers[8];
    MenuText text[2];
    s32 status;
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    s32 found;

    confirmed = 0;
    result = MENU_NONE;
    menu_enter_display_state();
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqPause(DAT_801b09fc.seq_id);
    }
    cd_request_wait_idle();
    menu_prepare_card_browser_rows(text);
    menu_draw_card_dialog_rows(text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    memory_card_start();
    for (i = 9; i != -1; i--) {
        if (memory_card_probe_temporary_file() == 0) {
            break;
        }
    }
    found = memory_card_scan_save_entries(dir, &status);
    if (found == 1 && status == 0) {
        found = memory_card_scan_save_entries(dir, &status);
    }
    n = menu_card_build_slot_rows(dir, entries->text, experience, levels, numbers);
    if (found == 0 || status == 0) {
        entries[n].text[0] = 0x0d;
        entries[n].text[1] = 4;
        entries[n].text[2] = 0x16;
        entries[n].text[3] = 0xff;
        experience[n] = -1;
        levels[n] = 0xff;
        numbers[n] = 0xff;
        n++;
    }
    menu_list_init(&list, 3, 5);
    list.rows = 5;
    list.y = 0x73;
    list.entries = entries;
    list.counts = levels;
    list.count = n;
    list.values = experience;
    menu_play_sound_cue(0xc);
    input_wait_release();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 7, 9, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = list.cursor;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        menu_update_list_input(&list, 0, &confirmed, &result, 0);
        if (confirmed == 1) {
            menu_play_sound_cue(0xc);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_render_list(&list, 9);
            menu_present_frame();
        }
    }
    if (result != -1) {
        menu_card_save_slot(numbers[result]);
    }
    memory_card_stop();
    menu_exit_display_state(0);
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
        SsSeqReplay(DAT_801b09fc.seq_id);
    }
}

ADDRESS(0x800203c8, 0x198)
void menu_card_save_slot(s32 number)
{
    MenuText text[4];
    s32 status;

    status = memory_card_probe_temporary_file();
    if (status != 0) {
        if (status != 2) {
            if (status == 1) {
                menu_prepare_card_not_inserted_rows(text);
            } else {
                menu_prepare_card_io_error_rows(text);
            }
            menu_draw_card_dialog_rows(text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            input_wait_release();
            input_wait_press();
            input_wait_release();
            return;
        }
        if (menu_confirm_card_format() != 0) {
            menu_prepare_card_format_declined_rows(text);
            menu_draw_card_dialog_rows(text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
            input_wait_release();
            input_wait_press();
            input_wait_release();
            return;
        }
        memory_card_format();
    }
    menu_prepare_card_write_rows(text);
    menu_draw_card_dialog_rows(text, 1, 0x46, 0x57, 0xb2, 0x42, 0);
    status = memory_card_write_slot(number);
    if (status == 0) {
        return;
    }
    if (status == 1) {
        menu_prepare_card_io_error_rows(text);
        menu_draw_card_dialog_rows(text, 2, 0x46, 0x57, 0xb2, 0x42, 1);
    } else {
        menu_prepare_card_write_full_rows(text);
        menu_draw_card_dialog_rows(text, 2, 0x3c, 0x57, 0xc6, 0x42, 1);
    }
    input_wait_release();
    input_wait_press();
    input_wait_release();
}

ADDRESS(0x80020560, 0x3ac)
s32 menu_confirm_card_format(void)
{
    s32 selected = 1;
    MenuText labels[2];
    MenuText lines[5];
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    chosen = -1;
    confirmed = 0;
    labels[0].text[0] = 0x18;
    labels[0].text[1] = 4;
    labels[0].text[2] = 0x12;
    labels[0].text[3] = 0xff;
    labels[1].text[0] = 0x0d;
    labels[1].text[1] = 0x0e;
    labels[1].text[2] = 0xff;
    lines[0].x = 0x4c;
    labels[0].x = DAT_8007e660[10].x;
    labels[0].y = DAT_8007e660[10].y;
    labels[1].x = DAT_8007e660[11].x;
    labels[1].y = DAT_8007e660[11].y;
    lines[0].y = 0x7a;
    memcpy((void *)lines[0].text, (const void *)&DAT_8007f230[36], sizeof(MenuEntry));
    lines[1].x = 0x4c;
    lines[1].y = 0x89;
    memcpy((void *)lines[1].text, (const void *)&DAT_8007f230[30], sizeof(MenuEntry));
    lines[2].x = 0x4c;
    lines[2].y = 0x98;
    memcpy((void *)lines[2].text, (const void *)&DAT_8007f230[31], sizeof(MenuEntry));
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            input_wait_release();
        }
        switch (chosen) {
        case 0:
            result = 0;
            break;
        case 1:
            result = -1;
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = menu_poll_choice_input(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_two_option(&labels[0], &labels[1], selected);
            menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], &DAT_8007e660[33]);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &DAT_8007e660[33]);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &lines[0]);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &lines[1]);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &lines[2]);
            menu_draw_nine_slice_panel(0x32, 0x68, 0xdc, 0x51);
            menu_draw_button_icon(0x5f, 0xd6, DAT_8009c254);
            menu_draw_ascii_string(0x69, 0xd6, "select", 0, 0);
            menu_draw_button_icon(0xa0, 0xd6, DAT_8009c258);
            menu_draw_ascii_string(0xaa, 0xd6, "return", 0, 0);
            menu_present_frame();
        }
    }
    return result;
}

ADDRESS(0x8002090c, 0x80)
void menu_prepare_card_browser_rows(MenuText *line)
{
    line->x = 0x4c;
    line->y = 0x70;
    memcpy((void *)line->text, (const void *)&DAT_8007f230[24], sizeof(MenuEntry));
}

ADDRESS(0x8002098c, 0xf8)
void menu_prepare_card_not_inserted_rows(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[36], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[38], sizeof(MenuEntry));
}

ADDRESS(0x80020a84, 0xf8)
void menu_prepare_card_io_error_rows(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[33], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[27], sizeof(MenuEntry));
}

ADDRESS(0x80020b7c, 0xf8)
void menu_prepare_card_format_declined_rows(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[32], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[39], sizeof(MenuEntry));
}

ADDRESS(0x80020c74, 0xf8)
void menu_prepare_card_write_full_rows(MenuText *lines)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[40], sizeof(MenuEntry));
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[41], sizeof(MenuEntry));
}

ADDRESS(0x80020d6c, 0x80)
void menu_prepare_card_write_rows(MenuText *line)
{
    line->x = 0x4c;
    line->y = 0x70;
    memcpy((void *)line->text, (const void *)&DAT_8007f230[10], sizeof(MenuEntry));
}

ADDRESS(0x80020dec, 0x80)
void menu_prepare_card_read_row(MenuText *line)
{
    line->x = 0x4c;
    line->y = 0x70;
    memcpy((void *)line->text, (const void *)&DAT_8007f230[37], sizeof(MenuEntry));
}

ADDRESS(0x80020e6c, 0x17c)
void menu_prepare_card_read_failure_rows(MenuText *lines, s32 error)
{
    lines->x = 0x4c;
    lines->y = 0x69;
    if (error == 1) {
        memcpy((void *)lines->text, (const void *)&DAT_8007f230[34], sizeof(MenuEntry));
    } else {
        memcpy((void *)lines->text, (const void *)&DAT_8007f230[35], sizeof(MenuEntry));
    }
    lines++;
    lines->x = 0x4c;
    lines->y = 0x78;
    memcpy((void *)lines->text, (const void *)&DAT_8007f230[27], sizeof(MenuEntry));
}

ADDRESS(0x80020fe8, 0x12c)
void menu_draw_card_dialog_rows(MenuText *lines, s32 count, s32 x, s32 y, s32 w, s32 h, s32 back)
{
    s32 i;
    s32 j;
    MenuText *line;

    for (i = 0; i < 2; i++) {
        menu_frame_begin();
        line = lines;
        menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], &DAT_8007e660[33]);
        menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &DAT_8007e660[33]);
        for (j = 0; j < count; j++) {
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], line);
            line++;
        }
        menu_draw_nine_slice_panel(x, y, w, h);
        if (back == 1) {
            menu_draw_button_icon(0x80, 0xd6, DAT_8009c258);
            menu_draw_ascii_string(0x8a, 0xd6, "return", 0, 0);
        }
        menu_present_frame();
    }
}

ADDRESS(0x80021114, 0x184)
void menu_item_buy_sell_controller(s32 shop)
{
    s32 selected = 0;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    menu_enter_display_state();
    for (i = 0; i < 2; i++) {
        menu_frame_begin();
        menu_draw_window(6, 2, selected, 0);
        menu_present_frame();
    }
    menu_play_sound_cue(0xc);
    input_wait_release();
    menu_cd_install_data_ready_handler();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            input_wait_release();
        }
        switch (chosen) {
        case 0:
            menu_item_buy_controller(shop);
            break;
        case 1:
            menu_item_sell_controller(shop);
            break;
        }
        if (result != MENU_NONE) {
            break;
        }
        selected = menu_poll_choice_input(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(6, 2, selected, 0);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
    menu_exit_display_state(0);
}

ADDRESS(0x80021298, 0x40c)
void menu_item_buy_controller(s32 shop)
{
    u32 pad = 0;
    MenuList list;
    MenuEntry entries[152];
    u8 counts[152];
    u32 prices[152];
    u8 ids[152];
    s32 confirmed = 0;
    s32 result = MENU_NONE;
    s32 mode;
    u8 cursors[5] = {0, 0xb, 7, 2, 0};
    s32 kind;
    u8 *bag = DAT_800c85e8[0];
    s32 n;
    s32 i;
    u8 item;

    kind = 0xa;
    if (shop == 0) {
        kind = 0xd;
    }
    mode = shop != 0;
    n = menu_collect_masked_item_rows(DAT_80080718[shop], entries, counts, ids, 0, 0x95);
    menu_fill_item_counts_and_prices(bag, counts, prices, ids, 0, n, shop);
    menu_list_init(&list, 6, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    list.values = prices;
    menu_list_set_cursor(&list, cursors[shop]);
    menu_request_item_model(ids[list.cursor]);
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 3, kind, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            menu_show_item_guide(&list, 2, item);
        }
        pad = menu_update_list_input(&list, ids, &confirmed, &result, mode);
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (shop == 0) {
                if ((u32)DAT_801b24e4.unk50 < list.values[list.cursor] || bag[item] + 1 >= 100) {
                    goto refuse;
                }
            } else if ((u32)DAT_801b24e4.unk50 < list.values[list.cursor] * DAT_8009c25c
                       || bag[item] + DAT_8009c25c >= 100) {
            refuse:
                menu_play_sound_cue(0xe);
                confirmed = 0;
                goto draw;
            }
            menu_play_sound_cue(0xd);
        }
    draw:
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, kind);
            menu_present_frame();
        }
    }
    if (result != -1) {
        if (shop == 0) {
            DAT_8009c25c = 1;
        }
        DAT_801b24e4.unk50 -= list.values[list.cursor] * DAT_8009c25c;
        bag[result] += DAT_8009c25c;
        if (shop == 0 && result != 0x6b) {
            DAT_80080718[0][result] -= 1;
        }
    }
}

ADDRESS(0x800216a4, 0x80)
void menu_fill_item_counts_and_prices(u8 *bag, u8 *counts, u32 *prices, u8 *ids, s32 first, s32 end, s32 shop)
{
    for (; first < end; first++) {
        *counts++ = bag[*ids];
        *prices++ = DAT_80080a9c[shop][*ids];
        ids++;
    }
}

ADDRESS(0x80021724, 0x328)
void menu_item_sell_controller(s32 shop)
{
    static u8 cursor = 0;
    MenuList list;
    MenuEntry entries[152];
    u8 counts[152];
    u32 prices[152];
    u8 ids[152];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u32 pad;
    u8 item;

    result = MENU_NONE;
    confirmed = 0;
    n = menu_collect_available_item_rows(DAT_800c85e8[0], entries, counts, ids, 0, 0x95);
    menu_fill_item_prices(prices, ids, 0, n, shop);
    menu_list_init(&list, 6, 1);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    list.values = prices;
    menu_list_set_cursor(&list, cursor);
    pad = 0;
    if (list.count != 0) {
        menu_request_item_model(ids[list.cursor]);
    }
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = menu_preview_choice(&list, 4, 0xb, item);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        if (confirmed != 1 && !(pad & PADselect) && !(pad & PADstart) && (pad & PADRup)) {
            menu_show_item_guide(&list, 0, item);
        }
        pad = menu_update_list_input(&list, ids, &confirmed, &result, 2);
        cursor = list.cursor;
        item = ids[list.cursor];
        if (confirmed == 1) {
            if (counts[list.cursor] < DAT_8009c25c) {
                menu_play_sound_cue(0xe);
                confirmed = 0;
            } else {
                menu_play_sound_cue(0xd);
            }
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, 0xb);
            menu_present_frame();
        }
    }
    if (result != -1) {
        DAT_801b24e4.unk50 += list.values[list.cursor] * DAT_8009c25c;
        DAT_800c85e8[0][result] -= DAT_8009c25c;
        DAT_80080718[0][result] += DAT_8009c25c;
        if (DAT_80080718[0][result] >= 100) {
            DAT_80080718[0][result] = 99;
        }
    }
}

ADDRESS(0x80021a4c, 0x60)
void menu_fill_item_prices(u32 *prices, u8 *ids, s32 first, s32 end, s32 shop)
{
    for (; first < end; first++) {
        *prices++ = DAT_800811a4[shop][*ids];
        ids++;
    }
}

ADDRESS(0x80021aac, 0x2e8)
s32 menu_intellectual_choose_item(void)
{
    MenuList list;
    MenuEntry entries[60];
    u8 counts[64];
    u8 ids[64];
    s32 confirmed;
    s32 result;
    s32 n;
    s32 i;
    u8 item;

    confirmed = 0;
    result = MENU_NONE;
    menu_enter_display_state();
    n = menu_collect_guide_locked_item_rows(DAT_800c85e8[0], entries, counts, ids, 0, 0x95);
    menu_list_init(&list, 8, 0);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    menu_cd_install_data_ready_handler();
    item = ids[list.cursor];
    if (list.count != 0) {
        menu_request_item_model(item);
    }
    for (i = 0; i < 2; i++) {
        menu_frame_begin();
        if (list.count != 0) {
            menu_update_item_preview(item);
        }
        menu_render_list(&list, 0xc);
        menu_present_frame();
    }
    menu_play_sound_cue(0xc);
    input_wait_release();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            result = item;
        }
        if (result != MENU_NONE) {
            break;
        }
        menu_update_list_input(&list, ids, &confirmed, &result, 0);
        item = ids[list.cursor];
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list.count != 0) {
                menu_update_item_preview(item);
            }
            menu_render_list(&list, 0xc);
            menu_present_frame();
        }
    }
    menu_cd_restore_data_ready_handler();
    if (result != -1) {
        DAT_80081780[result] = 4;
    }
    switch (result) {
    case 2:
        result = 300;
        break;
    case 8:
        result = 301;
        break;
    case 10:
        result = 302;
        break;
    case 32:
        result = 303;
        break;
    case 33:
        result = 304;
        break;
    case 38:
        result = 305;
        break;
    case 39:
        result = 306;
        break;
    case 41:
        result = 307;
        break;
    case 56:
        result = 308;
        break;
    case 58:
        result = 309;
        break;
    case 82:
        result = 310;
        break;
    case 89:
        result = 311;
        break;
    case 90:
        result = 312;
        break;
    case 91:
        result = 313;
        break;
    case 92:
        result = 314;
        break;
    case 126:
        result = 315;
        break;
    case 134:
        result = 316;
        break;
    case 140:
        result = 317;
        break;
    case 143:
        result = 318;
        break;
    }
    menu_exit_display_state(0);
    return result;
}

ADDRESS(0x80021d94, 0x278)
s32 menu_blacksmith_make_item(void)
{
    MenuList list;
    MenuEntry entries[20];
    u8 counts[24];
    u8 ids[24];
    s32 confirmed;
    s32 result;
    u8 saved;
    u8 item;
    s32 n;
    s32 i;

    confirmed = 0;
    result = MENU_NONE;
    menu_enter_display_state();
    saved = DAT_801b24e4.unkfb;
    DAT_801b24e4.unkfb = 0;
    n = menu_collect_masked_item_rows(DAT_80080718[5], entries, counts, ids, 0, 0x95);
    for (i = 0; i < 5; i++) {
        counts[i] = DAT_800c85e8[0][ids[i]] + DAT_800c85e8[1][ids[i]];
    }
    menu_list_init(&list, 8, 1);
    list.count = n;
    list.entries = entries;
    list.counts = counts;
    for (i = 0; i < 2; i++) {
        menu_frame_begin();
        menu_render_list(&list, 0xc);
        menu_present_frame();
    }
    menu_play_sound_cue(0xc);
    input_wait_release();
    for (;;) {
        if (confirmed != 0 || result != MENU_NONE) {
            input_wait_release();
        }
        if (confirmed == 1) {
            menu_play_sound_cue(0xd);
            result = menu_preview_choice(&list, 9, 0xc, 0xff);
            if (result == -1) {
                result = MENU_NONE;
            } else {
                result = item;
            }
        }
        if (result != MENU_NONE) {
            break;
        }
        menu_update_list_input(&list, 0, &confirmed, &result, 0);
        item = ids[list.cursor];
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_render_list(&list, 0xc);
            menu_present_frame();
        }
    }
    if (result != -1) {
        if (DAT_800c85e8[0][0x7c] != 0) {
            DAT_800c85e8[0][0x7c]--;
        } else if (DAT_800c85e8[1][0x7c] != 0) {
            DAT_800c85e8[1][0x7c]--;
        }
    }
    DAT_801b24e4.unkfb = saved;
    menu_exit_display_state(0);
    return result;
}

ADDRESS(0x8002200c, 0x1dc)
s32 menu_prompt_stay(s32 mode)
{
    s32 selected = 0;
    s32 paid;
    s32 chosen;
    s32 confirmed;
    s32 result;
    s32 i;

    result = MENU_NONE;
    confirmed = 0;
    chosen = -1;
    paid = -1;
    menu_enter_display_state();
    mode &= 0xf;
    for (i = 0; i < 2; i++) {
        menu_frame_begin();
        menu_draw_window(7, 2, selected, 0);
        if (mode == 0) {
            menu_draw_gold();
        }
        menu_present_frame();
    }
    menu_play_sound_cue(0xc);
    input_wait_release();
    for (;;) {
        if (chosen != -1 || result != MENU_NONE) {
            input_wait_release();
        }
        if (chosen == 0) {
            if (mode == 0) {
                if ((u32)DAT_801b24e4.unk50 >= 150) {
                    DAT_801b24e4.unk50 -= 150;
                    paid = 0;
                    break;
                }
                menu_play_sound_cue(0xe);
                chosen = -1;
                result = MENU_NONE;
            } else {
                paid = 0;
                break;
            }
        }
        if (chosen == 1 || result != MENU_NONE) {
            break;
        }
        selected = menu_poll_choice_input(selected, 1, &chosen, &confirmed, &result);
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_draw_window(7, 2, selected, 0);
            if (mode == 0) {
                menu_draw_gold();
            }
            menu_present_frame();
        }
    }
    menu_exit_display_state(0);
    return paid;
}

ADDRESS(0x800221e8, 0x114)
s32 menu_poll_choice_input(s32 selected, s32 last, s32 *chosen, s32 *confirmed, s32 *result)
{
    u32 pad;

    *chosen = -1;
    *confirmed = 0;
    input_wait_brief_release();
    pad = input_read_mark_active();
    if (pad & PADLup) {
        DAT_8009c268 = 0;
        menu_play_sound_cue(0xc);
        if (selected == 0) {
            selected = last;
        } else {
            selected--;
        }
    } else if (pad & PADLdown) {
        DAT_8009c268 = 0;
        menu_play_sound_cue(0xc);
        if (selected == last) {
            selected = 0;
        } else {
            selected++;
        }
    } else if (pad & DAT_8009c24c) {
        menu_play_sound_cue(0xd);
        *confirmed = 1;
        *chosen = selected;
    } else if (pad & DAT_8009c250) {
        menu_play_sound_cue(0xe);
        *result = -1;
    }
    return selected;
}

ADDRESS(0x800222fc, 0x4f0)
u32 menu_update_list_input(MenuList *list, u8 *ids, s32 *confirmed, s32 *result, s32 mode)
{
    u32 price;
    u8 count;
    u32 pad;
    s32 next;

    if (mode == 1) {
        price = list->values[list->cursor];
    }
    if (mode != 0) {
        count = list->counts[list->cursor];
    }
    *confirmed = 0;
    input_wait_brief_release();
    pad = input_read_mark_active();
    if (list->count == 0) {
        if (pad != 0) {
            menu_play_sound_cue(0xe);
            *result = -1;
        }
    } else if (pad & PADLup) {
        menu_play_sound_cue(0xc);
        if (list->cursor != 0) {
            list->cursor--;
            if (list->row == 0) {
                list->first--;
            } else {
                list->row--;
            }
        } else {
            list->cursor = list->count - 1;
            if (list->count < list->rows) {
                list->first = 0;
                list->row = list->count - 1;
            } else {
                list->first = list->count - list->rows;
                list->row = list->rows - 1;
            }
        }
        if (ids != 0) {
            menu_request_item_model(ids[list->cursor]);
        }
    } else if (pad & PADLdown) {
        menu_play_sound_cue(0xc);
        if (list->cursor < list->count - 1) {
            list->cursor++;
            if (list->row == list->rows - 1) {
                list->first++;
            } else {
                list->row++;
            }
        } else {
            list->cursor = 0;
            list->first = 0;
            list->row = 0;
        }
        if (ids != 0) {
            menu_request_item_model(ids[list->cursor]);
        }
    } else if (pad & PADLright) {
        if (mode == 1) {
            if ((u32)DAT_801b24e4.unk50 < (DAT_8009c25c + 1) * price
                || (next = count + 1) + DAT_8009c25c >= 100) {
                goto refuse;
            }
        } else if (mode == 2) {
            if (count < DAT_8009c25c + 1) {
                goto refuse;
            }
        } else {
            goto debug;
        }
        menu_play_sound_cue(0xc);
        DAT_8009c25c++;
        goto debug;
    refuse:
        menu_play_sound_cue(0xe);
    } else if (pad & PADLleft) {
        if (mode != 0 && DAT_8009c25c >= 2) {
            menu_play_sound_cue(0xc);
            DAT_8009c25c--;
        }
    } else if (pad & DAT_8009c24c) {
        *confirmed = 1;
    } else if (pad & DAT_8009c250) {
        menu_play_sound_cue(0xe);
        *result = -1;
    }
debug:
    if (pad & PADselect) {
        if (pad & PADR1) {
            DAT_8009c39c.vx += 32;
        }
        if (pad & PADR2) {
            DAT_8009c39c.vx -= 32;
        }
        if (pad & PADL1) {
            DAT_8009c39c.vz += 32;
        }
        if (pad & PADL2) {
            DAT_8009c39c.vz -= 32;
        }
        if (pad & PADRup) {
            DAT_8009c3a4++;
        }
        if (pad & PADRleft) {
            DAT_8009c3a4--;
        }
    }
    if (pad & PADstart) {
        if (pad & PADR1) {
            DAT_8009c394.vx += 32;
        }
        if (pad & PADR2) {
            DAT_8009c394.vx -= 32;
        }
        if (pad & PADL1) {
            DAT_8009c394.vy += 32;
        }
        if (pad & PADL2) {
            DAT_8009c394.vy -= 32;
        }
        if (pad & PADRup) {
            DAT_8009c394.vz += 32;
        }
        if ((pad & PADRleft) && DAT_8009c394.vz > 640) {
            DAT_8009c394.vz -= 32;
        }
    }
    if ((pad & PADselect) || (pad & PADstart)) {
        DAT_8009c248 = 0;
        VSync(2);
    }
    return pad;
}

ADDRESS(0x800227ec, 0xb14)
void menu_draw_player_status(void)
{
    MenuText label;
    MenuText number;
    s32 dy;

    label.x = 0xa3;
    label.y = 0x24;
    memcpy((void *)label.text, (const void *)&DAT_8007f230[12], 20);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.experience, 7, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.y += 17;
    memcpy((void *)label.text, (const void *)&DAT_8007f230[13], 20);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y += 17;
    menu_format_number(DAT_801b24e4.level, 7, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 7;
    label.text[1] = 0xf;
    label.text[2] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 63;
    number.y += 17;
    menu_format_number(DAT_801b24e4.hp, 4, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.text[0] = 0x14;
    number.text[1] = 0xff;
    number.x += 28;
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.x += 7;
    menu_format_number(DAT_801b24e4.hp_max, 4, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 0xc;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 63;
    number.y += 17;
    menu_format_number(DAT_801b24e4.unk1c, 4, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.text[0] = 0x14;
    number.text[1] = 0xff;
    number.x += 28;
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.x += 7;
    menu_format_number(DAT_801b24e4.unk1a, 4, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 0xf;
    label.text[1] = 0x16;
    label.text[2] = 0x11;
    label.text[3] = 0x3b;
    label.text[4] = 0x16;
    label.text[5] = 8;
    label.text[6] = 0x12;
    label.text[7] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 70;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk40, 5, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    number.x = label.x + 98;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk4c + 1, 4, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[0] = 0x3b;
    label.text[1] = 0xff;
    label.x += 105;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.x = 0xa3;
    label.text[0] = 0xc;
    label.text[1] = 0;
    label.text[2] = 6;
    label.text[3] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.x += 35;
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7d04;
    primitive_set_poly_ft4_rect(label.x, label.y - 1, 14, 14, 0, 0xc0, 14, 14);
    primitive_buffer_commit_poly_ft4(0x1e);
    number.x = label.x;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk42[4], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x += 49;
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7d44;
    primitive_set_poly_ft4_rect(label.x, label.y - 1, 14, 14, 0x10, 0xc0, 14, 14);
    primitive_buffer_commit_poly_ft4(0x1e);
    number.x = label.x;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk42[0], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa3;
    label.y += 15;
    label.x += 35;
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7d84;
    primitive_set_poly_ft4_rect(label.x, label.y - 1, 14, 14, 0x20, 0xc0, 14, 14);
    primitive_buffer_commit_poly_ft4(0x1e);
    number.x = label.x;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk42[1], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x += 49;
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7dc4;
    primitive_set_poly_ft4_rect(label.x, label.y - 1, 14, 14, 0x30, 0xc0, 14, 14);
    primitive_buffer_commit_poly_ft4(0x1e);
    number.x = label.x;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk42[3], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa3;
    label.y += 15;
    label.x += 35;
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, 0x80, 0x80, 0x80);
    DAT_8009c374->tpage = 0x1d;
    DAT_8009c374->clut = 0x7e04;
    primitive_set_poly_ft4_rect(label.x, label.y - 1, 14, 14, 0x40, 0xc0, 14, 14);
    primitive_buffer_commit_poly_ft4(0x1e);
    number.x = label.x;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk42[2], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa3;
    label.y += 17;
    memcpy((void *)label.text, (const void *)&DAT_8007f230[16], 12);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    dy = 17;
    number.x = label.x + 56;
    number.y += 17;
    if (DAT_801b24e4.unk84 != 0) {
        memcpy((void *)number.text, (const void *)&DAT_8007f230[17], 12);
    } else if (DAT_801b24e4.unk7a != 0) {
        memcpy((void *)number.text, (const void *)&DAT_8007f230[18], 12);
    } else if (DAT_801b24e4.unk82 != 0) {
        memcpy((void *)number.text, (const void *)&DAT_8007f230[19], 12);
    } else if (DAT_801b24e4.unk78 != 0) {
        memcpy((void *)number.text, (const void *)&DAT_8007f230[20], 12);
    } else if (DAT_801b24e4.unk7e != 0) {
        memcpy((void *)number.text, (const void *)&DAT_8007f230[21], 12);
    } else {
        memcpy((void *)number.text, (const void *)&DAT_8007f230[22], 12);
    }
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &number);
    label.y += dy;
    memcpy((void *)label.text, (const void *)&DAT_8007f230[23], 10);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y += dy;
    menu_format_number(DAT_801b24e4.unk50, 7, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    menu_draw_nine_slice_panel(0x9b, 0x1a, 0x8d, 0xb4);
}

ADDRESS(0x80023300, 0x9e8)
void menu_draw_combat_attributes(void)
{
    MenuText label;
    MenuText number;

    label.x = 0x1e;
    label.y = 0x22;
    memcpy((void *)label.text, (const void *)&DAT_8007f230[14], 10);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.text[0] = 0x7f;
    label.text[1] = 0x7f;
    label.text[2] = 0x12;
    label.text[3] = 0xb;
    label.text[4] = 0;
    label.text[5] = 0x12;
    label.text[6] = 0x7;
    label.text[7] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk54[0], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x1;
    label.text[3] = 0xb;
    label.text[4] = 0xe;
    label.text[5] = 0x16;
    label.text[6] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk54[1], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x12;
    label.text[3] = 0x13;
    label.text[4] = 0;
    label.text[5] = 0x1;
    label.text[6] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk54[2], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x7;
    label.text[3] = 0xe;
    label.text[4] = 0xb;
    label.text[5] = 0x18;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk54[3], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x5;
    label.text[3] = 0x8;
    label.text[4] = 0x11;
    label.text[5] = 0x4;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk54[4], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x4;
    label.text[3] = 0;
    label.text[4] = 0x11;
    label.text[5] = 0x13;
    label.text[6] = 0x7;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk54[5], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0x8;
    label.text[4] = 0xd;
    label.text[5] = 0x3;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk54[6], 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0;
    label.text[4] = 0x13;
    label.text[5] = 0x4;
    label.text[6] = 0x11;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 16;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk62, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.x = 0xa4;
    label.y = 0x22;
    memcpy((void *)label.text, (const void *)&DAT_8007f230[15], 12);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    label.text[0] = 0x7f;
    label.text[1] = 0x7f;
    label.text[2] = 0x12;
    label.text[3] = 0xb;
    label.text[4] = 0;
    label.text[5] = 0x12;
    label.text[6] = 0x7;
    label.text[7] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 77;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk66, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x1;
    label.text[3] = 0xb;
    label.text[4] = 0xe;
    label.text[5] = 0x16;
    label.text[6] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk68, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x12;
    label.text[3] = 0x13;
    label.text[4] = 0;
    label.text[5] = 0x1;
    label.text[6] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk6a, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0xf;
    label.text[3] = 0xe;
    label.text[4] = 0x8;
    label.text[5] = 0x12;
    label.text[6] = 0xe;
    label.text[7] = 0xd;
    label.text[8] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk76, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x3;
    label.text[3] = 0;
    label.text[4] = 0x11;
    label.text[5] = 0xa;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk6c, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x5;
    label.text[3] = 0x8;
    label.text[4] = 0x11;
    label.text[5] = 0x4;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk6e, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x4;
    label.text[3] = 0;
    label.text[4] = 0x11;
    label.text[5] = 0x13;
    label.text[6] = 0x7;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk70, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0x8;
    label.text[4] = 0xd;
    label.text[5] = 0x3;
    label.text[6] = 0x7f;
    label.text[7] = 0xc;
    label.text[8] = 0;
    label.text[9] = 0x6;
    label.text[10] = 0x8;
    label.text[11] = 0x2;
    label.text[12] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk72, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    label.text[2] = 0x16;
    label.text[3] = 0;
    label.text[4] = 0x13;
    label.text[5] = 0x4;
    label.text[6] = 0x11;
    label.text[7] = 0x7f;
    label.text[8] = 0xc;
    label.text[9] = 0;
    label.text[10] = 0x6;
    label.text[11] = 0x8;
    label.text[12] = 0x2;
    label.text[13] = 0xff;
    label.y += 17;
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk74, 6, 0, 0, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
    menu_draw_nine_slice_panel(0x14, 0x1a, 0x114, 0xb4);
    menu_draw_button_icon(0x80, 0xd6, DAT_8009c258);
    menu_draw_ascii_string(0x8a, 0xd6, "return", 0, 0);
}

ADDRESS(0x80023ce8, 0x2ec)
void menu_draw_conversation_page(s32 page)
{
    char names[48][12] = {
        "christy", "lyn", "", "gullick", "jens", "rene", "michael", "janan", "mark", "calmathar",
        "garnabus", "ungrund", "zul", "varde", "jack", "airon", "marcus", "jamie", "yvette", "toni",
        "priscilla", "marilyn", "ed", "robert", "krone", "jane", "john", "olivier", "sal", "giant",
        "light", "vallad", "leon", "", "", "orladin", "", "", "joe", "franz", "tim", "james",
        "alexander", "jean", "seath", "", "", "",
    };

    menu_draw_nine_slice_panel(0x1f, 0x20, 0x102, 0xb0);
    menu_draw_button_icon(0x7d, 0xbe, 8);
    menu_draw_ascii_string(0x8c, 0xbf, "talk", 1, 0);
    menu_draw_button_icon(0xb2, 0xbe, 9);
    menu_draw_ascii_string(0xc1, 0xbf, "character", 1, 0);
    menu_draw_button_icon(0x80, 0xd6, DAT_8009c258);
    menu_draw_ascii_string(0x8a, 0xd6, "return", 0, 0);
    if (page != -1) {
        primitive_buffer_begin_poly_ft4();
        DAT_8009c374->tpage = 0x1f;
        DAT_8009c374->clut = 0x7806;
        primitive_set_poly_ft4_rect(0x2c, 0x24, 0xf8, 0xc8, 0, 0, 0xf8, 0xc8);
        primitive_buffer_commit_poly_ft4(0x1e);
        if (DAT_801b24e4.unk100 == 1) {
            primitive_buffer_begin_poly_ft4();
            setRGB0(DAT_8009c374, 0x10, 0x10, 0x10);
            DAT_8009c374->tpage = 0x1f;
            DAT_8009c374->clut = 0x7806;
            primitive_set_poly_ft4_rect(0x2d, 0x25, 0xf8, 0xc8, 0, 0, 0xf8, 0xc8);
            primitive_buffer_commit_poly_ft4(0x1f);
        }
        primitive_buffer_begin_poly_ft4();
        setRGB0(DAT_8009c374, 0x60, 0x60, 0x60);
        DAT_8009c374->tpage = GetTPage(1, 1, 0x3c0, 0x100);
        DAT_8009c374->clut = 0x7f80;
        primitive_set_poly_ft4_rect(0x28, 0x98, 0x40, 0x2f, 0, 0xd0, 0x40, 0x2f);
        primitive_buffer_commit_poly_ft4(0x1e);
        menu_draw_ascii_string(0x6b, 0x9a, names[page], 1, 0);
    }
}

ADDRESS(0x80023fd4, 0x6e8)
void menu_draw_button_config(u8 style, u8 layout, s32 row)
{
    s32 order[6];
    char actions[4][8] = {"attack", "event", "magic", "status"};
    char moves[9][16] = {"move{left", "view{up", "move{right", "view{down", "forward{back", "turn", "move",
                         "turn{left", "turn{right"};
    char types[7][8] = {"type{a", "type{b", "type{c", "type{d", "type{e", "type{f", "type{g"};
    s32 i;
    s32 bright;
    s32 blue;

    switch (style) {
    case 0:
        order[0] = 0;
        order[1] = 1;
        order[2] = 2;
        order[3] = 3;
        break;
    case 1:
        order[0] = 2;
        order[1] = 1;
        order[2] = 0;
        order[3] = 3;
        break;
    case 2:
        order[0] = 0;
        order[1] = 3;
        order[2] = 2;
        order[3] = 1;
        break;
    case 3:
        order[0] = 2;
        order[1] = 3;
        order[2] = 0;
        order[3] = 1;
        break;
    }
    for (i = 0; i < 4; i++) {
        menu_draw_button_icon(0x89, i * 14 + 0x31, i);
        menu_draw_ascii_string(0x9b, i * 14 + 0x32, actions[order[i]], 1, 0);
    }
    switch (layout) {
    case 0:
        order[0] = 0;
        order[1] = 1;
        order[2] = 2;
        order[3] = 3;
        order[4] = 4;
        order[5] = 5;
        break;
    case 1:
        order[0] = 1;
        order[1] = 3;
        order[2] = 0;
        order[3] = 2;
        order[4] = 4;
        order[5] = 5;
        break;
    case 2:
        order[0] = 0;
        order[1] = 2;
        order[2] = 1;
        order[3] = 3;
        order[4] = 4;
        order[5] = 5;
        break;
    case 3:
        order[0] = 7;
        order[1] = 1;
        order[2] = 8;
        order[3] = 3;
        order[4] = 4;
        order[5] = 6;
        break;
    case 4:
        order[0] = 1;
        order[1] = 3;
        order[2] = 7;
        order[3] = 8;
        order[4] = 4;
        order[5] = 6;
        break;
    case 5:
        order[0] = 7;
        order[1] = 8;
        order[2] = 1;
        order[3] = 3;
        order[4] = 4;
        order[5] = 6;
        break;
    case 6:
        order[0] = 3;
        order[1] = 0;
        order[2] = 1;
        order[3] = 2;
        order[4] = 4;
        order[5] = 5;
        break;
    }
    for (i = 4; i < 10; i++) {
        menu_draw_button_icon(0x89, (i - 4) * 14 + 0x6f, i);
        menu_draw_ascii_string(0x9b, (i - 4) * 14 + 0x70, moves[order[i - 4]], 1, 0);
    }
    menu_draw_ascii_string(0x32, 0x32, "action", 1, 0);
    menu_draw_ascii_string(0x3a, 0x40, types[style], 1, 0);
    menu_draw_ascii_string(0x32, 0x70, "move", 1, 0);
    menu_draw_ascii_string(0x3a, 0x7e, types[layout], 1, 0);
    if (DAT_8009c260 < 48) {
        bright = DAT_8009c260 + DAT_8009c26c;
        blue = DAT_8009c260 / 2 + DAT_8009c26c;
    } else {
        bright = 95 - DAT_8009c260 + DAT_8009c26c;
        blue = 47 - DAT_8009c260 / 2 + DAT_8009c26c;
    }
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, bright, bright, blue);
    DAT_8009c374->tpage = DAT_8007e564[11].unk0;
    DAT_8009c374->clut = DAT_8007e564[11].unk2;
    if (row == 0) {
        primitive_set_poly_ft4_rect(0x37, 0x3c, 0x35, 0xf, DAT_8007e564[11].unk4, DAT_8007e564[11].unk6,
                                    DAT_8007e564[11].unk8, DAT_8007e564[11].unka);
    } else {
        primitive_set_poly_ft4_rect(0x37, 0x7a, 0x35, 0xf, DAT_8007e564[11].unk4, DAT_8007e564[11].unk6,
                                    DAT_8007e564[11].unk8, DAT_8007e564[11].unka);
    }
    SetSemiTrans((void *)DAT_8009c374, 1);
    primitive_buffer_commit_poly_ft4(0x31);
    menu_draw_nine_slice_panel(0x1f, 0x20, 0x102, 0xb0);
    menu_draw_button_icon(0x5b, 0xd6, 9);
    menu_draw_ascii_string(0x69, 0xd6, "select", 0, 0);
    menu_draw_button_icon(0xa0, 0xd6, DAT_8009c258);
    menu_draw_ascii_string(0xaa, 0xd6, "return", 0, 0);
}

ADDRESS(0x800246bc, 0x3bc)
void menu_draw_options_rows(u8 *options)
{
    MenuText t[3];
    s32 i;

    t[0].x = 0xab;
    t[0].y = 0x3a;
    t[0].text[0] = 0;
    t[0].text[1] = 0xff;
    t[1].x = 0xc7;
    t[1].y = 0x3a;
    t[1].text[0] = 1;
    t[1].text[1] = 0xff;
    t[2].x = 0xe3;
    t[2].y = 0x3a;
    t[2].text[0] = 2;
    t[2].text[1] = 0xff;
    for (i = 0; i < 2; i++) {
        if (*options == 2) {
            menu_blit_panel_sprite_pulsing(&DAT_8007e564[8], &t[0], DAT_8009c264);
            t[0].x += 4;
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            menu_blit_panel_sprite(&DAT_8007e564[8], &t[1]);
            t[1].x += 4;
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
            menu_blit_panel_sprite(&DAT_8007e564[8], &t[2]);
            t[2].x += 4;
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[2]);
        } else if (*options == 1) {
            menu_blit_panel_sprite(&DAT_8007e564[8], &t[0]);
            t[0].x += 4;
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            menu_blit_panel_sprite_pulsing(&DAT_8007e564[8], &t[1], DAT_8009c264);
            t[1].x += 4;
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
            menu_blit_panel_sprite(&DAT_8007e564[8], &t[2]);
            t[2].x += 4;
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[2]);
        } else {
            menu_blit_panel_sprite(&DAT_8007e564[8], &t[0]);
            t[0].x += 4;
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            menu_blit_panel_sprite(&DAT_8007e564[8], &t[1]);
            t[1].x += 4;
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
            menu_blit_panel_sprite_pulsing(&DAT_8007e564[8], &t[2], DAT_8009c264);
            t[2].x += 4;
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &t[2]);
        }
        options++;
        t[0].x = 0xab;
        t[1].x = 0xc7;
        t[2].x = 0xe3;
        t[0].y += 26;
        t[1].y += 26;
        t[2].y += 26;
    }
    t[0].x = 0xab;
    t[0].text[0] = 0x7f;
    t[0].text[1] = 0x7f;
    t[0].text[2] = 0xe;
    t[0].text[3] = 0xd;
    t[0].text[4] = 0xff;
    t[1].x = 0xe5;
    t[1].text[0] = 0x7f;
    t[1].text[1] = 0x7f;
    t[1].text[2] = 0xe;
    t[1].text[3] = 5;
    t[1].text[4] = 5;
    t[1].text[5] = 0xff;
    for (i = 0; i < 3; i++) {
        if (i == 2) {
            t[0].text[0] = 2;
            t[0].text[1] = 0xb;
            t[0].text[2] = 4;
            t[0].text[3] = 0;
            t[0].text[4] = 0x11;
            t[0].text[5] = 0xff;
            t[1].text[0] = 0x12;
            t[1].text[1] = 0xe;
            t[1].text[2] = 0xb;
            t[1].text[3] = 8;
            t[1].text[4] = 3;
            t[1].text[5] = 0xff;
        }
        if (*options == 1) {
            menu_blit_panel_sprite_pulsing(&DAT_8007e564[4], &t[0], DAT_8009c264);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            menu_blit_panel_sprite(&DAT_8007e564[4], &t[1]);
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
        } else {
            menu_blit_panel_sprite(&DAT_8007e564[4], &t[0]);
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], &t[0]);
            menu_blit_panel_sprite_pulsing(&DAT_8007e564[4], &t[1], DAT_8009c264);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &t[1]);
        }
        options++;
        t[0].y += 26;
        t[1].y += 26;
        t[2].y += 26;
    }
}

ADDRESS(0x80024a78, 0x1f8)
void menu_draw_volume_brightness_sliders(u8 *levels)
{
    s32 i;
    s32 k;

    primitive_buffer_add_draw_mode(0x32, DAT_8007e564[10].unk0);
    for (i = 0; i < 3; i++) {
        primitive_buffer_begin_sprt();
        setRGB0(DAT_8009c378, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
        k = 10;
        DAT_8009c378->clut = DAT_8007e564[k].unk2;
        DAT_8009c378->x0 = 0xa5;
        DAT_8009c378->y0 = i * 26 + 0x34;
        DAT_8009c378->w = DAT_8007e564[k].unk8;
        DAT_8009c378->h = DAT_8007e564[k].unka;
        DAT_8009c378->u0 = DAT_8007e564[k].unk4;
        DAT_8009c378->v0 = DAT_8007e564[k].unk6;
        SetSemiTrans((void *)DAT_8009c378, DAT_801b24e4.unk100);
        primitive_buffer_commit_sprt(0x32);
        primitive_buffer_begin_sprt();
        setRGB0(DAT_8009c378, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
        k = 9;
        DAT_8009c378->clut = DAT_8007e564[k].unk2;
        DAT_8009c378->x0 = *levels * 96 / 128 + 172;
        DAT_8009c378->y0 = i * 26 + 0x3c;
        DAT_8009c378->w = DAT_8007e564[k].unk8;
        DAT_8009c378->h = DAT_8007e564[k].unka;
        DAT_8009c378->u0 = DAT_8007e564[k].unk4 + DAT_801b24e4.unk100 * 16;
        DAT_8009c378->v0 = DAT_8007e564[k].unk6;
        primitive_buffer_commit_sprt(0x31);
        levels++;
    }
}

ADDRESS(0x80024c70, 0x318)
s32 menu_preview_choice(MenuList *list, s32 kind, s32 flags, u8 item)
{
    MenuText labels[2];
    s32 choice;
    s32 result;
    s32 pad;
    s32 i;

    labels[0].x = DAT_8007e660[10].x;
    labels[0].y = DAT_8007e660[10].y;
    labels[1].x = DAT_8007e660[11].x;
    labels[1].y = DAT_8007e660[11].y;
    choice = 0;
    result = MENU_NONE;
    flags |= 0x8000;
    if (kind == 0) {
        labels[0].text[0] = 0x14;
        labels[0].text[1] = 0x12;
        labels[0].text[2] = 4;
        labels[0].text[3] = 0xff;
    } else if (kind == 1) {
        labels[0].text[0] = 0x12;
        labels[0].text[1] = 0x13;
        labels[0].text[2] = 0xe;
        labels[0].text[3] = 0x11;
        labels[0].text[4] = 4;
        labels[0].text[5] = 0xff;
    } else if (kind == 2 || kind == 3 || kind == 4 || kind == 6 || kind == 7 || kind == 8) {
        labels[0].text[0] = 0x18;
        labels[0].text[1] = 4;
        labels[0].text[2] = 0x12;
        labels[0].text[3] = 0xff;
    } else if (kind == 5) {
        labels[0].text[0] = 0x14;
        labels[0].text[1] = 0x12;
        labels[0].text[2] = 4;
        labels[0].text[3] = 0xff;
    } else if (kind == 9) {
        labels[0].text[0] = 0xc;
        labels[0].text[1] = 0;
        labels[0].text[2] = 0xa;
        labels[0].text[3] = 4;
        labels[0].text[4] = 8;
        labels[0].text[5] = 0x13;
        labels[0].text[6] = 0xff;
    } else {
        labels[0].text[0] = 0xc;
        labels[0].text[1] = 0xe;
        labels[0].text[2] = 0x15;
        labels[0].text[3] = 4;
        labels[0].text[4] = 0xff;
    }
    if (kind == 2 || kind == 3 || kind == 4 || kind == 6 || kind == 7 || kind == 8) {
        labels[1].text[0] = 0xd;
        labels[1].text[1] = 0xe;
        labels[1].text[2] = 0xff;
    } else {
        labels[1].text[0] = 2;
        labels[1].text[1] = 0;
        labels[1].text[2] = 0xd;
        labels[1].text[3] = 2;
        labels[1].text[4] = 4;
        labels[1].text[5] = 0xb;
        labels[1].text[6] = 0xff;
    }
    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            break;
        }
        input_wait_brief_release();
        pad = input_read_mark_active();
        if ((pad & 0x1000) || (pad & 0x4000)) {
            DAT_8009c268 = 0;
            menu_play_sound_cue(0xc);
            if (choice == 0) {
                choice = 1;
            } else {
                choice = 0;
            }
        } else if (pad & DAT_8009c24c) {
            menu_play_sound_cue(0xd);
            result = -choice;
        } else if (pad & DAT_8009c250) {
            menu_play_sound_cue(0xe);
            result = MENU_CANCEL;
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            menu_update_item_preview(item);
            menu_render_list(list, flags);
            menu_draw_two_option(&labels[0], &labels[1], choice);
            menu_present_frame();
        }
    }
    return result;
}

ADDRESS(0x80024f88, 0x36c)
void menu_show_item_guide(MenuList *list, s32 mode, u8 item)
{
    RECT rect;
    s32 result;
    s32 entry;
    s32 size;
    s32 saved;
    u8 *buffer;
    s32 i;

    result = MENU_NONE;
    menu_play_sound_cue(0xd);
    if (mode != 1) {
        menu_wait_async_read();
    }
    rect.x = 0x3c0;
    rect.y = 0x100;
    rect.w = 0x40;
    rect.h = 0xfe;
    ClearImage(&rect, 0, 0, 0);
    if (mode == 0) {
        entry = item + 390;
        if (DAT_80081780[item] > DAT_801b24e4.unk4c) {
            entry = item + 540;
        }
    } else if (mode == 1) {
        entry = item + 690;
    } else if (mode == 2) {
        entry = item + 390;
    } else {
        entry = item + 390;
        DAT_80081780[item] = 4;
    }
    saved = DAT_8009c230;
    size = cd_archive_entry_extent(6, entry, 0);
    DAT_8009c234 = (size + 2047) >> 11;
    buffer = (u8 *)memory_allocate(size);
    DAT_8009c380 = buffer;
    cd_archive_start_async_read(6, entry);
    DAT_8009c230 = 1;
    do {
        VSync(2);
    } while (DAT_8009c230 == 1);
    tim_upload_images(buffer);
    DrawSync(0);
    memory_free((void *)buffer);
    DAT_8009c230 = saved;
    input_wait_release();
    for (;;) {
        if (result != MENU_NONE) {
            input_wait_release();
            break;
        }
        input_wait_brief_release();
        if (input_read_mark_active() & DAT_8009c250) {
            menu_play_sound_cue(0xe);
            result = MENU_CANCEL;
        }
        for (i = 0; i < 2; i++) {
            menu_frame_begin();
            if (list->unk0 != 0) {
                menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], (MenuText *)list);
                menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], (MenuText *)list);
            }
            if (mode != 1) {
                menu_update_item_preview(item);
            }
            menu_draw_nine_slice_panel(0x1b, 0x70, 0x10a, 0x5f);
            primitive_buffer_begin_poly_ft4();
            DAT_8009c374->tpage = 0x1f;
            DAT_8009c374->clut = 0x7806;
            primitive_set_poly_ft4_rect(0x20, 0x76, 0xfe, 0x50, 0, 0, 0xfe, 0x50);
            primitive_buffer_commit_poly_ft4(0x1d);
            primitive_buffer_begin_poly_ft4();
            setRGB0(DAT_8009c374, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
            DAT_8009c374->tpage = 0x1f;
            DAT_8009c374->clut = 0x7806;
            primitive_set_poly_ft4_rect(0x21, 0x77, 0xfe, 0x50, 0, 0, 0xfe, 0x50);
            primitive_buffer_commit_poly_ft4(0x1e);
            menu_draw_button_icon(0x80, 0xd6, DAT_8009c258);
            menu_draw_ascii_string(0x8a, 0xd6, "return", 0, 0);
            menu_present_frame();
        }
    }
}

ADDRESS(0x800252f4, 0x174)
void menu_draw_window(s32 page, s32 count, s32 selected, s32 flags)
{
    MenuText *title;
    MenuText *line;
    s32 i;

    title = &DAT_8007e660[page * 9];
    line = title + 1;
    if (title->x != 0) {
        menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], title);
        menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], title);
    }
    for (i = 0; i < count; i++) {
        if (i == selected) {
            menu_blit_panel_sprite_pulsing(&DAT_8007e564[MENU_FONT_FRAME], line, DAT_8009c260);
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], line);
        } else {
            menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], line);
            menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], line);
        }
        line++;
    }
    if (flags == 0) {
        menu_draw_button_icon(0x5f, 0xd6, DAT_8009c254);
    } else if (flags == 1) {
        menu_draw_button_icon(0x5b, 0xd6, 9);
    }
    menu_draw_ascii_string(0x69, 0xd6, "select", 0, 0);
    menu_draw_button_icon(0xa0, 0xd6, DAT_8009c258);
    menu_draw_ascii_string(0xaa, 0xd6, "return", 0, 0);
}

ADDRESS(0x80025468, 0x6bc)
void menu_render_list(MenuList *list, s32 flags)
{
    MenuText text;
    u8 *names;
    u8 *second;
    u32 *values;
    u8 *counts;
    s32 hide;
    s32 stock;
    s32 i;
    s32 j;
    s32 bright;
    s32 blue;

    hide = flags & 0x8000;
    names = (u8 *)list->entries;
    second = (u8 *)list->unk28;
    values = list->values;
    counts = list->counts;
    flags &= 0xfff;
    if (list->unk0 != 0) {
        menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], (MenuText *)list);
        menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], (MenuText *)list);
    }
    names += list->first * list->unk23;
    second += list->first * 24;
    values += list->first;
    counts += list->first;
    stock = flags == 8 || flags == 9;
    for (i = 0; i < list->rows; i++) {
        if (i >= list->count) {
            break;
        }
        text.x = list->x + 13;
        text.y = list->y + 8 + i * 16;
        for (j = 0; j < list->unk23; j++) {
            text.text[j] = *names++;
        }
        menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &text);
        if (flags == 3) {
            text.x += 84;
            for (j = 0; j < 24; j++) {
                text.text[j] = *second++;
            }
            text.text[j] = 0xff;
            menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &text);
        }
        if (flags >= 10 && flags < 16 && flags != 12) {
            text.x += 161;
            menu_format_number(*values++, 6, 0, 2, text.text);
            menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
        }
        if (flags == 2 || flags == 4) {
            if (*values++ != -1) {
                text.x += 206;
                menu_format_number(values[-1], 3, 0, 3, text.text);
                menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
        if (stock) {
            if (*values++ != -1) {
                text.x += 115;
                menu_format_number(values[-1], 7, 0, 4, text.text);
                menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
            if (*counts++ != 0xff) {
                text.x += 91;
                menu_format_number(counts[-1], 3, 0, 5, text.text);
                menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
        if (!(flags == 2 || flags == 3 || flags == 4 || stock || flags == 16)) {
            if (*counts++ != 0xff) {
                text.x = list->x + 233;
                menu_format_number(counts[-1], 2, 0, 1, text.text);
                menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
        if (flags == 16) {
            if (*counts++ != 0xff) {
                text.x += 220;
                menu_format_number(counts[-1], 2, 0, 1, text.text);
                menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
            if (*values++ != -1) {
                text.x += 206;
                menu_format_number(values[-1], 3, 0, 3, text.text);
                menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &text);
            }
        }
    }
    menu_draw_nine_slice_panel(list->x, list->y, 0x10a, list->rows * 16 + 12);
    for (i = 0; i < list->rows; i++) {
        if (i == list->row) {
            if (DAT_8009c260 < 48) {
                bright = DAT_8009c260 + DAT_8009c26c;
                blue = DAT_8009c260 / 2 + DAT_8009c26c;
            } else {
                bright = DAT_8009c26c + 95 - DAT_8009c260;
                blue = DAT_8009c26c + 47 - DAT_8009c260 / 2;
            }
            primitive_buffer_begin_poly_ft4();
            setRGB0(DAT_8009c374, bright, bright, blue);
            DAT_8009c374->tpage = DAT_8007e564[11].unk0;
            DAT_8009c374->clut = DAT_8007e564[11].unk2;
            primitive_set_poly_ft4_rect(list->x + 6, list->y + i * 16 + 5, DAT_8007e564[11].unk8,
                                        DAT_8007e564[11].unka, DAT_8007e564[11].unk4, DAT_8007e564[11].unk6,
                                        DAT_8007e564[11].unk8, DAT_8007e564[11].unka);
            SetSemiTrans((void *)DAT_8009c374, 1);
            primitive_buffer_commit_poly_ft4(0x31);
        }
    }
    if (flags == 7) {
        menu_draw_quantity(0);
    }
    if (flags == 10 || flags == 11 || flags == 14) {
        menu_draw_quantity(1);
    }
    if (flags == 13) {
        menu_draw_gold();
    }
    if (flags == 3 || flags == 8 || flags == 9 || flags == 12 || hide != 0) {
        menu_draw_button_icon(0x5f, 0xd6, DAT_8009c254);
        menu_draw_ascii_string(0x69, 0xd6, "select", 0, 0);
        menu_draw_button_icon(0xa0, 0xd6, DAT_8009c258);
        menu_draw_ascii_string(0xaa, 0xd6, "return", 0, 0);
    } else {
        menu_draw_button_icon(0x42, 0xd6, DAT_8009c254);
        menu_draw_ascii_string(0x4c, 0xd6, "select", 0, 0);
        menu_draw_button_icon(0x83, 0xd6, 0);
        menu_draw_ascii_string(0x8d, 0xd6, "guide", 0, 0);
        menu_draw_button_icon(0xbc, 0xd6, DAT_8009c258);
        menu_draw_ascii_string(0xc6, 0xd6, "return", 0, 0);
    }
}

ADDRESS(0x80025b24, 0xc4)
void menu_draw_two_option(MenuText *yes, MenuText *no, s32 choice)
{
    if (choice == 0) {
        menu_blit_panel_sprite_pulsing(&DAT_8007e564[4], yes, DAT_8009c260);
        menu_blit_panel_sprite(&DAT_8007e564[4], no);
        menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], yes);
        menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], no);
    } else {
        menu_blit_panel_sprite(&DAT_8007e564[4], yes);
        menu_blit_panel_sprite_pulsing(&DAT_8007e564[4], no, DAT_8009c260);
        menu_draw_string_dimmed(&DAT_8007e564[MENU_FONT_TEXT], yes);
        menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], no);
    }
}

ADDRESS(0x80025be8, 0x1d0)
void menu_update_item_preview(u8 item)
{
    MATRIX m;
    MATRIX unused[2];
    MATRIX light;
    MATRIX lights;
    MATRIX color;

    if (DAT_801b24e4.unkfb != 0 && item != 0xff && DAT_8009c230 != 1) {
        if (DAT_8009c23c != -1 && DAT_8009c23c != 0xff) {
            menu_start_item_model_read();
            return;
        }
        if (DAT_8009c240 != 0) {
            CdControl(CdlPause, 0, 0);
            if (cd_sectors_corrupt(DAT_8009c37c, DAT_8009c240) == 0) {
                DAT_8009c244 = -1;
            }
            DAT_8009c240 = 0;
        }
        if (DAT_8009c244 == -1) {
            if (DAT_8009c238 == 0) {
                tmd_register(3, DAT_8009c37c);
                DAT_8009c238 = 1;
            }
            m.t[0] = DAT_8009c394.vx;
            m.t[1] = DAT_8009c394.vy;
            m.t[2] = DAT_8009c394.vz;
            DAT_8009c39c.vy += DAT_8009c3a4;
            DAT_8009c39c.vx &= 0xfff;
            DAT_8009c39c.vy &= 0xfff;
            DAT_8009c39c.vz &= 0xfff;
            RotMatrix(&DAT_8009c39c, &m);
            light.m[0][0] = 0x800;
            light.m[0][1] = 0x800;
            light.m[0][2] = -0x800;
            light.m[1][0] = 0;
            light.m[1][1] = 0;
            light.m[1][2] = -0x800;
            light.m[2][0] = 0x1000;
            light.m[2][1] = 0;
            light.m[2][2] = -0x800;
            MulMatrix0(&light, &m, &lights);
            SetLightMatrix(&lights);
            color.m[0][0] = 0xc00;
            color.m[0][1] = 0xc00;
            color.m[0][2] = 0xc00;
            color.m[1][0] = 0xc00;
            color.m[1][1] = 0xc00;
            color.m[1][2] = 0xc00;
            color.m[2][0] = 0xc00;
            color.m[2][1] = 0xc00;
            color.m[2][2] = 0xc00;
            SetColorMatrix(&color);
            SetRotMatrix(&m);
            SetTransMatrix(&m);
            menu_render_item_model();
        }
    }
}

ADDRESS(0x80025db8, 0xc8)
void menu_draw_quantity(s32 big)
{
    MenuText label;
    MenuText number;

    if (big) {
        menu_draw_gold();
    }
    label.x = 0xb7;
    if (big == 0) {
        label.y = 0x2e;
    } else {
        label.y = 0x4a;
    }
    label.text[0] = 0x10;
    label.text[1] = 0x13;
    label.text[2] = 0x18;
    label.text[3] = 0xff;
    menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], &label);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 84;
    number.y = label.y;
    menu_format_number(DAT_8009c25c, 2, 0, 1, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
}

ADDRESS(0x80025e80, 0xb8)
void menu_draw_gold(void)
{
    MenuText label;
    MenuText number;

    label.x = 0xb7;
    label.y = 0x2e;
    label.text[0] = 6;
    label.text[1] = 0xe;
    label.text[2] = 0xb;
    label.text[3] = 3;
    label.text[4] = 0xff;
    menu_blit_panel_sprite(&DAT_8007e564[MENU_FONT_FRAME], &label);
    menu_draw_string(&DAT_8007e564[MENU_FONT_TEXT], &label);
    number.x = label.x + 49;
    number.y = label.y;
    menu_format_number(DAT_801b24e4.unk50, 7, 0, 2, number.text);
    menu_draw_number(&DAT_8007e564[MENU_FONT_NUMBER], &number);
}

ADDRESS(0x80025f38, 0xe8)
void menu_blit_panel_sprite(MenuFont *frame, MenuText *text)
{
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
    DAT_8009c374->tpage = frame->unk0;
    DAT_8009c374->clut = frame->unk2;
    primitive_set_poly_ft4_rect(text->x - 6, text->y - 6, frame->unk8, frame->unka, frame->unk4, frame->unk6,
                                frame->unk8, frame->unka);
    SetSemiTrans((void *)DAT_8009c374, DAT_801b24e4.unk100);
    primitive_buffer_commit_poly_ft4(0x32);
}

ADDRESS(0x80026020, 0x138)
void menu_blit_panel_sprite_pulsing(MenuFont *frame, MenuText *text, s32 shade)
{
    s32 bright;
    s32 blue;

    if (shade < 48) {
        bright = shade + DAT_8009c26c;
        blue = shade / 2 + DAT_8009c26c;
    } else {
        bright = 95 - shade + DAT_8009c26c;
        blue = 47 - shade / 2 + DAT_8009c26c;
    }
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, bright, bright, blue);
    DAT_8009c374->tpage = frame->unk0;
    DAT_8009c374->clut = frame->unk2;
    primitive_set_poly_ft4_rect(text->x - 6, text->y - 6, frame->unk8, frame->unka, frame->unk4, frame->unk6,
                                frame->unk8, frame->unka);
    SetSemiTrans((void *)DAT_8009c374, DAT_801b24e4.unk100);
    primitive_buffer_commit_poly_ft4(0x32);
}

ADDRESS(0x80026158, 0x84)
void menu_draw_string_dimmed(MenuFont *font, MenuText *text)
{
    s32 r;
    s32 g;
    s32 b;

    r = DAT_8009c270;
    g = DAT_8009c274;
    b = DAT_8009c278;
    if (DAT_801b24e4.unk100 == 0) {
        DAT_8009c270 = 0x47;
        DAT_8009c274 = 0x47;
        DAT_8009c278 = 0x57;
    } else {
        DAT_8009c270 = 0x57;
        DAT_8009c274 = 0x57;
    }
    menu_draw_string(font, text);
    DAT_8009c270 = r;
    DAT_8009c274 = g;
    DAT_8009c278 = b;
}

ADDRESS(0x800261dc, 0x190)
void menu_draw_string(MenuFont *font, MenuText *text)
{
    u8 *c;
    s32 dx;
    s32 u;
    s32 v;
    s32 ch;

    primitive_buffer_add_draw_mode(3, font->unk0);
    c = text->text;
    if (*c == 0xff) {
        return;
    }
    dx = 0;
    do {
        primitive_buffer_begin_sprt();
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx - 1;
        DAT_8009c378->y0 = text->y - 1;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        ch = *c;
        u = (ch & 0xf) * 8;
        v = (ch >> 4) * 15;
        DAT_8009c378->u0 = u;
        DAT_8009c378->v0 = v;
        primitive_buffer_commit_sprt(2);
        c++;
        primitive_buffer_begin_sprt();
        setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx;
        DAT_8009c378->y0 = text->y;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        DAT_8009c378->u0 = u;
        DAT_8009c378->v0 = v;
        primitive_buffer_commit_sprt(3);
        dx += 7;
    } while (*c != 0xff);
}

ADDRESS(0x8002636c, 0x204)
void menu_draw_number(MenuFont *font, MenuText *text)
{
    u8 *c;
    s32 dx;

    primitive_buffer_add_draw_mode(3, font->unk0);
    c = text->text;
    if (*c == 0xff) {
        return;
    }
    dx = 0;
    do {
        primitive_buffer_begin_sprt();
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx - 1;
        DAT_8009c378->y0 = text->y - 1;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        if (*c < 11) {
            DAT_8009c378->u0 = font->unk4;
            DAT_8009c378->v0 = *c * 15;
        } else {
            DAT_8009c378->u0 = font->unk4 + 7;
            DAT_8009c378->v0 = (*c - 11) * 15;
        }
        primitive_buffer_commit_sprt(2);
        primitive_buffer_begin_sprt();
        setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
        DAT_8009c378->clut = font->unk2;
        DAT_8009c378->x0 = text->x + dx;
        DAT_8009c378->y0 = text->y;
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        if (*c < 11) {
            DAT_8009c378->u0 = font->unk4;
            DAT_8009c378->v0 = *c * 15;
        } else {
            DAT_8009c378->u0 = font->unk4 + 7;
            DAT_8009c378->v0 = (*c - 11) * 15;
        }
        primitive_buffer_commit_sprt(3);
        c++;
        dx += 7;
    } while (*c != 0xff);
}

/*
 * `level = 0` comes before the font call, so level lives across it and is
 * allocated a saved register (retail's $s0 for level and shade).
 */
ADDRESS(0x80026570, 0x224)
void menu_draw_ascii_string(s32 x, s32 y, char *text, s32 shadow, s32 row)
{
    MenuFont *font;
    s32 level;
    s32 shade;
    s32 v;

    font = &DAT_8007e564[2];
    v = row != 0 ? 16 : 0;
    level = 0;
    primitive_buffer_add_draw_mode(6, font->unk0);
    if (DAT_801b24e4.unkfd > 128) {
        level = (DAT_801b24e4.unkfd - 128) / 2;
    }
    shade = level + 112;
    for (;;) {
        if (*text == 0) {
            return;
        }
        primitive_buffer_begin_sprt();
        if (shadow == 0) {
            setRGB0(DAT_8009c378, shade, shade, shade);
        }
        DAT_8009c378->clut = font->unk2;
        if (shadow == 0) {
            DAT_8009c378->x0 = x;
            DAT_8009c378->y0 = y;
        } else {
            DAT_8009c378->x0 = x - 1;
            DAT_8009c378->y0 = y - 1;
        }
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        DAT_8009c378->u0 = font->unk4 + (*text - 'a') * font->unk8;
        DAT_8009c378->v0 = font->unk6 + v;
        primitive_buffer_commit_sprt(5);
        if (shadow != 0) {
            primitive_buffer_begin_sprt();
            setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
            DAT_8009c378->clut = font->unk2;
            DAT_8009c378->x0 = x;
            DAT_8009c378->y0 = y;
            DAT_8009c378->w = font->unk8;
            DAT_8009c378->h = font->unka;
            DAT_8009c378->u0 = font->unk4 + (*text - 'a') * font->unk8;
            DAT_8009c378->v0 = font->unk6 + v;
            primitive_buffer_commit_sprt(6);
        }
        text++;
        x += 8;
    }
}

/* As menu_draw_ascii_string: level is cleared before the font call ($s2, shared with dx). */
ADDRESS(0x80026794, 0x22c)
void menu_draw_small_glyph_string(MenuText *text, s32 shadow)
{
    MenuFont *font;
    u8 *c;
    s32 level;
    s32 dx;
    s32 shade;

    font = &DAT_8007e564[2];
    level = 0;
    primitive_buffer_add_draw_mode(6, font->unk0);
    if (DAT_801b24e4.unkfd > 128) {
        level = (DAT_801b24e4.unkfd - 128) / 2;
    }
    c = text->text;
    if (*c == 0xff) {
        return;
    }
    shade = level + 112;
    dx = 0;
    do {
        primitive_buffer_begin_sprt();
        if (shadow == 0) {
            setRGB0(DAT_8009c378, shade, shade, shade);
        }
        DAT_8009c378->clut = font->unk2;
        if (shadow == 0) {
            DAT_8009c378->x0 = text->x + dx;
            DAT_8009c378->y0 = text->y;
        } else {
            DAT_8009c378->x0 = text->x + dx - 1;
            DAT_8009c378->y0 = text->y - 1;
        }
        DAT_8009c378->w = font->unk8;
        DAT_8009c378->h = font->unka;
        DAT_8009c378->u0 = font->unk4 + *c * 8;
        DAT_8009c378->v0 = font->unk6 + 8;
        primitive_buffer_commit_sprt(5);
        if (shadow != 0) {
            primitive_buffer_begin_sprt();
            setRGB0(DAT_8009c378, DAT_8009c27c, DAT_8009c27c, DAT_8009c27c);
            DAT_8009c378->clut = font->unk2;
            DAT_8009c378->x0 = text->x + dx;
            DAT_8009c378->y0 = text->y;
            DAT_8009c378->w = font->unk8;
            DAT_8009c378->h = font->unka;
            DAT_8009c378->u0 = font->unk4 + *c * 8;
            DAT_8009c378->v0 = font->unk6 + 8;
            primitive_buffer_commit_sprt(6);
        }
        c++;
        dx += 7;
    } while (*c != 0xff);
}

ADDRESS(0x800269c0, 0x10c)
void menu_draw_button_icon(s32 x, s32 y, s32 value)
{
    s32 shade;

    primitive_buffer_add_draw_mode(6, DAT_8007e564[6].unk0);
    shade = 0;
    primitive_buffer_begin_sprt();
    if (DAT_801b24e4.unkfd > 128) {
        shade = (DAT_801b24e4.unkfd - 128) / 2;
    }
    setRGB0(DAT_8009c378, shade + 112, shade + 112, shade + 112);
    DAT_8009c378->clut = DAT_8007e564[6].unk2;
    DAT_8009c378->x0 = x;
    DAT_8009c378->y0 = y;
    DAT_8009c378->w = DAT_8007e564[6].unk8;
    DAT_8009c378->h = DAT_8007e564[6].unka;
    DAT_8009c378->u0 = DAT_8007e564[6].unk4 + value * 16;
    DAT_8009c378->v0 = DAT_8007e564[6].unk6;
    primitive_buffer_commit_sprt(6);
}

ADDRESS(0x80026acc, 0x380)
void menu_draw_nine_slice_panel(s32 x0, s32 y0, s32 w, s32 h)
{
    MenuFont *f;
    s32 x;
    s32 y;
    s32 i;
    s32 k;
    s32 ww;
    s32 hh;
    s32 v;
    s32 wrem;
    s32 hrem;
    s32 done;
    s32 end;

    x = x0;
    y = y0;
    wrem = w - 66;
    hrem = h - 66;
    done = 0;
    for (i = 0, k = 9; i < 6; i++, k++) {
        if (i == 3) {
            k += 3;
            x = x0;
            if (hrem < 0) {
                y += 33;
            } else {
                y = y + 33 + hrem;
            }
            wrem = w - 66;
        }
        f = &DAT_8007e564[k + 3];
        if (i >= 3 && hrem < 0) {
            hh = h - 33;
            v = f->unk6 + 66 - h;
        } else {
            hh = f->unka;
            v = f->unk6;
        }
        ww = f->unk8;
        for (;;) {
            primitive_buffer_begin_poly_ft4();
            setRGB0(DAT_8009c374, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
            SetSemiTrans((void *)DAT_8009c374, DAT_801b24e4.unk100);
            DAT_8009c374->tpage = f->unk0;
            DAT_8009c374->clut = f->unk2;
            primitive_set_poly_ft4_rect(x, y, ww, hh, f->unk4, v, ww, hh);
            x += ww;
            primitive_buffer_commit_poly_ft4(0x32);
            if (i == 1 || i == 4) {
                wrem -= ww;
                if (wrem == 0) {
                    break;
                }
                if (wrem < ww) {
                    ww = wrem;
                }
            } else {
                break;
            }
        }
    }
    y = y0 + 33;
    x = x0;
    i = 11;
    while (!done) {
        i++;
        f = &DAT_8007e564[i + 3];
        hh = f->unka;
        ww = f->unk8;
        end = 0;
        if (hrem < hh) {
            hh = hrem;
        }
        wrem = w - 66;
        do {
            primitive_buffer_begin_poly_ft4();
            setRGB0(DAT_8009c374, DAT_8009c26c, DAT_8009c26c, DAT_8009c26c);
            SetSemiTrans((void *)DAT_8009c374, DAT_801b24e4.unk100);
            DAT_8009c374->tpage = f->unk0;
            DAT_8009c374->clut = f->unk2;
            primitive_set_poly_ft4_rect(x, y, ww, hh, f->unk4, f->unk6, ww, hh);
            x += ww;
            primitive_buffer_commit_poly_ft4(0x32);
            if (i != 13) {
                break;
            }
            wrem -= ww;
            if (wrem == 0) {
                end = 1;
            } else if (wrem < ww) {
                ww = wrem;
            }
        } while (!end);
        if (i == 14) {
            i = 11;
            y += 28;
            x = x0;
            hrem -= hh;
            if (hrem == 0) {
                done = 1;
            }
        }
    }
}

ADDRESS(0x80026e4c, 0x198)
void menu_draw_backdrop(s32 x, s32 y, s32 w, s32 h)
{
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, 0x40, 0x40, 0x40);
    SetSemiTrans((void *)DAT_8009c374, DAT_801b24e4.unk100);
    DAT_8009c374->tpage = 0x1c;
    DAT_8009c374->clut = 0x7b05;
    primitive_set_poly_ft4_rect(x, y, w, h, 0, 0x30, 0x80, 0xc8);
    primitive_buffer_commit_poly_ft4(0x32);
    primitive_buffer_begin_poly_ft4();
    setRGB0(DAT_8009c374, 0x40, 0x40, 0x40);
    SetSemiTrans((void *)DAT_8009c374, 1);
    DAT_8009c374->tpage = GetTPage(0, 1, 0x300, 0x100);
    DAT_8009c374->clut = 0x7b05;
    primitive_set_poly_ft4_rect(x, y, w, h, 0, 0x30, 0x80, 0xc8);
    primitive_buffer_commit_poly_ft4(0x31);
}

ADDRESS(0x80026fe4, 0x114)
void menu_frame_begin(void)
{
    DAT_80199154.frame = DAT_80199154.frame == 0;
    DAT_80199154.pool = &DAT_80199154.pools[DAT_80199154.frame];
    DAT_80199154.ot = DAT_80199154.ot_buffer[DAT_80199154.frame];
    ClearOTagR(DAT_80199154.ot, OT_LENGTH);
    DAT_80199154.pool->next = DAT_80199154.pool->base;
    if (DAT_80199154.frame == 1) {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0xf0;
    } else {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0;
    }
    if (++DAT_8009c260 == 0x60) {
        DAT_8009c260 = 0;
    }
    if (DAT_8009c268 == 0) {
        DAT_8009c260 = 0x30;
        DAT_8009c268 = -1;
    }
    if (++DAT_8009c264 == 0x60) {
        DAT_8009c264 = 0;
    }
}

ADDRESS(0x800270f8, 0xa0)
void menu_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&DAT_80199154.disp[DAT_80199154.frame]);
    PutDrawEnv(&DAT_80199154.draw[DAT_80199154.frame]);
    LoadImage(&DAT_8009c38c, (u_long *)DAT_8009c388);
    DrawOTag(DAT_80199154.ot + OT_LENGTH - 1);
}

ADDRESS(0x80027198, 0x178)
void menu_enter_display_state(void)
{
    RECT rect;

    pool_release_all();
    DAT_80199154.draw[0].isbg = 0;
    DAT_80199154.draw[0].dfe = 0;
    DAT_80199154.draw[1].isbg = 0;
    DAT_80199154.draw[1].dfe = 0;
    DAT_8009c404 = DAT_80199154.pools[0];
    DAT_8009c410 = DAT_80199154.pools[1];
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x7400;
    DAT_8009c388 = DAT_80199154.pools[1].end = DAT_80199154.pools[1].base + 0x7400;
    if (DAT_80199154.frame == 1) {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0xf0;
    } else {
        DAT_8009c38c.x = 0;
        DAT_8009c38c.y = 0;
    }
    DAT_8009c38c.w = 0x140;
    DAT_8009c38c.h = 0xf0;
    StoreImage(&DAT_8009c38c, (u_long *)DAT_8009c388);
    DrawSync(0);
    DAT_8009c384 = DAT_801b24e4.bgm_volume;
    menu_apply_display_colors();
    if (DAT_801b24e4.unkfe < 2) {
        DAT_8009c24c = 0x20;
        DAT_8009c250 = 0x40;
        DAT_8009c254 = 1;
        DAT_8009c258 = 3;
    } else {
        DAT_8009c24c = 0x40;
        DAT_8009c250 = 0x20;
        DAT_8009c254 = 3;
        DAT_8009c258 = 1;
    }
}

ADDRESS(0x80027310, 0x104)
void menu_exit_display_state(s32 mode)
{
    DAT_80199154.pools[0] = DAT_8009c404;
    DAT_80199154.pools[1] = DAT_8009c410;
    DAT_80199154.draw[0].isbg = 1;
    DAT_80199154.draw[0].dfe = 1;
    DAT_80199154.draw[1].isbg = 1;
    DAT_80199154.draw[1].dfe = 1;
    if (mode == 1) {
        audio_stop_sequence();
    } else if ((DAT_801b24e4.bgm_volume != 0 && DAT_8009c384 == 0) ||
               (DAT_801b24e4.bgm_volume == 0 && DAT_8009c384 != 0)) {
        if (DAT_801b24e4.bgm_volume == 0) {
            audio_stop_sequence();
        } else {
            audio_start_sequence();
        }
    }
}

ADDRESS(0x80027414, 0x80)
void primitive_buffer_begin_poly_ft4(void)
{
    DAT_8009c374 = (POLY_FT4 *)DAT_80199154.pool->next;
    SetPolyFT4(DAT_8009c374);
    setRGB0(DAT_8009c374, DAT_8009c270, DAT_8009c274, DAT_8009c278);
    DAT_80199154.pool->next += sizeof(POLY_FT4);
}

ADDRESS(0x80027494, 0x9c)
void primitive_set_poly_ft4_rect(s16 x, s16 y, s16 w, s16 h, s16 u, s16 v, s16 uw, s16 vh)
{
    setXY4(DAT_8009c374, x, y, x + w, y, x, y + h, x + w, y + h);
    setUV4(DAT_8009c374, u, v, u + uw, v, u, v + vh, u + uw, v + vh);
}

ADDRESS(0x80027530, 0x30)
void primitive_buffer_commit_poly_ft4(s32 depth)
{
    AddPrim((void *)(DAT_80199154.ot + depth), (void *)DAT_8009c374);
}

ADDRESS(0x80027560, 0x80)
void primitive_buffer_begin_sprt(void)
{
    DAT_8009c378 = (SPRT *)DAT_80199154.pool->next;
    SetSprt(DAT_8009c378);
    setRGB0(DAT_8009c378, DAT_8009c270, DAT_8009c274, DAT_8009c278);
    DAT_80199154.pool->next += sizeof(SPRT);
}

ADDRESS(0x800275e0, 0x30)
void primitive_buffer_commit_sprt(s32 depth)
{
    AddPrim((void *)(DAT_80199154.ot + depth), (void *)DAT_8009c378);
}

ADDRESS(0x80027610, 0x78)
void primitive_buffer_add_draw_mode(s32 depth, s16 tpage)
{
    DR_MODE *mode;

    mode = (DR_MODE *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(DR_MODE);
    SetDrawMode(mode, 0, 0, tpage, 0);
    AddPrim((void *)(DAT_80199154.ot + depth + 1), (void *)mode);
}

ADDRESS(0x80027688, 0xa0)
void menu_list_init(MenuList *list, s32 page, s32 button)
{
    u8 *src;
    u8 *dst;
    s32 i;

    dst = list->title;
    src = (&DAT_8007e660[page * 9])[button + 1].text;
    list->unk0 = 0x1f;
    list->unk2 = 0x20;
    for (i = 0; i < 16; i++) {
        *dst++ = *src++;
    }
    list->x = 0x1b;
    list->y = 0x93;
    list->count = 0;
    list->rows = 3;
    list->first = 0;
    list->cursor = 0;
    list->row = 0;
    list->unk23 = 0x18;
    DAT_8009c25c = 1;
}

ADDRESS(0x80027728, 0x98)
void menu_list_set_cursor(MenuList *list, u8 cursor)
{
    if (list->count != 0) {
        if (cursor > list->count - 1) {
            cursor = list->count - 1;
        }
        list->cursor = cursor;
        if (list->count - list->cursor < list->rows) {
            if (list->count < list->rows) {
                list->first = 0;
                list->row = list->cursor;
            } else {
                list->first = list->count - list->rows;
                list->row = list->cursor - list->first;
            }
        } else {
            list->first = cursor;
            list->row = 0;
        }
    }
}

ADDRESS(0x800277c0, 0x16c)
void menu_format_number(s32 value, s32 digits, s32 arg2, s32 arg3, u8 *out)
{
    s32 i;
    u8 fill;
    u8 *cursor;
    s16 frame_reserve[4]; /* unreferenced; reproduces retail's 8-byte frame slot */

    if (arg3 == 1 || arg3 == 2) {
        digits++;
    } else if (arg3 == 3 || arg3 == 5) {
        digits += 2;
    } else if (arg3 == 4) {
        digits += 3;
    }

    i = 0;
    fill = arg2 == 0 ? 10 : 0;
    if (digits > 0) {
        cursor = out;
        do {
            *cursor++ = fill;
            i++;
        } while (i < digits);
    }
    out[digits] = 0xff;

    if (arg3 == 1) {
        out[0] = 0x13;
    } else if (arg3 == 2) {
        out[digits - 1] = 0xd;
        digits--;
    } else if (arg3 == 3) {
        out[0] = 0xf;
        out[1] = 0x10;
    } else if (arg3 == 4) {
        out[0] = 0xc;
        out[1] = 0x12;
        out[2] = 0x10;
    } else if (arg3 == 5) {
        out[0] = 0xe;
        out[1] = 0x11;
    }

    for (i = digits - 1; i > -1; i--) {
        out[i] = value % 10;
        value /= 10;
        if (value == 0) {
            i = -1;
        }
    }
}

ADDRESS(0x8002792c, 0x78)
void menu_play_sound_cue(s32 se)
{
    if (se == 12 || se == 13 || se == 14) {
        audio_key_on(se, 0x38, 0x38, 0);
    } else if (se == 9) {
        audio_key_on(9, 0, 0x48, 0);
        VSync(0);
        VSync(0);
        audio_key_on(9, 0x48, 0, 0);
    }
}

ADDRESS(0x800279a4, 0x34)
s32 input_read_mark_active(void)
{
    u16 pad;

    pad = PadRead(1);
    if (pad != 0) {
        DAT_8009c248 = 1;
    }
    return pad;
}

ADDRESS(0x800279d8, 0x68)
void input_wait_brief_release(void)
{
    s32 i;

    i = 0;
    if (DAT_8009c248 == 1) {
        DAT_8009c248 = 0;
        while ((u16)PadRead(1) != 0) {
            if (i++ < 8) {
                VSync(0);
            } else {
                DAT_8009c260 = 0;
                break;
            }
        }
    }
}

ADDRESS(0x80027a40, 0x30)
void input_wait_release(void)
{
    DAT_8009c248 = 0;
    while ((u16)PadRead(1) != 0) {
    }
}

ADDRESS(0x80027a70, 0x2c)
void input_wait_press(void)
{
    while ((u16)PadRead(1) == 0) {
    }
}

ADDRESS(0x80027a9c, 0x24)
void menu_request_item_model(u8 item)
{
    DAT_8009c25c = 1;
    if (DAT_801b24e4.unkfb != 0) {
        DAT_8009c23c = item;
    }
}

ADDRESS(0x80027ac0, 0x9c)
void menu_start_item_model_read(void)
{
    u32 size;

    menu_release_item_model();
    size = cd_archive_entry_extent(6, DAT_8009c23c, 0);
    DAT_8009c234 = (size + 2047) >> 11;
    DAT_8009c380 = DAT_8009c37c = (u8 *)memory_allocate(size);
    DAT_8009c240 = cd_archive_start_async_read(6, DAT_8009c23c);
    DAT_8009c230 = 1;
    DAT_8009c244 = DAT_8009c23c;
    DAT_8009c23c = -1;
    DAT_8009c394.vz = 1000;
    DAT_8009c394.vx = 0;
    DAT_8009c394.vy = 0;
    DAT_8009c39c.vx = 0;
    DAT_8009c39c.vy = 0;
    DAT_8009c39c.vz = 0;
    DAT_8009c3a4 = 4;
}

ADDRESS(0x80027b5c, 0x58)
void menu_release_item_model(void)
{
    if (DAT_8009c238 == 1) {
        memory_free((void *)DAT_80199154.unk10148[3]);
    } else if (DAT_8009c230 != 0) {
        memory_free((void *)DAT_8009c37c);
    }
    DAT_8009c230 = 0;
    DAT_8009c238 = 0;
}

ADDRESS(0x80027bb4, 0xd8)
void menu_cd_install_data_ready_handler(void)
{
    cd_request_wait_idle();
    EnterCriticalSection();
    DisableEvent(DAT_801c12e4.error_event);
    DisableEvent(DAT_801c12e4.complete_event);
    DisableEvent(DAT_801c12e4.ready_event);
    CloseEvent(DAT_801c12e4.ready_event);
    DAT_801c12e4.ready_event = OpenEvent(HwCdRom, EvSpDR, EvMdINTR,
                                         (void (*)(void))menu_cd_data_ready_handler);
    EnableEvent(DAT_801c12e4.ready_event);
    EnableEvent(DAT_801c12e4.error_event);
    CdReadyCallback((void (*)(void))menu_cd_data_ready_handler);
    ExitCriticalSection();
    VSync(4);
    VSync(4);
}

ADDRESS(0x80027c8c, 0xfc)
void menu_cd_restore_data_ready_handler(void)
{
    menu_wait_async_read();
    menu_release_item_model();
    DAT_8009c23c = -1;
    EnterCriticalSection();
    DisableEvent(DAT_801c12e4.error_event);
    DisableEvent(DAT_801c12e4.ready_event);
    CloseEvent(DAT_801c12e4.ready_event);
    DAT_801c12e4.ready_event = OpenEvent(HwCdRom, EvSpDR, EvMdINTR, cd_data_ready_handler);
    EnableEvent(DAT_801c12e4.error_event);
    EnableEvent(DAT_801c12e4.complete_event);
    EnableEvent(DAT_801c12e4.ready_event);
    CdReadyCallback(cd_data_ready_handler);
    ExitCriticalSection();
    CdControl(CdlPause, 0, 0);
    VSync(4);
    VSync(4);
}

ADDRESS(0x80027d88, 0xd8)
s32 cd_archive_start_async_read(u16 file, u16 entry)
{
    CdlLOC pos;
    u8 result[8];
    u8 param[8];
    CdFile *f;
    s32 size;

    f = &DAT_801c12e4.files[file];
    size = f->sectors[entry + 1] - f->sectors[entry];
    cd_location_add(&f->extent.pos, f->sectors[entry], &pos);
    CdControl(CdlPause, 0, 0);
    param[0] = CdlModeSpeed;
    do {
        CdControl(CdlSetmode, param, 0);
    } while (CdSync(0, result) != CdlComplete);
    do {
        CdControl(CdlReadN, (u8 *)&pos, 0);
    } while (CdSync(0, result) != CdlComplete);
    return size;
}

ADDRESS(0x80027e60, 0x48)
void menu_wait_async_read(void)
{
    if (DAT_8009c230 == 1) {
        do {
            VSync(2);
        } while (DAT_8009c230 == 1);
    }
}

ADDRESS(0x80027ea8, 0x84)
void menu_cd_data_ready_handler(u8 status, u8 *data)
{
    u8 result[8];

    if (DAT_8009c230 == 1 && DAT_8009c234 > 0) {
        if (CdReady(1, result) == CdlDiskError) {
            DAT_8009c234 = 0;
            DAT_8009c230 = 3;
        } else {
            CdGetSector((void *)DAT_8009c380, 0x200);
            DAT_8009c380 += 2048;
            if (--DAT_8009c234 == 0) {
                DAT_8009c230 = 2;
            }
        }
    }
}
