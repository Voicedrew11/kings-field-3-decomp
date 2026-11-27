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
void render_reset_color_overlays(void)
{
    scene_display_sys.sum[0].mode = 1;
    scene_display_sys.sum[1].mode = 1;
    scene_display_sys.sum[0].count = scene_display_sys.sum[0].r = scene_display_sys.sum[0].g =
        scene_display_sys.sum[0].b = 0;
    scene_display_sys.sum[1].count = scene_display_sys.sum[1].r = scene_display_sys.sum[1].g =
        scene_display_sys.sum[1].b = 0;
    scene_display_sys.tint.mode = 0xff;
}

/* Copies the area's light records into the working set. */
ADDRESS(0x800341e8, 0x118)
void light_reset_records(void)
{
    LightSourceRecord *src;
    LightRecord *dst;
    s32 i;

    src = light_default_records;
    dst = scene_display_sys.lights;
    for (i = 63; i != -1; i--) {
        dst->color_matrix = src->color_matrix;
        dst->light_matrices[0] = src->light_matrix;
        dst->back_color = src->back_color;
        dst->fog_near = src->fog_near;
        dst->fog_far = src->fog_far;
        src++;
        dst++;
    }
    scene_display_sys.light_rotations_dirty = 1;
}

/* Rebuilds the quarter turns of every working light record when asked to. */
ADDRESS(0x80034300, 0x94)
void light_refresh_record_rotations(void)
{
    LightRecord *light;
    s32 i;

    if (scene_display_sys.light_rotations_dirty != 0) {
        light = scene_display_sys.lights;
        for (i = 63; i != -1; i--) {
            matrix_rotate_quarter_turns((MATRIX *)&light->light_matrices[0],
                                        (MATRIX *)&light->light_matrices[1], 1);
            matrix_rotate_quarter_turns((MATRIX *)&light->light_matrices[0],
                                        (MATRIX *)&light->light_matrices[2], 2);
            matrix_rotate_quarter_turns((MATRIX *)&light->light_matrices[0],
                                        (MATRIX *)&light->light_matrices[3], 3);
            light++;
        }
        scene_display_sys.light_rotations_dirty = 0;
    }
}

/* Blends the parts of dst that flags selects toward src by t (4.12). */
ADDRESS(0x80034394, 0x148)
void light_interpolate_record_fields(s32 flags, LightSourceRecord *src, LightRecord *dst, s32 t)
{
    if (flags & 2) {
        fixed_lerp_nine_halfwords_q12((MATRIX *)&dst->color_matrix, (MATRIX *)&src->color_matrix,
                                      (MATRIX *)&dst->color_matrix, t);
    }
    if (flags & 1) {
        fixed_lerp_nine_halfwords_q12((MATRIX *)&dst->light_matrices[0], (MATRIX *)&src->light_matrix,
                                      (MATRIX *)&dst->light_matrices[0], t);
    }
    if (flags & 4) {
        dst->back_color.r = fixed_lerp_q12(dst->back_color.r, src->back_color.r, t);
        dst->back_color.g = fixed_lerp_q12(dst->back_color.g, src->back_color.g, t);
        dst->back_color.b = fixed_lerp_q12(dst->back_color.b, src->back_color.b, t);
    }
    if (flags & 8) {
        dst->fog_near = fixed_lerp_q12(dst->fog_near, src->fog_near, t);
        if (dst->fog_far - 4096 < dst->fog_near) {
            dst->fog_far = dst->fog_near + 4096;
        }
    }
    if (flags & 0x10) {
        dst->fog_far = fixed_lerp_q12(dst->fog_far, src->fog_far, t);
        if (dst->fog_far < dst->fog_near + 4096) {
            dst->fog_near = dst->fog_far - 4096;
        }
    }
}

/* Blends the first 48 working light records toward src. */
ADDRESS(0x800344dc, 0x90)
void light_interpolate_records(s32 flags, LightSourceRecord *src, s32 t)
{
    LightRecord *light;
    s32 i;

    light = scene_display_sys.lights;
    for (i = 0; i < 48; i++) {
        light_interpolate_record_fields(flags, src, light, t);
        light++;
    }
    if (flags & 1) {
        scene_display_sys.light_rotations_dirty = 1;
    }
}

