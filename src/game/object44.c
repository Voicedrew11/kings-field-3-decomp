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
#include <game/cd.h>
#include <game/memory.h>
#include <game/collision.h>
#include <game/render.h>
#include <game/map_patch.h>
#include <game/object44.h>

RODATA(0x80011c54, 0x400)

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
        obj->arg.w.unk34 = obj->arg.w.unk38 = obj->arg.w.unk3c = 0;
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
            obj->arg.b.unk3d[1] = 0;
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
                if (obj->arg.b.unk3a.bytes[0] != 0xff) {
                    obj->angle.vx = obj->arg.b.unk3a.bytes[0] << 6;
                }
                if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                    obj->angle.vy = obj->arg.b.unk3a.bytes[1] << 6;
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
            obj->arg.b.unk3d[1] = 0xff;
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
        {
            MapCell *row = DAT_801d4464.cells[obj->pos.vz >> 11];

            cell = &row[obj->pos.vx >> 11];
        }
        layer = cell->layer;
        if (obj->unk0 != 1) {
            layer++;
        }
        layer->unk0 = value;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
    } else {
        {
            MapCell *row = DAT_801d4464.cells[obj->pos.vz >> 11];

            cell = &row[obj->pos.vx >> 11];
        }
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
            if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                def = &DAT_8018fb3c.defs[obj->unk6];
                func_80044b40(obj, mode, def->arg.b.unk17);
            }
            break;
        case 0x52:
            if (obj->arg.b.unk3d[0] != 0xff) {
                def = &DAT_8018fb3c.defs[obj->unk6];
                func_80044b40(obj, mode, def->arg.b.unk17);
            }
            break;
        case 7:
            if (obj->arg.b.unk38 != 0xfe) {
                func_80044900(obj->arg.h.unk3a, mode, obj->unk0);
                def = &DAT_8018fb3c.defs[obj->unk6];
                if (def->arg.b.unkc != 0xff) {
                    func_80044b40(obj, mode, def->arg.b.unkc);
                }
            }
            break;
        }
        obj++;
    }
}

/*
 * Sets up the area's 350 Object44 records from its placement table: def,
 * position on the map cell, radius on the cell grid, then per Object44Def
 * type the kind and its state (map patches, timers, cell bytes); types
 * without a case go to FDAT handler 8.
 * Not exact: retail adds the cell row (base + z * 800) before the column
 * (+ x * 10) where GCC here forms z * 800 + (x * 10 + base) (as in
 * func_80044b40), and loads def->arg.h.unke before def->arg.h.unk10 for the larger radius.
 */
