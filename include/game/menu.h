#ifndef GAME_MENU_H
#define GAME_MENU_H

/*
 * GAME.EXE in-game menus (src/game/menu_root.c, WIP names): the pause screen,
 * the menu's button page and the pages it opens. The helpers the pages call
 * (frame begin/present, text, windows, list input) lie past band E and are
 * declared from their call sites only.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/kernel.h>

/* Results of the menu pages (anything else is an item or action id). */
#define MENU_NONE (-99)
#define MENU_CANCEL (-1)

/*
 * The menu's glyph sets and window frame (12-byte records from 0x8007e564):
 * [0] digits, [1] text, [5] the frame menu_blit_panel_sprite draws.
 */
typedef struct MenuSpriteDef {
    s16 tpage;
    s16 clut;
    s16 u;
    s16 v;
    s16 width;
    s16 height;
} MenuSpriteDef;

#define MENU_FONT_NUMBER 0
#define MENU_FONT_TEXT 1
#define MENU_FONT_FRAME 5

/* A line of menu text: screen position and glyphs (0xff ends the line). */
typedef struct MenuGlyphString {
    s16 x;
    s16 y;
    u8 text[24];
} MenuGlyphString;

/* A map room: its rectangle on the area map (u8 x, y, w, h). */
typedef struct MapRect {
    u8 x;
    u8 y;
    u8 w;
    u8 h;
} MapRect;

/* One row of a menu list: the item name record copied out of menu_item_names. */
typedef struct MenuGlyphRow {
    u8 text[24];
} MenuGlyphRow;

/* A menu list window (menu_list_init sets it up from a button page title). */
typedef struct MenuList {
    s16 title_x;
    s16 title_y;
    u8 title[16];   /* +0x04 */
    u8 unk14[8];
    u8 x;           /* +0x1c */
    u8 y;
    u8 count;       /* +0x1e: rows in the list */
    u8 rows;        /* +0x1f: rows shown */
    u8 first;       /* +0x20: first row shown */
    u8 cursor;      /* +0x21: highlighted row */
    u8 row;         /* +0x22: highlighted row on screen */
    u8 glyphs_per_entry;
    MenuGlyphRow *entries; /* +0x24 */
    MenuGlyphRow *second_entries; /* +0x28: a second column of names */
    u8 *counts;     /* +0x2c */
    u32 *values;    /* +0x30 */
    u8 unk34[4];
} MenuList;

/* menu_root.c */
void menu_run_pause_screen(void);
s32 menu_run_root_controller(void);
void menu_draw_location_number(void);
s32 menu_item_selection_controller(void);
s32 menu_collect_masked_item_rows(u8 *bag, MenuGlyphRow *entries, u8 *counts, u8 *ids, s32 first, s32 last);
s32 menu_collect_available_item_rows(u8 *bag, MenuGlyphRow *entries, u8 *counts, u8 *ids, s32 first,
                                     s32 last);
s32 menu_collect_guide_locked_item_rows(u8 *bag, MenuGlyphRow *entries, u8 *counts, u8 *ids, s32 first,
                                        s32 last);
void menu_apply_item_effect(s32 item);
void player_clear_and_cap_status_effects(void);
void player_cap_darkness_phase(void);
void player_cap_curse_strength(void);
void menu_show_item_picture(u8 item);
void menu_draw_item_picture(s32 item);
void menu_show_area_map(void);
void menu_draw_revealed_map_rects(void);
void map_reveal_visited_rects(void);
s32 menu_choose_warp_destination(void);

/* helpers past band E (call-site signatures) */
s32 menu_choose_magic_action(void);
s32 menu_collect_available_magic_rows(MagicRecord *spells, MenuGlyphRow *entries, u32 *values, u8 *ids,
                                      s32 first, s32 last);
