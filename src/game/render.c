/*
 * GAME.EXE: band B's render module from 0x8003bfd0 (WIP name): the map
 * layers of the view grid, the 3D overlay and model drawing front ends,
 * full-screen 2D effects and the loading screen, object model files, the map
 * cell grid and the per-frame object drawer, the 2D overlay sprites and
 * messages, the animated model poses and the picture fade. One TU:
 * func_8003bfd0/func_8003c35c/func_8003df50/func_800400ac read DAT_8009c2e0
 * and func_800441d4 reads DAT_8009c2d0 $gp-relative, which ASPSX emits only
 * for .sdata defined in the same file.
 */
/* func_80040ae4 reads Data8018fad4.step as volatile (see game/data.h). */
#define GAME_RENDER_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <psyq/libc.h>
#include <game/data.h>
#include <game/render.h>
#include <INLINE.H>
#include <GTEMAC.H>
#include <kf/lib/null.h>
#include <game/sound.h>
#include <game/entity.h>
#include <game/object44.h>
#include <stdarg.h>
#include <MALLOC.H>
#include <RAND.H>
#include <game/cd.h>
#include <game/player.h>

RODATA(0x80011bd0, 0xc)

DATA(0x8009c2d0, 0x8, ".sdata")
RECT DAT_8009c2d0 = { 320, 0, 320, 240 };

DATA(0x8009c2d8, 0x8, ".sdata")
RECT DAT_8009c2d8 = { 960, 256, 64, 240 };

/* A delta run marker: the next word is a count instead of a vertex. */
#define DELTA_RUN (-32768)

/* Brightness the 3D overlays share with the status sprites (.sdata). */

DATA(0x8009c2e0, 0x4, ".sdata")
CVECTOR DAT_8009c2e0 = { 0x80, 0x80, 0x80, 0 };

DATA(0x8009c2e4, 0x8, ".sdata")
SVECTOR DAT_8009c2e4 = { 0x1000, 0x1000, 0x1000, 0 };

DATA(0x8009c2ec, 0x4, ".sdata")
s32 DAT_8009c2ec = 0;

/*
 * Draws the map layers of the 25 x 25 cell grid around the viewer
 * (Data80199154.grid, flags per cell as func_8003be34 takes them).
 */
ADDRESS(0x8003bfd0, 0x38c)
void func_8003bfd0(void)
{
    SVECTOR *pos;
    VECTOR *origin;
    MapCell *cell;
    u8 *grid;
    s32 kind;
    s32 x;
    s32 z;
    s32 i;
    s32 j;

    pos = &SCRATCH->u.layer.unk100;
    origin = &SCRATCH->u.layer.unk108;
    SCRATCH->model = DAT_80199154.unk10148[0];
    SCRATCH->ot = DAT_80199154.ot + 240;
    *(u32 *)&SCRATCH->w.map.rgb = *(u32 *)&DAT_8009c2e0;
    SCRATCH->next = DAT_80199154.pool->next;
    SCRATCH->end = DAT_80199154.pool->end;
    SCRATCH->w.map.clip = 100;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->u.layer.unk108.vx = DAT_80199154.unk15af8.vx;
    SCRATCH->u.layer.unk108.vy = DAT_80199154.unk15af8.vy;
    SCRATCH->u.layer.unk108.vz = DAT_80199154.unk15af8.vz;
    SCRATCH->u.layer.unk108.pad = DAT_80199154.unk15af8.pad;
    SCRATCH->w.map.count = DAT_80199154.unk159bc;
    SCRATCH->w.map.ot_count = DAT_80199154.ot_count;
    SCRATCH->w.map.prim_count = DAT_80199154.prim_count;
    func_80018f5c((u32 *)SCRATCH->u.view.grid, (u32 *)DAT_80199154.grid, 0x9c);
    grid = &SCRATCH->u.view.grid[0][0];
    SCRATCH->u.view.x = DAT_80199154.unk15b20;
    SCRATCH->u.view.z = DAT_80199154.unk15b24;
    z = DAT_80199154.unk15b24;
    for (i = 25; i != 0; z++, i--) {
        if (z >= 0 && z < 80) {
            x = SCRATCH->u.view.x;
            for (j = 25; j != 0; x++, j--, grid++) {
                if (x >= 0 && x < 80 && *grid != 0) {
                    cell = &DAT_801d4464.cells[z][x];
                    kind = cell->layer[0].unk0;
                    if ((*grid & 2) && kind < 240) {
                        pos->vx = (x << 11) - origin->vx + 1024;
                        pos->vy = (-cell->layer[0].unk1 << 7) - origin->vy;
                        pos->vz = (z << 11) - origin->vz + 1024;
                        func_8003bb04(&cell->layer[0], pos, *grid);
                        kind = cell->layer[1].unk0;
                        if ((*grid & 2) && kind < 240) {
                            pos->vy = (-cell->layer[1].unk1 << 7) - origin->vy;
                            func_8003bb04(&cell->layer[1], pos, *grid);
                        }
                    } else {
                        kind = cell->layer[1].unk0;
                        if ((*grid & 2) && kind < 240) {
                            pos->vx = (x << 11) - origin->vx + 1024;
                            pos->vy = (-cell->layer[1].unk1 << 7) - origin->vy;
                            pos->vz = (z << 11) - origin->vz + 1024;
                            func_8003bb04(&cell->layer[1], pos, *grid);
                        }
                    }
                }
            }
        } else {
            grid += 25;
        }
    }
    DAT_80199154.unk159bc = SCRATCH->w.map.count;
    DAT_80199154.ot_count = SCRATCH->w.map.ot_count;
    DAT_80199154.pool->next = SCRATCH->next;
}

/*
 * Draws the 3D overlays (the compass needles of DAT_80081c20, visible == 1)
 * in view space: each lit by its light record turned by its rotation, scaled,
 * posed by func_800431e8 (the base pose is rebuilt when it returns nonzero,
 * unlike func_800400ac), and projected flat (screen x/y = view x/y, depth =
 * view z / 4) before func_80035ca4.
 */

ADDRESS(0x8003c35c, 0x50c)
void func_8003c35c(void)
{
    MATRIX m;
    VECTOR scale;
    MATRIX lm;
    Data80081c20 *rec;
    ScratchPad *scratch;
    Record6c *light;
    u8 *file;
    u8 *model;
    s32 i;
    s32 offset;

    scratch = SCRATCH;
    for (rec = DAT_80081c20; rec->visible != 0xff; rec++) {
        if (rec->visible != 1) {
            continue;
        }
        light = &DAT_80199154.unk15da8[rec->light];
        m.t[0] = rec->pos.vx;
        m.t[1] = rec->pos.vy;
        m.t[2] = rec->pos.vz;
        RotMatrix(&rec->angle, &m);
        func_80035358(light->unk68, light->unk6a);
        gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
        gte_SetColorMatrix(&light->unk50);
        gte_MulMatrix0(&light->unk0[0], &m, &lm);
        gte_SetLightMatrix(&lm);
        scale.vx = rec->scale.vx;
        scale.vy = rec->scale.vy;
        scale.vz = rec->scale.vz;
        ScaleMatrix(&m, &scale);
        gte_SetRotMatrix(&m);
        gte_SetTransMatrix(&m);
        file = DAT_80199154.models[rec->model];
        model = file + ((ModelFile *)file)->vertices;
        scratch->ot = DAT_80199154.ot;
        *(u32 *)&scratch->w.model.rgb = *(u32 *)&DAT_8009c2e0;
        scratch->draw.count.unk4 = DAT_80199154.unk159bc;
        scratch->draw.count.ot_count = DAT_80199154.ot_count;
        scratch->draw.count.prim_count = DAT_80199154.prim_count;
        scratch->model = model;
        scratch->next = DAT_80199154.pool->next;
        scratch->end = DAT_80199154.pool->end;
        scratch->screen = DAT_80199154.screen;
        scratch->draw.count.clut = 0;
        if (func_800431e8(&rec->slot, rec->model, rec->anim, rec->time,
                          ((ModelData *)model)->parts[0].unk4) != 0) {
            u8 *vertex;
            ScreenVertex *screen;

            scratch->part = ((ModelData *)scratch->model)->parts;
            offset = scratch->part->vertices + 12;
            vertex = scratch->vertices = (u8 *)(offset + (u32)scratch->model);
            screen = scratch->screen;
            i = ((ModelData *)model)->parts[0].unk4;
            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.unk54);
                screen->sxy.vx = scratch->w.model.unk54.vx;
                screen->sxy.vy = scratch->w.model.unk54.vy;
                screen->fog = scratch->w.model.unk54.vz;
                screen->z = scratch->w.model.unk54.vz >> 2;
                screen++;
                vertex += 8;
            }
        } else {
            u8 *vertex;
            ScreenVertex *screen;

            i = ((ModelData *)model)->parts[0].unk4;
            screen = scratch->screen;
            vertex = scratch->vertices;
            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.unk54);
                screen->sxy.vx = scratch->w.model.unk54.vx;
                screen->sxy.vy = scratch->w.model.unk54.vy;
                screen->fog = scratch->w.model.unk54.vz;
                screen->z = scratch->w.model.unk54.vz >> 2;
                screen++;
                vertex += 8;
            }
        }
        func_80035ca4(0, 0);
        DAT_80199154.pool->next = scratch->next;
        DAT_80199154.unk159bc = scratch->draw.count.unk4;
        DAT_80199154.ot_count = scratch->draw.count.ot_count;
    }
}

ADDRESS(0x8003c868, 0x100)
void func_8003c868(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                   u16 tpage, u16 clut, u8 r, u8 g, u8 b)
{
    POLY_FT4 *p;

    p = (POLY_FT4 *)(DAT_80199154.buffer + DAT_80199154.frame * 160);
    setRGB0(p, r, g, b);
    p->tpage = tpage;
    p->clut = clut;
    setUVWH(p, u, v, w, h);
    p->x0 = p->x2 = x0;
    p->x1 = p->x3 = x1;
    p->y0 = p->y1 = y0;
    p->y2 = p->y3 = y1;
    setPolyFT4(p);
    if (semi != 0xff) {
        setSemiTrans(p, 1);
    }
    DrawPrim(p);
}

ADDRESS(0x8003c968, 0x74)
void func_8003c968(u8 level, u8 inset)
{
    func_8003c868(inset + 60, inset + 20, 260 - inset, 220 - inset, 0, 0, 200, 200, 0xff, 0x1f,
                  0x723c, level, level, level);
}

ADDRESS(0x8003c9dc, 0x1ec)
void func_8003c9dc(void)
{
    char name[64];
    CdlFILE file;
    u_char result[8];
    s32 i;
    s32 sync;

    strcpy(name, "\\LOAD.MSG;1");
    if (CdSearchFile(&file, name) == 0) {
        PutDispEnv(&DAT_80199154.disp[1]);
        PutDrawEnv(&DAT_80199154.draw[1]);
        SetDispMask(1);
        return;
    }
    file.size = (file.size + 2047) & ~2047;
    for (i = 0; i < 50; i++) {
        CdControl(CdlSetloc, (u_char *)&file.pos, 0);
        CdRead(file.size >> 11, (u_long *)DAT_80199154.buffer, CdlModeSpeed);
        while ((sync = CdReadSync(1, result)) > 0) {
        }
        if (sync == 0) {
            i = 100;
        }
    }
    func_80043b38(DAT_80199154.buffer);
    func_8003c868(100, 30, 220, 150, 0, 0, 120, 120, 0xff, 0x1f, 0x723c, 0x58, 0x58, 0x58);
    func_8003c868(96, 160, 224, 184, 0, 120, 128, 24, 0xff, 0x1f, 0x727c, 0x58, 0x58, 0x58);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&DAT_80199154.disp[1]);
    PutDrawEnv(&DAT_80199154.draw[1]);
    SetDispMask(1);
    DAT_80199154.fade_level = 0;
    DAT_80199154.fade_hold = 0;
}

