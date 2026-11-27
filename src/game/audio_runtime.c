/*
 * GAME.EXE: sound (KF2's audio_runtime.c): libsnd start-up and shutdown, the
 * background sequence, positional and plain sound effects, and VAB bank loading
 * through CD read requests. It references no small data, so -G8 and -G0 compile
 * it alike; the CD request pointers it reads are members of the CD state
 * aggregate (game/cd.h), reached absolutely either way.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/kernel.h>
#include <LIBSPU.H>
#include <game/math.h>
#include <game/sound.h>
#include <game/cd.h>
#include <game/data.h>
#include <game/funcs.h>

DATA(0x801b09fc, 0x1ae8, ".bss")
SoundState sound_status;

ADDRESS(0x8001509c, 0x1a4)
void audio_initialize_runtime(void)
{
    SoundBank *bank;
    SoundVoiceHandle *unk;
    SoundSlot *slot;
    s32 i;

    SsInit();
    SsSetMVol(0, 0);
    SsSetTableSize(ss_sep_area, 2, 1);
    SsSetTickMode(SS_TICK60);
    SsStart2();
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsUtSetReverbDepth(40, 40);
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    sound_status.bank_select = 0;
    sound_status.seq_buffer = seq_data;
    sound_status.seq_playing = 0;
    sound_status.seq_ready = 0;
    bank = sound_status.banks;
    for (i = 0xb2; i != -1; i--) {
        bank->vab_id = SOUND_VAB_NONE;
        bank->slot = 0;
        bank++;
    }
    unk = sound_status.voice_handles;
    for (i = 9; i != -1; i--) {
        unk->voice_id = -1;
        unk++;
    }
    slot = &sound_status.slots[0];
    for (i = 0; i < 2; i++) {
        slot->state = 0;
        slot->buffer = sound_magic_vab_buffers[i];
        slot++;
    }
    slot = &sound_status.slots[2];
    for (i = 0; i < 4; i++) {
        slot->state = 0;
        slot->buffer = sound_actor_vab_buffers[i];
        slot++;
    }
    slot = &sound_status.slots[6];
    for (i = 0; i < 2; i++) {
        slot->state = 0;
        slot->buffer = sound_map_object_vab_buffers[i];
        slot++;
    }
    sound_status.slots[8].buffer = sound_weapon_vab_buffer;
    sound_status.slots[9].buffer = sound_sequence_vab_buffer;
    sound_status.slots[10].buffer = sound_main_vab_buffer;
}

ADDRESS(0x80015240, 0xb8)
void audio_start_sequence(void)
{
    if (cur_avatar_status.bgm_volume != 0 && sound_status.seq_ready != 0
        && sound_status.banks[1].vab_id != SOUND_VAB_NONE) {
        sound_status.seq_id = SsSeqOpen(sound_status.seq_buffer, sound_status.banks[1].vab_id);
        SsSeqSetVol(sound_status.seq_id, cur_avatar_status.bgm_volume, cur_avatar_status.bgm_volume);
        SsSeqPlay(sound_status.seq_id, SSPLAY_PLAY, SSPLAY_INFINITY);
        sound_status.seq_playing = 1;
        SsSetMVol(127, 127);
    }
}

ADDRESS(0x800152f8, 0x5c)
void audio_stop_sequence(void)
{
    if (sound_status.seq_playing == 1) {
        SsSeqStop(sound_status.seq_id);
        SsSeqClose(sound_status.seq_id);
        sound_status.seq_playing = 0;
    }
}

ADDRESS(0x80015354, 0xbc)
void audio_shutdown(void)
{
    SoundBank *bank;
    s32 i;

    SsSetMVol(0, 0);
    if (sound_status.seq_playing == 1) {
        SsSeqSetVol(sound_status.seq_id, 0, 0);
        SsSeqStop(sound_status.seq_id);
        SsSeqClose(sound_status.seq_id);
    }
    bank = sound_status.banks;
    for (i = 0xb2; i != -1; i--) {
        if (bank->vab_id != SOUND_VAB_NONE) {
            SsVabClose(bank->vab_id);
            bank->vab_id = SOUND_VAB_NONE;
        }
        bank++;
    }
    SsEnd();
}

/*
 * `angle = angle < 0 ? ... : angle` assigns the conditional to its own
 * operand, so GCC builds it in a temporary and copies it back (retail's
 * `move v0,a0 ... move a0,v0`); `base` is the near arm's separate
 * `volume + 32` temporary.
 */
