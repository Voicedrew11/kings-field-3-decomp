/*
 * GAME.EXE: sound (WIP module): libsnd start-up and shutdown, the background
 * sequence, positional and plain sound effects, and VAB bank loading through
 * CD read requests. It references no small data, so -G8 and -G0 compile it
 * alike; the CD request pointers it reads are members of the CD state
 * aggregate (game/cd.h), reached absolutely either way.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/kernel.h>
#include <LIBGTE.H>
#include <LIBCD.H>
#include <LIBSND.H>
#include <LIBSPU.H>
#include <game/math.h>
#include <game/sound.h>
#include <game/cd.h>
#include <game/data.h>
#include <game/funcs.h>

DATA(0x801b09fc, 0x1ae8, ".bss")
SoundState DAT_801b09fc;

ADDRESS(0x8001509c, 0x1a4)
void func_8001509c(void)
{
    SoundBank *bank;
    SoundUnk5c8 *unk;
    SoundSlot *slot;
    s32 i;

    SsInit();
    SsSetMVol(0, 0);
    SsSetTableSize(DAT_800c8768, 2, 1);
    SsSetTickMode(SS_TICK60);
    SsStart2();
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_C);
    SsUtReverbOn();
    SsUtSetReverbDepth(40, 40);
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    DAT_801b09fc.bank_select = 0;
    DAT_801b09fc.seq_data = DAT_801b268c;
    DAT_801b09fc.seq_playing = 0;
    DAT_801b09fc.seq_ready = 0;
    bank = DAT_801b09fc.banks;
    for (i = 0xb2; i != -1; i--) {
        bank->vab_id = SOUND_VAB_NONE;
        bank->slot = 0;
        bank++;
    }
    unk = DAT_801b09fc.unk5c8;
    for (i = 9; i != -1; i--) {
        unk->unk0 = -1;
        unk++;
    }
    slot = &DAT_801b09fc.slots[0];
    for (i = 0; i < 2; i++) {
        slot->state = 0;
        slot->buffer = DAT_8014a958[i];
        slot++;
    }
    slot = &DAT_801b09fc.slots[2];
    for (i = 0; i < 4; i++) {
        slot->state = 0;
        slot->buffer = DAT_8017d964[i];
        slot++;
    }
    slot = &DAT_801b09fc.slots[6];
    for (i = 0; i < 2; i++) {
        slot->state = 0;
        slot->buffer = DAT_8017b964[i];
        slot++;
    }
    DAT_801b09fc.slots[8].buffer = DAT_80183568;
    DAT_801b09fc.slots[9].buffer = DAT_8014c958;
    DAT_801b09fc.slots[10].buffer = DAT_80181964;
}

ADDRESS(0x80015240, 0xb8)
void func_80015240(void)
{
    if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_ready != 0
        && DAT_801b09fc.banks[1].vab_id != SOUND_VAB_NONE) {
        DAT_801b09fc.seq_id = SsSeqOpen(DAT_801b09fc.seq_data, DAT_801b09fc.banks[1].vab_id);
        SsSeqSetVol(DAT_801b09fc.seq_id, DAT_801b24e4.bgm_volume, DAT_801b24e4.bgm_volume);
        SsSeqPlay(DAT_801b09fc.seq_id, SSPLAY_PLAY, SSPLAY_INFINITY);
        DAT_801b09fc.seq_playing = 1;
        SsSetMVol(127, 127);
    }
}

ADDRESS(0x800152f8, 0x5c)
void func_800152f8(void)
{
    if (DAT_801b09fc.seq_playing == 1) {
        SsSeqStop(DAT_801b09fc.seq_id);
        SsSeqClose(DAT_801b09fc.seq_id);
        DAT_801b09fc.seq_playing = 0;
    }
}

ADDRESS(0x80015354, 0xbc)
void func_80015354(void)
{
    SoundBank *bank;
    s32 i;

    SsSetMVol(0, 0);
    if (DAT_801b09fc.seq_playing == 1) {
        SsSeqSetVol(DAT_801b09fc.seq_id, 0, 0);
        SsSeqStop(DAT_801b09fc.seq_id);
        SsSeqClose(DAT_801b09fc.seq_id);
    }
    bank = DAT_801b09fc.banks;
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
 * Not exact: retail leaves `note` in its stack slot (loaded at the call) and
 * saves no $s5; GCC here gives it $s5, which shifts se/near between $s3/$s4
 * and the near-volume and turn temporaries (unattributed allocation residue).
 */
ADDRESS(0x80015410, 0x244)
s32 func_80015410(s32 se, VECTOR *pos, s16 unused, s32 range, s32 falloff, s32 note)
{
    s32 dx = (pos->vx - DAT_801b09fc.listener.vx) >> 3;
    s32 dy = (pos->vy - DAT_801b09fc.listener.vy) >> 3;
    s32 dz = (pos->vz - DAT_801b09fc.listener.vz) >> 3;
    s32 near = se & 0x8000;
    s32 distance;
    s32 volume;
    s32 left;
    s32 right;
    s32 turn;
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
        left = volume + 32 + (127 - volume) / 2;
        if (left >= 128) {
            left = 127;
        }
    }
    turn = func_80016ab8(pos->vx - DAT_801b09fc.listener.vx, pos->vz - DAT_801b09fc.listener.vz)
         - DAT_801b09fc.listener_angle.vy;
    angle = turn < 0 ? turn + 0x1000 : turn;
    right = left;
    if (angle < 2048) {
        left -= (left * rsin(angle) >> 12) * (127 - volume) / 128;
    } else {
        right = left + (left * rsin(angle) >> 12) * (127 - volume) / 128;
    }
    func_80015790(se, right, left, note);
    return 1;
}

