/*
 * GAME.EXE: Object88 per-frame update and area set-up (WIP module): the
 * loop over all records, the script pointers of the loaded Object78
 * records, and the records made from an area's placement records.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/data.h>
#include <game/entity.h>

/* Updates every record: wake/sleep, script pick, its script, attachment. */
ADDRESS(0x80052e5c, 0x228)
void actor_update_frame(void)
{
    Object88 *obj;
    Object44 *carrier;

    obj = actor_status.objects;
    actor_status.unk9d20 = 0;
    actor_status.unk9d18 = 0;
    do {
        if (obj->unk0 != OBJECT88_FREE) {
            actor_bind_current(obj);
            if ((actor_status.unk9d24 & 3) == (actor_status.unk9d18 & 3) || cur_avatar_status.unke != 0 ||
                cur_avatar_status.unk101 == 1) {
                actor_update_lifecycle_for_player_range();
            }
            if (obj->unk9 == 1) {
                if ((actor_status.unk9d24 & 3) == (actor_status.unk9d20 & 3)) {
                    actor_select_target_for_player_distance();
                }
                actor_update_behavior();
                actor_status.unk9d20++;
            }
            if (actor_status.def->unk34 & 0x10000) {
                carrier = &map_objects_slots.objects[obj->unk22];
                obj->pos.vx = carrier->pos.vx;
                obj->pos.vy = carrier->pos.vy - 500;
                obj->pos.vz = carrier->pos.vz;
                if (obj->unk9 == 1) {
                    obj->angle.vy = vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                                       cur_avatar_status.pos.vz - obj->pos.vz);
                }
            }
            if (obj->unk1 == 0x6e) {
                if (cur_avatar_status.unkf5 != 0x5c && cur_avatar_status.unkf6 != 0x5c) {
                    obj->unk3 = 0;
                } else if (cur_avatar_status.unk1c == 0) {
                    obj->unk3 = 0;
                }
            }
        }
        obj++;
    } while (++actor_status.unk9d18 < 200);
    actor_status.unk9d24++;
    actor_bind_current(NULL);
}

/* Turns the loaded Object78 records' script offsets into pointers (-1: none). */
ADDRESS(0x80053084, 0x74)
void actor_fixup_group_targets(void)
{
    Object78 *def;
    u8 **scripts;
    s32 i;
    s32 j;

    def = actor_status.defs;
    for (i = 0; i < 40; i++, def++) {
        if (def->unk0 == 0xff) {
            return;
        }
        scripts = def->scripts;
        for (j = 0; j < 16; j++, scripts++) {
            if ((s32)*scripts == -1) {
                *scripts = NULL;
            } else {
                *scripts = actor_status.unk7d00 + (s32)*scripts;
            }
        }
    }
}

/* Makes the 200 records from an area's placement records. */
ADDRESS(0x800530f8, 0x1dc)
void actor_load_records(Object88Place *place)
{
    Object88 *obj;
    Object78 *def;
    u16 i;

    obj = actor_status.objects;
    i = 200;
    while (i--) {
        obj->unk0 = place->unk0;
        if (obj->unk0 != OBJECT88_FREE) {
            obj->unk2 = place->unk1;
            obj->unk4 = 0;
            obj->unk5 = place->unk2;
            obj->unk6 = place->unk7;
            obj->unk7 = place->unk3;
            obj->unk8 = place->unk4;
            obj->unka = place->unk5;
            obj->unkb = place->unk6;
            obj->unk20 = -place->unk8 & 0xfff;
            obj->unk22 = place->unka;
            obj->unk24 = place->unkc;
            obj->unk26 = place->unke;
            obj->unk9 = 0;
            obj->unke = 0;
            obj->unkf = 0xff;
            obj->unk60 = NULL;
            def = &actor_status.defs[obj->unk2];
            actor_copy_group_defaults(obj);
            actor_set_home_position(obj);
            obj->unk15 = def->unk9;
            if (obj->unk28 & 0x10) {
                if (obj->unk0 == 3) {
                    if (obj->unk24 == -1) {
                        obj->unk24 = def->unk1a;
                    }
                    if (obj->unk26 == -1) {
                        obj->unk26 = def->unk1c;
                    }
                } else {
                    obj->unk0 = 4;
                    obj->unk26 = 0;
                }
            }
        } else {
            obj->unk9 = 0;
        }
        place++;
        obj++;
    }
}
