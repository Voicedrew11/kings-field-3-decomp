/*
 * GAME.EXE: an Object88's talk script (func_8005c308), an Object44 spawn
 * helper, the player's move to one of 14 start positions (areas loaded
 * through resource_request_transition) and the item use dispatcher (func_8005cbe0) (WIP
 * module). The .rdata is func_8005c864's local table initializer, then the
 * jump tables of func_8005c308 and func_8005cbe0: GCC 2.6 emits the
 * initializers of a TU ahead of its jump tables, so the three share a TU.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/funcs.h>
#include <game/cd.h>
#include <game/data.h>
#include <game/render.h>
#include <game/entity.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/misc.h>
#include <game/object44.h>
#include <game/scene.h>
#include <game/image.h>
#include <game/sound.h>

RODATA(0x800130f0, 0x1a4)

/* Runs obj's talk script record (id 0x70) from its entry point up to the 0xff end, then its end action. */
ADDRESS(0x8005c308, 0x4e0)
void func_8005c308(Object88 *obj)
{
    Object78 *def;
    u8 *script;
    u8 *p;
    s32 repeat;
    s32 posed;
    u8 anim;
    s32 n;
    u8 back;

    def = &DAT_80185da8.defs[obj->unk2];
    script = def->scripts[0];
    if (script == NULL || script[0] != 0x70) {
        return;
    }
    posed = 0;
    repeat = 0;
    if (script[16] == 0) {
        DAT_801ba984.unkaa = obj->unk1;
    }
    p = func_8005c1e8(obj);
    if (DAT_801ba984.unkaa != obj->unk1 && script[19] == 1) {
        while (*p++ != 0xf0)
            ;
        p++;
        script[16] = p - (script + 20);
        script[19] = 0;
    }
    for (;;) {
        switch (*p) {
        case 0xf0:
            script[19] = 1;
        case 0xf8:
            back = p[1];
            script[16] -= back;
            p -= back;
            continue;
        case 0xf9:
            if (DAT_801ba984.unk4[p[1]] == p[2]) {
                script[16] = func_8005c18c(script, p[3]);
                p = script + (script[16] + 20);
            } else {
                script[16] += 4;
                p += 4;
            }
            continue;
        case 0xf2:
            script[16] += 2;
            p += 2;
            continue;
        case 0xf4:
            script[16]++;
            p++;
            ((void (*)(Object88 *, u8))DAT_8018fad4.handlers[4])(obj, *p);
            break;
        case 0xf5:
            script[16]++;
            p++;
            repeat = *p;
            break;
        case 0xf6:
            DAT_801ba984.unkaa = obj->unk1;
            p++;
            script[19] = 0;
            script[16]++;
            continue;
        case 0xf7:
            DAT_801ba984.unk4[p[1]] = p[2];
            p += 2;
            script[16] += 2;
            break;
        case 0xf3:
            break;
        case 0xff:
            goto end;
        default:
            if (!posed && script[1] != 0xff) {
                anim = obj->unkc;
                posed = 1;
                if (obj->unk18 != 0) {
                    func_8005c0d4(obj, obj->unkc, obj->unk18, 0, obj->unk66);
                }
                func_8005c0d4(obj, script[1], 0, 0xfff, *(u16 *)(script + 8));
            }
            if (def->unk0 != 0x2b) {
                func_800608ec(def->unk0, *p);
            }
            render_game_frame(NULL, NULL);
            menu_show_transition_image(7, *(u16 *)(script + 12) + *p);
            if ((DAT_801b24e4.unk178 & DAT_80081868[7]) && script[18] == 0xff) {
                repeat++;
            }
            break;
        }
        script[16]++;
        p++;
        if (repeat == 0) {
            break;
        }
        repeat--;
    }
end:
    switch (script[18] & 0xf0) {
    case 0x00:
        player_render_frame_and_release_pool();
        menu_item_buy_sell_controller(script[18] & 0xf);
        break;
    case 0x10:
    case 0x20:
        player_render_frame_and_release_pool();
        break;
    case 0x30:
        if (DAT_801b24e4.unk4c == 4) {
            player_render_frame_and_release_pool();
            n = menu_intellectual_choose_item();
            if (n != -1) {
                render_game_frame(NULL, NULL);
                menu_show_transition_image(6, n + 488);
            }
        }
        break;
    case 0x40:
        player_render_frame_and_release_pool();
        if (menu_prompt_stay(script[18] & 0xf) != -1) {
            DAT_801e7edc.unk370 = 1;
            DAT_801e7edc.unk384 = script[18] & 3;
        }
        break;
    }
    DAT_801ba984.unkaa = obj->unk1;
    if (posed && script[17] != 0xff) {
        func_8005c0d4(obj, script[17], 0, 0xfff, *(u16 *)(script + 14));
        obj->unkc = anim;
    }
    player_clear_motion();
    DAT_801ba984.unk0 = 1;
}

