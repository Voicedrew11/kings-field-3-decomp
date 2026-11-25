/*
 * GAME.EXE: the player's frame and map cell selection (same WIP module as
 * src/game/player.c, after src/game/player_action.c; the collision TU starts
 * at func_8003260c).
 */
/* func_80030fcc polls DAT_8018fad4.step, which the CD callbacks set
 * asynchronously (game/data.h SCENE_STEP_SHARED). */
#define GAME_FLOOR_SELECT_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/audio.h>
#include <psyq/libc.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/render.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/entity.h>
#include <game/misc.h>
#include <game/memory.h>
#include <game/object44.h>
#include <kf/lib/null.h>

RODATA(0x80011ac0, 0x4c)

/* The map cell outside the 0x28000 square map. */
DATA(0x8008190c, 0xa, ".data")
static MapCell DAT_8008190c = { { { 0xff, 0x00, 0x00, 0x3f, 0x00 }, { 0xff, 0x00, 0x00, 0x3f, 0x00 } } };

/*
 * The player's frame: marks the map cell visited, reads the pad, steps the
 * walk speeds (unk180/unk184), runs the unk101 mode, then the per-frame
 * timers, the view angle sums, the Object88 hit test, func_8002d2a0 and the
 * effects of the weapon and the Record20 slots.
 */
