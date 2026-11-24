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
        opz = &SCRATCH->opz;
        rgb = &SCRATCH->rgb;
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
            if (scratch->opz <= 0) {
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
            if (scratch->opz <= 0) {
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
            if (scratch->opz <= 0) {
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
            if (scratch->opz <= 0) {
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
