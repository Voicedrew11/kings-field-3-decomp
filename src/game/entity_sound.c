/*
 * GAME.EXE: Object88 attacks, sound and script interpreter (WIP module): the
 * attack spawner of the current record, the note offset from the record's
 * scale, the sound effect of its running script record and the per-frame
 * interpreter of that script record.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/data.h>
#include <game/sound.h>
#include <game/entity.h>
#include <game/object44.h>
#include <game/object4c.h>

RODATA(0x800126e8, 0x5cc)

/* Aims a shot of speed from pos at target. */
#define AIM(speed) func_8004f1d0(obj, target, speed, &pos, &dir, -1, 0x400, 1)

/*
 * Fires attack id of the current record from its model vertex (-1: the
 * offset in the next three arguments rotated by its angle; -2: between two
 * vertices, aimed at a point eased toward the player): Object4c records
 * through func_80053c84, aimed at the player. The arguments past vertex
 * are read through its address: retail homes a2/a3 like a `...` function
 * and stores vertex to its slot.
 */
ADDRESS(0x8004f414, 0xb8c)
void func_8004f414(s32 id, s32 kind, s32 vertex, ...)
{
    Object88 *obj;
    Object88 *o;
    Object4c *shot;
    Object78 *def;
    VECTOR *target;
    s32 *args;
    VECTOR pos;
    VECTOR off;
    VECTOR from;
    VECTOR aim;
    SVECTOR dir;
    SVECTOR sv;
    SVECTOR rot;
    VECTOR p;
    s32 time;
    s32 pitch;
    s32 speed;
    s32 i;
    s32 delay;
    u16 n;

    args = &vertex;
    target = &DAT_801b24e4.pos;
    obj = DAT_80185da8.current;
    if (vertex == -1) {
        rot.vx = args[1];
        rot.vy = args[2];
        rot.vz = args[3];
        args += 3;
        vector_rotate_yxz(&obj->angle, &rot, &off);
    } else if (vertex == -2) {
        func_8004ede0(obj, args[1], &from);
        func_8004ede0(obj, args[2], &off);
        args += 3;
        speed = args[0];
        aim.vx = fixed_lerp_q12(DAT_801b24e4.pos.vx, ((off.vx - from.vx) << 8) + obj->pos.vx, speed);
        aim.vy = fixed_lerp_q12(DAT_801b24e4.pos.vy, ((off.vy - from.vy) << 8) + obj->pos.vy, speed);
        aim.vz = fixed_lerp_q12(DAT_801b24e4.pos.vz, ((off.vz - from.vz) << 8) + obj->pos.vz, speed);
        target = &aim;
    } else {
        func_8004ede0(obj, vertex, &off);
    }
    pos.vx = obj->pos.vx + off.vx;
    pos.vy = obj->pos.vy + off.vy;
    pos.vz = obj->pos.vz + off.vz;
    switch (id) {
    case 0:
    case 0x56:
    case 0x66:
        audio_play_spatial_default_range(0x31, &pos, 0x6e, 0);
        AIM(500);
        shot = func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        if (shot != NULL) {
            shot->unkb = 3;
        }
        break;
    case 0xf:
        func_8004f1d0(obj, target, 0x200, &pos, &dir, 0, 0x400, 1);
        func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir, 0, 0);
        func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir, 0, 1);
        break;
    case 0x27:
        if (DAT_801b24e4.unk84 != 0) {
            obj->unk28 |= 0x100;
        }
    case 0x26:
        func_8004f1d0(obj, target, 0, &pos, &dir, -1, 0x400, 1);
        shot = func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir);
        if (shot != NULL) {
            shot->unkb = 0;
        }
        break;
    case 0x13:
        id = 0x71;
        goto lob;
    case 0x29:
    case 0x7d:
        id = 0x7d;
        goto lob;
    case 0x2d:
    case 0x81:
        id = 0x81;
    lob:
        AIM(600);
        delay = (fixed_vector3_length(pos.vx - target->vx, pos.vy - target->vy,
                                      pos.vz - target->vz) - 2000) / 600;
        if (delay < 0) {
            delay = 0;
        }
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, delay);
        break;
    case 0xd:
        AIM(0x2a8);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0xb:
        for (i = 0; i < 4; i++) {
            AIM(200);
            func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, i);
        }
        break;
    case 0x32:
        for (i = 0; i < 4; i++) {
            AIM(200);
            func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        }
        break;
    case 0x2a:
        AIM(600);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &obj->angle);
        break;
    case 0x2c:
        AIM(0x1a4);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x35:
        AIM(300);
        func_80053c84((u8)kind, 0x81, (u8)id, &pos, &dir);
        break;
    case 0x31:
        AIM(600);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x44:
        AIM(0x226);
        func_80053c84((u8)kind, 0x83, 0x7c, &pos, &dir);
        break;
    case 0x45:
        AIM(700);
        func_80053c84((u8)kind, 0x83, 0x7b, &pos, &dir);
        break;
    case 0x42:
        vector_displacement_to_pitch_yaw(target->vx - pos.vx, -(pos.vy + 1600) + target->vy,
                                         target->vz - pos.vz, &sv);
        pitch_yaw_to_forward_vector(&sv, &dir);
        vector3s_scale_shift12(0x400, &dir);
        pos.vx += dir.vx;
        pos.vy += dir.vy;
        pos.vz += dir.vz;
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &sv);
        break;
    case 2:
    case 0x57:
        AIM(400);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, 0xfe, 2);
        break;
    case 0x2e:
        id = 100;
    case 0x64:
        AIM(0x3c0);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x2f:
        AIM(0x1a4);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x30:
        AIM(0x1e0);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 7:
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, 0x1000, 0x100, 0x1000, 1);
        break;
    case 0x55:
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, NULL, DAT_80185da8.unk9d18, vertex,
                      SCRIPT_U16((u8 *)args[1], 4));
        break;
    case 0x28:
        pitch_yaw_to_forward_vector(&obj->angle, &dir);
        vector3s_scale_shift12(0x226, &dir);
        shot = func_80053c84((u8)kind, 0x83, 0x28, &pos, &dir);
        if (shot != NULL) {
            shot->unkb = 3;
        }
        break;
    case 6:
    case 0x33:
    case 0x34:
        AIM(300);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir);
        break;
    case 0x1f:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
        AIM(300);
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &obj->angle);
        break;
    case 0x12:
    case 0x14:
        vector_displacement_to_pitch_yaw(target->vx - pos.vx, -(pos.vy + 1600) + target->vy,
                                         target->vz - pos.vz, &sv);
        pitch_yaw_to_forward_vector(&sv, &dir);
        if (id == 0x12) {
            vector3s_scale_shift12(200, &dir);
        } else {
            vector3s_scale_shift12(20, &dir);
        }
        func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &obj->angle, 600, 60, 0x80, 0x8c, 0xa0);
        break;
    case 0x76:
        n = SCRIPT_U16((u8 *)args[1], 4);
        o = func_8004b524();
        if (o == NULL) {
            break;
        }
        AIM(400);
        o->unk0 = 5;
        o->unk2 = n;
        o->unk4 = 0;
        o->unk5 = 0;
        o->unk3 = obj->unk6;
        o->unk9 = 1;
        def = &DAT_80185da8.defs[n];
        o->unk28 = def->unk34;
        o->unk15 = def->unk9;
        o->pos.vx = pos.vx;
        o->pos.vy = pos.vy + 4096;
        o->pos.vz = pos.vz;
        func_8004b698(o);
        o->vel = dir;
        o->angle = obj->angle;
        func_8004c0b0(o, 0x1d);
        break;
    case 0x50:
    case 0x53:
        p = *target;
        i = 6;
        for (;;) {
            if (trajectory_solve_time_angle(0, fixed_vector2_length(p.vx - pos.vx, p.vz - pos.vz),
                                            pos.vy + (1400 - p.vy), 10, 800, &time, &pitch)) {
                pitch = 0x100;
            }
            if (--i == 0) {
                break;
            }
            vector_add_scaled_delta(target, &DAT_801b24e4.unk11c, time >> 6, &p);
        }
        sv.vy = func_8004f1d0(obj, &p, 800, &pos, &dir, (s16)pitch, 0xc00, 1);
        sv.vx = pitch;
        sv.vz = 0;
        shot = func_80053c84((u8)kind, 0x83, (u8)id, &pos, &dir, &sv);
        if (shot != NULL) {
            shot->arg.h.unk44 = pos.vy;
            shot->unk10 = 0x32;
            shot->unk7 = 0;
        }
        break;
    }
}

