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
    memcpy((void *)data, (const void *)DAT_8018fad4.area, sizeof(DAT_8018fad4.area));
    save->pos = DAT_801b24e4.pos;
    save->unk56a4 = DAT_801b24e4.unk12c;
    save->play_minutes = DAT_801b24e4.play_minutes;
    save->experience = DAT_801b24e4.experience;
    save->unk56b4 = DAT_801b24e4.unk4;
    save->unk56b8 = DAT_801b24e4.unk50;
    save->unk56bc = DAT_801b24e4.unk8;
    save->unk56c0 = DAT_801b24e4.unk160;
    save->hp_max = DAT_801b24e4.hp_max;
    save->hp = DAT_801b24e4.hp;
    save->unk56c6 = DAT_801b24e4.unk1a;
    save->unk56c8 = DAT_801b24e4.unk1c;
    save->unk56ca = DAT_801b24e4.unk32;
    for (i = 4; i != -1; i--) {
        save->unk56cc[i] = DAT_801b24e4.unk34[i];
        save->unk56dc[i] = DAT_801b24e4.unk26[i];
    }
    save->unk56d6 = DAT_801b24e4.unk3e;
    save->unk56d8 = DAT_801b24e4.unk4c;
    save->unk56da = DAT_801b24e4.unk24;
    save->unk56e6 = DAT_801b24e4.unk30;
    save->unk56e8[0] = DAT_801b24e4.unk78;
    save->unk56e8[1] = DAT_801b24e4.unk7a;
    save->unk56e8[2] = DAT_801b24e4.unk7c;
    save->unk56e8[3] = DAT_801b24e4.unk7e;
    save->unk56e8[4] = DAT_801b24e4.unk80;
    save->unk56e8[5] = DAT_801b24e4.unk82;
    save->unk56e8[6] = DAT_801b24e4.unk84;
    save->unk56e8[7] = DAT_801b24e4.unk86;
    save->unk56e8[8] = DAT_801b24e4.unk88;
    save->unk56e8[9] = DAT_801b24e4.unk8a;
    save->unk56e8[10] = DAT_801b24e4.unk8c;
    save->unk56e8[11] = DAT_801b24e4.unk8e;
    save->unk56e8[12] = DAT_801b24e4.unk90;
    save->unk56e8[13] = DAT_801b24e4.unk92;
    save->unk56e8[14] = DAT_801b24e4.unk94;
    save->level = DAT_801b24e4.level;
    save->unk5707 = DAT_801b24e4.unkd;
    save->unk5708[0] = DAT_801b24e4.unkf0;
    save->unk5708[1] = DAT_801b24e4.unkf1;
    save->unk5708[2] = DAT_801b24e4.unkf2;
    save->unk5708[3] = DAT_801b24e4.unkf3;
    save->unk5708[4] = DAT_801b24e4.unkf4;
    save->unk5708[5] = DAT_801b24e4.unkf5;
    save->unk5708[6] = DAT_801b24e4.unkf6;
    save->unk570f = DAT_801b24e4.unkc7;
    save->unk5710 = DAT_801b24e4.unkc8;
    save->unk5711 = DAT_801b24e4.unkc9;
    save->unk5712 = DAT_801b24e4.unkcb;
    save->se_volume = DAT_801b24e4.se_volume;
    save->bgm_volume = DAT_801b24e4.bgm_volume;
    save->unk5715[0] = DAT_801b24e4.unkf9;
    save->unk5715[1] = DAT_801b24e4.unkfa;
    save->unk5715[2] = DAT_801b24e4.unkfb;
    save->unk5715[3] = DAT_801b24e4.unkfc;
    save->unk5715[4] = DAT_801b24e4.unkfd;
    save->unk5715[5] = DAT_801b24e4.unkfe;
    save->unk5715[6] = DAT_801b24e4.unkff;
    save->unk5715[7] = DAT_801b24e4.unk100;
    p = save->unk5634;
    r = DAT_801b77ec.unk0;
    for (i = 95; i != -1; i--) {
        *p++ = r->unk0;
        r++;
    }
    memcpy((void *)save->unk5508, (const void *)DAT_800c85e8, sizeof(save->unk5508));
    memcpy((void *)save->unk8, (const void *)DAT_801ba984.unk4, sizeof(save->unk8));
    memcpy((void *)save->unk108, (const void *)DAT_801ba984.unk104, sizeof(save->unk108));
    memcpy((void *)save->unk5108, (const void *)DAT_801ba984.unk5104, sizeof(save->unk5108));
    memcpy((void *)save->unk5148, (const void *)DAT_801983a8.unk1e0, sizeof(save->unk5148));
    memcpy((void *)save->unk571e, (const void *)DAT_801b56cc, sizeof(save->unk571e));
    memcpy((void *)save->unk599e, (const void *)DAT_80081780, sizeof(save->unk599e));
    memcpy((void *)save->unk5a34, (const void *)DAT_80080718[0], sizeof(save->unk5a34));
    save->unk5aca[0] = DAT_8009c22c;
    save->unk5aca[1] = DAT_8009c22d;
    save->unk5aca[2] = DAT_8009c22e;
    save->unk5aca[3] = DAT_8009c22f;
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
    memcpy((void *)DAT_8018fad4.area, (const void *)data, sizeof(DAT_8018fad4.area));
    DAT_8018fad4.area[2] = DAT_8018fad4.area[1] = DAT_8018fad4.area[0];
    DAT_801b24e4.pos = save->pos;
    DAT_801b24e4.unk12c = save->unk56a4;
    DAT_801b24e4.play_minutes = save->play_minutes;
    DAT_801b24e4.experience = save->experience;
    DAT_801b24e4.unk4 = save->unk56b4;
    DAT_801b24e4.unk50 = save->unk56b8;
    DAT_801b24e4.unk8 = save->unk56bc;
    DAT_801b24e4.unk160 = save->unk56c0;
    DAT_801b24e4.hp_max = save->hp_max;
    DAT_801b24e4.hp = save->hp;
    DAT_801b24e4.unk1a = save->unk56c6;
    DAT_801b24e4.unk1c = save->unk56c8;
    DAT_801b24e4.unk32 = save->unk56ca;
    for (i = 4; i != -1; i--) {
        DAT_801b24e4.unk34[i] = save->unk56cc[i];
        DAT_801b24e4.unk26[i] = save->unk56dc[i];
    }
    DAT_801b24e4.unk3e = save->unk56d6;
    DAT_801b24e4.unk4c = save->unk56d8;
    DAT_801b24e4.unk24 = save->unk56da;
    DAT_801b24e4.unk30 = save->unk56e6;
    DAT_801b24e4.unk78 = save->unk56e8[0];
    DAT_801b24e4.unk7a = save->unk56e8[1];
    DAT_801b24e4.unk7c = save->unk56e8[2];
    DAT_801b24e4.unk7e = save->unk56e8[3];
    DAT_801b24e4.unk80 = save->unk56e8[4];
    DAT_801b24e4.unk82 = save->unk56e8[5];
    DAT_801b24e4.unk84 = save->unk56e8[6];
    DAT_801b24e4.unk86 = save->unk56e8[7];
    DAT_801b24e4.unk88 = save->unk56e8[8];
    DAT_801b24e4.unk8a = save->unk56e8[9];
    DAT_801b24e4.unk8c = save->unk56e8[10];
    DAT_801b24e4.unk8e = save->unk56e8[11];
    DAT_801b24e4.unk90 = save->unk56e8[12];
    DAT_801b24e4.unk92 = save->unk56e8[13];
    DAT_801b24e4.unk94 = save->unk56e8[14];
    DAT_801b24e4.level = save->level;
    DAT_801b24e4.unkd = save->unk5707;
    DAT_801b24e4.unkf0 = save->unk5708[0];
    DAT_801b24e4.unkf1 = save->unk5708[1];
    DAT_801b24e4.unkf2 = save->unk5708[2];
    DAT_801b24e4.unkf3 = save->unk5708[3];
    DAT_801b24e4.unkf4 = save->unk5708[4];
    DAT_801b24e4.unkf5 = save->unk5708[5];
    DAT_801b24e4.unkf6 = save->unk5708[6];
    DAT_801b24e4.unkc7 = save->unk570f;
    DAT_801b24e4.unkc8 = save->unk5710;
    DAT_801b24e4.unkc9 = save->unk5711;
    DAT_801b24e4.unkcb = save->unk5712;
    DAT_801b24e4.se_volume = save->se_volume;
    DAT_801b24e4.bgm_volume = save->bgm_volume;
    DAT_801b24e4.unkf9 = save->unk5715[0];
    DAT_801b24e4.unkfa = save->unk5715[1];
    DAT_801b24e4.unkfb = save->unk5715[2];
    DAT_801b24e4.unkfc = save->unk5715[3];
    DAT_801b24e4.unkfd = save->unk5715[4];
    DAT_801b24e4.unkfe = save->unk5715[5];
    DAT_801b24e4.unkff = save->unk5715[6];
    DAT_801b24e4.unk100 = save->unk5715[7];
    p = save->unk5634;
    r = DAT_801b77ec.unk0;
    for (i = 95; i != -1; i--) {
        r->unk0 = *p++;
        r++;
    }
    memcpy((void *)DAT_800c85e8, (const void *)save->unk5508, sizeof(save->unk5508));
    memcpy((void *)DAT_801ba984.unk4, (const void *)save->unk8, sizeof(save->unk8));
    memcpy((void *)DAT_801ba984.unk104, (const void *)save->unk108, sizeof(save->unk108));
    memcpy((void *)DAT_801ba984.unk5104, (const void *)save->unk5108, sizeof(save->unk5108));
    memcpy((void *)DAT_801983a8.unk1e0, (const void *)save->unk5148, sizeof(save->unk5148));
    memcpy((void *)DAT_801b56cc, (const void *)save->unk571e, sizeof(save->unk571e));
    memcpy((void *)DAT_80081780, (const void *)save->unk599e, sizeof(save->unk599e));
    memcpy((void *)DAT_80080718[0], (const void *)save->unk5a34, sizeof(save->unk5a34));
    DAT_8009c22c = save->unk5aca[0];
    DAT_8009c22d = save->unk5aca[1];
    DAT_8009c22e = save->unk5aca[2];
    DAT_8009c22f = save->unk5aca[3];
}