ADDRESS(0x80015654, 0x34)
s32 func_80015654(s32 se, VECTOR *pos, s16 unused, s32 note)
{
    return func_80015410(se, pos, unused, 0x5000, 0x4800, note);
}

ADDRESS(0x80015688, 0x34)
s32 func_80015688(s32 se, VECTOR *pos, s16 unused, s32 range, s32 falloff, s32 note)
{
    return func_80015410(se, pos, unused, range, falloff, note);
}

ADDRESS(0x800156bc, 0xb0)
void func_800156bc(VECTOR *listener, SVECTOR *angle)
{
    if (listener != 0) {
        DAT_801b09fc.listener = *listener;
        func_800324f0(listener->vx, listener->vy, listener->vz);
        DAT_801b09fc.unk24 = DAT_801d4464.unk1200a;
    }
    if (angle != 0) {
        DAT_801b09fc.listener_angle = *angle;
    }
}

ADDRESS(0x8001576c, 0x24)
void func_8001576c(s32 se, s32 volume)
{
    func_80015790(se, volume, volume, 0);
}

ADDRESS(0x80015790, 0x178)
void func_80015790(s32 se, s32 left, s32 right, s32 note)
{
    char status[24];
    SoundEffect *effect;
    SoundBank *bank;
    s32 voice;

    if (se == 0xff) {
        return;
    }
    if (DAT_801b24e4.se_volume == 0) {
        return;
    }
    se &= 0x3ff;
    effect = &DAT_801b09fc.effects[se];
    if (effect->bank == SOUND_VAB_NONE) {
        return;
    }
    if (se >= 96 && se < 144) {
        bank = &DAT_801b09fc.banks[2];
    } else {
        bank = &DAT_801b09fc.banks[effect->bank];
    }
    if (bank->vab_id == SOUND_VAB_NONE || bank->vab_id == SOUND_VAB_LOADING) {
        return;
    }
    left = left * DAT_801b24e4.se_volume >> 7;
    right = right * DAT_801b24e4.se_volume >> 7;
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
void func_80015908(CdRequest *request)
{
    SoundBank *bank;

    switch (request->state) {
    case 0:
        bank = &DAT_801b09fc.banks[request->u.sound.bank];
        if (func_80019b68(request->u.sound.slot->buffer, request->count)) {
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
            func_8001967c(request);
            break;
        }
        func_80019a74(&request->pos, request->count, &request->pos);
        request->state = 1;
        request->buffer = DAT_801c17a4;
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
void func_80015a48(void)
{
    s32 id;
    CdRequest *request;
    SoundBank *bank;

    EnterCriticalSection();
    request = DAT_801c12e4.read;
    if (request->kind == 0x30 && request->state == 2) {
        ExitCriticalSection();
        bank = &DAT_801b09fc.banks[request->u.sound.bank];
        for (;;) {
            id = SsVabTransBodyPartly(request->buffer, 0xa000, bank->vab_id);
            if (id == -1) {
                SsVabClose(bank->vab_id);
                bank->vab_id = SOUND_VAB_NONE;
                func_8001967c(request);
            } else if (id == -2) {
                request->issued = 0;
                request->state = 1;
                func_80019a74(&request->pos, request->count, &request->pos);
                CdSeekL(&request->pos);
                SsVabTransCompleted(SS_WAIT_COMPLETED);
                return;
            } else {
                if (id == bank->vab_id) {
                    func_8001967c(request);
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
SoundSlot *func_80015b9c(s32 first_bank, s32 bank_count, s32 first_slot, s32 slot_count,
                         s32 *index)
{
    s32 i = 0;
    SoundSlot *slot = &DAT_801b09fc.slots[first_slot];
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
    slot = &DAT_801b09fc.slots[first_slot];
    for (n = slot_count - 1; n != -1; n--) {
        if (slot->state == 2) {
            bank = &DAT_801b09fc.banks[first_bank];
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
 * Not exact only through func_80015410: the callback's .text offset follows
 * that function's size.
 */
ADDRESS(0x80015ce0, 0x238)
void func_80015ce0(s32 file, s32 sector, s32 index)
{
    SoundBank *bank = &DAT_801b09fc.banks[index];
    SoundBank *other;
    SoundSlot *slot;
    CdRequest *request;
    s32 spu_addr;
    s32 found;
    s32 i;

    switch (index) {
    case 2:
        slot = &DAT_801b09fc.slots[8];
        spu_addr = 0x41800;
        break;
    case 1:
        slot = &DAT_801b09fc.slots[9];
        spu_addr = 0x19800;
        break;
    case 0:
        slot = &DAT_801b09fc.slots[10];
        spu_addr = 0x1000;
        break;
    default:
        if (index < 51) {
            if (DAT_801b09fc.bank_select == 0) {
                slot = &DAT_801b09fc.slots[0];
                spu_addr = 0x44800;
            } else {
                slot = &DAT_801b09fc.slots[1];
                spu_addr = 0x47800;
            }
            if (slot->state != 0) {
                other = &DAT_801b09fc.banks[3];
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
            slot = func_80015b9c(51, 96, 2, 4, &found);
            spu_addr = found * 0x9c00 + 0x52000;
        } else if (index < 179) {
            slot = func_80015b9c(147, 32, 6, 2, &found);
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
    request = DAT_801c12e4.write;
    func_80019b1c(request);
    request->state = 0;
    request->u.sound.slot = slot;
    request->u.sound.bank = index;
    request->u.sound.spu_addr = spu_addr;
    func_80019df0(file, sector * 2, slot->buffer, func_80015908);
}
