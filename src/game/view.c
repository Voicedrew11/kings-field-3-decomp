/*
 * GAME.EXE: view and frame state (WIP slice of the render module that runs
 * on into game.screen): the screen effect reset, the area light records, the
 * 25 x 25 map-cell visibility grid around the viewer, the VRAM scrolls, the
 * fog, the display and GTE set-up, the double-buffered ordering tables and
 * packet pools, the frame flip and the view matrices.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/memory.h>
#include <game/data.h>
#include <game/render.h>
#include <INLINE.H>
#include <kf/lib/null.h>

/* Turns the screen tint off and empties both colour sums. */
ADDRESS(0x80034180, 0x68)
void func_80034180(void)
{
    DAT_80199154.sum[0].mode = 1;
    DAT_80199154.sum[1].mode = 1;
    DAT_80199154.sum[0].count = DAT_80199154.sum[0].r = DAT_80199154.sum[0].g =
        DAT_80199154.sum[0].b = 0;
    DAT_80199154.sum[1].count = DAT_80199154.sum[1].r = DAT_80199154.sum[1].g =
        DAT_80199154.sum[1].b = 0;
    DAT_80199154.tint.mode = 0xff;
}

/* Copies the area's light records into the working set. */
ADDRESS(0x800341e8, 0x118)
void func_800341e8(void)
{
    Record30 *src;
    Record6c *dst;
    s32 i;

    src = DAT_80081c8c;
    dst = DAT_80199154.unk15da8;
    for (i = 63; i != -1; i--) {
        dst->unk50 = src->unk14;
        dst->unk0[0] = src->unk0;
        dst->unk64 = src->unk28;
        dst->unk68 = src->unk2c;
        dst->unk6a = src->unk2e;
        src++;
        dst++;
    }
    DAT_80199154.unk15da4 = 1;
}

/* Rebuilds the quarter turns of every working light record when asked to. */
ADDRESS(0x80034300, 0x94)
void func_80034300(void)
{
    Record6c *light;
    s32 i;

    if (DAT_80199154.unk15da4 != 0) {
        light = DAT_80199154.unk15da8;
        for (i = 63; i != -1; i--) {
            func_80016290((MATRIX *)&light->unk0[0], (MATRIX *)&light->unk0[1], 1);
            func_80016290((MATRIX *)&light->unk0[0], (MATRIX *)&light->unk0[2], 2);
            func_80016290((MATRIX *)&light->unk0[0], (MATRIX *)&light->unk0[3], 3);
            light++;
        }
        DAT_80199154.unk15da4 = 0;
    }
}

/* Blends the parts of dst that flags selects toward src by t (4.12). */
ADDRESS(0x80034394, 0x148)
void func_80034394(s32 flags, Record30 *src, Record6c *dst, s32 t)
{
    if (flags & 2) {
        func_800171b0((MATRIX *)&dst->unk50, (MATRIX *)&src->unk14, (MATRIX *)&dst->unk50, t);
    }
    if (flags & 1) {
        func_800171b0((MATRIX *)&dst->unk0[0], (MATRIX *)&src->unk0, (MATRIX *)&dst->unk0[0], t);
    }
    if (flags & 4) {
        dst->unk64.r = func_80017158(dst->unk64.r, src->unk28.r, t);
        dst->unk64.g = func_80017158(dst->unk64.g, src->unk28.g, t);
        dst->unk64.b = func_80017158(dst->unk64.b, src->unk28.b, t);
    }
    if (flags & 8) {
        dst->unk68 = func_80017158(dst->unk68, src->unk2c, t);
        if (dst->unk6a - 4096 < dst->unk68) {
            dst->unk6a = dst->unk68 + 4096;
        }
    }
    if (flags & 0x10) {
        dst->unk6a = func_80017158(dst->unk6a, src->unk2e, t);
        if (dst->unk6a < dst->unk68 + 4096) {
            dst->unk68 = dst->unk6a - 4096;
        }
    }
}

