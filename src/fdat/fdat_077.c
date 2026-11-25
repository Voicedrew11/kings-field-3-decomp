/*
 * FDAT_077.BIN (WIP module): the handler table, the shared slot 0, 2, 16 and
 * 17 handlers, empty handlers, a record handler (slot 19) and the load
 * handler (slot 5).
 */
#define FDAT_801E87A0_STEP 1024
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x88)

void func_801e8390(void);
void func_801e85a0(void);
void func_801e85dc(void);
void func_801e85e4(Object88 *obj);
void func_801e8744(void);
void func_801e874c(void);
void func_801e8754(void);
void func_801e8af0(void);
void func_801e8af8(void);
void func_801e8b00(void);

const FdatHeader fdat_header = {
    0x1e,
    {
        fdat_801e83b0, func_801e85a0, fdat_801e85a8, func_801e8744,
        func_801e8af8, func_801e8b00, func_801e8af0, func_801e8390,
        func_801e874c, func_801e8754, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        fdat_801e875c, fdat_801e87a0, func_801e85dc, func_801e85e4,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x3e,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

#include "shared/fdat_801e83b0.inc"

ADDRESS(0x801e85a0, 0x8)
void func_801e85a0(void)
{
}

#include "shared/fdat_801e85a8.inc"

ADDRESS(0x801e85dc, 0x8)
void func_801e85dc(void)
{
}

/* Slot 19: record 0 starts the ending once (flag 0x39): fades the sound out, stops the sequence. */
ADDRESS(0x801e85e4, 0x160)
void func_801e85e4(Object88 *obj)
{
    s32 volume;
    s32 i;

    if (obj - DAT_80185da8.objects != 0) {
        return;
    }
    if (DAT_801ba984.unk32[0x39] != 0) {
        return;
    }
    DAT_801ba984.unk32[0x39] = 1;
    DAT_801b24e4.unkc0 = -1;
    light_reset_records();
    render_reset_color_overlays();
    light_refresh_record_rotations();
    player_update_status_visuals();
    volume = 0x7f;
    for (i = 0x3f; i != -1; i--) {
        volume -= 2;
        if (volume < 0) {
            SsSetMVol(0, 0);
        } else {
            SsSetMVol(volume, volume);
        }
        DrawSync(0);
        VSync(0);
    }
    SsSetMVol(0, 0);
    if (DAT_801b09fc.seq_playing == 1) {
        SsSeqSetVol(DAT_801b09fc.seq_id, 0, 0);
        SsSeqStop(DAT_801b09fc.seq_id);
        SsSeqClose(DAT_801b09fc.seq_id);
        DAT_801b09fc.seq_playing = 0;
    }
    DAT_8009c3f8 = 3;
    render_frames_with_color_overlay(0x82, 0, 0x1000, 0x40);
}

ADDRESS(0x801e8744, 0x8)
void func_801e8744(void)
{
}

ADDRESS(0x801e874c, 0x8)
void func_801e874c(void)
{
}

ADDRESS(0x801e8754, 0x8)
void func_801e8754(void)
{
}

#include "shared/fdat_801e875c.inc"

#include "shared/fdat_801e87a0.inc"

ADDRESS(0x801e8af0, 0x8)
void func_801e8af0(void)
{
}

ADDRESS(0x801e8af8, 0x8)
void func_801e8af8(void)
{
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8b00, 0x14)
void func_801e8b00(void)
{
    DAT_801ba984.unk84[5] = 0x19;
}
