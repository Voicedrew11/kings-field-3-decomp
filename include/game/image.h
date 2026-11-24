#ifndef GAME_IMAGE_H
#define GAME_IMAGE_H

/*
 * GAME.EXE VRAM image streaming (src/game/image.c, WIP names): an archive
 * entry read through CD_REQUEST_IMAGE requests in 16-sector parts, each part
 * a run of RECT headers (stored twice) followed by their pixels, uploaded
 * with LoadImage as the parts arrive.
 */
#include <kf/lib/types.h>

void func_80018c60(int file, int entry);
void func_80018cd0(void);

#endif
