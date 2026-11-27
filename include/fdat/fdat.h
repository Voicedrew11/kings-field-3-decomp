#ifndef FDAT_FDAT_H
#define FDAT_FDAT_H

/*
 * The FDAT.T area modules: GAME.EXE loads one code entry whole at
 * 0x801e8308 and calls through the handler table at its +4
 * (asset_status.handlers). A module calls GAME.EXE's functions and reads its
 * data in place, so it spells them with their GAME.EXE identities.
 *
 * A body that several modules repeat is one fragment under src/fdat/shared/,
 * included at its place in each module's source and named after its
 * function. That function carries one shared identity name in every module,
 * and the module units bind their claims by name.
 */

/* Marks the declarations whose FDAT view differs from GAME.EXE's (game/entity.h). */
#define FDAT_MODULE
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
    FdatHandlers handlers;  /* +0x04: asset_status.handlers points here (game/data.h) */
    s32 unk84;              /* +0x84: id + 0x20 in every module */
} FdatHeader;

/* GAME.EXE .sdata word ($gp+0x8, owner unit unknown): FDAT_002's light-flicker step. */
extern s32 light_flicker_step;

/*
 * GAME.EXE function the modules call (game.item_count; no GAME header
 * prototype yet). Varargs: with obj NULL it takes the object index from the
 * second argument. event_target_stream_execute comes from game/misc.h.
 */
void event_map_object_interact(MapObject *obj, ...);

/* src/fdat/shared/light_blend_records.inc (4 modules) */
void light_blend_records(LightSourceRecord *from, LightSourceRecord *to, LightSourceRecord *out, s32 t);
/* src/fdat/shared/area_interact_item_containers.inc (slot 0 in 17 modules) */
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle);
/* src/fdat/shared/area_use_item_notify_no_target.inc (slot 2 in 2 modules) */
void area_use_item_notify_no_target(VECTOR *pos, SVECTOR *angle, s32 item);
/* src/fdat/shared/area_disable_actor_range.inc (2 modules) */
void area_disable_actor_range(s32 index, s32 count);
/* src/fdat/shared/area_actor_target_check_approach_range.inc (slot 16 in 2 modules) */
s32 area_actor_target_check_approach_range(u8 *script, s32 dist);
/* src/fdat/shared/area_update_actor_target_fade_approach.inc (slot 17 in 2 modules) */
void area_update_actor_target_fade_approach(void);

#endif
