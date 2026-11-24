#ifndef KF_DECENV_H
#define KF_DECENV_H

/*
 * Sony's movie-sample decode environment, with one slice buffer: OPEN.EXE's
 * and END.EXE's players (through kf/overlay.h) and GAME.EXE's (inside
 * DAT_801e7edc). Include after the LIBGPU header.
 */
typedef struct {
    u_long *vlcbuf[2]; /* VLC (run-level) buffers */
    int vlcid;
    u_long *imgbuf; /* DecDCTout slice buffer */
    RECT rect[2];   /* VRAM frame areas */
    int rectid;
    RECT slice; /* area one DecDCTout fills */
    int isdone;
} DECENV;

#endif
