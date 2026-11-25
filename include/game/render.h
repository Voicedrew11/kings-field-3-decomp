#ifndef GAME_RENDER_H
#define GAME_RENDER_H

/*
 * GAME.EXE render work area (WIP). The game loop clears 0x5e2a words from
 * 0x80199154 as one block; the members below are the ones the render and
 * screen-effect code reaches. GCC CSEs one base for addresses inside it
 * (loading_screen_update_fade indexes draw[] and disp[] from the address of fade_hold),
 * so it is one object. Field names are WIP.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/memory.h>
#include <game/cd.h>
#include <game/collision.h>
#include <game/data.h>

/* Ordering table depth: draw functions only link primitives whose depth is
 * inside 1..OT_LENGTH-1. */
#define OT_LENGTH 0x2000

/* A bump allocator for GPU packets: next moves up to end. */
typedef struct PrimPool {
    u8 *base;
    u8 *end;
    u8 *next;
} PrimPool;

/* Full-screen tint drawn every frame unless mode is 0xff. */
typedef struct ScreenTint {
    u8 mode; /* bits 0-1: blend rate; bit 7: draw at depth 1 instead of 0x40 */
    u8 r;
    u8 g;
    u8 b;
} ScreenTint;

/* Colour accumulated during a frame; the average is drawn as a tint. */
typedef struct ColorSum {
    u8 count;
    u8 mode;
    s16 r;
    s16 g;
    s16 b;
} ColorSum;

/* A model file as loaded: offsets are from the start of the file. */
typedef struct ModelFile {
    u32 unk0;
    s32 unk4;       /* 0: static model, no animation */
    s32 vertices;   /* offset of the vertex data */
    s32 deltas;     /* offset of the delta-run offset table */
    s32 anims;      /* offset of the animation offset table */
} ModelFile;

/* A vertex set of a model's data block. */
typedef struct ModelPart {
    s32 vertices;   /* offset from the block start, less 12 */
    s32 unk4;       /* +0x04: vertex count */
    s32 normals;    /* +0x08: offset of the normals from the block start, less 12 */
    u8 unkc[4];
    s32 prims;      /* +0x10: offset of the primitives from the block start, less 12 */
    s32 prim_count; /* +0x14 */
    u8 unk18[4];
} ModelPart;

/* The data block a ModelFile's vertices offset points at. */
typedef struct ModelData {
    u32 unk0;
    u32 unk4;       /* +0x04: render_map_cell_object draws only parts below unk4 / 2 while loading */
    s32 count;      /* +0x08: parts */
    ModelPart parts[1];
} ModelData;

/* One key frame of an animation. */
typedef struct AnimKey {
    u16 reverse;    /* nonzero: the sub-frame runs from 0x1000 down */
    u16 length;     /* time units */
    u16 base;       /* delta table entry blended by the sub-frame */
    u16 count;      /* delta runs applied to the vertices: first, then ids */
    u16 first;
    u16 ids[1];
} AnimKey;

/* A cached animated pose; owner points back at the model's slot pointer. */
typedef struct AnimSlot {
    s16 state;      /* 0 free, 1 unused this frame, 2 drawn this frame */
    u16 model;
    u16 anim;
    u16 key;
    s32 base;
    SVECTOR *pose;
    struct AnimSlot **owner;
} AnimSlot;

/* A placed effect (128 of them follow a frame counter); translate_active_world_positions moves
 * them with the area origin. */
typedef struct Effect {
    u16 id;         /* 0xffff: unused */
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;        /* random phase below unk3 */
    u8 unk6[2];
    VECTOR pos;
} Effect;

/* An effect placement record of a map file: map cell plus offsets. */
typedef struct EffectSpawn {
    u16 id;         /* 0xffff: none */
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 cell_z;
    u8 cell_x;
    u8 unk7[3];
    s16 dz;
    s16 dx;
    s16 dy;
} EffectSpawn;

/* Counter the effect code keeps ahead of the records (WIP). */
extern s32 DAT_80182964;
extern Effect DAT_80182968[128];

/* Heap the frame packet pools and the 2D quad buffer are carved from (WIP extent). */
extern u8 DAT_80116954[];

/* A 3x3 4.12 matrix without MATRIX's translation (20 bytes, 2-aligned). */
typedef struct Matrix3x3 {
    s16 m[3][3];
    s16 unk12;
} Matrix3x3;

