#ifndef GAME_RENDER_H
#define GAME_RENDER_H

/*
 * GAME.EXE render work area (WIP). The game loop clears 0x5e2a words from
 * 0x80199154 as one block; the members below are the ones the render and
 * screen-effect code reaches. GCC CSEs one base for addresses inside it
 * (func_8003cbc8 indexes draw[] and disp[] from the address of fade_hold),
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
    s32 unk4;
    u8 unk8[8];
    s32 prims;      /* +0x10: offset of the primitives from the block start, less 12 */
    s32 prim_count; /* +0x14 */
    u8 unk18[4];
} ModelPart;

/* The data block a ModelFile's vertices offset points at. */
typedef struct ModelData {
    u32 unk0;
    u32 unk4;       /* +0x04: func_8003bb04 draws only parts below unk4 / 2 while loading */
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

/* A placed effect (128 of them follow a frame counter); func_80017aa4 moves
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
    Matrix3x3 unk0;     /* func_800171b0 blends it into Record6c.unk0[0] */
    Matrix3x3 unk14;    /* into Record6c.unk50 */
    CVECTOR unk28;
    u16 unk2c;          /* kept 4096 below unk2e */
    u16 unk2e;
} Record30;

/* The area's light records (func_80017664 copies them out of the first FDAT.T entry). */
extern Record30 DAT_80081c8c[64];

/* The working copy of a Record30: unk0[0] and its three quarter turns. */
typedef struct Record6c {
    Matrix3x3 unk0[4];
    Matrix3x3 unk50;
    CVECTOR unk64;
    u16 unk68;
    u16 unk6a;
} Record6c;

/* A VRAM area copied from unk10 to unk8 every unk1 + 1 frames (func_800351fc). */
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

typedef struct Data80199154 {
    u8 *buffer;                     /* +0x00000: 2D quads per frame; CD load buffer */
    PrimPool pools[2];              /* +0x00004: one per frame, carved from the heap
                                     * (func_80035394); the wipe effect copies the
                                     * screen to pools[1].end */
    PrimPool *pool;                 /* +0x0001c: the pool draw functions allocate from */
    u32 ot_buffer[2][0x2000];       /* +0x00020: OT_LENGTH entries each */
    u32 *ot;                        /* +0x10020 */
    u32 ot2_buffer[2][8];           /* +0x10024: drawn ahead of ot (func_80035700) */
    u32 *ot2;                       /* +0x10064 */
    DRAWENV draw[2];                /* +0x10068 */
    DISPENV disp[2];                /* +0x10120 */
    u8 *unk10148[4];                /* +0x10148: model data by id (func_80035c44) */
    u8 *vertices;                   /* +0x10158: vertex data of the last model registered */
    u8 *models[0x62d];              /* +0x1015c: loaded files by id: models 0..103 stay
                                     * resident, area banks from 256 + area * 32
                                     * (func_80040568), the 32 blocks of an area part
                                     * from 556 + part * 32 (func_800290d8); WIP extent,
                                     * up to slots */
    AnimSlot slots[12];             /* +0x11a10 */
    u8 unk11b00[0x1f44];
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
    s32 unk15b28;                   /* +0x15b28: fog near (func_80035358) */
    s32 unk15b2c;                   /* +0x15b2c: fog far */
    u8 grid[25][25];                /* +0x15b30: map cells around the viewer */
    u8 unk15da1[3];
    s32 unk15da4;                   /* +0x15da4: unk15da8 turns need rebuilding */
    Record6c unk15da8[64];          /* +0x15da8 */
} Data80199154;

extern Data80199154 DAT_80199154;

/* Scratchpad words from +0x118 while an area loads. */
typedef struct ScratchLoad {
    u8 unk0[4];
    u8 *model_wanted;   /* +0x11c: one flag per model id of the area: keep it loaded */
    u8 *sound_wanted;   /* +0x120: one flag per sound bank of the area */
} ScratchLoad;

/* Scratchpad words from +0x118 while a frame is built (func_80034bf4). */
typedef struct ScratchView {
    s32 x;              /* +0x118: map cell x of grid column 0 */
    s32 z;              /* +0x11c: map cell z of grid row 0 */
    u8 grid[25][25];    /* +0x120: copied to Data80199154.grid */
} ScratchView;

typedef union ScratchPhase {
    ScratchLoad load;
    ScratchView view;
} ScratchPhase;

/*
 * The R3000 data cache used as scratchpad RAM at 0x1f800000 (a fixed
 * hardware address). The game keeps per-frame working pointers there.
 */
typedef struct ScratchPad {
    u8 unk0[0x10];
    u8 *model;          /* +0x10: data of the model being drawn */
    u32 unk14;          /* +0x14: func_8003bb04 drops flag 4 within 10240 of unk18 */
    u32 unk18;
    u8 unk1c[8];
    u8 *unk24;          /* +0x24: model data + 12 */
    u8 unk28[0x24];
    u8 *vertices;       /* +0x4c: vertices of the model being drawn */
    u8 unk50[0x24];
    MATRIX unk74;       /* +0x74: map layer transform (func_8003bb04) */
    u8 unk94[0x6c];
    SVECTOR unk100;     /* +0x100: map layer position func_8003be34 hands func_8003bb04 */
    VECTOR unk108;      /* +0x108: origin of the map layer positions */
    ScratchPhase u;     /* +0x118 */
} ScratchPad;

