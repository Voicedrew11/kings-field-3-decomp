/*
 * FDAT_011.BIN (WIP module): the handler table, the shared slot 0 handler,
 * empty handlers, an object handler (slot 3), an event handler (slot 4) and
 * the load handler (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x88)

void func_801e8390(void);
void func_801e8398(void);
void func_801e85a8(void);
void func_801e85b0(void);
void func_801e85b8(void);
void func_801e85c0(void);
void func_801e85c8(void);
void func_801e85d0(void);
void func_801e85d8(Object44 *obj);
void func_801e8698(void);
void func_801e86a0(void);
void func_801e86a8(void);
void func_801e86b0(void);
void func_801e86b8(void);
void func_801e86c0(Object88 *obj, s32 event);
void func_801e87c0(void);

const FdatHeader fdat_header = {
    0x8,
    {
        fdat_801e83b0, func_801e85b8, func_801e85c0, func_801e85d8,
        func_801e86c0, func_801e87c0, func_801e86b8, func_801e8390,
        func_801e8698, func_801e86a0, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e86a8, func_801e86b0, func_801e85c8, func_801e85d0,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
        func_801e8390, func_801e8390, func_801e8390, func_801e8390,
    },
    0x28,
};

ADDRESS(0x801e8390, 0x8)
void func_801e8390(void)
{
}

ADDRESS(0x801e8398, 0x8)
void func_801e8398(void)
{
}

#include "shared/fdat_801e83b0.inc"

ADDRESS(0x801e85a8, 0x8)
void func_801e85a8(void)
{
}

ADDRESS(0x801e85b0, 0x8)
void func_801e85b0(void)
{
}

ADDRESS(0x801e85b8, 0x8)
void func_801e85b8(void)
{
}

ADDRESS(0x801e85c0, 0x8)
void func_801e85c0(void)
{
}

ADDRESS(0x801e85c8, 0x8)
void func_801e85c8(void)
{
}

ADDRESS(0x801e85d0, 0x8)
void func_801e85d0(void)
{
}

/* Slot 3: for an object whose unk3a.bytes[1] is 10, shows one of two notes. */
ADDRESS(0x801e85d8, 0xc0)
void func_801e85d8(Object44 *obj)
{
    if (obj->arg.b.unk3a.bytes[1] != 10) {
        return;
    }
    if (player_camera_within_map_region(6, 0x1e, 3, 4, 0x8000) != 0) {
        if (DAT_801b24e4.unk10a != 0) {
            map_cell_copy_rotated_fields(2, 0x4a, 0, 6, 0x21, 3, 1, 0, 0x2f);
        }
    } else {
        DAT_801b24e4.unk10a = 0;
        map_cell_copy_rotated_fields(2, 0x4d, 0, 6, 0x21, 3, 1, 0, 0x2f);
    }
}

ADDRESS(0x801e8698, 0x8)
void func_801e8698(void)
{
}

ADDRESS(0x801e86a0, 0x8)
void func_801e86a0(void)
{
}

ADDRESS(0x801e86a8, 0x8)
void func_801e86a8(void)
{
}

ADDRESS(0x801e86b0, 0x8)
void func_801e86b0(void)
{
}

ADDRESS(0x801e86b8, 0x8)
void func_801e86b8(void)
{
}

/* Slot 4: script events 0..2 of an object record. */
ADDRESS(0x801e86c0, 0x100)
void func_801e86c0(Object88 *obj, s32 event)
{
    s32 result;

    switch (event) {
    case 0:
        if (game_counter_is_nonzero(0x7c) > 0) {
            DAT_801ba984.unk76[5] = 1;
        } else {
            DAT_801ba984.unk76[5] = 0;
        }
        break;
    case 1:
        player_render_frame_and_release_pool();
        result = menu_blacksmith_make_item();
        DAT_801b24e4.unk178 = ~DAT_80081868[7];
        if (result < 0) {
            DAT_801ba984.unk76[2] = 0;
            DAT_801ba984.unk76[0] = 0x68;
        } else {
            DAT_801ba984.unk76[3] = 0;
            DAT_801ba984.unk76[2] = 1;
            DAT_801ba984.unk76[0] = result;
        }
        break;
    case 2:
        event_map_object_interact(NULL, DAT_801ba984.unk76[0]);
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e87c0, 0x20)
void func_801e87c0(void)
{
    DAT_801ba984.unk76[3] = 1;
    DAT_801ba984.unk84[5] = 3;
}
