/*
 * GAME.EXE: KF2's display.c from tmd_prepare_primitive_indices on: the model
 * primitive fix-up, model data registration, the model part renderers, the
 * map layer front ends with their ground renderers, the map layers of the
 * view grid, the 3D overlay and model drawing front ends, full-screen 2D
 * effects and the loading screen, object model files, the map cell grid and
 * the per-frame object drawer, the 2D overlay sprites and messages, the
 * animated model poses and the picture fade. One TU: GCC 2.6 emits a TU's
 * literals ahead of its jump tables, and loading_screen_show's "\LOAD.MSG;1"
 * (0x80011bd0) precedes tmd_prepare_primitive_indices' jump table
 * (0x80011be0). render_map_cell_window, render_active_model_rows,
 * render_player_weapon and render_animated_object read
 * map_textured_primitive_color and menu_show_transition_image reads
 * menu_transition_rect $gp-relative, which ASPSX emits only for .sdata
 * defined in the same file.
 */
/* render_scene_and_update_resources reads ResourceState.step as volatile (see game/data.h). */
#define GAME_RENDER_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <INLINE.H>
#include <psyq/cd.h>
#include <psyq/libc.h>
#include <GTEMAC.H>
#include <kf/lib/null.h>
#include <game/sound.h>
#include <game/actor.h>
#include <game/map_object.h>
#include <stdarg.h>
#include <MALLOC.H>
#include <RAND.H>
#include <game/cd.h>
#include <game/player.h>

RODATA(0x80011bd0, 0x84)

/*
 * Fixes up the primitive records of every part of a model data block: the
 * vertex and normal numbers become byte offsets (8 per SVECTOR) and the
 * textured kinds lose the blend bits (5-6) of their texture page word. The
 * kind is the record's GPU code without the semi-transparency bit (2), which
 * is cleared in the copy of the head word only.
 * Not exact: retail keeps the part counter, the part pointer and the
 * 0x00ffffff mask in t0, t1 and t2; here the mask is used three times and
 * takes t0 (nine instructions differ by a register). The three stores of
 * the code byte stand in for a form not found yet that uses the mask twice.
 */
ADDRESS(0x8003591c, 0x328)
void tmd_prepare_primitive_indices(u8 *data)
{
    ModelData *model;
    ModelPart *part;
    union {
        u32 w;
        PrimTag t;
    } tag;
    u8 *prim;
    u16 *body;
    s32 i;
    s32 j;
    s32 code;

    model = (ModelData *)data;
    part = model->parts;
    i = model->count;
    while (--i != -1) {
        j = part->prim_count;
        prim = data + (part->prims + 12);
        while (--j != -1) {
            body = (u16 *)(prim + 4);
            tag.w = *(u32 *)prim;
            prim = (u8 *)body + prim[1] * 4;
            tag.t.code &= ~2;
            code = tag.t.code &= ~2;
            tag.t.code = code;
            switch (tag.t.code & 0xfd) {
            case 0x20: /* flat triangle: three vertices, one normal */
                body[3] <<= 3;
                body[4] <<= 3;
                body[5] <<= 3;
                body[2] <<= 3;
                break;
            case 0x24: { /* flat textured triangle */
                u16 tpage;

                body[7] <<= 3;
                body[8] <<= 3;
                body[9] <<= 3;
                body[6] <<= 3;
                tpage = body[3] & 0xff9f;
                body[3] = tpage;
                break;
            }
            case 0x28: /* flat quad: four vertices, one normal */
                body[3] <<= 3;
                body[4] <<= 3;
                body[5] <<= 3;
                body[6] <<= 3;
                body[2] <<= 3;
                break;
            case 0x2c: { /* flat textured quad */
                u16 tpage;

                body[9] <<= 3;
                body[10] <<= 3;
                body[11] <<= 3;
                body[12] <<= 3;
                body[8] <<= 3;
                tpage = body[3] & 0xff9f;
                body[3] = tpage;
                break;
            }
            case 0x30: /* gouraud triangle: three vertices, three normals */
                body[3] <<= 3;
                body[5] <<= 3;
                body[7] <<= 3;
                body[2] <<= 3;
                body[4] <<= 3;
                body[6] <<= 3;
                break;
            case 0x34: { /* gouraud textured triangle */
                u16 tpage;

                body[7] <<= 3;
                body[9] <<= 3;
                body[11] <<= 3;
                body[6] <<= 3;
                body[8] <<= 3;
                body[10] <<= 3;
                tpage = body[3] & 0xff9f;
                body[3] = tpage;
                break;
            }
            case 0x38: /* gouraud quad: four vertices, four normals */
                body[3] <<= 3;
                body[5] <<= 3;
                body[7] <<= 3;
                body[9] <<= 3;
                body[2] <<= 3;
                body[4] <<= 3;
                body[6] <<= 3;
                body[8] <<= 3;
                break;
            case 0x3c: { /* gouraud textured quad */
                u16 tpage;

                body[9] <<= 3;
                body[11] <<= 3;
                body[13] <<= 3;
                body[15] <<= 3;
                body[8] <<= 3;
                body[10] <<= 3;
                body[12] <<= 3;
                body[14] <<= 3;
                tpage = body[3] & 0xff9f;
                body[3] = tpage;
                break;
            }
            }
        }
        part++;
    }
}

/* Registers model data id (0..3) and fixes up its primitives. */
ADDRESS(0x80035c44, 0x40)
void tmd_register(u16 id, u8 *data)
{
    scene_display_sys.current_tmd = scene_display_sys.tmd_slots[id] = data;
    tmd_prepare_primitive_indices(data);
}

/* Registers model data id without the fix-up. */
ADDRESS(0x80035c84, 0x20)
void tmd_set_slot(u16 id, u8 *data)
{
    scene_display_sys.tmd_slots[id] = data;
}

/*
 * Draws part id of the model in SCRATCH->model from the projected vertices in
 * SCRATCH->screen: back faces (gte_nclip) dropped, each face lit from its
 * normals (normal colour depth-cued by the vertex fog values) and linked at
 * its average depth plus depth. Returns early when the packet pool is full.
 */
ADDRESS(0x80035ca4, 0xa04)
void render_enqueue_textured_tmd(u16 id, s32 depth)
{
    s32 n;
    ScreenVertex *v0;
    ScreenVertex *v1;
    ScreenVertex *v2;
    ScreenVertex *v3;
    u8 *p;
    ScratchPad *scratch;
    s32 *opz;
    CVECTOR *rgb;

    scratch = SCRATCH;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    SCRATCH->prim = (u32 *)(SCRATCH->part->prims + 12 + SCRATCH->model);
    n = SCRATCH->part->prim_count;
    SCRATCH->draw.count.tmd_prim_count += n;
    while (n-- != 0) {
        opz = &SCRATCH->w.model.opz;
        rgb = &SCRATCH->w.model.rgb;
        *(u32 *)&scratch->tag = *scratch->prim;
        scratch->prim++;
        switch (scratch->tag.code & 0xfd) {
        case 0x24: {
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[8] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            p = scratch->next;
            scratch->next += sizeof(POLY_FT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_FT3 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_FT3 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_FT3 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_FT3 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_FT3 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u16 *)&((POLY_FT3 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_FT3 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_FT3 *)p)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((v0->fog + v1->fog + v2->fog) / 3);
            gte_ncds();
            gte_strgb(&((POLY_FT3 *)p)->r0);
            setlen(p, 7);
            setcode(p, scratch->tag.code);
            LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z) / 3, depth, p);
            break;
        }
        case 0x2c: {
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            v3 = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            p = scratch->next;
            scratch->next += sizeof(POLY_FT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_FT4 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_FT4 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_FT4 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_FT4 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_FT4 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u32 *)&((POLY_FT4 *)p)->x3 = *(u32 *)&v3->sxy;
            *(u16 *)&((POLY_FT4 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_FT4 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_FT4 *)p)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_FT4 *)p)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((v0->fog + v1->fog + v2->fog + v3->fog) >> 2);
            gte_ncds();
            gte_strgb(&((POLY_FT4 *)p)->r0);
            setlen(p, 9);
            setcode(p, scratch->tag.code);
            LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z + v3->z) >> 2, depth, p);
            break;
        }
        case 0x34: {
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            p = scratch->next;
            scratch->next += sizeof(POLY_GT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT3 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT3 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_GT3 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_GT3 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_GT3 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u16 *)&((POLY_GT3 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT3 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT3 *)p)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv3(((u16 *)scratch->prim)[6] + scratch->normals,
                     ((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)v0->fog);
            gte_ncdt();
            gte_strgb3(&((POLY_GT3 *)p)->r0, &((POLY_GT3 *)p)->r1, &((POLY_GT3 *)p)->r2);
            setlen(p, 9);
            setcode(p, scratch->tag.code);
            LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z) / 3, depth, p);
            break;
        }
        case 0x3c: {
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[13] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            v3 = (ScreenVertex *)(((u16 *)scratch->prim)[15] + (u8 *)scratch->screen);
            p = scratch->next;
            scratch->next += sizeof(POLY_GT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT4 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT4 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_GT4 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x3 = *(u32 *)&v3->sxy;
            *(u16 *)&((POLY_GT4 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT4 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT4 *)p)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_GT4 *)p)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv3(((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals,
                     ((u16 *)scratch->prim)[12] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)v0->fog);
            gte_ncdt();
            gte_strgb3(&((POLY_GT4 *)p)->r0, &((POLY_GT4 *)p)->r1, &((POLY_GT4 *)p)->r2);
            gte_ldv0(((u16 *)scratch->prim)[14] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)v0->fog);
            gte_ncds();
            gte_strgb(&((POLY_GT4 *)p)->r3);
            setlen(p, 12);
            setcode(p, scratch->tag.code);
            LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z + v3->z) >> 2, depth, p);
            break;
        }
        }
        scratch->prim += scratch->tag.len;
    }
}