/* 48-byte light records of the area tables (DAT_80081c8c, WIP names). */
typedef struct Record30 {
    Matrix3x3 unk0;     /* fixed_lerp_nine_halfwords_q12 blends it into Record6c.unk0[0] */
    Matrix3x3 unk14;    /* into Record6c.unk50 */
    CVECTOR unk28;
    u16 unk2c;          /* kept 4096 below unk2e */
    u16 unk2e;
} Record30;

/* The area's light records (resource_initialize_game_assets copies them out of the first FDAT.T entry). */
extern Record30 DAT_80081c8c[64];

/* The working copy of a Record30: unk0[0] and its three quarter turns. */
typedef struct Record6c {
    Matrix3x3 unk0[4];
    Matrix3x3 unk50;
    CVECTOR unk64;
    u16 unk68;
    u16 unk6a;
} Record6c;

/* A VRAM area copied from unk10 to unk8 every unk1 + 1 frames (vram_scroll_update). */
typedef struct ScrollRec {
    u8 mode;            /* 0xff free; 1 scrolls the source by unk3 rows */
    u8 unk1;            /* frames between steps */
    u8 unk2;            /* frames to the next step */
    u8 unk3;            /* step */
    s16 unk4;           /* rows scrolled */
    u8 unk6[2];
    RECT unk8;          /* destination */
    RECT unk10;         /* source */
} ScrollRec;

/* A vertex after gte_rtps: screen position, depth and depth-cue value. */
typedef struct ScreenVertex {
    DVECTOR sxy;        /* GTE SXY */
    s16 z;              /* SZ / 4 (gte_stszotz writes z and fog as one word) */
    s16 fog;            /* depth-cue interpolation, 0..0x1f0f */
} ScreenVertex;

typedef struct Data80199154 {
    u8 *buffer;                     /* +0x00000: 2D quads per frame; CD load buffer */
    PrimPool pools[2];              /* +0x00004: one per frame, carved from the heap
                                     * (display_initialize); the wipe effect copies the
                                     * screen to pools[1].end */
    PrimPool *pool;                 /* +0x0001c: the pool draw functions allocate from */
    u32 ot_buffer[2][0x2000];       /* +0x00020: OT_LENGTH entries each */
    u32 *ot;                        /* +0x10020 */
    u32 ot2_buffer[2][8];           /* +0x10024: drawn ahead of ot (display_present_frame) */
    u32 *ot2;                       /* +0x10064 */
    DRAWENV draw[2];                /* +0x10068 */
    DISPENV disp[2];                /* +0x10120 */
    u8 *unk10148[4];                /* +0x10148: model data by id (tmd_register) */
    u8 *vertices;                   /* +0x10158: vertex data of the last model registered */
    u8 *models[0x62d];              /* +0x1015c: loaded files by id: models 0..103 stay
                                     * resident, area banks from 256 + area * 32
                                     * (map_object_asset_index), the 32 blocks of an area part
                                     * from 556 + part * 32 (resource_mark_area_models_reclaimable); WIP extent,
                                     * up to slots */
    AnimSlot slots[12];             /* +0x11a10 */
    ScreenVertex screen[1000];      /* +0x11b00: projected vertices of the part being
                                     * drawn (WIP extent) */
    u8 unk13a40[4];
    SVECTOR pose[1000];             /* +0x13a44: the blended pose being drawn */
    u16 message_arg[8];             /* +0x15984: number shown with message 25 */
    u8 frame;                       /* +0x15994: index of draw[]/disp[] in use */
    u8 unk15995;                    /* +0x15995: view range in map cells */
    u8 unk15996;
    u8 fade_level;                  /* +0x15997 */
    u8 fade_hold;                   /* +0x15998 */
    u8 message_queue[8];            /* +0x15999: message ids, 0xff when free */
    u8 message_tail;                /* +0x159a1: the message shown */
    u8 message_head;                /* +0x159a2: the next free queue slot */
    u8 message_state;               /* +0x159a3: 0 idle, 1 fade in, 2 hold, 3 fade out */
    u8 message_timer;               /* +0x159a4 */
    u8 message_level;               /* +0x159a5: message brightness */
    ScreenTint tint;                /* +0x159a6 */
    ColorSum sum[2];                /* +0x159aa */
    u8 unk159ba[2];
    s32 unk159bc;
    s32 ot_count;                   /* +0x159c0: primitives linked this frame */
    s32 prim_count;                 /* +0x159c4: packets allocated this frame */
    ScrollRec scroll[2];            /* +0x159c8: VRAM scrolls, mode 0xff when free */
    MATRIX unk159f8;                /* +0x159f8: view rotation */
    u8 unk15a18[0x20];
    MATRIX unk15a38[4];             /* +0x15a38: unk159f8 turned by 0..3 quarters */
    MATRIX unk15ab8;                /* +0x15ab8: rotation about x by the view pitch */
    u8 unk15ad8[0x20];
    VECTOR unk15af8;                /* +0x15af8: viewer position */
    SVECTOR unk15b08;               /* +0x15b08: viewer angles (vy the yaw) */
    s32 unk15b10;                   /* +0x15b10: map cell x of the viewer */
    s32 unk15b14;                   /* +0x15b14: map cell z of the viewer */
    s32 grid_x;                     /* +0x15b18: map cell x of grid column 0, negated */
    s32 grid_z;                     /* +0x15b1c: map cell z of grid row 0, negated */
    s32 unk15b20;                   /* +0x15b20: map cell x of grid column 0 */
    s32 unk15b24;                   /* +0x15b24: map cell z of grid row 0 */
    s32 unk15b28;                   /* +0x15b28: fog near (fog_set_range) */
    s32 unk15b2c;                   /* +0x15b2c: fog far */
    u8 grid[25][25];                /* +0x15b30: map cells around the viewer */
    u8 unk15da1[3];
    s32 unk15da4;                   /* +0x15da4: unk15da8 turns need rebuilding */
    Record6c unk15da8[64];          /* +0x15da8 */
} Data80199154;