void menu_equipment_list_controller(void);
void menu_build_equipped_label_rows(MenuGlyphRow *out);
void menu_equipment_category_controller(s32 slot);
void menu_choose_primary_magic_shortcut(void);
void menu_item_magic_controller(void);
void menu_status_records_controller(void);
void menu_show_combat_attributes(void);
void menu_conversation_records_controller(void);
s32 menu_conversation_load_character(s32 index, s32 book, u8 *out);
void menu_conversation_load_page(s32 book, s32 page);
void menu_storage_controller(void);
void menu_storage_store_controller(void);
void menu_storage_take_out_controller(void);
s32 menu_run_system_choice(void);
s32 menu_card_load_browser(void);
s32 menu_card_build_slot_rows(struct DIRENTRY *dir, u8 *text, u32 *experience, u8 *levels, s32 *numbers);
void menu_show_dialog_panel(s32 message, MenuGlyphString *lines, s32 count, s32 x, s32 y, s32 w, s32 h,
                            s32 back);
void menu_volume_brightness_controller(void);
void menu_display_options_controller(void);
void menu_apply_display_colors(void);
void menu_button_config_controller(void);
void menu_apply_button_config(void);
s32 menu_card_load_slot_browser(struct DIRENTRY *dir);
void menu_build_card_probe_error_rows(MenuGlyphString *lines);
void menu_build_card_full_rows(MenuGlyphString *lines);
s32 menu_prompt_quit_game(void);
void menu_prepare_card_browser_rows(MenuGlyphString *line);
void menu_prepare_card_not_inserted_rows(MenuGlyphString *lines);
void menu_prepare_card_io_error_rows(MenuGlyphString *lines);
void menu_prepare_card_format_declined_rows(MenuGlyphString *lines);
void menu_prepare_card_write_full_rows(MenuGlyphString *lines);
void menu_prepare_card_write_rows(MenuGlyphString *line);
void menu_prepare_card_read_row(MenuGlyphString *line);
void menu_prepare_card_read_failure_rows(MenuGlyphString *lines, s32 error);
void menu_card_save_browser(void);
void menu_card_save_slot(s32 number);
s32 menu_confirm_card_format(void);
void menu_draw_card_dialog_rows(MenuGlyphString *lines, s32 count, s32 x, s32 y, s32 w, s32 h, s32 back);
void menu_item_buy_sell_controller(s32 shop);
void menu_item_buy_controller(s32 shop);
void menu_fill_item_counts_and_prices(u8 *bag, u8 *counts, u32 *prices, u8 *ids, s32 first, s32 end,
                                      s32 shop);
