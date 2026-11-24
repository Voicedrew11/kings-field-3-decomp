#ifndef KF_OVERLAY_H
#define KF_OVERLAY_H

/*
 * Types OPEN.EXE and END.EXE share: the double display buffer of their main
 * units and the decode environment of their Sony-sample movie players.
 * Include after the LIBGPU header.
 */

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

#endif
