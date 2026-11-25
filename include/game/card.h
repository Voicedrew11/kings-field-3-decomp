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
    u8 area[5];             /* +0x0000: asset_status.area */
    u8 unk5[3];
    u8 unk8[0x100];         /* +0x0008: cur_evt_state from unk4 (the script bytes) */
    u8 unk108[0x5000];      /* +0x0108: cur_evt_state.unk104, the block arena */
    u16 unk5108[32];        /* +0x5108: cur_evt_state.unk5104 */
    u8 unk5148[0x3c0];      /* +0x5148: conversation_records.unk1e0, the slot runs */
    u8 unk5508[2][150];     /* +0x5508: event_item_counters_table, the item counts */
    u8 unk5634[96];         /* +0x5634: effect_status.unk0[n].unk0 */
    VECTOR pos;             /* +0x5694: Data801b24e4.pos */
    SVECTOR unk56a4;        /* +0x56a4: Data801b24e4.unk12c */
    u32 play_minutes;       /* +0x56ac */
    s32 experience;         /* +0x56b0 */
    s32 unk56b4;            /* +0x56b4: Data801b24e4.unk4 */
    s32 unk56b8;            /* +0x56b8: Data801b24e4.unk50 */
    s32 unk56bc;            /* +0x56bc: Data801b24e4.unk8 */
    u16 unk56c0;            /* +0x56c0: Data801b24e4.unk160 */
    u16 hp_max;             /* +0x56c2 */
    u16 hp;                 /* +0x56c4 */
    u16 unk56c6;            /* +0x56c6: Data801b24e4.unk1a */
    u16 unk56c8;            /* +0x56c8: Data801b24e4.unk1c */
    u16 unk56ca;            /* +0x56ca: Data801b24e4.unk32 */
    u16 unk56cc[5];         /* +0x56cc: Data801b24e4.unk34 */
    s16 unk56d6;            /* +0x56d6: Data801b24e4.unk3e */
    u16 unk56d8;            /* +0x56d8: Data801b24e4.unk4c */
    u16 unk56da;            /* +0x56da: Data801b24e4.unk24 */
    u16 unk56dc[5];         /* +0x56dc: Data801b24e4.unk26 */
    s16 unk56e6;            /* +0x56e6: Data801b24e4.unk30 */
    s16 unk56e8[15];        /* +0x56e8: Data801b24e4.unk78..unk94 */
    u8 level;               /* +0x5706 */
    u8 unk5707;             /* +0x5707: Data801b24e4.unkd */
    u8 unk5708[7];          /* +0x5708: Data801b24e4.unkf0..unkf6 */
    u8 unk570f;             /* +0x570f: Data801b24e4.unkc7 */
    u8 unk5710;             /* +0x5710: Data801b24e4.unkc8 */
    u8 unk5711;             /* +0x5711: Data801b24e4.unkc9 */
    u8 unk5712;             /* +0x5712: Data801b24e4.unkcb */
    u8 se_volume;           /* +0x5713 */
    u8 bgm_volume;          /* +0x5714 */
    u8 unk5715[8];          /* +0x5715: Data801b24e4.unkf9..unk100 */
    u8 unk571d;
    u8 unk571e[32][20];     /* +0x571e: map_revealed_rects, the rooms seen */
    u8 unk599e[150];        /* +0x599e: item_guide_level */
    u8 unk5a34[150];        /* +0x5a34: menu_shop_stock_pages[0], the first shop's stock */
    u8 unk5aca[4];          /* +0x5aca: the menu list cursors menu_item_list_cursor..menu_take_out_list_cursor */
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