ADDRESS(0x80015410, 0x244)
s32 audio_play_spatial(s32 se, VECTOR *pos, s32 unused, s32 range, s32 falloff, s32 note)
{
    s32 dx = (pos->vx - sound_status.listener.vx) >> 3;
    s32 dy = (pos->vy - sound_status.listener.vy) >> 3;
    s32 dz = (pos->vz - sound_status.listener.vz) >> 3;
    s32 near = se & 0x8000;
    s32 distance;
    s32 volume;
    s32 left;
    s32 right;
    s32 angle;

    se &= 0xfff;
    distance = SquareRoot0(dx * dx + dy * dy + dz * dz) << 3;
    if (distance >= range) {
        return 0;
    }
    volume = ((falloff - distance) << 7) / falloff;
    if (volume <= 0) {
        return 0;
    }
    if (volume >= 128) {
        volume = 127;
    }
    if (!near) {
        left = volume + (127 - volume) / 4;
    } else {
        s32 base = volume + 32;

        left = base + (127 - volume) / 2;
        if (left >= 128) {
            left = 127;
        }
    }
    angle = vector_xz_to_angle(pos->vx - sound_status.listener.vx, pos->vz - sound_status.listener.vz)
         - sound_status.listener_angle.vy;
    angle = angle < 0 ? angle + 0x1000 : angle;
    right = left;
    if (angle < 2048) {
        left -= (left * rsin(angle) >> 12) * (127 - volume) / 128;
    } else {
        right = left + (left * rsin(angle) >> 12) * (127 - volume) / 128;
    }
    audio_key_on(se, right, left, note);
    return 1;
}

ADDRESS(0x80015654, 0x34)
s32 audio_play_spatial_default_range(s32 se, VECTOR *pos, s16 unused, s32 note)
{
    return audio_play_spatial(se, pos, unused, 0x5000, 0x4800, note);
}

ADDRESS(0x80015688, 0x34)
s32 audio_play_spatial_range(s32 se, VECTOR *pos, s16 unused, s32 range, s32 falloff, s32 note)
{
    return audio_play_spatial(se, pos, unused, range, falloff, note);
}

ADDRESS(0x800156bc, 0xb0)
void audio_update_listener(VECTOR *listener, SVECTOR *angle)
{
    if (listener != 0) {
        sound_status.listener = *listener;
        collision_sample_map_cell_layer(listener->vx, listener->vy, listener->vz);
        sound_status.listener_layer_offset = map_world.collision_layer_offset;
    }
    if (angle != 0) {
        sound_status.listener_angle = *angle;
    }
}

ADDRESS(0x8001576c, 0x24)
void audio_play_sound(s32 se, s32 volume)
{
    audio_key_on(se, volume, volume, 0);
}

ADDRESS(0x80015790, 0x178)
void audio_key_on(s32 se, s32 left, s32 right, s32 note)
{
    char status[24];
    SoundEffect *effect;
    SoundBank *bank;
    s32 voice;

    if (se == 0xff) {
        return;
    }
    if (cur_avatar_status.se_volume == 0) {
        return;
    }
    se &= 0x3ff;
    effect = &sound_status.effects[se];
    if (effect->bank == SOUND_VAB_NONE) {
        return;
    }
    if (se >= 96 && se < 144) {
        bank = &sound_status.banks[2];
    } else {
        bank = &sound_status.banks[effect->bank];
    }
    if (bank->vab_id == SOUND_VAB_NONE || bank->vab_id == SOUND_VAB_LOADING) {
        return;
    }
    left = left * cur_avatar_status.se_volume >> 7;
    right = right * cur_avatar_status.se_volume >> 7;
    note += effect->note;
    if (note < 0) {
        note = 0;
    } else if (note >= 128) {
        note = 127;
    }
    SpuGetAllKeysStatus(status);
    for (voice = 22; voice != -1; voice--) {
        if (status[voice] == 0) {
            func_8006dce8(bank->vab_id, effect->program, effect->tone, note, 0, left, right);
            voice = 0;
        }
    }
}

ADDRESS(0x80015908, 0x140)
void audio_vab_stream_callback(CdRequest *request)
{
    SoundBank *bank;

    switch (request->state) {
    case 0:
        bank = &sound_status.banks[request->u.sound.bank];
        if (cd_sectors_corrupt(request->u.sound.slot->buffer, request->count)) {
            request->issued = 0;
            CdSeekL(&request->retry_pos);
            break;
        }
        if (bank->vab_id != SOUND_VAB_NONE && bank->vab_id != SOUND_VAB_LOADING) {
            SsVabClose(bank->vab_id);
            bank->vab_id = SOUND_VAB_NONE;
        }
        bank->vab_id = func_80068f7c(request->u.sound.slot->buffer, -1, request->u.sound.spu_addr);
        if (bank->vab_id == SOUND_VAB_NONE) {
            cd_request_advance(request);
            break;
        }
        cd_location_add(&request->pos, request->count, &request->pos);
        request->state = 1;
        request->buffer = stream_work_buffer;
        request->issued = 0;
        request->count = 20;
        CdSeekL(&request->pos);
        break;
    case 1:
        request->state = 2;
        break;
    }
}