extern Data80199154 DAT_80199154;

/* Scratchpad words from +0x100 while the map layers are drawn (render_map_cell_layers). */
typedef struct ScratchLayer {
    SVECTOR unk100;     /* +0x100: map layer position render_map_cell_layers hands render_map_cell_object */
    VECTOR unk108;      /* +0x108: origin of the map layer positions */
} ScratchLayer;

/*
 * Scratchpad words from +0x100 while the objects are drawn (render_scene_and_update_resources):
 * each pass flags the model ids and sound banks its objects use, and
 * resource_tmd_update_range/resource_vab_update_range keep those loaded through the wanted pointers.
 */
typedef struct ScratchLoad {
    u8 unk100[0x14];
    SVECTOR angle;      /* +0x114: rotation handed to the model front ends */
    u8 *model_wanted;   /* +0x11c: one flag per model id of the area: keep it loaded */
    u8 *sound_wanted;   /* +0x120: one flag per sound bank of the area */
    u8 models[332];     /* +0x124: model flags from id 128 (Object88) or 256 (Object44) */
    u8 sounds[96];      /* +0x270: sound bank flags from 51 (Object88) or 147 (Object44) */
} ScratchLoad;

/* Scratchpad words from +0x100 while a frame is built (view_build_visibility_grid). */
typedef struct ScratchView {
    u8 unk100[0x18];
    s32 x;              /* +0x118: map cell x of grid column 0 */
    s32 z;              /* +0x11c: map cell z of grid row 0 */
    u8 grid[25][25];    /* +0x120: copied to Data80199154.grid */
} ScratchView;

typedef union ScratchPhase {
    ScratchLayer layer;
    ScratchLoad load;
    ScratchView view;
} ScratchPhase;

/* The head word of a model primitive record: len words follow it, code is the
 * GPU primitive code (bit 1, semi-transparency, ignored when picking the kind). */
typedef struct PrimTag {
    u8 unk0;
    u8 len;
    u8 unk2;
    u8 code;
} PrimTag;

/* Scratchpad words from +0x54 while a model part is drawn. */
typedef struct ScratchModelWork {
    VECTOR unk54;       /* +0x54: gte_stlvnl result of the overlay vertex loop (render_active_model_rows) */
    CVECTOR rgb;        /* +0x64: colour the models are lit with */
    s32 near;           /* +0x68: Data80199154.unk15b28 (fog near, render_enqueue_tmd_subdivided) */
    s32 far;            /* +0x6c: Data80199154.unk15b2c (fog far) */
    s32 opz;            /* +0x70: gte_nclip result of the primitive being drawn */
} ScratchModelWork;

