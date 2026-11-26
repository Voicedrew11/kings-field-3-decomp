/*
 * GAME.EXE: Effect record list (WIP module, after effect_update_dispatch): clearing
 * the records, loading the MagicRecord table and the per-frame update loop.
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
    Effect *obj;
    u16 i;

    obj = effect_status.objects;
    for (i = 0; i < 128; i++) {
        obj->type = EFFECT_FREE;
        obj++;
    }
}

/* Copies the 96 MagicRecord records (0x900 bytes) from src. */
ADDRESS(0x8005bc24, 0x2c)
void magic_load_records(u32 *src)
{
    u32 *dst;
    s32 i;

    dst = (u32 *)effect_status.magic_records;
    i = 0x240;
    do {
        *dst++ = *src++;
    } while (--i != 0);
}

/* Counts down and updates (effect_update_dispatch) every live record. */
ADDRESS(0x8005bc50, 0xe4)
void effect_pool_sweep(void)
{
    Effect *obj;

    obj = effect_status.objects;
    effect_status.current_index = 0;
    do {
        if (obj->type != EFFECT_FREE) {
            effect_status.current = obj;
            effect_status.current_magic = &effect_status.magic_records[obj->kind];
            if (--obj->updates_remaining == -1) {
                obj->type = EFFECT_FREE;
            } else {
                effect_update_dispatch();
            }
        }
        obj++;
    } while (++effect_status.current_index < 128);
}
