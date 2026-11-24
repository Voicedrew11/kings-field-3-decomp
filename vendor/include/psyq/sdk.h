#ifndef KF_PSYQ_H
#define KF_PSYQ_H

/*
 * Single guarded entry point for the Psy-Q SDK headers, which ship without
 * include guards of their own. Include this instead of the raw SDK headers so
 * a TU pulls each in exactly once and gets the real library prototypes and
 * types rather than hand-rolled externs.
 *
 * LIBSND and LIBCD stay in psyq/audio.h so their declarations are limited
 * to audio/CD users. KERNEL and the BIOS file interfaces stay in
 * psyq/kernel.h, which also supplies the release's missing LIBAPI
 * declarations and case-sensitive include shim. C runtime declarations stay
 * in psyq/libc.h so only their callers include MEMORY.H and MALLOC.H.
 */
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <LIBETC.H>

/* LIBETC exports VSync, but Psy-Q 3.0's LIBETC.H omits its declaration;
 * later releases declare it with this signature. */
extern int VSync(int mode);

/* REG.OBJ exports this helper, but Psy-Q 3.0's LIBGTE.H omits it. The second
 * pointer is inherited from the SLPS-00017 callers; review against KF2 calls. */
extern void ReadSZ2(long *depth, long *unused_depth);

/* LIBGTE's DIVFT3A..DIVGT4A objects export the recursive polygon dividers,
 * but Runtime Library 3.3's LIBGTE.H leaves their prototypes (and the
 * Divide* front ends) commented out; these are its commented declarations. */
extern u_long *RCpolyF3(POLY_F3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyF4(POLY_F4 *s, DIVPOLYGON4 *divp);
extern u_long *RCpolyFT3(POLY_FT3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyFT4(POLY_FT4 *s, DIVPOLYGON4 *divp);
extern u_long *RCpolyG3(POLY_G3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyG4(POLY_G4 *s, DIVPOLYGON4 *divp);
extern u_long *RCpolyGT3(POLY_GT3 *s, DIVPOLYGON3 *divp);
extern u_long *RCpolyGT4(POLY_GT4 *s, DIVPOLYGON4 *divp);

#endif
