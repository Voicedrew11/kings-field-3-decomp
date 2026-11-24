/*
 * GAME.EXE: full-screen 2D effects (WIP module): the loading screen and its
 * fade, flat and gouraud quads and sprites linked into the ordering table,
 * screen tints and the screen wipe.
 */
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

RODATA(0x80011bd0, 0xc)

DATA(0x8009c2d0, 0x8, ".sdata")
RECT DAT_8009c2d0 = { 320, 0, 320, 240 };

/*
 * Draws the 3D overlays (the compass needles of DAT_80081c20, visible == 1)
 * in view space: each lit by its light record turned by its rotation, scaled,
 * posed by func_800431e8 (the base pose is rebuilt when it returns nonzero,
 * unlike func_800400ac), and projected flat (screen x/y = view x/y, depth =
 * view z / 4) before func_80035ca4.
 * Not exact: besides the $gp read of DAT_8009c2e0 (see func_8003df50), retail
 * sets the &SCRATCH->unk54 base after the empty-list test, not in the prologue.
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
    VECTOR *view;

    scratch = SCRATCH;
    view = &SCRATCH->unk54;
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
        *(u32 *)&scratch->rgb = *(u32 *)&DAT_8009c2e0;
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
                gte_stlvnl(view);
                screen->sxy.vx = scratch->unk54.vx;
                screen->sxy.vy = scratch->unk54.vy;
                screen->fog = scratch->unk54.vz;
                screen->z = scratch->unk54.vz >> 2;
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
                gte_stlvnl(view);
                screen->sxy.vx = scratch->unk54.vx;
                screen->sxy.vy = scratch->unk54.vy;
                screen->fog = scratch->unk54.vz;
                screen->z = scratch->unk54.vz >> 2;
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
        DAT_80199154.sum[0].mode = which;
        DAT_80199154.sum[0].count++;
        DAT_80199154.sum[0].r += (r * scale) >> 12;
        DAT_80199154.sum[0].g += (g * scale) >> 12;
        DAT_80199154.sum[0].b += (b * scale) >> 12;
        break;
    case 2:
        DAT_80199154.sum[1].mode = which;
        DAT_80199154.sum[1].count++;
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
        DAT_80199154.sum[0].mode = which;
        DAT_80199154.sum[0].count++;
        DAT_80199154.sum[0].r = (r * scale) >> 12;
        DAT_80199154.sum[0].g = (g * scale) >> 12;
        DAT_80199154.sum[0].b = (b * scale) >> 12;
        break;
    case 2:
        DAT_80199154.sum[1].mode = which;
        DAT_80199154.sum[1].count++;
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
 * Not exact only in the read of DAT_8009c2e0: retail reads it $gp-relative,
 * which ASPSX emits only for .sdata defined in the same file (hud.c).
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
    *(u32 *)&SCRATCH->rgb = *(u32 *)&DAT_8009c2e0;
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
                   AnimSlot **owner, MATRIX *view, u16 anim, u16 time, u8 light2, u16 t,
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

    mat = &m;
    scratch = SCRATCH;
    SCRATCH->draw.count.unk2 = 100;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->unk48 = DAT_80199154.screen;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->next = DAT_80199154.pool->next;
    SCRATCH->end = DAT_80199154.pool->end;
    *(u32 *)&SCRATCH->rgb = *(u32 *)&DAT_8009c2e0;
    if (SCRATCH->end - 30720 < SCRATCH->next) {
        flags &= 0xbf;
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
        layer = &DAT_801d4464.cells[pos->vz >> 11][pos->vx >> 11].layer[0];
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
    if (flags & 0x80) {
        flags &= 0x7f;
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
            func_80035358(func_80017158(light->unk68, other->unk68, (s16)t),
                          func_80017158(light->unk6a, other->unk6a, (s16)t));
        } else {
            func_80035358(light->unk68, light->unk6a);
        }
        if (other->unk64.r != 0xff) {
            gte_SetBackColor(func_80017158(light->unk64.r, other->unk64.r, (s16)t),
                             func_80017158(light->unk64.g, other->unk64.g, (s16)t),
                             func_80017158(light->unk64.b, other->unk64.b, (s16)t));
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
    n = 0;
    if (anim < 128) {
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
    if (flags & 0x40) {
        if (view != NULL) {
            func_800366a8(n);
            goto done;
        }
        {
            u8 *vertex = scratch->vertices;
            ScreenVertex *screen = scratch->screen;
            s32 i = part->unk4;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->unk54);
                screen->sxy.vx = scratch->unk54.vx;
                screen->sxy.vy = scratch->unk54.vy;
                screen->z = depth;
                screen->fog = scratch->unk54.vz;
                screen++;
                vertex += 8;
            }
        }
        func_80035ca4(n, depth);
    } else if (flags & 4) {
        if (view != NULL) {
            u8 *vertex = scratch->vertices;
            ScreenVertex *screen = scratch->screen;
            s32 i = part->unk4;

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
            u8 *vertex = scratch->vertices;
            ScreenVertex *screen = scratch->screen;
            s32 i = part->unk4;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->unk54);
                screen->sxy.vx = scratch->unk54.vx;
                screen->sxy.vy = scratch->unk54.vy;
                screen->z = depth;
                screen->fog = scratch->unk54.vz;
                screen++;
                vertex += 8;
            }
        }
        func_80037bec(n, depth, flags & 3);
    } else {
        if (view != NULL) {
            u8 *vertex = scratch->vertices;
            ScreenVertex *screen = scratch->screen;
            s32 i = part->unk4;

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
            u8 *vertex = scratch->vertices;
            ScreenVertex *screen = scratch->screen;
            s32 i = part->unk4;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->unk54);
                screen->sxy.vx = scratch->unk54.vx;
                screen->sxy.vy = scratch->unk54.vy;
                screen->z = depth;
                screen->fog = scratch->unk54.vz;
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
                   AnimSlot **owner, MATRIX *view, u16 anim, u16 time, u8 light2, u16 t,
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
    SCRATCH->draw.count.unk2 = 100;
    SCRATCH->screen = DAT_80199154.screen;
    SCRATCH->unk48 = DAT_80199154.screen;
    SCRATCH->draw.count.unk4 = DAT_80199154.unk159bc;
    SCRATCH->draw.count.ot_count = DAT_80199154.ot_count;
    SCRATCH->draw.count.prim_count = DAT_80199154.prim_count;
    SCRATCH->next = DAT_80199154.pool->next;
    *(u32 *)&SCRATCH->rgb = *(u32 *)&DAT_8009c2e0;
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
        rel.vz = delta.vz >>= 4;
        rel.vx = delta.vx;
        rel.vy = delta.vy;
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
            func_80035358(func_80017158(light->unk68, other->unk68, (s16)t),
                          func_80017158(light->unk6a, other->unk6a, (s16)t));
        } else {
            func_80035358(light->unk68, light->unk6a);
        }
        if (other->unk64.r != 0xff) {
            gte_SetBackColor(func_80017158(light->unk64.r, other->unk64.r, (s16)t),
                             func_80017158(light->unk64.g, other->unk64.g, (s16)t),
                             func_80017158(light->unk64.b, other->unk64.b, (s16)t));
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
    n = 0;
    if (anim < 128) {
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
        u8 *vertex = scratch->vertices;
        ScreenVertex *screen = scratch->screen;
        s32 i = part->unk4;

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
        u8 *vertex = scratch->vertices;
        ScreenVertex *screen = scratch->screen;
        s32 i = part->unk4;

        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rt();
            gte_stlvnl(&scratch->unk54);
            screen->sxy.vx = scratch->unk54.vx;
            screen->sxy.vy = scratch->unk54.vy;
            screen->z = depth;
            screen->fog = scratch->unk54.vz;
            screen++;
            vertex += 8;
        }
    }
    func_80038844(n, depth, shift, dist);
    DAT_80199154.pool->next = scratch->next;
    DAT_80199154.unk159bc = scratch->draw.count.unk4;
    DAT_80199154.ot_count = scratch->draw.count.ot_count;
}
