/*
 * GAME.EXE: between the switches func_8005c308 and func_8005cbe0 (WIP
 * module): an Object44 spawn helper and the player's move to one of 14
 * start positions (areas loaded through func_80017c78). The local table's
 * initializer is this unit's .rdata.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/cd.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/player.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/scene.h>

RODATA(0x800130f0, 0x70)

/* Spawns Object44 0x17c and links obj to it (its index - 124). */
ADDRESS(0x8005c7e8, 0x7c)
void func_8005c7e8(Object44 *obj, s16 id)
{
    Object44 *made;
    s32 i;

    made = func_80046034(0x17c, 0x10, -1);
    obj->arg.b.unk39 = made - DAT_8018fb3c.objects - 124;
    made->unk6 = id;
    made->arg.b.unk38 = 0;
}

/* Loads start position n's areas and puts the player there (12: none). */
ADDRESS(0x8005c864, 0x37c)
void func_8005c864(s32 n)
{
    Record08 table[14] = {
        {0, 0, 0, 10, 56, 100, 2048},
        {1, 4, 5, 32, 14, 100, 0},
        {2, 2, 2, 57, 36, 100, 0},
        {3, 16, 18, 7, 31, 100, 0},
        {4, 13, 15, 56, 13, 100, 2048},
        {5, 4, 5, 29, 31, 100, 0},
        {6, 17, 19, 34, 67, 100, 2048},
        {7, 7, 9, 19, 29, 104, 0},
        {255, 255, 255, 10, 56, 100, 0},
        {9, 5, 7, 39, 56, 100, 2048},
        {10, 6, 8, 37, 51, 204, 2048},
        {11, 6, 8, 58, 34, 100, 0},
        {27, 9, 11, 40, 35, 100, 0},
        {24, 9, 11, 7, 70, 200, 0},
    };
    Record08 *rec;
    s32 yaw;
    s32 z;

    rec = &table[n];
    if (n < 12 || n == 13) {
        if (rec->area == 0xff) {
            return;
        }
        func_80046c00(1, 0, 0x1000, 0x100);
        func_8004b794();
    }
    do {
        func_80019538();
        func_80018358();
    } while ((s16)DAT_8018fad4.loading != 0);
    if (rec->area != DAT_8018fad4.area[0]) {
        func_80017c78(rec->area, rec->area, rec->area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
    } else {
        func_80017c78(0xff, 0xff, rec->area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
        DAT_801b24e4.unke = 1;
    }
    do {
        func_80019538();
        func_80018358();
    } while ((s16)DAT_8018fad4.loading != 0);
    DAT_801b24e4.unk101 = 0;
    DAT_801b24e4.unk13c = DAT_801b24e4.unk13e = DAT_801b24e4.unk140 = 0;
    DAT_801b24e4.unk124 = DAT_801b24e4.unk12c;
    DAT_801b24e4.pos.vx = (rec->x << 11) + 1024;
    z = rec->z << 11;
    DAT_801b24e4.pos.vz = z + 1024;
    DAT_801b24e4.pos.vy = -(rec->y << 7);
    yaw = -rec->yaw;
    DAT_801b24e4.unk124.vy = DAT_801b24e4.unk12c.vy = yaw;
    if (n == 13) {
        DAT_801b24e4.unk124.vx = DAT_801b24e4.unk12c.vx = DAT_801b24e4.unk124.vz =
            DAT_801b24e4.unk12c.vz = 0;
        DAT_801b24e4.pos.vz = z + 2000;
    } else {
        func_80046c00(1, 0x1000, 0x1000, 0);
        func_80019ab4();
        func_80046c00(1, 0x1000, 0, -256);
        func_8003d624(0xff, 0, 0, 0);
    }
    if (n != 13) {
        func_80017c78(0xff, 0xff, 0xff, rec->area3, rec->area4, 0x7f, 0x7f, 0x7f);
    }
    if (n < 12) {
        DAT_801b24e4.unk10a = 1;
        DAT_801ba984.unk0 = 1;
        func_80029500();
    }
    DAT_801b24e4.unk134 = DAT_801b24e4.unk136 = DAT_801b24e4.unk138 = 0;
}