ADDRESS(0x80030fcc, 0x1524)
void func_80030fcc(void)
{
    MovieRec *rec;
    MovieRec *movie;
    s32 t;
    s32 u;
    s32 v;
    s32 hit;
    s32 i;
    s32 cz;
    u8 c;
    s32 part;
    Object4c *o4c;

    DAT_80185da8.unk9d00 = 4;
    func_80033c4c(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vz, 800, -1);
    if ((s16)DAT_8018fad4.loading == 0
        || ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.step != 0 && DAT_8018fad4.step != 1
            && DAT_8018fad4.step != 2 && DAT_8018fad4.step != 3 && DAT_8018fad4.step != 0xf0)) {
        cz = DAT_801b24e4.pos.vz >> 11;
        DAT_801b594c[DAT_801b24e4.pos.vx >> 11][cz >> 3] |= 1 << (7 - cz % 8);
    }
    func_80030e14();
    DAT_801b24e4.unk178 = PadRead(1);
    if (DAT_801b24e4.unk178 & DAT_80081868[12]) {
        cd_report_error(3);
    }
    if (DAT_801b24e4.unk178 & DAT_80081868[13]) {
        menu_run_pause_screen();
    }
    DAT_801b24e4.unk180 = 200;
    DAT_801b24e4.unk184 = 32;
    if (!(DAT_801b24e4.unk178 & (DAT_80081868[0] | DAT_80081868[1]))) {
        DAT_801b24e4.unk184 = 40;
    }
    if (DAT_801b24e4.unk82 != 0) {
        if (DAT_801b24e4.unk82 < 64) {
            DAT_801b24e4.unk12 += 100;
            if (DAT_801b24e4.unk12 > 0) {
                DAT_801b24e4.unk12 = 0;
            }
        } else {
            DAT_801b24e4.unk12 -= 100;
            if (DAT_801b24e4.unk12 < -3300) {
                DAT_801b24e4.unk12 = -3300;
            }
        }
        DAT_801b24e4.unk184 >>= 1;
        DAT_801b24e4.unk82--;
    } else if (DAT_801b24e4.unk11 == 0) {
        DAT_801b24e4.unk12 += 800;
        if (DAT_801b24e4.unk12 > 2800) {
            DAT_801b24e4.unk12 = 2800;
        }
    } else {
        DAT_801b24e4.unk12 -= 800;
        if (DAT_801b24e4.unk12 < 0) {
            DAT_801b24e4.unk12 = 0;
        }
    }
    DAT_801b24e4.unk180 += DAT_801b24e4.unk12 * DAT_801b24e4.unk180 >> 12;
    DAT_801b24e4.unk184 += DAT_801b24e4.unk12 * DAT_801b24e4.unk184 >> 13;
    if (DAT_801b24e4.unk84 != 0) {
        DAT_801b24e4.unk180 = 0;
        DAT_801b24e4.unk184 = 0;
        DAT_801b24e4.unk84--;
        DAT_801b24e4.unk162 >>= 1;
        DAT_801b24e4.unk164 >>= 1;
        if ((DAT_801b24e4.unk84 & 3) == 3) {
            func_8003d8ec(0x3c, 0x1e, 0, 0xc00, 1);
        }
    }
    if (DAT_801b24e4.unkd0 != 0) {
        DAT_801b24e4.unk180 >>= 1;
        DAT_801b24e4.unk184 >>= 1;
    }
    switch (DAT_801b24e4.unk101) {
    case 0:
    case 4:
        func_8002fe1c();
        func_8002f5c0();
        func_8002f9bc();
        func_8002ed60();
        func_800305d8();
        break;
    case 8:
        movie = &DAT_801e7edc.movies[DAT_801b24e4.unk188.turn.id];
        func_80061940();
        if (movie->unk22 == 0x82) {
            func_80046c00(0x82, 0x1000, 0x1000, 0);
            cd_request_wait_idle();
            func_80046c00(movie->unk22, 0x1000, 0, -256);
            func_8003d624(0xff, 0, 0, 0);
        } else if (movie->unk22 == 1) {
            MoveImage(&DAT_80199154.disp[DAT_80199154.frame].disp, DAT_80199154.draw[DAT_80199154.frame].clip.x,
                      DAT_80199154.draw[DAT_80199154.frame].clip.y);
            DrawSync(0);
            func_80046c00(1, 0x1000, 0x1000, 0);
            cd_request_wait_idle();
            func_80046c00(movie->unk22, 0x1000, 0, -256);
        } else {
            MoveImage(&DAT_80199154.disp[DAT_80199154.frame].disp, DAT_80199154.draw[DAT_80199154.frame].clip.x,
                      DAT_80199154.draw[DAT_80199154.frame].clip.y);
            DrawSync(0);
            func_80046c00(1, 0, 0, 0);
            cd_request_wait_idle();
        }
        break;
    case 7:
        rec = &DAT_801e7edc.movies[DAT_801b24e4.unk188.turn.id];
        t = DAT_801b24e4.unk188.turn.unk1 << 7;
        if (rec->unkc != -1) {
            DAT_801b24e4.pos.vx = fixed_lerp_q12(DAT_801b24e4.pos.vx, rec->unkc, t + 1);
            DAT_801b24e4.pos.vz = fixed_lerp_q12(DAT_801b24e4.pos.vz, rec->unk14, t + 1);
        }
        if (rec->unk10 != -1) {
            DAT_801b24e4.pos.vy = fixed_lerp_q12(DAT_801b24e4.pos.vy, rec->unk10, t + 1);
        }
        if (rec->unk4 != -1) {
            DAT_801b24e4.unk13c = angle_lerp_shortest_q12(0, rec->unk4, t + 1);
            DAT_801b24e4.unk12c.vx = angle_lerp_shortest_q12(DAT_801b24e4.unk188.turn.angle.vx, 0, t + 1);
        }
        if (rec->unk6 != -1) {
            DAT_801b24e4.unk13e = 0;
            DAT_801b24e4.unk12c.vy = angle_lerp_shortest_q12(DAT_801b24e4.unk188.turn.angle.vy, rec->unk6,
                                                             t + 1);
        }
        if (rec->unk8 != -1) {
            DAT_801b24e4.unk140 = angle_lerp_shortest_q12(0, rec->unk8, t + 1);
            DAT_801b24e4.unk12c.vz = angle_lerp_shortest_q12(DAT_801b24e4.unk188.turn.angle.vz, 0, t + 1);
        }
        if (0x7f - DAT_801b24e4.unk188.turn.unk1 * 4 < 0) {
            SsSetMVol(0, 0);
        } else {
            SsSetMVol(0x7f - DAT_801b24e4.unk188.turn.unk1 * 4, 0x7f - DAT_801b24e4.unk188.turn.unk1 * 4);
        }
        if (DAT_801b24e4.unk188.turn.unk1++ >= 32) {
            if (DAT_801b24e4.bgm_volume != 0 && DAT_801b09fc.seq_playing == 1) {
                SsSeqPause(DAT_801b09fc.seq_id);
            }
            func_800307f4(DAT_801b24e4.unk188.turn.id);
            if (rec->unk4 != -1) {
                DAT_801b24e4.unk12c.vx = rec->unk4;
                DAT_801b24e4.unk13c = 0;
            }
            if (rec->unk6 != -1) {
                DAT_801b24e4.unk12c.vy = rec->unk6;
                DAT_801b24e4.unk13e = 0;
            }
            if (rec->unk8 != -1) {
                DAT_801b24e4.unk12c.vz = rec->unk8;
                DAT_801b24e4.unk140 = 0;
            }
            DAT_801e7edc.unk372 = 1;
            if (rec->unka != 0) {
                func_80046c00(rec->unk22, 0, 0x1000, 0x100);
            }
        }
        break;
    case 9:
        rec = &DAT_801e7edc.movies[DAT_801b24e4.unk188.turn.id];
        u = DAT_801b24e4.unk188.turn.unk1 << 7;
        if (rec->unk24 != -1) {
            DAT_801b24e4.pos.vx = fixed_lerp_q12(DAT_801b24e4.pos.vx, rec->unk24, u);
            DAT_801b24e4.pos.vz = fixed_lerp_q12(DAT_801b24e4.pos.vz, rec->unk2c, u);
        }
        if (rec->unk28 != -1) {
            DAT_801b24e4.pos.vy = fixed_lerp_q12(DAT_801b24e4.pos.vy, rec->unk28, u);
        }
        if (rec->unk1c != -1) {
            DAT_801b24e4.unk13c = angle_lerp_shortest_q12(0, rec->unk1c, u);
            DAT_801b24e4.unk12c.vx = angle_lerp_shortest_q12(DAT_801b24e4.unk188.turn.angle.vx, 0, u);
        }
        if (rec->unk1e != -1) {
            DAT_801b24e4.unk13e = 0;
            DAT_801b24e4.unk12c.vy = angle_lerp_shortest_q12(DAT_801b24e4.unk188.turn.angle.vy, rec->unk1e,
                                                             u);
        }
        if (rec->unk20 != -1) {
            DAT_801b24e4.unk140 = angle_lerp_shortest_q12(0, rec->unk20, u);
            DAT_801b24e4.unk12c.vz = angle_lerp_shortest_q12(DAT_801b24e4.unk188.turn.angle.vz, 0, u);
        }
        if (DAT_801b24e4.unk188.turn.unk1 == 0 && DAT_801b24e4.bgm_volume != 0
            && DAT_801b09fc.seq_playing == 1) {
            SsSeqReplay(DAT_801b09fc.seq_id);
        }
        if (DAT_801b24e4.unk188.turn.unk1 * 4 < 0x7f) {
            SsSetMVol(DAT_801b24e4.unk188.turn.unk1 * 4, DAT_801b24e4.unk188.turn.unk1 * 4);
        } else {
            SsSetMVol(0x7f, 0x7f);
        }
        if (DAT_801b24e4.unk188.turn.unk1++ >= 32) {
            func_8003078c();
            if (rec->unk1c != -1) {
                DAT_801b24e4.unk12c.vx = rec->unk1c;
                DAT_801b24e4.unk13c = 0;
            }
            if (rec->unk1e != -1) {
                DAT_801b24e4.unk12c.vy = rec->unk1e;
                DAT_801b24e4.unk13e = 0;
            }
            if (rec->unk20 != -1) {
                DAT_801b24e4.unk12c.vz = rec->unk20;
                DAT_801b24e4.unk140 = 0;
            }
            DAT_801e7edc.unk372 = 1;
        }
        break;
    case 3:
        if (func_8002f320()) {
            func_8003078c();
        }
        func_800305d8();
        break;
    case 5:
        func_8002fe1c();
        func_8002f5c0();
        func_8002f9bc();
        DAT_801b24e4.unk16c = 0;
        func_8002ed60();
        DAT_801b24e4.unk16c += (rcos(DAT_801b24e4.unk188.count) >> 4) - 256;
        DAT_801b24e4.unk140 = rsin(DAT_801b24e4.unk188.count) >> 6;
        DAT_801b24e4.unk188.count = (DAT_801b24e4.unk188.count + 128) & 0xfff;
        if (DAT_801b24e4.unk188.count == 0) {
            DAT_801b24e4.unk140 = 0;
            func_8003078c();
        }
        func_800305d8();
        break;
    case 0x10:
        func_8002fe1c();
        func_8002f5c0();
        func_8002f9bc();
        DAT_801b24e4.unk188.knock.dir.vy = 1;
        func_8002f320();
        func_8003047c();
        /* unk134 and unk136 are tested as one word (lw). */
        if (*(s32 *)&DAT_801b24e4.unk134 == 0 && DAT_801b24e4.unk138 == 0) {
            func_8003078c();
        }
        func_8002ed60();
        func_800305d8();
        break;
    case 0x12:
        func_8003047c();
        if (*(s32 *)&DAT_801b24e4.unk134 == 0 && DAT_801b24e4.unk138 == 0) {
            func_8003078c();
        }
        func_800305d8();
        break;
    case 0x11:
        DAT_801b24e4.hp = 0;
        DAT_801b24e4.unk188.knock.push.vx =
            angle_velocity_step(-1024, DAT_801b24e4.unk134, DAT_801b24e4.unk188.knock.push.vx, 8, 4);
        DAT_801b24e4.unk134 += DAT_801b24e4.unk188.knock.push.vx * 3 >> 1;
        if (DAT_801b24e4.unk16c < 1500) {
            DAT_801b24e4.unk16c += 500;
        } else {
            DAT_801b24e4.unk16c = 1500;
        }
        func_8002f320();
        if (++DAT_801b24e4.unk13a >= 32) {
            if (DAT_801b24e4.unk13a < 65) {
                c = fixed_lerp_q12(0, 0xff, (DAT_801b24e4.unk13a - 32) << 7);
                func_8003d624(0x82, c, c, c);
            } else {
                o4c = DAT_801b77ec.objects;
                for (i = 128; i != 0; i--) {
                    o4c->unk0 = 0xff;
                    o4c++;
                }
                part = 0xff;
                if (func_8005d7bc(0x6b)) {
                    func_8004b794();
                    func_8005d7f8(0x6b);
                    DAT_801b24e4.unk12c.vy = 0x2d5;
                    DAT_801b24e4.pos.vy = -14976;
                    DAT_801b24e4.pos.vz = 0xc000;
                    DAT_801b24e4.unk12c.vx = 0;
                    DAT_801b24e4.unk12c.vz = 0;
                    DAT_801b24e4.pos.vx = 0x1c800;
                    DAT_801b24e4.unk160 = 5;
                    map_reveal_visited_rects();
                    repeat_store_word((u32 *)DAT_801b594c, 0, 200);
                    func_8005efd4(DAT_8018fad4.area[0]);
                    func_8002b3a8();
                    func_80029500();
                } else {
                    func_8002b468();
                    func_80029500();
                    func_80061918();
                    func_8005ea64();
                    DAT_801ba984.unkd = 1;
                    func_80060800();
                    repeat_store_word((u32 *)DAT_801b594c, 0, 200);
                }
                func_80029188(0, 0, 0, 0, 0, part);
            }
        }
        break;
    }
    DAT_801b24e4.unk17a = DAT_801b24e4.unk178;
    if (DAT_801b24e4.unk78 != 0) {
        if (DAT_801b24e4.unk78 % 30 == 0) {
            DAT_801b24e4.unk174 = 2400;
            DAT_801b24e4.unk176 = 60;
            func_8002a6a0(-1);
        }
        DAT_801b24e4.unk78--;
    }
    if (DAT_801b24e4.unk86 != 0) {
        if (--DAT_801b24e4.unk86 == 0) {
            func_80029500();
        }
    }
    if (DAT_801b24e4.unk88 != 0) {
        if (--DAT_801b24e4.unk88 == 0) {
            func_80029500();
        }
    }
    if (DAT_801b24e4.unk8a != 0) {
        if (--DAT_801b24e4.unk8a == 0) {
            func_80029500();
        }
    }
    if (DAT_801b24e4.unk92 != 0) {
        DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
        if (--DAT_801b24e4.unk92 == 0) {
            func_80041eec(0x1b);
        }
    }
    if (DAT_801b24e4.unk94 != 0) {
        if (--DAT_801b24e4.unk94 == 0) {
            func_80029500();
            func_80041eec(0x1b);
        }
    }
    DAT_801b24e4.unk124.vx = DAT_801b24e4.unk12c.vx + DAT_801b24e4.unk134 + DAT_801b24e4.unk13c + DAT_801b24e4.unk144;
    DAT_801b24e4.unk124.vy = DAT_801b24e4.unk12c.vy + DAT_801b24e4.unk136 + DAT_801b24e4.unk13e + DAT_801b24e4.unk146;
    DAT_801b24e4.unk124.vz = DAT_801b24e4.unk12c.vz + DAT_801b24e4.unk138 + DAT_801b24e4.unk140 + DAT_801b24e4.unk148;
    func_80033c4c(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vz, 800, 1);
    DAT_80185da8.unk9d00 = 0;
    hit = func_8004d644(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vy, DAT_801b24e4.pos.vz, 1, 0x6a4);
    if (hit != -1 && (DAT_80185da8.objects[hit].unk28 & 8)) {
        func_80030af8();
    }
    func_8002d2a0();
    if (DAT_801b24e4.unkcb != 0xff) {
        if (DAT_801b24e4.unkb0->unk16 != 0 && DAT_801b24e4.unk9c % DAT_801b24e4.unkb0->unk16 == 0) {
            func_80030be0(1);
        }
        if (DAT_801b24e4.unkb0->unk18 != 0 && DAT_801b24e4.unk9c % DAT_801b24e4.unkb0->unk18 == 0) {
            func_80030c6c(1);
        }
        if (DAT_801b24e4.unkf2 == 0x35 && DAT_80199154.unk15996 != 0 && DAT_801b24e4.unk9c % 900 == 0) {
            func_80030c6c(1);
        }
    }
    if (DAT_801b24e4.unkf0 != 0xff) {
        func_80030cbc(DAT_801b24e4.unkd4);
        func_80030d68(DAT_801b24e4.unkd4);
    }
    if (DAT_801b24e4.unkf1 != 0xff) {
        func_80030cbc(DAT_801b24e4.unkd8);
        func_80030d68(DAT_801b24e4.unkd8);
    }
    if (DAT_801b24e4.unkf2 != 0xff) {
        func_80030cbc(DAT_801b24e4.unkdc);
        func_80030d68(DAT_801b24e4.unkdc);
    }
    if (DAT_801b24e4.unkf3 != 0xff) {
        func_80030cbc(DAT_801b24e4.unke0);
        func_80030d68(DAT_801b24e4.unke0);
    }
    if (DAT_801b24e4.unkf4 != 0xff) {
        func_80030cbc(DAT_801b24e4.unke4);
        func_80030d68(DAT_801b24e4.unke4);
    }
    if (DAT_801b24e4.unkf5 != 0xff) {
        func_80030cbc(DAT_801b24e4.unke8);
        func_80030d68(DAT_801b24e4.unke8);
    }
    if (DAT_801b24e4.unkf6 != 0xff) {
        func_80030cbc(DAT_801b24e4.unkec);
        func_80030d68(DAT_801b24e4.unkec);
    }
    if (DAT_801b24e4.unkcb == 0x16 && DAT_801b24e4.unk9c % 150 == 0 && (rand() >> 12) == 0) {
        DAT_801b24e4.unk84 = 200;
    }
    if (DAT_801b24e4.unkf2 == 0x3a) {
        func_8003456c(10, 10, 10, 0xd05, 6000, 0x800);
    }
    if ((DAT_801b24e4.unkf5 == 0x52 || DAT_801b24e4.unkf6 == 0x52) && (rand() >> 7) == 0) {
        DAT_801b24e4.unk78 = 900;
    }
    DAT_801b24e4.unk9c++;
}

