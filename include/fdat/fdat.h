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
#include <game/map_patch.h>
#include <game/object44.h>

/* The table at the module's first byte (WIP layout). */
typedef struct FdatHeader {
    s32 id;                 /* +0x00: 5, 6, ... 0x20, one per module */
    FdatHandler handlers[32]; /* +0x04: DAT_8018fad4.handlers points here */
    s32 unk84;              /* +0x84: id + 0x20 in every module */
} FdatHeader;

/* GAME.EXE functions the modules call (no GAME source or prototype yet). */
/* Varargs: with obj NULL it takes the object index from the second argument. */
void func_8005db30(Object44 *obj, ...);

/* src/fdat/shared/fdat_801e83a4.inc (4 modules) */
void fdat_801e83a4(Record30 *from, Record30 *to, Record30 *out, s32 t);
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
