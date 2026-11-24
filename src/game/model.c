/*
 * GAME.EXE: model data registration (WIP slice of the render module; the
 * primitive fix-up func_8003591c before it is not claimed yet).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>

/* Registers model data id (0..3) and fixes up its primitives. */
ADDRESS(0x80035c44, 0x40)
void func_80035c44(u16 id, u8 *data)
{
    DAT_80199154.vertices = DAT_80199154.unk10148[id] = data;
    func_8003591c(data);
}

/* Registers model data id without the fix-up. */
ADDRESS(0x80035c84, 0x20)
void func_80035c84(u16 id, u8 *data)
{
    DAT_80199154.unk10148[id] = data;
}
