/*
 * GAME.EXE: map layer drawing front ends (WIP slice of the render module):
 * the per-layer GTE set-up before the two ground renderers, and a cell
 * front end nothing in GAME.EXE or the FDAT modules calls.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <INLINE.H>

/*
 * Draws one map layer at pos (relative to SCRATCH->unk108): loads its light
 * record (layer->unk4) turned by layer->unk2, the fog and the view transform,
 * then runs the near (flags bit 2) or far ground renderer on model part
 * layer->unk0.
 * Not exact: retail converts the part number to u16 (andi 0xffff) in each
 * branch; GCC drops the conversion of the byte it loaded once.
 */
ADDRESS(0x8003bb04, 0x330)
void func_8003bb04(MapLayer *layer, SVECTOR *pos, u8 flags)
{
    Record6c *light;
    s32 turn;
    long flag;
    u16 part;
    MATRIX *transform;
    ScratchPad *scratch;

    transform = &SCRATCH->draw.transform;
    scratch = SCRATCH;
    turn = layer->unk2 & 3;
    light = &DAT_80199154.unk15da8[layer->unk4 & 0x3f];
    gte_SetLightMatrix(&light->unk0[turn]);
    gte_SetColorMatrix(&light->unk50);
    func_80035358(light->unk68, light->unk6a);
    gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
    gte_SetRotMatrix(&DAT_80199154.unk159f8);
    gte_SetTransMatrix(&DAT_80199154.unk159f8);
    gte_ldv0(pos);
    gte_rt();
    gte_stlvnl(SCRATCH->draw.transform.t);
    gte_stflg(&flag);
    gte_SetRotMatrix(&DAT_80199154.unk15a38[turn]);
    gte_SetTransMatrix(transform);
    part = layer->unk0;
    if ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.unk16 != 0 &&
        part >= ((ModelData *)SCRATCH->model)->unk4 >> 1) {
        return;
    }
    if (scratch->end - 10240 < scratch->next) {
        flags &= ~4;
    }
    if (flags & 4) {
        scratch->part = &((ModelData *)scratch->model)->parts[part];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        func_8003ab04();
    } else {
        scratch->part = &((ModelData *)scratch->model)->parts[part];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        func_80039d50();
    }
}

/*
 * Draws the layers of map cell (x, z) that are visible, with flags bit 1 set.
 * Not exact: control flow, calls and constants agree; retail copies flags to
 * $a3 and x/z to $t0/$t1 at entry (unattributed allocation residue).
 */
ADDRESS(0x8003be34, 0x19c)
void func_8003be34(s32 x, s32 z, s32 flags)
{
    SVECTOR *pos;
    VECTOR *origin;
    MapCell *cell;
    s32 kind;

    pos = &SCRATCH->unk100;
    origin = &SCRATCH->unk108;
    cell = &DAT_801d4464.cells[z][x];
    kind = cell->layer[0].unk0;
    if ((flags & 2) && kind < 240) {
        pos->vx = (x << 11) - origin->vx + 1024;
        pos->vz = (z << 11) - origin->vz + 1024;
        pos->vy = (-cell->layer[0].unk1 << 7) - origin->vy;
        func_8003bb04(&cell->layer[0], &SCRATCH->unk100, flags);
        kind = cell->layer[1].unk0;
        if (kind < 240) {
            pos->vy = (-cell->layer[1].unk1 << 7) - origin->vy;
            func_8003bb04(&cell->layer[1], &SCRATCH->unk100, flags);
        }
    } else {
        kind = cell->layer[1].unk0;
        if ((flags & 2) && kind < 240) {
            pos->vx = (x << 11) - origin->vx + 1024;
            pos->vz = (z << 11) - origin->vz + 1024;
            pos->vy = (-cell->layer[1].unk1 << 7) - origin->vy;
            func_8003bb04(&cell->layer[1], pos, flags);
        }
    }
}
