/*
 * GAME.EXE: an Object88's talk script (func_8005c308), an Object44 spawn
 * helper and the player's move to one of 14 start positions (areas loaded
 * through func_80017c78) (WIP module). The .rdata is func_8005c864's local
 * table initializer, then func_8005c308's jump table: GCC 2.6 emits the
 * initializers of a TU ahead of its jump tables, so the two share a TU.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/cd.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/scene.h>

RODATA(0x800130f0, 0xb0)

/* Runs obj's talk script record (id 0x70) from its entry point up to the 0xff end, then its end action. */
ADDRESS(0x8005c308, 0x4e0)
void func_8005c308(Object88 *obj)
{
    Object78 *def;
    u8 *script;
    u8 *p;
    s32 repeat;
    s32 posed;
    u8 anim;
    s32 n;
    u8 back;

    def = &DAT_80185da8.defs[obj->unk2];
    script = def->scripts[0];
    if (script == NULL || script[0] != 0x70) {
        return;
    }
    posed = 0;
    repeat = 0;
    if (script[16] == 0) {
        DAT_801ba984.unkaa = obj->unk1;
    }
    p = func_8005c1e8(obj);
    if (DAT_801ba984.unkaa != obj->unk1 && script[19] == 1) {
        while (*p++ != 0xf0)
            ;
        p++;
        script[16] = p - (script + 20);
        script[19] = 0;
    }
    for (;;) {
        switch (*p) {
        case 0xf0:
            script[19] = 1;
        case 0xf8:
            back = p[1];
            script[16] -= back;
            p -= back;
            continue;
        case 0xf9:
            if (DAT_801ba984.unk4[p[1]] == p[2]) {
                script[16] = func_8005c18c(script, p[3]);
                p = script + (script[16] + 20);
            } else {
                script[16] += 4;
                p += 4;
            }
            continue;
        case 0xf2:
            script[16] += 2;
            p += 2;
            continue;
        case 0xf4:
            script[16]++;
            p++;
            DAT_8018fad4.handlers[4](obj, *p);
            break;
        case 0xf5:
            script[16]++;
            p++;
            repeat = *p;
            break;
        case 0xf6:
            DAT_801ba984.unkaa = obj->unk1;
            p++;
            script[19] = 0;
            script[16]++;
            continue;
        case 0xf7:
            DAT_801ba984.unk4[p[1]] = p[2];
            p += 2;
            script[16] += 2;
            break;
        case 0xf3:
            break;
        case 0xff:
            goto end;
        default:
            if (!posed && script[1] != 0xff) {
                anim = obj->unkc;
                posed = 1;
                if (obj->unk18 != 0) {
                    func_8005c0d4(obj, obj->unkc, obj->unk18, 0, obj->unk66);
                }
                func_8005c0d4(obj, script[1], 0, 0xfff, *(u16 *)(script + 8));
            }
            if (def->unk0 != 0x2b) {
                func_800608ec(def->unk0, *p);
            }
            func_800422b8(NULL, NULL);
            func_800441d4(7, *(u16 *)(script + 12) + *p);
            if ((DAT_801b24e4.unk178 & DAT_80081868[7]) && script[18] == 0xff) {
                repeat++;
            }
            break;
        }
        script[16]++;
        p++;
        if (repeat == 0) {
            break;
        }
        repeat--;
    }
end:
    switch (script[18] & 0xf0) {
    case 0x00:
        func_80030568();
        func_80021114(script[18] & 0xf);
        break;
    case 0x10:
    case 0x20:
        func_80030568();
        break;
    case 0x30:
        if (DAT_801b24e4.unk4c == 4) {
            func_80030568();
            n = func_80021aac();
            if (n != -1) {
                func_800422b8(NULL, NULL);
                func_800441d4(6, n + 488);
            }
        }
        break;
    case 0x40:
        func_80030568();
        if (func_8002200c(script[18] & 0xf) != -1) {
            DAT_801e7edc.unk370 = 1;
            DAT_801e7edc.unk384 = script[18] & 3;
        }
        break;
    }
    DAT_801ba984.unkaa = obj->unk1;
    if (posed && script[17] != 0xff) {
        func_8005c0d4(obj, script[17], 0, 0xfff, *(u16 *)(script + 14));
        obj->unkc = anim;
    }
    func_8002b6f0();
    DAT_801ba984.unk0 = 1;
}

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