/* Blends the first 48 working light records toward src. */
ADDRESS(0x800344dc, 0x90)
void func_800344dc(s32 flags, Record30 *src, s32 t)
{
    Record6c *light;
    s32 i;

    light = DAT_80199154.unk15da8;
    for (i = 0; i < 48; i++) {
        func_80034394(flags, src, light, t);
        light++;
    }
    if (flags & 1) {
        DAT_80199154.unk15da4 = 1;
    }
}

/* Blends the colour (unless any part is 0xff) and the distances (unless 0xffff). */
ADDRESS(0x8003456c, 0x88)
void func_8003456c(u8 r, u8 g, u8 b, s32 near, s32 far, s16 t)
{
    Record30 target;
    s32 flags;

    flags = 0;
    if (r != 0xff && g != 0xff && b != 0xff) {
        flags = 4;
        target.unk28.r = r;
        target.unk28.g = g;
        target.unk28.b = b;
    }
    if (near != 0xffff) {
        target.unk2c = near;
        flags |= 8;
    }
    if (far != 0xffff) {
        target.unk2e = far;
        flags |= 0x10;
    }
    func_800344dc(flags, &target, t);
}

/*
 * Hides the grid cells behind each wall cell, walking away from the viewer
 * (grid rows and columns ascending; for views facing +z).
 * Not exact: control flow, calls and constants agree; z and the x * 10 cell
 * offset trade $s5/$s6 (unattributed allocation residue; the mirror walk
 * below matches with the same shape).
 */
ADDRESS(0x800345f4, 0x300)
void func_800345f4(void)
{
    u8 *start;
    u8 *end;
    u8 *p;
    u8 *q;
    s32 x;
    s32 z;
    s32 i;
    s32 j;
    s32 n;
    s32 angle;
    MapCell *cell;

    j = 24;
    start = SCRATCH->u.view.grid[0];
    end = &SCRATCH->u.view.grid[24][24];
    p = SCRATCH->u.view.grid[0];
    z = SCRATCH->u.view.z;
    do {
        if (z < 80) {
            x = SCRATCH->u.view.x;
            for (i = 24; i != 0; i--) {
                if (x < 80 && *p != 0) {
                    cell = &DAT_801d4464.cells[z][x];
                    if (cell->layer[1].unk0 == 0xff || cell->layer[1].unk0 == 0xfe ||
                        cell->layer[1].unk0 == 0xfd) {
                        angle = func_80016ab8(x - DAT_80199154.unk15b10, z - DAT_80199154.unk15b14);
                        q = p;
                        if (angle < 256 || angle >= 3840) {
                            do {
                                *q = 0;
                                q += 25;
                            } while (q >= start && q <= end);
                        } else if (angle >= 256 && angle < 768) {
                            do {
                                *q = 0;
                                q += 24;
                            } while (q >= start && q <= end);
                        } else if (angle >= 768 && angle < 1280) {
                            n = 25 - i;
                            do {
                                *q = 0;
                                q--;
                            } while (q >= start && q <= end && --n != 0);
                        } else if (angle >= 1280 && angle < 1792) {
                            do {
                                *q = 0;
                                q -= 26;
                            } while (q >= start && q <= end);
                        } else if (angle >= 1792 && angle < 2304) {
                            do {
                                *q = 0;
                                q -= 25;
                            } while (q >= start && q <= end);
                        } else if (angle >= 2304 && angle < 2816) {
                            do {
                                *q = 0;
                                q -= 24;
                            } while (q >= start && q <= end);
                        } else if (angle >= 2816 && angle < 3328) {
                            n = i;
                            do {
                                *q = 0;
                                q++;
                            } while (q >= start && q <= end && --n != 0);
                        } else if (angle >= 3328 && angle < 3840) {
                            do {
                                *q = 0;
                                q += 26;
                            } while (q >= start && q <= end);
                        }
                    }
                }
                x++;
                p++;
            }
        } else {
            p += 25;
        }
        p++;
        z++;
    } while (--j != 0);
}

