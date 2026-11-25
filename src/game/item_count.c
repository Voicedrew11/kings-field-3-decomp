/*
 * GAME.EXE: item counts (WIP module): the two counts per id in
 * DAT_800c85e8, a timed screen tint fade and a brightening loop, and the
 * item pick-up shown in front of the viewer.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/image.h>
#include <game/data.h>
#include <game/render.h>
#include <game/player.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/entity.h>
#include <game/cd.h>
#include <game/menu.h>
#include <game/memory.h>
#include <psyq/libc.h>
#include <stdarg.h>

/* Non-zero when either count of id is. */
ADDRESS(0x8005d7bc, 0x3c)
s32 func_8005d7bc(s32 id)
{
    if (DAT_800c85e8[0][id] == 0) {
        return DAT_800c85e8[1][id] != 0;
    }
    return 1;
}

/* Takes one of id (first count first); 1 when there is none. */
ADDRESS(0x8005d7f8, 0xa0)
s32 func_8005d7f8(s32 id)
{
    if (DAT_800c85e8[0][id] != 0) {
        DAT_800c85e8[0][id]--;
    } else if (DAT_800c85e8[1][id] != 0) {
        DAT_800c85e8[1][id]--;
    } else {
        return 1;
    }
    DAT_8018fad4.handlers[11]();
    return 0;
}

/* Adds one of id (each count holds 99); 1 and message 11 when both are full. */
ADDRESS(0x8005d898, 0xb0)
s32 func_8005d898(s32 id)
{
    if (DAT_800c85e8[0][id] < 99) {
        DAT_800c85e8[0][id]++;
    } else if (DAT_800c85e8[1][id] < 99) {
        DAT_800c85e8[1][id]++;
    } else {
        notify_enqueue(11);
        return 1;
    }
    DAT_8018fad4.handlers[6]();
    return 0;
}

/* Fades the screen tint from (r0, g0, b0) to (r1, g1, b1), step per frame out of 4096. */
ADDRESS(0x8005d948, 0x14c)
void func_8005d948(s32 step, s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1)
{
    s32 t;
    s32 r;
    s32 g;

    t = 0;
    do {
        light_reset_records();
        render_reset_color_overlays();
        light_refresh_record_rotations();
        r = fixed_lerp_q12(r0, r1, t);
        g = fixed_lerp_q12(g0, g1, t);
        accumulate_color_overlay(r, g, fixed_lerp_q12(b0, b1, t), 0x800, 1);
        t += step;
        player_update_status_visuals();
        render_game_frame(NULL, NULL);
    } while (t < 4096);
    light_reset_records();
    render_reset_color_overlays();
    light_refresh_record_rotations();
    accumulate_color_overlay(r1, g1, b1, 0x800, 1);
    player_update_status_visuals();
    render_game_frame(NULL, NULL);
}

/* Brightens the screen over 33 frames, then sets DAT_8009c3f8 to 1. */
ADDRESS(0x8005da94, 0x9c)
void func_8005da94(void)
{
    VECTOR pos;
    SVECTOR angle;
    s32 i;
    s32 v;

    i = 0;
    for (;;) {
        v = i * i >> 16;
        if (v >= 256) {
            v = 255;
        }
        render_set_color_overlay(0x82, v, v, v);
        cd_request_service_vab();
        cd_request_service_stream();
        player_get_camera_pose(&pos, &angle);
        render_game_frame(&pos, &angle);
        if (i >= 4096) {
            break;
        }
        i += 128;
    }
    DAT_8009c3f8 = 1;
}

/*
 * Picks up obj: lifts it in front of the viewer (levelling the view pitch)
 * and spins it until a button press; the pick-up button (DAT_80081868[7]) or
 * a forced id adds it to the item counts, else it turns back and returns to
 * its place. obj NULL spawns a record for the def id given after it.
 * pad doubles as the taken flag after the press. This module passes
 * angle_lerp_shortest_q12 both angles narrowed to s16 (an s16 prototype in its view;
 * the callee narrows them again).
 */
