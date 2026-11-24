/*
 * GAME.EXE: model data registration and the lit model part renderer (WIP
 * slice of the render module; the primitive fix-up func_8003591c before it
 * is not claimed yet).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <INLINE.H>

/* Registers model data id (0..3) and fixes up its primitives. */
ADDRESS(0x80035c44, 0x40)
void func_80035c44(u16 id, u8 *data)
{
    DAT_80199154.vertices = DAT_80199154.unk10148[id] = data;
    func_8003591c(data);
}

/* Registers model data id without the fix-up. */
ADDRESS(0x80035c84, 0x20)
void func_80035c84(u16 id, u8 *data)
{
    DAT_80199154.unk10148[id] = data;
}

/*
 * Draws part id of the model in SCRATCH->model from the projected vertices in
 * SCRATCH->screen: back faces (gte_nclip) dropped, each face lit from its
 * normals (normal colour depth-cued by the vertex fog values) and linked at
 * its average depth plus depth. Returns early when the packet pool is full.
 */
ADDRESS(0x80035ca4, 0xa04)
void func_80035ca4(u16 id, s32 depth)
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
    SCRATCH->draw.count.unk4 += n;
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
 * vertices with a fog value from Data80199154's fog range, then hands each
 * front face to the LIBGTE subdivision (RCpolyFT3/FT4/GT3/GT4), more finely
 * the nearer it is. Faces with every vertex nearer than draw.count.clip are
 * dropped.
 */
ADDRESS(0x800366a8, 0x1544)
void func_800366a8(u16 id)
{
    s32 n;
    ScratchPad *scratch;
    CVECTOR *rgb;
    union {
        DIVPOLYGON3 d3;
        DIVPOLYGON4 d4;
    } div;

    scratch = SCRATCH;
    SCRATCH->unk48 = SCRATCH->screen;
    SCRATCH->vertex = SCRATCH->vertices;
    SCRATCH->w.model.near = DAT_80199154.unk15b28;
    SCRATCH->w.model.far = DAT_80199154.unk15b2c;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    n = SCRATCH->part->unk4;
    while (n-- != 0) {
        gte_ldv0(scratch->vertex);
        gte_rtps();
        gte_stsxy(&scratch->unk48->sxy);
        gte_stszotz(&scratch->unk48->z);
        if (scratch->w.model.near < 32000) {
            scratch->unk48->fog = ((scratch->unk48->z - (scratch->w.model.near >> 2)) << 14) /
                                  (scratch->w.model.far - scratch->w.model.near);
        } else {
            scratch->unk48->fog = 0;
        }
        if (scratch->unk48->fog >= 8192) {
            scratch->unk48->fog = 0x1fff;
        }
        if (scratch->unk48->fog < 0) {
            scratch->unk48->fog = 0;
        }
        scratch->unk48++;
        scratch->vertex += 8;
    }
    scratch->prim = (u32 *)(scratch->part->prims + 12 + scratch->model);
    n = scratch->part->prim_count;
    scratch->draw.count.unk4 += n;
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
