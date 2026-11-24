/*
 * GAME.EXE: clearing the script flags, the block arena and its saved
 * offsets, and the item counts with the starting items (WIP module).
 * func_8005eb20 follows.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/memory.h>
#include <game/data.h>
#include <game/misc.h>

ADDRESS(0x8005ea64, 0xbc)
void func_8005ea64(void)
{
    u16 *off;
    s32 i;

    func_80018fbc((u32 *)DAT_801ba984.unk4, 0, 0x40);
    func_80018fbc((u32 *)DAT_801ba984.unk104, 0, 0x1400);
    func_8001937c(DAT_801ba984.unk104, 0x5000);
    off = DAT_801ba984.unk5104;
    for (i = 31; i != -1; i--) {
        *off++ = 0xffff;
    }
    func_80018fbc((u32 *)DAT_800c85e8, 0, 0x4b);
    DAT_800c85e8[0][0] = 1;
    DAT_800c85e8[0][42] = 1;
    DAT_800c85e8[0][104] = 2;
    DAT_800c85e8[0][105] = 1;
}