ADDRESS(0x8005db30, 0x7a0)
void func_8005db30(Object44 *obj, ...)
{
    va_list args;
    SVECTOR angle_to;
    SVECTOR angle_from;
    VECTOR to;
    VECTOR from;
    u8 *model;
    Object44Def *def;
    Object44 *link;
    Object44 *driver;
    s32 id;
    s32 pitch;
    s32 start;
    s16 spin;
    s16 held;
    s32 model_id;
    s32 t;
    u32 pad;
    u32 prev;
    s32 n;

    id = -1;
    if (obj == NULL) {
        va_start(args, obj);
        id = va_arg(args, s32);
        va_end(args);
        obj = map_object_effect_pool_acquire(350, 10, DAT_8018fb3c.unk885e);
        map_object_reset(obj);
        obj->unk6 = id;
        obj->unk0 = 2;
        obj->unk4 = 0xff;
        obj->angle.vz = 0;
        obj->angle.vy = 0;
        obj->angle.vx = 0;
        obj->arg.b.unk38 = 0xff;
        obj->pos = DAT_801b24e4.pos;
        obj->pos.vx += rcos(DAT_80199154.unk15b08.vy - 3072) * 1024 >> 12;
        obj->pos.vz += rsin(DAT_80199154.unk15b08.vy - 3072) * 1024 >> 12;
        obj->pos.vy -= 1024;
    }
    def = &DAT_8018fb3c.defs[obj->unk6];
    spin = obj->unke;
    if (obj->arg.b.unk38 != 0xff) {
        return;
    }
    if (def->unk1 == 0x20) {
        notify_enqueue(25, obj->arg.h.unk3a);
        obj->unk6 = 0xff;
        DAT_801b24e4.unk50 += obj->arg.h.unk3a;
        return;
    }
    if (obj->unk4 == 0x70 && obj->unk6 == 0x86) {
        driver = &DAT_8018fb3c.objects[obj->arg.h.unk40];
        link = &DAT_8018fb3c.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->unk4 == 2 && link->unk8 != 0) {
            return;
        }
    } else if (obj->unk4 == 0x70 && obj->unk6 == 0x8d) {
        driver = &DAT_8018fb3c.objects[obj->arg.h.unk40];
        link = &DAT_8018fb3c.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->unk4 == 2 && link->unk8 != 0) {
            return;
        }
    } else if (obj->unk4 == 0x70 && obj->unk6 == 0x88) {
        driver = &DAT_8018fb3c.objects[obj->arg.h.unk40];
        link = &DAT_8018fb3c.objects[driver->arg.b.unk3a.bytes[1]];
        if (link->unk4 == 2 && link->unk8 != 0) {
            return;
        }
    } else if (obj->unk6 == 0x85 && DAT_8018fad4.area[0] == 7) {
        if (!angle_within_tolerance(DAT_801b24e4.unk12c.vy + 2048, obj->angle.vy, 256)) {
            return;
        }
    }
    pitch = DAT_801b24e4.unk124.vx & 0xfff;
    start = pitch;
    if (pitch < 2048) {
        if (pitch > 0) {
            pitch = 0;
        }
    } else if (pitch < 4095) {
        pitch = 4095;
    }
    prev = PadRead(1);
    held = def->arg.sh.unk10;
    angle_from = obj->angle;
    from = obj->pos;
    func_8005be28(0, 500, 1500, pitch, DAT_801b24e4.unk124.vy, def->arg.sh.unkc, def->arg.sh.unke, &to);
    angle_to.vz = 0;
    angle_to.vx = 0;
    angle_to.vy = (DAT_801b24e4.unk124.vy + 2048) & 0xfff;
    model_id = map_object_asset_index(obj->unk6);
    resource_registry_get(model_id, &model);
    while (model == NULL) {
        cd_request_service_vab();
        cd_request_service_stream();
        render_game_frame(NULL, &DAT_801b24e4.unk124);
        resource_registry_get(model_id, &model);
    }
    for (t = 0; t <= 4096; t += 512) {
        pad = PadRead(1);
        if (prev == 0 && pad != 0) {
            goto pressed;
        }
        prev = pad;
        func_8005bee0(obj, &from, &to, &angle_from, &angle_to, t);
        obj->unke = value_approach(spin, held, t >> 4);
        DAT_801b24e4.unk124.vx = angle_lerp_shortest_q12((s16)start, (s16)pitch, t);
        cd_request_service_vab();
        cd_request_service_stream();
        render_game_frame(NULL, &DAT_801b24e4.unk124);
    }
    obj->unke = held;
    for (;;) {
        pad = PadRead(1);
        if (prev == 0 && pad != 0) {
            break;
        }
        prev = pad;
        obj->angle.vy += 64;
        cd_request_service_vab();
        cd_request_service_stream();
        render_game_frame(NULL, &DAT_801b24e4.unk124);
    }
