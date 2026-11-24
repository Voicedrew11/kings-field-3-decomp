#ifndef KF_PSYQ_CD_H
#define KF_PSYQ_CD_H

/* Guarded entry point for the unguarded Psy-Q 3.0 LIBCD.H. */

#include <psyq/sdk.h>
#if defined(__cplusplus)
/*
 * The 3.3 LIBCD.H declares StFreeRing twice inside one extern "C" block,
 * once unprototyped and once as StFreeRing(u_long *base); C accepts that,
 * C++ rejects the pair as conflicting C-linkage functions. For the modern
 * view only, this rewrites the empty parameter list to the full one:
 * `StFreeRing()` becomes `StFreeRing(u_long *)`, and a declaration or call
 * with arguments is left as written. The compiled C never sees it.
 */
#define KF_CD_FIRST(first, ...) first
#define StFreeRing(...) StFreeRing(__VA_ARGS__ KF_CD_FIRST(__VA_OPT__(, ) u_long *))
#endif
#include <LIBCD.H>

/* CDREAD.OBJ exports this status poll, omitted by Psy-Q 3.0 LIBCD.H. */
extern int CdReadSync(int mode, unsigned char *result);
/* LIBETC's CALLBACK.OBJ exports this sector-transfer hook; no Psy-Q 3.0
 * header declares it. The return value is unused by every caller. */
extern int CdDataCallback(void (*func)());

#endif
