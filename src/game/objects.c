/*
 * GAME.EXE: object drawing helpers (WIP module): model ids, resident and
 * streamed model files, the map cell grid around the viewer, and the sound
 * banks of the objects in view.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <game/sound.h>

ADDRESS(0x80040568, 0x80)
u16 func_80040568(u16 id)
{
    if (id < 300) {
        return id + 256;
    }
    if ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.unk17 == 1 && DAT_8018fad4.previous[0] != 0xff) {
        return id + (DAT_8018fad4.previous[0] * 32 + 256);
    }
    return id + (DAT_8018fad4.area[0] * 32 + 256);
}

ADDRESS(0x800405e8, 0x70)
void func_800405e8(u16 id, u8 **out)
{
    u8 *model;
    u8 state;

    model = DAT_80199154.models[id];
    *out = model;
    if (id < 104) {
        return;
    }
    if (model != NULL) {
        state = func_8001947c(model);
        if (state == 1 || state == 2) {
            return;
        }
    }
    *out = NULL;
}

/* Load-complete callback for a streamed model file. */
ADDRESS(0x80040658, 0x3c)
void func_80040658(u8 *file)
{
    func_8003591c(file + *(s32 *)(file + 8));
    func_80019474(file, 2);
}

ADDRESS(0x80040694, 0x74)
u8 func_80040694(VECTOR *pos)
{
    s32 row;
    s32 col;

    row = (pos->vz >> 11) + DAT_80199154.grid_z;
    if (row >= 0 && row < 25) {
        col = (pos->vx >> 11) + DAT_80199154.grid_x;
        if (col >= 0 && col < 25) {
            return DAT_80199154.grid[row][col];
        }
        return 0;
    }
    return 0;
}

ADDRESS(0x80040708, 0xc4)
u8 func_80040708(VECTOR *pos, s32 radius)
{
    u8 cells;
    s32 row;
    s32 col0;
    s32 col;
    s32 i;
    s32 j;
    u8 *line;
    u8 *cell;

    cells = 0;
    i = radius * 2;
    row = (pos->vz >> 11) + DAT_80199154.grid_z - radius;
    col0 = (pos->vx >> 11) + DAT_80199154.grid_x - radius;
    line = DAT_80199154.grid[row];
    do {
        if (row >= 0 && (u32)row < 25) {
                j = radius * 2;
                col = col0;
                cell = &line[col];
                do {
                    if (col >= 0 && (u32)col < 25) {
                        cells |= *cell;
                    }
                    cell++;
                    col++;
                } while (--j != -1);
        }
        line += 25;
        row++;
    } while (--i != -1);
    return cells;
}

ADDRESS(0x800407cc, 0x64)
s32 func_800407cc(VECTOR *pos, s32 range_x, s32 range_z)
{
    s32 inside;
    s32 x;
    s32 z;

    inside = 0;
    z = pos->vz >> 11;
    if (z - range_z <= DAT_80199154.unk15b14 && z + range_z >= DAT_80199154.unk15b14) {
        x = pos->vx >> 11;
        if (x - range_x <= DAT_80199154.unk15b10) {
            inside = x + range_x >= DAT_80199154.unk15b10;
        }
    }
    return inside;
}

ADDRESS(0x80040830, 0xa0)
void func_80040830(s32 file, s32 entry, s32 id)
{
    u8 *model;

    model = func_8001939c((HeapBlock *)DAT_800c8954, func_80019f14(file, entry), &DAT_80199154.models[id]);
    if (model != NULL) {
        func_80019474(model, 3);
        func_8001949c(model, id);
        func_80019d48(file, entry, model, func_80040658);
    }
}

ADDRESS(0x800408d0, 0xf8)
void func_800408d0(s32 file, s32 entry, s32 bank, s32 count)
{
    u8 *wanted;
    s32 end;
    SoundSlot *slot;

    wanted = SCRATCH->u.load.sound_wanted;
    for (end = count + entry; entry < end; entry++, bank++) {
        slot = DAT_801b09fc.banks[bank].slot;
        if (*wanted++ != 0) {
            if (slot == NULL) {
                func_80015ce0(file, entry, bank);
                continue;
            }
            if (slot->state == 2) {
                slot->state = 1;
            }
        } else if (slot != NULL && slot->state == 1) {
            slot->state = 2;
        }
    }
}

ADDRESS(0x800409c8, 0x11c)
void func_800409c8(s32 file, s32 entry, s32 id, s32 count)
{
    u8 *wanted;
    s32 end;
    u8 *model;

    wanted = SCRATCH->u.load.model_wanted;
    for (end = count + entry; entry < end; entry++, id++) {
        if (*wanted++ != 0) {
            model = DAT_80199154.models[id];
            if (model == NULL) {
                func_80040830(file, entry, id);
                continue;
            }
            if (func_8001947c(model) == 1) {
                func_80019474(model, 2);
            }
        } else {
            model = DAT_80199154.models[id];
            if (model != NULL && func_8001947c(model) != 3) {
                func_80019474(model, 1);
            }
        }
    }
}
