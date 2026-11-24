#ifndef GAME_DATA_H
#define GAME_DATA_H

/*
 * GAME.EXE globals outside the sound, CD and render headers (WIP), mostly
 * the BSS blocks the game loop clears word by word at start-up. Each extent
 * is the word count main passes to func_80018fbc; whether a block is one
 * object or a run of several is not yet known, so the names stay
 * address-derived and only the fields the matched code touches are named.
 * The block at 0x80199154 is Data80199154 and the effect records at
 * 0x80182968 are Effect, both in game/render.h; the word at 0x801c12f0 is
 * CdState.unkc (game/cd.h).
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <kf/decenv.h>

/* Data801b24e4.unk188 in modes 3, 0x10, 0x11 and 0x12 (func_80030908). */
typedef struct Record10 {
    SVECTOR dir;    /* +0x0 */
    SVECTOR push;   /* +0x8: added to unk134..unk138 (func_8003047c) */
} Record10;

/* Data801b24e4.unk188 in modes 7, 8 and 9 (func_80030810). */
typedef struct Record0a {
    u8 id;          /* +0x0 */
    u8 unk1;
    SVECTOR angle;  /* +0x2: copy of Data801b24e4.unk12c */
} Record0a;

/* Data801b24e4.unk188: one view per unk101 mode (mode 5 counts in count). */
typedef union Data801b24e4Mode {
    Record10 knock;
    Record0a turn;
    s16 count;
} Data801b24e4Mode;

