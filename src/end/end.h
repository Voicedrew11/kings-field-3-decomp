#ifndef KF_END_END_H
#define KF_END_END_H

/* END.EXE declarations shared by src/end/main.c and src/end/movie.c. */

#define OT_LENGTH 256

/* One display buffer: its environments, ordering table and primitive area. */
typedef struct {
    DRAWENV draw;
    DISPENV disp;
    u_long ot[OT_LENGTH];
    u_long *prim;
} DB;

/* Sony's movie-sample decode environment, with one slice buffer. */
typedef struct {
    u_long *vlcbuf[2]; /* VLC (run-level) buffers */
    int vlcid;
    u_long *imgbuf; /* DecDCTout slice buffer */
    RECT rect[2];   /* VRAM frame areas */
    int rectid;
    RECT slice; /* area one DecDCTout fills */
    int isdone;
} DECENV;

extern DB *cdb;
extern u_long *prim_next;
extern DB db[2];

void play_movie(int movie);
void swap_buffers(void);
void flip_buffers(void);

#endif