/* Scratchpad words from +0x54 while a map layer part is drawn (render_enqueue_map). */
typedef struct ScratchMapWork {
    CVECTOR rgb;        /* +0x54: colour the ground is lit with */
    s32 near;           /* +0x58: Data80199154.unk15b28 (fog near) */
    s32 far;            /* +0x5c: Data80199154.unk15b2c (fog far) */
    s32 opz;            /* +0x60: gte_nclip result of the face being drawn */
    s16 z;              /* +0x64: ordering table depth of the face */
    s16 clip;           /* +0x66: faces with every vertex nearer than this are dropped */
    s32 count;          /* +0x68: the renderers add each part's primitive count */
    s32 ot_count;       /* +0x6c */
    s32 prim_count;     /* +0x70 */
} ScratchMapWork;

typedef union ScratchWork {
    ScratchModelWork model;
    ScratchMapWork map;
} ScratchWork;

/* Scratchpad words from +0x74 while a model is drawn: the frame counters,
 * copied in and back by the model front ends (menu_render_item_model). */
typedef struct ScratchCount {
    s16 z;              /* +0x74: ordering table depth of the face (render_enqueue_tmd_subdivided) */
    s16 clip;           /* +0x76: faces with every vertex nearer than this are dropped
                         * (render_enqueue_tmd_subdivided); render_world_model sets 100 */
    s32 unk4;           /* +0x78: Data80199154.unk159bc; the renderers add each part's
                         * primitive count */
    s32 ot_count;       /* +0x7c */
    s32 prim_count;     /* +0x80 */
    u16 clut;           /* +0x84: added to each primitive's CLUT */
    u8 unk12[0xe];
} ScratchCount;

typedef union ScratchDraw {
    MATRIX transform;   /* map layer transform (render_map_cell_object) */
    ScratchCount count;
} ScratchDraw;

/*
 * The R3000 data cache used as scratchpad RAM at 0x1f800000 (a fixed
 * hardware address). The game keeps per-frame working pointers there.
 */
typedef struct ScratchPad {
    s32 nclip[2];       /* +0x00: gte_nclip results of a quad's two halves (render_enqueue_map_subdivided) */
    u32 *ot;            /* +0x08: Data80199154.ot while a model is drawn */
    u32 *ot2;           /* +0x0c: Data80199154.ot2 (render_scene_and_update_resources) */
    u8 *model;          /* +0x10: data of the model being drawn */
    u8 *next;           /* +0x14: Data80199154.pool->next while a model is drawn;
                         * render_map_cell_object drops flag 4 within 10240 of end */
    u8 *end;            /* +0x18: pool->end */
    PrimTag tag;        /* +0x1c: head word of the primitive record being drawn */
    u32 *prim;          /* +0x20: the primitive record after its head word */
    ModelPart *part;    /* +0x24: the part being drawn */
    u8 *normals;        /* +0x28: the part's normals (records index them by byte offset) */
    u8 *poly;           /* +0x2c: the packet being filled (render_enqueue_blended_tmd) */
    ScreenVertex *vert[4]; /* +0x30: its projected vertices (render_enqueue_blended_tmd) */
    CVECTOR lit;        /* +0x40: map layers: a face's lit colour before depth cueing */
    ScreenVertex *screen; /* +0x44: the part's projected vertices */
    ScreenVertex *unk48;  /* +0x48: render_world_model sets it to screen too; the map layer
                           * renderers' vertex loop cursor */
    u8 *vertices;       /* +0x4c: vertices of the model being drawn */
    u8 *vertex;         /* +0x50: map layers: vertex loop cursor */
    ScratchWork w;      /* +0x54 */
    ScratchDraw draw;   /* +0x74 */
    u8 unk94[0x6c];
    ScratchPhase u;     /* +0x100 */
} ScratchPad;

#define SCRATCH ((ScratchPad *)0x1f800000)

/* A 2D overlay sprite record; lists end with visible == 0xff. */
typedef struct SpriteRec {
    u8 visible;     /* 0 hidden; bits 0-2 pick the passes notification_draw draws */
    u8 mode;        /* blend rate for render_sprite_record, 0xff opaque */
    u8 u;
    u8 v;
    u8 w;
    u8 h;
    s16 x;
    s16 y;
    s16 unka;
    s16 unkc;
    u16 clut;
    u16 tpage;
    s16 z;
} SpriteRec;

/* The message strip sprites (6 and the terminator) and the status overlay. */
extern SpriteRec DAT_80081928[7];
extern SpriteRec DAT_800819b4[31];

