/*
 * GAME.EXE: area set-up (WIP module). func_80017664 opens the archive
 * files, loads the first FDAT.T entry into the arena and copies its tables
 * out; func_8001796c asks for a full area load and steps the loader until it
 * finishes; func_80017aa4 shifts the player and every live object. Its unit
 * is -G8: the handler-table pointer is a $gp-relative .sbss word, while the
 * FDAT window base is reached absolutely (declared without a size here).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <game/sound.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/image.h>
#include <game/scene.h>
#include <game/render.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/player.h>

/* archive file numbers (func_8001a34c table slots) */
#define FILE_FDAT 4

/* area[] value that asks func_80018358 to load the part again. */
#define AREA_RELOAD 99
#define AREA_KEEP 0xff
#define SHIFT_NONE 0x7f

RODATA(0x80011000, 0x6c)

DATA(0x8009c3fc, 0x4, ".sbss")
u32 DAT_8009c3fc;

ADDRESS(0x80017664, 0x308)
void func_80017664(void)
{
    u32 *p = (u32 *)DAT_800c8954;

    DAT_8018fad4.area[1] = DAT_8018fad4.area[2] = DAT_8018fad4.area[3] = AREA_KEEP;
    DAT_8018fad4.loading = 0;
    DAT_8018fad4.step = 0;
    DAT_8018fad4.area[0] = 0;
    DAT_8018fad4.area[1] = 0;
    DAT_8018fad4.area[2] = 0;
    DAT_8018fad4.area[3] = 0;
    DAT_8018fad4.area[4] = 0;
    DAT_8018fad4.unk9 = 0;
    DAT_8009c3fc = DAT_800136e8[2] + 4;
    DAT_8009c400[0] = DAT_800136e8[2];
    func_8001a34c(0, "COM\\MO.T");
    func_8001a34c(1, "COM\\MOF.T");
    func_8001a34c(2, "COM\\VAB.T");
    func_8001a34c(3, "COM\\RTIM.T");
    func_8001a34c(4, "COM\\FDAT.T");
    func_8001a34c(5, "COM\\RTMD.T");
    func_8001a34c(6, "COM\\ITEM.T");
    func_8001a34c(7, "COM\\TALK.T");
    func_8001a34c(8, "COM\\STALK.T");
    func_8001a154(FILE_FDAT, 0x61, (u8 *)p);
    func_80018c60(FILE_FDAT, 0x60);
    func_80015ce0(2, 0, 0);
    func_80018f5c((u32 *)&DAT_8018fb3c, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c((u32 *)DAT_801d37a4, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c(DAT_801e64b8, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c((u32 *)DAT_8009f114, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c((u32 *)&DAT_801b77ec, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c((u32 *)DAT_80081c8c, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c((u32 *)&DAT_801e7edc, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c((u32 *)DAT_801b09fc.effects, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c(DAT_801983a8, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80018f5c(DAT_80116154, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80042ba8(0, (u8 *)DAT_80116154);
    func_80018f5c(DAT_800aedc4, p + 1, *p >> 2);
    func_80042ba8(0x28, (u8 *)DAT_800aedc4);
    func_8002b64c();
}

ADDRESS(0x8001796c, 0x138)
void func_8001796c(void)
{
    DAT_8018fad4.loading = 1;
    DAT_8018fad4.step = 0;
    DAT_8018fad4.previous[0] = DAT_8018fad4.area[0];
    DAT_8018fad4.previous[1] = DAT_8018fad4.area[1];
    DAT_8018fad4.previous[2] = DAT_8018fad4.area[2];
    DAT_8018fad4.previous[3] = DAT_8018fad4.area[3];
    DAT_8018fad4.previous[4] = DAT_8018fad4.area[4];
    DAT_8018fad4.area[0] = AREA_RELOAD;
    DAT_8018fad4.area[1] = AREA_RELOAD;
    DAT_8018fad4.area[2] = AREA_RELOAD;
    DAT_8018fad4.area[3] = AREA_RELOAD;
    DAT_8018fad4.area[4] = AREA_RELOAD;
    DAT_8018fad4.shift[0] = SHIFT_NONE;
    DAT_8018fad4.shift[1] = SHIFT_NONE;
    DAT_8018fad4.shift[2] = SHIFT_NONE;
    do {
        func_80019538();
        func_80018358();
    } while (DAT_8018fad4.loading != 0);
    func_80035c84(0, DAT_8014f164);
    func_80061918();
    DAT_8018fad4.handlers[5]();
}

ADDRESS(0x80017aa4, 0x1d4)
void func_80017aa4(s32 dx, s32 dy, s32 dz)
{
    Object4c *a;
    Object44 *b;
    Object88 *c;
    Effect *d;
    u16 i;
    u16 j;
    u16 k;
    u16 l;

    DAT_801b24e4.pos.vx += dx;
    DAT_801b24e4.pos.vz += dz;
    DAT_801b24e4.pos.vy += dy;
    a = DAT_801b77ec.objects;
    i = 128;
    while (i--) {
        if (a->unk0 != 0xff && (a->unk8 & 0xc) != 0xc) {
            a->x += dx;
            a->z += dz;
            a->y += dy;
        }
        a++;
    }
    b = DAT_8018fb3c.objects;
    j = 0x18c;
    while (j--) {
        if (b->unk6 != 0xff) {
            b->pos.vx += dx;
            b->pos.vz += dz;
            b->pos.vy += dy;
        }
        b++;
    }
    c = DAT_80185da8.objects;
    k = 200;
    while (k--) {
        if (c->unk0 != 0xff) {
            c->pos.vx += dx;
            c->pos.vz += dz;
            c->pos.vy += dy;
        }
        c++;
    }
    d = DAT_80182968;
    l = 128;
    while (l--) {
        if (d->id != 0xffff) {
            d->pos.vx += dx;
            d->pos.vz += dz;
            d->pos.vy += dy;
        }
        d++;
    }
}