/* The same walk with grid rows and columns descending (views facing -z). */
ADDRESS(0x800348f4, 0x300)
void func_800348f4(void)
{
    u8 *start;
    u8 *end;
    u8 *p;
    u8 *q;
    s32 x;
    s32 z;
    s32 i;
    s32 j;
    s32 n;
    s32 angle;
    MapCell *cell;

    j = 24;
    start = SCRATCH->u.view.grid[0];
    end = &SCRATCH->u.view.grid[24][24];
    p = &SCRATCH->u.view.grid[24][24];
    z = SCRATCH->u.view.z + 24;
    do {
        if (z > 0) {
            x = SCRATCH->u.view.x + 24;
            for (i = 24; i != 0; i--) {
                if (x > 0 && *p != 0) {
                    cell = &DAT_801d4464.cells[z][x];
                    if (cell->layer[1].unk0 == 0xff || cell->layer[1].unk0 == 0xfe ||
                        cell->layer[1].unk0 == 0xfd) {
                        angle = func_80016ab8(x - DAT_80199154.unk15b10, z - DAT_80199154.unk15b14);
                        q = p;
                        if (angle < 256 || angle >= 3840) {
                            do {
                                *q = 0;
                                q += 25;
                            } while (q >= start && q <= end);
                        } else if (angle >= 256 && angle < 768) {
                            do {
                                *q = 0;
                                q += 24;
                            } while (q >= start && q <= end);
                        } else if (angle >= 768 && angle < 1280) {
                            n = i;
                            do {
                                *q = 0;
                                q--;
                            } while (q >= start && q <= end && --n != 0);
                        } else if (angle >= 1280 && angle < 1792) {
                            do {
                                *q = 0;
                                q -= 26;
                            } while (q >= start && q <= end);
                        } else if (angle >= 1792 && angle < 2304) {
                            do {
                                *q = 0;
                                q -= 25;
                            } while (q >= start && q <= end);
                        } else if (angle >= 2304 && angle < 2816) {
                            do {
                                *q = 0;
                                q -= 24;
                            } while (q >= start && q <= end);
                        } else if (angle >= 2816 && angle < 3328) {
                            n = 25 - i;
                            do {
                                *q = 0;
                                q++;
                            } while (q >= start && q <= end && --n != 0);
                        } else if (angle >= 3328 && angle < 3840) {
                            do {
                                *q = 0;
                                q += 26;
                            } while (q >= start && q <= end);
                        }
                    }
                }
                x--;
                p--;
            }
        } else {
            p -= 25;
        }
        p--;
        z--;
    } while (--j != 0);
}

/*
 * Builds the visibility grid: marks the cells inside the view cone (by the
 * yaw, the pitch-widened half angle and the view range) around the viewer,
 * hides those behind walls and copies the result to Data80199154.grid.
 * Not exact: control flow, calls and constants agree; register allocation
 * and the spilled point coordinates differ (first attempt only).
 */
