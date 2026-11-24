#ifndef GAME_SOUND_H
#define GAME_SOUND_H

/*
 * GAME.EXE sound state (src/game/sound.c): the libsnd sequence, the listener
 * for positional effects, the VAB banks, the effect table and the
 * main-RAM buffers VAB bodies are read into. Field names are WIP.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

/* SoundBank.vab_id values besides a libsnd VAB id. */
#define SOUND_VAB_NONE (-1)
#define SOUND_VAB_LOADING 0xfe

typedef struct SoundSlot {
    u8 *buffer;
    s16 state; /* 0 free, 2 loaded, 3 loading */
    s16 pad;
} SoundSlot;

typedef struct SoundBank {
    SoundSlot *slot;
    s16 vab_id;
    s16 pad;
} SoundBank;

typedef struct SoundEffect {
    s16 bank;
    s16 program;
    s16 tone;
    s16 note;
    s16 unk8;
} SoundEffect;

typedef struct SoundUnk5c8 {
    s16 unk0;
    s16 unk2;
} SoundUnk5c8;

typedef struct SoundState {
    u8 bank_select;         /* +0x000: picks slot 0 or 1 for banks 3..50 */
    u_long *seq_data;       /* +0x004 */
    s32 seq_playing;        /* +0x008 */
    s16 seq_id;             /* +0x00c */
    s32 seq_ready;          /* +0x010 */
    VECTOR listener;        /* +0x014 */
    s16 unk24;              /* +0x024 */
    SVECTOR listener_angle; /* +0x026 */
    SoundBank banks[0xb3];  /* +0x030 */
    SoundUnk5c8 unk5c8[10]; /* +0x5c8 */
    SoundEffect effects[528]; /* +0x5f0 */
    SoundSlot slots[11];    /* +0x1a90 */
} SoundState;

extern SoundState DAT_801b09fc;

void func_8001509c(void);
void func_80015240(void);
void func_800152f8(void);
void func_80015354(void);
s32 func_80015410(s32 se, VECTOR *pos, s16 unused, s32 range, s32 falloff, s32 note);
s32 func_80015654(s32 se, VECTOR *pos, s16 unused, s32 note);
s32 func_80015688(s32 se, VECTOR *pos, s16 unused, s32 range, s32 falloff, s32 note);
void func_800156bc(VECTOR *listener, SVECTOR *angle);
void func_8001576c(s32 se, s32 volume);
void func_80015790(s32 se, s32 left, s32 right, s32 note);
void func_80015a48(void);
void func_80015ce0(s32 file, s32 sector, s32 bank);

#endif
