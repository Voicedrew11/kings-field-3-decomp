/*
 * GAME.EXE: band B's render module from 0x8003bfd0 (WIP name): the map
 * layers of the view grid, the 3D overlay and model drawing front ends,
 * full-screen 2D effects and the loading screen, object model files, the map
 * cell grid and the per-frame object drawer, the 2D overlay sprites and
 * messages, the animated model poses and the picture fade. One TU:
 * render_map_cell_window, render_active_model_rows, render_player_weapon and
 * render_animated_object read map_textured_primitive_color and menu_show_transition_image
 * reads menu_transition_rect $gp-relative, which ASPSX emits only for .sdata defined
 * in the same file.
 */
/* render_scene_and_update_resources reads ResourceState.step as volatile (see game/data.h). */
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
RECT menu_transition_rect = { 320, 0, 320, 240 };

DATA(0x8009c2d8, 0x8, ".sdata")
RECT menu_transition_image_rect = { 960, 256, 64, 240 };

/* A delta run marker: the next word is a count instead of a vertex. */
#define DELTA_RUN (-32768)

/* Brightness the 3D overlays share with the status sprites (.sdata). */

DATA(0x8009c2e0, 0x4, ".sdata")
CVECTOR map_textured_primitive_color = { 0x80, 0x80, 0x80, 0 };

DATA(0x8009c2e4, 0x8, ".sdata")
SVECTOR DAT_8009c2e4 = { 0x1000, 0x1000, 0x1000, 0 };

DATA(0x8009c2ec, 0x4, ".sdata")
s32 render_model_yaw_smoothing_accumulator = 0;

/*
 * Draws the map layers of the 25 x 25 cell grid around the viewer
 * (GraphicsRuntime.grid, flags per cell as render_map_cell_layers takes them).
 */
ADDRESS(0x8003bfd0, 0x38c)
void render_map_cell_window(void)
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

    pos = &SCRATCH->u.layer.pos;
    origin = &SCRATCH->u.layer.origin;
    SCRATCH->model = scene_display_sys.tmd_slots[0];
    SCRATCH->ot = scene_display_sys.ot + 240;
    *(u32 *)&SCRATCH->w.map.rgb = *(u32 *)&map_textured_primitive_color;
    SCRATCH->next = scene_display_sys.pool->next;
    SCRATCH->end = scene_display_sys.pool->end;
    SCRATCH->w.map.clip = 100;
    SCRATCH->screen = scene_display_sys.screen;
    SCRATCH->u.layer.origin.vx = scene_display_sys.view_position.vx;
    SCRATCH->u.layer.origin.vy = scene_display_sys.view_position.vy;
    SCRATCH->u.layer.origin.vz = scene_display_sys.view_position.vz;
    SCRATCH->u.layer.origin.pad = scene_display_sys.view_position.pad;
    SCRATCH->w.map.tmd_prim_count = scene_display_sys.tmd_prim_count;
    SCRATCH->w.map.ot_count = scene_display_sys.ot_count;
    SCRATCH->w.map.prim_count = scene_display_sys.prim_count;
    resource_copy_words((u32 *)SCRATCH->u.view.grid, (u32 *)scene_display_sys.grid, 0x9c);
    grid = &SCRATCH->u.view.grid[0][0];
    SCRATCH->u.view.x = scene_display_sys.map_scan_start_x;
    SCRATCH->u.view.z = scene_display_sys.map_scan_start_z;
    z = scene_display_sys.map_scan_start_z;
    for (i = 25; i != 0; z++, i--) {
        if (z >= 0 && z < 80) {
            x = SCRATCH->u.view.x;
            for (j = 25; j != 0; x++, j--, grid++) {
                if (x >= 0 && x < 80 && *grid != 0) {
                    cell = &map_world.cells[z][x];
                    kind = cell->layer[0].object_index;
                    if ((*grid & 2) && kind < 240) {
                        pos->vx = (x << 11) - origin->vx + 1024;
                        pos->vy = (-cell->layer[0].elevation << 7) - origin->vy;
                        pos->vz = (z << 11) - origin->vz + 1024;
                        render_map_cell_object(&cell->layer[0], pos, *grid);
                        kind = cell->layer[1].object_index;
                        if ((*grid & 2) && kind < 240) {
                            pos->vy = (-cell->layer[1].elevation << 7) - origin->vy;
                            render_map_cell_object(&cell->layer[1], pos, *grid);
                        }
                    } else {
                        kind = cell->layer[1].object_index;
                        if ((*grid & 2) && kind < 240) {
                            pos->vx = (x << 11) - origin->vx + 1024;
                            pos->vy = (-cell->layer[1].elevation << 7) - origin->vy;
                            pos->vz = (z << 11) - origin->vz + 1024;
                            render_map_cell_object(&cell->layer[1], pos, *grid);
                        }
                    }
                }
            }
        } else {
            grid += 25;
        }
    }
    scene_display_sys.tmd_prim_count = SCRATCH->w.map.tmd_prim_count;
    scene_display_sys.ot_count = SCRATCH->w.map.ot_count;
    scene_display_sys.pool->next = SCRATCH->next;
}

/*
 * Draws the 3D overlays (the compass needles of render_model_rows, visible == 1)
 * in view space: each lit by its light record turned by its rotation, scaled,
 * posed by animation_prepare_asset_vertices (the base pose is rebuilt when it returns nonzero,
 * unlike render_animated_object), and projected flat (screen x/y = view x/y, depth =
 * view z / 4) before render_enqueue_textured_tmd.
 */

ADDRESS(0x8003c35c, 0x50c)
void render_active_model_rows(void)
{
    MATRIX m;
    VECTOR scale;
    MATRIX lm;
    RenderModelRow *rec;
    ScratchPad *scratch;
    LightRecord *light;
    u8 *file;
    u8 *model;
    s32 i;
    s32 offset;

    scratch = SCRATCH;
    for (rec = render_model_rows; rec->visible != 0xff; rec++) {
        if (rec->visible != 1) {
            continue;
        }
        light = &scene_display_sys.lights[rec->lighting_index];
        m.t[0] = rec->pos.vx;
        m.t[1] = rec->pos.vy;
        m.t[2] = rec->pos.vz;
        RotMatrix(&rec->angle, &m);
        fog_set_range(light->fog_near, light->fog_far);
        gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
        gte_SetColorMatrix(&light->color_matrix);
        gte_MulMatrix0(&light->light_matrices[0], &m, &lm);
        gte_SetLightMatrix(&lm);
        scale.vx = rec->scale.vx;
        scale.vy = rec->scale.vy;
        scale.vz = rec->scale.vz;
        ScaleMatrix(&m, &scale);
        gte_SetRotMatrix(&m);
        gte_SetTransMatrix(&m);
        file = scene_display_sys.models[rec->model];
        model = file + ((ModelFile *)file)->tmd;
        scratch->ot = scene_display_sys.ot;
        *(u32 *)&scratch->w.model.rgb = *(u32 *)&map_textured_primitive_color;
        scratch->draw.count.tmd_prim_count = scene_display_sys.tmd_prim_count;
        scratch->draw.count.ot_count = scene_display_sys.ot_count;
        scratch->draw.count.prim_count = scene_display_sys.prim_count;
        scratch->model = model;
        scratch->next = scene_display_sys.pool->next;
        scratch->end = scene_display_sys.pool->end;
        scratch->screen = scene_display_sys.screen;
        scratch->draw.count.clut = 0;
        if (animation_prepare_asset_vertices(&rec->animation_cache, rec->model, rec->animation_id,
                                             rec->animation_phase,
                                             ((ModelData *)model)->parts[0].vertex_count) != 0) {
            u8 *vertex;
            ScreenVertex *screen;

            scratch->part = ((ModelData *)scratch->model)->parts;
            offset = scratch->part->vertices + 12;
            vertex = scratch->vertices = (u8 *)(offset + (u32)scratch->model);
            screen = scratch->screen;
            i = ((ModelData *)model)->parts[0].vertex_count;
            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.transformed);
                screen->sxy.vx = scratch->w.model.transformed.vx;
                screen->sxy.vy = scratch->w.model.transformed.vy;
                screen->fog = scratch->w.model.transformed.vz;
                screen->z = scratch->w.model.transformed.vz >> 2;
                screen++;
                vertex += 8;
            }
        } else {
            u8 *vertex;
            ScreenVertex *screen;

            i = ((ModelData *)model)->parts[0].vertex_count;
            screen = scratch->screen;
            vertex = scratch->vertices;
            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.transformed);
                screen->sxy.vx = scratch->w.model.transformed.vx;
                screen->sxy.vy = scratch->w.model.transformed.vy;
                screen->fog = scratch->w.model.transformed.vz;
                screen->z = scratch->w.model.transformed.vz >> 2;
                screen++;
                vertex += 8;
            }
        }
        render_enqueue_textured_tmd(0, 0);
        scene_display_sys.pool->next = scratch->next;
        scene_display_sys.tmd_prim_count = scratch->draw.count.tmd_prim_count;
        scene_display_sys.ot_count = scratch->draw.count.ot_count;
    }
}

ADDRESS(0x8003c868, 0x100)
void render_textured_quad_immediate(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                                    u16 tpage, u16 clut, u8 r, u8 g, u8 b)
{
    POLY_FT4 *p;

    p = (POLY_FT4 *)(scene_display_sys.buffer + scene_display_sys.frame * 160);
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
    DrawPrim((void *)p);
}

ADDRESS(0x8003c968, 0x74)
void loading_screen_draw_quad(u8 level, u8 inset)
{
    render_textured_quad_immediate(inset + 60, inset + 20, 260 - inset, 220 - inset, 0, 0, 200, 200, 0xff,
                                   0x1f, 0x723c, level, level, level);
}

ADDRESS(0x8003c9dc, 0x1ec)
void loading_screen_show(void)
{
    char name[64];
    CdlFILE file;
    u_char result[8];
    s32 i;
    s32 sync;

    strcpy(name, "\\LOAD.MSG;1");
    if (CdSearchFile(&file, name) == 0) {
        PutDispEnv(&scene_display_sys.disp[1]);
        PutDrawEnv(&scene_display_sys.draw[1]);
        SetDispMask(1);
        return;
    }
    file.size = (file.size + 2047) & ~2047;
    for (i = 0; i < 50; i++) {
        CdControl(CdlSetloc, (u_char *)&file.pos, 0);
        CdRead(file.size >> 11, (u_long *)scene_display_sys.buffer, CdlModeSpeed);
        while ((sync = CdReadSync(1, result)) > 0) {
        }
        if (sync == 0) {
            i = 100;
        }
    }
    tim_upload_images(scene_display_sys.buffer);
    render_textured_quad_immediate(100, 30, 220, 150, 0, 0, 120, 120, 0xff, 0x1f, 0x723c, 0x58, 0x58, 0x58);
    render_textured_quad_immediate(96, 160, 224, 184, 0, 120, 128, 24, 0xff, 0x1f, 0x727c, 0x58, 0x58, 0x58);
    DrawSync(0);
    VSync(0);
    PutDispEnv(&scene_display_sys.disp[1]);
    PutDrawEnv(&scene_display_sys.draw[1]);
    SetDispMask(1);
    scene_display_sys.fade_level = 0;
    scene_display_sys.fade_hold = 0;
}

