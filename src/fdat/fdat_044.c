/*
 * FDAT_044.BIN (WIP module): the handler table, its slot 0 handler, empty
 * handlers, a trap handler (slot 3) and the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x88)

void func_801e8390(void);
void func_801e8398(VECTOR *pos, SVECTOR *angle);
void func_801e85c4(void);
void func_801e85cc(void);
void func_801e85d4(void);
void func_801e85dc(void);
void func_801e85e4(Object44 *obj);
void func_801e8680(void);
void func_801e8688(void);
void func_801e8690(void);
void func_801e8698(void);
void func_801e86a0(void);
void func_801e86a8(void);
void func_801e86b0(void);

const FdatHeader fdat_header = {
    0x13,
    {
        func_801e8398, func_801e85c4, func_801e85cc, func_801e85e4,
        func_801e86a8, func_801e86b0, func_801e86a0, func_801e8390,
        func_801e8680, func_801e8688, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8690, func_801e8698, func_801e85d4, func_801e85dc,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x33,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

/* Slot 0: fdat_801e83b0's shape with this module's message table. */
ADDRESS(0x801e8398, 0x22c)
void func_801e8398(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    for (i = 0; (i = func_80045c7c(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        if ((def->unk0 == 9 || def->unk0 == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        func_80041eec(obj->arg.b.unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7) {
                        func_80041eec(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        func_80041eec(0x1e);
                    } else if (obj->unk6 == 0xf5) {
                        func_80041eec(0xe);
                    } else if (obj->unk6 == 0xba) {
                        func_80041eec(0x22);
                    } else if (obj->unk6 == 0x12e || obj->unk6 == 0x12f || obj->unk6 == 0x130) {
                        func_80041eec(0x21);
                    } else if (obj->unk6 >= 190 && obj->unk6 < 198) {
                        func_80041eec(8);
                    }
                }
            } else {
                link = &DAT_8018fb3c.objects[obj->arg.b.unk3a.index];
                if (link->unk6 == 0xff) {
                    goto message;
                }
                link->unk0 = obj->arg.b.unk40;
                link->arg.b.unk38 = 0xff;
                func_8005db30(link);
                link->unk0 = 0;
                link->arg.b.unk38 = 0;
                if (link->unk6 == 0xff) {
                    obj->arg.b.unk3a.index = 0xffff;
                }
            }
        }
    }
}

ADDRESS(0x801e85c4, 0x8)
void func_801e85c4(void)
{
}

ADDRESS(0x801e85cc, 0x8)
void func_801e85cc(void)
{
}

ADDRESS(0x801e85d4, 0x8)
void func_801e85d4(void)
{
}

ADDRESS(0x801e85dc, 0x8)
void func_801e85dc(void)
{
}

/* Slot 3: objects 0x62 (when func_80046884 hits) and 0x63 hurt the player. */
ADDRESS(0x801e85e4, 0x9c)
void func_801e85e4(Object44 *obj)
{
    switch (obj->arg.b.unk3a.bytes[1]) {
    case 0x62:
        if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38, obj->arg.b.unk39,
                          obj->pos.vy + 3200) == 0) {
            break;
        }
        /* fall through */
    case 0x63:
        func_8002ab18(0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0);
        break;
    }
}

ADDRESS(0x801e8680, 0x8)
void func_801e8680(void)
{
}

ADDRESS(0x801e8688, 0x8)
void func_801e8688(void)
{
}

ADDRESS(0x801e8690, 0x8)
void func_801e8690(void)
{
}

ADDRESS(0x801e8698, 0x8)
void func_801e8698(void)
{
}

ADDRESS(0x801e86a0, 0x8)
void func_801e86a0(void)
{
}

ADDRESS(0x801e86a8, 0x8)
void func_801e86a8(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e86b0, 0x14c)
void func_801e86b0(void)
{
    s32 state;

    if (DAT_801ba984.unke[0xa] >= 2) {
        DAT_8018fb3c.objects[7].unk6 = 0xff;
        DAT_8018fb3c.objects[49].unk6 = 0xff;
        func_800445b8(2, 0x4e, 0, 0x21, 2, 2, 3, 0, 0x2f);
        func_800445b8(2, 0x4e, 3, 12, 0x48, 2, 3, 0, 0x2f);
        state = 8;
    } else {
        func_800445b8(2, 0x4c, 0, 0x21, 2, 2, 3, 0, 0x2f);
        func_800445b8(2, 0x4c, 3, 12, 0x48, 2, 3, 0, 0x2f);
        state = 1;
    }
    DAT_80185da8.objects[66].unk0 = state;
    DAT_80185da8.objects[70].unk0 = state;
    DAT_80185da8.objects[24].unk0 = state;
    DAT_80185da8.objects[43].unk0 = state;
    DAT_801ba984.unk84[5] = 0xe;
}