/*
 * Draws part id of the model in SCRATCH->model near the eye: projects its
 * vertices with a fog value from GraphicsRuntime's fog range, then hands each
 * front face to the LIBGTE subdivision (RCpolyFT3/FT4/GT3/GT4), more finely
 * the nearer it is. Faces with every vertex nearer than draw.count.clip are
 * dropped.
 */
ADDRESS(0x800366a8, 0x1544)
void render_enqueue_tmd_subdivided(u16 id)
{
    s32 n;
    ScratchPad *scratch;
    CVECTOR *rgb;
    union {
        DIVPOLYGON3 d3;
        DIVPOLYGON4 d4;
    } div;

    scratch = SCRATCH;
    SCRATCH->screen_cursor = SCRATCH->screen;
    SCRATCH->vertex = SCRATCH->vertices;
    SCRATCH->w.model.near = scene_display_sys.fog_near;
    SCRATCH->w.model.far = scene_display_sys.fog_far;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    n = SCRATCH->part->vertex_count;
    while (n-- != 0) {
        gte_ldv0(scratch->vertex);
        gte_rtps();
        gte_stsxy(&scratch->screen_cursor->sxy);
        gte_stszotz(&scratch->screen_cursor->z);
        if (scratch->w.model.near < 32000) {
            scratch->screen_cursor->fog = ((scratch->screen_cursor->z - (scratch->w.model.near >> 2)) << 14) /
                                  (scratch->w.model.far - scratch->w.model.near);
        } else {
            scratch->screen_cursor->fog = 0;
        }
        if (scratch->screen_cursor->fog >= 8192) {
            scratch->screen_cursor->fog = 0x1fff;
        }
        if (scratch->screen_cursor->fog < 0) {
            scratch->screen_cursor->fog = 0;
        }
        scratch->screen_cursor++;
        scratch->vertex += 8;
    }
    scratch->prim = (u32 *)(scratch->part->prims + 12 + scratch->model);
    n = scratch->part->prim_count;
    scratch->draw.count.tmd_prim_count += n;
    while (n-- != 0) {
        rgb = &scratch->w.model.rgb;
        *(u32 *)&scratch->tag = *scratch->prim;
        scratch->prim++;
        switch (scratch->tag.code & 0xfd) {
        case 0x24:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[8] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->w.model.opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->draw.count.clip &&
                scratch->vert[1]->z < scratch->draw.count.clip &&
                scratch->vert[2]->z < scratch->draw.count.clip) {
                break;
            }
            scratch->draw.count.z = (scratch->vert[0]->z + scratch->vert[1]->z +
                                     scratch->vert[2]->z) / 3;
            div.d3.ndiv = scratch->draw.count.z < 1024 ? ((1024 - scratch->draw.count.z) >> 9) + 1 : 1;
            div.d3.pih = 320;
            div.d3.piv = 240;
            div.d3.clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            div.d3.tpage = ((u16 *)scratch->prim)[3];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((scratch->vert[0]->fog + scratch->vert[1]->fog + scratch->vert[2]->fog) / 3);
            gte_ncds();
            gte_strgb(&div.d3.rgbc);
            div.d3.cr[0].r0 = &div.d3.r0;
            div.d3.cr[0].r1 = &div.d3.r1;
            div.d3.cr[0].r2 = &div.d3.r2;
            div.d3.rgbc.cd = scratch->tag.code | 0x24;
            div.d3.ot = scratch->ot + scratch->draw.count.z;
            div.d3.r0.sxy = scratch->vert[0]->sxy;
            div.d3.r1.sxy = scratch->vert[1]->sxy;
            div.d3.r2.sxy = scratch->vert[2]->sxy;
            div.d3.r0.sz = scratch->vert[0]->z;
            div.d3.r1.sz = scratch->vert[1]->z;
            div.d3.r2.sz = scratch->vert[2]->z;
            div.d3.r0.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[7]);
            div.d3.r1.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[8]);
            div.d3.r2.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[9]);
            ((u32 *)&div.d3.r0)[2] = scratch->prim[0];
            ((u32 *)&div.d3.r1)[2] = scratch->prim[1];
            ((u32 *)&div.d3.r2)[2] = scratch->prim[2];
            scratch->next = (u8 *)RCpolyFT3((POLY_FT3 *)scratch->next, &div.d3);
            scratch->draw.count.ot_count += div.d3.ndiv ^ 2;
            if (scratch->next > scratch->end) {
                return;
            }
            break;
        case 0x2c:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->nclip[0]);
            gte_ldsxy3(*(u32 *)&scratch->vert[3]->sxy, *(u32 *)&scratch->vert[2]->sxy,
                       *(u32 *)&scratch->vert[1]->sxy);
            gte_nclip();
            gte_stopz(&scratch->nclip[1]);
            if (scratch->nclip[0] > 0 || scratch->nclip[1] > 0) {
                scratch->w.model.opz = 1;
            } else {
                scratch->w.model.opz = -1;
            }
            if (scratch->w.model.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->draw.count.clip &&
                scratch->vert[1]->z < scratch->draw.count.clip &&
                scratch->vert[2]->z < scratch->draw.count.clip &&
                scratch->vert[3]->z < scratch->draw.count.clip) {
                break;
            }
            scratch->draw.count.z = (scratch->vert[0]->z + scratch->vert[1]->z +
                                     scratch->vert[2]->z + scratch->vert[3]->z) >> 2;
            div.d4.ndiv = scratch->draw.count.z < 1024 ? ((1024 - scratch->draw.count.z) >> 9) + 1 : 1;
            div.d4.pih = 320;
            div.d4.piv = 240;
            div.d4.clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            div.d4.tpage = ((u16 *)scratch->prim)[3];
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((scratch->vert[0]->fog + scratch->vert[1]->fog + scratch->vert[2]->fog +
                      scratch->vert[3]->fog) / 4);
            gte_ncds();
            gte_strgb(&div.d4.rgbc);
            div.d4.cr[0].r0 = &div.d4.r0;
            div.d4.cr[0].r1 = &div.d4.r1;
            div.d4.cr[0].r2 = &div.d4.r2;
            div.d4.cr[0].r3 = &div.d4.r3;
            div.d4.rgbc.cd = scratch->tag.code | 0x2c;
            div.d4.ot = scratch->ot + scratch->draw.count.z;
            div.d4.r0.sxy = scratch->vert[0]->sxy;
            div.d4.r1.sxy = scratch->vert[1]->sxy;
            div.d4.r2.sxy = scratch->vert[2]->sxy;
            div.d4.r3.sxy = scratch->vert[3]->sxy;
            div.d4.r0.sz = scratch->vert[0]->z;
            div.d4.r1.sz = scratch->vert[1]->z;
            div.d4.r2.sz = scratch->vert[2]->z;
            div.d4.r3.sz = scratch->vert[3]->z;
            div.d4.r0.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[9]);
            div.d4.r1.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[10]);
            div.d4.r2.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[11]);
            div.d4.r3.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[12]);
            ((u32 *)&div.d4.r0)[2] = scratch->prim[0];
            ((u32 *)&div.d4.r1)[2] = scratch->prim[1];
            ((u32 *)&div.d4.r2)[2] = scratch->prim[2];
            ((u32 *)&div.d4.r3)[2] = scratch->prim[3];
            scratch->next = (u8 *)RCpolyFT4((POLY_FT4 *)scratch->next, &div.d4);
            scratch->draw.count.ot_count += div.d4.ndiv ^ 2;
            if (scratch->next > scratch->end) {
                return;
            }
            break;
        case 0x34:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->w.model.opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->draw.count.clip &&
                scratch->vert[1]->z < scratch->draw.count.clip &&
                scratch->vert[2]->z < scratch->draw.count.clip) {
                break;
            }
            scratch->draw.count.z = (scratch->vert[0]->z + scratch->vert[1]->z +
                                     scratch->vert[2]->z) / 3;
            div.d3.ndiv = scratch->draw.count.z < 1024 ? ((1024 - scratch->draw.count.z) >> 9) + 1 : 1;
            div.d3.pih = 320;
            div.d3.piv = 240;
            div.d3.clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            div.d3.tpage = ((u16 *)scratch->prim)[3];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_ncds();
            gte_strgb(&div.d3.r0.c);
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[1]->fog);
            gte_ncds();
            gte_strgb(&div.d3.r1.c);
            gte_ldv0(((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[2]->fog);
            gte_ncds();
            gte_strgb(&div.d3.r2.c);
            div.d3.rgbc = div.d3.r0.c;
            div.d3.cr[0].r0 = &div.d3.r0;
            div.d3.cr[0].r1 = &div.d3.r1;
            div.d3.cr[0].r2 = &div.d3.r2;
            div.d3.rgbc.cd = scratch->tag.code | 0x34;
            div.d3.ot = scratch->ot + scratch->draw.count.z;
            div.d3.r0.sxy = scratch->vert[0]->sxy;
            div.d3.r1.sxy = scratch->vert[1]->sxy;
            div.d3.r2.sxy = scratch->vert[2]->sxy;
            div.d3.r0.sz = scratch->vert[0]->z;
            div.d3.r1.sz = scratch->vert[1]->z;
            div.d3.r2.sz = scratch->vert[2]->z;
            div.d3.r0.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[7]);
            div.d3.r1.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[9]);
            div.d3.r2.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[11]);
            ((u32 *)&div.d3.r0)[2] = scratch->prim[0];
            ((u32 *)&div.d3.r1)[2] = scratch->prim[1];
            ((u32 *)&div.d3.r2)[2] = scratch->prim[2];
            scratch->next = (u8 *)RCpolyGT3((POLY_GT3 *)scratch->next, &div.d3);
            scratch->draw.count.ot_count += div.d3.ndiv ^ 2;
            if (scratch->next > scratch->end) {
                return;
            }
            break;
        case 0x3c:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[13] + (u8 *)scratch->screen);
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[15] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->nclip[0]);
            gte_ldsxy3(*(u32 *)&scratch->vert[3]->sxy, *(u32 *)&scratch->vert[2]->sxy,
                       *(u32 *)&scratch->vert[1]->sxy);
            gte_nclip();
            gte_stopz(&scratch->nclip[1]);
            if (scratch->nclip[0] > 0 || scratch->nclip[1] > 0) {
                scratch->w.model.opz = 1;
            } else {
                scratch->w.model.opz = -1;
            }
            if (scratch->w.model.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->draw.count.clip &&
                scratch->vert[1]->z < scratch->draw.count.clip &&
                scratch->vert[2]->z < scratch->draw.count.clip &&
                scratch->vert[3]->z < scratch->draw.count.clip) {
                break;
            }
            scratch->draw.count.z = (scratch->vert[0]->z + scratch->vert[1]->z +
                                     scratch->vert[2]->z) / 3;
            div.d4.ndiv = scratch->draw.count.z < 1024 ? ((1024 - scratch->draw.count.z) >> 9) + 1 : 1;
            div.d4.pih = 320;
            div.d4.piv = 240;
            div.d4.clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            div.d4.tpage = ((u16 *)scratch->prim)[3];
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_ncds();
            gte_strgb(&div.d4.r0.c);
            gte_ldv0(((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[1]->fog);
            gte_ncds();
            gte_strgb(&div.d4.r1.c);
            gte_ldv0(((u16 *)scratch->prim)[12] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[2]->fog);
            gte_ncds();
            gte_strgb(&div.d4.r2.c);
            gte_ldv0(((u16 *)scratch->prim)[14] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[3]->fog);
            gte_ncds();
            gte_strgb(&div.d4.r3.c);
            div.d4.rgbc = div.d4.r0.c;
            div.d4.cr[0].r0 = &div.d4.r0;
            div.d4.cr[0].r1 = &div.d4.r1;
            div.d4.cr[0].r2 = &div.d4.r2;
            div.d4.cr[0].r3 = &div.d4.r3;
            div.d4.rgbc.cd = scratch->tag.code | 0x3c;
            div.d4.ot = scratch->ot + scratch->draw.count.z;
            div.d4.r0.sxy = scratch->vert[0]->sxy;
            div.d4.r1.sxy = scratch->vert[1]->sxy;
            div.d4.r2.sxy = scratch->vert[2]->sxy;
            div.d4.r3.sxy = scratch->vert[3]->sxy;
            div.d4.r0.sz = scratch->vert[0]->z;
            div.d4.r1.sz = scratch->vert[1]->z;
            div.d4.r2.sz = scratch->vert[2]->z;
            div.d4.r3.sz = scratch->vert[3]->z;
            div.d4.r0.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[9]);
            div.d4.r1.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[11]);
            div.d4.r2.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[13]);
            div.d4.r3.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[15]);
            ((u32 *)&div.d4.r0)[2] = scratch->prim[0];
            ((u32 *)&div.d4.r1)[2] = scratch->prim[1];
            ((u32 *)&div.d4.r2)[2] = scratch->prim[2];
            ((u32 *)&div.d4.r3)[2] = scratch->prim[3];
            scratch->next = (u8 *)RCpolyGT4((POLY_GT4 *)scratch->next, &div.d4);
            scratch->draw.count.ot_count += div.d4.ndiv ^ 2;
            if (scratch->next > scratch->end) {
                return;
            }
            break;
        }
        scratch->prim += scratch->tag.len;
    }
}