typedef struct Data801b24e4 {
    s32 experience; /* +0x00: the save title shows it as six digits */
    s32 unk4;
    s32 unk8;
    u8 level;       /* +0x0c: the save title shows it as two digits */
    u8 unkd;
    u8 unke;        /* +0x0e: cleared every frame by the game loop */
    u8 unkf;
    u8 unk10;
    u8 unk11;
    s16 unk12;
    s16 unk14;
    u16 hp_max;     /* +0x16: 0..999, from the level table */
    u16 hp;         /* +0x18: 0 is death (func_8002a6a0) */
    u16 unk1a;      /* +0x1a: maximum of unk1c; 0..999, from the level table */
    u16 unk1c;
    u16 unk1e;      /* +0x1e: out of 5000 */
    s16 unk20;
    u16 unk22;      /* +0x22: out of 5000 */
    u16 unk24;      /* +0x24: counts up to 100 for a point of unk32 */
    u16 unk26[5];   /* +0x26: count up to 400 for a point of unk34[n] */
    s16 unk30;
    u16 unk32;      /* +0x32: func_80029500 copies it to unk40 */
    u16 unk34[5];   /* +0x34: func_80029500 copies them to unk42 */
    s16 unk3e;
    u16 unk40;      /* +0x40: 0..999 */
    u16 unk42[5];   /* +0x42: 0..999 each */
    u16 unk4c;      /* +0x4c: the menu shows it plus one (func_800227ec, lhu) */
    u8 unk4e[0x2];
    s32 unk50;
    u16 unk54[7];   /* +0x54: func_80029500 rebuilds them; never above 0x7fff */
    u16 unk62;
    u8 unk64[0x2];
    u16 unk66;      /* +0x66..+0x76: func_80029500 rebuilds them from Record20s */
    u16 unk68;
    u16 unk6a;
    u16 unk6c;
    u16 unk6e;
    u16 unk70;
    u16 unk72;
    u16 unk74;
    u16 unk76;
    s16 unk78;
    s16 unk7a;
    s16 unk7c;
    s16 unk7e;
    s16 unk80;
    s16 unk82;
    s16 unk84;
    s16 unk86;
    s16 unk88;
    s16 unk8a;
    s16 unk8c;
    s16 unk8e;
    s16 unk90;
    s16 unk92;
    s16 unk94;
    u8 unk96[0x6];
    u32 unk9c;      /* +0x9c: frame counter (func_80030cbc periods) */
    s32 unka0;
    u32 play_minutes; /* +0xa4: the VSync handler adds one every 3600 VSyncs */
    u16 *unka8;     /* +0xa8: next pad bit of the DAT_8009c3cc sequence (func_8002fe1c) */
    struct Record18 *unkac; /* +0xac: &DAT_801b77ec.unk0[id] (func_8002d130) */
    struct Record44 *unkb0; /* +0xb0: &DAT_801d37a4[unkcb] */
    u8 *unkb4;      /* +0xb4 */
    s32 unkb8;
    s32 unkbc;
    s16 unkc0;      /* +0xc0: -1 blocks the Object88 script records of ids 19 and 20 */
    u16 unkc2;
    u16 unkc4;
    u8 unkc6;
    u8 unkc7;
    u8 unkc8;
    u8 unkc9;
    u8 unkca;
    u8 unkcb;
    u8 unkcc;
    u8 unkcd[0x1];
    u8 unkce;
    u8 unkcf;
    u8 unkd0;
    u8 unkd1[0x3];
    struct Record20 *unkd4; /* +0xd4..+0xec: &DAT_801d4464.unk11c14[] of unkf0..unkf6 */
    struct Record20 *unkd8;
    struct Record20 *unkdc;
    struct Record20 *unke0;
    struct Record20 *unke4;
    struct Record20 *unke8;
    struct Record20 *unkec;
    u8 unkf0;       /* +0xf0..+0xf6: Record20 indices, 0xff for none */
    u8 unkf1;
    u8 unkf2;
    u8 unkf3;
    u8 unkf4;
    u8 unkf5;
    u8 unkf6;
    u8 se_volume;   /* +0xf7: 0 mutes sound effects; scales them by n/128 */
    u8 bgm_volume;  /* +0xf8: 0 keeps the sequence off */
    u8 unkf9;       /* +0xf9: status overlay layout, bits 0-1 show its two halves */
    u8 unkfa;       /* +0xfa: compass, bits 0-1 show its needles */
    u8 unkfb;
    u8 unkfc;
    u8 unkfd;       /* +0xfd: brightness of the status overlay */
    u8 unkfe;
    u8 unkff;
    u8 unk100;
    u8 unk101;
    u8 unk102[0x2];
    u8 unk104;
    u8 unk105;
    u8 unk106;
    u8 unk107;
    u8 unk108;
    u8 unk109;
    u8 unk10a;
    u8 unk10b[0x1];
    VECTOR pos;     /* +0x10c: func_80017aa4 moves it with the objects */
    SVECTOR unk11c; /* +0x11c */
    SVECTOR unk124; /* +0x124 */
    SVECTOR unk12c; /* +0x12c */
    s16 unk134;
    s16 unk136;
    s16 unk138;
    s16 unk13a;
    s16 unk13c;
    s16 unk13e;
    s16 unk140;
    u8 unk142[0x2];
    s16 unk144;
    s16 unk146;
    s16 unk148;
    u8 unk14a[0x2];
    SVECTOR unk14c; /* +0x14c: func_8002c164 rotates it by unk124 */
    s32 unk154;     /* +0x154: top edge of the full-screen overlay pair; >= 240 hides it */
    s32 unk158;     /* +0x158: top edge of a single overlay; >= 240 hides it */
    s32 unk15c;     /* +0x15c: top edge of a single overlay; >= 240 hides it */
    u16 unk160;
    s16 unk162;
    s16 unk164;
    s16 unk166;
    s16 unk168;
    s16 unk16a;
    s16 unk16c;
    s16 unk16e;
    s16 unk170;
    s16 unk172;
    s16 unk174;
    s16 unk176;
    u16 unk178;     /* +0x178: pad bits held (DAT_80081868) */
    u16 unk17a;     /* +0x17a: pad bits held the frame before */
    u8 unk17c[0x4];
    s32 unk180;
    s32 unk184;
    Data801b24e4Mode unk188; /* +0x188: parameters of the unk101 mode */
    u8 unk198[0x4];
} Data801b24e4;

typedef struct Data801ba984 {
    s32 unk0;
    u8 unk4[9];
    u8 unkd;        /* +0x0d: the game loop sets it to 1 on a continued game */
    u8 unke[0x1d];
    u8 unk2b;
    u8 unk2c[5];
    u8 unk31;
    u8 unk32[0x33];
    u8 unk65;
    u8 unk66[0xb];
    u8 unk71[5];    /* +0x71: per magic element 0..3; element 4 needs none */
    u8 unk76[9];
    u8 unk7f[5];    /* +0x7f: per magic element */
    u8 unk84[0x26];
    u8 unkaa;       /* +0xaa: Object88.unk1 of the record whose script func_8005c1e8 entered */
    u8 unkab[0x59];
    u8 unk104[0x5000]; /* +0x104: block arena (game/memory.h), cleared by func_8005ea64 */
    u16 unk5104[32]; /* +0x5104: arena offsets of 32 pointers, 0xffff for NULL (func_8005ef18) */
} Data801ba984;

