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
#include <LIBGTE.H>
#include <LIBGPU.H>

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

typedef struct Data80199154 {
    u8 *buffer;                     /* +0x00000: 2D quads per frame; CD load buffer */
    u8 *heap;                       /* +0x00004: base the packet pool is carved from */
    PrimPool pool0;                 /* +0x00008 */
    u8 *image;                      /* +0x00014: screen copy for the wipe effect */
    u8 unk18[4];
    PrimPool *pool;                 /* +0x0001c: the pool draw functions allocate from */
    u32 ot_buffer[2][OT_LENGTH];    /* +0x00020 */
    u32 *ot;                        /* +0x10020 */
    u8 unk10024[0x44];
    DRAWENV draw[2];                /* +0x10068 */
    DISPENV disp[2];                /* +0x10120 */
    u8 unk10148[0x14];
    u8 *models[0x100];              /* +0x1015c: loaded model files by id; 0..103 stay resident */
    u8 unk1055c[0x5428];
    u16 message_arg[8];             /* +0x15984: number shown with message 25 */
    u8 frame;                       /* +0x15994: index of draw[]/disp[] in use */
    u8 unk15995[2];
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
    u8 unk159c8[0x148];
    s32 unk15b10;                   /* +0x15b10: map cell x of the viewer */
    s32 unk15b14;                   /* +0x15b14: map cell z of the viewer */
    s32 grid_x;                     /* +0x15b18: map cell x of grid column 0, negated */
    s32 grid_z;                     /* +0x15b1c: map cell z of grid row 0, negated */
    u8 unk15b20[0x10];
    u8 grid[25][25];                /* +0x15b30: map cells around the viewer */
    u8 unk15da1[0x178a8 - 0x15da1];
} Data80199154;

extern Data80199154 DAT_80199154;

/*
 * The R3000 data cache used as scratchpad RAM at 0x1f800000 (a fixed
 * hardware address). The game keeps per-frame working pointers there.
 */
typedef struct ScratchPad {
    u8 unk0[0x11c];
    u8 *model_wanted;   /* +0x11c: one flag per model id of the area: keep it loaded */
    u8 *sound_wanted;   /* +0x120: one flag per sound bank of the area */
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

/* Callees outside the render modules; signatures from these call sites. */
u8 *func_8001939c(u8 *arena, s32 size, u8 **owner);
void func_80019474(u8 *block, u8 state);
u8 func_8001947c(u8 *block);
void func_8001949c(u8 *block, u16 id);
void func_800277c0(u16 value, s32 digits, s32 arg2, s32 arg3, u8 *out);
void func_80019d48(s32 file, s32 entry, u8 *buffer, void (*done)(u8 *buffer));
s32 func_80019f14(s32 file, s32 entry);
void func_80035630(void);
void func_80035700(void);
void func_8003591c(u8 *data);
void func_80043b38(u8 *file);

#endif