ADDRESS(0x8003cbc8, 0x134)
void func_8003cbc8(void)
{
    u8 level;

    if (DAT_80199154.fade_hold != 0) {
        if (DAT_80199154.fade_level == 0) {
            DAT_80199154.fade_level = 0xff;
            DAT_80199154.fade_hold--;
        } else {
            DAT_80199154.fade_level--;
        }
    } else if (DAT_80199154.fade_level != 0) {
        level = DAT_80199154.fade_level >> 1;
        DAT_80199154.frame = DAT_80199154.frame == 0;
        PutDispEnv(&DAT_80199154.disp[DAT_80199154.frame]);
        PutDrawEnv(&DAT_80199154.draw[DAT_80199154.frame]);
        func_8003c968(level, level >> 1);
        func_8003c968(level >> 1, level >> 2);
        DrawSync(0);
        VSync(0);
        DAT_80199154.fade_level--;
    }
}

ADDRESS(0x8003ccfc, 0x1c8)
void func_8003ccfc(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                   u16 tpage, u16 clut, u8 r, u8 g, u8 b, s32 z)
{
    POLY_FT4 *p;

    p = (POLY_FT4 *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(POLY_FT4);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    setRGB0(p, r, g, b);
    p->tpage = tpage;
    p->clut = clut;
    setUVWH(p, u, v, w, h);
    p->x0 = p->x2 = x0;
    p->x1 = p->x3 = x1;
    p->y0 = p->y1 = y0;
    p->y2 = p->y3 = y1;
    setPolyFT4(p);
    if (semi != 0xff) {
        setSemiTrans(p, 1);
    }
    LINK_PRIM(z, p);
}

ADDRESS(0x8003cec4, 0x178)
void func_8003cec4(int x0, int y0, int x1, int y1, u8 semi, u32 *rgb0, u32 *rgb1, u32 *rgb2,
                   u32 *rgb3, s32 z)
{
    POLY_G4 *p;

    p = (POLY_G4 *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(POLY_G4);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    *(u32 *)&p->r0 = *rgb0;
    *(u32 *)&p->r1 = *rgb1;
    *(u32 *)&p->r2 = *rgb2;
    *(u32 *)&p->r3 = *rgb3;
    p->x0 = p->x2 = x0;
    p->x1 = p->x3 = x1;
    p->y0 = p->y1 = y0;
    p->y2 = p->y3 = y1;
    setPolyG4(p);
    if (semi != 0xff) {
        setSemiTrans(p, semi);
    }
    LINK_PRIM(z, p);
}

ADDRESS(0x8003d03c, 0x244)
void func_8003d03c(int x, int y, u8 u, u8 v, u8 w, u8 h, u8 semi, u16 tpage, u16 clut, u8 r,
                   u8 g, u8 b, s32 z)
{
    DR_MODE *mode;
    SPRT *p;

    mode = (DR_MODE *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(DR_MODE);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    p = (SPRT *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(SPRT);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    p->x0 = x;
    p->y0 = y;
    p->u0 = u;
    p->v0 = v;
    p->r0 = r;
    p->g0 = g;
    p->b0 = b;
    p->clut = clut;
    p->w = w;
    p->h = h;
    SetDrawMode(mode, 0, 1, tpage, 0);
    setSprt(p);
    if (semi != 0xff) {
        setSemiTrans(p, 1);
    }
    LINK_PRIM(z, p);
    LINK_PRIM(z, mode);
}

ADDRESS(0x8003d280, 0x10c)
void func_8003d280(void)
{
    s32 y;

    if (DAT_801b24e4.unk154 < 240) {
        y = DAT_801b24e4.unk154 + 120;
        if (y < 0) {
            y = 0;
        }
        func_8003ccfc(0, 0, 320, 240, 0, 0xa0, 15, 15, 0xff, 0x6c, 0x7a8d, 0x3c, 0x3c, 0x80, 0x1fff);
        func_8003ccfc(0, y, 320, 240, 0, 0xb0, 15, 15, 1, 0x2c, 0x7a8d, 0x3c, 0x3c, 0x80, 0x40);
    }
}

ADDRESS(0x8003d38c, 0x90)
void func_8003d38c(void)
{
    s32 y;

    if (DAT_801b24e4.unk158 < 240) {
        y = DAT_801b24e4.unk158 + 120;
        if (y < 0) {
            y = 0;
        }
        func_8003ccfc(0, y, 320, 240, 0x10, 0xa0, 15, 15, 1, 0x2c, 0x7a8d, 0x3c, 0x3c, 0x3c, 0x40);
    }
}

ADDRESS(0x8003d41c, 0x90)
void func_8003d41c(void)
{
    s32 y;

    if (DAT_801b24e4.unk15c < 240) {
        y = DAT_801b24e4.unk15c + 120;
        if (y < 0) {
            y = 0;
        }
        func_8003ccfc(0, y, 320, 240, 0x10, 0xb0, 15, 15, 1, 0x2c, 0x7a8d, 0x3c, 0x3c, 0x3c, 0x40);
    }
}

/* Same body as func_8003d568; retail keeps both. */

ADDRESS(0x8003d4ac, 0xbc)
void func_8003d4ac(void)
{
    if (DAT_80199154.tint.mode != 0xff) {
        func_8003ccfc(0, 0, 320, 240, 0, 0xa0, 15, 15, 1, ((DAT_80199154.tint.mode & 3) << 5) | 12,
                      0x7a8d, DAT_80199154.tint.r, DAT_80199154.tint.g, DAT_80199154.tint.b,
                      (DAT_80199154.tint.mode & 0x80) ? 1 : 0x40);
    }
}

ADDRESS(0x8003d568, 0xbc)
void func_8003d568(void)
{
    if (DAT_80199154.tint.mode != 0xff) {
        func_8003ccfc(0, 0, 320, 240, 0, 0xa0, 15, 15, 1, ((DAT_80199154.tint.mode & 3) << 5) | 12,
                      0x7a8d, DAT_80199154.tint.r, DAT_80199154.tint.g, DAT_80199154.tint.b,
                      (DAT_80199154.tint.mode & 0x80) ? 1 : 0x40);
    }
}

ADDRESS(0x8003d624, 0x28)
void func_8003d624(u8 mode, u8 r, u8 g, u8 b)
{
    DAT_80199154.tint.mode = mode;
    DAT_80199154.tint.r = r;
    DAT_80199154.tint.g = g;
    DAT_80199154.tint.b = b;
}

ADDRESS(0x8003d64c, 0x150)
void func_8003d64c(void)
{
    if (DAT_80199154.sum[0].count != 0) {
        func_8003ccfc(0, 0, 320, 240, 0, 0xa0, 15, 15, 1, ((DAT_80199154.sum[0].mode & 3) << 5) | 12,
                      0x7a8d, DAT_80199154.sum[0].r / DAT_80199154.sum[0].count,
                      DAT_80199154.sum[0].g / DAT_80199154.sum[0].count,
                      DAT_80199154.sum[0].b / DAT_80199154.sum[0].count, 0x30);
    }
}

ADDRESS(0x8003d79c, 0x150)
void func_8003d79c(void)
{
    if (DAT_80199154.sum[1].count != 0) {
        func_8003ccfc(0, 0, 320, 240, 0, 0xa0, 15, 15, 1, ((DAT_80199154.sum[1].mode & 3) << 5) | 12,
                      0x7a8d, DAT_80199154.sum[1].r / DAT_80199154.sum[1].count,
                      DAT_80199154.sum[1].g / DAT_80199154.sum[1].count,
                      DAT_80199154.sum[1].b / DAT_80199154.sum[1].count, 0x30);
    }
}

ADDRESS(0x8003d8ec, 0x124)
void func_8003d8ec(s32 r, s32 g, s32 b, s32 scale, s32 which)
{
    switch (which) {
    case 1:
        DAT_80199154.sum[0].count++;
        DAT_80199154.sum[0].mode = which;
        DAT_80199154.sum[0].r += (r * scale) >> 12;
        DAT_80199154.sum[0].g += (g * scale) >> 12;
        DAT_80199154.sum[0].b += (b * scale) >> 12;
        break;
    case 2:
        DAT_80199154.sum[1].count++;
        DAT_80199154.sum[1].mode = which;
        DAT_80199154.sum[1].r += (r * scale) >> 12;
        DAT_80199154.sum[1].g += (g * scale) >> 12;
        DAT_80199154.sum[1].b += (b * scale) >> 12;
        break;
    }
}

ADDRESS(0x8003da10, 0xdc)
void func_8003da10(s32 r, s32 g, s32 b, s32 scale, s32 which)
{
    switch (which) {
    case 1:
        DAT_80199154.sum[0].count++;
        DAT_80199154.sum[0].mode = which;
        DAT_80199154.sum[0].r = (r * scale) >> 12;
        DAT_80199154.sum[0].g = (g * scale) >> 12;
        DAT_80199154.sum[0].b = (b * scale) >> 12;
        break;
    case 2:
        DAT_80199154.sum[1].count++;
        DAT_80199154.sum[1].mode = which;
        DAT_80199154.sum[1].r = (r * scale) >> 12;
        DAT_80199154.sum[1].g = (g * scale) >> 12;
        DAT_80199154.sum[1].b = (b * scale) >> 12;
        break;
    }
}

ADDRESS(0x8003daec, 0x464)
void func_8003daec(u8 *rgb, s32 x, s32 y, s32 step)
{
    s32 count;
    u8 r;
    u8 g;
    u8 b;
    POLY_FT4 *p;
    s16 top;
    s16 bottom;
    s16 right;

    count = 32;
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
    DrawSync(0);
    DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x7400;
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end;
    DAT_80199154.pools[1].end = DAT_80199154.pools[0].end + 0x7400;
    StoreImage(&DAT_8009c2d0, (u_long *)DAT_80199154.pools[1].end);
    DrawSync(0);
    MoveImage(&DAT_80199154.draw[DAT_80199154.frame].clip, DAT_8009c2d0.x, DAT_8009c2d0.y);
    DrawSync(0);
    DAT_80199154.draw[0].isbg = 0;
    DAT_80199154.draw[1].isbg = 0;
    while (count > 0) {
        if (step < 0) {
            r -= 4;
            g -= 4;
            b -= 4;
        } else {
            r += 3;
            g += 3;
            b += 3;
        }
        func_80035630();
        p = (POLY_FT4 *)DAT_80199154.pool->next;
        DAT_80199154.pool->next += sizeof(POLY_FT4);
        if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
            return;
        }
        DAT_80199154.prim_count++;
        top = y;
        bottom = 240 - y;
        setPolyFT4(p);
        p->y2 = top;
        p->y3 = top;
        p->u0 = 0;
        p->v0 = 0;
        p->v1 = 0;
        p->u2 = 0;
        p->clut = 0;
        p->x1 = 192;
        p->x3 = 192;
        p->u1 = 192;
        p->v2 = 240;
        p->u3 = 192;
        p->x0 = 320 - x;
        p->y0 = bottom;
        p->y1 = bottom;
        p->x2 = 320 - x;
        p->v3 = 240;
        p->tpage = GetTPage(2, 0, 320, 0);
        p->r0 = r;
        p->g0 = g;
        p->b0 = b;
        addPrim(DAT_80199154.ot, p);
        p = (POLY_FT4 *)DAT_80199154.pool->next;
        DAT_80199154.pool->next += sizeof(POLY_FT4);
        if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
            return;
        }
        DAT_80199154.prim_count++;
        right = x;
        setPolyFT4(p);
        p->x0 = 192;
        p->x2 = 192;
        p->v2 = 240;
        p->y0 = bottom;
        p->x1 = right;
        p->y1 = bottom;
        p->y2 = top;
        p->x3 = right;
        p->y3 = top;
        p->u0 = 0;
        p->v0 = 0;
        p->u1 = 128;
        p->v1 = 0;
        p->u2 = 0;
        p->u3 = 128;
        p->v3 = 240;
        p->clut = 0;
        p->tpage = GetTPage(2, 0, 512, 0);
        p->r0 = r;
        p->g0 = g;
        p->b0 = b;
        addPrim(DAT_80199154.ot, p);
        count--;
        DrawSync(0);
        func_80035700();
        x += step * 4;
        y += step * 3;
    }
    DAT_80199154.draw[0].isbg = 1;
    DAT_80199154.draw[1].isbg = 1;
    LoadImage(&DAT_8009c2d0, (u_long *)DAT_80199154.pools[1].end);
    DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x1a000;
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end;
    DAT_80199154.pools[1].end = DAT_80199154.pools[0].end + 0x1a000;
    DrawSync(0);
}

/*
 * Draws the weapon model (resident model 32) while the player attacks
 * (unkc0 is the attack time, -1 for none): lit by the light record of the map
 * layer under the player, posed by func_800431e8 and placed by the weapon
 * record's offset and angle; drawn at depth offset 24.
 */

ADDRESS(0x8003df50, 0x3fc)
void func_8003df50(void)
{
    MATRIX m;
    ScratchPad *scratch;
    Record6c *light;
    u8 *file;
    u8 *model;
    u8 *vertex;
    ScreenVertex *screen;
    s32 i;

    if (DAT_801b24e4.unkc0 == -1) {
        return;
    }
    scratch = SCRATCH;
    light = &DAT_80199154.unk15da8[((MapLayer *)((u8 *)&DAT_801d4464.cells[DAT_801b24e4.pos.vz >> 11]
                                                         [DAT_801b24e4.pos.vx >> 11] +
                                                DAT_801b24e4.unk160))->unk4 & 0x3f];
    gte_SetColorMatrix(&light->unk50);
    gte_SetLightMatrix(&light->unk0[0]);
    func_80035358(light->unk68, light->unk6a);
    gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
    m.t[0] = (s16)DAT_801b24e4.unkb0->unk34;
    m.t[1] = (s16)DAT_801b24e4.unkb0->unk36;
    m.t[2] = (s16)DAT_801b24e4.unkb0->unk38;
    RotMatrix((SVECTOR *)&DAT_801b24e4.unkb0->unk3c, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    file = DAT_80199154.models[32];
    model = file + ((ModelFile *)file)->vertices;
    SCRATCH->model = model;
    if (func_800431e8((AnimSlot **)&DAT_801b24e4.unkb8, 32, DAT_801b24e4.unkca,
                      DAT_801b24e4.unkc0, ((ModelData *)model)->parts[0].unk4) == 0) {
        return;
    }
    SCRATCH->ot = DAT_80199154.ot;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->next = DAT_80199154.pool->next;
    SCRATCH->end = DAT_80199154.pool->end;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->draw.count.clut = 0;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&DAT_8009c2e0;
    vertex = SCRATCH->vertices;
    screen = DAT_80199154.screen;
    i = ((ModelData *)model)->parts[0].unk4;
    while (--i != -1) {
        gte_ldv0(vertex);
        gte_rtps();
        gte_stsxy(&screen->sxy);
        gte_stszotz(&screen->z);
        if (DAT_80199154.unk15b28 != DAT_80199154.unk15b2c) {
            screen->fog = ((screen->z - (DAT_80199154.unk15b28 >> 2)) << 14) /
                          (DAT_80199154.unk15b2c - DAT_80199154.unk15b28);
        } else {
            screen->fog = 0;
        }
        if (screen->fog >= 7952) {
            screen->z = screen->fog = 0x1f0f;
        }
        if (screen->fog < 0) {
            screen->fog = 0;
        }
        screen++;
        vertex += 8;
    }
    func_80035ca4(0, 24);
    DAT_80199154.pool->next = scratch->next;
    DAT_80199154.unk159bc = scratch->draw.count.unk4;
    DAT_80199154.ot_count = scratch->draw.count.ot_count;
}

/*
 * Draws model file id (a monster or object model) at pos turned by rotation:
 * lit by the light record of the map layer under it (area 1 or 2), blended by
 * t toward light record light2 (0xff for none), scaled by scale and turned by
 * view when given, posed by func_800431e8 for anim < 128 (part anim & 0x7f
 * otherwise). flags: 0x40 func_800366a8 / flat, 4 func_80037bec (flags & 3),
 * else func_80035ca4; 0x80 keeps the depth offset for models below the eye.
 * With view == NULL the vertices are transformed flat (x/y kept, z = depth).
 */

ADDRESS(0x8003e34c, 0xfb8)
void func_8003e34c(u8 area, u16 id, VECTOR *pos, SVECTOR *rotation, SVECTOR *scale,
                   AnimSlot **owner, MATRIX *view, u16 anim, u16 time, u8 light2, s16 t,
                   s32 flags, s32 depth)
{
    SVECTOR rel;
    VECTOR delta;
    VECTOR size;
    MATRIX m;
    MATRIX unused;
    MATRIX lm;
    MATRIX blend;
    long flag;
    MATRIX *mat;
    ScratchPad *scratch;
    Record6c *light;
    Record6c *other;
    MapLayer *layer;
    ModelPart *part;
    u8 *file;
    u8 *model;
    s32 n;
    u8 mode;

    mat = &m;
    scratch = SCRATCH;
    SCRATCH->draw.count.clip = 100;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->unk48 = DAT_80199154.screen;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->next = DAT_80199154.pool->next;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&DAT_8009c2e0;
    SCRATCH->end = DAT_80199154.pool->end;
    mode = flags;
    if (SCRATCH->end - 30720 < SCRATCH->next) {
        mode &= 0xbf;
    }
    gte_SetRotMatrix(&DAT_80199154.unk159f8);
    gte_SetTransMatrix(&DAT_80199154.unk159f8);
    if (view != NULL) {
        delta.vx = pos->vx - DAT_80199154.unk15af8.vx;
        delta.vy = pos->vy - DAT_80199154.unk15af8.vy;
        delta.vz = pos->vz - DAT_80199154.unk15af8.vz;
        rel.vx = pos->vx - DAT_80199154.unk15af8.vx;
        rel.vy = pos->vy - DAT_80199154.unk15af8.vy;
        rel.vz = pos->vz - DAT_80199154.unk15af8.vz;
        gte_ldv0(&rel);
        gte_rt();
        gte_stlvnl(mat->t);
        gte_stflg(&flag);
        {
            MapCell *row = DAT_801d4464.cells[pos->vz >> 11];

            layer = row[pos->vx >> 11].layer;
        }
        if (area != 1) {
            layer++;
        }
        light = &DAT_80199154.unk15da8[layer->unk4 & 0x3f];
    } else {
        mat->t[0] = pos->vx;
        mat->t[1] = pos->vy;
        mat->t[2] = pos->vz;
        light = &DAT_80199154.unk15da8
            [((MapLayer *)((u8 *)&DAT_801d4464.cells[DAT_80199154.unk15af8.vz >> 11]
                                                      [DAT_80199154.unk15af8.vx >> 11] +
                           DAT_801b24e4.unk160))->unk4 & 0x3f];
    }
    if (mode & 0x80) {
        mode &= 0x7f;
    } else if (rel.vy <= 0) {
        depth += 240;
    }
    func_800166f4(rotation, mat);
    if (light2 != 0xff) {
        other = &DAT_80199154.unk15da8[light2];
        if (other->unk50.m[0][0] != -1) {
            func_800171b0((MATRIX *)&light->unk50, (MATRIX *)&other->unk50, &blend, t);
            gte_SetColorMatrix(&blend);
        } else {
            gte_SetColorMatrix(&light->unk50);
        }
        if (other->unk0[0].m[0][0] != -1) {
            gte_MulMatrix0(&other->unk0[0], mat, &lm);
        } else {
            gte_MulMatrix0(&light->unk0[0], mat, &lm);
        }
        if (other->unk68 != 0xffff && other->unk6a != 0xffff) {
            func_80035358(func_80017158(light->unk68, other->unk68, t),
                          func_80017158(light->unk6a, other->unk6a, t));
        } else {
            func_80035358(light->unk68, light->unk6a);
        }
        if (other->unk64.r != 0xff) {
            gte_SetBackColor(func_80017158(light->unk64.r, other->unk64.r, t),
                             func_80017158(light->unk64.g, other->unk64.g, t),
                             func_80017158(light->unk64.b, other->unk64.b, t));
        } else {
            gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
        }
    } else {
        gte_SetColorMatrix(&light->unk50);
        func_80035358(light->unk68, light->unk6a);
        gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
        gte_MulMatrix0(&light->unk0[0], mat, &lm);
    }
    gte_SetLightMatrix(&lm);
    if (scale != NULL) {
        size.vx = scale->vx;
        size.vy = scale->vy;
        size.vz = scale->vz;
        ScaleMatrix(mat, &size);
    }
    if (view != NULL) {
        gte_MulMatrix0(view, mat, mat);
    }
    gte_SetRotMatrix(mat);
    gte_SetTransMatrix(mat);
    file = DAT_80199154.models[id];
    model = file + ((ModelFile *)file)->vertices;
    scratch->model = model;
    if (anim < 128) {
        n = 0;
        part = ((ModelData *)model)->parts;
        if (func_800431e8(owner, id, anim, time, ((ModelData *)model)->parts[0].unk4) != 0) {
            goto draw;
        }
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    } else {
        n = anim & 0x7f;
        part = &((ModelData *)scratch->model)->parts[n];
        scratch->part = &((ModelData *)model)->parts[n];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    }
draw:
    if (mode & 0x40) {
        if (view != NULL) {
            func_800366a8(n);
            goto done;
        }
        {
            s32 i = part->unk4;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.unk54);
                screen->sxy.vx = scratch->w.model.unk54.vx;
                screen->sxy.vy = scratch->w.model.unk54.vy;
                screen->fog = scratch->w.model.unk54.vz;
                screen->z = depth;
                screen++;
                vertex += 8;
            }
        }
        func_80035ca4(n, depth);
    } else if (mode & 4) {
        if (view != NULL) {
            s32 i = part->unk4;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rtps();
                gte_stsxy(&screen->sxy);
                gte_stszotz(&screen->z);
                if (DAT_80199154.unk15b28 != DAT_80199154.unk15b2c) {
                    screen->fog = ((screen->z - (DAT_80199154.unk15b28 >> 2)) << 14) /
                                  (DAT_80199154.unk15b2c - DAT_80199154.unk15b28);
                } else {
                    screen->fog = 0;
                }
                if (screen->fog >= 7952) {
                    screen->z = screen->fog = 0x1f0f;
                }
                if (screen->fog < 0) {
                    screen->fog = 0;
                }
                screen++;
                vertex += 8;
            }
        } else {
            s32 i = part->unk4;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.unk54);
                screen->sxy.vx = scratch->w.model.unk54.vx;
                screen->sxy.vy = scratch->w.model.unk54.vy;
                screen->fog = scratch->w.model.unk54.vz;
                screen->z = depth;
                screen++;
                vertex += 8;
            }
        }
        func_80037bec(n, depth, mode & 3);
    } else {
        if (view != NULL) {
            s32 i = part->unk4;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rtps();
                gte_stsxy(&screen->sxy);
                gte_stszotz(&screen->z);
                if (DAT_80199154.unk15b28 != DAT_80199154.unk15b2c) {
                    screen->fog = ((screen->z - (DAT_80199154.unk15b28 >> 2)) << 14) /
                                  (DAT_80199154.unk15b2c - DAT_80199154.unk15b28);
                } else {
                    screen->fog = 0;
                }
                if (screen->fog >= 7952) {
                    screen->z = screen->fog = 0x1f0f;
                }
                if (screen->fog < 0) {
                    screen->fog = 0;
                }
                screen++;
                vertex += 8;
            }
        } else {
            s32 i = part->unk4;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.unk54);
                screen->sxy.vx = scratch->w.model.unk54.vx;
                screen->sxy.vy = scratch->w.model.unk54.vy;
                screen->fog = scratch->w.model.unk54.vz;
                screen->z = depth;
                screen++;
                vertex += 8;
            }
        }
        func_80035ca4(n, depth);
    }
