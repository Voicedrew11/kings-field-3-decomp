/*
 * FDAT_002.BIN (WIP module): the handler table and the switch table of its
 * slot 4 handler, an empty default handler, the shared slot 0 handler (this
 * module's copy is the claim), a swinging object and distance check (slot
 * 1), record handlers (slots 18 and 19), a switch handler (slot 3), an item
 * check (slot 6), an event handler (slot 7), a record event handler (slot
 * 4) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0xa0)

/* GAME.EXE .sdata word (owner unit unknown): the swing step of object 158. */
extern s32 DAT_8009c21c;

void func_801e83a8(void);
void func_801e85b8(void);
void func_801e8704(void);
void func_801e870c(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
                   s32 arg7, s32 arg8, s32 arg9);
void func_801e8768(Object88 *obj);
void func_801e87e0(Object44 *obj);
void func_801e8960(void);
void func_801e8968(void);
void func_801e8970(void);
void func_801e8978(void);
void func_801e8980(void);
void func_801e89d0(void);
void func_801e89d8(u8 id);
void func_801e8a34(Object88 *record, s32 event);
void func_801e8d68(void);

const FdatHeader fdat_header = {
    0x5,
    {
        fdat_801e83b0, func_801e85b8, func_801e8704, func_801e87e0,
        func_801e8a34, func_801e8d68, func_801e8980, func_801e89d8,
        func_801e8960, func_801e8968, func_801e89d0, func_801e83a8,
        func_801e83a8, func_801e83a8, func_801e83a8, func_801e83a8,
        func_801e8970, func_801e8978, func_801e870c, func_801e8768,
        func_801e83a8, func_801e83a8, func_801e83a8, func_801e83a8,
        func_801e83a8, func_801e83a8, func_801e83a8, func_801e83a8,
        func_801e83a8, func_801e83a8, func_801e83a8, func_801e83a8,
    },
    0x25,
};

ADDRESS(0x801e83a8, 0x8)
void func_801e83a8(void)
{
}

#include "shared/fdat_801e83b0.inc"

/*
 * Slot 1: swings object 158 back and forth; with flags 5 and 6 set and 0xb
 * clear, records 0..2 move on once record 1 is far from the player.
 */
ADDRESS(0x801e85b8, 0x14c)
void func_801e85b8(void)
{
    s32 k = 158;
    s32 dx;
    s32 dz;

    if (rand() < 8192) {
        DAT_8018fb3c.objects[k].unk10 += DAT_8009c21c * 16;
    }
    if (DAT_8018fb3c.objects[k].unk10 > 2048) {
        DAT_8009c21c = -DAT_8009c21c;
        if (DAT_8018fb3c.objects[k].unk10 >= 4096) {
            DAT_8018fb3c.objects[k].unk10 = 0;
        }
    }
    if (DAT_801ba984.unk4[1] != 0 && DAT_801ba984.unk4[2] != 0 && DAT_801ba984.unk4[7] == 0) {
        dx = (DAT_80185da8.objects[1].pos.vx - DAT_801b24e4.pos.vx) / 2048;
        dz = (DAT_80185da8.objects[1].pos.vz - DAT_801b24e4.pos.vz) / 2048;
        /* ^ is exclusive or here, as written. */
        if ((dx ^ 2 + dz ^ 2) > 400) {
            DAT_80185da8.objects[0].unk0 = 8;
            DAT_80185da8.objects[1].unk0 = 8;
            DAT_80185da8.objects[2].unk0 = 1;
        }
    }
}

ADDRESS(0x801e8704, 0x8)
void func_801e8704(void)
{
}

/* Slot 18: record 32 reports 0 while its unk1a is above the value. */
ADDRESS(0x801e870c, 0x5c)
void func_801e870c(Object88 *obj, s32 *out, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
                   s32 arg7, s32 arg8, s32 arg9)
{
    if (obj - DAT_80185da8.objects == 32 && *out < obj->unk1a) {
        *out = 0;
    }
}

/* Slot 19: record 32 sets flag 0xc once and opens object 344. */
ADDRESS(0x801e8768, 0x78)
void func_801e8768(Object88 *obj)
{
    s32 k;

    if (obj - DAT_80185da8.objects == 32) {
        if (DAT_801ba984.unk4[8] == 0) {
            DAT_801ba984.unk4[8] = 1;
            k = 344;
            if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xf1) {
                DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
            }
        }
    }
}

/*
 * Slot 3: switch 0 (with flag 0xa set and 0xb clear) starts records 0..2 and
 * sets the doors, then, like switch 10, shows one of two notes.
 */