ADDRESS(0x80044d9c, 0xd2c)
void func_80044d9c(Object44Place *place)
{
    Object44 *obj;
    Object44Def *def;
    u32 radius;
    s32 i;
    u32 frames;

    frames = DAT_801c12e4.frames;
    obj = DAT_8018fb3c.objects;
    for (i = 349; i != -1; place++, obj++, i--) {
        if (place->unk4 == 0xffff) {
            obj->unk6 = 0xff;
        } else {
            obj->unk6 = place->unk4;
        }
        obj->unk4 = 0xff;
        if (obj->unk6 == 0xff) {
            continue;
        }
        def = &DAT_8018fb3c.defs[obj->unk6];
        obj->unk8 = 0;
        obj->unk1 = 0x80;
        obj->unka = 0;
        obj->unk2 = 0;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = -place->unk6 & 0xfff;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->unk0 = place->unk0;
        obj->unk3 = def->unk3;
        obj->unke = def->unka;
        obj->unk5 = 0xff;
        obj->unk10 = 0;
        if (obj->unk3 & 0x20) {
            obj->unk2 |= 0x80;
        }
        if (obj->unk3 & 0x10) {
            obj->unkc = (def->unk8 * place->unk10[0]) >> 7;
            obj->scale.vx = obj->scale.vy = obj->scale.vz = place->unk10[0] << 5;
        } else {
            obj->unkc = def->unk8;
        }
        obj->pos.vx = (place->unk2 << 11) + place->unka;
        obj->pos.vz = (place->unk1 << 11) + place->unk8;
        {
            MapCell *row = DAT_801d4464.cells[place->unk1];
            MapLayer *layer = row[place->unk2].layer;

            if (obj->unk0 != 1) {
                layer++;
            }
            obj->pos.vy = place->unkc - (layer->unk1 << 7);
        }
        func_80018f5c((u32 *)&obj->arg.b.unk38, (u32 *)place->unk10, 2);
        func_80018fbc((u32 *)&obj->arg.w.unk40, 0xff, 1);
        radius = def->unk4;
        if (radius != 0) {
            if (obj->unk3 & 0x10) {
                func_80033c4c(obj->pos.vx, obj->pos.vz, radius * obj->arg.b.unk38 >> 7, 1);
            } else {
                func_80033c4c(obj->pos.vx, obj->pos.vz, radius, 1);
            }
        } else if (obj->unk3 & 4) {
            radius = def->arg.h.unk10;
            if (def->arg.h.unke > radius) {
                radius = def->arg.h.unke;
            }
            func_80033c4c(obj->pos.vx, obj->pos.vz, radius, 1);
        }
        switch (def->unk0) {
        case 1:
            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + 1, obj->arg.b.unk3c,
                          obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd, def->arg.b.unke,
                          obj->angle.vy, 45);
            obj->unk4 = 1;
            obj->unk8 = 2;
            obj->arg.h.unk40 = 999;
            obj->arg.h.unk42 = obj->angle.vy;
            func_80033c4c(obj->pos.vx, obj->pos.vz, 1500, 1);
            break;
        case 27:
            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + 3, obj->arg.b.unk3c,
                          obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2, obj->angle.vy, 45);
            obj->unk4 = 0x1b;
            obj->unk8 = 2;
            obj->arg.h.unk40 = 999;
            obj->arg.h.unk42 = obj->angle.vy;
            func_80033c4c(obj->pos.vx, obj->pos.vz, 3000, 1);
            break;
        case 0:
            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + 3, obj->arg.b.unk3c,
                          obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2, obj->angle.vy, 45);
            obj->unk4 = 0;
            obj->unk8 = 2;
            obj->arg.h.unk40 = 999;
            obj->arg.h.unk42 = obj->angle.vy;
            func_80033c4c(obj->pos.vx, obj->pos.vz, 3000, 1);
            break;
        case 3:
            obj->unk4 = 3;
            obj->unk1 = 0;
            func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828dc, 0,
                          0xff);
            func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828fc, 0,
                          0xff);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 4:
            obj->unk4 = 4;
            obj->unk1 = 0;
            func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_800828dc, 0,
                          0xff);
            func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008291c, 0,
                          0xff);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 5:
            obj->unk4 = 5;
            obj->unk1 = 0;
            func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008293c, 0,
                          0xff);
            func_800443c8(obj->unk0, obj->pos.vx, obj->pos.vz, obj->angle.vy, DAT_8008295c, 0,
                          0xff);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 2:
            obj->unk4 = 2;
            obj->unk1 = 0;
            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd * 2, obj->arg.b.unk3c,
                          obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd, def->arg.b.unke,
                          obj->angle.vy, 45);
            func_80033c4c(obj->pos.vx, obj->pos.vz, 4400, 1);
            break;
        case 23:
            obj->unk4 = 0x17;
            break;
        case 6:
            obj->unk4 = 6;
            break;
        case 7:
            obj->unk4 = 7;
            obj->unk1 = 0;
            if (def->arg.b.unkc != 0xff) {
                func_80044b40(obj, 0, def->arg.b.unkc);
            }
            break;
        case 8:
            obj->unk4 = 8;
            break;
        case 9:
            obj->unk4 = 9;
            obj->arg.b.unk40 = obj->unk0;
            break;
        case 21:
            obj->unk4 = 9;
            obj->arg.b.unk40 = obj->unk0;
            obj->unk0 = 0;
            break;
        case 83:
            obj->unk4 = 0x53;
            obj->unk1 = 0;
            obj->unk8 = 9;
            break;
        case 84:
            obj->unk1 = 0;
            obj->arg.b.unk40 = obj->unk0;
            obj->unk0 = 2;
            obj->unk4 = 0x54;
            func_800443c8(obj->arg.b.unk40, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                          DAT_8008288c[def->arg.b.unke * 2 + (obj->arg.b.unk3d[1] & 1)], 0, 0);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 95:
            obj->unk0 = 0;
            obj->unk4 = 0x5f;
            obj->unk8 = 1;
            break;
        case 81:
            obj->unk1 = 0;
            obj->unk4 = 0x51;
            obj->arg.b.unk40 = 0;
            if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                func_80044b40(obj, 0, def->arg.b.unk17);
            }
            break;
        case 87:
            obj->unk1 = 0;
            obj->unk0 = 2;
            obj->unk4 = 0x57;
            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0],
                          obj->arg.b.unk3c, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                          obj->arg.b.unk3d[0], obj->arg.b.unk3d[1], obj->angle.vy, 45);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 90:
            obj->unk1 = 0;
            obj->unk0 = 2;
            obj->unk4 = 0x5a;
            func_800445b8(obj->unk0, obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0],
                          obj->arg.b.unk3c, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                          obj->arg.b.unk3d[0], obj->arg.b.unk3d[1], obj->angle.vy, 45);
            break;
        case 82:
            obj->unk1 = 0;
            obj->unk4 = 0x52;
            obj->arg.b.unk40 = 0;
            if (obj->arg.b.unk3d[0] != 0xff) {
                func_80044b40(obj, 0, def->arg.b.unk17);
            }
            break;
        case 85:
            obj->unk1 = 0;
            obj->unk4 = 0x55;
            obj->arg.b.unk40 = 0;
            if (obj->arg.b.unk3a.bytes[1] == obj->arg.b.unk3d[0]) {
                if (obj->arg.b.unk3c < obj->arg.b.unk3d[1]) {
                    obj->arg.b.unk41[1] = 0;
                } else {
                    u8 t;

                    obj->arg.b.unk41[1] = 1;
                    t = obj->arg.b.unk3c;
                    obj->arg.b.unk3c = obj->arg.b.unk3d[1];
                    obj->arg.b.unk3d[1] = t;
                }
            } else if (obj->arg.b.unk3c == obj->arg.b.unk3d[1]) {
                if (obj->arg.b.unk3a.bytes[1] < obj->arg.b.unk3d[0]) {
                    obj->arg.b.unk41[1] = 2;
                } else {
                    u8 t;

                    obj->arg.b.unk41[1] = 3;
                    t = obj->arg.b.unk3a.bytes[1];
                    obj->arg.b.unk3a.bytes[1] = obj->arg.b.unk3d[0];
                    obj->arg.b.unk3d[0] = t;
                }
            }
            obj->arg.b.unk41[2] = 1;
            break;
        case 86:
            obj->unk4 = 0x56;
            obj->unk8 = 0;
            break;
        case 231:
            obj->unk0 = 0;
            obj->unk4 = 0xe7;
            obj->arg.b.unk40 = 0;
            break;
        case 235:
            obj->unk4 = 0xeb;
            obj->arg.b.unk40 = 1;
            break;
        case 234:
            obj->unk0 = 0;
            obj->unk4 = 0xea;
            obj->arg.b.unk40 = 0;
            break;
        case 232:
            obj->unk1 = 0x80;
            obj->unk4 = 0xe8;
            obj->unka = 0;
            obj->arg.b.unk40 = 0xff;
            break;
        case 224:
            obj->unk0 = 0;
            obj->unk4 = 0xe0;
            obj->pos.vx -= place->unka;
            obj->pos.vz -= place->unk8;
            obj->pos.vy -= place->unkc;
            if (place->unka == 0xff) {
                obj->arg.b.unk40 = obj->arg.b.unk41[0] = obj->arg.b.unk41[1] = 0x7f;
            } else {
                obj->arg.b.unk40 = (u8)place->unka - place->unk2;
                obj->arg.b.unk41[0] = (u8)place->unk8 - place->unk1;
                obj->arg.b.unk41[1] = (u8)place->unkc - (-obj->pos.vy >> 7);
            }
            break;
        case 225:
            obj->unk0 = 0;
            obj->unk4 = 0xe1;
            obj->arg.b.unk40 = 0;
            break;
        case 227:
            obj->unk0 = 0;
            obj->unk4 = 0xe3;
            if (func_80046884(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                              obj->arg.b.unk39, 0x8000) != 0) {
                DAT_80199154.unk15995 = obj->arg.b.unk3a.bytes[0];
                DAT_80199154.unk15996 = obj->arg.b.unk3a.bytes[1];
            }
            break;
        case 228:
            obj->unk0 = 0;
            obj->unk4 = 0xe4;
            break;
        case 229:
            obj->unk4 = 0xe5;
            break;
        case 230:
            obj->unk0 = 0;
            obj->unk4 = 0xe6;
            obj->arg.b.unk40 = 0;
            break;
        case 31:
            obj->unk0 = 0;
            obj->unk4 = 0x1f;
            obj->arg.w.unk40 = frames + (u16)obj->arg.h.unk3e * 6;
            break;
        case 240:
            obj->unk4 = 0xf0;
            break;
        case 242:
            obj->unk4 = 0xf2;
            break;
        case 233:
            obj->unk4 = 0xe9;
            break;
        case 13:
            if (obj->arg.b.unk3a.bytes[0] != 0xff) {
                obj->angle.vx = obj->arg.b.unk3a.bytes[0] << 6;
            }
            if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                obj->angle.vy = obj->arg.b.unk3a.bytes[1] << 6;
            }
            if (obj->arg.b.unk3c != 0xff) {
                obj->angle.vz = obj->arg.b.unk3c << 6;
            }
            break;
        case 226:
            obj->arg.b.unk40 = obj->unk0;
        case 11:
        case 20:
        clear_area:
            obj->unk0 = 0;
            break;
        case 88:
            obj->unk1 = 0;
            obj->unk4 = 0x58;
            obj->unk8 = 1;
            break;
        case 89:
            obj->unk4 = 0x59;
            obj->unk5 = 0x32;
            obj->unk10 = 0x1000;
            obj->unk2 = (obj->unk2 & 0xf8) | 5;
            obj->pos.vy += 256;
            {
                MapCell *row = DAT_801d4464.cells[obj->pos.vz >> 11];
                MapLayer *layer = row[obj->pos.vx >> 11].layer;

                if (obj->unk0 != 1) {
                    layer++;
                }
                layer->unk3 = 0x75;
            }
            obj->arg.b.unk41[1] = obj->unk0;
            goto clear_area;
        case 15:
            obj->unk4 = 0xf;
            break;
        case 17:
            obj->unk4 = 0x11;
            break;
        case 24:
            obj->unk4 = 0x18;
            break;
        case 26:
            obj->unk4 = 0x1a;
            break;
        case 25:
            obj->unk4 = 0x19;
            break;
        case 49:
            obj->unk4 = 0x31;
            break;
        case 50:
            obj->unk4 = 0x32;
            obj->unk1 = 0;
            obj->unk8 = 0;
            break;
        case 64:
            if (def->unk1 != 0x20) {
                if (obj->arg.b.unk3a.bytes[0] != 0xff) {
                    obj->angle.vx = obj->arg.b.unk3a.bytes[0] << 6;
                }
                if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                    obj->angle.vy = obj->arg.b.unk3a.bytes[1] << 6;
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
        case 19:
            obj->unk4 = 0x13;
            break;
        case 48:
            obj->unk4 = 0x30;
            obj->unk2 = (obj->unk2 & 0xf8) | 5;
            obj->pos.vy -= obj->arg.b.unk3c << 8;
            break;
        case 22:
            obj->unk4 = 0x16;
            obj->arg.w.unk40 = frames + 30;
            break;
        case 18:
            obj->unk4 = 0x12;
            obj->arg.w.unk40 = frames + 30;
            break;
        default:
            DAT_8018fad4.handlers[8](obj, def);
            break;
        case 33:
        case 255:
            break;
        }
    }
}