/*
 * Draws part id of the model in SCRATCH->model like render_enqueue_textured_tmd with every
 * face semi-transparent at blend rate mode (tpage bits 5-6); the packet and
 * its vertices are kept in the scratchpad while a face is filled.
 */
ADDRESS(0x80037bec, 0xc58)
void render_enqueue_blended_tmd(u16 id, s32 depth, s32 mode)
{
    s32 n;
    ScratchPad *scratch;
    s32 *opz;
    CVECTOR *rgb;

    mode <<= 5;
    scratch = SCRATCH;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    SCRATCH->prim = (u32 *)(SCRATCH->part->prims + 12 + SCRATCH->model);
    n = SCRATCH->part->prim_count;
    SCRATCH->draw.count.tmd_prim_count += n;
    while (n-- != 0) {
        opz = &SCRATCH->w.model.opz;
        rgb = &SCRATCH->w.model.rgb;
        *(u32 *)&scratch->tag = *scratch->prim;
        scratch->prim++;
        switch (scratch->tag.code & 0xfd) {
        case 0x24:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[8] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            scratch->poly = scratch->next;
            scratch->next += sizeof(POLY_FT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_FT3 *)scratch->poly)->clut =
                ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_FT3 *)scratch->poly)->tpage = mode | (((u16 *)scratch->prim)[3] & 0xff9f);
            *(u32 *)&((POLY_FT3 *)scratch->poly)->x0 = *(u32 *)&scratch->vert[0]->sxy;
            *(u32 *)&((POLY_FT3 *)scratch->poly)->x1 = *(u32 *)&scratch->vert[1]->sxy;
            *(u32 *)&((POLY_FT3 *)scratch->poly)->x2 = *(u32 *)&scratch->vert[2]->sxy;
            *(u16 *)&((POLY_FT3 *)scratch->poly)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_FT3 *)scratch->poly)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_FT3 *)scratch->poly)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((scratch->vert[0]->fog + scratch->vert[1]->fog + scratch->vert[2]->fog) / 3);
            gte_ncds();
            gte_strgb(&((POLY_FT3 *)scratch->poly)->r0);
            setlen(scratch->poly, 7);
            setcode(scratch->poly, 0x26);
            LINK_PART_PRIM(scratch,
                (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z) / 3,
                depth, scratch->poly);
            break;
        case 0x2c:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            scratch->poly = scratch->next;
            scratch->next += sizeof(POLY_FT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_FT4 *)scratch->poly)->clut =
                ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_FT4 *)scratch->poly)->tpage = mode | (((u16 *)scratch->prim)[3] & 0xff9f);
            *(u32 *)&((POLY_FT4 *)scratch->poly)->x0 = *(u32 *)&scratch->vert[0]->sxy;
            *(u32 *)&((POLY_FT4 *)scratch->poly)->x1 = *(u32 *)&scratch->vert[1]->sxy;
            *(u32 *)&((POLY_FT4 *)scratch->poly)->x2 = *(u32 *)&scratch->vert[2]->sxy;
            *(u32 *)&((POLY_FT4 *)scratch->poly)->x3 = *(u32 *)&scratch->vert[3]->sxy;
            *(u16 *)&((POLY_FT4 *)scratch->poly)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_FT4 *)scratch->poly)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_FT4 *)scratch->poly)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_FT4 *)scratch->poly)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((scratch->vert[0]->fog + scratch->vert[1]->fog + scratch->vert[2]->fog +
                       scratch->vert[3]->fog) >> 2);
            gte_ncds();
            gte_strgb(&((POLY_FT4 *)scratch->poly)->r0);
            setlen(scratch->poly, 9);
            setcode(scratch->poly, 0x2e);
            LINK_PART_PRIM(scratch,
                (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z +
                 scratch->vert[3]->z) >> 2,
                depth, scratch->poly);
            break;
        case 0x34:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            scratch->poly = scratch->next;
            scratch->next += sizeof(POLY_GT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT3 *)scratch->poly)->clut =
                ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT3 *)scratch->poly)->tpage = mode | (((u16 *)scratch->prim)[3] & 0xff9f);
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x0 = *(u32 *)&scratch->vert[0]->sxy;
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x1 = *(u32 *)&scratch->vert[1]->sxy;
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x2 = *(u32 *)&scratch->vert[2]->sxy;
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv3(((u16 *)scratch->prim)[6] + scratch->normals,
                     ((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_ncdt();
            gte_strgb3(&((POLY_GT3 *)scratch->poly)->r0, &((POLY_GT3 *)scratch->poly)->r1,
                       &((POLY_GT3 *)scratch->poly)->r2);
            setlen(scratch->poly, 9);
            setcode(scratch->poly, 0x36);
            LINK_PART_PRIM(scratch,
                (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z) / 3,
                depth, scratch->poly);
            break;
        case 0x3c:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[13] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[15] + (u8 *)scratch->screen);
            scratch->poly = scratch->next;
            scratch->next += sizeof(POLY_GT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT4 *)scratch->poly)->clut =
                ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT4 *)scratch->poly)->tpage = mode | (((u16 *)scratch->prim)[3] & 0xff9f);
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x0 = *(u32 *)&scratch->vert[0]->sxy;
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x1 = *(u32 *)&scratch->vert[1]->sxy;
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x2 = *(u32 *)&scratch->vert[2]->sxy;
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x3 = *(u32 *)&scratch->vert[3]->sxy;
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv3(((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals,
                     ((u16 *)scratch->prim)[12] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_ncdt();
            gte_strgb3(&((POLY_GT4 *)scratch->poly)->r0, &((POLY_GT4 *)scratch->poly)->r1,
                       &((POLY_GT4 *)scratch->poly)->r2);
            gte_ldv0(((u16 *)scratch->prim)[14] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_ncds();
            gte_strgb(&((POLY_GT4 *)scratch->poly)->r3);
            setlen(scratch->poly, 12);
            setcode(scratch->poly, 0x3e);
            LINK_PART_PRIM(scratch,
                (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z +
                 scratch->vert[3]->z) >> 2,
                depth, scratch->poly);
            break;
        }
        scratch->prim += scratch->tag.len;
    }
}

/*
 * Draws part id of the model in SCRATCH->model like render_enqueue_textured_tmd for the
 * large models of render_distant_world_model: the depth-cue values are scaled down by
 * 3 - shift, and with shift set the faces go into the eight SCRATCH->ot2
 * entries by depth / 2048 instead of SCRATCH->ot. dist is unused.
 */
ADDRESS(0x80038844, 0xbe4)
void render_enqueue_distant_tmd(u16 id, s32 depth, s32 shift, s32 dist)
{
    s32 n;
    ScreenVertex *v0;
    ScreenVertex *v1;
    ScreenVertex *v2;
    ScreenVertex *v3;
    u8 *p;
    ScratchPad *scratch;
    s32 *opz;
    CVECTOR *rgb;

    scratch = SCRATCH;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    SCRATCH->prim = (u32 *)(SCRATCH->part->prims + 12 + SCRATCH->model);
    n = SCRATCH->part->prim_count;
    SCRATCH->draw.count.tmd_prim_count += n;
    while (n-- != 0) {
        opz = &SCRATCH->w.model.opz;
        rgb = &SCRATCH->w.model.rgb;
        *(u32 *)&scratch->tag = *scratch->prim;
        scratch->prim++;
        switch (scratch->tag.code & 0xfd) {
        case 0x24:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[8] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            p = scratch->next;
            scratch->next += sizeof(POLY_FT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_FT3 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_FT3 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_FT3 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_FT3 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_FT3 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u16 *)&((POLY_FT3 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_FT3 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_FT3 *)p)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp(((v0->fog + v1->fog + v2->fog) / 3) >> (3 - shift));
            gte_ncds();
            gte_strgb(&((POLY_FT3 *)p)->r0);
            setlen(p, 7);
            setcode(p, scratch->tag.code);
            if (shift != 0) {
                LINK_PART_BIN(scratch, ((v0->z + v1->z + v2->z) / 3) >> 11, depth, p);
            } else {
                LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z) / 3, depth, p);
            }
            break;
        case 0x2c:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            v3 = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            p = scratch->next;
            scratch->next += sizeof(POLY_FT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_FT4 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_FT4 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_FT4 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_FT4 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_FT4 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u32 *)&((POLY_FT4 *)p)->x3 = *(u32 *)&v3->sxy;
            *(u16 *)&((POLY_FT4 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_FT4 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_FT4 *)p)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_FT4 *)p)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp(((v0->fog + v1->fog + v2->fog + v3->fog) >> 2) >> (3 - shift));
            gte_ncds();
            gte_strgb(&((POLY_FT4 *)p)->r0);
            setlen(p, 9);
            setcode(p, scratch->tag.code);
            if (shift != 0) {
                LINK_PART_BIN(scratch, ((v0->z + v1->z + v2->z + v3->z) >> 2) >> 11, depth, p);
            } else {
                LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z + v3->z) >> 2, depth, p);
            }
            break;
        case 0x34:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            p = scratch->next;
            scratch->next += sizeof(POLY_GT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT3 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT3 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_GT3 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_GT3 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_GT3 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u16 *)&((POLY_GT3 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT3 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT3 *)p)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv3(((u16 *)scratch->prim)[6] + scratch->normals,
                     ((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp(v0->fog >> (3 - shift));
            gte_ncdt();
            gte_strgb3(&((POLY_GT3 *)p)->r0, &((POLY_GT3 *)p)->r1, &((POLY_GT3 *)p)->r2);
            setlen(p, 9);
            setcode(p, scratch->tag.code);
            if (shift != 0) {
                LINK_PART_BIN(scratch, ((v0->z + v1->z + v2->z) / 3) >> 11, depth, p);
            } else {
                LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z) / 3, depth, p);
            }
            break;
        case 0x3c:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[13] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            v3 = (ScreenVertex *)(((u16 *)scratch->prim)[15] + (u8 *)scratch->screen);
            p = scratch->next;
            scratch->next += sizeof(POLY_GT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT4 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT4 *)p)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_GT4 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x3 = *(u32 *)&v3->sxy;
            *(u16 *)&((POLY_GT4 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT4 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT4 *)p)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_GT4 *)p)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv3(((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals,
                     ((u16 *)scratch->prim)[12] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp(v0->fog >> (3 - shift));
            gte_ncdt();
            gte_strgb3(&((POLY_GT4 *)p)->r0, &((POLY_GT4 *)p)->r1, &((POLY_GT4 *)p)->r2);
            gte_ldv0(((u16 *)scratch->prim)[14] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp(v0->fog >> (3 - shift));
            gte_ncds();
            gte_strgb(&((POLY_GT4 *)p)->r3);
            setlen(p, 12);
            setcode(p, scratch->tag.code);
            if (shift != 0) {
                LINK_PART_BIN(scratch, ((v0->z + v1->z + v2->z + v3->z) >> 2) >> 11, depth, p);
            } else {
                LINK_PART_PRIM(scratch, (v0->z + v1->z + v2->z + v3->z) >> 2, depth, p);
            }
            break;
        }
        scratch->prim += scratch->tag.len;
    }
}

/*
 * Draws part id of the model in SCRATCH->model as Gouraud-shaded faces lit
 * with gte_ncct/gte_nccs: textured faces take their colour from SCRATCH->w.model.rgb
 * and the blend rate shade in their tpage, plain ones the colour at the head of
 * the record. Front faces only (gte_nclip); each face is linked into
 * SCRATCH->ot2 at depth.
 */
ADDRESS(0x80039428, 0x928)
void render_enqueue_tmd_fixed_depth(u16 id, s16 depth, s32 shade)
{
    s32 n;
    ScreenVertex *v0;
    ScreenVertex *v1;
    ScreenVertex *v2;
    ScreenVertex *v3;
    u8 *p;
    ScratchPad *scratch;
    s32 *opz;
    CVECTOR *rgb;

    scratch = SCRATCH;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    SCRATCH->prim = (u32 *)(SCRATCH->part->prims + 12 + SCRATCH->model);
    n = SCRATCH->part->prim_count;
    SCRATCH->draw.count.tmd_prim_count += n;
    shade <<= 5;
    while (n-- != 0) {
        opz = &SCRATCH->w.model.opz;
        rgb = &SCRATCH->w.model.rgb;
        *(u32 *)&scratch->tag = *scratch->prim;
        scratch->prim++;
        switch (scratch->tag.code & 0xfd) {
        case 0x34:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            p = scratch->next;
            scratch->next += sizeof(POLY_GT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT3 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT3 *)p)->tpage = (((u16 *)scratch->prim)[3] & 0xff9f) | shade;
            *(u32 *)&((POLY_GT3 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_GT3 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_GT3 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u16 *)&((POLY_GT3 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT3 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT3 *)p)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv3(((u16 *)scratch->prim)[6] + scratch->normals,
                     ((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_ncct();
            gte_strgb3(&((POLY_GT3 *)p)->r0, &((POLY_GT3 *)p)->r1, &((POLY_GT3 *)p)->r2);
            setlen(p, 9);
            setcode(p, (scratch->tag.code & 2) | 0x34);
            LINK_PART_BIN(scratch, depth, 0, p);
            break;
        case 0x3c:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[13] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            v3 = (ScreenVertex *)(((u16 *)scratch->prim)[15] + (u8 *)scratch->screen);
            p = scratch->next;
            scratch->next += sizeof(POLY_GT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            ((POLY_GT4 *)p)->clut = ((u16 *)scratch->prim)[1] + scratch->draw.count.clut;
            ((POLY_GT4 *)p)->tpage = (((u16 *)scratch->prim)[3] & 0xff9f) | shade;
            *(u32 *)&((POLY_GT4 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u32 *)&((POLY_GT4 *)p)->x3 = *(u32 *)&v3->sxy;
            *(u16 *)&((POLY_GT4 *)p)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT4 *)p)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT4 *)p)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_GT4 *)p)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv3(((u16 *)scratch->prim)[8] + scratch->normals,
                     ((u16 *)scratch->prim)[10] + scratch->normals,
                     ((u16 *)scratch->prim)[12] + scratch->normals);
            gte_ldrgb(rgb);
            gte_ncct();
            gte_strgb3(&((POLY_GT4 *)p)->r0, &((POLY_GT4 *)p)->r1, &((POLY_GT4 *)p)->r2);
            gte_ldv0(((u16 *)scratch->prim)[14] + scratch->normals);
            gte_ldrgb(rgb);
            gte_nccs();
            gte_strgb(&((POLY_GT4 *)p)->r3);
            setlen(p, 12);
            setcode(p, (scratch->tag.code & 2) | 0x3c);
            LINK_PART_BIN(scratch, depth, 0, p);
            break;
        case 0x30:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[3] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[5] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            p = scratch->next;
            scratch->next += sizeof(POLY_G3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            *(u32 *)&((POLY_G3 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_G3 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_G3 *)p)->x2 = *(u32 *)&v2->sxy;
            gte_ldv3(((u16 *)scratch->prim)[2] + scratch->normals,
                     ((u16 *)scratch->prim)[4] + scratch->normals,
                     ((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(scratch->prim);
            gte_ncct();
            gte_strgb3(&((POLY_G3 *)p)->r0, &((POLY_G3 *)p)->r1, &((POLY_G3 *)p)->r2);
            setlen(p, 6);
            setcode(p, 0x30);
            LINK_PART_BIN(scratch, depth, 0, p);
            break;
        case 0x38:
            v0 = (ScreenVertex *)(((u16 *)scratch->prim)[3] + (u8 *)scratch->screen);
            v1 = (ScreenVertex *)(((u16 *)scratch->prim)[5] + (u8 *)scratch->screen);
            v2 = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&v0->sxy, *(u32 *)&v1->sxy, *(u32 *)&v2->sxy);
            gte_nclip();
            gte_stopz(opz);
            if (scratch->w.model.opz <= 0) {
                break;
            }
            v3 = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            p = scratch->next;
            scratch->next += sizeof(POLY_G4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->draw.count.prim_count++;
            *(u32 *)&((POLY_G4 *)p)->x0 = *(u32 *)&v0->sxy;
            *(u32 *)&((POLY_G4 *)p)->x1 = *(u32 *)&v1->sxy;
            *(u32 *)&((POLY_G4 *)p)->x2 = *(u32 *)&v2->sxy;
            *(u32 *)&((POLY_G4 *)p)->x3 = *(u32 *)&v3->sxy;
            gte_ldv3(((u16 *)scratch->prim)[2] + scratch->normals,
                     ((u16 *)scratch->prim)[4] + scratch->normals,
                     ((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(scratch->prim);
            gte_ncct();
            gte_strgb3(&((POLY_G4 *)p)->r0, &((POLY_G4 *)p)->r1, &((POLY_G4 *)p)->r2);
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(scratch->prim);
            gte_nccs();
            gte_strgb(&((POLY_G4 *)p)->r3);
            setlen(p, 8);
            setcode(p, 0x38);
            LINK_PART_BIN(scratch, depth, 0, p);
            break;
        }
        scratch->prim += scratch->tag.len;
    }
}

/*
 * Draws part id of the map layer model in SCRATCH->model (the far ground
 * renderer of render_map_cell_object): projects its vertices with a fog value from
 * GraphicsRuntime's fog range, then draws its faces as Gouraud-textured packets
 * lit from one normal and depth-cued per vertex. Faces behind (gte_nclip) or
 * with every vertex nearer than w.map.clip are dropped; flat-textured records
 * become GT3/GT4 packets.
 */
ADDRESS(0x80039d50, 0xdb4)
void render_enqueue_map(u16 id)
{
    s32 n;
    ScratchPad *scratch;
    CVECTOR *rgb;
    CVECTOR *lit;

    scratch = SCRATCH;
    SCRATCH->screen_cursor = SCRATCH->screen;
    SCRATCH->vertex = SCRATCH->vertices;
    SCRATCH->w.map.near = scene_display_sys.fog_near;
    SCRATCH->w.map.far = scene_display_sys.fog_far;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    n = SCRATCH->part->vertex_count;
    while (n-- != 0) {
        gte_ldv0(scratch->vertex);
        gte_rtps();
        gte_stsxy(&scratch->screen_cursor->sxy);
        gte_stszotz(&scratch->screen_cursor->z);
        if (scratch->w.map.near < 32000) {
            scratch->screen_cursor->fog = ((scratch->screen_cursor->z - (scratch->w.map.near >> 2)) << 14) /
                                  (scratch->w.map.far - scratch->w.map.near);
        } else {
            scratch->screen_cursor->fog = 0;
        }
        if (scratch->screen_cursor->fog >= 7952) {
            scratch->screen_cursor->fog = 0x1f0f;
        }
        if (scratch->screen_cursor->fog < 0) {
            scratch->screen_cursor->fog = 0;
        }
        scratch->screen_cursor++;
        scratch->vertex += 8;
    }
    scratch->prim = (u32 *)(scratch->part->prims + 12 + scratch->model);
    n = scratch->part->prim_count;
    scratch->w.map.tmd_prim_count += n;
    while (n-- != 0) {
        rgb = &scratch->w.map.rgb;
        lit = &scratch->lit;
        *(u32 *)&scratch->tag = *scratch->prim;
        scratch->prim++;
        switch (scratch->tag.code & 0xfd) {
        case 0x24:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[8] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->w.map.opz);
            if (scratch->w.map.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->w.map.clip &&
                scratch->vert[1]->z < scratch->w.map.clip &&
                scratch->vert[2]->z < scratch->w.map.clip) {
                break;
            }
            scratch->w.map.z = (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z) / 3;
            if (scratch->w.map.z >= 7952) {
                scratch->w.map.z = 0x1f0f;
            }
            scratch->poly = scratch->next;
            scratch->next += sizeof(POLY_GT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->w.map.prim_count++;
            ((POLY_GT3 *)scratch->poly)->clut = ((u16 *)scratch->prim)[1];
            ((POLY_GT3 *)scratch->poly)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x0 = *(u32 *)&scratch->vert[0]->sxy;
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x1 = *(u32 *)&scratch->vert[1]->sxy;
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x2 = *(u32 *)&scratch->vert[2]->sxy;
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_nccs();
            gte_strgb(lit);
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_nccs();
            gte_strgb(lit);
            gte_ldrgb(lit);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_dpcs();
            gte_strgb(&((POLY_GT3 *)scratch->poly)->r0);
            gte_ldrgb(lit);
            gte_lddp((u16)scratch->vert[1]->fog);
            gte_dpcs();
            gte_strgb(&((POLY_GT3 *)scratch->poly)->r1);
            gte_ldrgb(lit);
            gte_lddp((u16)scratch->vert[2]->fog);
            gte_dpcs();
            gte_strgb(&((POLY_GT3 *)scratch->poly)->r2);
            setlen(scratch->poly, 9);
            setcode(scratch->poly, scratch->tag.code | 0x34);
            {
                s32 z = scratch->w.map.z;

                if (z >= 0 && z < 7952) {
                    addPrim(scratch->ot + z, scratch->poly);
                    scratch->w.map.ot_count++;
                }
            }
            break;
        case 0x2c:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->w.map.opz);
            if (scratch->w.map.opz <= 0) {
                break;
            }
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            if (scratch->vert[0]->z < scratch->w.map.clip &&
                scratch->vert[1]->z < scratch->w.map.clip &&
                scratch->vert[2]->z < scratch->w.map.clip &&
                scratch->vert[3]->z < scratch->w.map.clip) {
                break;
            }
            scratch->w.map.z = (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z +
                                scratch->vert[3]->z) >> 2;
            if (scratch->w.map.z >= 7952) {
                scratch->w.map.z = 0x1f0f;
            }
            scratch->poly = scratch->next;
            scratch->next += sizeof(POLY_GT4);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->w.map.prim_count++;
            ((POLY_GT4 *)scratch->poly)->clut = ((u16 *)scratch->prim)[1];
            ((POLY_GT4 *)scratch->poly)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x0 = *(u32 *)&scratch->vert[0]->sxy;
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x1 = *(u32 *)&scratch->vert[1]->sxy;
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x2 = *(u32 *)&scratch->vert[2]->sxy;
            *(u32 *)&((POLY_GT4 *)scratch->poly)->x3 = *(u32 *)&scratch->vert[3]->sxy;
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u2 = ((u16 *)scratch->prim)[4];
            *(u16 *)&((POLY_GT4 *)scratch->poly)->u3 = ((u16 *)scratch->prim)[6];
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_nccs();
            gte_strgb(lit);
            gte_ldrgb(lit);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_dpcs();
            gte_strgb(&((POLY_GT4 *)scratch->poly)->r0);
            gte_ldrgb(lit);
            gte_lddp((u16)scratch->vert[1]->fog);
            gte_dpcs();
            gte_strgb(&((POLY_GT4 *)scratch->poly)->r1);
            gte_ldrgb(lit);
            gte_lddp((u16)scratch->vert[2]->fog);
            gte_dpcs();
            gte_strgb(&((POLY_GT4 *)scratch->poly)->r2);
            gte_ldrgb(lit);
            gte_lddp((u16)scratch->vert[3]->fog);
            gte_dpcs();
            gte_strgb(&((POLY_GT4 *)scratch->poly)->r3);
            setlen(scratch->poly, 12);
            setcode(scratch->poly, scratch->tag.code | 0x3c);
            {
                s32 z = scratch->w.map.z;

                if (z >= 0 && z < 7952) {
                    addPrim(scratch->ot + z, scratch->poly);
                    scratch->w.map.ot_count++;
                }
            }
            break;
        case 0x34:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[8] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->w.map.opz);
            if (scratch->w.map.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->w.map.clip &&
                scratch->vert[1]->z < scratch->w.map.clip &&
                scratch->vert[2]->z < scratch->w.map.clip) {
                break;
            }
            scratch->w.map.z = (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z) / 3;
            if (scratch->w.map.z >= 7952) {
                scratch->w.map.z = 0x1f0f;
            }
            scratch->poly = scratch->next;
            scratch->next += sizeof(POLY_GT3);
            if (scratch->end < scratch->next) {
                return;
            }
            scratch->w.map.prim_count++;
            ((POLY_GT3 *)scratch->poly)->clut = ((u16 *)scratch->prim)[1];
            ((POLY_GT3 *)scratch->poly)->tpage = ((u16 *)scratch->prim)[3];
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x0 = *(u32 *)&scratch->vert[0]->sxy;
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x1 = *(u32 *)&scratch->vert[1]->sxy;
            *(u32 *)&((POLY_GT3 *)scratch->poly)->x2 = *(u32 *)&scratch->vert[2]->sxy;
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u0 = ((u16 *)scratch->prim)[0];
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u1 = ((u16 *)scratch->prim)[2];
            *(u16 *)&((POLY_GT3 *)scratch->poly)->u2 = ((u16 *)scratch->prim)[4];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_ncds();
            gte_strgb(&((POLY_GT3 *)scratch->poly)->r0);
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[1]->fog);
            gte_ncds();
            gte_strgb(&((POLY_GT3 *)scratch->poly)->r1);
            gte_ldv0(((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[2]->fog);
            gte_ncds();
            gte_strgb(&((POLY_GT3 *)scratch->poly)->r2);
            setlen(scratch->poly, 9);
            setcode(scratch->poly, scratch->tag.code);
            {
                s32 z = scratch->w.map.z;

                if (z >= 0 && z < 7952) {
                    addPrim(scratch->ot + z, scratch->poly);
                    scratch->w.map.ot_count++;
                }
            }
            break;
        }
        scratch->prim += scratch->tag.len;
    }
}

/*
 * Draws part id of the map layer model in SCRATCH->model near the eye (the
 * near ground renderer of render_map_cell_object): projects the vertices like
 * render_enqueue_map, then hands each front face to the LIBGTE subdivision
 * (RCpolyFT3/FT4/GT3), more finely the nearer it is.
 */
ADDRESS(0x8003ab04, 0x1000)
void render_enqueue_map_subdivided(u16 id)
{
    s32 n;
    ScratchPad *scratch;
    CVECTOR *rgb;
    union {
        DIVPOLYGON3 d3;
        DIVPOLYGON4 d4;
    } div;

    scratch = SCRATCH;
    SCRATCH->screen_cursor = SCRATCH->screen;
    SCRATCH->vertex = SCRATCH->vertices;
    SCRATCH->w.map.near = scene_display_sys.fog_near;
    SCRATCH->w.map.far = scene_display_sys.fog_far;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    n = SCRATCH->part->vertex_count;
    while (n-- != 0) {
        gte_ldv0(scratch->vertex);
        gte_rtps();
        gte_stsxy(&scratch->screen_cursor->sxy);
        gte_stszotz(&scratch->screen_cursor->z);
        if (scratch->w.map.near < 32000) {
            scratch->screen_cursor->fog = ((scratch->screen_cursor->z - (scratch->w.map.near >> 2)) << 14) /
                                  (scratch->w.map.far - scratch->w.map.near);
        } else {
            scratch->screen_cursor->fog = 0;
        }
        if (scratch->screen_cursor->fog >= 7952) {
            scratch->screen_cursor->fog = 0x1f0f;
        }
        if (scratch->screen_cursor->fog < 0) {
            scratch->screen_cursor->fog = 0;
        }
        scratch->screen_cursor++;
        scratch->vertex += 8;
    }
    scratch->prim = (u32 *)(scratch->part->prims + 12 + scratch->model);
    n = scratch->part->prim_count;
    scratch->w.map.tmd_prim_count += n;
    while (n-- != 0) {
        rgb = &scratch->w.map.rgb;
        *(u32 *)&scratch->tag = *scratch->prim;
        scratch->prim++;
        switch (scratch->tag.code & 0xfd) {
        case 0x24:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[8] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->w.map.opz);
            if (scratch->w.map.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->w.map.clip &&
                scratch->vert[1]->z < scratch->w.map.clip &&
                scratch->vert[2]->z < scratch->w.map.clip) {
                break;
            }
            scratch->w.map.z = (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z) / 3;
            if (scratch->w.map.z >= 7952) {
                scratch->w.map.z = 0x1f0f;
            }
            div.d3.ndiv = scratch->w.map.z < 1024 ? ((1024 - scratch->w.map.z) >> 9) + 1 : 1;
            div.d3.pih = 320;
            div.d3.piv = 240;
            div.d3.clut = ((u16 *)scratch->prim)[1];
            div.d3.tpage = ((u16 *)scratch->prim)[3];
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((scratch->vert[0]->fog + scratch->vert[1]->fog + scratch->vert[2]->fog) / 3);
            gte_ncds();
            gte_strgb(&div.d3.rgbc);
            div.d3.cr[0].r0 = &div.d3.r0;
            div.d3.cr[0].r1 = &div.d3.r1;
            div.d3.cr[0].r2 = &div.d3.r2;
            div.d3.rgbc.cd = scratch->tag.code | 0x24;
            div.d3.ot = scratch->ot + scratch->w.map.z;
            div.d3.r0.sxy = scratch->vert[0]->sxy;
            div.d3.r1.sxy = scratch->vert[1]->sxy;
            div.d3.r2.sxy = scratch->vert[2]->sxy;
            div.d3.r0.sz = scratch->vert[0]->z;
            div.d3.r1.sz = scratch->vert[1]->z;
            div.d3.r2.sz = scratch->vert[2]->z;
            div.d3.r0.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[7]);
            div.d3.r1.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[8]);
            div.d3.r2.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[9]);
            ((u32 *)&div.d3.r0)[2] = scratch->prim[0];
            ((u32 *)&div.d3.r1)[2] = scratch->prim[1];
            ((u32 *)&div.d3.r2)[2] = scratch->prim[2];
            scratch->next = (u8 *)RCpolyFT3((POLY_FT3 *)scratch->next, &div.d3);
            scratch->w.map.ot_count += div.d3.ndiv ^ 2;
            if (scratch->next > scratch->end) {
                return;
            }
            break;
        case 0x2c:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->nclip[0]);
            gte_ldsxy3(*(u32 *)&scratch->vert[3]->sxy, *(u32 *)&scratch->vert[2]->sxy,
                       *(u32 *)&scratch->vert[1]->sxy);
            gte_nclip();
            gte_stopz(&scratch->nclip[1]);
            if (scratch->nclip[0] > 0 || scratch->nclip[1] > 0) {
                scratch->w.map.opz = 1;
            } else {
                scratch->w.map.opz = -1;
            }
            if (scratch->w.map.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->w.map.clip &&
                scratch->vert[1]->z < scratch->w.map.clip &&
                scratch->vert[2]->z < scratch->w.map.clip &&
                scratch->vert[3]->z < scratch->w.map.clip) {
                break;
            }
            scratch->w.map.z = (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z +
                                scratch->vert[3]->z) >> 2;
            if (scratch->w.map.z >= 7952) {
                scratch->w.map.z = 0x1f0f;
            }
            div.d4.ndiv = scratch->w.map.z < 1024 ? ((1024 - scratch->w.map.z) >> 9) + 1 : 1;
            div.d4.pih = 320;
            div.d4.piv = 240;
            div.d4.clut = ((u16 *)scratch->prim)[1];
            div.d4.tpage = ((u16 *)scratch->prim)[3];
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[10] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            scratch->vert[3] = (ScreenVertex *)(((u16 *)scratch->prim)[12] + (u8 *)scratch->screen);
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((scratch->vert[0]->fog + scratch->vert[1]->fog + scratch->vert[2]->fog +
                      scratch->vert[3]->fog) / 4);
            gte_ncds();
            gte_strgb(&div.d4.rgbc);
            div.d4.cr[0].r0 = &div.d4.r0;
            div.d4.cr[0].r1 = &div.d4.r1;
            div.d4.cr[0].r2 = &div.d4.r2;
            div.d4.cr[0].r3 = &div.d4.r3;
            div.d4.rgbc.cd = scratch->tag.code | 0x2c;
            div.d4.ot = scratch->ot + scratch->w.map.z;
            div.d4.r0.sxy = scratch->vert[0]->sxy;
            div.d4.r1.sxy = scratch->vert[1]->sxy;
            div.d4.r2.sxy = scratch->vert[2]->sxy;
            div.d4.r3.sxy = scratch->vert[3]->sxy;
            div.d4.r0.sz = scratch->vert[0]->z;
            div.d4.r1.sz = scratch->vert[1]->z;
            div.d4.r2.sz = scratch->vert[2]->z;
            div.d4.r3.sz = scratch->vert[3]->z;
            div.d4.r0.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[9]);
            div.d4.r1.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[10]);
            div.d4.r2.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[11]);
            div.d4.r3.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[12]);
            ((u32 *)&div.d4.r0)[2] = scratch->prim[0];
            ((u32 *)&div.d4.r1)[2] = scratch->prim[1];
            ((u32 *)&div.d4.r2)[2] = scratch->prim[2];
            ((u32 *)&div.d4.r3)[2] = scratch->prim[3];
            scratch->next = (u8 *)RCpolyFT4((POLY_FT4 *)scratch->next, &div.d4);
            scratch->w.map.ot_count += div.d4.ndiv ^ 2;
            if (scratch->next > scratch->end) {
                return;
            }
            break;
        case 0x34:
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldsxy3(*(u32 *)&scratch->vert[0]->sxy, *(u32 *)&scratch->vert[1]->sxy,
                       *(u32 *)&scratch->vert[2]->sxy);
            gte_nclip();
            gte_stopz(&scratch->w.map.opz);
            if (scratch->w.map.opz <= 0) {
                break;
            }
            if (scratch->vert[0]->z < scratch->w.map.clip &&
                scratch->vert[1]->z < scratch->w.map.clip &&
                scratch->vert[2]->z < scratch->w.map.clip) {
                break;
            }
            scratch->w.map.z = (scratch->vert[0]->z + scratch->vert[1]->z + scratch->vert[2]->z) / 3;
            if (scratch->w.map.z >= 7952) {
                scratch->w.map.z = 0x1f0f;
            }
            div.d3.ndiv = scratch->w.map.z < 1024 ? ((1024 - scratch->w.map.z) >> 9) + 1 : 1;
            div.d3.pih = 320;
            div.d3.piv = 240;
            div.d3.clut = ((u16 *)scratch->prim)[1];
            div.d3.tpage = ((u16 *)scratch->prim)[3];
            scratch->vert[0] = (ScreenVertex *)(((u16 *)scratch->prim)[7] + (u8 *)scratch->screen);
            scratch->vert[1] = (ScreenVertex *)(((u16 *)scratch->prim)[9] + (u8 *)scratch->screen);
            scratch->vert[2] = (ScreenVertex *)(((u16 *)scratch->prim)[11] + (u8 *)scratch->screen);
            gte_ldv0(((u16 *)scratch->prim)[6] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[0]->fog);
            gte_ncds();
            gte_strgb(&div.d3.r0.c);
            gte_ldv0(((u16 *)scratch->prim)[8] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[1]->fog);
            gte_ncds();
            gte_strgb(&div.d3.r1.c);
            gte_ldv0(((u16 *)scratch->prim)[10] + scratch->normals);
            gte_ldrgb(rgb);
            gte_lddp((u16)scratch->vert[2]->fog);
            gte_ncds();
            gte_strgb(&div.d3.r2.c);
            div.d3.cr[0].r0 = &div.d3.r0;
            div.d3.cr[0].r1 = &div.d3.r1;
            div.d3.cr[0].r2 = &div.d3.r2;
            div.d3.rgbc.cd = scratch->tag.code | 0x34;
            div.d3.ot = scratch->ot + scratch->w.map.z;
            div.d3.r0.sxy = scratch->vert[0]->sxy;
            div.d3.r1.sxy = scratch->vert[1]->sxy;
            div.d3.r2.sxy = scratch->vert[2]->sxy;
            div.d3.r0.sz = scratch->vert[0]->z;
            div.d3.r1.sz = scratch->vert[1]->z;
            div.d3.r2.sz = scratch->vert[2]->z;
            div.d3.r0.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[7]);
            div.d3.r1.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[8]);
            div.d3.r2.v = *(SVECTOR *)(scratch->vertices + ((u16 *)scratch->prim)[9]);
            ((u32 *)&div.d3.r0)[2] = scratch->prim[0];
            ((u32 *)&div.d3.r1)[2] = scratch->prim[1];
            ((u32 *)&div.d3.r2)[2] = scratch->prim[2];
            scratch->next = (u8 *)RCpolyGT3((POLY_GT3 *)scratch->next, &div.d3);
            scratch->w.map.ot_count += div.d3.ndiv ^ 2;
            if (scratch->next > scratch->end) {
                return;
            }
            break;
        }
        scratch->prim += scratch->tag.len;
    }
}