/* 0x20-byte records func_800293e4 adds to the player's halfwords +0x66..+0x76. */
typedef struct Record20 {
    u8 unk0[2];
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u16 unk8;
    u16 unka;
    u16 unkc;
    u16 unke;
    u16 unk10;
    u16 unk12;
    u16 unk14;      /* +0x14..+0x1a: frame periods of the hp/unk1c gain and loss */
    u16 unk16;
    u16 unk18;
    u16 unk1a;
    u8 unk1c[0x4];
} Record20;

/*
 * One layer of a map cell; func_8003bb04 draws one (field names WIP).
 */
typedef struct MapLayer {
    u8 unk0;        /* +0x00: below 240 the layer is drawn; 0xfd..0xff are walls */
    u8 unk1;        /* +0x01: floor height / -128 */
    u8 unk2;        /* +0x02: layer 0: bits 2-7 count the Object88 records on the
                     * cell (func_80033c4c) */
    u8 unk3;
    u8 unk4;
} MapLayer;

/*
 * One 2048-unit square of the area map (80 x 80 of them start DAT_801d4464,
 * indexed [z >> 11][x >> 11]): layer 0 for area 1, layer 1 for area 2.
 */
typedef struct MapCell {
    MapLayer layer[2];
} MapCell;

typedef struct Data801d4464 {
    MapCell cells[80][80];
    u8 unkfa00[0x2214];
    Record20 unk11c14[31]; /* +0x11c14: WIP count */
    u8 unk11ff4[0xc];
    MapCell *unk12000; /* +0x12000: cell of the last collision test */
    u8 *unk12004;   /* +0x12004: record of the last collision test; [2] bits 0-1 its slope */
    u8 unk12008[2];
    u16 unk1200a;   /* +0x1200a: copied to SoundState.unk24 with the listener */
    s32 unk1200c;   /* +0x1200c: height given to Object88 records with flag 0x400 */
    s32 unk12010;   /* +0x12010: floor height of the last collision test */
    u8 unk12014[4];
    s32 unk12018[3]; /* +0x12018: heights func_80028d54 measures the player against; [0] the
                      * floor the Object4c spawns of func_80056268 must reach */
    u32 unk12024;   /* +0x12024: hit bits of the last func_80033f38 test */
    s32 unk12028;   /* +0x12028: Object88 index it hit, -1 for none (func_800533e8 reads it) */
    s32 unk1202c;   /* +0x1202c: Object44 index it hit, -1 for none */
    u8 unk12030[0xc];
    VECTOR unk1203c; /* +0x1203c: point of the last wall hit (func_80033d84) */
    u16 unk1204c;   /* +0x1204c: radius of what was hit (func_80033d84) */
    u16 unk1204e;   /* +0x1204e: height of what was hit */
    u16 unk12050;
    u8 unk12052[2];
} Data801d4464;

typedef struct Data801e7edc {
    u8 unk0[0x9c];
    u8 unk9c[0x2a4]; /* +0x9c: the in-game movie records (unk380 points into them) */
    DECENV dec;     /* +0x340: the in-game movie player's decode environment */
    s16 unk370;
    s16 unk372;     /* +0x372: 99 once the stream passed the movie's last frame */
    u_long *unk374[2]; /* +0x374: VLC buffers func_80061364 hands to dec */
    u_long *unk37c; /* +0x37c: slice buffer */
    u8 *unk380;     /* +0x380: the movie's record; u16 [1] its last frame */
    u8 unk384;      /* +0x384: movie id, 0xff for none */
    u8 unk385[3];
    s32 unk388;     /* +0x388: frame number of the last sector (func_800616dc) */
} Data801e7edc;

/*
 * Object records func_80017aa4 moves when the area origin shifts (WIP: named
 * by size; 0xff/0xffff in the first field marks a free record). The 24-byte
 * records it also moves are the placed effects (Effect, game/render.h).
 */
