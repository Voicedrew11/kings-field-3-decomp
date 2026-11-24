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

#include <kf/decenv.h>

#endif