/*
 * Draws one map layer at pos (relative to SCRATCH->u.layer.unk108): loads its light
 * record (layer->unk4) turned by layer->unk2, the fog and the view transform,
 * then runs the near (flags bit 2) or far ground renderer on model part
 * layer->unk0.
 */
ADDRESS(0x8003bb04, 0x330)
void render_map_cell_object(MapLayer *layer, SVECTOR *pos, u8 flags)
{
    LightRecord *light;
    s32 turn;
    long flag;
    u16 part;
    MATRIX *transform;
    ScratchPad *scratch;

    transform = &SCRATCH->draw.transform;
    scratch = SCRATCH;
    turn = layer->quarter_turns & 3;
    light = &scene_display_sys.lights[layer->lighting_index & 0x3f];
    gte_SetLightMatrix(&light->light_matrices[turn]);
    gte_SetColorMatrix(&light->color_matrix);
    fog_set_range(light->fog_near, light->fog_far);
    gte_SetBackColor(light->back_color.r, light->back_color.g, light->back_color.b);
    gte_SetRotMatrix(&scene_display_sys.view_matrix);
    gte_SetTransMatrix(&scene_display_sys.view_matrix);
    gte_ldv0(pos);
    gte_rt();
    gte_stlvnl(SCRATCH->draw.transform.t);
    gte_stflg(&flag);
    gte_SetRotMatrix(&scene_display_sys.view_matrix_turns[turn]);
    gte_SetTransMatrix(transform);
    part = layer->object_index;
    if ((s16)asset_status.loading == 1 && asset_status.tmd_object_limit_active != 0 &&
        part >= ((ModelData *)SCRATCH->model)->unk4 >> 1) {
        return;
    }
    if (scratch->end - 10240 < scratch->next) {
        flags &= ~4;
    }
    if (flags & 4) {
        scratch->part = &((ModelData *)scratch->model)->parts[part];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        render_enqueue_map_subdivided(part);
    } else {
        scratch->part = &((ModelData *)scratch->model)->parts[part];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        render_enqueue_map(part);
    }
}