/* Spawns Object44 0x17c and links obj to it (its index - 124). */
ADDRESS(0x8005c7e8, 0x7c)
void func_8005c7e8(Object44 *obj, s32 id)
{
    Object44 *made;
    s32 i;

    made = map_object_effect_pool_acquire(0x17c, 0x10, -1);
    obj->arg.b.unk39 = made - DAT_8018fb3c.objects - 124;
    made->unk6 = id;
    made->arg.b.unk38 = 0;
}

/* Loads start position n's areas and puts the player there (12: none). */
ADDRESS(0x8005c864, 0x37c)
void func_8005c864(s32 n)
{
    Record08 table[14] = {
        {0, 0, 0, 10, 56, 100, 2048},
        {1, 4, 5, 32, 14, 100, 0},
        {2, 2, 2, 57, 36, 100, 0},
        {3, 16, 18, 7, 31, 100, 0},
        {4, 13, 15, 56, 13, 100, 2048},
        {5, 4, 5, 29, 31, 100, 0},
        {6, 17, 19, 34, 67, 100, 2048},
        {7, 7, 9, 19, 29, 104, 0},
        {255, 255, 255, 10, 56, 100, 0},
        {9, 5, 7, 39, 56, 100, 2048},
        {10, 6, 8, 37, 51, 204, 2048},
        {11, 6, 8, 58, 34, 100, 0},
        {27, 9, 11, 40, 35, 100, 0},
        {24, 9, 11, 7, 70, 200, 0},
    };
    Record08 *rec;
    s32 yaw;
    s32 z;

    rec = &table[n];
    if (n < 12 || n == 13) {
        if (rec->area == 0xff) {
            return;
        }
        render_frames_with_color_overlay(1, 0, 0x1000, 0x100);
        func_8004b794();
    }
    do {
        cd_request_yield();
        resource_advance_transition();
    } while ((s16)DAT_8018fad4.loading != 0);
    if (rec->area != DAT_8018fad4.area[0]) {
        resource_request_transition(rec->area, rec->area, rec->area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
    } else {
        resource_request_transition(0xff, 0xff, rec->area, 0xff, 0xff, 0x7f, 0x7f, 0x7f);
        DAT_801b24e4.unke = 1;
    }
    do {
        cd_request_yield();
        resource_advance_transition();
    } while ((s16)DAT_8018fad4.loading != 0);
    DAT_801b24e4.unk101 = 0;
    DAT_801b24e4.unk13c = DAT_801b24e4.unk13e = DAT_801b24e4.unk140 = 0;
    DAT_801b24e4.unk124 = DAT_801b24e4.unk12c;
    DAT_801b24e4.pos.vx = (rec->x << 11) + 1024;
    z = rec->z << 11;
    DAT_801b24e4.pos.vz = z + 1024;
    DAT_801b24e4.pos.vy = -(rec->y << 7);
    yaw = -rec->yaw;
    DAT_801b24e4.unk124.vy = DAT_801b24e4.unk12c.vy = yaw;
    if (n == 13) {
        DAT_801b24e4.unk124.vx = DAT_801b24e4.unk12c.vx = DAT_801b24e4.unk124.vz =
            DAT_801b24e4.unk12c.vz = 0;
        DAT_801b24e4.pos.vz = z + 2000;
    } else {
        render_frames_with_color_overlay(1, 0x1000, 0x1000, 0);
        cd_request_wait_idle();
        render_frames_with_color_overlay(1, 0x1000, 0, -256);
        render_set_color_overlay(0xff, 0, 0, 0);
    }
    if (n != 13) {
        resource_request_transition(0xff, 0xff, 0xff, rec->area3, rec->area4, 0x7f, 0x7f, 0x7f);
    }
    if (n < 12) {
        DAT_801b24e4.unk10a = 1;
        DAT_801ba984.unk0 = 1;
        player_recalculate_combat_stats();
    }
    DAT_801b24e4.unk134 = DAT_801b24e4.unk136 = DAT_801b24e4.unk138 = 0;
}

/* Uses item id at pos/angle (stat raises, objects near the player, start-position moves),
 * then passes it to the loaded FDAT module's handler 2; message 10 when nothing took it. */
ADDRESS(0x8005cbe0, 0xbdc)
void func_8005cbe0(VECTOR *pos, SVECTOR *angle, s32 id)
{
    VECTOR unused[2]; /* the frame keeps 32 unused bytes here */
    Object88 *o88;
    Object44 *obj;
    Object44 *obj2;
    Object88 *o88b;
    Object44 *obj2b;
    Object88 *target;
    Object44 *made;
    s32 dist;
    s32 i;
    s32 element;
    s32 flag;
    s32 n;
    s32 k;
    s32 j;

    DAT_801ba984.unk0 = 0;
    switch (id) {
    case 113:
        element = 0;
        goto raise;
    case 114:
        element = 1;
        goto raise;
    case 115:
        element = 2;
        goto raise;
    case 116:
        element = 3;
        goto raise;
    case 117:
        element = 4;
    raise:
        func_8005d7f8(id);
        for (j = 2; j != -1; j--) {
            DAT_801b24e4.unk34[element]++;
            if (DAT_801b24e4.unk34[element] >= 1000) {
                DAT_801b24e4.unk34[element] = 999;
            } else {
                player_learn_element_magic(element);
                player_recalculate_combat_stats();
                if (DAT_801ba984.unk0 == 0) {
                    DAT_801ba984.unk0 = 1;
                    notify_enqueue(element + 18);
                }
            }
        }
        break;
    case 110:
        func_8005d7f8(id);
        for (i = 4; i != -1; i--) {
            DAT_801b24e4.unk34[i]++;
            if (DAT_801b24e4.unk34[i] >= 1000) {
                DAT_801b24e4.unk34[i] = 999;
            } else {
                player_learn_element_magic(i);
                player_recalculate_combat_stats();
                if (DAT_801ba984.unk0 == 0) {
                    DAT_801ba984.unk0 = 1;
                    notify_enqueue(13);
                }
            }
        }
        break;
    case 111:
        for (j = 2; j != -1; j--) {
            DAT_801b24e4.unk32++;
            if (DAT_801b24e4.unk32 >= 1000) {
                DAT_801b24e4.unk32 = 999;
            } else {
                player_recalculate_combat_stats();
                if (DAT_801ba984.unk0 == 0) {
                    DAT_801ba984.unk0 = 1;
                    notify_enqueue(1);
                }
            }
        }
        break;
    case 112:
        DAT_801b24e4.unk4c++;
        if (DAT_801b24e4.unk4c >= 5) {
            DAT_801b24e4.unk4c = 4;
            break;
        }
        player_recalculate_combat_stats();
        if (DAT_801ba984.unk0 == 0) {
            DAT_801ba984.unk0 = 1;
            notify_enqueue(2);
        }
        break;
    case 128:
        if (DAT_8018fad4.area[0] == 6) {
            k = 0;
            for (;;) {
                k = map_object_find_interaction_target(k, pos, 800, 1700, angle->vy, 512);
                if (k == -1) {
                    break;
                }
                if (k == 0x5b) {
                    DAT_801ba984.unk0 = 1;
                    func_8005d7f8(0x80);
                    obj2 = &DAT_8018fb3c.objects[0x5b];
                    i = 16;
                    o88 = &DAT_80185da8.objects[3];
                    o88->unk9 = 0;
                    o88->unk0 = 2;
                    while (o88->unk9 == 0) {
                        func_80052e5c();
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    cd_request_wait_idle();
                    o88->unk15 = -1;
                    obj2->unk10 = 0x400;
                    obj2->unk2 = 5;
                    o88->unk16 = 0x1000;
                    o88->unk13 = 5;
                    while (--i != -1) {
                        if (o88->unk15 > 100) {
                            o88->unk15 -= 8;
                            obj2->scale.vx += 8;
                            obj2->scale.vz += 8;
                            obj2->unk10 += 256;
                            o88->unk16 -= 256;
                        } else {
                            o88->unk15 = 100;
                        }
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    o88->unk16 = 0x800;
                    o88->unk13 = 0;
                    o88->unk0 = 1;
                    obj2->unk6 = 0xff;
                    DAT_801ba984.unke[0x19] = 1;
                    break;
                }
                if (k == 0x53) {
                    DAT_801ba984.unk0 = 1;
                    func_8005d7f8(0x80);
                    o88b = &DAT_80185da8.objects[58];
                    o88b->unk9 = 0;
                    o88b->unk0 = 2;
                    func_8004c0b0(o88b, 0x15);
                    obj2b = &DAT_8018fb3c.objects[0x53];
                    i = 16;
                    while (o88b->unk9 == 0) {
                        func_80052e5c();
                        o88b->unk18 = 0;
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    cd_request_wait_idle();
                    o88b->unk15 = -1;
                    obj2b->unk10 = 0x400;
                    obj2b->unk2 = 5;
                    o88b->unk16 = 0x1000;
                    o88b->unk13 = 5;
                    while (--i != -1) {
                        if (o88b->unk15 > 0) {
                            o88b->unk15 -= 8;
                            obj2b->scale.vx += 8;
                            obj2b->scale.vz += 8;
                            obj2b->unk10 += 256;
                            o88b->unk16 -= 256;
                        } else {
                            o88b->unk15 = 0;
                        }
                        cd_request_service_vab();
                        cd_request_service_stream();
                        render_game_frame(NULL, NULL);
                    }
                    o88b->unk16 = 0x800;
                    o88b->unk13 = 0;
                    o88b->unk0 = 1;
                    obj2b->unk6 = 0xff;
                    DAT_801ba984.unk2c[0] = 1;
                    break;
                }
                k++;
            }
        }
        if (DAT_801ba984.unk0 == 0) {
            func_8005d7f8(id);
            DAT_801b24e4.unk94 = 900;
            DAT_801ba984.unk0 = 1;
            player_recalculate_combat_stats();
        }
        break;
    case 135:
    case 137:
    case 138:
    case 139:
    case 140:
    case 142:
    case 144:
        i = 0;
        for (;;) {
            i = map_object_find_interaction_target(i, pos, 800, 1700, angle->vy, 512);
            if (i == -1) {
                goto done;
            }
            switch (map_object_check_and_consume_marker(&DAT_8018fb3c.objects[i], id)) {
            case 0:
                break;
            case 1:
                func_8005bd34();
                DAT_801ba984.unk0 = 1;
                goto done;
            case 3:
                notify_enqueue(7);
                DAT_801ba984.unk0 = 1;
                goto done;
            case 4:
                notify_enqueue(7);
                DAT_801ba984.unk0 = 1;
                goto done;
            }
            i++;
        }
    case 141:
        i = map_object_find_interaction_target(0, pos, 800, 1700, angle->vy, 512);
        if (i == -1) {
            break;
        }
        made = &DAT_8018fb3c.objects[i];
        if (made->unk6 == 0x101) {
            if (made->arg.b.unk38 == 0xff) {
                made->arg.b.unk38 = id;
                made->unk8 = 0;
                DAT_801ba984.unk0 = 1;
                func_8005d7f8(id);
                func_8005c7e8(made, id);
            }
        } else if (map_object_check_and_consume_marker(made, id) == 3) {
            notify_enqueue(7);
            DAT_801ba984.unk0 = 1;
        }
        break;
    case 136:
        i = 0;
        for (;;) {
            i = map_object_find_interaction_target(i, pos, 800, 1700, angle->vy, 512);
            if (i == -1) {
                break;
            }
            obj = &DAT_8018fb3c.objects[i];
            if (obj->unk6 == 0xa2) {
                if (obj->arg.b.unk38 == 0x88) {
                    obj->arg.b.unk38 = 0xfe;
                    DAT_801ba984.unk0 = 1;
                    func_8005d7f8(id);
                }
            } else if (obj->unk6 == 0x102) {
                if (obj->arg.b.unk38 == 0xff) {
                    Object44 *made;

                    obj->arg.b.unk38 = id;
                    obj->unk8 = 0;
                    DAT_801ba984.unk0 = 1;
                    func_8005d7f8(id);
                    made = map_object_effect_pool_acquire(350, 10, DAT_8018fb3c.unk885e);
                    map_object_reset(made);
                    made->arg.h.unk3c = DAT_8018fb3c.unk885e++;
                    made->unk6 = id;
                    made->arg.b.unk38 = 0;
                    obj->arg.b.unk39 = made - DAT_8018fb3c.objects - 94;
                }
            } else if (map_object_check_and_consume_marker(obj, id) == 3) {
                notify_enqueue(7);
                DAT_801ba984.unk0 = 1;
            }
            i++;
        }
        break;
    case 118:
        i = map_object_find_interaction_target(0, pos, 800, 1700, angle->vy, 512);
        if (i == -1) {
            break;
        }
        if (DAT_8018fb3c.objects[i].unk6 == 0xfe) {
            if (func_8005d898(0x77) == 0) {
                notify_enqueue(12);
                func_8005d7f8(id);
            }
            DAT_801ba984.unk0 = 1;
        } else if (DAT_8018fb3c.objects[i].unk6 == 0xfb) {
            if (func_8005d898(0x78) == 0) {
                notify_enqueue(12);
                func_8005d7f8(id);
            }
            DAT_801ba984.unk0 = 1;
        }
        break;
    case 130:
        k = 1;
        goto start;
    case 131:
        k = 6;
        goto start;
    case 132:
        k = 9;
        goto start;
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
        k = id - 150;
        if ((u32)k >= 12) {
            break;
        }
    start:
        if (DAT_8018fad4.area[0] == 25 || DAT_8018fad4.area[0] == 27) {
            k = 0;
            break;
        }
        if ((s16)DAT_8018fad4.loading == 1
            && (DAT_8018fad4.previous[0] == 25 || DAT_8018fad4.previous[0] == 27)) {
            break;
        }
        func_8005c864(k);
        break;
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 119:
    case 120:
        DAT_801ba984.unk0 = 1;
        break;
    case 101:
        flag = 2;
        if (DAT_801b24e4.unk160 == 0) {
            flag = 1;
        }
        target = func_8004d3c4(pos, angle->vy, angle->vx, 8000, 500, 500, &dist, -1);
        if (target != NULL && target->unk3 == flag) {
            menu_show_transition_image(6, target->unk1 + 270);
            DAT_801ba984.unk0 = 1;
        } else {
            n = map_object_find_interaction_target(0, pos, 800, 1700, angle->vy, 512);
            if (n != -1 && DAT_8018fb3c.objects[n].unk0 != 0 && n == 11 && DAT_8018fad4.area[0] == 17) {
                menu_show_transition_image(6, 305);
                DAT_801ba984.unk0 = 1;
            }
        }
        if (DAT_801ba984.unk0 == 0) {
            obj = DAT_8018fb3c.objects;
            for (i = 395; i != -1; i--, obj++) {
                if (DAT_8018fb3c.defs[obj->unk6].unk0 == 0xe2 && obj->arg.b.unk40 == flag
                    && player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                                       obj->arg.b.unk39,
                                                       0x8000) != 0) {
                    menu_show_transition_image(6, obj->arg.h.unk3a + 810);
                    DAT_801ba984.unk0 = 1;
                    break;
                }
            }
        }
        break;
    }
done:
    ((void (*)(VECTOR *, SVECTOR *, s32))DAT_8018fad4.handlers[2])(pos, angle, id);
    if (DAT_801ba984.unk0 == 0) {
        notify_enqueue(10);
    }
    player_clear_motion();
}