#define SCRATCH ((ScratchPad *)0x1f800000)

/* A 2D overlay sprite record; lists end with visible == 0xff. */
typedef struct SpriteRec {
    u8 visible;     /* 0 hidden; bits 0-2 pick the passes func_80041d9c draws */
    u8 mode;        /* blend rate for func_80041ad4, 0xff opaque */
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
    u8 visible;
    u8 unk1[0x17];
    u16 unk18;
    s16 unk1a;      /* angle, eased toward the viewer's heading */
    u8 unk1c[8];
} Data80081c20;

extern Data80081c20 DAT_80081c20[3];

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

void func_8003c868(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                   u16 tpage, u16 clut, u8 r, u8 g, u8 b);
void func_8003c968(u8 level, u8 inset);
void func_8003c9dc(void);
void func_8003cbc8(void);
void func_8003ccfc(int x0, int y0, int x1, int y1, u8 u, u8 v, s32 w, s32 h, u8 semi,
                   u16 tpage, u16 clut, u8 r, u8 g, u8 b, s32 z);
void func_8003cec4(int x0, int y0, int x1, int y1, u8 semi, u32 *rgb0, u32 *rgb1, u32 *rgb2,
                   u32 *rgb3, s32 z);
void func_8003d03c(int x, int y, u8 u, u8 v, u8 w, u8 h, u8 semi, u16 tpage, u16 clut, u8 r,
                   u8 g, u8 b, s32 z);
void func_8003d280(void);
void func_8003d38c(void);
void func_8003d41c(void);
void func_8003d4ac(void);
void func_8003d568(void);
void func_8003d624(u8 mode, u8 r, u8 g, u8 b);
void func_8003d64c(void);
void func_8003d79c(void);
void func_8003d8ec(s32 r, s32 g, s32 b, s32 scale, s32 which);
void func_8003da10(s32 r, s32 g, s32 b, s32 scale, s32 which);
void func_8003daec(u8 *rgb, s32 x, s32 y, s32 step);

u16 func_80040568(u16 id);
void func_800405e8(u16 id, u8 **out);
void func_80040658(u8 *file);
u8 func_80040694(VECTOR *pos);
u8 func_80040708(VECTOR *pos, s32 radius);
s32 func_800407cc(VECTOR *pos, s32 range_x, s32 range_z);
void func_80040830(s32 file, s32 entry, s32 id);
void func_800408d0(s32 file, s32 entry, s32 bank, s32 count);
void func_800409c8(s32 file, s32 entry, s32 id, s32 count);

void func_80041ad4(SpriteRec *sprite, u16 mode, u8 *rgb);
void func_80041d9c(void);
void func_80041e68(void);
void func_80041eec(s32 message, ...);
void func_80041f8c(SpriteRec *sprite, u8 digit);
void func_80041f9c(void);
void func_800422b8(VECTOR *pos, SVECTOR *angle);

void func_80042b70(u16 id);
void func_80042ba8(u16 id, u8 *file);
void func_80042c64(u16 id, u8 *file);
AnimKey *func_80042cac(ModelFile *model, s32 anim, s32 time, s32 *index, s32 *frac);
void func_80042d70(SVECTOR *pose, SVECTOR *base, s16 *deltas);
void func_80042e34(SVECTOR *pose, s16 *deltas);
void func_80042eb0(SVECTOR *pose, s16 *deltas, s32 t);
s16 *func_8004316c(s16 *deltas, s32 index);
s32 func_800431e8(AnimSlot **owner, s32 id, s32 anim, s32 time, s32 count);
s32 func_8004352c(u16 id, s32 anim, s32 time, s32 vertex, SVECTOR *out);
s32 func_800437c8(s32 id, s32 part);
void func_80043828(void);
void func_80043858(void);
void func_80043894(AnimSlot *slot);
void func_800438e0(void);
void func_80043940(void);
void func_800439b0(AnimSlot **out);
void func_80043a08(EffectSpawn *spawn);

void func_80034180(void);
void func_800341e8(void);
void func_80034300(void);
void func_80034394(s32 flags, Record30 *src, Record6c *dst, s32 t);
void func_800344dc(s32 flags, Record30 *src, s32 t);
void func_8003456c(u8 r, u8 g, u8 b, s32 near, s32 far, s16 t);
void func_800345f4(void);
void func_800348f4(void);
void func_80034bf4(void);
ScrollRec *func_800350c0(void);
void func_800350fc(RECT *rect, s32 delay, s32 step, s32 mode);
void func_800351fc(void);
void func_80035358(s32 near, s32 far);

void func_80035394(void);
void func_80035630(void);
void func_80035700(void);
void func_800357e8(VECTOR *pos, SVECTOR *angle);
void func_8003591c(u8 *data);
void func_80035c44(u16 id, u8 *data);
void func_80035c84(u16 id, u8 *data);

void func_8003bb04(MapLayer *layer, SVECTOR *pos, u8 flags);
void func_8003be34(s32 x, s32 z, s32 flags);

/* Callees outside the render modules; signatures from these call sites
 * (game/math.h, game/memory.h and game/cd.h declare the others). */
void func_80039d50(void);
void func_8003ab04(void);
void func_8003bfd0(void);
void func_8003c35c(void);
void func_8003df50(void);
void func_80040ae4(void);
void func_800277c0(u16 value, s32 digits, s32 arg2, s32 arg3, u8 *out);
void func_80043b38(u8 *file);

#endif