done:
    DAT_80199154.pool->next = scratch->next;
    DAT_80199154.unk159bc = scratch->draw.count.unk4;
    DAT_80199154.ot_count = scratch->draw.count.ot_count;
}

/*
 * func_8003e34c's variant for large models (func_80038844): lit by light
 * record 0 (blended by t toward light2). With a view matrix the model is
 * skipped outside a 1536-unit wide angle around the player's heading, and the
 * translation is kept below 0x8000 by halving it shift times (the view matrix
 * rows are scaled down to match); func_80038844 gets shift and the distance.
 * area is unused.
 */

ADDRESS(0x8003f304, 0xda8)
void func_8003f304(u8 area, u16 id, VECTOR *pos, SVECTOR *rotation, SVECTOR *scale,
                   AnimSlot **owner, MATRIX *view, u16 anim, u16 time, u8 light2, s16 t,
                   u8 flags, s32 depth)
{
    SVECTOR rel;
    VECTOR delta;
    VECTOR size;
    MATRIX m;
    MATRIX vm;
    MATRIX lm;
    MATRIX blend;
    long flag;
    MATRIX *mat;
    ScratchPad *scratch;
    Record6c *light;
    Record6c *other;
    ModelPart *part;
    u8 *file;
    u8 *model;
    s32 n;
    s32 shift;
    s32 turn;
    s32 angle;
    s32 dist;

    mat = &m;
    scratch = SCRATCH;
    SCRATCH->draw.count.clip = 100;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->unk48 = DAT_80199154.screen;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->next = DAT_80199154.pool->next;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&DAT_8009c2e0;
    SCRATCH->end = DAT_80199154.pool->end;
    gte_SetRotMatrix(&DAT_80199154.unk159f8);
    gte_SetTransMatrix(&DAT_80199154.unk159f8);
    light = DAT_80199154.unk15da8;
    if (view != NULL) {
        delta.vx = pos->vx - DAT_80199154.unk15af8.vx;
        delta.vy = pos->vy - DAT_80199154.unk15af8.vy;
        delta.vz = pos->vz - DAT_80199154.unk15af8.vz;
        turn = ratan2(delta.vz, delta.vx) - 1024;
        angle = (DAT_801b24e4.unk124.vy - turn) & 0xfff;
        if ((u32)(angle - 2048) < 1536 || (u32)(angle - 513) < 1536) {
            return;
        }
        shift = 0;
        delta.vx >>= 4;
        delta.vy >>= 4;
        delta.vz >>= 4;
        rel.vx = delta.vx;
        rel.vy = delta.vy;
        rel.vz = delta.vz;
        gte_ldv0(&rel);
        gte_rt();
        gte_stlvnl(m.t);
        gte_stflg(&flag);
        m.t[0] <<= 4;
        m.t[1] <<= 4;
        m.t[2] <<= 4;
        while (mat->t[0] > 0x7fff || mat->t[1] > 0x7fff || mat->t[2] > 0x7fff) {
            shift++;
            mat->t[0] >>= 1;
            mat->t[1] >>= 1;
            mat->t[2] >>= 1;
        }
        light = DAT_80199154.unk15da8;
    } else {
        m.t[0] = pos->vx;
        m.t[1] = pos->vy;
        m.t[2] = pos->vz;
    }
    if (!(flags & 0x80) && rel.vy <= 0) {
        depth += 240;
    }
    func_800166f4(rotation, mat);
    if (light2 != 0xff) {
        other = &DAT_80199154.unk15da8[light2];
        if (other->unk50.m[0][0] != -1) {
            func_800171b0((MATRIX *)&light->unk50, (MATRIX *)&other->unk50, &blend, t);
            gte_SetColorMatrix(&blend);
        } else {
            gte_SetColorMatrix(&light->unk50);
        }
        if (other->unk0[0].m[0][0] != -1) {
            gte_MulMatrix0(&other->unk0[0], mat, &lm);
        } else {
            gte_MulMatrix0(&light->unk0[0], mat, &lm);
        }
        if (other->unk68 != 0xffff && other->unk6a != 0xffff) {
            func_80035358(func_80017158(light->unk68, other->unk68, t),
                          func_80017158(light->unk6a, other->unk6a, t));
        } else {
            func_80035358(light->unk68, light->unk6a);
        }
        if (other->unk64.r != 0xff) {
            gte_SetBackColor(func_80017158(light->unk64.r, other->unk64.r, t),
                             func_80017158(light->unk64.g, other->unk64.g, t),
                             func_80017158(light->unk64.b, other->unk64.b, t));
        } else {
            gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
        }
    } else {
        gte_SetColorMatrix(&light->unk50);
        func_80035358(light->unk68, light->unk6a);
        gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
        gte_MulMatrix0(&light->unk0[0], mat, &lm);
    }
    gte_SetLightMatrix(&lm);
    if (scale != NULL) {
        size.vx = scale->vx;
        size.vy = scale->vy;
        size.vz = scale->vz;
        ScaleMatrix(mat, &size);
    }
    vm = *view;
    vm.m[0][0] >>= shift;
    vm.m[0][1] >>= shift;
    vm.m[0][2] >>= shift;
    vm.m[1][0] >>= shift;
    vm.m[1][1] >>= shift;
    vm.m[1][2] >>= shift;
    vm.m[2][0] >>= shift;
    vm.m[2][1] >>= shift;
    vm.m[2][2] >>= shift;
    if (view != NULL) {
        gte_MulMatrix0(&vm, mat, mat);
    }
    gte_SetRotMatrix(mat);
    gte_SetTransMatrix(mat);
    file = DAT_80199154.models[id];
    model = file + ((ModelFile *)file)->vertices;
    scratch->model = model;
    if (anim < 128) {
        n = 0;
        part = ((ModelData *)model)->parts;
        if (func_800431e8(owner, id, anim, time, ((ModelData *)model)->parts[0].unk4) != 0) {
            goto draw;
        }
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    } else {
        n = anim & 0x7f;
        part = &((ModelData *)scratch->model)->parts[n];
        scratch->part = &((ModelData *)model)->parts[n];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    }
draw:
    dist = SquareRoot0(mat->t[0] * mat->t[0] + mat->t[1] * mat->t[1] + mat->t[2] * mat->t[2]);
    if (view != NULL) {
        s32 i = part->unk4;
        ScreenVertex *screen = scratch->screen;
        u8 *vertex = scratch->vertices;

        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rtps();
            gte_stsxy(&screen->sxy);
            gte_stszotz(&screen->z);
            screen->fog = 0;
            screen++;
            vertex += 8;
        }
    } else {
        s32 i = part->unk4;
        ScreenVertex *screen = scratch->screen;
        u8 *vertex = scratch->vertices;

        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rt();
            gte_stlvnl(&scratch->w.model.unk54);
            screen->sxy.vx = scratch->w.model.unk54.vx;
            screen->sxy.vy = scratch->w.model.unk54.vy;
            screen->fog = scratch->w.model.unk54.vz;
            screen->z = depth;
            screen++;
            vertex += 8;
        }
    }
    func_80038844(n, depth, shift, dist);
    DAT_80199154.pool->next = scratch->next;
    DAT_80199154.unk159bc = scratch->draw.count.unk4;
    DAT_80199154.ot_count = scratch->draw.count.ot_count;
}

