/*
 * GAME.EXE: model part renderers between the lit part renderer (game.model)
 * and the map layer front ends (game.tiles) (WIP slice of the render module).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <INLINE.H>

/*
 * Draws part id of the model in SCRATCH->model like func_80035ca4 with every
 * face semi-transparent at blend rate mode (tpage bits 5-6); the packet and
 * its vertices are kept in the scratchpad while a face is filled.
 */
ADDRESS(0x80037bec, 0xc58)
void func_80037bec(u16 id, s32 depth, s32 mode)
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
    SCRATCH->draw.count.unk4 += n;
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
 * Draws part id of the model in SCRATCH->model like func_80035ca4 for the
 * large models of func_8003f304: the depth-cue values are scaled down by
 * 3 - shift, and with shift set the faces go into the eight SCRATCH->ot2
 * entries by depth / 2048 instead of SCRATCH->ot. dist is unused.
 */
ADDRESS(0x80038844, 0xbe4)
void func_80038844(u16 id, s32 depth, s32 shift, s32 dist)
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
void func_80039428(u16 id, s16 depth, s32 shade)
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
 * renderer of func_8003bb04): projects its vertices with a fog value from
 * Data80199154's fog range, then draws its faces as Gouraud-textured packets
 * lit from one normal and depth-cued per vertex. Faces behind (gte_nclip) or
 * with every vertex nearer than w.map.clip are dropped; flat-textured records
 * become GT3/GT4 packets.
 */
ADDRESS(0x80039d50, 0xdb4)
void func_80039d50(u16 id)
{
    s32 n;
    ScratchPad *scratch;
    CVECTOR *rgb;
    CVECTOR *lit;

    scratch = SCRATCH;
    SCRATCH->unk48 = SCRATCH->screen;
    SCRATCH->vertex = SCRATCH->vertices;
    SCRATCH->w.map.near = DAT_80199154.unk15b28;
    SCRATCH->w.map.far = DAT_80199154.unk15b2c;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    n = SCRATCH->part->unk4;
    while (n-- != 0) {
        gte_ldv0(scratch->vertex);
        gte_rtps();
        gte_stsxy(&scratch->unk48->sxy);
        gte_stszotz(&scratch->unk48->z);
        if (scratch->w.map.near < 32000) {
            scratch->unk48->fog = ((scratch->unk48->z - (scratch->w.map.near >> 2)) << 14) /
                                  (scratch->w.map.far - scratch->w.map.near);
        } else {
            scratch->unk48->fog = 0;
        }
        if (scratch->unk48->fog >= 7952) {
            scratch->unk48->fog = 0x1f0f;
        }
        if (scratch->unk48->fog < 0) {
            scratch->unk48->fog = 0;
        }
        scratch->unk48++;
        scratch->vertex += 8;
    }
    scratch->prim = (u32 *)(scratch->part->prims + 12 + scratch->model);
    n = scratch->part->prim_count;
    scratch->w.map.count += n;
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
 * near ground renderer of func_8003bb04): projects the vertices like
 * func_80039d50, then hands each front face to the LIBGTE subdivision
 * (RCpolyFT3/FT4/GT3), more finely the nearer it is.
 */
ADDRESS(0x8003ab04, 0x1000)
void func_8003ab04(u16 id)
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
    SCRATCH->w.map.near = DAT_80199154.unk15b28;
    SCRATCH->w.map.far = DAT_80199154.unk15b2c;
    SCRATCH->part = &((ModelData *)SCRATCH->model)->parts[id];
    SCRATCH->normals = SCRATCH->part->normals + 12 + SCRATCH->model;
    n = SCRATCH->part->unk4;
    while (n-- != 0) {
        gte_ldv0(scratch->vertex);
        gte_rtps();
        gte_stsxy(&scratch->unk48->sxy);
        gte_stszotz(&scratch->unk48->z);
        if (scratch->w.map.near < 32000) {
            scratch->unk48->fog = ((scratch->unk48->z - (scratch->w.map.near >> 2)) << 14) /
                                  (scratch->w.map.far - scratch->w.map.near);
        } else {
            scratch->unk48->fog = 0;
        }
        if (scratch->unk48->fog >= 7952) {
            scratch->unk48->fog = 0x1f0f;
        }
        if (scratch->unk48->fog < 0) {
            scratch->unk48->fog = 0;
        }
        scratch->unk48++;
        scratch->vertex += 8;
    }
    scratch->prim = (u32 *)(scratch->part->prims + 12 + scratch->model);
    n = scratch->part->prim_count;
    scratch->w.map.count += n;
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