/* Object44 +0x3a as the FDAT module handlers read it (WIP). */
typedef union Object44Link {
    u16 index;      /* slot 0: a linked object, 0xffff for none; kind 7 (func_80044c34) */
    u8 bytes[2];    /* slot 3 compares bytes[1] with a per-module value; func_80044900 reads
                     * bytes[0] and bytes[1] as angles */
} Object44Link;

/*
 * Per-kind state of an Object44 from +0x34 (WIP views, by Object44.unk4): the
 * GAME and FDAT code read it as bytes or halfwords by kind; func_8004485c
 * clears the first three words.
 */
typedef struct Object44Bytes {
    u8 unk34[4];
    u8 unk38;       /* +0x38: radius scale, 0x80 = 1 (flag 0x10) */
    u8 unk39;       /* +0x39: func_8005c7e8 stores the index - 124 of the record it spawns */
    Object44Link unk3a; /* +0x3a */
    u8 unk3c;       /* +0x3c: FDAT slot 0 acts only when set */
    u8 unk3d[2];
    u8 unk3f;       /* +0x3f: FDAT slot 0 shows this message when below 35 */
    u8 unk40;       /* +0x40: kind 0x70 driver: steps taken (func_80046914); FDAT slot 0
                     * copies it to the linked object's unk0 */
    u8 unk41[3];
} Object44Bytes;

typedef struct Object44Halves {
    u16 unk34[2];
    u16 unk38;
    u16 unk3a;
    u16 unk3c;      /* +0x3c: age stamp func_80046034 replaces the oldest by */
    s16 unk3e;
    u16 unk40;      /* +0x40: kind 0x70: index of its driver (func_80046914) */
    u16 unk42;
} Object44Halves;

typedef struct Object44Words {
    s32 unk34;
    s32 unk38;
    s32 unk3c;
    s32 unk40;      /* +0x40: kind 0x10: height it started from */
} Object44Words;

typedef union Object44Arg {
    Object44Bytes b;
    Object44Halves h;
    Object44Words w;
} Object44Arg;

typedef struct Object44 {
    u8 unk0;        /* +0x00: area (1 or 2): picks the map cell layer */
    u8 unk1;        /* +0x01: 0x80 after set-up */
    u8 unk2;
    u8 unk3;        /* +0x03: flags; 0x10 scales its radius by arg.b.unk38, 0x4 takes Object44Def.unke/unk10 */
    u8 unk4;        /* +0x04: kind, 0xff when free */
    u8 unk5;
    u16 unk6;       /* +0x06: index of its Object44Def; 0xff when free */
    u16 unk8;       /* +0x08: FDAT_023 clears it when the key is used */
    u16 unka;       /* +0x0a: FDAT_017 tests it for 0 or >= 4095 */
    u16 unkc;       /* +0x0c: reach below its position (collision tests) */
    s16 unke;
    u16 unk10;      /* +0x10: FDAT_047 raises it by 256 a frame up to 0xfff */
    u8 unk12[2];
    VECTOR pos;     /* +0x14 */
    SVECTOR angle;  /* +0x24 (func_8005bee0) */
    SVECTOR scale;  /* +0x2c: 0x1000 = 1; FDAT_017 clears vx, vy and vz */
    Object44Arg arg; /* +0x34 */
} Object44;

/* The 0x18-byte records Object44.unk6 indexes (WIP). */
typedef struct Object44Def {
    u8 unk0;        /* +0x00: type (func_80044900) */
    u8 unk1;        /* +0x01: picks the kind func_800460bc gives a spawned record */
    u8 unk2;
    u8 unk3;        /* +0x03: Object44.unk3 flags at set-up */
    u16 unk4;       /* +0x04: radius */
    u16 unk6;       /* +0x06: radius of the turned kinds (func_80045c7c) */
    u16 unk8;       /* +0x08: height */
    u16 unka;
    u8 unkc;        /* +0x0c: map layer byte kind 7 sets, 0xff for none */
    u8 unkd;
    u16 unke;
    u16 unk10;
    u8 unk12[5];
    u8 unk17;       /* +0x17: map layer byte kinds 0x51/0x52 set */
} Object44Def;

