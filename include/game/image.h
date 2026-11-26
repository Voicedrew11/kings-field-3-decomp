#ifndef GAME_IMAGE_H
#define GAME_IMAGE_H

/*
 * GAME.EXE VRAM image streaming (src/game/image.c, WIP names): an archive
 * entry read through CD_REQUEST_IMAGE requests in 16-sector parts, each part
 * a run of RECT headers (stored twice) followed by their pixels, uploaded
 * with LoadImage as the parts arrive.
 */
#include <kf/lib/types.h>

void cd_map_stream_read(int file, int entry);
void cd_request_service_stream(void);

#endif