ADDRESS(0x80015a48, 0x154)
void cd_request_service_vab(void)
{
    s32 id;
    CdRequest *request;
    SoundBank *bank;

    EnterCriticalSection();
    request = cdrom_ctl.read;
    if (request->kind == 0x30 && request->state == 2) {
        ExitCriticalSection();
        bank = &sound_status.banks[request->u.sound.bank];
        for (;;) {
            id = SsVabTransBodyPartly(request->buffer, 0xa000, bank->vab_id);
            if (id == -1) {
                SsVabClose(bank->vab_id);
                bank->vab_id = SOUND_VAB_NONE;
                cd_request_advance(request);
            } else if (id == -2) {
                request->issued = 0;
                request->state = 1;
                cd_location_add(&request->pos, request->count, &request->pos);
                CdSeekL(&request->pos);
                SsVabTransCompleted(SS_WAIT_COMPLETED);
                return;
            } else {
                if (id == bank->vab_id) {
                    cd_request_advance(request);
                    SsVabTransCompleted(SS_WAIT_COMPLETED);
                    request->u.sound.slot->state = 1;
                    request->count = 0;
                }
                return;
            }
        }
    } else {
        ExitCriticalSection();
    }
}

ADDRESS(0x80015b9c, 0x144)
SoundSlot *audio_acquire_vab_stream_slot(s32 first_bank, s32 bank_count, s32 first_slot, s32 slot_count,
                                         s32 *index)
{
    s32 i = 0;
    SoundSlot *slot = &sound_status.slots[first_slot];
    SoundBank *bank;
    s32 n;
    s32 j;

    for (n = slot_count - 1; n != -1; n--) {
        if (slot->state == 0) {
            *index = i;
            return slot;
        }
        i++;
        slot++;
    }
    i = 0;
    slot = &sound_status.slots[first_slot];
    for (n = slot_count - 1; n != -1; n--) {
        if (slot->state == 2) {
            bank = &sound_status.banks[first_bank];
            for (j = bank_count - 1; j != -1; j--) {
                if (bank->vab_id != SOUND_VAB_NONE && bank->slot == slot) {
                    SsVabClose(bank->vab_id);
                    bank->vab_id = SOUND_VAB_NONE;
                    bank->slot = 0;
                    break;
                }
                bank++;
            }
            slot->state = 0;
            *index = i;
            return slot;
        }
        i++;
        slot++;
    }
    return 0;
}

/*
 * Not exact only through audio_play_spatial: the callback's .text offset follows
 * that function's size.
 */
ADDRESS(0x80015ce0, 0x238)
void audio_queue_vab_stream(s32 file, s32 sector, s32 index)
{
    SoundBank *bank = &sound_status.banks[index];
    SoundBank *other;
    SoundSlot *slot;
    CdRequest *request;
    s32 spu_addr;
    s32 found;
    s32 i;

    switch (index) {
    case 2:
        slot = &sound_status.slots[8];
        spu_addr = 0x41800;
        break;
    case 1:
        slot = &sound_status.slots[9];
        spu_addr = 0x19800;
        break;
    case 0:
        slot = &sound_status.slots[10];
        spu_addr = 0x1000;
        break;
    default:
        if (index < 51) {
            if (sound_status.bank_select == 0) {
                slot = &sound_status.slots[0];
                spu_addr = 0x44800;
            } else {
                slot = &sound_status.slots[1];
                spu_addr = 0x47800;
            }
            if (slot->state != 0) {
                other = &sound_status.banks[3];
                for (i = 47; i != -1; i--) {
                    if (other->vab_id != SOUND_VAB_NONE && other->slot == slot) {
                        SsVabClose(other->vab_id);
                        other->vab_id = SOUND_VAB_NONE;
                        other->slot = 0;
                        break;
                    }
                    other++;
                }
                slot->state = 0;
            }
        } else if (index < 147) {
            slot = audio_acquire_vab_stream_slot(51, 96, 2, 4, &found);
            spu_addr = found * 0x9c00 + 0x52000;
        } else if (index < 179) {
            slot = audio_acquire_vab_stream_slot(147, 32, 6, 2, &found);
            spu_addr = found * 0x3c00 + 0x4a800;
        }
        if (slot == 0) {
            return;
        }
        bank->vab_id = SOUND_VAB_LOADING;
        break;
    }
    bank->slot = slot;
    slot->state = 3;
    request = cdrom_ctl.write;
    cd_request_wait_done(request);
    request->state = 0;
    request->u.sound.slot = slot;
    request->u.sound.bank = index;
    request->u.sound.spu_addr = spu_addr;
    cd_archive_queue_stream_read(file, sector * 2, slot->buffer, (CdCallback)audio_vab_stream_callback);
}