void menu_item_sell_controller(s32 shop);
void menu_fill_item_prices(u32 *prices, u8 *ids, s32 first, s32 end, s32 shop);
s32 menu_intellectual_choose_item(void);
s32 menu_blacksmith_make_item(void);
s32 menu_prompt_stay(s32 mode);
void menu_draw_gold(void);
s32 menu_poll_choice_input(s32 selected, s32 last, s32 *chosen, s32 *confirmed, s32 *result);
void menu_draw_player_status(void);
void menu_draw_window(s32 page, s32 count, s32 selected, s32 flags);
void menu_blit_panel_sprite(MenuSpriteDef *frame, MenuGlyphString *text);
void menu_draw_string(MenuSpriteDef *font, MenuGlyphString *text);
void menu_draw_number(MenuSpriteDef *font, MenuGlyphString *text);
void menu_draw_nine_slice_panel(s32 x, s32 y, s32 w, s32 h);
void menu_draw_ascii_string(s32 x, s32 y, char *text, s32 a3, s32 a4);
void menu_frame_begin(void);
void menu_present_frame(void);
void menu_enter_display_state(void);
void menu_exit_display_state(s32 a0);
void menu_play_sound_cue(s32 se);
s32 input_read_mark_active(void);
void input_wait_release(void);
void menu_draw_combat_attributes(void);
void menu_draw_volume_brightness_sliders(u8 *levels);
void menu_draw_options_rows(u8 *options);
void menu_draw_button_config(u8 style, u8 layout, s32 row);
void menu_draw_two_option(MenuGlyphString *yes, MenuGlyphString *no, s32 choice);
void menu_draw_conversation_page(s32 page);
s32 menu_preview_choice(MenuList *list, s32 kind, s32 flags, u8 item);
void menu_show_item_guide(MenuList *list, s32 a1, u8 item);
u32 menu_update_list_input(MenuList *list, u8 *ids, s32 *confirmed, s32 *result, s32 a4);
void menu_update_item_preview(u8 item);
void menu_render_list(MenuList *list, s32 flags);
void menu_list_init(MenuList *list, s32 page, s32 button);
void menu_list_set_cursor(MenuList *list, u8 cursor);
void menu_request_item_model(u8 item);
void menu_release_item_model(void);
void menu_cd_install_data_ready_handler(void);
void menu_cd_restore_data_ready_handler(void);
void menu_wait_async_read(void);
void primitive_buffer_begin_poly_ft4(void);
void primitive_set_poly_ft4_rect(s16 x, s16 y, s16 w, s16 h, s16 u, s16 v, s16 uw, s16 vh);
void primitive_buffer_commit_poly_ft4(s32 depth);
void menu_draw_button_icon(s32 x, s32 y, s32 value);
void menu_draw_backdrop(s32 x, s32 y, s32 w, s32 h);
void menu_draw_quantity(s32 big);
void menu_draw_small_glyph_string(MenuGlyphString *text, s32 shadow);
void input_wait_press(void);
void input_wait_brief_release(void);
s32 cd_archive_start_async_read(u16 file, u16 entry);
void menu_cd_data_ready_handler(u8 status, u8 *data);
void primitive_buffer_begin_sprt(void);
void primitive_buffer_commit_sprt(s32 depth);
void primitive_buffer_add_draw_mode(s32 depth, s16 tpage);
void menu_start_item_model_read(void);
void menu_blit_panel_sprite_pulsing(MenuSpriteDef *font, MenuGlyphString *text, s32 shade);
void menu_draw_string_dimmed(MenuSpriteDef *font, MenuGlyphString *text);

extern MenuSpriteDef menu_sprite_defs[];
/* The menu's text records (MenuGlyphString, 0x1c bytes): button page titles from [0],
 * yes/no positions [10] and [11], message titles from [28]. */
extern MenuGlyphString menu_window_layouts[];
extern MenuGlyphRow menu_item_names[];
extern MenuGlyphRow menu_label_rows[42];
extern MenuGlyphRow menu_magic_names[];
extern u8 item_guide_level[];
/* Shop stock counts per shop and item id, buy prices and sell prices. */
extern u8 menu_shop_stock_pages[6][150];
extern u16 menu_shop_buy_prices[6][150];
extern u16 menu_shop_sell_prices[5][150];
extern MapRect map_rects[160];

/* The two frame packet pools (scene_display_sys.pools) as the menu saved them on
 * opening (menu_enter_display_state); menu_exit_display_state puts them back. */
extern PrimPool menu_saved_primitive_buffer_0;
extern PrimPool menu_saved_primitive_buffer_1;

/* menu .sdata */
extern u8 menu_item_list_cursor;
extern u8 menu_magic_list_cursor;
extern u8 menu_store_list_cursor;
extern u8 menu_take_out_list_cursor;
extern s32 menu_cd_read_state;
extern s32 menu_cd_sectors_left;
extern s32 menu_confirm_button;
extern s32 menu_cancel_button;
extern s32 menu_confirm_button_icon;
extern s32 input_press_pending;
extern s32 menu_cursor_animation_state;
extern s32 menu_sprite_shade;
extern s32 menu_text_red;
extern s32 menu_text_green;
extern s32 menu_text_blue;
extern s32 menu_text_shadow_shade;
extern s32 menu_cancel_button_icon;
extern s32 menu_item_quantity;
extern s32 menu_cursor_animation_frame;
extern s32 menu_item_model_registered;
extern s32 menu_item_model_request;
extern s32 menu_item_model_read_sectors;
extern s32 menu_item_model_read_id;
extern s32 menu_option_pulse_frame;

/* The menu .sbss words (0x8009c374..0x8009c3a7) are statics of menu_root.c. */

#endif