/*
 * Draws the layers of map cell (x, z) that are visible, with flags bit 1 set.
 * The position is built in the scratchpad in King's Field II's order (x, y, z).
 * Nothing in the decompiled images calls it.
 */
ADDRESS(0x8003be34, 0x19c)
void render_map_cell_layers(s32 x, s32 z, u8 flags)
{
    SVECTOR *pos;
    VECTOR *origin;
    MapCell *cell;
    s32 kind;

    pos = &SCRATCH->u.layer.pos;
    origin = &SCRATCH->u.layer.origin;
    cell = &map_world.cells[z][x];
    kind = cell->layer[0].object_index;
    if ((flags & 2) && kind < 240) {
        pos->vx = (x << 11) - origin->vx + 1024;
        pos->vy = (-cell->layer[0].elevation << 7) - origin->vy;
        pos->vz = (z << 11) - origin->vz + 1024;
        render_map_cell_object(&cell->layer[0], &SCRATCH->u.layer.pos, flags);
        kind = cell->layer[1].object_index;
        if (kind < 240) {
            pos->vy = (-cell->layer[1].elevation << 7) - origin->vy;
            render_map_cell_object(&cell->layer[1], &SCRATCH->u.layer.pos, flags);
        }
    } else {
        kind = cell->layer[1].object_index;
        if ((flags & 2) && kind < 240) {
            pos->vx = (x << 11) - origin->vx + 1024;
            pos->vy = (-cell->layer[1].elevation << 7) - origin->vy;
            pos->vz = (z << 11) - origin->vz + 1024;
            render_map_cell_object(&cell->layer[1], pos, flags);
        }
    }
}

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