ADDRESS(0x80034bf4, 0x4cc)
void func_80034bf4(void)
{
    s32 half;
    s32 yaw;
    s32 c;
    s32 s;
    s32 cl;
    s32 sl;
    s32 cr;
    s32 sr;
    s16 eye_z;
    s16 eye_x;
    s16 back_z;
    s16 back_x;
    s32 dlx;
    s32 dlz;
    s32 drx;
    s32 drz;
    s32 kl;
    s32 kr;
    u32 range;
    u8 *p;
    s32 i;
    s32 row;
    s32 col;
    s32 lz;
    s32 lx;
    s32 rz;
    s32 rx;
    u32 d;
    s32 left;
    s32 right;

    half = DAT_80199154.unk15b08.vx & 0xfff;
    yaw = DAT_80199154.unk15b08.vy + 1024;
    if (half > 2048) {
        half = 4096 - half;
    }
    if (half > 1024) {
        half = 2048 - half;
    }
    half = half * 584 / 1024 + 440;
    c = rcos(yaw);
    s = rsin(yaw);
    cl = rcos(yaw - half);
    sl = rsin(yaw - half);
    cr = rcos(yaw + half);
    sr = rsin(yaw + half);
    back_x = (-11 * c + 0xc7ff) >> 12;
    back_z = (-11 * s + 0xc7ff) >> 12;
    dlz = (s16)((sl * 16 + -11 * s + 0xc7ff) >> 12) - back_z;
    dlx = (s16)((cl * 16 + -11 * c + 0xc7ff) >> 12) - back_x;
    drz = (s16)((sr * 16 + -11 * s + 0xc7ff) >> 12) - back_z;
    drx = (s16)((cr * 16 + -11 * c + 0xc7ff) >> 12) - back_x;
    eye_x = (u32)(0xc7ff - c * 8) >> 12;
    eye_z = (u32)(0xc7ff - s * 8) >> 12;
    p = SCRATCH->u.view.grid[0];
    kr = -drz * back_x + drx * back_z;
    kl = -dlz * back_x + dlx * back_z;
    if (half < 600) {
        range = DAT_80199154.unk15995 * DAT_80199154.unk15995;
    } else {
        range = DAT_80199154.unk15995 * DAT_80199154.unk15995 * rcos(half >> 1) >> 12;
    }
    row = 0;
    col = 0;
    rx = 0;
    lx = 0;
    rz = 0;
    lz = 0;
    for (i = 624; i != -1; i--) {
        if (col >= 25) {
            rz = 0;
            lz = 0;
            col = 0;
            rx += drx;
            lx += dlx;
            row++;
        }
        d = (col - eye_x) * (col - eye_x) + (row - eye_z) * (row - eye_z);
        left = lz - lx + kl;
        right = rz - rx + kr;
        if (d < 5) {
            if (left <= 0 && right >= 0) {
                *p = 30;
            } else {
                *p = 0;
            }
        } else if (d < range) {
            if (left <= 0 && right >= 0) {
                *p = 26;
            } else {
                *p = 0;
            }
        } else if (d < 256) {
            *p = 8;
        } else {
            *p = 0;
        }
        p++;
        rz += drz;
        lz += dlz;
        col++;
    }
    SCRATCH->u.view.grid[eye_z - 1][eye_x] = 30;
    SCRATCH->u.view.grid[eye_z][eye_x - 1] = 30;
    SCRATCH->u.view.grid[eye_z][eye_x] = 30;
    SCRATCH->u.view.grid[eye_z][eye_x + 1] = 30;
    SCRATCH->u.view.grid[eye_z + 1][eye_x] = 30;
    if (DAT_801b24e4.unk101 != 0x11) {
        SCRATCH->u.view.grid[back_z - 1][back_x] = 0;
        SCRATCH->u.view.grid[back_z][back_x - 1] = 0;
        SCRATCH->u.view.grid[back_z][back_x] = 0;
        SCRATCH->u.view.grid[back_z][back_x + 1] = 0;
        SCRATCH->u.view.grid[back_z + 1][back_x] = 0;
    }
    DAT_80199154.grid_z = eye_z - DAT_80199154.unk15b14;
    DAT_80199154.unk15b24 = -DAT_80199154.grid_z;
    DAT_80199154.grid_x = eye_x - DAT_80199154.unk15b10;
    DAT_80199154.unk15b20 = -DAT_80199154.grid_x;
    SCRATCH->u.view.x = DAT_80199154.unk15b20;
    SCRATCH->u.view.z = DAT_80199154.unk15b24;
    if ((yaw & 0xfff) < 2048) {
        func_800345f4();
    } else {
        func_800348f4();
    }
    func_80018f5c((u32 *)DAT_80199154.grid, (u32 *)SCRATCH->u.view.grid,
                  sizeof(DAT_80199154.grid) / 4);
}

/* A free VRAM scroll record, or NULL. */
ADDRESS(0x800350c0, 0x3c)
ScrollRec *func_800350c0(void)
{
    ScrollRec *scroll;
    s32 i;

    scroll = DAT_80199154.scroll;
    i = 2;
    do {
        if (scroll->mode == 0xff) {
            return scroll;
        }
        scroll++;
    } while (--i != 0);
    return NULL;
}