ADDRESS(0x8003cbc8, 0x134)
void loading_screen_update_fade(void)
{
    u8 level;

    if (scene_display_sys.fade_hold != 0) {
        if (scene_display_sys.fade_level == 0) {
            scene_display_sys.fade_level = 0xff;
            scene_display_sys.fade_hold--;
        } else {
            scene_display_sys.fade_level--;
        }
    } else if (scene_display_sys.fade_level != 0) {
        level = scene_display_sys.fade_level >> 1;
        scene_display_sys.frame = scene_display_sys.frame == 0;
        PutDispEnv(&scene_display_sys.disp[scene_display_sys.frame]);
        PutDrawEnv(&scene_display_sys.draw[scene_display_sys.frame]);
        loading_screen_draw_quad(level, level >> 1);
        loading_screen_draw_quad(level >> 1, level >> 2);
        DrawSync(0);
        VSync(0);
        scene_display_sys.fade_level--;
    }
}

ADDRESS(0x8003ccfc, 0x1c8)
void render_textured_quad(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                          u16 tpage, u16 clut, u8 r, u8 g, u8 b, s32 z)
{
    POLY_FT4 *p;

    p = (POLY_FT4 *)scene_display_sys.pool->next;
    scene_display_sys.pool->next += sizeof(POLY_FT4);
    if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
        return;
    }
    scene_display_sys.prim_count++;
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
void render_gouraud_quad(int x0, int y0, int x1, int y1, u8 semi, u32 *rgb0, u32 *rgb1, u32 *rgb2,
                         u32 *rgb3, s32 z)
{
    POLY_G4 *p;

    p = (POLY_G4 *)scene_display_sys.pool->next;
    scene_display_sys.pool->next += sizeof(POLY_G4);
    if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
        return;
    }
    scene_display_sys.prim_count++;
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
void render_sprite(int x, int y, u8 u, u8 v, u8 w, u8 h, u8 semi, u16 tpage, u16 clut, u8 r,
                   u8 g, u8 b, s32 z)
{
    DR_MODE *mode;
    SPRT *p;

    mode = (DR_MODE *)scene_display_sys.pool->next;
    scene_display_sys.pool->next += sizeof(DR_MODE);
    if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
        return;
    }
    scene_display_sys.prim_count++;
    p = (SPRT *)scene_display_sys.pool->next;
    scene_display_sys.pool->next += sizeof(SPRT);
    if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
        return;
    }
    scene_display_sys.prim_count++;
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
void render_sliding_panel_primary(void)
{
    s32 y;

    if (cur_avatar_status.collision_lower_clearance < 240) {
        y = cur_avatar_status.collision_lower_clearance + 120;
        if (y < 0) {
            y = 0;
        }
        render_textured_quad(0, 0, 320, 240, 0, 0xa0, 15, 15, 0xff, 0x6c, 0x7a8d, 0x3c, 0x3c, 0x80, 0x1fff);
        render_textured_quad(0, y, 320, 240, 0, 0xb0, 15, 15, 1, 0x2c, 0x7a8d, 0x3c, 0x3c, 0x80, 0x40);
    }
}

ADDRESS(0x8003d38c, 0x90)
void render_sliding_panel_secondary(void)
{
    s32 y;

    if (cur_avatar_status.collision_upper_clearance < 240) {
        y = cur_avatar_status.collision_upper_clearance + 120;
        if (y < 0) {
            y = 0;
        }
        render_textured_quad(0, y, 320, 240, 0x10, 0xa0, 15, 15, 1, 0x2c, 0x7a8d, 0x3c, 0x3c, 0x3c, 0x40);
    }
}

ADDRESS(0x8003d41c, 0x90)
void render_sliding_panel_tertiary(void)
{
    s32 y;

    if (cur_avatar_status.collision_second_upper_clearance < 240) {
        y = cur_avatar_status.collision_second_upper_clearance + 120;
        if (y < 0) {
            y = 0;
        }
        render_textured_quad(0, y, 320, 240, 0x10, 0xb0, 15, 15, 1, 0x2c, 0x7a8d, 0x3c, 0x3c, 0x3c, 0x40);
    }
}

/* Same body as render_color_overlay; retail keeps both. */

ADDRESS(0x8003d4ac, 0xbc)
void func_8003d4ac(void)
{
    if (scene_display_sys.tint.mode != 0xff) {
        render_textured_quad(0, 0, 320, 240, 0, 0xa0, 15, 15, 1,
                             ((scene_display_sys.tint.mode & 3) << 5) | 12, 0x7a8d, scene_display_sys.tint.r,
                             scene_display_sys.tint.g, scene_display_sys.tint.b,
                             (scene_display_sys.tint.mode & 0x80) ? 1 : 0x40);
    }
}

ADDRESS(0x8003d568, 0xbc)
void render_color_overlay(void)
{
    if (scene_display_sys.tint.mode != 0xff) {
        render_textured_quad(0, 0, 320, 240, 0, 0xa0, 15, 15, 1,
                             ((scene_display_sys.tint.mode & 3) << 5) | 12, 0x7a8d, scene_display_sys.tint.r,
                             scene_display_sys.tint.g, scene_display_sys.tint.b,
                             (scene_display_sys.tint.mode & 0x80) ? 1 : 0x40);
    }
}

ADDRESS(0x8003d624, 0x28)
void render_set_color_overlay(u8 mode, u8 r, u8 g, u8 b)
{
    scene_display_sys.tint.mode = mode;
    scene_display_sys.tint.r = r;
    scene_display_sys.tint.g = g;
    scene_display_sys.tint.b = b;
}

ADDRESS(0x8003d64c, 0x150)
void render_accumulated_color_overlay_additive(void)
{
    if (scene_display_sys.sum[0].count != 0) {
        render_textured_quad(0, 0, 320, 240, 0, 0xa0, 15, 15, 1,
                             ((scene_display_sys.sum[0].mode & 3) << 5) | 12, 0x7a8d,
                             scene_display_sys.sum[0].r / scene_display_sys.sum[0].count,
                             scene_display_sys.sum[0].g / scene_display_sys.sum[0].count,
                             scene_display_sys.sum[0].b / scene_display_sys.sum[0].count, 0x30);
    }
}

ADDRESS(0x8003d79c, 0x150)
void render_accumulated_color_overlay_subtractive(void)
{
    if (scene_display_sys.sum[1].count != 0) {
        render_textured_quad(0, 0, 320, 240, 0, 0xa0, 15, 15, 1,
                             ((scene_display_sys.sum[1].mode & 3) << 5) | 12, 0x7a8d,
                             scene_display_sys.sum[1].r / scene_display_sys.sum[1].count,
                             scene_display_sys.sum[1].g / scene_display_sys.sum[1].count,
                             scene_display_sys.sum[1].b / scene_display_sys.sum[1].count, 0x30);
    }
}

ADDRESS(0x8003d8ec, 0x124)
void accumulate_color_overlay(s32 r, s32 g, s32 b, s32 scale, s32 which)
{
    switch (which) {
    case 1:
        scene_display_sys.sum[0].count++;
        scene_display_sys.sum[0].mode = which;
        scene_display_sys.sum[0].r += (r * scale) >> 12;
        scene_display_sys.sum[0].g += (g * scale) >> 12;
        scene_display_sys.sum[0].b += (b * scale) >> 12;
        break;
    case 2:
        scene_display_sys.sum[1].count++;
        scene_display_sys.sum[1].mode = which;
        scene_display_sys.sum[1].r += (r * scale) >> 12;
        scene_display_sys.sum[1].g += (g * scale) >> 12;
        scene_display_sys.sum[1].b += (b * scale) >> 12;
        break;
    }
}

ADDRESS(0x8003da10, 0xdc)
void set_accumulated_color_overlay(s32 r, s32 g, s32 b, s32 scale, s32 which)
{
    switch (which) {
    case 1:
        scene_display_sys.sum[0].count++;
        scene_display_sys.sum[0].mode = which;
        scene_display_sys.sum[0].r = (r * scale) >> 12;
        scene_display_sys.sum[0].g = (g * scale) >> 12;
        scene_display_sys.sum[0].b = (b * scale) >> 12;
        break;
    case 2:
        scene_display_sys.sum[1].count++;
        scene_display_sys.sum[1].mode = which;
        scene_display_sys.sum[1].r = (r * scale) >> 12;
        scene_display_sys.sum[1].g = (g * scale) >> 12;
        scene_display_sys.sum[1].b = (b * scale) >> 12;
        break;
    }
}

ADDRESS(0x8003daec, 0x464)
void render_zoom_screen_transition(u8 *rgb, s32 x, s32 y, s32 step)
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
    scene_display_sys.pools[0].end = scene_display_sys.pools[0].base + 0x7400;
    scene_display_sys.pools[1].base = scene_display_sys.pools[0].end;
    scene_display_sys.pools[1].end = scene_display_sys.pools[0].end + 0x7400;
    StoreImage(&menu_transition_rect, (u_long *)scene_display_sys.pools[1].end);
    DrawSync(0);
    MoveImage(&scene_display_sys.draw[scene_display_sys.frame].clip, menu_transition_rect.x,
              menu_transition_rect.y);
    DrawSync(0);
    scene_display_sys.draw[0].isbg = 0;
    scene_display_sys.draw[1].isbg = 0;
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
        display_begin_frame();
        p = (POLY_FT4 *)scene_display_sys.pool->next;
        scene_display_sys.pool->next += sizeof(POLY_FT4);
        if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
            return;
        }
        scene_display_sys.prim_count++;
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
        addPrim(scene_display_sys.ot, p);
        p = (POLY_FT4 *)scene_display_sys.pool->next;
        scene_display_sys.pool->next += sizeof(POLY_FT4);
        if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
            return;
        }
        scene_display_sys.prim_count++;
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
        addPrim(scene_display_sys.ot, p);
        count--;
        DrawSync(0);
        display_present_frame();
        x += step * 4;
        y += step * 3;
    }
    scene_display_sys.draw[0].isbg = 1;
    scene_display_sys.draw[1].isbg = 1;
    LoadImage(&menu_transition_rect, (u_long *)scene_display_sys.pools[1].end);
    scene_display_sys.pools[0].end = scene_display_sys.pools[0].base + 0x1a000;
    scene_display_sys.pools[1].base = scene_display_sys.pools[0].end;
    scene_display_sys.pools[1].end = scene_display_sys.pools[0].end + 0x1a000;
    DrawSync(0);
}

