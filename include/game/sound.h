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
#include <LIBSND.H>

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

typedef struct SoundVoiceHandle {
    s16 voice_id;
    s16 unk2;
} SoundVoiceHandle;

typedef struct SoundState {
    u8 bank_select;         /* +0x000: picks slot 0 or 1 for banks 3..50 */
    u_long *seq_buffer;     /* +0x004 */
    s32 seq_playing;        /* +0x008 */
    s16 seq_id;             /* +0x00c */
    s32 seq_ready;          /* +0x010 */
    VECTOR listener;        /* +0x014 */
    s16 listener_layer_offset; /* +0x024 */
    SVECTOR listener_angle; /* +0x026 */
    SoundBank banks[0xb3];  /* +0x030 */
    SoundVoiceHandle voice_handles[10]; /* +0x5c8 */
    SoundEffect effects[528]; /* +0x5f0 */
    SoundSlot slots[11];    /* +0x1a90 */
} SoundState;

extern SoundState sound_status;

/* Main-RAM buffers the sound slots read VAB bodies into. */
extern u8 sound_magic_vab_buffers[2][0x1000];
extern u8 sound_sequence_vab_buffer[0x1000];
extern u8 sound_map_object_vab_buffers[2][0x1000];
extern u8 sound_actor_vab_buffers[4][0x1000];
extern u8 sound_main_vab_buffer[0x1000];
extern u8 sound_weapon_vab_buffer[0x1000];

/* libsnd sequence attribute table for SsSetTableSize(table, 2, 1). */
extern char ss_sep_area[SS_SEQ_TABSIZ * 2 * 1];

/* The sequence the background music plays from. */
extern u_long seq_data[];

void audio_initialize_runtime(void);
void audio_start_sequence(void);
void audio_stop_sequence(void);
void audio_shutdown(void);
s32 audio_play_spatial(s32 se, VECTOR *pos, s32 unused, s32 range, s32 falloff, s32 note);
s32 audio_play_spatial_default_range(s32 se, VECTOR *pos, s16 unused, s32 note);
s32 audio_play_spatial_range(s32 se, VECTOR *pos, s16 unused, s32 range, s32 falloff, s32 note);
void audio_update_listener(VECTOR *listener, SVECTOR *angle);
void audio_play_sound(s32 se, s32 volume);
void audio_key_on(s32 se, s32 left, s32 right, s32 note);
void cd_request_service_vab(void);
void audio_queue_vab_stream(s32 file, s32 sector, s32 bank);

#endif