/* Starts a VRAM scroll of rect: copied to its place, the source 32 pixels left. */
ADDRESS(0x800350fc, 0x100)
void func_800350fc(RECT *rect, s32 delay, s32 step, s32 mode)
{
    ScrollRec *scroll;

    scroll = func_800350c0();
    if (scroll != NULL) {
        scroll->mode = mode;
        scroll->unk4 = 0;
        scroll->unk1 = delay;
        scroll->unk3 = step;
        scroll->unk2 = 0;
        scroll->unk10 = *rect;
        scroll->unk8 = scroll->unk10;
        scroll->unk10.x -= 32;
        if (mode == 1) {
            scroll->unk8.w >>= 2;
            scroll->unk10.w >>= 2;
        }
        MoveImage(&scroll->unk8, scroll->unk10.x, scroll->unk10.y);
        DrawSync(0);
    }
}

/*
 * Steps the VRAM scrolls.
 * Not exact: only the rows scrolled and the source height trade $v0/$v1
 * before the first MoveImage (unattributed allocation residue).
 */
ADDRESS(0x800351fc, 0x15c)
void func_800351fc(void)
{
    ScrollRec *scroll;
    RECT rect;
    RECT unused;
    s32 i;
    s32 x;
    s32 step;
    s32 y;

    scroll = DAT_80199154.scroll;
    for (i = 1; i != -1; i--) {
        if (scroll->unk2 == 0) {
            rect = scroll->unk10;
            scroll->unk2 = scroll->unk1;
            if (scroll->mode == 1) {
                scroll->unk4 += scroll->unk3;
                if (scroll->unk4 >= scroll->unk10.h) {
                    scroll->unk4 -= scroll->unk10.h;
                }
                x = scroll->unk8.x;
                y = scroll->unk8.y + (step = scroll->unk4);
                rect.h = scroll->unk10.h - step;
                MoveImage(&rect, x, y);
                if (scroll->unk4 != 0) {
                    y = scroll->unk8.y;
                    rect.y = scroll->unk10.y + rect.h;
                    rect.h = scroll->unk4;
                    MoveImage(&rect, x, y);
                }
            }
        } else {
            scroll->unk2--;
        }
        scroll++;
    }
}

/* Sets the fog distances; equal distances turn the fog off. */
ADDRESS(0x80035358, 0x3c)
void func_80035358(s32 near, s32 far)
{
    DAT_80199154.unk15b28 = near;
    DAT_80199154.unk15b2c = far;
    if (near == far) {
        near = 0xffff;
    }
    SetFogNear(near, 200);
}

/* Sets up the display, the GTE, the two buffers, the packet pools and the screen state. */
ADDRESS(0x80035394, 0x29c)
void func_80035394(void)
{
    u8 *heap;
    s32 i;
    u8 *queue;
    ScrollRec *scroll;
    s32 h;

    VSync(0);
    SetDispMask(0);
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();
    h = 240;
    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(200);
    SetDefDrawEnv(&DAT_80199154.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&DAT_80199154.disp[0], 0, 240, 320, h);
    SetDefDrawEnv(&DAT_80199154.draw[1], 0, 240, 320, h);
    SetDefDispEnv(&DAT_80199154.disp[1], 0, 0, 320, h);
    DAT_80199154.draw[0].dtd = 1;
    DAT_80199154.draw[1].dtd = 1;
    DAT_80199154.draw[0].isbg = 1;
    DAT_80199154.draw[1].isbg = 1;
    setRGB0(&DAT_80199154.draw[0], 0, 0, 0);
    setRGB0(&DAT_80199154.draw[1], 0, 0, 0);
    PutDispEnv(&DAT_80199154.disp[0]);
    PutDrawEnv(&DAT_80199154.draw[0]);
    gte_SetGeomScreen(200);
    gte_SetBackColor(80, 80, 80);
    gte_SetFarColor(5, 5, 5);
    func_80035358(20000, 24000);
    DAT_80199154.frame = 0xff;
    DAT_80199154.buffer = DAT_80116954;
    DAT_80199154.pools[0].base = DAT_80116954;
    DAT_80199154.pools[0].end = DAT_80199154.pools[0].base + 0x1a000;
    DAT_80199154.pools[1].base = DAT_80199154.pools[0].end;
    DAT_80199154.pools[1].end = DAT_80199154.pools[1].base + 0x1a000;
    DAT_80199154.message_state = 0;
    DAT_80199154.message_tail = 0;
    DAT_80199154.message_head = 0;
    queue = DAT_80199154.message_queue;
    for (i = 7; i != -1; i--) {
        *queue++ = 0xff;
    }
    scroll = DAT_80199154.scroll;
    for (i = 1; i != -1; i--) {
        scroll->mode = 0xff;
        scroll++;
    }
    DAT_80199154.tint.mode = 0xff;
    DAT_80182964 = 0;
    func_80043828();
    DAT_80199154.unk15995 = 13;
    DAT_80199154.unk15996 = 1;
}