pressed:
    if ((pad & DAT_80081868[7]) || id != -1) {
        if (func_8005d898(obj->unk6) == 0) {
            switch (obj->unk6) {
            case 0x93:
                n = (rand() * 6 >> 15) + 4;
                if (DAT_800c85e8[0][0x93] < 99 - n) {
                    DAT_800c85e8[0][0x93] += n;
                } else {
                    DAT_800c85e8[0][0x93] = 99;
                }
                break;
            case 0x94:
                DAT_800c85e8[0][0x94] = DAT_800c85e8[0][0x94] < 95 ? DAT_800c85e8[0][0x94] + 4 : 99;
                break;
            }
            pad = 1;
            func_8005be28(-500, 500, 0, pitch, DAT_801b24e4.unk124.vy, 0, 0, &from);
            angle_from = obj->angle;
            spin = 0;
            goto back;
        }
        if (id != -1) {
            obj->unk6 = 0xff;
            player_clear_motion();
            return;
        }
    }
    pad = 0;
    for (;;) {
        if (angle_within_tolerance(obj->angle.vy, (DAT_801b24e4.unk124.vy + 2048) & 0xfff, 128)) {
            obj->angle.vy = (DAT_801b24e4.unk124.vy + 2048) & 0xfff;
            break;
        }
        obj->angle.vy = (obj->angle.vy + 256) & 0xfff;
        render_game_frame(NULL, &DAT_801b24e4.unk124);
    }
back:
    angle_to = obj->angle;
    pitch = DAT_801b24e4.unk124.vx;
    to = obj->pos;
    for (t = 4096; t >= 0; t -= 512) {
        func_8005bee0(obj, &from, &to, &angle_from, &angle_to, t);
        DAT_801b24e4.unk124.vx = angle_lerp_shortest_q12((s16)start, (s16)pitch, t);
        obj->unke = value_approach(spin, held, t >> 4);
        render_game_frame(NULL, &DAT_801b24e4.unk124);
    }
    if (pad) {
        obj->unk6 = 0xff;
    }
    player_clear_motion();
}

/*
 * The action button at pos facing angle: the Object88 within reach in front
 * gets func_8005c308, then each Object44 record map_object_find_interaction_target finds in front
 * acts by the type of its def (doors and locks show why they stay shut or
 * open, items are picked up, the healing and save points fade the screen),
 * and the loaded FDAT module's handler 0 gets the point last. The full
 * restore (type 0x12) clamps both values like the heal (type 0x16) does.
 */
