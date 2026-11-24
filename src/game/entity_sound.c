/*
 * GAME.EXE: Object88 sound helpers (WIP module): the note offset from the
 * record's scale and the sound effect of its running script record.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/data.h>
#include <game/sound.h>
#include <game/entity.h>

/* Note offset for obj's sound: lower for a larger record, plus a little noise. */
ADDRESS(0x8004ffa0, 0x70)
s32 func_8004ffa0(Object88 *obj)
{
    s32 note;

    note = 16 - (s8)(obj->scale.vy >> 8);
    if (note > 12) {
        note = 12;
    } else if (note < -12) {
        note = -12;
    }
    return note - 2 + ((rand() * 5) >> 15);
}

/* Plays the sound effect of obj's running script record at obj. */
ADDRESS(0x80050010, 0x98)
void func_80050010(Object88 *obj)
{
    u8 *script;

    script = obj->unk60;
    if (obj->unk28 & 0x800000) {
        func_80015688(script[4] + 240, &obj->pos, 0x7f, 0x6000, 0x7800, func_8004ffa0(obj));
    } else {
        func_80015654(script[4] + 240, &obj->pos, 0x6e, func_8004ffa0(obj));
    }
}