/* Starts a frame: flips the buffers and clears both ordering tables. */
ADDRESS(0x80035630, 0xd0)
void func_80035630(void)
{
    DAT_80199154.frame = !DAT_80199154.frame;
    DAT_80199154.pool = &DAT_80199154.pools[DAT_80199154.frame];
    DAT_80199154.ot = DAT_80199154.ot_buffer[DAT_80199154.frame];
    ClearOTagR(DAT_80199154.ot, OT_LENGTH);
    DAT_80199154.ot2 = DAT_80199154.ot2_buffer[DAT_80199154.frame];
    ClearOTagR(DAT_80199154.ot2, 8);
    DAT_80199154.pool->next = DAT_80199154.pool->base;
    DAT_80199154.unk159bc = DAT_80199154.ot_count = DAT_80199154.prim_count = 0;
}

/* Ends a frame: shows the finished buffer and draws ot2 then ot. */
ADDRESS(0x80035700, 0xe8)
void func_80035700(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&DAT_80199154.disp[DAT_80199154.frame]);
    PutDrawEnv(&DAT_80199154.draw[DAT_80199154.frame]);
    addPrims(DAT_80199154.ot + OT_LENGTH - 2, DAT_80199154.ot2 + 7, DAT_80199154.ot2);
    DrawOTag(DAT_80199154.ot + OT_LENGTH - 1);
}

/* Sets the viewer (either may be NULL to keep it) and rebuilds the view matrices. */
ADDRESS(0x800357e8, 0x134)
void func_800357e8(VECTOR *pos, SVECTOR *angle)
{
    SVECTOR rotation;
    VECTOR unused;

    if (pos) {
        DAT_80199154.unk15af8 = *pos;
        DAT_80199154.unk15b10 = DAT_80199154.unk15af8.vx >> 11;
        DAT_80199154.unk15b14 = DAT_80199154.unk15af8.vz >> 11;
    }
    if (angle) {
        DAT_80199154.unk15b08 = *angle;
    }
    rotation.vx = DAT_80199154.unk15b08.vx;
    rotation.vz = DAT_80199154.unk15b08.vz;
    rotation.vy = DAT_80199154.unk15b08.vy;
    RotMatrix(&rotation, &DAT_80199154.unk159f8);
    func_80016598(rotation.vx, &DAT_80199154.unk15ab8);
    func_80016290(&DAT_80199154.unk159f8, &DAT_80199154.unk15a38[0], 0);
    func_80016290(&DAT_80199154.unk159f8, &DAT_80199154.unk15a38[1], 1);
    func_80016290(&DAT_80199154.unk159f8, &DAT_80199154.unk15a38[2], 2);
    func_80016290(&DAT_80199154.unk159f8, &DAT_80199154.unk15a38[3], 3);
}
