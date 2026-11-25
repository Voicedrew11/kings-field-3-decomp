/*
 * GAME.EXE: Object4c record list (WIP module, after effect_update_dispatch): clearing
 * the records, loading the Record18 table and the per-frame update loop.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/object4c.h>

/* Frees every record. */
ADDRESS(0x8005bbf4, 0x30)
void effect_pool_reset(void)
{
    Object4c *obj;
    u16 i;

    obj = effect_status.objects;
    for (i = 0; i < 128; i++) {
        obj->unk0 = OBJECT4C_FREE;
        obj++;
    }
}

/* Copies the 96 Record18 records (0x900 bytes) from src. */
ADDRESS(0x8005bc24, 0x2c)
void magic_load_records(u32 *src)
{
    u32 *dst;
    s32 i;

    dst = (u32 *)effect_status.unk0;
    i = 0x240;
    do {
        *dst++ = *src++;
    } while (--i != 0);
}

/* Counts down and updates (effect_update_dispatch) every live record. */
ADDRESS(0x8005bc50, 0xe4)
void effect_pool_sweep(void)
{
    Object4c *obj;

    obj = effect_status.objects;
    effect_status.unk2f08 = 0;
    do {
        if (obj->unk0 != OBJECT4C_FREE) {
            effect_status.unk2f04 = obj;
            effect_status.unk2f00 = &effect_status.unk0[obj->unk1];
            if (--obj->unk10 == -1) {
                obj->unk0 = OBJECT4C_FREE;
            } else {
                effect_update_dispatch();
            }
        }
        obj++;
    } while (++effect_status.unk2f08 < 128);
}
