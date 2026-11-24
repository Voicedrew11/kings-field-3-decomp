/*
 * GAME.EXE: the last function of the game band (WIP module, after the
 * switch func_80061940): runs the in-game movie player.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/misc.h>

ADDRESS(0x80061bc4, 0x20)
void func_80061bc4(void)
{
    func_80060d20();
}
