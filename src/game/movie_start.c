/*
 * GAME.EXE: the last function of the game band (WIP module, after the
 * switch movie_update_state): runs the in-game movie player.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/misc.h>

ADDRESS(0x80061bc4, 0x20)
void movie_play(void)
{
    movie_play_stream();
}