/*
 * Not exact (one instruction and one delay slot): retail reads the low half
 * of the spilled x for the second quad straight from its stack slot (`sw
 * a1,16(sp)` ... `lhu s1,16(sp)`) and fills the next branch's delay slot
 * with `li a0,2`; this build reloads the whole word (`lw t0,16(sp)`) and puts
 * `move s1,t0` in that slot. GNU C 2.6.0's reload cannot read part of a
 * spilled word. FSF's reload change of 1994-08-07 (in 2.6.1 and later) does,
 * and decompals' gcc 2.6.3-psx compiles this source to retail's code, but
 * 2.6.3 breaks 0x80047010 and 0x800568bc (frame size), so GAME's compiler
 * looks like a Sony 2.6.0 build with that reload fix (docs/toolchain.md).
 */
ADDRESS(0x8003daec, 0x464)
void render_zoom_screen_transition(u8 *rgb, s32 x, s32 y, s32 step)
{
    s32 count;
    u8 r;
    u8 g;
    u8 b;

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
    for (;;) {
        POLY_FT4 *p;

        if (count <= 0) {
            break;
        }
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
        setPolyFT4(p);
        setXYWH(p, 320 - x, 240 - y, x - 128, y * 2 - 240);
        setUVWH(p, 0, 0, 192, 240);
        p->clut = 0;
        p->tpage = GetTPage(2, 0, 320, 0);
        setRGB0(p, r, g, b);
        addPrim(scene_display_sys.ot, p);
        p = (POLY_FT4 *)scene_display_sys.pool->next;
        scene_display_sys.pool->next += sizeof(POLY_FT4);
        if (scene_display_sys.pool->next > scene_display_sys.pool->end) {
            return;
        }
        scene_display_sys.prim_count++;
        setPolyFT4(p);
        setXYWH(p, 192, 240 - y, x - 192, y * 2 - 240);
        setUVWH(p, 0, 0, 128, 240);
        p->clut = 0;
        p->tpage = GetTPage(2, 0, 512, 0);
        setRGB0(p, r, g, b);
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
    if (animation_prepare_asset_vertices(&cur_avatar_status.weapon_animation_cache, 32,
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
        scratch->part = &((ModelData *)scratch->model)->parts[n];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        part = &((ModelData *)scratch->model)->parts[n];
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
        scratch->part = &((ModelData *)scratch->model)->parts[n];
        scratch->vertices = scratch->part->vertices + 12 + scratch->model;
        part = &((ModelData *)scratch->model)->parts[n];
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
u32 map_cell_layer_mask(VECTOR *pos)
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
u32 map_cell_layer_mask_radius(VECTOR *pos, s32 radius)
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
 * and resource_tmd_update_range/resource_vab_update_range load the missing
 * ones and release the rest. The gotos and the scale variable follow King's
 * Field II's function.
 *
 * Not exact, three differences. Retail tests case 0xe5 with a `beq` to the
 * next object and a `j` to the default case; here reorg inverts that pair
 * because the branch is predicted not taken (retail's was not, as if it
 * left an inner loop). Retail loads the constant 1 of the mark stores in
 * each case; here loop.c hoists it, since the object loop has 467 insns at
 * loop time against a limit of 468, so retail's loop was larger at that
 * point. Retail updates map_placed_frame_counter through an address
 * register, the code of a struct member (King's Field II has
 * scene_display_sys.map_placed_frame_counter++); here it is a plain s32.
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
    MapObject *animated;
    MapObjectDef *rec_def;
    Effect *effect;
    MapPlacedEntry *placed;
    VECTOR *at;
    SVECTOR *scale;
    s32 i;
    s32 now;
    s32 near;
    s32 dist;
    s32 volume;
    SoundEffect *sound;
    u16 id;
    u32 mask;
    u32 cells;
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
    for (i = 199; i != -1; obj++, i--) {
        if (obj->lifecycle != 1) {
            continue;
        }
        if (obj->flags & 0x2000) {
            mask = obj->layer_mask | 0x10;
        } else {
            mask = obj->layer_mask;
        }
        if (obj->flags & 0x80000) {
            goto wide;
        }
        if (!(map_cell_layer_mask(&obj->pos) & mask)) {
            continue;
        }
    draw:
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
        continue;
    wide:
        if (map_cell_layer_mask_radius(&obj->pos, 3) & obj->layer_mask) {
            goto draw;
        }
    }
    resource_tmd_update_range(0, 0, 0x80, 0x80);
    resource_vab_update_range(2, 0x91, 0x33, 0x60);
    now = cdrom_ctl.frames;
    repeat_store_word((u32 *)scratch->u.load.models, 0, 0x53);
    repeat_store_word((u32 *)scratch->u.load.sounds, 0, 8);
    if (scene_display_sys.unk15996 == 0) {
        asset_status.animated_object = NULL;
    }
    rec = map_objects_slots.objects;
    scratch->draw.count.clut = 0;
    for (i = 395; i != -1; rec++, i--) {
        /* The frame keeps 16 unused bytes here, after model: King's Field II
         * builds the rotation in an SVECTOR beside one unreferenced SVECTOR,
         * and this version builds it in the scratchpad. */
        SVECTOR rotation;
        SVECTOR unused;

        if (rec->object_id == 0xff) {
            continue;
        }
        rec->flags &= 0x7f;
        switch (rec->action) {
        case 0xf2:
            if (map_cell_visible(&rec->pos, rec->arg.m.radius_x, rec->arg.m.radius_z) == 0) {
                break;
            }
            scratch->u.load.models[rec->object_id] = 1;
            scale = &rec->scale;
            id = map_object_asset_index(rec->object_id);
            resource_registry_get(id, &model);
            if (model == NULL) {
                break;
            }
            scratch->u.load.angle.vx = rec->angle.vx;
            scratch->u.load.angle.vy = rec->angle.vy + 2048;
            scratch->u.load.angle.vz = rec->angle.vz;
            flags = rec->render_mode;
            pos.vx = ((rec->arg.m.cell_offset_x << 11) + rec->pos.vx) >> 1;
            pos.vz = ((rec->arg.m.cell_offset_z << 11) + rec->pos.vz) >> 1;
            pos.vy = (rec->pos.vy - (rec->arg.m.cell_height << 11)) >> 1;
            render_distant_world_model(rec->layer_mask, id, &pos, &scratch->u.load.angle, scale,
                                       &rec->arg.m.animation_cache, &scene_display_sys.view_matrix,
                                       rec->animation_id, rec->animation_phase, rec->lighting_override_index,
                                       rec->lighting_blend, flags, rec->render_depth_offset);
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
                    /* One local carries each half extent, the fade distance
                     * and finally the volume (King's Field II's shape). */
                    volume = rec->arg.s.region_width << 10;
                    near = cur_avatar_status.pos.vx - (volume + rec->pos.vx);
                    if (near < 0) {
                        near = -near;
                    }
                    near = volume - near;
                    volume = rec->arg.s.region_depth << 10;
                    dist = cur_avatar_status.pos.vz - (volume + rec->pos.vz);
                    if (dist < 0) {
                        dist = -dist;
                    }
                    dist = volume - dist;
                    if (dist < near) {
                        near = dist;
                    }
                    volume = rec->arg.s.fade_distance << 11;
                    if (near >= volume) {
                        volume = rec->arg.s.maximum_volume;
                    } else {
                        if (volume == 0) {
                            break;
                        }
                        volume = rec->arg.s.maximum_volume * near / volume;
                    }
                    if (rec->arg.s.vertical_attenuation_flags & 1) {
                        dist = cur_avatar_status.pos.vy - rec->pos.vy;
                        if (dist < 0) {
                            dist = -dist;
                        }
                        dist = (rec->arg.s.maximum_volume * dist) >> 13;
                        volume -= dist;
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
            if ((rec->flags & 2) && !(rec->flags & 8)) {
                goto reach;
            }
            cells = map_cell_layer_mask(&rec->pos);
            if (!(cells & rec->layer_mask) && !(rec->flags & 8)) {
                break;
            }
            rec_def = &map_objects_slots.defs[rec->object_id];
        shown:
            scratch->u.load.models[rec->object_id] = 1;
            scratch->u.load.sounds[rec_def->vab_resource_index] = 1;
            scale = &rec->scale;
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
                render_world_model(rec->layer_mask, id, &rec->pos, &scratch->u.load.angle, scale,
                                   &rec->arg.m.animation_cache, &scene_display_sys.pitch_matrix,
                                   rec->animation_id, rec->animation_phase, rec->lighting_override_index,
                                   rec->lighting_blend, flags, rec->render_depth_offset);
            } else if (rec->flags & 8) {
                render_distant_world_model(rec->layer_mask, id, &rec->pos, &scratch->u.load.angle,
                                           scale, &rec->arg.m.animation_cache,
                                           &scene_display_sys.view_matrix, rec->animation_id,
                                           rec->animation_phase, rec->lighting_override_index,
                                           rec->lighting_blend, flags, rec->render_depth_offset);
            } else {
                render_world_model(rec->layer_mask, id, &rec->pos, &scratch->u.load.angle, scale,
                                   &rec->arg.m.animation_cache, &scene_display_sys.view_matrix,
                                   rec->animation_id, rec->animation_phase, rec->lighting_override_index,
                                   rec->lighting_blend, flags, rec->render_depth_offset);
            }
            rec->flags |= 0x80;
            break;
        reach:
            rec_def = &map_objects_slots.defs[rec->object_id];
            cells = map_cell_layer_mask_radius(&rec->pos, rec_def->arg.b.unkc);
            if ((cells & rec->layer_mask) || (rec->flags & 8)) {
                goto shown;
            }
            /* fall through */
        case 0xe5:
        case 0xe9:
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
        && (animated = asset_status.animated_object) != NULL) {
        id = animated->object_id - 221;
        render_animated_object(id, &animated->angle, &animated->arg.v.animation_cache, animated->animation_id,
                               animated->animation_phase, animated->arg.v.blend_mode, animated->arg.v.lighting_index,
                               0x1fff - animated->arg.v.depth_code);
        animated->flags |= 0x80;
    }
    effect = effect_status.objects;
    for (i = 127; i != -1; effect++, i--) {
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
    for (i = 127; i != -1; placed++, i--) {
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
 */

ADDRESS(0x8004290c, 0x264)
void menu_render_item_model(void)
{
    ScratchPad *scratch;
    u8 *model;
    ModelPart *part;
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
    part = ((ModelData *)SCRATCH->model)->parts;
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
    i = part->vertex_count;
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