typedef struct Object4c {
    u8 unk0;        /* +0x00: 0xff when free; bits 0-2 pick the collision mask,
                     * 0x40 averages the Record18.unk5 stats (func_80053624) */
    u8 unk1;        /* +0x01: index of its Record18 record */
    u8 unk2;
    u8 unk3;        /* +0x03: model id - 40 (func_80053a0c) */
    u8 unk4;        /* +0x04: animation */
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;        /* +0x08: both of bits 2 and 3 set keeps it in place */
    u8 unk9;
    u8 unka;
    u8 unkb;        /* +0x0b: frames before the collision test (func_80053310) */
    u8 unkc;
    u8 unkd;
    u16 unke;
    s16 unk10;
    u16 unk12;
    u16 unk14;      /* +0x14: animation time */
    u8 unk16[2];
    VECTOR pos;     /* +0x18 */
    SVECTOR angle;  /* +0x28 */
    SVECTOR scale;  /* +0x30: 0x1000 = 1 */
    SVECTOR vel;    /* +0x38 */
    u8 unk40[0xc];
} Object4c;

typedef struct Object88 {
    u8 unk0;        /* +0x00: 0xff when free */
    u8 unk1;        /* +0x01: Object78.unk0 of its record */
    u8 unk2;        /* +0x02: index of its Object78 record */
    u8 unk3;
    u8 unk4;
    u8 unk5;        /* +0x05: bit 0 keeps its initial yaw */
    u8 unk6;        /* +0x06: area passed to func_80033b8c */
    u8 unk7;        /* +0x07: map cell z */
    u8 unk8;        /* +0x08: map cell x */
    u8 unk9;        /* +0x09: state, 1 active */
    u8 unka;
    u8 unkb;
    u8 unkc;        /* +0x0c: animation */
    u8 unkd;
    u8 unke;        /* +0x0e: id of the running script record, 0xff none */
    u8 unkf;        /* +0x0f: script step, 0xff restart */
    u8 unk10;       /* +0x10: previous unke */
    u8 unk11;
    u8 unk12;
    u8 unk13;
    u8 unk14;
    u8 unk15;
    s16 unk16;      /* +0x16: FDAT slot 17 raises and lowers it (-12000 at the start) */
    u16 unk18;      /* +0x18: animation time */
    u16 unk1a;
    u16 unk1c;
    u16 unk1e;
    u16 unk20;      /* +0x20: initial yaw */
    s16 unk22;      /* +0x22: z offset in its cell */
    s16 unk24;      /* +0x24: x offset in its cell */
    s16 unk26;      /* +0x26: height above the floor */
    u32 unk28;      /* +0x28: flags (Object78.unk34) */
    VECTOR pos;     /* +0x2c */
    s32 unk3c;
    SVECTOR angle;  /* +0x40 */
    s16 unk48;      /* +0x48: scale, 0x1000 = 1 */
    s16 unk4a;
    s16 unk4c;
    u16 unk4e;
    SVECTOR vel;    /* +0x50 */
    s16 unk58;
    u8 unk5a[2];
    u32 unk5c;
    u8 *unk60;      /* +0x60: running script record */
    s16 unk64;      /* +0x64: FDAT slot 17 stores the angle to the player */
    s16 unk66;
    s16 unk68;
    s16 unk6a;
    s16 unk6c;
    u8 unk6e[2];
    s16 unk70;      /* +0x70: FDAT slot 17 phase 0..2 */
    u8 unk72[2];
    s16 unk74;      /* +0x74: FDAT slot 17 toggles it when the turn completes */
    u8 unk76[0x12];
} Object88;

/*
 * The record an Object88 is made from (its unk2 indexes DAT_80185da8.defs);
 * func_8002c164 hands one to func_8004c068.
 */
typedef struct Object78 {
    u8 unk0;
    u8 unk1[4];
    u8 unk5;        /* +0x05: fall acceleration */
    u8 unk6;
    u8 unk7[2];
    u8 unk9;
    u8 unka;        /* +0x0a: wake range in map cells */
    u8 unkb;        /* +0x0b: sleep range in map cells */
    s16 unkc;       /* +0x0c: offset of the record's point (Object88.unk28 bits 0-1) */
    s16 unke;
    s16 unk10;
    u16 unk12;
    u16 unk14;
    u8 unk16[2];
    s16 unk18;      /* +0x18: yaw range (func_800533e8) */
    u16 unk1a;
    u16 unk1c;
    u8 unk1e[0x14];
    u16 unk32;
    u32 unk34;      /* +0x34: flags copied to Object88.unk28 */
    u8 *scripts[16]; /* +0x38: script records, the first byte their id; 0 ends */
} Object78;

