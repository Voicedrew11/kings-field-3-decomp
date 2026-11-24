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
void func_80052e5c(void)
{
    Object88 *obj;
    Object44 *carrier;

    obj = DAT_80185da8.objects;
    DAT_80185da8.unk9d20 = 0;
    DAT_80185da8.unk9d18 = 0;
    do {
        if (obj->unk0 != OBJECT88_FREE) {
            func_8004da2c(obj);
            if ((DAT_80185da8.unk9d24 & 3) == (DAT_80185da8.unk9d18 & 3) || DAT_801b24e4.unke != 0 ||
                DAT_801b24e4.unk101 == 1) {
                func_8004c1f0();
            }
            if (obj->unk9 == 1) {
                if ((DAT_80185da8.unk9d24 & 3) == (DAT_80185da8.unk9d20 & 3)) {
                    func_8004c01c();
                }
                func_800500a8();
                DAT_80185da8.unk9d20++;
            }
            if (DAT_80185da8.def->unk34 & 0x10000) {
                carrier = &DAT_8018fb3c.objects[obj->unk22];
                obj->pos.vx = carrier->pos.vx;
                obj->pos.vy = carrier->pos.vy - 500;
                obj->pos.vz = carrier->pos.vz;
                if (obj->unk9 == 1) {
                    obj->angle.vy = func_80016ab8(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vz - obj->pos.vz);
                }
            }
            if (obj->unk1 == 0x6e) {
                if (DAT_801b24e4.unkf5 != 0x5c && DAT_801b24e4.unkf6 != 0x5c) {
                    obj->unk3 = 0;
                } else if (DAT_801b24e4.unk1c == 0) {
                    obj->unk3 = 0;
                }
            }
        }
        obj++;
    } while (++DAT_80185da8.unk9d18 < 200);
    DAT_80185da8.unk9d24++;
    func_8004da2c(NULL);
}

/* Turns the loaded Object78 records' script offsets into pointers (-1: none). */
ADDRESS(0x80053084, 0x74)
void func_80053084(void)
{
    Object78 *def;
    u8 **scripts;
    s32 i;
    s32 j;

    def = DAT_80185da8.defs;
    for (i = 0; i < 40; i++, def++) {
        if (def->unk0 == 0xff) {
            return;
        }
        scripts = def->scripts;
        for (j = 0; j < 16; j++, scripts++) {
            if ((s32)*scripts == -1) {
                *scripts = NULL;
            } else {
                *scripts = DAT_80185da8.unk7d00 + (s32)*scripts;
            }
        }
    }
}

/* Makes the 200 records from an area's placement records. */
ADDRESS(0x800530f8, 0x1dc)
void func_800530f8(Object88Place *place)
{
    Object88 *obj;
    Object78 *def;
    u16 i;

    obj = DAT_80185da8.objects;
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
            def = &DAT_80185da8.defs[obj->unk2];
            func_8004b624(obj);
            func_8004b560(obj);
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
