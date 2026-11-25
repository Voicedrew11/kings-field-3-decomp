/*
 * GAME.EXE: the game state a memory card save holds (WIP module):
 * card_payload_capture_game_state copies it into the save data (SaveData, game/card.h) and
 * card_payload_restore_game_state copies it back. Every global is reached through lui/low
 * pairs, the menu's .sdata cursors included, so the unit is -G0.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/card.h>
#include <game/data.h>
#include <game/render.h>
#include <game/menu.h>
#include <game/funcs.h>

/* Copies the game state into the save data at data. */
ADDRESS(0x8005f7bc, 0x814)
void card_payload_capture_game_state(u8 *data)
{
    SaveData *save = (SaveData *)data;
    Record18 *r;
    u8 *p;
    s32 i;

    /* Through the byte pointer: retail moves the area list unaligned (lwl/lwr). */
    memcpy((void *)data, (const void *)asset_status.area, sizeof(asset_status.area));
    save->pos = cur_avatar_status.pos;
    save->unk56a4 = cur_avatar_status.unk12c;
    save->play_minutes = cur_avatar_status.play_minutes;
    save->experience = cur_avatar_status.experience;
    save->unk56b4 = cur_avatar_status.unk4;
    save->unk56b8 = cur_avatar_status.unk50;
    save->unk56bc = cur_avatar_status.unk8;
    save->unk56c0 = cur_avatar_status.unk160;
    save->hp_max = cur_avatar_status.hp_max;
    save->hp = cur_avatar_status.hp;
    save->unk56c6 = cur_avatar_status.unk1a;
    save->unk56c8 = cur_avatar_status.unk1c;
    save->unk56ca = cur_avatar_status.unk32;
    for (i = 4; i != -1; i--) {
        save->unk56cc[i] = cur_avatar_status.unk34[i];
        save->unk56dc[i] = cur_avatar_status.unk26[i];
    }
    save->unk56d6 = cur_avatar_status.unk3e;
    save->unk56d8 = cur_avatar_status.unk4c;
    save->unk56da = cur_avatar_status.unk24;
    save->unk56e6 = cur_avatar_status.unk30;
    save->unk56e8[0] = cur_avatar_status.unk78;
    save->unk56e8[1] = cur_avatar_status.unk7a;
    save->unk56e8[2] = cur_avatar_status.unk7c;
    save->unk56e8[3] = cur_avatar_status.unk7e;
    save->unk56e8[4] = cur_avatar_status.unk80;
    save->unk56e8[5] = cur_avatar_status.unk82;
    save->unk56e8[6] = cur_avatar_status.unk84;
    save->unk56e8[7] = cur_avatar_status.unk86;
    save->unk56e8[8] = cur_avatar_status.unk88;
    save->unk56e8[9] = cur_avatar_status.unk8a;
    save->unk56e8[10] = cur_avatar_status.unk8c;
    save->unk56e8[11] = cur_avatar_status.unk8e;
    save->unk56e8[12] = cur_avatar_status.unk90;
    save->unk56e8[13] = cur_avatar_status.unk92;
    save->unk56e8[14] = cur_avatar_status.unk94;
    save->level = cur_avatar_status.level;
    save->unk5707 = cur_avatar_status.unkd;
    save->unk5708[0] = cur_avatar_status.unkf0;
    save->unk5708[1] = cur_avatar_status.unkf1;
    save->unk5708[2] = cur_avatar_status.unkf2;
    save->unk5708[3] = cur_avatar_status.unkf3;
    save->unk5708[4] = cur_avatar_status.unkf4;
    save->unk5708[5] = cur_avatar_status.unkf5;
    save->unk5708[6] = cur_avatar_status.unkf6;
    save->unk570f = cur_avatar_status.unkc7;
    save->unk5710 = cur_avatar_status.unkc8;
    save->unk5711 = cur_avatar_status.unkc9;
    save->unk5712 = cur_avatar_status.unkcb;
    save->se_volume = cur_avatar_status.se_volume;
    save->bgm_volume = cur_avatar_status.bgm_volume;
    save->unk5715[0] = cur_avatar_status.unkf9;
    save->unk5715[1] = cur_avatar_status.unkfa;
    save->unk5715[2] = cur_avatar_status.unkfb;
    save->unk5715[3] = cur_avatar_status.unkfc;
    save->unk5715[4] = cur_avatar_status.unkfd;
    save->unk5715[5] = cur_avatar_status.unkfe;
    save->unk5715[6] = cur_avatar_status.unkff;
    save->unk5715[7] = cur_avatar_status.unk100;
    p = save->unk5634;
    r = effect_status.unk0;
    for (i = 95; i != -1; i--) {
        *p++ = r->unk0;
        r++;
    }
    memcpy((void *)save->unk5508, (const void *)event_item_counters_table, sizeof(save->unk5508));
    memcpy((void *)save->unk8, (const void *)cur_evt_state.unk4, sizeof(save->unk8));
    memcpy((void *)save->unk108, (const void *)cur_evt_state.unk104, sizeof(save->unk108));
    memcpy((void *)save->unk5108, (const void *)cur_evt_state.unk5104, sizeof(save->unk5108));
    memcpy((void *)save->unk5148, (const void *)conversation_records.unk1e0, sizeof(save->unk5148));
    memcpy((void *)save->unk571e, (const void *)map_revealed_rects, sizeof(save->unk571e));
    memcpy((void *)save->unk599e, (const void *)item_guide_level, sizeof(save->unk599e));
    memcpy((void *)save->unk5a34, (const void *)menu_shop_stock_pages[0], sizeof(save->unk5a34));
    save->unk5aca[0] = menu_item_list_cursor;
    save->unk5aca[1] = menu_magic_list_cursor;
    save->unk5aca[2] = menu_store_list_cursor;
    save->unk5aca[3] = menu_take_out_list_cursor;
}

