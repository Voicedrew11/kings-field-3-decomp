#ifndef FDAT_FDAT_H
#define FDAT_FDAT_H

/*
 * The FDAT.T area modules: GAME.EXE loads one code entry whole at
 * 0x801e8308 and calls through the handler table at its +4
 * (DAT_8018fad4.handlers). A module calls GAME.EXE's functions and reads its
 * data in place, so it spells them with their GAME.EXE identities.
 *
 * A body that several modules repeat is one fragment under src/fdat/shared/,
 * included at its place in each module's source. Its functions carry one
 * shared identity, fdat_<address in the first module that has it>, in every
 * module, and the module units bind their claims by name.
 */
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/image.h>
#include <game/cd.h>
#include <game/render.h>
#include <game/player.h>
#include <game/entity.h>
#include <game/misc.h>
#include <game/menu.h>

/* The table at the module's first byte (WIP layout). */
typedef struct FdatHeader {
    s32 id;                 /* +0x00: 5, 6, ... 0x20, one per module */
    FdatHandler handlers[32]; /* +0x04: DAT_8018fad4.handlers points here */
    s32 unk84;              /* +0x84: id + 0x20 in every module */
} FdatHeader;

/* GAME.EXE functions the modules call (no GAME source or prototype yet). */
void func_800445b8(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6,
                   s32 arg7, s32 arg8);
void func_800463bc(s32 arg0);
s32 func_80045c7c(s32 start, VECTOR *pos, s32 range, s32 height, s32 angle, s32 arc);
s32 func_80046608(Object44 *obj, s32 item);
s32 func_80046884(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
s32 func_80046914(Object44 *obj, Object44 *target, SVECTOR *from, SVECTOR *to, s32 arg4,
                  s32 arg5);
/* Varargs: with obj NULL it takes the object index from the second argument. */
void func_8005db30(Object44 *obj, ...);

/* src/fdat/shared/fdat_801e83b0.inc (slot 0 in 17 modules) */
void fdat_801e83b0(VECTOR *pos, SVECTOR *angle);
/* src/fdat/shared/fdat_801e85a8.inc (slot 2 in 2 modules) */
void fdat_801e85a8(VECTOR *pos, SVECTOR *angle, s32 item);
/* src/fdat/shared/fdat_801e86d8.inc (2 modules) */
void fdat_801e86d8(s32 index, s32 count);
/* src/fdat/shared/fdat_801e875c.inc (slot 16 in 2 modules) */
s32 fdat_801e875c(u8 *script, s32 dist);
/* src/fdat/shared/fdat_801e87a0.inc (slot 17 in 2 modules) */
void fdat_801e87a0(void);

#endif