/* A 3D overlay record (compass needles); the list ends with visible == 0xff. */
typedef struct Data80081c20 {
    u8 visible;     /* 1 drawn (render_active_model_rows) */
    u8 anim;        /* +0x01 */
    u8 light;       /* +0x02: light record (Data80199154.unk15da8) */
    u8 unk3;
    u16 model;      /* +0x04: model file id */
    u16 time;       /* +0x06: animation time */
    SVECTOR scale;  /* +0x08: ScaleMatrix factors */
    SVECTOR pos;    /* +0x10: translation */
    SVECTOR angle;  /* +0x18: vx follows the view pitch; vy eased toward the viewer's heading */
    AnimSlot *slot; /* +0x20 */
} Data80081c20;

extern Data80081c20 DAT_80081c20[3];

/*
 * View matrix render_scene_and_update_resources hands the model front ends for the objects drawn
 * without the view rotation (load-image .data; owner unit unknown).
 */
extern MATRIX DAT_8007e4c4;

extern RECT DAT_8009c2d0;
extern CVECTOR DAT_8009c2e0;
extern SVECTOR DAT_8009c2e4;
extern s32 DAT_8009c2ec;

/* Links p at depth z when z is inside the ordering table. */
#define LINK_PRIM(z, p)                                         \
    {                                                           \
        s32 depth = (z);                                        \
        if (depth > 0 && (u32)depth < OT_LENGTH) {              \
            addPrim(DAT_80199154.ot + depth, (p));              \
            DAT_80199154.ot_count++;                            \
        }                                                       \
    }

/*
 * Links packet p of a model part into scratch->ot at z + depth, z being the
 * face's depth: faces at z <= 0 or past the table are dropped. The part
 * renderers expand it once per face kind, each with its own z.
 */
#define LINK_PART_PRIM(scratch, zexpr, depth, p)                    \
    {                                                               \
        s32 z = (zexpr);                                            \
        if (z > 0 && (u32)(z + (depth)) < OT_LENGTH) {              \
            addPrim((scratch)->ot + (z + (depth)), (p));            \
            (scratch)->draw.count.ot_count++;                       \
        }                                                           \
    }

/* As LINK_PART_PRIM into the eight scratch->ot2 entries, at (z + depth) & 7. */
#define LINK_PART_BIN(scratch, zexpr, depth, p)                     \
    {                                                               \
        s32 z = (zexpr);                                            \
        if (z > 0 && (u32)((z + (depth)) & 7) < 8) {                \
            addPrim((scratch)->ot2 + ((z + (depth)) & 7), (p));     \
            (scratch)->draw.count.ot_count++;                       \
        }                                                           \
    }

void render_textured_quad_immediate(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                                    u16 tpage, u16 clut, u8 r, u8 g, u8 b);
void loading_screen_draw_quad(u8 level, u8 inset);
void loading_screen_show(void);
void loading_screen_update_fade(void);
void render_textured_quad(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                          u16 tpage, u16 clut, u8 r, u8 g, u8 b, s32 z);
void render_gouraud_quad(int x0, int y0, int x1, int y1, u8 semi, u32 *rgb0, u32 *rgb1, u32 *rgb2,
                         u32 *rgb3, s32 z);
void render_sprite(int x, int y, u8 u, u8 v, u8 w, u8 h, u8 semi, u16 tpage, u16 clut, u8 r,
                   u8 g, u8 b, s32 z);
void render_sliding_panel_primary(void);
void render_sliding_panel_secondary(void);
void render_sliding_panel_tertiary(void);
void func_8003d4ac(void);
void render_color_overlay(void);
void render_set_color_overlay(u8 mode, u8 r, u8 g, u8 b);
void render_accumulated_color_overlay_additive(void);
void render_accumulated_color_overlay_subtractive(void);
void accumulate_color_overlay(s32 r, s32 g, s32 b, s32 scale, s32 which);
void set_accumulated_color_overlay(s32 r, s32 g, s32 b, s32 scale, s32 which);
void render_zoom_screen_transition(u8 *rgb, s32 x, s32 y, s32 step);

void render_animated_object(u16 id, SVECTOR *rotation, AnimSlot **owner, u16 anim, u16 time, s32 shade,
                            s32 light_id, int depth);
u16 map_object_asset_index(u16 id);
void resource_registry_get(u16 id, u8 **out);
void resource_tmd_read_complete(u8 *file);
u8 map_cell_layer_mask(VECTOR *pos);
u8 map_cell_layer_mask_radius(VECTOR *pos, s32 radius);
s32 map_cell_visible(VECTOR *pos, s32 range_x, s32 range_z);
void resource_tmd_queue_read(s32 file, s32 entry, s32 id);
void resource_vab_update_range(s32 file, s32 entry, s32 bank, s32 count);
void resource_tmd_update_range(s32 file, s32 entry, s32 id, s32 count);

