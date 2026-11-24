/*
 * GAME.EXE: Object44 records (WIP module): the 396 records after the
 * Object44Def table in DAT_8018fb3c, their set-up, per-kind state and the map
 * cell bytes some kinds keep in step with their state.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <stdarg.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/sound.h>
#include <game/object44.h>

/* Plays sound effect se at the record. */
ADDRESS(0x8004482c, 0x30)
void func_8004482c(Object44 *obj, s32 se)
{
    func_80015654(se, &obj->pos, 0x78, 0);
}

/* Frees every record. */
ADDRESS(0x8004485c, 0x5c)
void func_8004485c(void)
{
    Object44 *obj;
    u16 i;

    obj = DAT_8018fb3c.objects;
    i = 396;
    while (i--) {
        obj->unk4 = obj->unk6 = 0xff;
        obj->unk34 = obj->arg.w.unk38 = obj->arg.w.unk3c = 0;
        obj++;
    }
    DAT_8018fb3c.unk885e = DAT_8018fb3c.unk8860 = DAT_8018fb3c.unk8862 = 0;
}

/* Resets a record's state, angle and scale. */
ADDRESS(0x800448b8, 0x48)
void func_800448b8(Object44 *obj)
{
    obj->unk1 = 0x80;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
    obj->unka = 0;
    obj->unk2 = 0;
    obj->unk0 = 0;
    obj->angle.vy = obj->angle.vx = obj->angle.vz = 0;
    obj->unk4 = 0xff;
    obj->unke = 0;
    obj->unk5 = 0xff;
    obj->unk10 = 0;
}

/*
 * Sets the state of record id by the type of its Object44Def: mode 0 stops
 * it, 1 sets unk0 to the byte argument, 2 marks it, 3 sets unke to the
 * halfword argument.
 */
ADDRESS(0x80044900, 0x240)
void func_80044900(s32 id, s32 mode, ...)
{
    va_list args;
    Object44 *obj;
    Object44Def *def;

    va_start(args, mode);
    if (id == 0xffff) {
        return;
    }
    obj = &DAT_8018fb3c.objects[id];
    def = &DAT_8018fb3c.defs[obj->unk6];
    switch (mode) {
    case 0:
        switch (def->unk0) {
        case 0xd:
        case 0x53:
            obj->unk0 = 0;
            /* fall through */
        case 0x14:
            obj->arg.b.unk3e = 0;
            break;
        case 9:
        case 0x15:
            obj->unk0 = 0;
            obj->arg.b.unk3c = 0;
            break;
        case 0x40:
            obj->unk0 = 0;
            obj->arg.b.unk38 = 0;
            if (obj->unk6 != 0x95) {
                if (obj->arg.b.unk3a != 0xff) {
                    obj->angle.vx = obj->arg.b.unk3a << 6;
                }
                if (obj->arg.b.unk3b != 0xff) {
                    obj->angle.vy = obj->arg.b.unk3b << 6;
                }
                if (obj->arg.b.unk3c != 0xff) {
                    obj->angle.vz = obj->arg.b.unk3c << 6;
                }
                if (obj->arg.b.unk39 == 0) {
                    obj->unk4 = 0x10;
                    obj->arg.w.unk40 = obj->pos.vy;
                }
            }
            break;
        default:
            obj->unk0 = 0;
            obj->arg.b.unk38 = 0;
            break;
        }
        break;
    case 1:
        switch (def->unk0) {
        case 0x14:
        case 0x15:
            break;
        default:
            obj->unk0 = va_arg(args, u8);
            break;
        }
        break;
    case 2:
        switch (def->unk0) {
        case 0xd:
        case 0x14:
        case 0x53:
            obj->arg.b.unk3e = 0xff;
            break;
        case 9:
        case 0x15:
            obj->arg.b.unk3c = 0xff;
            break;
        default:
            obj->arg.b.unk38 = 0xff;
            break;
        }
        break;
    case 3:
        obj->unke = va_arg(args, u16);
        break;
    }
    va_end(args);
}

/*
 * Writes the record's map cell byte for its area layer: value with no scale
 * when mode is 0 and Data801b24e4.unk90 is clear, else 0xfc with full scale.
 */
ADDRESS(0x80044b40, 0xf4)
void func_80044b40(Object44 *obj, s32 mode, u8 value)
{
    MapCell *cell;
    MapLayer *layer;

    if (mode == 0 && DAT_801b24e4.unk90 == 0) {
        cell = &DAT_801d4464.cells[obj->pos.vz >> 11][obj->pos.vx >> 11];
        layer = cell->layer;
        if (obj->unk0 != 1) {
            layer++;
        }
        layer->unk0 = value;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
    } else {
        cell = &DAT_801d4464.cells[obj->pos.vz >> 11][obj->pos.vx >> 11];
        layer = cell->layer;
        if (obj->unk0 != 1) {
            layer++;
        }
        layer->unk0 = 0xfc;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
    }
}

/* Brings the map cell bytes of kinds 7, 0x51 and 0x52 in line with mode. */
ADDRESS(0x80044c34, 0x168)
void func_80044c34(s32 mode)
{
    Object44 *obj;
    Object44Def *def;
    s32 i;

    obj = DAT_8018fb3c.objects;
    for (i = 396; i != 0; i--) {
        switch (obj->unk4) {
        case 0x51:
            if (obj->arg.b.unk3b != 0xff) {
                def = &DAT_8018fb3c.defs[obj->unk6];
                func_80044b40(obj, mode, def->unk17);
            }
            break;
        case 0x52:
            if (obj->arg.b.unk3d != 0xff) {
                def = &DAT_8018fb3c.defs[obj->unk6];
                func_80044b40(obj, mode, def->unk17);
            }
            break;
        case 7:
            if (obj->arg.b.unk38 != 0xfe) {
                func_80044900(obj->arg.h.unk3a, mode, obj->unk0);
                def = &DAT_8018fb3c.defs[obj->unk6];
                if (def->unkc != 0xff) {
                    func_80044b40(obj, mode, def->unkc);
                }
            }
            break;
        }
        obj++;
    }
}
