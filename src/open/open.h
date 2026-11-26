#ifndef KF_OPEN_OPEN_H
#define KF_OPEN_OPEN_H

/* OPEN.EXE declarations shared by src/open/main.c, movie.c and card.c. */

#include <kf/overlay.h>

/* play_movie's index for each movie on the disc. */
#define MOVIE_ATTRACT_A 0 /* \OP\M0.S */
#define MOVIE_ATTRACT_B 1 /* \OP\M1.S */
#define MOVIE_NEW_GAME 2  /* \OP\M3.S */
#define MOVIE_BOOT_LOGO 3 /* \OP\L0.S */
#define MOVIE_LOGO 4      /* \OP\L1.S */

extern DB *cdb;
extern DB db[2];

int play_movie(int movie);
void swap_buffers(void);
void flip_buffers(void);

void card_init(void);
void card_close(void);
void card_start(void);
void card_stop(void);
int card_check_writable(void);
int card_list_saves(struct DIRENTRY *dir, int *count);

#endif