/* Note offset for obj's sound: lower for a larger record, plus a little noise. */
ADDRESS(0x8004ffa0, 0x70)
s32 func_8004ffa0(Object88 *obj)
{
    s32 note = 16 - (s8)(obj->scale.vy >> 8);

    if (note > 12) {
        note = 12;
    } else if (note < -12) {
        note = -12;
    }
    return note + (((rand() * 5) >> 15) - 2);
}

/* Plays the sound effect of obj's running script record at obj. */
ADDRESS(0x80050010, 0x98)
void func_80050010(Object88 *obj)
{
    u8 *script;

    script = obj->unk60;
    if (obj->unk28 & 0x800000) {
        audio_play_spatial_range(script[4] + 240, &obj->pos, 0x7f, 0x6000, 0x7800, func_8004ffa0(obj));
    } else {
        audio_play_spatial_default_range(script[4] + 240, &obj->pos, 0x6e, func_8004ffa0(obj));
    }
}

/*
 * Runs one frame of the current record's script record (DAT_80185da8.current
 * and .def): its sound and light fade first, then the record's behaviour by
 * script id (Object88.unke; unkf 0 starts it), FDAT slot 17 for the ids this
 * module does not know, and last the ride on its target record (flag 0x10).
 * The angles passed to func_8004e824 are narrowed to s16 here; the FDAT
 * callers pass theirs as int.
 */