/* Copies the game state back from the save data at data; the area list
 * comes back with parts 1 and 2 set to part 0. */
ADDRESS(0x8005ffd0, 0x830)
void card_payload_restore_game_state(u8 *data)
{
    SaveData *save = (SaveData *)data;
    Record18 *r;
    u8 *p;
    s32 i;

    /* Through the byte pointer: retail moves the area list unaligned (lwl/lwr). */
    memcpy((void *)asset_status.area, (const void *)data, sizeof(asset_status.area));
    asset_status.area[2] = asset_status.area[1] = asset_status.area[0];
    cur_avatar_status.pos = save->pos;
    cur_avatar_status.unk12c = save->unk56a4;
    cur_avatar_status.play_minutes = save->play_minutes;
    cur_avatar_status.experience = save->experience;
    cur_avatar_status.unk4 = save->unk56b4;
    cur_avatar_status.unk50 = save->unk56b8;
    cur_avatar_status.unk8 = save->unk56bc;
    cur_avatar_status.unk160 = save->unk56c0;
    cur_avatar_status.hp_max = save->hp_max;
    cur_avatar_status.hp = save->hp;
    cur_avatar_status.unk1a = save->unk56c6;
    cur_avatar_status.unk1c = save->unk56c8;
    cur_avatar_status.unk32 = save->unk56ca;
    for (i = 4; i != -1; i--) {
        cur_avatar_status.unk34[i] = save->unk56cc[i];
        cur_avatar_status.unk26[i] = save->unk56dc[i];
    }
    cur_avatar_status.unk3e = save->unk56d6;
    cur_avatar_status.unk4c = save->unk56d8;
    cur_avatar_status.unk24 = save->unk56da;
    cur_avatar_status.unk30 = save->unk56e6;
    cur_avatar_status.unk78 = save->unk56e8[0];
    cur_avatar_status.unk7a = save->unk56e8[1];
    cur_avatar_status.unk7c = save->unk56e8[2];
    cur_avatar_status.unk7e = save->unk56e8[3];
    cur_avatar_status.unk80 = save->unk56e8[4];
    cur_avatar_status.unk82 = save->unk56e8[5];
    cur_avatar_status.unk84 = save->unk56e8[6];
    cur_avatar_status.unk86 = save->unk56e8[7];
    cur_avatar_status.unk88 = save->unk56e8[8];
    cur_avatar_status.unk8a = save->unk56e8[9];
    cur_avatar_status.unk8c = save->unk56e8[10];
    cur_avatar_status.unk8e = save->unk56e8[11];
    cur_avatar_status.unk90 = save->unk56e8[12];
    cur_avatar_status.unk92 = save->unk56e8[13];
    cur_avatar_status.unk94 = save->unk56e8[14];
    cur_avatar_status.level = save->level;
    cur_avatar_status.unkd = save->unk5707;
    cur_avatar_status.unkf0 = save->unk5708[0];
    cur_avatar_status.unkf1 = save->unk5708[1];
    cur_avatar_status.unkf2 = save->unk5708[2];
    cur_avatar_status.unkf3 = save->unk5708[3];
    cur_avatar_status.unkf4 = save->unk5708[4];
    cur_avatar_status.unkf5 = save->unk5708[5];
    cur_avatar_status.unkf6 = save->unk5708[6];
    cur_avatar_status.unkc7 = save->unk570f;
    cur_avatar_status.unkc8 = save->unk5710;
    cur_avatar_status.unkc9 = save->unk5711;
    cur_avatar_status.unkcb = save->unk5712;
    cur_avatar_status.se_volume = save->se_volume;
    cur_avatar_status.bgm_volume = save->bgm_volume;
    cur_avatar_status.unkf9 = save->unk5715[0];
    cur_avatar_status.unkfa = save->unk5715[1];
    cur_avatar_status.unkfb = save->unk5715[2];
    cur_avatar_status.unkfc = save->unk5715[3];
    cur_avatar_status.unkfd = save->unk5715[4];
    cur_avatar_status.unkfe = save->unk5715[5];
    cur_avatar_status.unkff = save->unk5715[6];
    cur_avatar_status.unk100 = save->unk5715[7];
    p = save->unk5634;
    r = effect_status.unk0;
    for (i = 95; i != -1; i--) {
        r->unk0 = *p++;
        r++;
    }
    memcpy((void *)event_item_counters_table, (const void *)save->unk5508, sizeof(save->unk5508));
    memcpy((void *)cur_evt_state.unk4, (const void *)save->unk8, sizeof(save->unk8));
    memcpy((void *)cur_evt_state.unk104, (const void *)save->unk108, sizeof(save->unk108));
    memcpy((void *)cur_evt_state.unk5104, (const void *)save->unk5108, sizeof(save->unk5108));
    memcpy((void *)conversation_records.unk1e0, (const void *)save->unk5148, sizeof(save->unk5148));
    memcpy((void *)map_revealed_rects, (const void *)save->unk571e, sizeof(save->unk571e));
    memcpy((void *)item_guide_level, (const void *)save->unk599e, sizeof(save->unk599e));
    memcpy((void *)menu_shop_stock_pages[0], (const void *)save->unk5a34, sizeof(save->unk5a34));
    menu_item_list_cursor = save->unk5aca[0];
    menu_magic_list_cursor = save->unk5aca[1];
    menu_store_list_cursor = save->unk5aca[2];
    menu_take_out_list_cursor = save->unk5aca[3];
}
