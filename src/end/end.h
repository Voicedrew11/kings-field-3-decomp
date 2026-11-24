#ifndef KF_END_END_H
#define KF_END_END_H

/* END.EXE declarations shared by src/end/main.c and src/end/movie.c. */

#include <kf/overlay.h>

extern DB *cdb;
extern u_long *prim_next;
extern DB db[2];

void play_movie(int movie);
void swap_buffers(void);
void flip_buffers(void);

#endif