ADDRESS(0x8005e2d0, 0x794)
void func_8005e2d0(VECTOR *pos, SVECTOR *angle)
{
    VECTOR p;
    Object88 *o88;
    Object44 *obj;
    Object44Def *def;
    Object44 *link;
    s32 i;

    p.vx = pos->vx;
    p.vy = pos->vy + 500;
    p.vz = pos->vz;
    DAT_801ba984.unk0 = 0;
    i = func_8004d644(p.vx, p.vy, p.vz, 1400, 3200);
    if (i != -1) {
        o88 = &DAT_80185da8.objects[i];
        if (angle_within_tolerance(angle->vy, vector_xz_to_angle(o88->pos.vx - p.vx, o88->pos.vz - p.vz),
                                   300)) {
            func_8005c308(o88);
        }
    }
    for (i = 0; (i = map_object_find_interaction_target(i, &p, 800, 2500, angle->vy, 512)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        switch (def->unk0) {
        case 0xa5:
        case 0xff:
            if (obj->unk6 == 0xf8) {
                notify_enqueue(0x1d);
            } else if (obj->unk6 == 0xf9) {
                notify_enqueue(0x1c);
            }
            break;
        case 0x40:
            func_8005db30(obj);
            if (obj->unk6 == 0xff) {
                goto done;
            }
            break;
        case 0x53:
            if (obj->arg.b.unk3d[1] == 0) {
                break;
            }
            if (obj->unk8 != 0) {
                break;
            }
            if (obj->arg.b.unk3d[0] == 0xff) {
                obj->unk8 = 1;
            }
            break;
        case 0x57:
            if (obj->unk8 == 0) {
                obj->unk8 = 1;
            }
            break;
        case 3:
        case 4:
        case 5:
            if (obj->unk8 != 0) {
                break;
            }
            if (obj->arg.b.unk38 == 0xff) {
                obj->unk8 = 1;
            } else {
                notify_enqueue(6);
            }
            break;
        case 0:
        case 1:
        case 2:
        case 0x1b:
            if (obj->unk8 != 0) {
                break;
            }
            switch (obj->arg.b.unk38) {
            case 0x98:
            case 0xf1:
                notify_enqueue(5);
                break;
            case 0x96:
            case 0xf0:
                notify_enqueue(0x17);
                break;
            case 0x9a:
            case 0xf2:
                notify_enqueue(0xf);
                break;
            case 0xfd:
            case 0xfe:
                if (obj->unk6 != 0xae) {
                    obj->unk8 = 1;
                    break;
                }
                notify_enqueue(6);
                break;
            case 0xff:
                obj->unk8 = 1;
                break;
            default:
                notify_enqueue(6);
                break;
            }
            break;
        case 0x51:
            if (obj->unk8 != 0) {
                break;
            }
            if (obj->arg.b.unk38 == 0xff) {
                obj->unk8 = 1;
            }
            break;
        case 0x52:
            if (obj->unk8 != 0) {
                break;
            }
            if (obj->arg.b.unk38 == 0xff) {
                obj->unk8 = 1;
            }
            break;
        case 6:
        case 8:
            if (!angle_within_tolerance(angle->vy, obj->angle.vy + 2048, 0x155)) {
                break;
            }
            /* fall through */
        case 7:
        case 0x17:
            switch (obj->arg.b.unk38) {
            case 0xf1:
                notify_enqueue(5);
                break;
            case 0xf0:
                notify_enqueue(0x17);
                break;
            case 0xfe:
                if (obj->arg.h.unk3a == 0xffff) {
                missing:
                    notify_enqueue(0x1a);
                } else {
                    link = &DAT_8018fb3c.objects[obj->arg.h.unk3a];
                    if (link->unk6 == 0xff) {
                        goto missing;
                    }
                }
                break;
            case 0xff:
                obj->arg.b.unk38 = 0xfe;
                break;
            default:
                notify_enqueue(6);
                break;
            }
            break;
        case 0xd:
        case 0x14:
            if (obj->arg.b.unk3d[1] == 0) {
                break;
            }
            if (obj->arg.b.unk3d[0] == 0 && !angle_within_tolerance(angle->vy, obj->angle.vy + 2048, 0x155)) {
                break;
            }
            menu_show_transition_image(6, obj->arg.h.unk38 + 150);
            break;
        case 0x12:
            if (!angle_within_tolerance(angle->vy, vector_xz_to_angle(obj->pos.vx - p.vx, obj->pos.vz - p.vz),
                                        600)) {
                break;
            }
            if (!angle_within_tolerance(angle->vy + 2048, obj->angle.vy + 1024, 1024)) {
                break;
            }
            func_8005d948(512, 0, 0, 0, 128, 200, 255);
            DAT_801b24e4.hp = DAT_801b24e4.hp_max;
            if (DAT_801b24e4.hp_max < DAT_801b24e4.hp) {
                DAT_801b24e4.hp = DAT_801b24e4.hp_max;
            }
            DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
            if (DAT_801b24e4.unk1a < DAT_801b24e4.unk1c) {
                DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
            }
            player_cap_status_components(7);
            func_8005d948(512, 128, 200, 255, 0, 0, 0);
            player_clear_motion();
            break;
        case 0x16:
            if (!angle_within_tolerance(angle->vy, vector_xz_to_angle(obj->pos.vx - p.vx, obj->pos.vz - p.vz),
                                        600)) {
                break;
            }
            if (!angle_within_tolerance(angle->vy + 2048, obj->angle.vy + 1024, 1024)) {
                break;
            }
            func_8005d948(512, 0, 0, 0, 128, 128, 200);
            DAT_801b24e4.hp = DAT_801b24e4.hp_max;
            if (DAT_801b24e4.hp_max < DAT_801b24e4.hp) {
                DAT_801b24e4.hp = DAT_801b24e4.hp_max;
            }
            func_8005d948(512, 128, 128, 200, 0, 0, 0);
            player_clear_motion();
            break;
        case 0xe:
            if ((s16)DAT_8018fad4.loading != 1) {
                cd_request_wait_idle();
                map_reveal_visited_rects();
                repeat_store_word((u32 *)DAT_801b594c, 0, 200);
                func_8005efd4(DAT_8018fad4.area[0]);
                player_render_frame_and_release_pool();
                menu_card_save_browser();
            }
            break;
        }
    }
done:
    ((void (*)(VECTOR *, SVECTOR *))DAT_8018fad4.handlers[0])(&p, angle);
}