/*
 * Draws model file id (an object's model) turned by rotation and placed 100
 * units down in view space: lit by light record light_id, posed by
 * func_800431e8 (the base pose when no slot is free) and drawn by
 * func_80039428 at depth offset depth.
 * depth arrives as int (the calls in func_80040ae4) and is narrowed to u16.
 */

ADDRESS(0x800400ac, 0x4bc)
void func_800400ac(u16 id, SVECTOR *rotation, AnimSlot **owner, u16 anim, u16 time, s32 shade,
                   s32 light_id, int depth_in)
{
    u16 depth = depth_in;
    MATRIX m;
    ScratchPad *scratch;
    Record6c *light;
    u8 *file;
    u8 *model;
    s32 i;
    s32 offset;

    m.t[0] = 0;
    m.t[1] = 100;
    m.t[2] = 0;
    func_800166f4(rotation, &m);
    scratch = SCRATCH;
    gte_MulMatrix0(&DAT_80199154.unk159f8, &m, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    light = &DAT_80199154.unk15da8[light_id & 0x7f];
    gte_SetColorMatrix(&light->unk50);
    gte_SetBackColor(light->unk64.r, light->unk64.g, light->unk64.b);
    gte_SetLightMatrix(&light->unk0[0]);
    file = DAT_80199154.models[id];
    model = file + ((ModelFile *)file)->vertices;
    SCRATCH->ot = DAT_80199154.ot;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->model = model;
    SCRATCH->next = DAT_80199154.pool->next;
    SCRATCH->end = DAT_80199154.pool->end;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->draw.count.clut = 0;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&DAT_8009c2e0;
    if (func_800431e8(owner, id, anim, time, ((ModelData *)model)->parts[0].unk4) == 0) {
        u8 *vertex;
        ScreenVertex *screen;

        SCRATCH->part = ((ModelData *)SCRATCH->model)->parts;
        offset = SCRATCH->part->vertices + 12;
        vertex = SCRATCH->vertices = (u8 *)(offset + (u32)SCRATCH->model);
        screen = SCRATCH->screen;
        i = ((ModelData *)model)->parts[0].unk4;
        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rtps();
            gte_stsxy(&screen->sxy);
            gte_stszotz(&screen->z);
            screen->fog = 0;
            screen++;
            vertex += 8;
        }
    } else {
        u8 *vertex;
        ScreenVertex *screen;

        i = ((ModelData *)model)->parts[0].unk4;
        screen = SCRATCH->screen;
        vertex = SCRATCH->vertices;
        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rtps();
            gte_stsxy(&screen->sxy);
            gte_stszotz(&screen->z);
            screen->fog = 0;
            screen++;
            vertex += 8;
        }
    }
    func_80039428(0, depth, shade);
    DAT_80199154.pool->next = scratch->next;
    DAT_80199154.unk159bc = scratch->draw.count.unk4;
    DAT_80199154.ot_count = scratch->draw.count.ot_count;
}

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
    u8 cells = 0;
    s32 row = (pos->vz >> 11) + DAT_80199154.grid_z - radius;
    s32 col0 = (pos->vx >> 11) + DAT_80199154.grid_x - radius;
    s32 col;
    s32 i;
    s32 j;

    radius *= 2;
    i = radius;
    do {
        if (row >= 0 && (u32)row < 25) {
            col = col0;
            j = radius;
            do {
                if (col >= 0 && (u32)col < 25) {
                    cells |= DAT_80199154.grid[row][col];
                }
                col++;
                j--;
            } while (j != -1);
        }
        row++;
        i--;
    } while (i != -1);
    return cells;
}