/*
 * Selects the map cell under x/z (2048 units a cell) and the layer of it
 * (0 or 5 bytes in) whose height suits y; returns that floor height.
 */
ADDRESS(0x800324f0, 0x11c)
s32 func_800324f0(s32 x, s32 y, s32 z)
{
    MapCell *cell;
    u16 h;

    if (x >= 0 && x <= 0x27fff && z >= 0 && z <= 0x27fff) {
        cell = &DAT_801d4464.cells[z >> 11][x >> 11];
        h = -(y >> 7);
        if (cell->layer[0].unk1 > cell->layer[1].unk1) {
            if (h >= cell->layer[0].unk1 || cell->layer[1].unk1 == 0) {
            lower:
                DAT_801d4464.unk1200a = 0;
                DAT_801d4464.unk1200c = -cell->layer[0].unk1 << 7;
            } else {
            upper:
                DAT_801d4464.unk1200a = 5;
                DAT_801d4464.unk1200c = -cell->layer[1].unk1 << 7;
            }
        } else {
            if (h < cell->layer[1].unk1 && cell->layer[0].unk1 != 0) {
                goto lower;
            }
            goto upper;
        }
    } else {
        cell = &DAT_8008190c;
        DAT_801d4464.unk1200a = 0;
        DAT_801d4464.unk1200c = 0;
    }
    DAT_801d4464.unk12000 = cell;
    return DAT_801d4464.unk1200c;
}