ADDRESS(0x800500a8, 0x2db4)
void func_800500a8(void)
{
    Object88 *obj;
    Object78 *def;
    u8 *script;
    s32 phase;
    SVECTOR dir;
    VECTOR at;
    SVECTOR target;
    SVECTOR step;

    obj = DAT_80185da8.current;
    def = DAT_80185da8.def;
    script = obj->unk60;
    if (obj->unk28 & 4) {
        DAT_80185da8.unk9d04 = 3;
    } else {
        DAT_80185da8.unk9d04 = 0x93;
    }
    map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->unk1c, -1);
    if (script[4] != 0xff) {
        s32 n;
        s32 slot;

        n = SCRIPT_U16(script, 10) % 0x4000;
        if (script[0] != 0x15 || obj->unk70 == 2) {
            switch (SCRIPT_U16(script, 10) & 0xc000) {
            case 0:
                if (func_8004db98(obj, n)) {
                    func_80050010(obj);
                }
                break;
            case 0x8000:
                if ((rand() >> 3) < n) {
                    func_80050010(obj);
                }
                break;
            case 0x4000:
                slot = n * DAT_80185da8.unk9d18 / 3 % n;
                if (DAT_80185da8.unk9d24 % n == slot) {
                    func_80050010(obj);
                }
                break;
            }
        }
    }
    if (obj->unk28 & 0x200000) {
        u8 *light;

        light = func_8004c068(def, 2);
        if (light != NULL) {
            switch (obj->unk84) {
            case 0:
                obj->unk14 = 0x3b;
                obj->unk16 = 0;
                obj->unk13 &= 0xf8;
                obj->unk13 |= light[14];
                obj->unk84++;
                break;
            case 1:
                obj->unk16 += 512;
                if (obj->unk16 >= 4096) {
                    obj->unk16 = 0xfff;
                    obj->unk84++;
                }
                break;
            case 2:
                obj->unk16 -= 512;
                if (obj->unk16 < 0) {
                    obj->unk16 = 0;
                    obj->unk84++;
                }
                break;
            case 3:
                if (obj->unk1 < 50) {
                    obj->unk14 = 0x3e;
                } else {
                    obj->unk14 = 0x37;
                }
                if (obj->unk28 & 0x80) {
                    obj->unk13 |= 7;
                } else {
                    obj->unk13 &= 0xfb;
                }
                obj->unk16 = 0x800;
                obj->unk28 &= ~0x200000;
                break;
            }
        }
    }
    switch (obj->unke) {
    case 2:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = 0;
            func_8004c130(script[1]);
        }
        phase = obj->unk70;
        if (phase < 2 || !(obj->unk28 & 0x800)) {
            func_8004db3c(obj, SCRIPT_U16(script, 8));
        }
        switch (phase) {
        case 0:
            obj->unk14 = 0x3b;
            obj->unk16 = 0;
            obj->unk13 &= 0xf8;
            obj->unk13 |= script[14];
            obj->unk70++;
            break;
        case 1:
            if (obj->unk16 < 4096) {
                obj->unk16 += 512;
            }
            if (obj->unk18 >= 2047) {
                obj->unk70++;
            }
            break;
        case 2:
            if (obj->unk16 > 0) {
                obj->unk16 -= 512;
            }
            if (obj->unk18 >= 4095) {
                obj->unk70++;
            }
            break;
        case 3:
            if (obj->unk1 < 50) {
                obj->unk14 = 0x3e;
            } else {
                obj->unk14 = 0x37;
            }
            if (obj->unk28 & 0x80) {
                obj->unk13 |= 7;
            } else {
                obj->unk13 &= 0xfb;
            }
            obj->unk16 = 0x800;
            break;
        }
        if (phase == 3) {
            func_8004c104();
        }
        func_8004e6f8(def->unk1[2] * 2, 10);
        break;
    case 3: {
        u8 *light;

        phase = obj->unk70;
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
            obj->unk28 &= ~0x10000;
            obj->unk70 = 0;
            light = func_8004c068(def, 2);
            if (light != NULL) {
                obj->unk14 = 0x3b;
                obj->unk16 = 0;
                obj->unk13 &= 0xf8;
                obj->unk13 |= light[14];
            }
        }
        if (obj->unk70 == 0) {
            if (obj->unk18 >= 1024 && (obj->unk28 & 0x800)) {
                break;
            }
            func_8004db3c(obj, SCRIPT_U16(script, 8));
            if (func_8004c068(def, 2) != NULL) {
                if (obj->unk18 < 2048) {
                    if (obj->unk16 < 4096) {
                        obj->unk16 += 512;
                    }
                } else if (obj->unk16 > 0) {
                    obj->unk16 -= 512;
                }
            }
            if (func_8004db98(obj, 0x800)) {
                u16 drop;

                drop = def->unk30 + random_centered_triangular_scaled(def->unk30);
                if (drop != 0) {
                    map_object_spawn_scattered_effect(drop, &obj->pos, -(obj->unk1e >> 1));
                }
                if (obj->unk0 == 0 || obj->unk0 == 4) {
                    if (script[12] != 0xff && (rand() >> 7) < script[13]) {
                        map_object_spawn_effect(1, script[12], &obj->pos, -(obj->unk1e >> 1));
                    }
                } else if (obj->unk0 == 1) {
                    if (obj->unkb != 0xff) {
                        map_object_spawn_effect(0, obj->unkb, &obj->pos, -(obj->unk1e >> 1));
                    }
                }
            }
            if (obj->unk18 >= 4095 || SCRIPT_U16(script, 8) == 0) {
                obj->unk70 = 15;
                if (func_8004c068(def, 2) != NULL) {
                    if (obj->unk1 < 50) {
                        obj->unk14 = 0x3e;
                    } else {
                        obj->unk14 = 0x37;
                    }
                    if (obj->unk28 & 0x80) {
                        obj->unk13 |= 7;
                    } else {
                        obj->unk13 &= 0xfb;
                    }
                    obj->unk16 = 0x800;
                }
            }
        } else {
            if (obj->unk70 == 99) {
                break;
            }
            obj->unk70++;
            if (phase >= 20) {
                if (phase == 20) {
                    obj->unk14 = 0x32;
                    obj->unk16 = 0x400;
                    obj->unk13 = (obj->unk13 & 7) | 5;
                } else if (phase < 38) {
                    if (obj->unk16 < 4096) {
                        obj->unk16 += 192;
                    } else {
                        obj->unk16 = 0x1000;
                    }
                } else {
                    ((void (*)(Object88 *))DAT_8018fad4.handlers[19])(obj);
                    if (obj->unk0 == 1) {
                        func_8004b770(obj);
                    } else if (obj->unk0 == 2) {
                        obj->unk9 = 0;
                        func_8004b560(obj);
                    } else if (obj->unk0 == 5) {
                        obj->unk0 = 0xff;
                        obj->unk9 = 0;
                    } else {
                        obj->unk9 = 2;
                        func_8004b560(obj);
                    }
                }
            }
        }
        func_8004e6f8(def->unk1[2] * 2, 10);
        break;
    }
    case 0:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        func_8004e6f8(def->unk1[2], 10);
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        break;
    case 1:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
            obj->unk64 = rand() >> 3;
        } else if (func_8004ea7c(obj->unk64, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14), def->unk1[2],
                                 def->unk1[3], 5) ||
                   (rand() >> 5) < script[20]) {
            obj->unk64 = rand() >> 3;
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        break;
    case 12:
    case 16: {
        s32 flags;

        if (obj->unkf == 0) {
            obj->unkf = 0xf1;
            obj->unk72.vz = 0;
            obj->unk72.vy = 0;
            obj->unk72.vx = 0;
            obj->unk72.pad = collision_sample_map_layer_height(obj->unk6, (obj->unk8 << 11) + obj->unk24,
                                                               (obj->unk7 << 11) + obj->unk22, obj->unk1c,
                                                               obj->unk1e) +
                             obj->unk26;
            func_8004c184(script[1]);
            flags = 3;
            func_8004e31c();
        } else {
            flags = func_8004ed50(&obj->unk72, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14), def->unk1[2],
                                  def->unk1[3], 0x11);
            if ((rand() >> 5) < script[19]) {
                flags |= 1;
            }
            if ((rand() >> 5) < script[19]) {
                flags |= 2;
            }
            if (obj->unk72.pad + SCRIPT_U16(script, 16) < obj->pos.vy) {
                obj->vel.vy -= script[18];
            } else if (obj->pos.vy < obj->unk72.pad - SCRIPT_U16(script, 16)) {
                obj->vel.vy += script[18];
            }
        }
        if (flags & 1) {
            obj->unk72.vy = rand() >> 3;
        }
        if (flags & 2) {
            obj->unk72.vx = (rand() >> 5) - 512;
        }
        if (obj->unke == 16) {
            func_8004f008(script[1], script[20], script[21], script[22], script[23], SCRIPT_U16(script, 8));
        } else {
            func_8004db08(obj, SCRIPT_U16(script, 8));
        }
        break;
    }
    case 5: {
        s32 r;
        s32 mode;
        s32 dist;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
            switch (obj->unk10) {
            case 4:
            case 18:
            case 23:
            case 24:
                OBJECT88_B70(obj) = 1;
                break;
            default:
                OBJECT88_B70(obj) = 0;
                break;
            }
            OBJECT88_B71(obj) = 0;
        }
        if (rand() < 6000) {
            dist = fixed_vector2_length(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vz - obj->pos.vz);
            if (dist >= SCRIPT_U16(script, 22)) {
                OBJECT88_B70(obj) = 0;
            } else if (dist <= SCRIPT_U16(script, 20)) {
                OBJECT88_B70(obj) = 1;
            }
        }
        obj->unk64 = vector_xz_to_angle(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vz - obj->pos.vz);
        mode = 0x21;
        if (OBJECT88_B71(obj) == 0) {
            mode = 5;
        }
        if (OBJECT88_B70(obj) == 0) {
            r = func_8004ea7c(obj->unk64, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3],
                              mode);
        } else {
            r = func_8004e824((s16)(obj->unk64 + 2048), SCRIPT_U16(script, 12), def->unk1[2], mode);
        }
        if (r & 0x100) {
            if (OBJECT88_B71(obj) == 0) {
                if (rand() < SCRIPT_U16(script, 24)) {
                    OBJECT88_B71(obj) = 1;
                } else if (r != 0) {
                    OBJECT88_B70(obj) = OBJECT88_B70(obj) == 0;
                }
            }
        } else {
            OBJECT88_B71(obj) = 0;
            if (r != 0) {
                OBJECT88_B70(obj) = OBJECT88_B70(obj) == 0;
            }
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        break;
    }
    case 13:
    case 17: {
        s32 dx;
        s32 dy;
        s32 dz;
        s32 dist;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c184(script[1]);
            obj->unk70 = 0;
        }
        dx = DAT_801b24e4.pos.vx - obj->pos.vx;
        dy = DAT_801b24e4.pos.vy - obj->pos.vy - 1600;
        dz = DAT_801b24e4.pos.vz - obj->pos.vz;
        dist = fixed_vector3_length(dx, dy, dz);
        vector_displacement_to_pitch_yaw(dx, dy, dz, &obj->unk72);
        if (obj->unk72.vx > 0xe00) {
            obj->unk72.vx = 0xe00;
        } else if (obj->unk72.vx > 0x200) {
            obj->unk72.vx = 0x200;
        }
        if (dist >= SCRIPT_U16(script, 22)) {
            obj->unk70 = 0;
            obj->unkf = 0xf1;
        } else if (dist <= SCRIPT_U16(script, 20)) {
            obj->unk70 = 1;
            obj->unkf = 0xf0;
        }
        if (obj->unk70 == 0) {
            func_8004ed50(&obj->unk72, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3],
                          0x11);
        } else {
            dir.vx = -512;
            dir.vy = obj->unk72.vy + 2048;
            dir.vz = 0;
            func_8004ed50(&dir, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3], 0x11);
        }
        if (obj->unke == 17) {
            func_8004f008(script[1], script[24], script[25], script[26], script[27], SCRIPT_U16(script, 8));
        } else {
            func_8004db08(obj, SCRIPT_U16(script, 8));
        }
        break;
    }
    case 9:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = 0;
            func_8004c130(script[20]);
        }
        switch (obj->unk70) {
        case 0:
            if (func_8004e824((s16)(obj->angle.vy + 2048), SCRIPT_U16(script, 18), def->unk1[2], 5) == 0) {
                func_8004db08(obj, SCRIPT_U16(script, 8));
                if (fixed_vector2_length(DAT_801b24e4.pos.vx - obj->pos.vx,
                                         DAT_801b24e4.pos.vz - obj->pos.vz) <
                    SCRIPT_U16(script, 14)) {
                    break;
                }
            }
            obj->unk70 = 1;
            func_8004c130(script[1]);
            break;
        case 1:
            func_8004db3c(obj, script[22]);
            if (obj->unk18 < 4095) {
                func_8004e6f8(def->unk1[2] * 2, 10);
                break;
            }
            if ((s16)func_8004e27c(1, DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vy - 500, DAT_801b24e4.pos.vz, script[23],
                                   SCRIPT_U16(script, 16)) > 0) {
                obj->unk70 = 2;
                func_8004e6f8(def->unk1[2] * 2, 10);
                break;
            }
            obj->unk70 = 3;
            func_8004c130(script[21]);
            break;
        case 2:
            if (obj->unkd == 0) {
                obj->unk70 = 3;
                func_8004c130(script[21]);
                break;
            }
            func_8004ea7c(obj->angle.vy, obj->unk68, 0, obj->unk68, def->unk1[3], 10);
            break;
        case 3:
            func_8004db3c(obj, script[22]);
            if (obj->unk18 >= 4095) {
                func_8004c104();
            }
            func_8004e6f8(obj->unk68 >> 4, 10);
            break;
        }
        break;
    case 10: {
        s32 r;

        if (obj->unkf == 0) {
            obj->unkf = 0xf1;
            func_8004c184(script[1]);
            func_8004e31c();
        }
        r = rand();
        if (r < 2048) {
            obj->vel.vx += SCRIPT_U16(script, 16);
            if (obj->vel.vx > SCRIPT_U16(script, 12)) {
                obj->vel.vx = SCRIPT_U16(script, 12);
            }
        } else if (r < 4096) {
            obj->vel.vx -= SCRIPT_U16(script, 16);
            if (obj->vel.vx < -SCRIPT_U16(script, 12)) {
                obj->vel.vx = -SCRIPT_U16(script, 12);
            }
        }
        r = rand();
        if (r < 2048) {
            obj->vel.vz += SCRIPT_U16(script, 16);
            if (obj->vel.vz > SCRIPT_U16(script, 12)) {
                obj->vel.vz = SCRIPT_U16(script, 12);
            }
        } else if (r < 4096) {
            obj->vel.vz -= SCRIPT_U16(script, 16);
            if (obj->vel.vz < -SCRIPT_U16(script, 12)) {
                obj->vel.vz = -SCRIPT_U16(script, 12);
            }
        }
        r = rand();
        if (r < 2048) {
            obj->vel.vy += SCRIPT_U16(script, 16);
            if (obj->vel.vy > SCRIPT_U16(script, 12)) {
                obj->vel.vy = SCRIPT_U16(script, 12);
            }
        } else if (r < 4096) {
            obj->vel.vy -= SCRIPT_U16(script, 16);
            if (obj->vel.vy < -SCRIPT_U16(script, 12)) {
                obj->vel.vy = -SCRIPT_U16(script, 12);
            }
        }
        func_8004e094(&obj->vel);
        func_8004db08(obj, SCRIPT_U16(script, 8));
        func_8004e928(obj, vector_xz_to_angle(obj->vel.vx, obj->vel.vz), SCRIPT_U16(script, 14),
                      def->unk1[3]);
        break;
    }
    case 132: {
        s16 *p;
        s32 c;
        s32 tries;
        u16 time;
        VECTOR *player;

        tries = 1;
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = SCRIPT_U16(script, 24);
            obj->unk72.vx = 0;
            obj->unk72.vy = 0;
            func_8004c130(script[1]);
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        p = (s16 *)(script + 28) + obj->unk72.vx;
        for (;;) {
            c = *p;
            obj->unk72.vx++;
            p++;
            if (c == 0x8000) {
                p = (s16 *)(script + 28);
                obj->unk72.vx = 0;
                continue;
            }
            if (c == 0x8001) {
                tries = *p++;
                obj->unk72.vx++;
                continue;
            }
            if (c == 0x8003) {
                p += *p + 1;
                continue;
            }
            if (c == 0x8002) {
                at.vx = *p++;
                obj->unk72.vx++;
                at.vy = *p++;
                obj->unk72.vx++;
                at.vz = *p++;
                obj->unk72.vx++;
            } else {
                func_8004ede0(obj, c, &at);
            }
            at.vx += obj->pos.vx;
            at.vy += obj->pos.vy;
            at.vz += obj->pos.vz;
            if (vector_distance_to_point(&at, DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vy, DAT_801b24e4.pos.vz,
                                         script[14], script[15], 0x6a4) != -1) {
                time = obj->unk18;
                obj->unk18 -= obj->unk66 << 2;
                if ((s16)obj->unk18 < 0) {
                    obj->unk18 = 0;
                }
                func_8004ede0(obj, c, &at);
                at.vx += obj->pos.vx;
                at.vy += (script[15] << 4) + obj->pos.vy;
                at.vz += obj->pos.vz;
                player_apply_damage(SCRIPT_U16(script, 18), SCRIPT_U16(script, 20), SCRIPT_U16(script, 22),
                                    script[17], 0, 0, 0, 0, 0, 0x1000, 10, &at);
                obj->unk18 = time;
                break;
            }
            if (--tries == 0) {
                break;
            }
        }
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        func_8004e6f8(script[13], 10);
        break;
    }
    case 4:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (func_8004db98(obj, SCRIPT_U16(script, 24))) {
            func_8004d254(0, script[14], script[15], script[16], SCRIPT_U16(script, 18), SCRIPT_U16(script, 20),
                          SCRIPT_U16(script, 22), script[17]);
        }
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        func_8004e6f8(script[13], 10);
        break;
    case 23:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = SCRIPT_U16(script, 24);
            func_8004c130(script[1]);
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (func_8004db98(obj, obj->unk70)) {
            obj->unk70 += SCRIPT_U16(script, 40);
            if (obj->unk70 > SCRIPT_U16(script, 38)) {
                obj->unk70 = 0;
            }
            func_8004d254(0, script[14], script[15], script[16], SCRIPT_U16(script, 18), SCRIPT_U16(script, 20),
                          SCRIPT_U16(script, 22), script[17] | 0x80);
        }
        if (SCRIPT_U16(script, 42) != 0 && func_8004db98(obj, SCRIPT_U16(script, 42))) {
            func_8004d254(0, script[28], script[29], script[30], SCRIPT_U16(script, 32), SCRIPT_U16(script, 34),
                          SCRIPT_U16(script, 36), script[31]);
        }
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        func_8004e6f8(script[13], 10);
        break;
    case 24: {
        s32 speed;
        s32 turn;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (obj->unk18 >= SCRIPT_U16(script, 34)) {
            speed = 0;
            turn = SCRIPT_S16(script, 38);
        } else if (obj->unk18 >= SCRIPT_U16(script, 32)) {
            speed = SCRIPT_U16(script, 28);
            turn = SCRIPT_S16(script, 36);
        } else {
            speed = SCRIPT_U16(script, 28);
            turn = script[13];
        }
        obj->unk64 = vector_xz_to_angle(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vz - obj->pos.vz);
        func_8004ea7c(obj->unk64, speed, SCRIPT_U16(script, 30), turn, def->unk1[3], 4);
        if (func_8004db98(obj, SCRIPT_U16(script, 24))) {
            func_8004d254(0, script[14], script[15], script[16], SCRIPT_U16(script, 18), SCRIPT_U16(script, 20),
                          SCRIPT_U16(script, 22), script[17]);
        }
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        break;
    }
    case 11: {
        s32 hit;
        u16 spin;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c184(script[1]);
            obj->unk70 = 0;
            func_8004e31c();
        }
        switch (obj->unk70) {
        case 0:
            func_8004db3c(obj, SCRIPT_U16(script, 8));
            if (obj->unk18 >= 4095) {
                vector_displacement_to_pitch_yaw(DAT_801b24e4.pos.vx - obj->pos.vx,
                                                 DAT_801b24e4.pos.vy - obj->pos.vy,
                                                 DAT_801b24e4.pos.vz - obj->pos.vz, &dir);
                pitch_yaw_to_forward_vector(&dir, &obj->unk72);
                obj->unk70 = 1;
            }
            func_8004e094(&obj->vel);
            break;
        case 1:
            target = step = obj->unk72;
            vector3s_scale_shift12(SCRIPT_U16(script, 20), &target);
            vector3s_scale_shift12(SCRIPT_U16(script, 24), &step);
            obj->vel.vx = value_approach(obj->vel.vx, target.vx, step.vx);
            obj->vel.vy = value_approach(obj->vel.vy, target.vy, step.vy);
            obj->vel.vz = value_approach(obj->vel.vz, target.vz, step.vz);
            hit = func_8004e094(&obj->vel);
            if (hit != 0) {
                if (hit & 0x80) {
                    player_apply_damage(SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), SCRIPT_U16(script, 18), script[13], 0,
                                        0, 0, 0, 0, 0x1000, 10, &obj->pos);
                }
                obj->unk70 = 2;
            }
            obj->angle.vx = (obj->angle.vx + 256) & 0xfff;
            break;
        case 2:
            spin = (obj->angle.vx + 256) & 0xfff;
            obj->angle.vx = spin;
            if (spin < 256) {
                obj->angle.vx = 0;
                obj->unk70 = 3;
            }
            func_8004e094(&obj->vel);
            break;
        case 3:
            func_8004db3c(obj, -SCRIPT_U16(script, 8));
            if (obj->unk18 == 0) {
                func_8004c104();
            }
            func_8004e094(&obj->vel);
            break;
        }
        func_8004e928(obj, vector_xz_to_angle(obj->vel.vx, obj->vel.vz), SCRIPT_U16(script, 22),
                      def->unk1[3]);
        break;
    }
    case 18: {
        s32 dx;
        s32 dy;
        s32 dz;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c184(script[1]);
        }
        dx = DAT_801b24e4.pos.vx - obj->pos.vx;
        dy = DAT_801b24e4.pos.vy - obj->pos.vy - 1600;
        dz = DAT_801b24e4.pos.vz - obj->pos.vz;
        fixed_vector3_length(dx, dy, dz);
        vector_displacement_to_pitch_yaw(dx, dy, dz, &obj->unk72);
        func_8004ed50(&obj->unk72, SCRIPT_U16(script, 28), SCRIPT_U16(script, 30), script[13], def->unk1[3], 6);
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (func_8004db98(obj, SCRIPT_U16(script, 24))) {
            func_8004d254(0, script[14], script[15], script[16], SCRIPT_U16(script, 18), SCRIPT_U16(script, 20),
                          SCRIPT_U16(script, 22), script[17]);
        }
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        break;
    }
    case 14: {
        Object88 *target;

        target = DAT_80185da8.target;
        if (obj->unkf == 0) {
            obj->unkf = 0xf1;
            obj->unk70 = 0;
        }
        switch (obj->unk70) {
        case 0:
            if (target->unk58 > 0) {
                func_8004c130(script[13]);
                obj->unk70 = 1;
            } else if (target->unk58 < 0) {
                func_8004c130(script[12]);
                obj->unk70 = 2;
            }
            break;
        case 1:
            if (obj->unk18 < 2048) {
                obj->unk18 += 128;
            } else if (target->unk58 <= 0) {
                obj->unk70 = 3;
            }
            break;
        case 2:
            if (obj->unk18 < 2048) {
                obj->unk18 += 128;
            } else if (target->unk58 >= 0) {
                obj->unk70 = 3;
            }
            break;
        case 3:
            obj->unk18 -= 128;
            if (obj->unk18 == 0) {
                obj->unk70 = 0;
            }
            break;
        }
        break;
    }
    case 15:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c184(script[1]);
        }
        func_8004ea7c(obj->unk64, SCRIPT_U16(script, 12), SCRIPT_U16(script, 14), def->unk1[2], def->unk1[3], 5);
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        break;
    case 19:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = 0;
            func_8004c184(script[1]);
        }
        switch (obj->unk70) {
        case 0:
            func_8004db3c(obj, SCRIPT_U16(script, 8));
            if (obj->unk18 >= SCRIPT_U16(script, 16)) {
                obj->unk70 = 16;
            }
            break;
        case 16:
            if (DAT_801b24e4.unkc0 == -1) {
                obj->unk70 = 32;
            }
            break;
        case 32:
            func_8004db3c(obj, SCRIPT_U16(script, 8));
            if (obj->unk18 >= 4095) {
                func_8004c104();
            }
            break;
        }
        func_8004e6f8(def->unk1[2], 10);
        break;
    case 20:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = SCRIPT_U16(script, 18);
            func_8004c184(script[1]);
        }
        if (func_8004e824((s16)(obj->angle.vy + 2048), SCRIPT_U16(script, 16), 1000, 5)) {
            obj->unk70 = 1;
        }
        func_8004db08(obj, SCRIPT_U16(script, 8));
        if (obj->unk18 >= 0xfff - SCRIPT_U16(script, 8) && --obj->unk70 == 0) {
            obj->unk18 = 0xfff;
            func_8004c104();
        }
        break;
    case 22:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        func_8004e6f8(def->unk1[2], 10);
        break;
    case 21: {
        s32 mask;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = 0;
            func_8004c130(script[1]);
            obj->unk1c = SCRIPT_U16(script, 20);
            obj->unk1e = SCRIPT_U16(script, 22);
        }
        switch (obj->unk70) {
        case 0:
            if (fixed_vector2_length(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vz - obj->pos.vz) <
                SCRIPT_U16(script, 12)) {
                obj->unk70 = 2;
            }
            break;
        case 2:
            func_8004db3c(obj, SCRIPT_U16(script, 8));
            obj->unk1c = fixed_lerp_q12(SCRIPT_U16(script, 20), def->unk12, obj->unk18);
            obj->unk1e = fixed_lerp_q12(SCRIPT_U16(script, 22), def->unk14, obj->unk18);
            if (obj->unk18 >= 4095) {
                obj->unk1c = def->unk12;
                obj->unk1e = def->unk14;
                obj->unk70 = 3;
                obj->unk72.vx = SCRIPT_U16(script, 16);
                func_8004c130(script[24]);
            }
            if (obj->unk18 >= SCRIPT_U16(script, 18)) {
                mask = DAT_80185da8.unk9d04;
                DAT_80185da8.unk9d04 = mask & ~0x20;
                func_8004ea7c(obj->unk64, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
                DAT_80185da8.unk9d04 = mask;
            }
            break;
        case 3:
            mask = DAT_80185da8.unk9d04;
            DAT_80185da8.unk9d04 = mask & ~0x20;
            func_8004ea7c(obj->unk64, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
            if (--obj->unk72.vx == -1) {
                func_8004c104();
            }
            DAT_80185da8.unk9d04 = mask;
            func_8004db08(obj, script[25]);
            break;
        }
        break;
    }
    case 6: {
        s32 mask;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = 0;
            func_8004c130(script[1]);
            obj->unk1c = SCRIPT_U16(script, 22);
            obj->unk1e = SCRIPT_U16(script, 24);
            if (SCRIPT_U16(script, 14) != 0) {
                obj->unk64 = rand() >> 3;
            }
        }
        switch (obj->unk70) {
        case 0:
            func_8004db08(obj, SCRIPT_U16(script, 8));
            if (fixed_vector2_length(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vz - obj->pos.vz) <
                    SCRIPT_U16(script, 12) &&
                obj->unk18 >= 0xfff - SCRIPT_U16(script, 8)) {
                obj->unk70 = 2;
                obj->unk18 = 0;
                func_8004c130(script[26]);
            }
            if (func_8004ea7c(obj->unk64, SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), def->unk1[2], def->unk1[3],
                              5) ||
                (rand() >> 5) < script[28]) {
                obj->unk64 = rand() >> 3;
            }
            break;
        case 2:
            func_8004db3c(obj, SCRIPT_U16(script, 8));
            obj->unk1c = fixed_lerp_q12(SCRIPT_U16(script, 22), def->unk12, obj->unk18);
            obj->unk1e = fixed_lerp_q12(SCRIPT_U16(script, 24), def->unk14, obj->unk18);
            if (obj->unk18 >= 4095) {
                obj->unk1c = def->unk12;
                obj->unk1e = def->unk14;
                obj->unk70 = 3;
                obj->unk72.vx = SCRIPT_U16(script, 18);
            } else if (obj->unk18 >= SCRIPT_U16(script, 20)) {
                mask = DAT_80185da8.unk9d04;
                DAT_80185da8.unk9d04 = mask & ~0x20;
                func_8004ea7c(obj->unk64, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
                DAT_80185da8.unk9d04 = mask;
            }
            break;
        case 3:
            mask = DAT_80185da8.unk9d04;
            DAT_80185da8.unk9d04 = mask & ~0x20;
            func_8004ea7c(obj->unk64, SCRIPT_U16(script, 14), 0, 0xff, 0, 5);
            if (--obj->unk72.vx == -1) {
                func_8004c104();
            }
            DAT_80185da8.unk9d04 = mask;
            func_8004db3c(obj, script[27]);
            break;
        }
        break;
    }
    case 26: {
        Object44 *door;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            obj->unk70 = 0;
            func_8004c130(script[1]);
            obj->scale.vz = 0;
            obj->scale.vy = 0;
            obj->scale.vx = 0;
        }
        switch (obj->unk70) {
        case 0:
            door = &DAT_8018fb3c.objects[obj->unk20];
            if (door->unk8 < 2 || door->angle.vx == 0 || door->angle.vx >= 3072) {
                break;
            }
            obj->unk70 = 2;
            obj->unk72.vx = 8;
            obj->unk1c = def->unk12;
            obj->unk1e = def->unk14;
            obj->pos.vy += 2048;
            obj->scale.vx = obj->scale.vy = obj->scale.vz = def->unk32;
            obj->angle.vy = door->angle.vy;
        case 2:
            obj->unkd = 0;
            obj->pos.vy -= 256;
            if (--obj->unk72.vx == 0) {
                func_8004c104();
            }
            break;
        }
        break;
    }
    case 25: {
        u16 *p;
        s32 c;
        s32 tries;
        s16 x;
        s16 y;
        s16 z;
        s32 a;
        s32 b;
        s32 d;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
            obj->unk70 = SCRIPT_U16(script, 14);
            obj->unk72.vx = 0;
            obj->unk72.vy = 0;
            if (script[7] == 1) {
                obj->unkd = 0x10;
            }
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (func_8004db98(obj, obj->unk70)) {
            obj->unk70 += SCRIPT_U16(script, 18);
            if (obj->unk70 > SCRIPT_U16(script, 16)) {
                obj->unk70 = 0;
            }
            tries = 1;
            p = (u16 *)(script + 26) + obj->unk72.vx;
            for (;;) {
                c = *p;
                obj->unk72.vx++;
                p++;
                if (c == 0x8000) {
                    p = (u16 *)(script + 26);
                    obj->unk72.vx = 0;
                    continue;
                }
                if (c == 0x8001) {
                    tries = *p++;
                    obj->unk72.vx++;
                    continue;
                }
                if (c == 0x8003) {
                    p += *p + 1;
                    continue;
                }
                if (c == 0x8002) {
                    x = *p++;
                    obj->unk72.vx++;
                    y = *p++;
                    obj->unk72.vx++;
                    z = *p++;
                    obj->unk72.vx++;
                    func_8004f414(script[12], SCRIPT_U16(script, 24), -1, x, y, z, p);
                } else if (c == 0x8004) {
                    a = *p++;
                    b = *p++;
                    d = *p++;
                    obj->unk72.vx += 3;
                    func_8004f414(script[12], SCRIPT_U16(script, 24), -2, a, b, d);
                } else {
                    func_8004f414(script[12], SCRIPT_U16(script, 24), c, p + tries - 1, obj->unk72.vy);
                }
                if (--tries == 0) {
                    break;
                }
            }
            obj->unk72.vy++;
        }
        if (obj->unk18 >= 4095) {
            func_8004c104();
        }
        func_8004e6f8(def->unk1[2], 10);
        break;
    }
    case 27: {
        u8 *next;

        if (obj->unkf == 0) {
            obj->unkf = 0xf1;
            obj->unk70 = 0;
            obj->unk72.vx = -1;
            func_8004c130(script[22]);
        }
        switch (obj->unk70) {
        case 0:
            if (SCRIPT_U16(script, 14) == 0) {
                obj->unk70 = 1;
            } else {
                obj->unk72.vx = func_8004eaf0((obj->unk8 << 11) + obj->unk24, (obj->unk7 << 11) + obj->unk22,
                                              SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), obj->unk72.vx,
                                              def->unk1[2], def->unk1[3], 5);
                if (obj->unk72.vx == -1) {
                    obj->unk70 = 1;
                }
            }
            func_8004db08(obj, script[23]);
            break;
        case 1:
            func_8004e928(obj, obj->unk20, SCRIPT_U16(script, 16), def->unk1[3]);
            if (obj->angle.vy == obj->unk20) {
                func_8004c130(script[1]);
                obj->unk70 = 2;
            }
            break;
        case 2:
            func_8004db3c(obj, SCRIPT_U16(script, 8));
            obj->unk1c = fixed_lerp_q12(def->unk12, SCRIPT_U16(script, 18), obj->unk18);
            obj->unk1e = fixed_lerp_q12(def->unk14, SCRIPT_U16(script, 20), obj->unk18);
            if (obj->unk18 >= 4095) {
                obj->unk1c = SCRIPT_U16(script, 18);
                obj->unk1e = SCRIPT_U16(script, 20);
                next = func_8004c068(def, 0x15);
                if (next != NULL) {
                    func_8004b94c(obj, next);
                } else {
                    func_8004c104();
                }
            }
            break;
        }
        break;
    }
    case 29:
        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
            func_8004e31c();
            obj->unk72.vx = 16;
        }
        obj->angle.vx += 128;
        obj->angle.vy += 64;
        obj->pos.vy -= 256;
        if (--obj->unk72.vx == 0) {
            func_8004c104();
        }
        break;
    case 28: {
        s32 hit;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
            func_8004e31c();
            audio_play_spatial_default_range(0x32, &obj->pos, 0x78, 0);
        }
        obj->angle.vx += 128;
        obj->angle.vy += 64;
        hit = func_8004e094(&obj->vel);
        if (hit != 0) {
            if (hit & 0x80) {
                player_apply_damage(SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), SCRIPT_U16(script, 18),
                                    SCRIPT_U16(script, 12), SCRIPT_U16(script, 20), SCRIPT_U16(script, 22),
                                    SCRIPT_U16(script, 24), SCRIPT_U16(script, 26), SCRIPT_U16(script, 28),
                                    0x1000, 10, &obj->pos);
            }
            func_8004c0b0(obj, 3);
            obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
            obj->unkd = 0x10;
        }
        break;
    }
    case 30: {
        VECTOR next;
        s32 hit;

        if (obj->unkf == 0) {
            obj->unkf = 0xf0;
            func_8004c130(script[1]);
            func_8004e31c();
        }
        next.vx = obj->vel.vx + obj->pos.vx;
        next.vy = obj->vel.vy + obj->pos.vy;
        next.vz = obj->vel.vz + obj->pos.vz;
        hit = collision_query_world(next.vx, next.vy, next.vz, obj->unk1c, OBJECT88_SHAPE(obj),
                                    DAT_80185da8.unk9d04);
        if (hit == 0) {
        move:
            obj->pos.vx = next.vx;
            obj->pos.vy = next.vy;
            obj->pos.vz = next.vz;
        } else if (hit & 0x80) {
            player_apply_damage(SCRIPT_U16(script, 14), SCRIPT_U16(script, 16), SCRIPT_U16(script, 18),
                                SCRIPT_U16(script, 12), SCRIPT_U16(script, 20), SCRIPT_U16(script, 22),
                                SCRIPT_U16(script, 24), SCRIPT_U16(script, 26), SCRIPT_U16(script, 28),
                                0x1000, 10, &obj->pos);
            goto move;
        } else {
            obj->vel.vx = obj->vel.vy = obj->vel.vz = 0;
        }
        func_8004db3c(obj, SCRIPT_U16(script, 8));
        if (obj->unk18 >= 4095) {
            func_8004c0b0(obj, 3);
            obj->unkd = 0x10;
        }
        break;
    }
    case 31:
        phase = obj->unk80;
        if (phase == 0 && SCRIPT_U16(script, 14) != 0) {
            obj->unk80++;
            obj->angle.vy = DAT_801b24e4.unk124.vy + 2048;
            func_8004c0b0(obj, 0x19);
        } else {
            obj->unk28 &= ~0x1000000;
            obj->unk80++;
            if (phase > 0) {
                if (phase == 1) {
                    obj->unk14 = 0x32;
                    obj->unk16 = 0x400;
                    obj->unk13 = (obj->unk13 & 7) | 5;
                } else if (phase < 38) {
                    if (obj->unk16 < 4096) {
                        obj->unk16 += 192;
                    } else {
                        obj->unk16 = 0x1000;
                    }
                } else if (obj->unk0 == 1) {
                    func_8004b770(obj);
                } else if (obj->unk0 == 2) {
                    obj->unk9 = 0;
                    func_8004b560(obj);
                } else if (obj->unk0 == 5) {
                    obj->unk0 = 0xff;
                    obj->unk9 = 0;
                } else {
                    obj->unk9 = 2;
                    func_8004b560(obj);
                }
            }
        }
        func_8004e6f8(def->unk1[2] * 2, 10);
        break;
    case 240:
        func_8004e31c();
        break;
    default:
        DAT_8018fad4.handlers[17]();
        break;
    }
    if (obj->unk28 & 0x10) {
        Object88 *target;
        SVECTOR rot;
        VECTOR from;
        VECTOR to;
        s32 hit;

        target = DAT_80185da8.target;
        if (obj->unk0 == 3) {
            obj->unk3 = 0;
            if (target->unk9 != 1) {
                obj->unk9 = 0;
                goto done;
            }
            if (target->unke == 3) {
                obj->unke = 3;
                obj->unkf = 0xf0;
                obj->unk70 = 99;
                goto done;
            }
        } else {
            obj->unk3 = target->unk3;
            if (target->unk9 != 1) {
                obj->unk28 = (obj->unk28 & ~0x10) | 0x100;
                hit = collision_query_world(obj->pos.vx, obj->pos.vy, obj->pos.vz, obj->unk1c, obj->unk1e,
                                            0x81);
                if (hit & 0x80) {
                    obj->pos.vx += 800 + obj->unk1c;
                }
                if (hit & 0xf) {
                    obj->unk28 |= 0x400;
                }
                if (obj->unk28 & 0x200) {
                    func_8004c0b0(obj, 3);
                }
                obj->vel.vx = target->vel.vx;
                obj->vel.vy = target->vel.vy;
                obj->vel.vz = target->vel.vz;
                goto done;
            }
        }
        obj->angle.vx = target->angle.vx;
        obj->angle.vz = target->angle.vz;
        rot.vx = target->angle.vx;
        rot.vy = target->angle.vy;
        rot.vz = target->angle.vz;
        vector_rotate_yxz(&rot, (SVECTOR *)&def->unkc, &from);
        func_8004ede0(target, obj->unk24, &to);
        obj->pos.vx = target->pos.vx + to.vx - from.vx;
        obj->pos.vy = target->pos.vy + to.vy - from.vy;
        obj->pos.vz = target->pos.vz + to.vz - from.vz;
    } else if (!(obj->unk28 & 0x10000)) {
        func_8004e330();
    }
done:
    if (obj->unk9 == 1) {
        map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, obj->unk1c, 1);
    }
}
