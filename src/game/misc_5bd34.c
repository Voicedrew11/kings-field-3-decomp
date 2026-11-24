/*
 * GAME.EXE: two sound-effect wrappers, a collision probe in front of a
 * position and two empty functions (WIP module; the empty pair is taken
 * as its end, like 0x8001765c and 0x8001a62c).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/sound.h>
#include <game/entity.h>
#include <game/misc.h>

ADDRESS(0x8005bd34, 0x24)
void func_8005bd34(void)
{
    func_8001576c(0x10, 100);
}

ADDRESS(0x8005bd58, 0x20)
void func_8005bd58(s32 se)
{
    func_8001576c(se, 100);
}

/* Collision test 800 units ahead of pos along angle->vy. */
ADDRESS(0x8005bd78, 0xa0)
s32 func_8005bd78(VECTOR *pos, SVECTOR *angle)
{
    s32 x;

    x = pos->vx - (rsin(angle->vy) * 800 >> 12);
    func_80033b10(x, pos->vy, (rcos(angle->vy) * 800 >> 12) + pos->vz, 800, 0x6a4);
    return 0;
}

ADDRESS(0x8005be18, 0x8)
void func_8005be18(void)
{
}

ADDRESS(0x8005be20, 0x8)
void func_8005be20(void)
{
}
