#ifndef GAME_CARD_H
#define GAME_CARD_H

/*
 * GAME.EXE memory card saves (src/game/card.c, WIP module). A save file is
 * BASLUS-00255 plus the save number (1 to 15) on slot 1 and holds 0x6000
 * bytes: the BIOS file header (SaveHeader), the checksum of the data at
 * +0x200, and 0x5c00 data bytes at CARD_DATA.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/kernel.h>

#define CARD_FILE_SIZE 0x6000
#define CARD_DATA 0x400
#define CARD_DATA_SIZE 0x5c00

/*
 * The game state at CARD_DATA (WIP field names): card_payload_capture_game_state fills it
 * from the globals each comment names and card_payload_restore_game_state copies it back.
 */
typedef struct SaveData {
    u8 active_resource_ids[5]; /* +0x0000: asset_status.active_resource_ids */
    u8 unk5[3];
    u8 event_flags[0x100];  /* +0x0008: cur_evt_state from flags (the script bytes) */
    u8 event_arena[0x5000]; /* +0x0108: cur_evt_state.arena, the block arena */
    u16 saved_event_offsets[32]; /* +0x5108: cur_evt_state.saved_offsets */
    u8 conversation_page_states[0x3c0]; /* +0x5148: conversation_records.page_states, the slot runs */
    u8 game_counters[2][150]; /* +0x5508: event_item_counters_table, the item counts */
    u8 magic_menu_available[96]; /* +0x5634: effect_status.magic_records[n].unk0 */
    VECTOR pos;             /* +0x5694: PlayerState.pos */
    SVECTOR camera_rotation_target; /* +0x56a4: PlayerState.camera_rotation_target */
    u32 play_minutes;       /* +0x56ac */
    s32 experience;         /* +0x56b0 */
    s32 next_level_experience; /* +0x56b4: PlayerState.next_level_experience */
    s32 gold;               /* +0x56b8: PlayerState.gold */
    s32 weapon_growth_experience; /* +0x56bc: PlayerState.weapon_growth_experience */
    u16 map_layer_offset;   /* +0x56c0: PlayerState.map_layer_offset */
    u16 hp_max;             /* +0x56c2 */
    u16 hp;                 /* +0x56c4 */
    u16 mp_max;             /* +0x56c6: PlayerState.mp_max */
    u16 mp;                 /* +0x56c8: PlayerState.mp */
    u16 base_physical_power; /* +0x56ca: PlayerState.base_physical_power */
    u16 base_magic[5];      /* +0x56cc: PlayerState.base_magic */
    s16 unk56d6;            /* +0x56d6: PlayerState.unk3e */
    u16 wisdom;             /* +0x56d8: PlayerState.wisdom */
    u16 physical_power_training; /* +0x56da: PlayerState.physical_power_training */
    u16 magic_training[5];  /* +0x56dc: PlayerState.magic_training */
    s16 unk56e6;            /* +0x56e6: PlayerState.unk30 */
    s16 status_timers[15];  /* +0x56e8: PlayerState.poison_timer..magic_boost_timer (timers and phases) */
    u8 level;               /* +0x5706 */
    u8 unk5707;             /* +0x5707: PlayerState.unkd */
    u8 equipped_ids[7];     /* +0x5708: PlayerState.equipped_head_id..equipped_extra_id */
    u8 primary_magic_shortcut_id; /* +0x570f: PlayerState.primary_magic_shortcut_id */
    u8 secondary_magic_shortcut_id; /* +0x5710: PlayerState.secondary_magic_shortcut_id */
    u8 secondary_item_shortcut_id; /* +0x5711: PlayerState.secondary_item_shortcut_id */
    u8 equipped_weapon_id;  /* +0x5712: PlayerState.equipped_weapon_id */
    u8 se_volume;           /* +0x5713 */
    u8 bgm_volume;          /* +0x5714 */
    u8 options[8];          /* +0x5715: PlayerState.vitals_display..panel_translucent */
    u8 unk571d;
    u8 revealed_map_rects[32][20]; /* +0x571e: map_revealed_rects, the rooms seen */
    u8 item_guide_levels[150]; /* +0x599e: item_guide_level */
    u8 shop_stock[150];     /* +0x5a34: menu_shop_stock_pages[0], the first shop's stock */
    u8 menu_list_cursors[4]; /* +0x5aca: the menu list cursors
                              * menu_item_list_cursor..menu_take_out_list_cursor */
    u8 unk5ace[0x132];
} SaveData;

/* The memory card file header the BIOS file browser reads. */
typedef struct SaveHeader {
    char magic[2];      /* +0x000: "SC" */
    u8 type;            /* +0x002: 0x13, a three-frame icon */
    u8 blocks;          /* +0x003 */
    char title[64];     /* +0x004: Shift-JIS; the save number, experience and level are digits in it */
    u8 unk44[0x1c];
    u16 clut[16];       /* +0x060 */
    u8 icon[3][128];    /* +0x080: 16x16 4-bit frames */
    u32 checksum;       /* +0x200: byte sum of the data */
    u8 unk204[0x7c];
} SaveHeader;

void memory_card_initialize(void);
void memory_card_shutdown_events(void);
void memory_card_start(void);
void memory_card_stop(void);
s32 memory_card_probe_temporary_file(void);
s32 memory_card_scan_save_entries(struct DIRENTRY *dir, s32 *count);
s32 memory_card_read_slot_summary(char *name, s32 *experience, s32 *level, s32 *number);
s32 memory_card_format(void);
s32 memory_card_read_slot(s32 number);
s32 memory_card_write_slot(s32 number);
void memory_card_write_title_stats(SaveHeader *header, s32 number);
u32 memory_card_payload_byte_sum(u8 *data);
s32 memory_card_wait_event(void);
void memory_card_clear_events(void);

#endif