typedef struct Data80185da8 {
    Object88 objects[200];
    Object78 defs[40];      /* +0x6a40 */
    u8 unk7d00[0x2000];
    u8 unk9d00;             /* +0x9d00: Object88.unk28 flags the searches skip */
    u8 unk9d01[3];
    u16 unk9d04;            /* +0x9d04: passed to the collision tests */
    u8 unk9d06[2];
    Object78 *def;          /* +0x9d08: record of current */
    Object88 *current;      /* +0x9d0c: the record being updated */
    Object78 *target_def;   /* +0x9d10: record of target */
    Object88 *target;       /* +0x9d14: objects[current->unk22] when flag 0x10 */
    s32 unk9d18;            /* +0x9d18: index of current */
    s32 unk9d1c;            /* +0x9d1c: current->unk2 */
    s32 unk9d20;            /* +0x9d20: active records updated this frame */
    s32 unk9d24;            /* +0x9d24: frame counter */
    Object88 *unk9d28;      /* +0x9d28: func_8002c164 keeps its last object here */
} Data80185da8;

/* 0x18-byte records at the start of DAT_801b77ec (WIP). */
typedef struct Record18 {
    u8 unk0;        /* +0x00: func_80029f1c sets it to 1 */
    u8 unk1;
    u8 unk2;        /* +0x02: SoundState.effects[unk2 + 144], 0xff for none */
    u8 unk3;
    u8 unk4;
    u8 unk5;        /* +0x05: bits 0-4 pick Data801b24e4.unk42[0..4] */
    u16 unk6;
    u16 unk8;
    u16 unka;
    u16 unkc;
    u16 unke;
    u16 unk10;
    u16 unk12;
    u16 unk14;
    u16 unk16;      /* +0x16: unk1c cost of the spell (func_8002d130); func_8001c6c0 lists it */
} Record18;

typedef struct Data801b77ec {
    Record18 unk0[96];
    Object4c objects[128];  /* +0x900 */
    Record18 *unk2f00;      /* +0x2f00: record of unk2f04 */
    Object4c *unk2f04;      /* +0x2f04: the Object4c record being updated */
    s32 unk2f08;            /* +0x2f08: index of unk2f04 (func_8005bc50) */
} Data801b77ec;

/* 12-byte level records (DAT_8009f114, one per level). */
typedef struct Record0c {
    u16 unk0;       /* new Data801b24e4.hp_max */
    u16 unk2;       /* new Data801b24e4.unk1a */
    u16 unk4;       /* added to Data801b24e4.unk32 */
    u16 unk6;
    s32 unk8;       /* experience for the next level */
} Record0c;

/* 0x44-byte records func_80029500 adds to the player's unk54[]/unk62 (WIP). */
typedef struct Record44 {
    u8 unk0[4];
    u8 unk4;
    u8 unk5;
    u16 unk6[8];
    u8 unk16[8];
    u16 unk1e;
    u8 unk20[4];
    u16 unk24;
    u16 unk26;
    u8 unk28[4];
    u16 unk2c;
    u16 unk2e;
    u8 unk30[4];
    u16 unk34;      /* +0x34..+0x42: model offsets of the weapon tip (func_8002cea8) */
    u16 unk36;
    u16 unk38;
    s16 unk3a;
    u16 unk3c;
    u16 unk3e;
    u16 unk40;
    s16 unk42;
} Record44;

/* One entry of a loaded FDAT module's handler table. */
typedef s32 (*FdatHandler)();

/*
 * Area state func_80017664 and func_8001796c set up (WIP). Only the scene
 * module sees the step the CD callbacks set as volatile: the menu module's
 * accesses to the same object show none of that (a volatile member changes
 * how GCC 2.6 reaches every member of the aggregate; see game/cd.h).
 */
#ifdef GAME_SCENE_MODULE
#define SCENE_STEP_SHARED volatile
#else
#define SCENE_STEP_SHARED
#endif

