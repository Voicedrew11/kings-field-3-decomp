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

RODATA(0x80011bd0, 0xc)

DATA(0x8009c2d0, 0x8, ".sdata")
RECT DAT_8009c2d0 = { 320, 0, 320, 240 };

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
    DAT_80199154.pool0.base = DAT_80199154.heap + 0x7400;
    DAT_80199154.pool0.next = DAT_80199154.pool0.base;
    DAT_80199154.image = DAT_80199154.pool0.base + 0x7400;
    StoreImage(&DAT_8009c2d0, (u_long *)DAT_80199154.image);
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
    LoadImage(&DAT_8009c2d0, (u_long *)DAT_80199154.image);
    DAT_80199154.pool0.base = DAT_80199154.heap + 0x1a000;
    DAT_80199154.pool0.next = DAT_80199154.pool0.base;
    DAT_80199154.image = DAT_80199154.pool0.base + 0x1a000;
    DrawSync(0);
}