ADDRESS(0x801e87e0, 0x180)
void func_801e87e0(Object44 *obj)
{
    Object44 *door;

    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0:
        if (DAT_801ba984.unk4[6] == 1) {
            if (DAT_801ba984.unk4[7] == 0) {
            DAT_80185da8.objects[0].unk0 = 8;
            DAT_80185da8.objects[1].unk0 = 8;
            DAT_80185da8.objects[2].unk0 = 8;
            door = &DAT_8018fb3c.objects[214];
            door->unk0 = 2;
            DAT_8018fb3c.objects[214].arg.b.unk3c = 0xff;
            DAT_8018fb3c.objects[177].arg.b.unk3d[1] = 0xff;
            door = &DAT_8018fb3c.objects[111];
            door->unk0 = 0;
            DAT_8018fb3c.objects[111].arg.b.unk3c = 0;
            DAT_8018fb3c.objects[182].arg.b.unk3d[1] = 0;
            door = &DAT_8018fb3c.objects[62];
            door->unk0 = 2;
            DAT_8018fb3c.objects[62].arg.b.unk38 = 0xff;
            door = &DAT_8018fb3c.objects[195];
            door->unk0 = 2;
            DAT_8018fb3c.objects[195].arg.b.unk38 = 0xff;
            door = &DAT_8018fb3c.objects[172];
            door->unk0 = 2;
            DAT_8018fb3c.objects[172].arg.b.unk38 = 0xff;
            }
        }
    case 10:
        if (func_80046884(9, 0x37, 3, 3, 0x8000) != 0) {
            if (DAT_801b24e4.unk10a != 0) {
                func_800445b8(2, 0x4a, 0, 9, 0x37, 3, 1, 0, 0x2f);
            }
        } else {
            DAT_801b24e4.unk10a = 0;
            func_800445b8(2, 0x4d, 0, 9, 0x37, 3, 1, 0, 0x2f);
        }
        break;
    }
}

ADDRESS(0x801e8960, 0x8)
void func_801e8960(void)
{
}

ADDRESS(0x801e8968, 0x8)
void func_801e8968(void)
{
}

ADDRESS(0x801e8970, 0x8)
void func_801e8970(void)
{
}

ADDRESS(0x801e8978, 0x8)
void func_801e8978(void)
{
}

/* Slot 6: sets flag 0xb once items 0, 0x85 and 0x64 are all held. */
ADDRESS(0x801e8980, 0x50)
void func_801e8980(void)
{
    if (func_8005d7bc(0) > 0 && func_8005d7bc(0x85) > 0 && func_8005d7bc(0x64) > 0) {
        DAT_801ba984.unk4[7] = 1;
    }
}

ADDRESS(0x801e89d0, 0x8)
void func_801e89d0(void)
{
}

/* Slot 7: event 0 restores the player's hp and raises unk1c by 30 up to its maximum. */
ADDRESS(0x801e89d8, 0x5c)
void func_801e89d8(u8 id)
{
    if (id == 0) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
        DAT_801b24e4.unk1c += 30;
        if (DAT_801b24e4.unk1c > DAT_801b24e4.unk1a) {
            DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
        }
    }
}

/* Slot 4: script events 0..5 of an object record. */
ADDRESS(0x801e8a34, 0x334)
void func_801e8a34(Object88 *record, s32 event)
{
    u32 minutes;
    s32 hundreds;

    switch (event) {
    case 0:
        if (func_8005d7bc(2) > 0 && func_8005d7bc(0x82) > 0 && func_8005d7bc(0x83) > 0 &&
            func_8005d7bc(0x84) > 0) {
            DAT_801ba984.unk4[3] = 1;
        } else {
            DAT_801ba984.unk4[3] = 0;
        }
        break;
    case 1:
        if (func_8005d7bc(0x82) > 0 && func_8005d7bc(0x83) > 0 && func_8005d7bc(0x84) > 0 &&
            func_8005d7bc(2) > 0) {
            DAT_801ba984.unk4[1] = 1;
            DAT_801ba984.unk0 = 1;
            func_8005d7f8(0x82);
            func_8005d7f8(0x83);
            func_8005d7f8(0x84);
            func_8005d7f8(2);
            if (DAT_801b24e4.unkcb == 2) {
                func_8002bdc0(0xff);
            }
        }
        break;
    case 2:
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_801ba984.unk4[2] = 1;
        DAT_80185da8.objects[2].unk0 = 1;
        DAT_801ba984.unk4[4] = DAT_801b24e4.play_minutes % 60;
        DAT_801ba984.unk4[5] = DAT_801b24e4.experience % 10000 / 100;
        break;
    case 4:
        minutes = DAT_801b24e4.play_minutes % 60;
        if (minutes < DAT_801ba984.unk4[4] || minutes >= DAT_801ba984.unk4[4] + 10) {
            DAT_801ba984.unk4[6] = 1;
        }
        hundreds = DAT_801b24e4.experience % 10000 / 100;
        if (hundreds < DAT_801ba984.unk4[5] || hundreds >= DAT_801ba984.unk4[5] + 10) {
            DAT_801ba984.unk4[6] = 1;
        }
        break;
    case 5:
        if (func_8005d7bc(0x82) > 0 && func_8005d7bc(0x83) > 0 && func_8005d7bc(0x84) > 0) {
            DAT_801ba984.unk4[1] = 1;
        } else {
            DAT_801ba984.unk4[1] = 0;
        }
        break;
    }
}