typedef struct Data8018fad4 {
    u16 loading;    /* +0x00: func_8001796c steps func_80018358 until it clears */
    SCENE_STEP_SHARED s16 step; /* +0x02: the CD callbacks func_800182e0.. set it */
    u8 area[5];     /* +0x04: 99 asks for a reload, 0xff keeps a part; [0] picks
                     * the model id bank (func_80040568) */
    u8 unk9;
    u8 unka[2];
    FdatHandler *handlers; /* +0x0c: the loaded FDAT module's handler table */
    u8 previous[5]; /* +0x10: [0] is the model id bank while loading, 0xff for none */
    u8 unk15;
    u8 unk16;       /* +0x16: func_8003bb04 limits the parts drawn while loading */
    u8 unk17;       /* +0x17 */
    u8 unk18;
    s8 shift[3];    /* +0x19: 0x7f is no shift */
    u8 unk1c[0x4c];
} Data8018fad4;

typedef struct Data8018fb3c {
    Object44Def defs[332];  /* WIP count: fills the space before objects */
    Object44 objects[0x18c];
    u8 unk8850[8];
    Object44 *unk8858;      /* +0x8858: the record func_80045ac8 does not test; the object
                             * FDAT slot 9 handlers test */
    u8 unk885c[2];
    u16 unk885e;
    u16 unk8860;
    u16 unk8862;
    u8 unk8864[4];
} Data8018fb3c;

/*
 * FDAT window words in the load image (owner unit unknown): [2] the window
 * base, [4] the end of the loaded module, [5] the heap base main stores.
 */
extern u32 DAT_800136e8[6];

/* Arena func_8001937c formats with a 0x4d800-byte free block. */
extern u8 DAT_800c8954[0x4d800];

extern Data80185da8 DAT_80185da8;
extern Data8018fad4 DAT_8018fad4;
extern Data8018fb3c DAT_8018fb3c;
extern Data801b24e4 DAT_801b24e4;
/* Area map bits: rooms seen per area (32 areas x 160 bits) and the 80 x 80 grid of
 * visited cells func_8001bea8 marks rooms from. */
extern u8 DAT_801b56cc[32][20];
extern u8 DAT_801b594c[80][10];
extern Data801b77ec DAT_801b77ec;

/* A record of the table func_80017664 copies in (DAT_801983a8.unk0). */
typedef struct Record04 {
    u8 id;          /* 0xff ends the table */
    u8 count;       /* slots in its run of DAT_801983a8.unk1e0 */
    u16 unk2;
} Record04;

/* A record func_80060800 builds from a Record04 (DAT_801983a8.unkc0). */
typedef struct Record06 {
    u8 id;          /* 0xff ends the table */
    u8 unk1;
    u16 unk2;       /* Record04.unk2 */
    u16 offset;     /* start of its run in DAT_801983a8.unk1e0, 0xffff for none */
} Record06;

/*
 * Slot tables (WIP extent, up to DAT_80199148): the copied records, the
 * records func_80060800 builds from them and their runs of slot bytes
 * (bits 0-1 set: in use; 0xff: free or the end of a run).
 */
typedef struct Data801983a8 {
    Record04 unk0[48];
    Record06 unkc0[48];  /* +0xc0 */
    u8 unk1e0[0xbc0];    /* +0x1e0 */
} Data801983a8;
extern Data801983a8 DAT_801983a8;
extern Data801ba984 DAT_801ba984;
/* Two counts per item id (0..149; func_8005d898 caps each at 99), cleared by func_8005ea64. */
extern u8 DAT_800c85e8[2][150];
extern Data801d4464 DAT_801d4464;
extern Data801e7edc DAT_801e7edc;

/*
 * Tables func_80017664 copies out of the first FDAT.T entry: the level
 * records (one per level; 100 fill the gap to DAT_8009f5c4), the 0x44-byte
 * records (48 fill the gap to DAT_801d4464), and the rest (WIP extents).
 */
extern Record0c DAT_8009f114[100];
extern Record44 DAT_801d37a4[48];
extern u32 DAT_800aedc4[];
extern u32 DAT_80116154[];
extern u32 DAT_801e64b8[];
extern u8 DAT_8014f164[];

/* What func_8002b64c points Data801b24e4.unkb4 at (WIP extent). */
extern u8 DAT_8009f5c4[];

#endif