/*
 * Draws the weapon model (resident model 32) while the player attacks
 * (weapon_attack_phase is the attack time, -1 for none): lit by the light record of the map
 * layer under the player, posed by animation_prepare_asset_vertices and placed by the weapon
 * record's offset and angle; drawn at depth offset 24.
 */

ADDRESS(0x8003df50, 0x3fc)
void render_player_weapon(void)
{
    MATRIX m;
    ScratchPad *scratch;
    LightRecord *light;
    u8 *file;
    u8 *model;
    u8 *vertex;
    ScreenVertex *screen;
    s32 i;

    if (cur_avatar_status.weapon_attack_phase == -1) {
        return;
    }
    scratch = SCRATCH;
    light = &scene_display_sys.lights[((MapLayer *)((u8 *)&map_world.cells[cur_avatar_status.pos.vz >> 11]
                                                         [cur_avatar_status.pos.vx >> 11] +
                                                cur_avatar_status.map_layer_offset))->lighting_index & 0x3f];
    gte_SetColorMatrix(&light->color_matrix);
    gte_SetLightMatrix(&light->light_matrices[0]);
    fog_set_range(light->fog_near, light->fog_far);
    gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
    m.t[0] = (s16)cur_avatar_status.equipped_weapon_record->position_offset_x;
    m.t[1] = (s16)cur_avatar_status.equipped_weapon_record->position_offset_y;
    m.t[2] = (s16)cur_avatar_status.equipped_weapon_record->position_offset_z;
    RotMatrix((SVECTOR *)&cur_avatar_status.equipped_weapon_record->rotation_offset_x, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    file = scene_display_sys.models[32];
    model = file + ((ModelFile *)file)->tmd;
    SCRATCH->model = model;
    if (animation_prepare_asset_vertices((AnimSlot **)&cur_avatar_status.weapon_animation_cache, 32,
                                         cur_avatar_status.weapon_attack_mode,
                                         cur_avatar_status.weapon_attack_phase,
                                         ((ModelData *)model)->parts[0].vertex_count) == 0) {
        return;
    }
    SCRATCH->ot = scene_display_sys.ot;
    SCRATCH->draw.count.tmd_prim_count = scene_display_sys.tmd_prim_count;
    SCRATCH->draw.count.ot_count = scene_display_sys.ot_count;
    SCRATCH->draw.count.prim_count = scene_display_sys.prim_count;
    SCRATCH->next = scene_display_sys.pool->next;
    SCRATCH->end = scene_display_sys.pool->end;
    SCRATCH->screen = scene_display_sys.screen;
    SCRATCH->draw.count.clut = 0;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&map_textured_primitive_color;
    vertex = SCRATCH->vertices;
    screen = scene_display_sys.screen;
    i = ((ModelData *)model)->parts[0].vertex_count;
    while (--i != -1) {
        gte_ldv0(vertex);
        gte_rtps();
        gte_stsxy(&screen->sxy);
        gte_stszotz(&screen->z);
        if (scene_display_sys.fog_near != scene_display_sys.fog_far) {
            screen->fog = ((screen->z - (scene_display_sys.fog_near >> 2)) << 14) /
                          (scene_display_sys.fog_far - scene_display_sys.fog_near);
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
    render_enqueue_textured_tmd(0, 24);
    scene_display_sys.pool->next = scratch->next;
    scene_display_sys.tmd_prim_count = scratch->draw.count.tmd_prim_count;
    scene_display_sys.ot_count = scratch->draw.count.ot_count;
}

/*
 * Draws model file id (a monster or object model) at pos turned by rotation:
 * lit by the light record of the map layer under it (layer_mask 1 or 2), blended
 * by lighting_blend (Q12) toward light record lighting_override_index (0xff for
 * none), scaled by scale and turned by view when given, posed by
 * animation_prepare_asset_vertices from animation_cache at animation_phase (Q12)
 * for animation_id < 128 (part animation_id & 0x7f otherwise). render_mode: 0x40
 * render_enqueue_tmd_subdivided / flat, 4 render_enqueue_blended_tmd (render_mode & 3),
 * else render_enqueue_textured_tmd; 0x80 keeps render_depth_offset for models below
 * the eye. With view == NULL the vertices are transformed flat (x/y kept, z =
 * render_depth_offset).
 */

ADDRESS(0x8003e34c, 0xfb8)
void render_world_model(u8 layer_mask, u16 id, VECTOR *pos, SVECTOR *rotation, SVECTOR *scale,
                        AnimSlot **animation_cache, MATRIX *view, u16 animation_id, u16 animation_phase,
                        u8 lighting_override_index, s16 lighting_blend,
                        s32 render_mode, s32 render_depth_offset)
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
    LightRecord *light;
    LightRecord *other;
    MapLayer *layer;
    ModelPart *part;
    u8 *file;
    u8 *model;
    s32 n;
    u8 mode;

    mat = &m;
    scratch = SCRATCH;
    SCRATCH->draw.count.clip = 100;
    SCRATCH->screen = scene_display_sys.screen;
    SCRATCH->screen_cursor = scene_display_sys.screen;
    SCRATCH->draw.count.tmd_prim_count = scene_display_sys.tmd_prim_count;
    SCRATCH->draw.count.ot_count = scene_display_sys.ot_count;
    SCRATCH->draw.count.prim_count = scene_display_sys.prim_count;
    SCRATCH->next = scene_display_sys.pool->next;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&map_textured_primitive_color;
    SCRATCH->end = scene_display_sys.pool->end;
    mode = render_mode;
    if (SCRATCH->end - 30720 < SCRATCH->next) {
        mode &= 0xbf;
    }
    gte_SetRotMatrix(&scene_display_sys.view_matrix);
    gte_SetTransMatrix(&scene_display_sys.view_matrix);
    if (view != NULL) {
        delta.vx = pos->vx - scene_display_sys.view_position.vx;
        delta.vy = pos->vy - scene_display_sys.view_position.vy;
        delta.vz = pos->vz - scene_display_sys.view_position.vz;
        rel.vx = pos->vx - scene_display_sys.view_position.vx;
        rel.vy = pos->vy - scene_display_sys.view_position.vy;
        rel.vz = pos->vz - scene_display_sys.view_position.vz;
        gte_ldv0(&rel);
        gte_rt();
        gte_stlvnl(mat->t);
        gte_stflg(&flag);
        {
            MapCell *row = map_world.cells[pos->vz >> 11];

            layer = row[pos->vx >> 11].layer;
        }
        if (layer_mask != 1) {
            layer++;
        }
        light = &scene_display_sys.lights[layer->lighting_index & 0x3f];
    } else {
        mat->t[0] = pos->vx;
        mat->t[1] = pos->vy;
        mat->t[2] = pos->vz;
        light = &scene_display_sys.lights
            [((MapLayer *)((u8 *)&map_world.cells[scene_display_sys.view_position.vz >> 11]
                                                      [scene_display_sys.view_position.vx >> 11] +
                           cur_avatar_status.map_layer_offset))->lighting_index & 0x3f];
    }
    if (mode & 0x80) {
        mode &= 0x7f;
    } else if (rel.vy <= 0) {
        render_depth_offset += 240;
    }
    matrix_set_rotation_yxz(rotation, mat);
    if (lighting_override_index != 0xff) {
        other = &scene_display_sys.lights[lighting_override_index];
        if (other->color_matrix.m[0][0] != -1) {
            fixed_lerp_nine_halfwords_q12((MATRIX *)&light->color_matrix, (MATRIX *)&other->color_matrix,
                                          &blend, lighting_blend);
            gte_SetColorMatrix(&blend);
        } else {
            gte_SetColorMatrix(&light->color_matrix);
        }
        if (other->light_matrices[0].m[0][0] != -1) {
            gte_MulMatrix0(&other->light_matrices[0], mat, &lm);
        } else {
            gte_MulMatrix0(&light->light_matrices[0], mat, &lm);
        }
        if (other->fog_near != 0xffff && other->fog_far != 0xffff) {
            fog_set_range(fixed_lerp_q12(light->fog_near, other->fog_near, lighting_blend),
                          fixed_lerp_q12(light->fog_far, other->fog_far, lighting_blend));
        } else {
            fog_set_range(light->fog_near, light->fog_far);
        }
        if (other->back_color.r != 0xff) {
            gte_SetBackColor(fixed_lerp_q12(light->back_color.r, other->back_color.r, lighting_blend),
                             fixed_lerp_q12(light->back_color.g, other->back_color.g, lighting_blend),
                             fixed_lerp_q12(light->back_color.b, other->back_color.b, lighting_blend));
        } else {
            gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
        }
    } else {
        gte_SetColorMatrix(&light->color_matrix);
        fog_set_range(light->fog_near, light->fog_far);
        gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
        gte_MulMatrix0(&light->light_matrices[0], mat, &lm);
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
    file = scene_display_sys.models[id];
    model = file + ((ModelFile *)file)->tmd;
    scratch->model = model;
    if (animation_id < 128) {
        n = 0;
        part = ((ModelData *)model)->parts;
        if (animation_prepare_asset_vertices(animation_cache, id, animation_id, animation_phase,
                                             ((ModelData *)model)->parts[0].vertex_count) != 0) {
            goto draw;
        }
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    } else {
        n = animation_id & 0x7f;
        part = &((ModelData *)scratch->model)->parts[n];
        scratch->part = &((ModelData *)model)->parts[n];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    }
draw:
    if (mode & 0x40) {
        if (view != NULL) {
            render_enqueue_tmd_subdivided(n);
            goto done;
        }
        {
            s32 i = part->vertex_count;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.transformed);
                screen->sxy.vx = scratch->w.model.transformed.vx;
                screen->sxy.vy = scratch->w.model.transformed.vy;
                screen->fog = scratch->w.model.transformed.vz;
                screen->z = render_depth_offset;
                screen++;
                vertex += 8;
            }
        }
        render_enqueue_textured_tmd(n, render_depth_offset);
    } else if (mode & 4) {
        if (view != NULL) {
            s32 i = part->vertex_count;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rtps();
                gte_stsxy(&screen->sxy);
                gte_stszotz(&screen->z);
                if (scene_display_sys.fog_near != scene_display_sys.fog_far) {
                    screen->fog = ((screen->z - (scene_display_sys.fog_near >> 2)) << 14) /
                                  (scene_display_sys.fog_far - scene_display_sys.fog_near);
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
            s32 i = part->vertex_count;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.transformed);
                screen->sxy.vx = scratch->w.model.transformed.vx;
                screen->sxy.vy = scratch->w.model.transformed.vy;
                screen->fog = scratch->w.model.transformed.vz;
                screen->z = render_depth_offset;
                screen++;
                vertex += 8;
            }
        }
        render_enqueue_blended_tmd(n, render_depth_offset, mode & 3);
    } else {
        if (view != NULL) {
            s32 i = part->vertex_count;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rtps();
                gte_stsxy(&screen->sxy);
                gte_stszotz(&screen->z);
                if (scene_display_sys.fog_near != scene_display_sys.fog_far) {
                    screen->fog = ((screen->z - (scene_display_sys.fog_near >> 2)) << 14) /
                                  (scene_display_sys.fog_far - scene_display_sys.fog_near);
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
            s32 i = part->vertex_count;
            ScreenVertex *screen = scratch->screen;
            u8 *vertex = scratch->vertices;

            while (--i != -1) {
                gte_ldv0(vertex);
                gte_rt();
                gte_stlvnl(&scratch->w.model.transformed);
                screen->sxy.vx = scratch->w.model.transformed.vx;
                screen->sxy.vy = scratch->w.model.transformed.vy;
                screen->fog = scratch->w.model.transformed.vz;
                screen->z = render_depth_offset;
                screen++;
                vertex += 8;
            }
        }
        render_enqueue_textured_tmd(n, render_depth_offset);
    }
done:
    scene_display_sys.pool->next = scratch->next;
    scene_display_sys.tmd_prim_count = scratch->draw.count.tmd_prim_count;
    scene_display_sys.ot_count = scratch->draw.count.ot_count;
}

/*
 * render_world_model's variant for large models (render_enqueue_distant_tmd): lit by light
 * record 0 (blended by lighting_blend toward lighting_override_index). With a view matrix the model is
 * skipped outside a 1536-unit wide angle around the player's heading, and the
 * translation is kept below 0x8000 by halving it shift times (the view matrix
 * rows are scaled down to match); render_enqueue_distant_tmd gets shift and the distance.
 * layer_mask is unused.
 */

ADDRESS(0x8003f304, 0xda8)
void render_distant_world_model(u8 layer_mask, u16 id, VECTOR *pos, SVECTOR *rotation, SVECTOR *scale,
                                AnimSlot **animation_cache, MATRIX *view, u16 animation_id,
                                u16 animation_phase, u8 lighting_override_index, s16 lighting_blend,
                                u8 render_mode, s32 render_depth_offset)
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
    LightRecord *light;
    LightRecord *other;
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
    SCRATCH->screen = scene_display_sys.screen;
    SCRATCH->screen_cursor = scene_display_sys.screen;
    SCRATCH->draw.count.tmd_prim_count = scene_display_sys.tmd_prim_count;
    SCRATCH->draw.count.ot_count = scene_display_sys.ot_count;
    SCRATCH->draw.count.prim_count = scene_display_sys.prim_count;
    SCRATCH->next = scene_display_sys.pool->next;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&map_textured_primitive_color;
    SCRATCH->end = scene_display_sys.pool->end;
    gte_SetRotMatrix(&scene_display_sys.view_matrix);
    gte_SetTransMatrix(&scene_display_sys.view_matrix);
    light = scene_display_sys.lights;
    if (view != NULL) {
        delta.vx = pos->vx - scene_display_sys.view_position.vx;
        delta.vy = pos->vy - scene_display_sys.view_position.vy;
        delta.vz = pos->vz - scene_display_sys.view_position.vz;
        turn = ratan2(delta.vz, delta.vx) - 1024;
        angle = (cur_avatar_status.camera_rotation.vy - turn) & 0xfff;
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
        light = scene_display_sys.lights;
    } else {
        m.t[0] = pos->vx;
        m.t[1] = pos->vy;
        m.t[2] = pos->vz;
    }
    if (!(render_mode & 0x80) && rel.vy <= 0) {
        render_depth_offset += 240;
    }
    matrix_set_rotation_yxz(rotation, mat);
    if (lighting_override_index != 0xff) {
        other = &scene_display_sys.lights[lighting_override_index];
        if (other->color_matrix.m[0][0] != -1) {
            fixed_lerp_nine_halfwords_q12((MATRIX *)&light->color_matrix, (MATRIX *)&other->color_matrix,
                                          &blend, lighting_blend);
            gte_SetColorMatrix(&blend);
        } else {
            gte_SetColorMatrix(&light->color_matrix);
        }
        if (other->light_matrices[0].m[0][0] != -1) {
            gte_MulMatrix0(&other->light_matrices[0], mat, &lm);
        } else {
            gte_MulMatrix0(&light->light_matrices[0], mat, &lm);
        }
        if (other->fog_near != 0xffff && other->fog_far != 0xffff) {
            fog_set_range(fixed_lerp_q12(light->fog_near, other->fog_near, lighting_blend),
                          fixed_lerp_q12(light->fog_far, other->fog_far, lighting_blend));
        } else {
            fog_set_range(light->fog_near, light->fog_far);
        }
        if (other->back_color.r != 0xff) {
            gte_SetBackColor(fixed_lerp_q12(light->back_color.r, other->back_color.r, lighting_blend),
                             fixed_lerp_q12(light->back_color.g, other->back_color.g, lighting_blend),
                             fixed_lerp_q12(light->back_color.b, other->back_color.b, lighting_blend));
        } else {
            gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
        }
    } else {
        gte_SetColorMatrix(&light->color_matrix);
        fog_set_range(light->fog_near, light->fog_far);
        gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
        gte_MulMatrix0(&light->light_matrices[0], mat, &lm);
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
    file = scene_display_sys.models[id];
    model = file + ((ModelFile *)file)->tmd;
    scratch->model = model;
    if (animation_id < 128) {
        n = 0;
        part = ((ModelData *)model)->parts;
        if (animation_prepare_asset_vertices(animation_cache, id, animation_id, animation_phase,
                                             ((ModelData *)model)->parts[0].vertex_count) != 0) {
            goto draw;
        }
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    } else {
        n = animation_id & 0x7f;
        part = &((ModelData *)scratch->model)->parts[n];
        scratch->part = &((ModelData *)model)->parts[n];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
    }
draw:
    dist = SquareRoot0(mat->t[0] * mat->t[0] + mat->t[1] * mat->t[1] + mat->t[2] * mat->t[2]);
    if (view != NULL) {
        s32 i = part->vertex_count;
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
        s32 i = part->vertex_count;
        ScreenVertex *screen = scratch->screen;
        u8 *vertex = scratch->vertices;

        while (--i != -1) {
            gte_ldv0(vertex);
            gte_rt();
            gte_stlvnl(&scratch->w.model.transformed);
            screen->sxy.vx = scratch->w.model.transformed.vx;
            screen->sxy.vy = scratch->w.model.transformed.vy;
            screen->fog = scratch->w.model.transformed.vz;
            screen->z = render_depth_offset;
            screen++;
            vertex += 8;
        }
    }
    render_enqueue_distant_tmd(n, render_depth_offset, shift, dist);
    scene_display_sys.pool->next = scratch->next;
    scene_display_sys.tmd_prim_count = scratch->draw.count.tmd_prim_count;
    scene_display_sys.ot_count = scratch->draw.count.ot_count;
}

/*
 * Draws model file id (an object's model) turned by rotation and placed 100
 * units down in view space: lit by light record light_id, posed by
 * animation_prepare_asset_vertices (the base pose when no slot is free) and drawn by
 * render_enqueue_tmd_fixed_depth at depth offset depth.
 * depth arrives as int (the calls in render_scene_and_update_resources) and is narrowed to u16.
 */

ADDRESS(0x800400ac, 0x4bc)
void render_animated_object(u16 id, SVECTOR *rotation, AnimSlot **owner, u16 anim, u16 time, s32 shade,
                            s32 light_id, int depth_in)
{
    u16 depth = depth_in;
    MATRIX m;
    ScratchPad *scratch;
    LightRecord *light;
    u8 *file;
    u8 *model;
    s32 i;
    s32 offset;

    m.t[0] = 0;
    m.t[1] = 100;
    m.t[2] = 0;
    matrix_set_rotation_yxz(rotation, &m);
    scratch = SCRATCH;
    gte_MulMatrix0(&scene_display_sys.view_matrix, &m, &m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    light = &scene_display_sys.lights[light_id & 0x7f];
    gte_SetColorMatrix(&light->color_matrix);
    gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
    gte_SetLightMatrix(&light->light_matrices[0]);
    file = scene_display_sys.models[id];
    model = file + ((ModelFile *)file)->tmd;
    SCRATCH->ot = scene_display_sys.ot;
    SCRATCH->draw.count.tmd_prim_count = scene_display_sys.tmd_prim_count;
    SCRATCH->draw.count.ot_count = scene_display_sys.ot_count;
    SCRATCH->draw.count.prim_count = scene_display_sys.prim_count;
    SCRATCH->model = model;
    SCRATCH->next = scene_display_sys.pool->next;
    SCRATCH->end = scene_display_sys.pool->end;
    SCRATCH->screen = scene_display_sys.screen;
    SCRATCH->draw.count.clut = 0;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&map_textured_primitive_color;
    if (animation_prepare_asset_vertices(owner, id, anim, time,
                                         ((ModelData *)model)->parts[0].vertex_count) == 0) {
        u8 *vertex;
        ScreenVertex *screen;

        SCRATCH->part = ((ModelData *)SCRATCH->model)->parts;
        offset = SCRATCH->part->vertices + 12;
        vertex = SCRATCH->vertices = (u8 *)(offset + (u32)SCRATCH->model);
        screen = SCRATCH->screen;
        i = ((ModelData *)model)->parts[0].vertex_count;
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

        i = ((ModelData *)model)->parts[0].vertex_count;
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
    render_enqueue_tmd_fixed_depth(0, depth, shade);
    scene_display_sys.pool->next = scratch->next;
    scene_display_sys.tmd_prim_count = scratch->draw.count.tmd_prim_count;
    scene_display_sys.ot_count = scratch->draw.count.ot_count;
}

ADDRESS(0x80040568, 0x80)
u16 map_object_asset_index(u16 id)
{
    if (id < 300) {
        return id + 256;
    }
    if ((s16)asset_status.loading == 1 && asset_status.area_records_loaded == 1
        && asset_status.requested_resource_ids[0] != 0xff) {
        return id + (asset_status.requested_resource_ids[0] * 32 + 256);
    }
    return id + (asset_status.active_resource_ids[0] * 32 + 256);
}

ADDRESS(0x800405e8, 0x70)
void resource_registry_get(u16 id, u8 **out)
{
    u8 *model;
    u8 state;

    model = scene_display_sys.models[id];
    *out = model;
    if (id < 104) {
        return;
    }
    if (model != NULL) {
        state = memory_block_kind(model);
        if (state == 1 || state == 2) {
            return;
        }
    }
    *out = NULL;
}

/* Load-complete callback for a streamed model file. */

ADDRESS(0x80040658, 0x3c)
void resource_tmd_read_complete(u8 *file)
{
    tmd_prepare_primitive_indices(file + *(s32 *)(file + 8));
    memory_block_set_kind(file, 2);
}

ADDRESS(0x80040694, 0x74)
u8 map_cell_layer_mask(VECTOR *pos)
{
    s32 row;
    s32 col;

    row = (pos->vz >> 11) + scene_display_sys.grid_z;
    if (row >= 0 && row < 25) {
        col = (pos->vx >> 11) + scene_display_sys.grid_x;
        if (col >= 0 && col < 25) {
            return scene_display_sys.grid[row][col];
        }
        return 0;
    }
    return 0;
}

ADDRESS(0x80040708, 0xc4)
u8 map_cell_layer_mask_radius(VECTOR *pos, s32 radius)
{
    u8 cells = 0;
    s32 row = (pos->vz >> 11) + scene_display_sys.grid_z - radius;
    s32 col0 = (pos->vx >> 11) + scene_display_sys.grid_x - radius;
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
                    cells |= scene_display_sys.grid[row][col];
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
s32 map_cell_visible(VECTOR *pos, s32 range_x, s32 range_z)
{
    s32 z = pos->vz >> 11;
    s32 x;

    if (z - range_z <= scene_display_sys.view_cell_z && scene_display_sys.view_cell_z <= z + range_z) {
        x = pos->vx >> 11;
        if (x - range_x <= scene_display_sys.view_cell_x && scene_display_sys.view_cell_x <= x + range_x) {
            return 1;
        }
    }
    return 0;
}

ADDRESS(0x80040830, 0xa0)
void resource_tmd_queue_read(s32 file, s32 entry, s32 id)
{
    u8 *model;

    model = memory_arena_allocate_block((HeapBlock *)resource_space, cd_archive_entry_size(file, entry),
                                        &scene_display_sys.models[id]);
    if (model != NULL) {
        memory_block_set_kind(model, 3);
        memory_block_set_tag(model, id);
        cd_archive_queue_read(file, entry, model, (CdCallback)resource_tmd_read_complete);
    }
}

ADDRESS(0x800408d0, 0xf8)
void resource_vab_update_range(s32 file, s32 entry, s32 bank, s32 count)
{
    u8 *wanted;
    s32 end;
    s32 i;

    wanted = SCRATCH->u.load.sound_wanted;
    for (i = entry, end = count + entry; i < end; i++, bank++) {
        SoundBank *record = &sound_status.banks[bank];
        SoundSlot *slot = record->slot;

        if (*wanted++ != 0) {
            if (slot == NULL) {
                audio_queue_vab_stream(file, i, bank);
            } else if (slot->state == 2) {
                slot->state = 1;
            }
        } else if (slot != NULL && slot->state == 1) {
            slot->state = 2;
        }
    }
}

ADDRESS(0x800409c8, 0x11c)
void resource_tmd_update_range(s32 file, s32 entry, s32 id, s32 count)
{
    u8 *wanted;
    s32 end;
    u8 *model;
    s32 i;

    wanted = SCRATCH->u.load.model_wanted;
    for (i = entry, end = count + entry; i < end; i++, id++) {
        if (*wanted++ != 0) {
            model = scene_display_sys.models[id];
            if (model == NULL) {
                resource_tmd_queue_read(file, i, id);
                continue;
            }
            if (memory_block_kind(model) == 1) {
                memory_block_set_kind(model, 2);
            }
        } else {
            model = scene_display_sys.models[id];
            if (model != NULL && memory_block_kind(model) != 3) {
                memory_block_set_kind(model, 1);
            }
        }
    }
}

/*
 * Draws the objects in view once a frame and keeps their files loaded: the
 * Actor records (models from 128, sound banks from 51), the MapObject
 * records (models from 256 and the area bank, sound banks from 147; kind
 * 0x1f plays its sound by the player's distance), the Effect records and
 * the placed models. The passes flag the model ids and sound banks in use
 * and resource_tmd_update_range/resource_vab_update_range load the missing ones and release the rest.
 */

ADDRESS(0x80040ae4, 0xff0)
void render_scene_and_update_resources(void)
{
    VECTOR pos;
    u8 *model;
    ScratchPad *scratch;
    Actor *obj;
    ActorGroup *def;
    MapObject *rec;
    MapObjectDef *rec_def;
    Effect *effect;
    MapPlacedEntry *placed;
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
    SCRATCH->ot = scene_display_sys.ot;
    SCRATCH->ot2 = scene_display_sys.ot2;
    repeat_store_word((u32 *)SCRATCH->u.load.models, 0, 0x20);
    repeat_store_word((u32 *)SCRATCH->u.load.sounds, 0, 0x18);
    SCRATCH->draw.count.clut = 0;
    obj = actor_status.objects;
    for (i = 199; i != -1; i--, obj++) {
        if (obj->lifecycle != 1) {
            continue;
        }
        if (obj->flags & 0x2000) {
            mask = obj->layer_mask | 0x10;
        } else {
            mask = obj->layer_mask;
        }
        if (!(obj->flags & 0x80000)) {
            if (!(map_cell_layer_mask(&obj->pos) & mask)) {
                continue;
            }
        } else {
            if (!(map_cell_layer_mask_radius(&obj->pos, 3) & obj->layer_mask)) {
                continue;
            }
        }
        resource_registry_get(obj->definition_id + 128, &model);
        if (model != NULL) {
            at = actor_resolve_group_position(obj, &pos);
            if (obj->flags & 0x20) {
                scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
                render_world_model(obj->layer_mask, obj->definition_id + 128, &obj->pos,
                                   &scratch->u.load.angle, &obj->scale, &obj->animation_cache,
                                   &render_world_identity_matrix, obj->animation_id, obj->animation_phase,
                                   obj->lighting_override_index, obj->lighting_blend, obj->render_mode,
                                   obj->render_depth_offset);
            } else {
                scratch->u.load.angle.vx = obj->angle.vx;
                scratch->u.load.angle.vy = obj->angle.vy + 2048;
                scratch->u.load.angle.vz = obj->angle.vz;
                render_world_model(obj->layer_mask, obj->definition_id + 128, at, &scratch->u.load.angle,
                                   &obj->scale, &obj->animation_cache, &scene_display_sys.view_matrix,
                                   obj->animation_id, obj->animation_phase, obj->lighting_override_index,
                                   obj->lighting_blend, obj->render_mode, obj->render_depth_offset);
            }
        }
        def = &actor_status.groups[obj->group_index];
        scratch->u.load.sounds[def->vab_resource_indices[0]] = 1;
        scratch->u.load.sounds[def->vab_resource_indices[1]] = 1;
        scratch->u.load.models[obj->definition_id] = 1;
    }
    resource_tmd_update_range(0, 0, 0x80, 0x80);
    resource_vab_update_range(2, 0x91, 0x33, 0x60);
    now = cdrom_ctl.frames;
    repeat_store_word((u32 *)scratch->u.load.models, 0, 0x53);
    repeat_store_word((u32 *)scratch->u.load.sounds, 0, 8);
    if (scene_display_sys.unk15996 == 0) {
        asset_status.animated_object = NULL;
    }
    scratch->draw.count.clut = 0;
    rec = map_objects_slots.objects;
    for (i = 395; i != -1; i--, rec++) {
        if (rec->object_id == 0xff) {
            continue;
        }
        rec->flags &= 0x7f;
        switch (rec->action) {
        case 0xe5:
        case 0xe9:
            break;
        case 0xf2:
            if (map_cell_visible(&rec->pos, rec->arg.m.radius_x, rec->arg.m.radius_z) == 0) {
                break;
            }
            scratch->u.load.models[rec->object_id] = 1;
            id = map_object_asset_index(rec->object_id);
            resource_registry_get(id, &model);
            if (model == NULL) {
                break;
            }
            scratch->u.load.angle.vx = rec->angle.vx;
            scratch->u.load.angle.vy = rec->angle.vy + 2048;
            scratch->u.load.angle.vz = rec->angle.vz;
            pos.vx = ((rec->arg.m.cell_offset_x << 11) + rec->pos.vx) >> 1;
            pos.vz = ((rec->arg.m.cell_offset_z << 11) + rec->pos.vz) >> 1;
            pos.vy = (rec->pos.vy - (rec->arg.m.cell_height << 11)) >> 1;
            render_distant_world_model(rec->layer_mask, id, &pos, &scratch->u.load.angle, &rec->scale,
                                       &rec->arg.m.animation_cache, &scene_display_sys.view_matrix,
                                       rec->animation_id, rec->animation_phase, rec->lighting_override_index,
                                       rec->lighting_blend, rec->render_mode, rec->render_depth_offset);
            rec->flags |= 0x80;
            break;
        case 0xf0:
            if (scene_display_sys.unk15996 == 0) {
                break;
            }
            if (map_cell_visible(&rec->pos, rec->arg.v.radius_x, rec->arg.v.radius_z) == 0) {
                break;
            }
            id = rec->object_id - 221;
            resource_registry_get(id, &model);
            if (model != NULL) {
                render_animated_object(id, &rec->angle, &rec->arg.v.animation_cache, rec->animation_id,
                                       rec->animation_phase, rec->arg.v.blend_mode, rec->arg.v.lighting_index,
                                       0x1fff - rec->arg.v.depth_code);
                rec->flags |= 0x80;
            }
            asset_status.animated_object_copy = *rec;
            asset_status.animated_object = &asset_status.animated_object_copy;
            break;
        case 0x1f:
            if (player_camera_within_map_region(rec->pos.vx >> 11, rec->pos.vz >> 11, rec->arg.s.region_width,
                                                rec->arg.s.region_depth, 0x8000) != 0) {
                sound = &sound_status.effects[rec->arg.s.sound_id + 496];
                if (sound->bank >= 147 && sound->bank < 179) {
                    scratch->u.load.sounds[sound->bank - 147] = 1;
                }
                if (rec->arg.s.next_sound_frame - now < 0) {
                    rec->arg.s.next_sound_frame = now + rec->arg.s.repeat_delay_units * 6;
                    range = rec->arg.s.region_width << 10;
                    dist = cur_avatar_status.pos.vx - (range + rec->pos.vx);
                    if (dist < 0) {
                        dist = -dist;
                    }
                    near = range - dist;
                    range = rec->arg.s.region_depth << 10;
                    dist = cur_avatar_status.pos.vz - (range + rec->pos.vz);
                    if (dist < 0) {
                        dist = -dist;
                    }
                    dist = range - dist;
                    if (dist < near) {
                        near = dist;
                    }
                    range = rec->arg.s.fade_distance << 11;
                    if (near >= range) {
                        volume = rec->arg.s.maximum_volume;
                    } else {
                        if (range == 0) {
                            break;
                        }
                        volume = rec->arg.s.maximum_volume * near / range;
                    }
                    if (rec->arg.s.vertical_attenuation_flags & 1) {
                        dist = cur_avatar_status.pos.vy - rec->pos.vy;
                        if (dist < 0) {
                            dist = -dist;
                        }
                        volume -= (rec->arg.s.maximum_volume * dist) >> 13;
                    }
                    if (volume >= 20) {
                        audio_play_sound(rec->arg.s.sound_id + 496, volume);
                    }
                }
            } else {
                rec->arg.s.next_sound_frame = now + rec->arg.s.repeat_delay_units * 6;
            }
            break;
        default:
            if (!(rec->flags & 2) || (rec->flags & 8)) {
                cells = map_cell_layer_mask(&rec->pos);
                if (!(cells & rec->layer_mask) && !(rec->flags & 8)) {
                    break;
                }
                rec_def = &map_objects_slots.defs[rec->object_id];
            } else {
                rec_def = &map_objects_slots.defs[rec->object_id];
                cells = map_cell_layer_mask_radius(&rec->pos, rec_def->arg.b.unkc);
                if (!(cells & rec->layer_mask) && !(rec->flags & 8)) {
                    break;
                }
            }
            scratch->u.load.models[rec->object_id] = 1;
            scratch->u.load.sounds[rec_def->vab_resource_index] = 1;
            id = map_object_asset_index(rec->object_id);
            resource_registry_get(id, &model);
            if (model == NULL) {
                break;
            }
            scratch->u.load.angle.vx = rec->angle.vx;
            scratch->u.load.angle.vy = rec->angle.vy + 2048;
            scratch->u.load.angle.vz = rec->angle.vz;
            flags = rec->render_mode;
            if (rec->flags & 1) {
                if (cells & 4) {
                    flags |= 0x40;
                } else {
                    flags &= 0xbf;
                }
            }
            if (rec->flags & 0x40) {
                scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
                render_world_model(rec->layer_mask, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                                   &rec->arg.m.animation_cache, &scene_display_sys.pitch_matrix,
                                   rec->animation_id, rec->animation_phase, rec->lighting_override_index,
                                   rec->lighting_blend, flags, rec->render_depth_offset);
            } else if (rec->flags & 8) {
                render_distant_world_model(rec->layer_mask, id, &rec->pos, &scratch->u.load.angle,
                                           &rec->scale, &rec->arg.m.animation_cache,
                                           &scene_display_sys.view_matrix, rec->animation_id,
                                           rec->animation_phase, rec->lighting_override_index,
                                           rec->lighting_blend, flags, rec->render_depth_offset);
            } else {
                render_world_model(rec->layer_mask, id, &rec->pos, &scratch->u.load.angle, &rec->scale,
                                   &rec->arg.m.animation_cache, &scene_display_sys.view_matrix,
                                   rec->animation_id, rec->animation_phase, rec->lighting_override_index,
                                   rec->lighting_blend, flags, rec->render_depth_offset);
            }
            rec->flags |= 0x80;
            break;
        }
    }
    scratch->u.load.models[265] = 0;
    scratch->u.load.models[264] = 0;
    resource_tmd_update_range(0, 0x80, 0x100, 0x118);
    scratch->u.load.model_wanted = &scratch->u.load.models[300];
    if ((s16)asset_status.loading == 1 && asset_status.area_records_loaded == 1
        && asset_status.requested_resource_ids[0] != 0xff) {
        resource_tmd_update_range(1, asset_status.requested_resource_ids[0] * 32,
                                  asset_status.requested_resource_ids[0] * 32 + 556, 32);
    } else {
        resource_tmd_update_range(1, asset_status.active_resource_ids[0] * 32,
                                  asset_status.active_resource_ids[0] * 32 + 556, 32);
    }
    resource_vab_update_range(2, 0xf1, 0x93, 0x20);
    if ((s16)asset_status.loading == 1 && asset_status.step > 0
        && (rec = asset_status.animated_object) != NULL) {
        id = rec->object_id - 221;
        render_animated_object(id, &rec->angle, &rec->arg.v.animation_cache, rec->animation_id,
                               rec->animation_phase, rec->arg.v.blend_mode, rec->arg.v.lighting_index,
                               0x1fff - rec->arg.v.depth_code);
        rec->flags |= 0x80;
    }
    effect = effect_status.objects;
    for (i = 127; i != -1; i--, effect++) {
        if (effect->type == 0xff) {
            continue;
        }
        if ((effect->render_flags & 3) == 0) {
            continue;
        }
        if ((effect->render_flags & 3) != 2 && !(map_cell_layer_mask(&effect->pos) & effect->layer_mask)) {
            continue;
        }
        scratch->draw.count.clut = effect->clut_offset;
        switch (effect->render_flags & 0xc) {
        case 0:
            scratch->u.load.angle.vx = effect->angle.vx;
            scratch->u.load.angle.vy = effect->angle.vy + 2048;
            scratch->u.load.angle.vz = effect->angle.vz;
            render_world_model(effect->layer_mask, effect->render_id + 40, &effect->pos,
                               &scratch->u.load.angle, &effect->scale, &effect->animation_cache,
                               &scene_display_sys.view_matrix, effect->animation_id, effect->animation_phase,
                               effect->lighting_override_index, effect->lighting_blend, effect->render_mode,
                               -60);
            break;
        case 4:
            render_world_model(effect->layer_mask, effect->render_id + 40, &effect->pos, &effect->angle,
                               &effect->scale, &effect->animation_cache, &render_world_identity_matrix,
                               effect->animation_id, effect->animation_phase, effect->lighting_override_index,
                               effect->lighting_blend, effect->render_mode, -60);
            break;
        case 8:
            render_world_model(effect->layer_mask, effect->render_id + 40, &effect->pos, &effect->angle,
                               &effect->scale, &effect->animation_cache, &scene_display_sys.pitch_matrix,
                               effect->animation_id, effect->animation_phase, effect->lighting_override_index,
                               effect->lighting_blend, effect->render_mode, -60);
            break;
        case 0xc:
            render_world_model(effect->layer_mask, effect->render_id + 40, &effect->pos, &effect->angle,
                               &effect->scale, &effect->animation_cache, NULL, effect->animation_id,
                               effect->animation_phase, effect->lighting_override_index,
                               effect->lighting_blend, effect->render_mode, 20);
            break;
        }
    }
    placed = map_placed_entries;
    scratch->u.load.angle.vx = scratch->u.load.angle.vy = scratch->u.load.angle.vz = 0;
    scratch->draw.count.clut = 0;
    for (i = 127; i != -1; i--, placed++) {
        if (placed->model_index == 0xffff) {
            continue;
        }
        if (placed->model_index & 0x8000) {
            render_distant_world_model(placed->layer_mask, (placed->model_index & 0x7fff) + 40, &placed->pos,
                                       &scratch->u.load.angle, NULL, NULL, &render_world_identity_matrix,
                                       placed->frame_index + 128, 0, 0x3c, 0x1000, 7, 0);
        } else if (map_cell_layer_mask(&placed->pos) & placed->layer_mask) {
            render_world_model(placed->layer_mask, placed->model_index + 40, &placed->pos,
                               &scratch->u.load.angle, NULL, NULL, &scene_display_sys.pitch_matrix,
                               placed->frame_index + 128, 0, 0x36, 0x1000, 5, 0);
        }
        if (placed->frame_period != 0 && map_placed_frame_counter % placed->frame_period == 0) {
            placed->frame_index++;
            if (placed->frame_index >= placed->frame_count) {
                placed->frame_index = 0;
            }
        }
    }
    map_placed_frame_counter++;
}

ADDRESS(0x80041ad4, 0x2c8)
void render_sprite_record(SpriteRec *sprite, u16 mode, u8 *rgb)
{
    DR_MODE *dr;
    SPRT *p;

    dr = (DR_MODE *)scene_display_sys.pool->next;
    scene_display_sys.pool->next += sizeof(DR_MODE);
    if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
        return;
    }
    scene_display_sys.prim_count++;
    p = (SPRT *)scene_display_sys.pool->next;
    scene_display_sys.pool->next += sizeof(SPRT);
    if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
        return;
    }
    scene_display_sys.prim_count++;
    if (asset_status.active_resource_ids[0] == 10 && mode == 2) {
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
void notification_draw(void)
{
    SpriteRec *sprite;
    u8 bright[4];
    u8 dim[4];

    sprite = notification_quads;
    bright[0] = bright[1] = bright[2] = scene_display_sys.message_level;
    dim[0] = dim[2] = dim[1] = scene_display_sys.message_level >> 2;
    for (; sprite->visible != 0xff; sprite++) {
        if (sprite->visible & 1) {
            render_sprite_record(sprite, 1, bright);
        }
        if (sprite->visible & 2) {
            render_sprite_record(sprite, 2, bright);
        }
        if (sprite->visible & 4) {
            render_sprite_record(sprite, 2, dim);
        }
    }
}

ADDRESS(0x80041e68, 0x84)
void render_hud_sprites(void)
{
    SpriteRec *sprite;
    u8 rgb[4];

    sprite = hud_sprites;
    rgb[0] = rgb[1] = rgb[2] = cur_avatar_status.brightness;
    for (; sprite->visible != 0xff; sprite++) {
        if (sprite->visible != 0) {
            render_sprite_record(sprite, sprite->mode, rgb);
        }
    }
}

ADDRESS(0x80041eec, 0xa0)
void notify_enqueue(s32 message, ...)
{
    va_list args;

    va_start(args, message);
    if (message < 240) {
        if (scene_display_sys.message_queue[scene_display_sys.message_head] == 0xff) {
            scene_display_sys.message_queue[scene_display_sys.message_head] = message;
            if (message == 25) {
                scene_display_sys.message_arg[scene_display_sys.message_head] = va_arg(args, s32);
            }
            scene_display_sys.message_head = (scene_display_sys.message_head + 1) & 7;
        }
    }
    va_end(args);
}

ADDRESS(0x80041f8c, 0x10)
void notification_digit_set_u(SpriteRec *sprite, u8 digit)
{
    sprite->u = digit * 8;
}

ADDRESS(0x80041f9c, 0x31c)
void notification_update(void)
{
    u32 message;
    u8 digits[20];

    switch (scene_display_sys.message_state) {
    case 0:
        message = scene_display_sys.message_queue[scene_display_sys.message_tail];
        if (message == 0xff) {
            break;
        }
        scene_display_sys.message_state = 1;
        scene_display_sys.message_timer = 15;
        scene_display_sys.message_level = 0;
        if (message == 25) {
            notification_quads[0].visible = 0;
            notification_quads[1].visible = 3;
            notification_quads[1].u = 0x80;
            notification_quads[1].v = 0x62;
            menu_format_number(scene_display_sys.message_arg[scene_display_sys.message_tail], 4, 0, 0,
                               digits);
            notification_quads[2].visible = 3;
            notification_digit_set_u(&notification_quads[2], digits[3]);
            notification_quads[3].visible = 3;
            notification_digit_set_u(&notification_quads[3], digits[2]);
            notification_quads[4].visible = 3;
            notification_digit_set_u(&notification_quads[4], digits[1]);
            notification_quads[5].visible = 3;
            notification_digit_set_u(&notification_quads[5], digits[0]);
        } else {
            notification_quads[0].visible = 3;
            notification_quads[1].visible = notification_quads[2].visible = notification_quads[3].visible =
                notification_quads[4].visible = notification_quads[5].visible = 0;
            notification_quads[0].u = message / 18 * 128;
            notification_quads[0].v = (u8)(message % 18) * 14;
        }
        break;
    case 1:
        scene_display_sys.message_level += 20;
        if (scene_display_sys.message_level >= 100) {
            scene_display_sys.message_state = 2;
        }
        break;
    case 2:
        if (--scene_display_sys.message_timer == 0) {
            scene_display_sys.message_state = 3;
        }
        break;
    case 3:
        scene_display_sys.message_level -= 20;
        if (scene_display_sys.message_level == 0) {
            message = notification_quads[0].visible = notification_quads[1].visible =
                notification_quads[2].visible = notification_quads[3].visible =
                    notification_quads[4].visible =
                    notification_quads[5].visible =
                        scene_display_sys.message_queue[scene_display_sys.message_tail];
            do {
                scene_display_sys.message_queue[scene_display_sys.message_tail] = 0xff;
                scene_display_sys.message_tail = (scene_display_sys.message_tail + 1) & 7;
            } while (message == scene_display_sys.message_queue[scene_display_sys.message_tail]
                     && message != 25);
            scene_display_sys.message_state = 0;
        }
        break;
    }
}

ADDRESS(0x800422b8, 0x654)
void render_game_frame(VECTOR *pos, SVECTOR *angle)
{
    s32 i;

    map_textured_primitive_color.b = map_textured_primitive_color.g = map_textured_primitive_color.r =
        cur_avatar_status.brightness;
    display_set_view_transform(pos, angle);
    vram_scroll_update();
    notification_update();
    view_build_visibility_grid();
    display_begin_frame();
    pool_mark_allocated();
    render_player_weapon();
    hud_sprites[13].visible = hud_sprites[14].visible = 0;
    render_model_rows[0].visible = cur_avatar_status.compass_display & 1;
    render_model_rows[1].visible = (cur_avatar_status.compass_display & 2) >> 1;
    hud_sprites[0].visible = hud_sprites[1].visible = hud_sprites[2].visible =
        hud_sprites[3].visible = hud_sprites[4].visible = hud_sprites[5].visible =
            hud_sprites[6].visible = hud_sprites[7].visible = hud_sprites[8].visible =
                hud_sprites[9].visible = hud_sprites[10].visible = hud_sprites[11].visible =
                    hud_sprites[12].visible = cur_avatar_status.vitals_display & 1;
    hud_sprites[15].visible = hud_sprites[16].visible = hud_sprites[17].visible =
        hud_sprites[18].visible = hud_sprites[19].visible = hud_sprites[20].visible =
            hud_sprites[21].visible = hud_sprites[22].visible = hud_sprites[23].visible =
                hud_sprites[24].visible = hud_sprites[25].visible = hud_sprites[26].visible =
                    hud_sprites[27].visible = hud_sprites[28].visible =
                        hud_sprites[29].visible = (cur_avatar_status.vitals_display & 2) >> 1;
    i = cur_avatar_status.compass_display - 1;
    if (i >= 0) {
        render_model_yaw_smoothing_accumulator +=
            angle_shortest_delta(render_model_rows[i].angle.vy, scene_display_sys.view_rotation.vy);
        if (render_model_yaw_smoothing_accumulator > 0) {
            render_model_yaw_smoothing_accumulator -= (render_model_yaw_smoothing_accumulator + 7) >> 3;
        } else if (render_model_yaw_smoothing_accumulator < 0) {
            render_model_yaw_smoothing_accumulator -= (render_model_yaw_smoothing_accumulator - 7) >> 3;
        }
        render_model_rows[i == 0].angle.vy = render_model_rows[i].angle.vy =
            render_model_rows[i].angle.vy + (render_model_yaw_smoothing_accumulator >> 6);
        render_model_rows[i == 0].angle.vx = render_model_rows[i].angle.vx =
            scene_display_sys.view_rotation.vx;
    }
    if (cur_avatar_status.vitals_display == 1) {
        hud_sprites[4].width = hud_sprites[4].w = cur_avatar_status.hp * 48 / cur_avatar_status.hp_max;
        hud_sprites[5].width = hud_sprites[5].w = cur_avatar_status.attack_charge_current * 48 / 5000;
        hud_sprites[6].width = hud_sprites[6].w = cur_avatar_status.mp * 48 / cur_avatar_status.mp_max;
        hud_sprites[7].width = hud_sprites[7].w = cur_avatar_status.magic_charge * 48 / 5000;
    } else if (cur_avatar_status.vitals_display == 2) {
        u16 hp;
        u16 mp;
        s32 n;

        hp = cur_avatar_status.hp % 1000;
        n = hp;
        hud_sprites[17].u = n / 100 * 12 + 0x80;
        hud_sprites[18].u = n % 100 / 10 * 12 + 0x80;
        hud_sprites[19].u = n % 10 * 12 + 0x80;
        mp = cur_avatar_status.mp % 1000;
        n = mp;
        hud_sprites[22].u = n / 100 * 12 + 0x80;
        hud_sprites[23].u = n % 100 / 10 * 12 + 0x80;
        hud_sprites[24].u = n % 10 * 12 + 0x80;
        hud_sprites[25].width = hud_sprites[25].w = cur_avatar_status.attack_charge_current * 64 / 5000;
        hud_sprites[26].width = hud_sprites[26].w = cur_avatar_status.magic_charge * 64 / 5000;
    }
    render_active_model_rows();
    render_hud_sprites();
    notification_draw();
    render_map_cell_window();
    render_scene_and_update_resources();
    render_sliding_panel_primary();
    render_sliding_panel_secondary();
    render_sliding_panel_tertiary();
    render_color_overlay();
    render_accumulated_color_overlay_additive();
    render_accumulated_color_overlay_subtractive();
    display_present_frame();
    cd_wait_four_vsyncs();
    pool_release_stale();
}

/*
 * Draws model 3 (the menu's item model) with the matrices the caller set:
 * projects its vertices into GraphicsRuntime.screen with a depth-cue value from
 * the fog range, then draws part 0 at depth offset 100.
 * Not exact: retail stores the CLUT offset between SCRATCH->screen and
 * SCRATCH->end; every statement order tried schedules it earlier.
 */

ADDRESS(0x8004290c, 0x264)
void menu_render_item_model(void)
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
    model = scene_display_sys.tmd_slots[3];
    SCRATCH->model = model;
    SCRATCH->part = ((ModelData *)model)->parts;
    offset = SCRATCH->part->vertices + 12;
    SCRATCH->vertices = (u8 *)(offset + (s32)model);
    fog_set_range(20000, 24000);
    SCRATCH->ot = scene_display_sys.ot;
    SCRATCH->draw.count.tmd_prim_count = scene_display_sys.tmd_prim_count;
    SCRATCH->draw.count.ot_count = scene_display_sys.ot_count;
    SCRATCH->draw.count.prim_count = scene_display_sys.prim_count;
    SCRATCH->next = scene_display_sys.pool->next;
    SCRATCH->draw.count.clut = 0;
    SCRATCH->end = scene_display_sys.pool->end;
    *(u32 *)&SCRATCH->w.model.rgb = *(u32 *)&map_textured_primitive_color;
    SCRATCH->screen = scene_display_sys.screen;
    i = ((ModelData *)SCRATCH->model)->parts[0].vertex_count;
    vertex = SCRATCH->vertices;
    screen = scene_display_sys.screen;
    while (--i != -1) {
        gte_ldv0(vertex);
        gte_rtps();
        gte_stsxy(&screen->sxy);
        gte_stszotz(&screen->z);
        if (scene_display_sys.fog_near != scene_display_sys.fog_far) {
            screen->fog = ((screen->z - (scene_display_sys.fog_near >> 2)) << 14) /
                          (scene_display_sys.fog_far - scene_display_sys.fog_near);
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
    render_enqueue_textured_tmd(0, 100);
    scene_display_sys.pool->next = scratch->next;
    scene_display_sys.tmd_prim_count = scratch->draw.count.tmd_prim_count;
    scene_display_sys.ot_count = scratch->draw.count.ot_count;
}

ADDRESS(0x80042b70, 0x38)
void asset_registry_select(u16 id)
{
    u8 *file;

    file = scene_display_sys.models[id];
    scene_display_sys.current_tmd = file + ((ModelFile *)file)->tmd;
}

ADDRESS(0x80042ba8, 0xbc)
void asset_registry_load_tmd_archive(u16 id, u8 *file)
{
    u16 count;
    u32 size;

    count = *(u16 *)file;
    file += 4;
    while (count--) {
        size = *(u32 *)file;
        if (size >= 5) {
            scene_display_sys.models[id] = file;
            asset_registry_select(id);
            tmd_prepare_primitive_indices(scene_display_sys.current_tmd);
        }
        file += size;
        id++;
    }
}

ADDRESS(0x80042c64, 0x48)
void asset_registry_set(u16 id, u8 *file)
{
    scene_display_sys.models[id] = file;
    asset_registry_select(id);
    tmd_prepare_primitive_indices(scene_display_sys.current_tmd);
}

ADDRESS(0x80042cac, 0xc4)
AnimKey *animation_select_keyframe(ModelFile *model, s32 anim, s32 time, s32 *index, s32 *frac)
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
void animation_expand_sparse_vertices(SVECTOR *pose, SVECTOR *base, s16 *deltas)
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
void animation_decode_sparse_vertices(SVECTOR *pose, s16 *deltas)
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
void animation_apply_sparse_morph(SVECTOR *pose, s16 *deltas, s32 t)
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
s16 *animation_find_sparse_vertex(s16 *deltas, s32 index)
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
s32 animation_prepare_asset_vertices(AnimSlot **owner, s32 id, s32 anim, s32 time, s32 count)
{
    ModelFile *model = (ModelFile *)scene_display_sys.models[id];
    AnimSlot *slot = *owner;
    ScratchPad *scratch = SCRATCH;
    AnimKey *key;
    s32 *deltas;
    u16 *ids;
    u16 n;
    s32 index;
    s32 frac;

    if (model->animated == 0) {
        u8 *data;

        if (slot != NULL) {
            pool_record_release(slot);
        }
        data = scene_display_sys.models[id];
        data += ((ModelFile *)data)->tmd;
        scratch->model = data;
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        return 1;
    }

    if (slot == NULL) {
        pool_allocate(&slot);
        if (slot == NULL) {
            return 0;
        }
    init:
        slot->model = id;
        slot->owner = owner;
    retry:
        if ((slot->pose = (SVECTOR *)memory_malloc_checked(count * 8)) == NULL) {
            pool_release_all();
            goto retry;
        }
        *owner = slot;
    } else if (slot->model != id) {
        pool_record_release(slot);
        slot->anim = 0xff;
        goto init;
    }

    key = animation_select_keyframe(model, anim, time, &index, &frac);
    if (slot->anim != anim || slot->key != index) {
        scratch->part = ((ModelData *)scratch->model)->parts;
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        deltas = (s32 *)((u8 *)model + model->deltas);
        n = key->count;
        if (n != 0) {
            ids = key->ids;
            animation_expand_sparse_vertices(slot->pose, (SVECTOR *)scratch->vertices,
                                             (s16 *)((u8 *)model + deltas[key->first]));
            for (--n; n != 0; --n) {
                animation_decode_sparse_vertices(slot->pose, (s16 *)((u8 *)model + deltas[*ids++]));
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
        u32 *dst = (u32 *)scene_display_sys.pose;

        do {
            *dst++ = *src++;
            *dst++ = *src++;
        } while (--c != 0);
    }
    animation_apply_sparse_morph(scene_display_sys.pose, (s16 *)((u8 *)model + slot->base), frac);
    scratch->vertices = (u8 *)scene_display_sys.pose;
    slot->state = 2;
    return (s32)slot;
}

/*
 * id is a full int (narrowed only for resource_registry_get): a narrow first
 * parameter would emit an entry conversion, and GCC then drops the stack
 * equivalence of `out`, which changes its allocation priority ($s6/$s7).
 */
ADDRESS(0x8004352c, 0x29c)
s32 animation_sample_vertex(s32 id, s32 anim, s32 time, s32 vertex, SVECTOR *out)
{
    u8 *loaded;
    ModelFile *file;
    u8 *data;
    SVECTOR *vertices;

    resource_registry_get(id, &loaded);
    file = (ModelFile *)loaded;
    if (file == NULL) {
        out->vx = out->vy = out->vz = 0;
        return 1;
    }
    data = (u8 *)file + file->tmd;
    if (anim >= 128) {
    single:
        vertices = (SVECTOR *)(data + ((((ModelData *)data)->parts + (anim & 0x7f))->vertices + 12));
        *out = vertices[vertex];
    } else if (file->animated == 0) {
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
        key = animation_select_keyframe(file, anim, time, &index, &frac);
        v = vertices[vertex];
        deltas = (s32 *)((u8 *)file + file->deltas);
        ids = &key->first;
        n = key->count;
        while (--n != -1) {
            d = animation_find_sparse_vertex((s16 *)((u8 *)file + deltas[*ids++]), vertex);
            if (d != NULL) {
                v.vx = *d++;
                v.vy = *d++;
                v.vz = *d;
            }
        }
        d = animation_find_sparse_vertex((s16 *)((u8 *)file + deltas[key->base]), vertex);
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
s32 asset_vertex_count(s32 id, s32 part)
{
    u8 *file;
    ModelData *data;

    file = scene_display_sys.models[id];
    if (file == NULL) {
        return 0;
    }
    data = (ModelData *)(file + ((ModelFile *)file)->tmd);
    return part < 128 ? data->parts[0].vertex_count : (data->parts + (part & 0x7f))->vertex_count;
}

ADDRESS(0x80043828, 0x30)
void pool_reset(void)
{
    AnimSlot *slot;
    u16 n;

    slot = scene_display_sys.slots;
    n = 12;
    do {
        slot->state = 0;
        slot->pose = NULL;
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043858, 0x3c)
void pool_mark_allocated(void)
{
    AnimSlot *slot;
    u16 n;

    slot = scene_display_sys.slots;
    n = 12;
    do {
        if (slot->state != 0) {
            slot->state = 1;
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043894, 0x4c)
void pool_record_release(AnimSlot *slot)
{
    slot->state = 0;
    *slot->owner = NULL;
    if (slot->pose != NULL) {
        free((void *)slot->pose);
        slot->pose = NULL;
    }
}

ADDRESS(0x800438e0, 0x60)
void pool_release_all(void)
{
    AnimSlot *slot;
    s16 n;

    slot = scene_display_sys.slots;
    n = 12;
    do {
        if (slot->state != 0) {
            pool_record_release(slot);
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x80043940, 0x70)
void pool_release_stale(void)
{
    AnimSlot *slot;
    u16 n;

    slot = scene_display_sys.slots;
    n = 12;
    do {
        if (slot->state == 1) {
            pool_record_release(slot);
        }
        slot++;
    } while (--n != 0);
}

ADDRESS(0x800439b0, 0x58)
void pool_allocate(AnimSlot **out)
{
    u16 n;

    *out = scene_display_sys.slots;
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
void map_placed_expand_sources(MapPlacedSource *spawn)
{
    MapPlacedEntry *placed;
    s32 i;

    placed = map_placed_entries;
    for (i = 127; i != -1; i--) {
        if (spawn->model_index != 0xffff) {
            placed->model_index = spawn->model_index;
            placed->layer_mask = spawn->layer_mask;
            placed->frame_count = spawn->frame_count;
            placed->frame_period = spawn->frame_period;
            placed->pos.vx = (spawn->cell_x << 11) + spawn->dx;
            placed->pos.vz = (spawn->cell_z << 11) + spawn->dz;
            placed->pos.vy = collision_sample_map_layer_height(placed->layer_mask, placed->pos.vx,
                                                               placed->pos.vz, 0, 0)
                + spawn->dy;
            placed->frame_index = (rand() * placed->frame_count) >> 15;
        } else {
            placed->model_index = 0xffff;
        }
        placed++;
        spawn++;
    }
}

ADDRESS(0x80043b38, 0x80)
void tim_upload_images(u8 *file)
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
s32 menu_fade_transition(s32 level, s32 step)
{
    POLY_FT4 *p;
    s32 result;
    u32 pad;
    s32 dim;

    result = -1;
    for (;;) {
        display_begin_frame();
        p = (POLY_FT4 *)scene_display_sys.pool->next;
        scene_display_sys.pool->next += sizeof(POLY_FT4);
        dim = 128 - (level >> 1);
        if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
            return KF_VALUELESS;
        }
        scene_display_sys.prim_count++;
        setPolyFT4(p);
        setSemiTrans(p, 1);
        setXYWH(p, 32, 32, 256, 208);
        setUVWH(p, 0, 0, 255, 208);
        p->clut = GetClut(96, 480);
        p->tpage = GetTPage(0, 1, 960, 256);
        setRGB0(p, level, level, level);
        addPrim(scene_display_sys.ot, p);
        p = (POLY_FT4 *)scene_display_sys.pool->next;
        scene_display_sys.pool->next += sizeof(POLY_FT4);
        if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
            return KF_VALUELESS;
        }
        scene_display_sys.prim_count++;
        setPolyFT4(p);
        setSemiTrans(p, 1);
        setXYWH(p, 33, 33, 256, 208);
        setUVWH(p, 0, 0, 255, 208);
        p->clut = GetClut(96, 480);
        p->tpage = GetTPage(0, 2, 960, 256);
        setRGB0(p, level, level, level);
        addPrim(scene_display_sys.ot, p);
        p = (POLY_FT4 *)scene_display_sys.pool->next;
        scene_display_sys.pool->next += sizeof(POLY_FT4);
        if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
            return KF_VALUELESS;
        }
        scene_display_sys.prim_count++;
        setPolyFT4(p);
        setXYWH(p, 0, 0, 192, 240);
        setUVWH(p, 0, 0, 192, 240);
        p->clut = 0;
        p->tpage = GetTPage(2, 0, 320, 0);
        setRGB0(p, dim, dim, dim);
        addPrim(scene_display_sys.ot, p);
        p = (POLY_FT4 *)scene_display_sys.pool->next;
        scene_display_sys.pool->next += sizeof(POLY_FT4);
        if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
            return KF_VALUELESS;
        }
        scene_display_sys.prim_count++;
        setPolyFT4(p);
        setXYWH(p, 192, 0, 128, 240);
        setUVWH(p, 0, 0, 128, 240);
        p->clut = 0;
        p->tpage = GetTPage(2, 0, 512, 0);
        setRGB0(p, dim, dim, dim);
        addPrim(scene_display_sys.ot, p);
        level += step;
        DrawSync(0);
        display_present_frame();
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
            if (pad & action_button_masks[7]) {
                cur_avatar_status.pad_buttons |= action_button_masks[7];
            } else if (pad != 0) {
                cur_avatar_status.pad_buttons &= ~action_button_masks[7];
            }
            return level;
        }
    }
    DrawSync(0);
    if (pad & action_button_masks[7]) {
        cur_avatar_status.pad_buttons |= action_button_masks[7];
    } else if (pad != 0) {
        cur_avatar_status.pad_buttons &= ~action_button_masks[7];
    }
    return result;
}

/* Shows TIM entry `entry` of file `file` between a fade in and a fade out over
 * the saved screen; a button press cuts the wait short. */

ADDRESS(0x800441d4, 0x1f4)
void menu_show_transition_image(u16 file, u16 entry)
{
    s32 level;
    u32 pad;

    DrawSync(0);
    cd_archive_read(file, entry, scene_display_sys.buffer);
    ClearImage(&menu_transition_image_rect, 0, 0, 0);
    DrawSync(0);
    tim_upload_images(scene_display_sys.buffer);
    DrawSync(0);
    scene_display_sys.pools[0].end = scene_display_sys.pools[0].base + 0x7400;
    scene_display_sys.pools[1].base = scene_display_sys.pools[0].end;
    scene_display_sys.pools[1].end = scene_display_sys.pools[0].end + 0x7400;
    StoreImage(&menu_transition_rect, (u_long *)scene_display_sys.pools[1].end);
    DrawSync(0);
    MoveImage(&scene_display_sys.draw[scene_display_sys.frame].clip, menu_transition_rect.x,
              menu_transition_rect.y);
    DrawSync(0);
    level = menu_fade_transition(0, 12);
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
        if (pad & action_button_masks[7]) {
            cur_avatar_status.pad_buttons |= action_button_masks[7];
        } else if (pad != 0) {
            cur_avatar_status.pad_buttons &= ~action_button_masks[7];
        }
        level = 80;
    }
    menu_fade_transition(level, -12);
    LoadImage(&menu_transition_rect, (u_long *)scene_display_sys.pools[1].end);
    scene_display_sys.pools[0].end = scene_display_sys.pools[0].base + 0x1a000;
    scene_display_sys.pools[1].base = scene_display_sys.pools[0].end;
    scene_display_sys.pools[1].end = scene_display_sys.pools[0].end + 0x1a000;
}