/* Slot 5, called after the module loads: restores the records and doors of the flags. */
ADDRESS(0x801e8d68, 0x39c)
void func_801e8d68(void)
{
    Object44 *door;
    s32 k;

    if (DAT_801ba984.unke[0xc] != 0) {
        k = 70;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        k = 33;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
        k = 116;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xc8) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
    }
    if (DAT_801ba984.unke[0x14] == 0) {
        DAT_80185da8.objects[0].unk0 = 1;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        DAT_8018fb3c.objects[214].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[177].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        DAT_8018fb3c.objects[111].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[182].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        DAT_8018fb3c.objects[62].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        DAT_8018fb3c.objects[195].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        DAT_8018fb3c.objects[172].arg.b.unk38 = 0;
    } else if (DAT_801ba984.unk4[2] == 0) {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 1;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        DAT_8018fb3c.objects[214].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[177].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        DAT_8018fb3c.objects[111].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[182].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        DAT_8018fb3c.objects[62].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        DAT_8018fb3c.objects[195].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        DAT_8018fb3c.objects[172].arg.b.unk38 = 0;
    } else if (DAT_801ba984.unk4[6] == 0) {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 1;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        DAT_8018fb3c.objects[214].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[177].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        DAT_8018fb3c.objects[111].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[182].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        DAT_8018fb3c.objects[62].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        DAT_8018fb3c.objects[195].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        DAT_8018fb3c.objects[172].arg.b.unk38 = 0;
    } else if (DAT_801ba984.unk4[7] == 0) {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 2;
        DAT_8018fb3c.objects[214].arg.b.unk3c = 0xff;
        DAT_8018fb3c.objects[177].arg.b.unk3d[1] = 0xff;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 0;
        DAT_8018fb3c.objects[111].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[182].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 2;
        DAT_8018fb3c.objects[62].arg.b.unk38 = 0xff;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 2;
        DAT_8018fb3c.objects[195].arg.b.unk38 = 0xff;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 2;
        DAT_8018fb3c.objects[172].arg.b.unk38 = 0xff;
    } else {
        DAT_80185da8.objects[0].unk0 = 8;
        DAT_80185da8.objects[1].unk0 = 8;
        DAT_80185da8.objects[2].unk0 = 8;
        door = &DAT_8018fb3c.objects[214];
        door->unk0 = 0;
        DAT_8018fb3c.objects[214].arg.b.unk3c = 0;
        DAT_8018fb3c.objects[177].arg.b.unk3d[1] = 0;
        door = &DAT_8018fb3c.objects[111];
        door->unk0 = 2;
        DAT_8018fb3c.objects[111].arg.b.unk3c = 0xff;
        DAT_8018fb3c.objects[182].arg.b.unk3d[1] = 0xff;
        door = &DAT_8018fb3c.objects[62];
        door->unk0 = 0;
        DAT_8018fb3c.objects[62].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[195];
        door->unk0 = 0;
        DAT_8018fb3c.objects[195].arg.b.unk38 = 0;
        door = &DAT_8018fb3c.objects[172];
        door->unk0 = 0;
        DAT_8018fb3c.objects[172].arg.b.unk38 = 0;
    }
    if (func_8005d7bc(2) > 0 && func_8005d7bc(0x82) > 0 && func_8005d7bc(0x83) > 0 &&
        func_8005d7bc(0x84) > 0) {
        DAT_801ba984.unk4[3] = 1;
    } else {
        DAT_801ba984.unk4[3] = 0;
    }
    if (DAT_801ba984.unk4[8] != 0) {
        k = 344;
        if (DAT_8018fb3c.objects[k].arg.b.unk38 == 0xf1) {
            DAT_8018fb3c.objects[k].arg.b.unk38 = 0xff;
        }
    }
    k = 158;
    if (DAT_8018fb3c.objects[k].unk5 == 0xff) {
        DAT_8018fb3c.objects[k].unk5 = 0x3f;
        DAT_8018fb3c.objects[k].unk2 = 7;
        DAT_8018fb3c.objects[k].unk10 = 0x800;
    }
    DAT_801ba984.unk84[5] = 0;
}