ADDRESS(0x800407cc, 0x64)
s32 func_800407cc(VECTOR *pos, s32 range_x, s32 range_z)
{
    s32 z = pos->vz >> 11;
    s32 x;

    if (z - range_z <= DAT_80199154.unk15b14 && DAT_80199154.unk15b14 <= z + range_z) {
        x = pos->vx >> 11;
        if (x - range_x <= DAT_80199154.unk15b10 && DAT_80199154.unk15b10 <= x + range_x) {
            return 1;
        }
    }
    return 0;
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
    s32 i;

    wanted = SCRATCH->u.load.sound_wanted;
    for (i = entry, end = count + entry; i < end; i++, bank++) {
        SoundBank *record = &DAT_801b09fc.banks[bank];
        SoundSlot *slot = record->slot;

        if (*wanted++ != 0) {
            if (slot == NULL) {
                func_80015ce0(file, i, bank);
            } else if (slot->state == 2) {
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
    s32 i;

    wanted = SCRATCH->u.load.model_wanted;
    for (i = entry, end = count + entry; i < end; i++, id++) {
        if (*wanted++ != 0) {
            model = DAT_80199154.models[id];
            if (model == NULL) {
                func_80040830(file, i, id);
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

/*
 * Draws the objects in view once a frame and keeps their files loaded: the
 * Object88 records (models from 128, sound banks from 51), the Object44
 * records (models from 256 and the area bank, sound banks from 147; kind
 * 0x1f plays its sound by the player's distance), the Object4c records and
 * the placed effects. The passes flag the model ids and sound banks in use
 * and func_800409c8/func_800408d0 load the missing ones and release the rest.
 */

ADDRESS(0x80040ae4, 0xff0)
void func_80040ae4(void)
{
    VECTOR pos;
    u8 *model;
    ScratchPad *scratch;
    Object88 *obj;
    Object78 *def;
    Object44 *rec;
    Object44Def *rec_def;
    Object4c *obj4c;
    Effect *effect;
    VECTOR *at;
    s32 i;
    s32 now;
    s32 near;
    s32 dist;
    s32 range;
    s32 volume;
    SoundEffect *sound;
    u16 id;
    u8 mask;
    u8 cells;
    u8 flags;

    scratch = SCRATCH;
    SCRATCH->u.load.model_wanted = SCRATCH->u.load.models;
    SCRATCH->u.load.sound_wanted = SCRATCH->u.load.sounds;
    SCRATCH->ot = DAT_80199154.ot;
    SCRATCH->ot2 = DAT_80199154.ot2;
    func_80018fbc((u32 *)SCRATCH->u.load.models, 0, 0x20);
    func_80018fbc((u32 *)SCRATCH->u.load.sounds, 0, 0x18);
    SCRATCH->draw.count.clut = 0;
    obj = DAT_80185da8.objects;
    for (i = 199; i != -1; i--, obj++) {
        if (obj->unk9 != 1) {
            continue;
        }
        if (obj->unk28 & 0x2000) {
            mask = obj->unk3 | 0x10;
        } else {
            mask = obj->unk3;
        }
        if (!(obj->unk28 & 0x80000)) {
            if (!(func_80040694(&obj->pos) & mask)) {
                continue;
            }
        } else {
            if (!(func_80040708(&obj->pos, 3) & obj->unk3)) {
                continue;
            }
        }
        func_800405e8(obj->unk1 + 128, &model);
        if (model != NULL) {
            at = func_8004eee0(obj, &pos);
            if (obj->unk28 & 0x20) {
                scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
                func_8003e34c(obj->unk3, obj->unk1 + 128, &obj->pos, &scratch->u.load.angle,
                              &obj->scale, &obj->unk5c, &DAT_8007e4c4, obj->unkc, obj->unk18,
                              obj->unk14, obj->unk16, obj->unk13, obj->unk15);
            } else {
                scratch->u.load.angle.vx = obj->angle.vx;
                scratch->u.load.angle.vy = obj->angle.vy + 2048;
                scratch->u.load.angle.vz = obj->angle.vz;
                func_8003e34c(obj->unk3, obj->unk1 + 128, at, &scratch->u.load.angle,
                              &obj->scale, &obj->unk5c, &DAT_80199154.unk159f8, obj->unkc,
                              obj->unk18, obj->unk14, obj->unk16, obj->unk13, obj->unk15);
            }
        }
        def = &DAT_80185da8.defs[obj->unk2];
        scratch->u.load.sounds[def->unk7[0]] = 1;
        scratch->u.load.sounds[def->unk7[1]] = 1;
        scratch->u.load.models[obj->unk1] = 1;
    }
    func_800409c8(0, 0, 0x80, 0x80);
    func_800408d0(2, 0x91, 0x33, 0x60);
    now = DAT_801c12e4.frames;
    func_80018fbc((u32 *)scratch->u.load.models, 0, 0x53);
    func_80018fbc((u32 *)scratch->u.load.sounds, 0, 8);
    if (DAT_80199154.unk15996 == 0) {
        DAT_8018fad4.unk20 = NULL;
    }
    scratch->draw.count.clut = 0;
    rec = DAT_8018fb3c.objects;
    for (i = 395; i != -1; i--, rec++) {
        if (rec->unk6 == 0xff) {
            continue;
        }
        rec->unk3 &= 0x7f;
        switch (rec->unk4) {
        case 0xe5:
        case 0xe9:
            break;
        case 0xf2:
            if (func_800407cc(&rec->pos, rec->arg.m.unk38, rec->arg.m.unk39) == 0) {
                break;
            }
            scratch->u.load.models[rec->unk6] = 1;
            id = func_80040568(rec->unk6);
            func_800405e8(id, &model);
            if (model == NULL) {
                break;
            }
            scratch->u.load.angle.vx = rec->angle.vx;
            scratch->u.load.angle.vy = rec->angle.vy + 2048;
            scratch->u.load.angle.vz = rec->angle.vz;
            pos.vx = ((rec->arg.m.unk3a << 11) + rec->pos.vx) >> 1;
            pos.vz = ((rec->arg.m.unk3c << 11) + rec->pos.vz) >> 1;
            pos.vy = (rec->pos.vy - (rec->arg.m.unk3e << 11)) >> 1;
            func_8003f304(rec->unk0, id, &pos, &scratch->u.load.angle, &rec->scale,
                          &rec->arg.m.unk34, &DAT_80199154.unk159f8, rec->unk1, rec->unka,
                          rec->unk5, rec->unk10, rec->unk2, rec->unke);
            rec->unk3 |= 0x80;
            break;
        case 0xf0:
            if (DAT_80199154.unk15996 == 0) {
                break;
            }
            if (func_800407cc(&rec->pos, rec->arg.v.unk38, rec->arg.v.unk39) == 0) {
                break;
            }
            id = rec->unk6 - 221;
            func_800405e8(id, &model);
            if (model != NULL) {
                func_800400ac(id, &rec->angle, &rec->arg.v.unk34, rec->unk1, rec->unka,
                              rec->arg.v.unk3c, rec->arg.v.unk3b, 0x1fff - rec->arg.v.unk3a);
                rec->unk3 |= 0x80;
            }
            DAT_8018fad4.unk24 = *rec;
            DAT_8018fad4.unk20 = &DAT_8018fad4.unk24;
            break;
        case 0x1f:
            if (func_80046884(rec->pos.vx >> 11, rec->pos.vz >> 11, rec->arg.s.unk38,
                              rec->arg.s.unk39, 0x8000) != 0) {
                sound = &DAT_801b09fc.effects[rec->arg.s.unk3a + 496];
                if (sound->bank >= 147 && sound->bank < 179) {
                    scratch->u.load.sounds[sound->bank - 147] = 1;
                }
                if (rec->arg.s.unk40 - now < 0) {
                    rec->arg.s.unk40 = now + rec->arg.s.unk3e * 6;
                    range = rec->arg.s.unk38 << 10;
                    dist = DAT_801b24e4.pos.vx - (range + rec->pos.vx);
                    if (dist < 0) {
                        dist = -dist;
                    }
                    near = range - dist;
                    range = rec->arg.s.unk39 << 10;
                    dist = DAT_801b24e4.pos.vz - (range + rec->pos.vz);
                    if (dist < 0) {
                        dist = -dist;
                    }
                    dist = range - dist;
                    if (dist < near) {
                        near = dist;
                    }
                    range = rec->arg.s.unk3c << 11;
                    if (near >= range) {
                        volume = rec->arg.s.unk3b;
                    } else {
                        if (range == 0) {
                            break;
                        }
                        volume = rec->arg.s.unk3b * near / range;
                    }
                    if (rec->arg.s.unk3d & 1) {
                        dist = DAT_801b24e4.pos.vy - rec->pos.vy;
                        if (dist < 0) {
                            dist = -dist;
                        }
                        volume -= (rec->arg.s.unk3b * dist) >> 13;
                    }
                    if (volume >= 20) {
                        func_8001576c(rec->arg.s.unk3a + 496, volume);
                    }
                }
            } else {
                rec->arg.s.unk40 = now + rec->arg.s.unk3e * 6;
            }
            break;
        default:
            if (!(rec->unk3 & 2) || (rec->unk3 & 8)) {
                cells = func_80040694(&rec->pos);
                if (!(cells & rec->unk0) && !(rec->unk3 & 8)) {
                    break;
                }
                rec_def = &DAT_8018fb3c.defs[rec->unk6];
            } else {
                rec_def = &DAT_8018fb3c.defs[rec->unk6];
                cells = func_80040708(&rec->pos, rec_def->arg.b.unkc);
                if (!(cells & rec->unk0) && !(rec->unk3 & 8)) {
                    break;
                }
            }
            scratch->u.load.models[rec->unk6] = 1;
            scratch->u.load.sounds[rec_def->unk2] = 1;
            id = func_80040568(rec->unk6);
            func_800405e8(id, &model);
            if (model == NULL) {
                break;
            }
            scratch->u.load.angle.vx = rec->angle.vx;
            scratch->u.load.angle.vy = rec->angle.vy + 2048;
            scratch->u.load.angle.vz = rec->angle.vz;
            flags = rec->unk2;
            if (rec->unk3 & 1) {
                if (cells & 4) {
                    flags |= 0x40;
                } else {
                    flags &= 0xbf;
                }
            }
            if (rec->unk3 & 0x40) {
                scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
                func_8003e34c(rec->unk0, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                              &rec->arg.m.unk34, &DAT_80199154.unk15ab8, rec->unk1, rec->unka,
                              rec->unk5, rec->unk10, flags, rec->unke);
            } else if (rec->unk3 & 8) {
                func_8003f304(rec->unk0, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                              &rec->arg.m.unk34, &DAT_80199154.unk159f8, rec->unk1, rec->unka,
                              rec->unk5, rec->unk10, flags, rec->unke);
            } else {
                func_8003e34c(rec->unk0, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                              &rec->arg.m.unk34, &DAT_80199154.unk159f8, rec->unk1, rec->unka,
                              rec->unk5, rec->unk10, flags, rec->unke);
            }
            rec->unk3 |= 0x80;
            break;
        }
    }
    scratch->u.load.models[265] = 0;
    scratch->u.load.models[264] = 0;
    func_800409c8(0, 0x80, 0x100, 0x118);
    scratch->u.load.model_wanted = &scratch->u.load.models[300];
    if ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.unk17 == 1
        && DAT_8018fad4.previous[0] != 0xff) {
        func_800409c8(1, DAT_8018fad4.previous[0] * 32, DAT_8018fad4.previous[0] * 32 + 556, 32);
    } else {
        func_800409c8(1, DAT_8018fad4.area[0] * 32, DAT_8018fad4.area[0] * 32 + 556, 32);
    }
    func_800408d0(2, 0xf1, 0x93, 0x20);
    if ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.step > 0
        && (rec = DAT_8018fad4.unk20) != NULL) {
        id = rec->unk6 - 221;
        func_800400ac(id, &rec->angle, &rec->arg.v.unk34, rec->unk1, rec->unka,
                      rec->arg.v.unk3c, rec->arg.v.unk3b, 0x1fff - rec->arg.v.unk3a);
        rec->unk3 |= 0x80;
    }
    obj4c = DAT_801b77ec.objects;
    for (i = 127; i != -1; i--, obj4c++) {
        if (obj4c->unk0 == 0xff) {
            continue;
        }
        if ((obj4c->unk8 & 3) == 0) {
            continue;
        }
        if ((obj4c->unk8 & 3) != 2 && !(func_80040694(&obj4c->pos) & obj4c->unka)) {
            continue;
        }
        scratch->draw.count.clut = obj4c->unke;
        switch (obj4c->unk8 & 0xc) {
        case 0:
            scratch->u.load.angle.vx = obj4c->angle.vx;
            scratch->u.load.angle.vy = obj4c->angle.vy + 2048;
            scratch->u.load.angle.vz = obj4c->angle.vz;
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &scratch->u.load.angle,
                          &obj4c->scale, &obj4c->unk40, &DAT_80199154.unk159f8, obj4c->unk4,
                          obj4c->unk14, obj4c->unkc, obj4c->unk12, obj4c->unk9, -60);
            break;
        case 4:
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &obj4c->angle,
                          &obj4c->scale, &obj4c->unk40, &DAT_8007e4c4, obj4c->unk4,
                          obj4c->unk14, obj4c->unkc, obj4c->unk12, obj4c->unk9, -60);
            break;
        case 8:
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &obj4c->angle,
                          &obj4c->scale, &obj4c->unk40, &DAT_80199154.unk15ab8, obj4c->unk4,
                          obj4c->unk14, obj4c->unkc, obj4c->unk12, obj4c->unk9, -60);
            break;
        case 0xc:
            func_8003e34c(obj4c->unka, obj4c->unk3 + 40, &obj4c->pos, &obj4c->angle,
                          &obj4c->scale, &obj4c->unk40, NULL, obj4c->unk4, obj4c->unk14,
                          obj4c->unkc, obj4c->unk12, obj4c->unk9, 20);
            break;
        }
    }
    effect = DAT_80182968;
    scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
    scratch->draw.count.clut = 0;
    for (i = 127; i != -1; i--, effect++) {
        if (effect->id == 0xffff) {
            continue;
        }
        if (effect->id & 0x8000) {
            func_8003f304(effect->unk2, (effect->id & 0x7fff) + 40, &effect->pos,
                          &scratch->u.load.angle, NULL, NULL, &DAT_8007e4c4, effect->unk5 + 128, 0,
                          0x3c, 0x1000, 7, 0);
        } else if (func_80040694(&effect->pos) & effect->unk2) {
            func_8003e34c(effect->unk2, effect->id + 40, &effect->pos, &scratch->u.load.angle,
                          NULL, NULL, &DAT_80199154.unk15ab8, effect->unk5 + 128, 0, 0x36, 0x1000,
                          5, 0);
        }
        if (effect->unk4 != 0 && DAT_80182964 % effect->unk4 == 0) {
            effect->unk5++;
            if (effect->unk5 >= effect->unk3) {
                effect->unk5 = 0;
            }
        }
    }
    DAT_80182964++;
}

ADDRESS(0x80041ad4, 0x2c8)
void func_80041ad4(SpriteRec *sprite, u16 mode, u8 *rgb)
{
    DR_MODE *dr;
    SPRT *p;

    dr = (DR_MODE *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(DR_MODE);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    p = (SPRT *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(SPRT);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    if (DAT_8018fad4.area[0] == 10 && mode == 2) {
        p->x0 = sprite->x + 1;
        p->y0 = sprite->y + 1;
    } else {
        p->x0 = sprite->x;
        p->y0 = sprite->y;
    }
    p->clut = sprite->clut;
    p->u0 = sprite->u;
    p->v0 = sprite->v;
    p->w = sprite->w;
    p->h = sprite->h;
    setRGB0(p, rgb[0], rgb[1], rgb[2]);
    setSprt(p);
    if (mode != 0xff) {
        SetDrawMode(dr, 0, 1, sprite->tpage | ((mode & 3) << 5), NULL);
        setSemiTrans(p, 1);
    }
    LINK_PRIM(sprite->z, p);
    LINK_PRIM(sprite->z, dr);
}

ADDRESS(0x80041d9c, 0xcc)
void func_80041d9c(void)
{
    SpriteRec *sprite;
    u8 bright[4];
    u8 dim[4];

    sprite = DAT_80081928;
    bright[0] = bright[1] = bright[2] = DAT_80199154.message_level;
    dim[0] = dim[2] = dim[1] = DAT_80199154.message_level >> 2;
    for (; sprite->visible != 0xff; sprite++) {
        if (sprite->visible & 1) {
            func_80041ad4(sprite, 1, bright);
        }
        if (sprite->visible & 2) {
            func_80041ad4(sprite, 2, bright);
        }
        if (sprite->visible & 4) {
            func_80041ad4(sprite, 2, dim);
        }
    }
}

ADDRESS(0x80041e68, 0x84)
void func_80041e68(void)
{
    SpriteRec *sprite;
    u8 rgb[4];

    sprite = DAT_800819b4;
    rgb[0] = rgb[1] = rgb[2] = DAT_801b24e4.unkfd;
    for (; sprite->visible != 0xff; sprite++) {
        if (sprite->visible != 0) {
            func_80041ad4(sprite, sprite->mode, rgb);
        }
    }
}

ADDRESS(0x80041eec, 0xa0)
void func_80041eec(s32 message, ...)
{
    va_list args;

    va_start(args, message);
    if (message < 240) {
        if (DAT_80199154.message_queue[DAT_80199154.message_head] == 0xff) {
            DAT_80199154.message_queue[DAT_80199154.message_head] = message;
            if (message == 25) {
                DAT_80199154.message_arg[DAT_80199154.message_head] = va_arg(args, s32);
            }
            DAT_80199154.message_head = (DAT_80199154.message_head + 1) & 7;
        }
    }
    va_end(args);
}

ADDRESS(0x80041f8c, 0x10)
void func_80041f8c(SpriteRec *sprite, u8 digit)
{
    sprite->u = digit * 8;
}

ADDRESS(0x80041f9c, 0x31c)
void func_80041f9c(void)
{
    u32 message;
    u8 digits[20];

    switch (DAT_80199154.message_state) {
    case 0:
        message = DAT_80199154.message_queue[DAT_80199154.message_tail];
        if (message == 0xff) {
            break;
        }
        DAT_80199154.message_state = 1;
        DAT_80199154.message_timer = 15;
        DAT_80199154.message_level = 0;
        if (message == 25) {
            DAT_80081928[0].visible = 0;
            DAT_80081928[1].visible = 3;
            DAT_80081928[1].u = 0x80;
            DAT_80081928[1].v = 0x62;
            func_800277c0(DAT_80199154.message_arg[DAT_80199154.message_tail], 4, 0, 0, digits);
            DAT_80081928[2].visible = 3;
            func_80041f8c(&DAT_80081928[2], digits[3]);
            DAT_80081928[3].visible = 3;
            func_80041f8c(&DAT_80081928[3], digits[2]);
            DAT_80081928[4].visible = 3;
            func_80041f8c(&DAT_80081928[4], digits[1]);
            DAT_80081928[5].visible = 3;
            func_80041f8c(&DAT_80081928[5], digits[0]);
        } else {
            DAT_80081928[0].visible = 3;
            DAT_80081928[1].visible = DAT_80081928[2].visible = DAT_80081928[3].visible =
                DAT_80081928[4].visible = DAT_80081928[5].visible = 0;
            DAT_80081928[0].u = message / 18 * 128;
            DAT_80081928[0].v = (u8)(message % 18) * 14;
        }
        break;
    case 1:
        DAT_80199154.message_level += 20;
        if (DAT_80199154.message_level >= 100) {
            DAT_80199154.message_state = 2;
        }
        break;
    case 2:
        if (--DAT_80199154.message_timer == 0) {
            DAT_80199154.message_state = 3;
        }
        break;
    case 3:
        DAT_80199154.message_level -= 20;
        if (DAT_80199154.message_level == 0) {
            message = DAT_80081928[0].visible = DAT_80081928[1].visible =
                DAT_80081928[2].visible = DAT_80081928[3].visible = DAT_80081928[4].visible =
                    DAT_80081928[5].visible =
                        DAT_80199154.message_queue[DAT_80199154.message_tail];
            do {
                DAT_80199154.message_queue[DAT_80199154.message_tail] = 0xff;
                DAT_80199154.message_tail = (DAT_80199154.message_tail + 1) & 7;
            } while (message == DAT_80199154.message_queue[DAT_80199154.message_tail]
                     && message != 25);
            DAT_80199154.message_state = 0;
        }
        break;
    }
}

ADDRESS(0x800422b8, 0x654)
void func_800422b8(VECTOR *pos, SVECTOR *angle)
{
    s32 i;

    DAT_8009c2e0.b = DAT_8009c2e0.g = DAT_8009c2e0.r = DAT_801b24e4.unkfd;
    func_800357e8(pos, angle);
    func_800351fc();
    func_80041f9c();
    func_80034bf4();
    func_80035630();
    func_80043858();
    func_8003df50();
    DAT_800819b4[13].visible = DAT_800819b4[14].visible = 0;
    DAT_80081c20[0].visible = DAT_801b24e4.unkfa & 1;
    DAT_80081c20[1].visible = (DAT_801b24e4.unkfa & 2) >> 1;
    DAT_800819b4[0].visible = DAT_800819b4[1].visible = DAT_800819b4[2].visible =
        DAT_800819b4[3].visible = DAT_800819b4[4].visible = DAT_800819b4[5].visible =
            DAT_800819b4[6].visible = DAT_800819b4[7].visible = DAT_800819b4[8].visible =
                DAT_800819b4[9].visible = DAT_800819b4[10].visible = DAT_800819b4[11].visible =
                    DAT_800819b4[12].visible = DAT_801b24e4.unkf9 & 1;
    DAT_800819b4[15].visible = DAT_800819b4[16].visible = DAT_800819b4[17].visible =
        DAT_800819b4[18].visible = DAT_800819b4[19].visible = DAT_800819b4[20].visible =
            DAT_800819b4[21].visible = DAT_800819b4[22].visible = DAT_800819b4[23].visible =
                DAT_800819b4[24].visible = DAT_800819b4[25].visible = DAT_800819b4[26].visible =
                    DAT_800819b4[27].visible = DAT_800819b4[28].visible =
                        DAT_800819b4[29].visible = (DAT_801b24e4.unkf9 & 2) >> 1;
    i = DAT_801b24e4.unkfa - 1;
    if (i >= 0) {
        DAT_8009c2ec += func_80016a98(DAT_80081c20[i].angle.vy, DAT_80199154.unk15b08.vy);
        if (DAT_8009c2ec > 0) {
            DAT_8009c2ec -= (DAT_8009c2ec + 7) >> 3;
        } else if (DAT_8009c2ec < 0) {
            DAT_8009c2ec -= (DAT_8009c2ec - 7) >> 3;
        }
        DAT_80081c20[i == 0].angle.vy = DAT_80081c20[i].angle.vy =
            DAT_80081c20[i].angle.vy + (DAT_8009c2ec >> 6);
        DAT_80081c20[i == 0].angle.vx = DAT_80081c20[i].angle.vx = DAT_80199154.unk15b08.vx;
    }
    if (DAT_801b24e4.unkf9 == 1) {
        DAT_800819b4[4].unka = DAT_800819b4[4].w = DAT_801b24e4.hp * 48 / DAT_801b24e4.hp_max;
        DAT_800819b4[5].unka = DAT_800819b4[5].w = DAT_801b24e4.unk1e * 48 / 5000;
        DAT_800819b4[6].unka = DAT_800819b4[6].w = DAT_801b24e4.unk1c * 48 / DAT_801b24e4.unk1a;
        DAT_800819b4[7].unka = DAT_800819b4[7].w = DAT_801b24e4.unk22 * 48 / 5000;
    } else if (DAT_801b24e4.unkf9 == 2) {
        u16 hp;
        u16 mp;
        s32 n;

        hp = DAT_801b24e4.hp % 1000;
        n = hp;
        DAT_800819b4[17].u = n / 100 * 12 + 0x80;
        DAT_800819b4[18].u = n % 100 / 10 * 12 + 0x80;
        DAT_800819b4[19].u = n % 10 * 12 + 0x80;
        mp = DAT_801b24e4.unk1c % 1000;
        n = mp;
        DAT_800819b4[22].u = n / 100 * 12 + 0x80;
        DAT_800819b4[23].u = n % 100 / 10 * 12 + 0x80;
        DAT_800819b4[24].u = n % 10 * 12 + 0x80;
        DAT_800819b4[25].unka = DAT_800819b4[25].w = DAT_801b24e4.unk1e * 64 / 5000;
        DAT_800819b4[26].unka = DAT_800819b4[26].w = DAT_801b24e4.unk22 * 64 / 5000;
    }
    func_8003c35c();
    func_80041e68();
    func_80041d9c();
    func_8003bfd0();
    func_80040ae4();
    func_8003d280();
    func_8003d38c();
    func_8003d41c();
    func_8003d568();
    func_8003d64c();
    func_8003d79c();
    func_80035700();
    func_80019614();
    func_80043940();
}

/*
 * Draws model 3 (the menu's item model) with the matrices the caller set:
 * projects its vertices into Data80199154.screen with a depth-cue value from
 * the fog range, then draws part 0 at depth offset 100.
 * Not exact: retail stores the CLUT offset between SCRATCH->screen and
 * SCRATCH->end; every statement order tried schedules it earlier.
 */

ADDRESS(0x8004290c, 0x264)
void func_8004290c(void)
{
    ScratchPad *scratch;
    u8 *model;
    u8 *vertex;
    ScreenVertex *screen;
    s32 i;
    s32 offset;

    scratch = SCRATCH;
    gte_SetBackColor(60, 60, 60);
    SetGeomScreen(200);
    model = DAT_80199154.unk10148[3];
    SCRATCH->model = model;
    SCRATCH->part = ((ModelData *)model)->parts;
    offset = SCRATCH->part->vertices + 12;
    SCRATCH->vertices = (u8 *)(offset + (s32)model);
    func_80035358(20000, 24000);
    SCRATCH->ot = DAT_80199154.ot;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->next = DAT_80199154.pool->next;
    SCRATCH->draw.count.clut = 0;
    SCRATCH->end = DAT_80199154.pool->end;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&DAT_8009c2e0;
    SCRATCH->screen = DAT_80199154.screen;
    i = ((ModelData *)SCRATCH->model)->parts[0].unk4;
    vertex = SCRATCH->vertices;
    screen = DAT_80199154.screen;
    while (--i != -1) {
        gte_ldv0(vertex);
        gte_rtps();
        gte_stsxy(&screen->sxy);
        gte_stszotz(&screen->z);
        if (DAT_80199154.unk15b28 != DAT_80199154.unk15b2c) {
            screen->fog = ((screen->z - (DAT_80199154.unk15b28 >> 2)) << 14) /
                          (DAT_80199154.unk15b2c - DAT_80199154.unk15b28);
        } else {
            screen->fog = 0;
        }
        if (screen->fog >= 7952) {
            screen->z = screen->fog = 0x1f0f;
        }
        if (screen->fog < 0) {
            screen->fog = 0;
        }
        screen++;
        vertex += 8;
    }
    func_80035ca4(0, 100);
    DAT_80199154.pool->next = scratch->next;
    DAT_80199154.unk159bc = scratch->draw.count.unk4;
    DAT_80199154.ot_count = scratch->draw.count.ot_count;
}

ADDRESS(0x80042b70, 0x38)
void func_80042b70(u16 id)
{
    u8 *file;

    file = DAT_80199154.models[id];
    DAT_80199154.vertices = file + ((ModelFile *)file)->vertices;
}

ADDRESS(0x80042ba8, 0xbc)
void func_80042ba8(u16 id, u8 *file)
{
    u16 count;
    u32 size;

    count = *(u16 *)file;
    file += 4;
    while (count--) {
        size = *(u32 *)file;
        if (size >= 5) {
            DAT_80199154.models[id] = file;
            func_80042b70(id);
            func_8003591c(DAT_80199154.vertices);
        }
        file += size;
        id++;
    }
}

ADDRESS(0x80042c64, 0x48)
void func_80042c64(u16 id, u8 *file)
{
    DAT_80199154.models[id] = file;
    func_80042b70(id);
    func_8003591c(DAT_80199154.vertices);
}

ADDRESS(0x80042cac, 0xc4)
AnimKey *func_80042cac(ModelFile *model, s32 anim, s32 time, s32 *index, s32 *frac)
{
    s32 key;
    s32 end;
    s32 start;
    s32 i;
    s32 *keys;
    AnimKey *frame;
    u32 length;
    s32 t;
    u16 *list;

    key = 0;
    end = 0;
    list = (u16 *)((u8 *)model + ((s32 *)((u8 *)model + model->anims))[anim]);
    keys = (s32 *)(list + 2);
    start = 0;
    i = *list;
    while (--i != -1) {
        frame = (AnimKey *)((u8 *)model + *keys++);
        length = frame->length;
        end += length;
        if (time < end) {
            t = ((time - start) << 12) / length;
            if (frame->reverse != 0) {
                t = 0x1000 - t;
            }
            goto done;
        }
        start = end;
        key++;
    }
    key--;
    t = 0x1000;
done:
    *index = key;
    *frac = t;
    return frame;
}

ADDRESS(0x80042d70, 0xc4)
void func_80042d70(SVECTOR *pose, SVECTOR *base, s16 *deltas)
{
    s32 i;
    s32 j;
    s16 v;

    i = *deltas++;
    while (--i != -1) {
        v = *deltas++;
        if (v == DELTA_RUN) {
            j = *deltas++;
            while (--j != -1) {
                pose->vx = base->vx;
                pose->vy = base->vy;
                pose->vz = base->vz;
                pose++;
                base++;
            }
        } else {
            pose->vx = v;
            pose->vy = *deltas++;
            base++;
            pose->vz = *deltas++;
            pose++;
        }
    }
}

ADDRESS(0x80042e34, 0x7c)
void func_80042e34(SVECTOR *pose, s16 *deltas)
{
    s32 i;
    s16 v;

    i = *deltas++;
    while (--i != -1) {
        v = *deltas++;
        if (v == DELTA_RUN) {
            pose += *deltas++;
        } else {
            pose->vx = v;
            pose->vy = *deltas++;
            pose->vz = *deltas++;
            pose++;
        }
    }
}

ADDRESS(0x80042eb0, 0x2bc)
void func_80042eb0(SVECTOR *pose, s16 *deltas, s32 t)
{
    MATRIX m;
    VECTOR scale;
    s32 i;
    s32 j;
    s32 n;
    s16 v;
    s16 *d;
    SVECTOR *out;

    scale.vx = scale.vy = scale.vz = t;
    i = *deltas++;
    n = 0;
    out = pose;
    d = m.m[0];
    while (--i != -1) {
        v = *deltas++;
        if (v == DELTA_RUN) {
            pose += *deltas++;
            if (n != 0) {
                ScaleMatrix(&m, &scale);
                d = m.m[0];
                while (--n != -1) {
                    out->vx += *d++;
                    out->vy += *d++;
                    out->vz += *d++;
                    out++;
                }
                n = 0;
                d = m.m[0];
            }
            out = pose;
        } else {
            *d++ = v - pose->vx;
            *d++ = *deltas++ - pose->vy;
            *d++ = *deltas++ - pose->vz;
            pose++;
            if (n == 2) {
                ScaleMatrix(&m, &scale);
                out[0].vx += m.m[0][0];
                out[0].vy += m.m[0][1];
                out[0].vz += m.m[0][2];
                out[1].vx += m.m[1][0];
                out[1].vy += m.m[1][1];
                out[1].vz += m.m[1][2];
                out[2].vx += m.m[2][0];
                out[2].vy += m.m[2][1];
                out[2].vz += m.m[2][2];
                n = 0;
                d = m.m[0];
                out = pose;
            } else {
                n++;
            }
        }
    }
    if (n != 0) {
        ScaleMatrix(&m, &scale);
        d = m.m[0];
        while (--n != -1) {
            out->vx += *d++;
            out->vy += *d++;
            out->vz += *d++;
            out++;
        }
    }
}

ADDRESS(0x8004316c, 0x7c)
s16 *func_8004316c(s16 *deltas, s32 index)
{
    s32 k;
    s32 i;

    k = 0;
    i = *deltas++;
    while (--i != -1) {
        if (*deltas == DELTA_RUN) {
            deltas++;
            k += *deltas++;
            if (index < k) {
                return NULL;
            }
        } else {
            if (k == index) {
                return deltas;
            }
            k++;
            deltas += 3;
        }
    }
    return NULL;
}

/*
 * Returns 1 for an unanimated model, 0 when no slot is free, else the slot.
 * The allocation retry is a goto loop: as a while loop, `count * 8` would be
 * weighted by loop depth and `count` would outrank `scratch` ($s7/$s8).
 */
ADDRESS(0x800431e8, 0x344)
s32 func_800431e8(AnimSlot **owner, s32 id, s32 anim, s32 time, s32 count)
{
    ModelFile *model = (ModelFile *)DAT_80199154.models[id];
    AnimSlot *slot = *owner;
    ScratchPad *scratch = SCRATCH;
    AnimKey *key;
    s32 *deltas;
    u16 *ids;
    u16 n;
    s32 index;
    s32 frac;

    if (model->unk4 == 0) {
        u8 *data;

        if (slot != NULL) {
            func_80043894(slot);
        }
        data = DAT_80199154.models[id];
        data += ((ModelFile *)data)->vertices;
        scratch->model = data;
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        return 1;
    }

    if (slot == NULL) {
        func_800439b0(&slot);
        if (slot == NULL) {
            return 0;
        }
    init:
        slot->model = id;
        slot->owner = owner;
    retry:
        if ((slot->pose = func_800194b0(count * 8)) == NULL) {
            func_800438e0();
            goto retry;
        }
        *owner = slot;
    } else if (slot->model != id) {
        func_80043894(slot);
        slot->anim = 0xff;
        goto init;
    }

    key = func_80042cac(model, anim, time, &index, &frac);
    if (slot->anim != anim || slot->key != index) {
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        deltas = (s32 *)((u8 *)model + model->deltas);
        n = key->count;
        if (n != 0) {
            ids = key->ids;
            func_80042d70(slot->pose, (SVECTOR *)scratch->vertices,
                          (s16 *)((u8 *)model + deltas[key->first]));
            for (--n; n != 0; --n) {
                func_80042e34(slot->pose, (s16 *)((u8 *)model + deltas[*ids++]));
            }
        } else {
            u16 c = count;
            u32 *src = (u32 *)scratch->vertices;
            u32 *dst = (u32 *)slot->pose;

            do {
                *dst++ = *src++;
                *dst++ = *src++;
            } while (--c != 0);
        }
        slot->base = deltas[key->base];
    }
    slot->anim = anim;
    slot->key = index;

    {
        u16 c = count;
        u32 *src = (u32 *)slot->pose;
        u32 *dst = (u32 *)DAT_80199154.pose;

        do {
            *dst++ = *src++;
            *dst++ = *src++;
        } while (--c != 0);
    }
    func_80042eb0(DAT_80199154.pose, (s16 *)((u8 *)model + slot->base), frac);
    scratch->vertices = (u8 *)DAT_80199154.pose;
    slot->state = 2;
    return (s32)slot;
}

/*
 * id is a full int (narrowed only for func_800405e8): a narrow first
 * parameter would emit an entry conversion, and GCC then drops the stack
 * equivalence of `out`, which changes its allocation priority ($s6/$s7).
 */
ADDRESS(0x8004352c, 0x29c)
s32 func_8004352c(s32 id, s32 anim, s32 time, s32 vertex, SVECTOR *out)
{
    u8 *loaded;
    ModelFile *file;
    u8 *data;
    SVECTOR *vertices;

    func_800405e8(id, &loaded);
    file = (ModelFile *)loaded;
    if (file == NULL) {
        out->vx = out->vy = out->vz = 0;
        return 1;
    }
    data = (u8 *)file + file->vertices;
    if (anim >= 128) {
    single:
        vertices = (SVECTOR *)(data + ((((ModelData *)data)->parts + (anim & 0x7f))->vertices + 12));
        *out = vertices[vertex];
    } else if (file->unk4 == 0) {
        anim = 0;
        goto single;
    } else {
        SVECTOR v;
        s32 index;
        s32 frac;
        AnimKey *key;
        s32 *deltas;
        u16 *ids;
        s32 n;
        s16 *d;

        vertices = (SVECTOR *)(data + (((ModelData *)data)->parts[0].vertices + 12));
        key = func_80042cac(file, anim, time, &index, &frac);
        v = vertices[vertex];
        deltas = (s32 *)((u8 *)file + file->deltas);
        ids = &key->first;
        n = key->count;
        while (--n != -1) {
            d = func_8004316c((s16 *)((u8 *)file + deltas[*ids++]), vertex);
            if (d != NULL) {
                v.vx = *d++;
                v.vy = *d++;
                v.vz = *d;
            }
        }
        d = func_8004316c((s16 *)((u8 *)file + deltas[key->base]), vertex);
        if (d != NULL) {
            v.vx = (((*d++ - v.vx) * frac) >> 12) + v.vx;
            v.vy = (((*d++ - v.vy) * frac) >> 12) + v.vy;
            v.vz = (((*d - v.vz) * frac) >> 12) + v.vz;
        }
        *out = v;
    }
    return 0;
}

ADDRESS(0x800437c8, 0x60)
s32 func_800437c8(s32 id, s32 part)
{
    u8 *file;
    ModelData *data;

    file = DAT_80199154.models[id];
    if (file == NULL) {
        return 0;
    }
    data = (ModelData *)(file + ((ModelFile *)file)->vertices);
    return part < 128 ? data->parts[0].unk4 : (data->parts + (part & 0x7f))->unk4;
}

ADDRESS(0x80043828, 0x30)
void func_80043828(void)
{
    AnimSlot *slot;
    u16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        slot->state = 0;
        slot->pose = NULL;
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043858, 0x3c)
void func_80043858(void)
{
    AnimSlot *slot;
    u16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        if (slot->state != 0) {
            slot->state = 1;
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043894, 0x4c)
void func_80043894(AnimSlot *slot)
{
    slot->state = 0;
    *slot->owner = NULL;
    if (slot->pose != NULL) {
        free(slot->pose);
        slot->pose = NULL;
    }
}

ADDRESS(0x800438e0, 0x60)
void func_800438e0(void)
{
    AnimSlot *slot;
    s16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        if (slot->state != 0) {
            func_80043894(slot);
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043940, 0x70)
void func_80043940(void)
{
    AnimSlot *slot;
    u16 n;

    slot = DAT_80199154.slots;
    n = 12;
    do {
        if (slot->state == 1) {
            func_80043894(slot);
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x800439b0, 0x58)
void func_800439b0(AnimSlot **out)
{
    u16 n;

    *out = DAT_80199154.slots;
    n = 12;
    do {
        if ((*out)->state == 0) {
            (*out)->anim = 0xff;
            return;
        }
        (*out)++;
    } while (--n != 0);
    *out = NULL;
}

ADDRESS(0x80043a08, 0x130)
void func_80043a08(EffectSpawn *spawn)
{
    Effect *effect;
    s32 i;

    effect = DAT_80182968;
    for (i = 127; i != -1; i--) {
        if (spawn->id != 0xffff) {
            effect->id = spawn->id;
            effect->unk2 = spawn->unk4;
            effect->unk3 = spawn->unk2;
            effect->unk4 = spawn->unk3;
            effect->pos.vx = (spawn->cell_x << 11) + spawn->dx;
            effect->pos.vz = (spawn->cell_z << 11) + spawn->dz;
            effect->pos.vy = func_80033b8c(effect->unk2, effect->pos.vx, effect->pos.vz, 0, 0)
                + spawn->dy;
            effect->unk5 = (rand() * effect->unk3) >> 15;
        } else {
            effect->id = 0xffff;
        }
        effect++;
        spawn++;
    }
}

ADDRESS(0x80043b38, 0x80)
void func_80043b38(u8 *file)
{
    TIM_IMAGE tim;
    u_long unused[128];

    OpenTIM((u_long *)file);
    while (ReadTIM(&tim) != NULL) {
        if (tim.paddr != NULL) {
            LoadImage(tim.prect, tim.paddr);
            DrawSync(0);
        }
        if (tim.caddr != NULL) {
            LoadImage(tim.crect, tim.caddr);
            DrawSync(0);
        }
    }
}

/* Fades a full-screen picture by step per frame from level (0..120) over the
 * saved screen. Returns the level when a button was pressed after all were
 * released, -1 when one was held throughout, -2 when the fade ended first. */

ADDRESS(0x80043bb8, 0x61c)
s32 func_80043bb8(s32 level, s32 step)
{
    POLY_FT4 *p;
    s32 result;
    u32 pad;
    s32 dim;

    result = -1;
    for (;;) {
        func_80035630();
        p = (POLY_FT4 *)DAT_80199154.pool->next;
        DAT_80199154.pool->next += sizeof(POLY_FT4);
        dim = 128 - (level >> 1);
        if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
            return KF_VALUELESS;
        }
        DAT_80199154.prim_count++;
        setPolyFT4(p);
        setSemiTrans(p, 1);
        setXYWH(p, 32, 32, 256, 208);
        setUVWH(p, 0, 0, 255, 208);
        p->clut = GetClut(96, 480);
        p->tpage = GetTPage(0, 1, 960, 256);
        setRGB0(p, level, level, level);
        addPrim(DAT_80199154.ot, p);
        p = (POLY_FT4 *)DAT_80199154.pool->next;
        DAT_80199154.pool->next += sizeof(POLY_FT4);
        if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
            return KF_VALUELESS;
        }
        DAT_80199154.prim_count++;
        setPolyFT4(p);
        setSemiTrans(p, 1);
        setXYWH(p, 33, 33, 256, 208);
        setUVWH(p, 0, 0, 255, 208);
        p->clut = GetClut(96, 480);
        p->tpage = GetTPage(0, 2, 960, 256);
        setRGB0(p, level, level, level);
        addPrim(DAT_80199154.ot, p);
        p = (POLY_FT4 *)DAT_80199154.pool->next;
        DAT_80199154.pool->next += sizeof(POLY_FT4);
        if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
            return KF_VALUELESS;
        }
        DAT_80199154.prim_count++;
        setPolyFT4(p);
        setXYWH(p, 0, 0, 192, 240);
        setUVWH(p, 0, 0, 192, 240);
        p->clut = 0;
        p->tpage = GetTPage(2, 0, 320, 0);
        setRGB0(p, dim, dim, dim);
        addPrim(DAT_80199154.ot, p);
        p = (POLY_FT4 *)DAT_80199154.pool->next;
        DAT_80199154.pool->next += sizeof(POLY_FT4);
        if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
            return KF_VALUELESS;
        }
        DAT_80199154.prim_count++;
        setPolyFT4(p);
        setXYWH(p, 192, 0, 128, 240);
        setUVWH(p, 0, 0, 128, 240);
        p->clut = 0;
        p->tpage = GetTPage(2, 0, 512, 0);
        setRGB0(p, dim, dim, dim);
        addPrim(DAT_80199154.ot, p);
        level += step;
        DrawSync(0);
        func_80035700();
        if (level <= 0 || level >= 120) {
            break;
        }
        pad = PadRead(1);
        if (result == -1) {
            if (pad == 0) {
                result = -2;
            }
        } else if (pad != 0) {
            DrawSync(0);
            if (pad & DAT_80081868[7]) {
                DAT_801b24e4.unk178 |= DAT_80081868[7];
            } else if (pad != 0) {
                DAT_801b24e4.unk178 &= ~DAT_80081868[7];
            }
            return level;
        }
    }
    DrawSync(0);
    if (pad & DAT_80081868[7]) {
        DAT_801b24e4.unk178 |= DAT_80081868[7];
    } else if (pad != 0) {
        DAT_801b24e4.unk178 &= ~DAT_80081868[7];
    }
    return result;
}

/* Shows TIM entry `entry` of file `file` between a fade in and a fade out over
 * the saved screen; a button press cuts the wait short. */

ADDRESS(0x800441d4, 0x1f4)
void func_800441d4(u16 file, u16 entry)
{
    s32 level;
    u32 pad;

    DrawSync(0);
    func_8001a154(file, entry, DAT_80199154.buffer);
    ClearImage(&DAT_8009c2d8, 0, 0, 0);
    DrawSync(0);
    func_80043b38(DAT_80199154.buffer);
    DrawSync(0);
    DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x7400;
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end;
    DAT_80199154.pools[1].end = DAT_80199154.pools[0].end + 0x7400;
    StoreImage(&DAT_8009c2d0, (u_long *)DAT_80199154.pools[1].end);
    DrawSync(0);
    MoveImage(&DAT_80199154.draw[DAT_80199154.frame].clip, DAT_8009c2d0.x, DAT_8009c2d0.y);
    DrawSync(0);
    level = func_80043bb8(0, 12);
    if (level < 0) {
        for (;;) {
            pad = PadRead(1);
            if (level == -1) {
                if (pad == 0) {
                    level = -2;
                }
            } else if (pad != 0) {
                break;
            }
        }
        if (pad & DAT_80081868[7]) {
            DAT_801b24e4.unk178 |= DAT_80081868[7];
        } else if (pad != 0) {
            DAT_801b24e4.unk178 &= ~DAT_80081868[7];
        }
        level = 80;
    }
    func_80043bb8(level, -12);
    LoadImage(&DAT_8009c2d0, (u_long *)DAT_80199154.pools[1].end);
    DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x1a000;
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end;
    DAT_80199154.pools[1].end = DAT_80199154.pools[0].end + 0x1a000;
}