void render_sprite_record(SpriteRec *sprite, u16 mode, u8 *rgb);
void notification_draw(void);
void render_hud_sprites(void);
void notify_enqueue(s32 message, ...);
void notification_digit_set_u(SpriteRec *sprite, u8 digit);
void notification_update(void);
void render_game_frame(VECTOR *pos, SVECTOR *angle);
void menu_render_item_model(void);

void asset_registry_select(u16 id);
void asset_registry_load_tmd_archive(u16 id, u8 *file);
void asset_registry_set(u16 id, u8 *file);
AnimKey *animation_select_keyframe(ModelFile *model, s32 anim, s32 time, s32 *index, s32 *frac);
void animation_expand_sparse_vertices(SVECTOR *pose, SVECTOR *base, s16 *deltas);
void animation_decode_sparse_vertices(SVECTOR *pose, s16 *deltas);
void animation_apply_sparse_morph(SVECTOR *pose, s16 *deltas, s32 t);
s16 *animation_find_sparse_vertex(s16 *deltas, s32 index);
s32 animation_prepare_asset_vertices(AnimSlot **owner, s32 id, s32 anim, s32 time, s32 count);
s32 animation_sample_vertex(s32 id, s32 anim, s32 time, s32 vertex, SVECTOR *out);
s32 asset_vertex_count(s32 id, s32 part);
void pool_reset(void);
void pool_mark_allocated(void);
void pool_record_release(AnimSlot *slot);
void pool_release_all(void);
void pool_release_stale(void);
void pool_allocate(AnimSlot **out);
void map_placed_expand_sources(EffectSpawn *spawn);
s32 menu_fade_transition(s32 level, s32 step);
void menu_show_transition_image(u16 file, u16 entry);

void render_reset_color_overlays(void);
void light_reset_records(void);
void light_refresh_record_rotations(void);
void light_interpolate_record_fields(s32 flags, Record30 *src, Record6c *dst, s32 t);
void light_interpolate_records(s32 flags, Record30 *src, s32 t);
void light_interpolate_color_and_fog(u8 r, u8 g, u8 b, s32 near, s32 far, s16 t);
void view_grid_occlude_walls_ascending(void);
void view_grid_occlude_walls_descending(void);
void view_build_visibility_grid(void);
ScrollRec *vram_scroll_find_free(void);
void vram_scroll_start(RECT *rect, s32 delay, s32 step, s32 mode);
void vram_scroll_update(void);
void fog_set_range(s32 near, s32 far);

void display_initialize(void);
void display_begin_frame(void);
void display_present_frame(void);
void display_set_view_transform(VECTOR *pos, SVECTOR *angle);
void tmd_prepare_primitive_indices(u8 *data);
void tmd_register(u16 id, u8 *data);
void tmd_set_slot(u16 id, u8 *data);
void render_enqueue_textured_tmd(u16 part, s32 depth);

void render_map_cell_object(MapLayer *layer, SVECTOR *pos, u8 flags);
void render_map_cell_layers(s32 x, s32 z, s32 flags);

/* Callees outside the render modules; signatures from these call sites
 * (game/math.h, game/memory.h and game/cd.h declare the others). */
void render_enqueue_tmd_subdivided(u16 part);
void render_enqueue_blended_tmd(u16 part, s32 depth, s32 mode);
void render_enqueue_distant_tmd(u16 part, s32 depth, s32 shift, s32 dist);
void render_enqueue_tmd_fixed_depth(u16 part, s16 depth, s32 shade);
void render_enqueue_map(u16 id);
void render_enqueue_map_subdivided(u16 id);
void render_map_cell_window(void);
void render_active_model_rows(void);
void render_player_weapon(void);
void render_world_model(u8 area, u16 id, VECTOR *pos, SVECTOR *rotation, SVECTOR *scale,
                        AnimSlot **owner, MATRIX *view, u16 anim, u16 time, u8 light2, s16 t,
                        s32 flags, s32 depth);
void render_distant_world_model(u8 area, u16 id, VECTOR *pos, SVECTOR *rotation, SVECTOR *scale,
                                AnimSlot **owner, MATRIX *view, u16 anim, u16 time, u8 light2, s16 t,
                                u8 flags, s32 depth);
void render_scene_and_update_resources(void);
void menu_format_number(s32 value, s32 digits, s32 arg2, s32 arg3, u8 *out);
void tim_upload_images(u8 *file);

#endif