/* Blends the colour (unless any part is 0xff) and the distances (unless 0xffff). */
ADDRESS(0x8003456c, 0x88)
void light_interpolate_color_and_fog(u8 r, u8 g, u8 b, s32 near, s32 far, s16 t)
{
    LightSourceRecord target;
    s32 flags;

    flags = 0;
    if (r != 0xff && g != 0xff && b != 0xff) {
        flags = 4;
        target.back_color.r = r;
        target.back_color.g = g;
        target.back_color.b = b;
    }
    if (near != 0xffff) {
        target.fog_near = near;
        flags |= 8;
    }
    if (far != 0xffff) {
        target.fog_far = far;
        flags |= 0x10;
    }
    light_interpolate_records(flags, &target, t);
}

/*
 * Hides the grid cells behind each wall cell, walking away from the viewer
 * (grid rows and columns ascending; for views facing +z).
 * z is read before p: its longer life puts it just below the x * 10 cell
 * offset in allocation priority ($s6 / $s5).
 */
ADDRESS(0x800345f4, 0x300)
void view_grid_occlude_walls_ascending(void)
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
    z = SCRATCH->u.view.z;
    p = SCRATCH->u.view.grid[0];
    do {
        if (z < 80) {
            x = SCRATCH->u.view.x;
            for (i = 24; i != 0; i--) {
                if (x < 80 && *p != 0) {
                    cell = &map_world.cells[z][x];
                    if (cell->layer[1].object_index == 0xff || cell->layer[1].object_index == 0xfe ||
                        cell->layer[1].object_index == 0xfd) {
                        angle = vector_xz_to_angle(x - scene_display_sys.view_cell_x,
                                                   z - scene_display_sys.view_cell_z);
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
void view_grid_occlude_walls_descending(void)
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
                    cell = &map_world.cells[z][x];
                    if (cell->layer[1].object_index == 0xff || cell->layer[1].object_index == 0xfe ||
                        cell->layer[1].object_index == 0xfd) {
                        angle = vector_xz_to_angle(x - scene_display_sys.view_cell_x,
                                                   z - scene_display_sys.view_cell_z);
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
 * hides those behind walls and copies the result to GraphicsRuntime.grid.
 */
ADDRESS(0x80034bf4, 0x4cc)
void view_build_visibility_grid(void)
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
    ScratchPad *scratch = SCRATCH;
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

    half = scene_display_sys.view_rotation.vx & 0xfff;
    yaw = scene_display_sys.view_rotation.vy + 1024;
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
    kl = -dlz * back_x + dlx * back_z;
    kr = -drz * back_x + drx * back_z;
    if (half < 600) {
        range = scene_display_sys.view_range * scene_display_sys.view_range;
    } else {
        range = scene_display_sys.view_range * scene_display_sys.view_range * rcos(half >> 1) >> 12;
    }
    row = 0;
    col = 0;
    for (i = 624, rx = 0, lx = 0, rz = 0, lz = 0; i != -1; i--) {
        if (col >= 25) {
            rz = 0;
            lz = 0;
            col = 0;
            rx += drx;
            lx += dlx;
            row++;
        }
        left = lz - lx + kl;
        right = rz - rx + kr;
        d = (col - eye_x) * (col - eye_x) + (row - eye_z) * (row - eye_z);
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
    scratch->u.view.grid[eye_z - 1][eye_x] = 30;
    scratch->u.view.grid[eye_z][eye_x - 1] = 30;
    scratch->u.view.grid[eye_z][eye_x] = 30;
    scratch->u.view.grid[eye_z][eye_x + 1] = 30;
    scratch->u.view.grid[eye_z + 1][eye_x] = 30;
    if (cur_avatar_status.reaction_mode != 0x11) {
        scratch->u.view.grid[back_z - 1][back_x] = 0;
        scratch->u.view.grid[back_z][back_x - 1] = 0;
        scratch->u.view.grid[back_z][back_x] = 0;
        scratch->u.view.grid[back_z][back_x + 1] = 0;
        scratch->u.view.grid[back_z + 1][back_x] = 0;
    }
    scene_display_sys.grid_x = eye_x - scene_display_sys.view_cell_x;
    scene_display_sys.grid_z = eye_z - scene_display_sys.view_cell_z;
    scene_display_sys.map_scan_start_x = -scene_display_sys.grid_x;
    scene_display_sys.map_scan_start_z = -scene_display_sys.grid_z;
    scratch->u.view.x = scene_display_sys.map_scan_start_x;
    scratch->u.view.z = scene_display_sys.map_scan_start_z;
    if ((yaw & 0xfff) < 2048) {
        view_grid_occlude_walls_ascending();
    } else {
        view_grid_occlude_walls_descending();
    }
    resource_copy_words((u32 *)scene_display_sys.grid, (u32 *)scratch->u.view.grid,
                        sizeof(scene_display_sys.grid) / 4);
}

/* A free VRAM scroll record, or NULL. */
ADDRESS(0x800350c0, 0x3c)
VramScroll *vram_scroll_find_free(void)
{
    VramScroll *scroll;
    s32 i;

    scroll = scene_display_sys.scroll;
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
void vram_scroll_start(RECT *rect, s32 delay, s32 step, s32 mode)
{
    VramScroll *scroll;

    scroll = vram_scroll_find_free();
    if (scroll != NULL) {
        scroll->mode = mode;
        scroll->row_offset = 0;
        scroll->update_interval = delay;
        scroll->row_step = step;
        scroll->frames_until_update = 0;
        scroll->copy_rect = *rect;
        scroll->rect = scroll->copy_rect;
        scroll->copy_rect.x -= 32;
        if (mode == 1) {
            scroll->rect.w >>= 2;
            scroll->copy_rect.w >>= 2;
        }
        MoveImage(&scroll->rect, scroll->copy_rect.x, scroll->copy_rect.y);
        DrawSync(0);
    }
}

/*
 * Steps the VRAM scrolls.
 * Not exact: only the rows scrolled and the source height trade $v0/$v1
 * before the first MoveImage (unattributed allocation residue).
 */
ADDRESS(0x800351fc, 0x15c)
void vram_scroll_update(void)
{
    VramScroll *scroll;
    RECT rect;
    RECT unused;
    s32 i;
    s32 x;
    s32 step;
    s32 y;

    scroll = scene_display_sys.scroll;
    for (i = 1; i != -1; i--) {
        if (scroll->frames_until_update == 0) {
            rect = scroll->copy_rect;
            scroll->frames_until_update = scroll->update_interval;
            if (scroll->mode == 1) {
                scroll->row_offset += scroll->row_step;
                if (scroll->row_offset >= scroll->copy_rect.h) {
                    scroll->row_offset -= scroll->copy_rect.h;
                }
                x = scroll->rect.x;
                y = scroll->rect.y + (step = scroll->row_offset);
                rect.h = scroll->copy_rect.h - step;
                MoveImage(&rect, x, y);
                if (scroll->row_offset != 0) {
                    y = scroll->rect.y;
                    rect.y = scroll->copy_rect.y + rect.h;
                    rect.h = scroll->row_offset;
                    MoveImage(&rect, x, y);
                }
            }
        } else {
            scroll->frames_until_update--;
        }
        scroll++;
    }
}

/* Sets the fog distances; equal distances turn the fog off. */
ADDRESS(0x80035358, 0x3c)
void fog_set_range(s32 near, s32 far)
{
    scene_display_sys.fog_near = near;
    scene_display_sys.fog_far = far;
    if (near == far) {
        near = 0xffff;
    }
    SetFogNear(near, 200);
}

/* Sets up the display, the GTE, the two buffers, the packet pools and the screen state. */
ADDRESS(0x80035394, 0x29c)
void display_initialize(void)
{
    u8 *heap;
    s32 i;
    u8 *queue;
    VramScroll *scroll;
    s32 h;

    VSync(0);
    SetDispMask(0);
    ResetGraph(0);
    SetGraphDebug(0);
    InitGeom();
    h = 240;
    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(200);
    SetDefDrawEnv(&scene_display_sys.draw[0], 0, 0, 320, h);
    SetDefDispEnv(&scene_display_sys.disp[0], 0, 240, 320, h);
    SetDefDrawEnv(&scene_display_sys.draw[1], 0, 240, 320, h);
    SetDefDispEnv(&scene_display_sys.disp[1], 0, 0, 320, h);
    scene_display_sys.draw[0].dtd = 1;
    scene_display_sys.draw[1].dtd = 1;
    scene_display_sys.draw[0].isbg = 1;
    scene_display_sys.draw[1].isbg = 1;
    setRGB0(&scene_display_sys.draw[0], 0, 0, 0);
    setRGB0(&scene_display_sys.draw[1], 0, 0, 0);
    PutDispEnv(&scene_display_sys.disp[0]);
    PutDrawEnv(&scene_display_sys.draw[0]);
    gte_SetGeomScreen(200);
    gte_SetBackColor(80, 80, 80);
    gte_SetFarColor(5, 5, 5);
    fog_set_range(20000, 24000);
    scene_display_sys.frame = 0xff;
    scene_display_sys.buffer = ot_packet_memory;
    scene_display_sys.pools[0].base = ot_packet_memory;
    scene_display_sys.pools[0].end = scene_display_sys.pools[0].base + 0x1a000;
    scene_display_sys.pools[1].base = scene_display_sys.pools[0].end;
    scene_display_sys.pools[1].end = scene_display_sys.pools[1].base + 0x1a000;
    scene_display_sys.message_state = 0;
    scene_display_sys.message_tail = 0;
    scene_display_sys.message_head = 0;
    queue = scene_display_sys.message_queue;
    for (i = 7; i != -1; i--) {
        *queue++ = 0xff;
    }
    scroll = scene_display_sys.scroll;
    for (i = 1; i != -1; i--) {
        scroll->mode = 0xff;
        scroll++;
    }
    scene_display_sys.tint.mode = 0xff;
    map_placed_frame_counter = 0;
    pool_reset();
    scene_display_sys.view_range = 13;
    scene_display_sys.unk15996 = 1;
}

/* Starts a frame: flips the buffers and clears both ordering tables. */
ADDRESS(0x80035630, 0xd0)
void display_begin_frame(void)
{
    scene_display_sys.frame = !scene_display_sys.frame;
    scene_display_sys.pool = &scene_display_sys.pools[scene_display_sys.frame];
    scene_display_sys.ot = scene_display_sys.ot_buffer[scene_display_sys.frame];
    ClearOTagR(scene_display_sys.ot, OT_LENGTH);
    scene_display_sys.ot2 = scene_display_sys.ot2_buffer[scene_display_sys.frame];
    ClearOTagR(scene_display_sys.ot2, 8);
    scene_display_sys.pool->next = scene_display_sys.pool->base;
    scene_display_sys.tmd_prim_count = scene_display_sys.ot_count = scene_display_sys.prim_count = 0;
}

/* Ends a frame: shows the finished buffer and draws ot2 then ot. */
ADDRESS(0x80035700, 0xe8)
void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDispEnv(&scene_display_sys.disp[scene_display_sys.frame]);
    PutDrawEnv(&scene_display_sys.draw[scene_display_sys.frame]);
    addPrims(scene_display_sys.ot + OT_LENGTH - 2, scene_display_sys.ot2 + 7, scene_display_sys.ot2);
    DrawOTag(scene_display_sys.ot + OT_LENGTH - 1);
}

/* Sets the viewer (either may be NULL to keep it) and rebuilds the view matrices. */
ADDRESS(0x800357e8, 0x134)
void display_set_view_transform(VECTOR *pos, SVECTOR *angle)
{
    SVECTOR rotation;
    VECTOR unused;

    if (pos) {
        scene_display_sys.view_position = *pos;
        scene_display_sys.view_cell_x = scene_display_sys.view_position.vx >> 11;
        scene_display_sys.view_cell_z = scene_display_sys.view_position.vz >> 11;
    }
    if (angle) {
        scene_display_sys.view_rotation = *angle;
    }
    rotation.vx = scene_display_sys.view_rotation.vx;
    rotation.vz = scene_display_sys.view_rotation.vz;
    rotation.vy = scene_display_sys.view_rotation.vy;
    RotMatrix(&rotation, &scene_display_sys.view_matrix);
    matrix_set_rotation_x(rotation.vx, &scene_display_sys.pitch_matrix);
    matrix_rotate_quarter_turns(&scene_display_sys.view_matrix, &scene_display_sys.view_matrix_turns[0], 0);
    matrix_rotate_quarter_turns(&scene_display_sys.view_matrix, &scene_display_sys.view_matrix_turns[1], 1);
    matrix_rotate_quarter_turns(&scene_display_sys.view_matrix, &scene_display_sys.view_matrix_turns[2], 2);
    matrix_rotate_quarter_turns(&scene_display_sys.view_matrix, &scene_display_sys.view_matrix_turns[3], 3);
}
