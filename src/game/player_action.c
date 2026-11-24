/*
 * GAME.EXE: player state, continued (same WIP module as src/game/player.c;
 * split from it at the unreconstructed func_8002c30c): the unk101 modes and
 * their parameters, hp/unk1c changes and the per-frame effect timers.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/sound.h>
#include <game/render.h>
#include <game/data.h>
#include <game/funcs.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/entity.h>
#include <game/misc.h>
#include <kf/lib/null.h>

RODATA(0x80011a78, 0x44)

/* The pad bits of the sequence func_8002fe1c follows, 0xffff ended (rebuilt every frame). */
DATA(0x8009c3cc, 0x8, ".sbss")
static u16 DAT_8009c3cc[4];

/* A fall from 20480 above the floor at speed 400 or more is death. */
ADDRESS(0x8002ecbc, 0x60)
void func_8002ecbc(void)
{
    if (DAT_801b24e4.unk172 >= 400 && DAT_801d4464.unk12010 - DAT_801b24e4.pos.vy > 20480) {
        func_80030a6c(NULL);
        DAT_801b24e4.unk109 = 1;
    }
}

/* The landing sound, louder with the fall speed. */
ADDRESS(0x8002ed1c, 0x44)
void func_8002ed1c(s32 speed)
{
    if (speed >= 320) {
        speed -= 320;
        if (speed > 896) {
            speed = 0x380;
        }
        func_8001576c(7, (speed >> 3) + 32);
    }
}

/* Vertical movement per unk104 fall state: steps up and down, falls, jumps and landings. */
ADDRESS(0x8002ed60, 0x5c0)
void func_8002ed60(void)
{
    s32 y;
    s32 hit;
    s32 d;
    s32 v;
    s32 speed;
    s32 bob;

    func_80033b10(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vy, DAT_801b24e4.pos.vz, 800, 1700);
    DAT_801b24e4.unk11c.vy = 0;
    switch (DAT_801b24e4.unk104) {
    case 0x10:
        func_8002ecbc();
        DAT_801b24e4.pos.vy += DAT_801b24e4.unk172;
        DAT_801b24e4.unk11c.vy = DAT_801b24e4.unk172;
        DAT_801b24e4.unk172 += 80;
        if (DAT_801b24e4.pos.vy > DAT_801d4464.unk12010 + 100) {
            DAT_801b24e4.pos.vy = DAT_801d4464.unk12010;
            DAT_801b24e4.unk104 = 0;
        }
        break;
    case 0x20:
        func_8002ecbc();
        DAT_801b24e4.pos.vy += DAT_801b24e4.unk172;
        if (DAT_801b24e4.pos.vy <= DAT_801d4464.unk12010 || DAT_801b24e4.unk172 >= 0) {
            if (DAT_801b24e4.pos.vy - DAT_801b24e4.unk172 > DAT_801d4464.unk12010) {
                DAT_801b24e4.pos.vy = DAT_801d4464.unk12010;
            }
            DAT_801b24e4.unk104 = 0;
        }
        DAT_801b24e4.unk11c.vy = DAT_801b24e4.unk172;
        DAT_801b24e4.unk172 += 10;
        break;
    case 0x40:
        func_8002ecbc();
        y = DAT_801b24e4.unk172 + DAT_801b24e4.pos.vy;
        hit = func_80033f38(DAT_801b24e4.pos.vx, y, DAT_801b24e4.pos.vz, 800, 1700, 0x31);
        if (hit == 0) {
            DAT_801b24e4.pos.vy = y;
            DAT_801b24e4.unk11c.vy = DAT_801b24e4.unk172;
            DAT_801b24e4.unk172 += 40;
            DAT_801b24e4.unk144 = DAT_801b24e4.unk172 >> 1;
            break;
        }
        if (DAT_801b24e4.unk172 < 0) {
            DAT_801b24e4.unk172 = 0;
            break;
        }
        func_8002ed1c(DAT_801b24e4.unk172);
        if (DAT_801b24e4.unk172 >= 480) {
            v = DAT_801b24e4.unk172 * DAT_801b24e4.unk172 >> 12;
            func_8002a6f4(NULL, v * v * v / 118000, 0);
        }
        if (hit & 4) {
            DAT_801b24e4.pos.vy = DAT_801d4464.unk12010;
            if (DAT_801b24e4.unk172 >= 512) {
                DAT_801b24e4.pos.vy = DAT_801d4464.unk12010 - (DAT_801b24e4.unk172 >> 1);
            }
        } else {
            func_80033d84();
            y = DAT_801d4464.unk1203c.vy - DAT_801d4464.unk1204e - 1;
            if (func_80033f38(DAT_801b24e4.pos.vx, y, DAT_801b24e4.pos.vz, 800, 1700, 0x31) == 0) {
                DAT_801b24e4.pos.vy = y;
            }
        }
        DAT_801b24e4.unk170 = 1;
        DAT_801b24e4.unk104 = 0x50;
        /* fall through */
    case 0x50:
        if (DAT_801b24e4.unk170 > 0) {
            DAT_801b24e4.unk170 += DAT_801b24e4.unk172 >> 2;
        }
        DAT_801b24e4.unk172 -= 300;
        if (DAT_801b24e4.unk144 > 0) {
            if (DAT_801b24e4.unk172 > 0) {
                DAT_801b24e4.unk144 += 10;
            } else {
                DAT_801b24e4.unk144 -= 30;
            }
        }
        if (DAT_801b24e4.unk170 <= 0 && DAT_801b24e4.unk144 <= 0) {
            DAT_801b24e4.unk104 = 0;
            DAT_801b24e4.unk172 = 0;
            DAT_801b24e4.unk170 = 0;
            DAT_801b24e4.unk144 = 0;
        }
        /* fall through */
    case 0:
        d = DAT_801d4464.unk12010 - DAT_801b24e4.pos.vy;
        if (d < 0) {
            if (d >= -256) {
                if (d < -128) {
                    DAT_801b24e4.pos.vy -= 128;
                } else {
                    DAT_801b24e4.pos.vy = DAT_801d4464.unk12010;
                }
            } else if (d >= -512) {
                DAT_801b24e4.pos.vy -= 256;
            } else if (d >= -1024) {
                DAT_801b24e4.pos.vy -= 512;
            } else {
                DAT_801b24e4.unk104 = 0x20;
                speed = DAT_801b24e4.unk166 > 200 ? -300 : -150;
                goto start;
            }
        } else if (d > 0) {
            if (func_80033f38(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vy + 1, DAT_801b24e4.pos.vz, 800, 1700,
                              0x31) == 0) {
                if (d <= 256) {
                    if (d > 128) {
                        DAT_801b24e4.pos.vy += 128;
                    } else {
                        DAT_801b24e4.pos.vy = DAT_801d4464.unk12010;
                    }
                } else if (d <= 512) {
                    DAT_801b24e4.pos.vy += 256;
                } else {
                    DAT_801b24e4.unk104 = d <= 1024 ? 0x10 : 0x40;
                    speed = 40;
                start:
                    DAT_801b24e4.unk172 = speed;
                    DAT_801b24e4.unk170 = 0;
                    DAT_801b24e4.unk144 = 0;
                }
            }
        }
        break;
    }
    if (DAT_801b24e4.unk104 == 0) {
        if (DAT_801b24e4.unkfc != 0) {
            DAT_801b24e4.unk16e = (DAT_801b24e4.unk16e + DAT_801b24e4.unk166) & 0xfff;
            bob = rsin(DAT_801b24e4.unk16e) >> 5;
            if (bob < 0) {
                bob = -bob;
            }
            DAT_801b24e4.unk16c = bob - (bob >> 2);
        } else {
            DAT_801b24e4.unk16c = 0;
        }
    }
    func_80028d54();
}

/* Mode 0x10 flight: moves the player along unk188.knock.dir, sliding on walls; 1 when it lands. */
ADDRESS(0x8002f320, 0x2a0)
s32 func_8002f320(void)
{
    VECTOR p;
    s32 hit;
    s32 friction;
    s32 d;

    p.vx = DAT_801b24e4.pos.vx + DAT_801b24e4.unk188.knock.dir.vx;
    p.vy = DAT_801b24e4.pos.vy + DAT_801b24e4.unk188.knock.dir.vy;
    p.vz = DAT_801b24e4.pos.vz + DAT_801b24e4.unk188.knock.dir.vz;
    if (func_80033f38(p.vx, p.vy, p.vz, 800, 1700, 0x31) == 0) {
    moved:
        if (DAT_801b24e4.unk188.knock.dir.vy >= 300 && DAT_801d4464.unk12010 - DAT_801b24e4.pos.vy > 20480) {
            func_80030a6c(NULL);
            DAT_801b24e4.unk109 = 1;
        }
        DAT_801b24e4.pos.vx = p.vx;
        DAT_801b24e4.pos.vy = p.vy;
        DAT_801b24e4.pos.vz = p.vz;
        DAT_801b24e4.unk188.knock.dir.vy += 32;
    } else {
        p.vy = DAT_801b24e4.pos.vy;
        hit = func_80033f38(p.vx, p.vy, p.vz, 800, 1700, 0x31);
        if (hit == 0) {
            DAT_801b24e4.unk188.knock.dir.vy = 1;
            hit = func_80033f38(p.vx, p.vy, p.vz, 800, 1700, 0x31);
            if (hit == 0) {
                friction = 0x20;
            slide:
                d = func_80016c08(DAT_801b24e4.unk188.knock.dir.vx, DAT_801b24e4.unk188.knock.dir.vz);
                if (d <= friction) {
                    DAT_801b24e4.unk188.knock.dir.vz = 0;
                    DAT_801b24e4.unk188.knock.dir.vx = 0;
                    goto stop;
                }
                DAT_801b24e4.unk188.knock.dir.vx = DAT_801b24e4.unk188.knock.dir.vx * (d - friction) / d;
                DAT_801b24e4.unk188.knock.dir.vz = DAT_801b24e4.unk188.knock.dir.vz * (d - friction) / d;
                goto moved;
            }
        }
        if ((hit & ~0x205) || DAT_801d4464.unk12010 + 256 < DAT_801b24e4.pos.vy) {
            return 1;
        }
        p.vy = DAT_801d4464.unk12010;
        friction = 0x38;
        goto slide;
    stop:
        return 1;
    }
    func_80028d54();
    return 0;
}

/* Turning (unk168 into the yaw unk12c.vy) and looking up and down (unk16a into the pitch). */
ADDRESS(0x8002f5c0, 0x3fc)
void func_8002f5c0(void)
{
    s16 pitch;
    s32 limit;

    if (DAT_801b24e4.unk178 & DAT_80081868[2]) {
        DAT_801b24e4.unk168 += DAT_801b24e4.unk184 >> 2;
        if (DAT_801b24e4.unk168 > DAT_801b24e4.unk184) {
            DAT_801b24e4.unk168 = DAT_801b24e4.unk184;
        }
    } else if (DAT_801b24e4.unk178 & DAT_80081868[3]) {
        DAT_801b24e4.unk168 -= DAT_801b24e4.unk184 >> 2;
        if (DAT_801b24e4.unk168 < -DAT_801b24e4.unk184) {
            DAT_801b24e4.unk168 = -DAT_801b24e4.unk184;
        }
    } else if (DAT_801b24e4.unk168 > 0) {
        DAT_801b24e4.unk168 -= DAT_801b24e4.unk184 >> 2;
        if (DAT_801b24e4.unk168 < 0) {
            DAT_801b24e4.unk168 = 0;
        }
    } else if (DAT_801b24e4.unk168 < 0) {
        DAT_801b24e4.unk168 += DAT_801b24e4.unk184 >> 2;
        if (DAT_801b24e4.unk168 > 0) {
            DAT_801b24e4.unk168 = 0;
        }
    }
    DAT_801b24e4.unk12c.vy = (DAT_801b24e4.unk12c.vy + DAT_801b24e4.unk168) & 0xfff;
    if (DAT_801b24e4.unk178 & DAT_80081868[11]) {
        if (DAT_801b24e4.unk178 & DAT_80081868[9]) {
            pitch = DAT_801b24e4.unk12c.vx;
            if (pitch > 2048) {
                limit = (0x1000 - pitch) >> 2;
                DAT_801b24e4.unk16a += 8;
                if (DAT_801b24e4.unk16a > limit) {
                    DAT_801b24e4.unk16a = limit;
                    if (limit == 0) {
                        DAT_801b24e4.unk16a = 0x1000 - DAT_801b24e4.unk12c.vx;
                    }
                }
            } else if (pitch < 2048) {
                DAT_801b24e4.unk16a -= 8;
                if (DAT_801b24e4.unk16a < -(pitch >> 2)) {
                    DAT_801b24e4.unk16a = -(pitch >> 2);
                    if (DAT_801b24e4.unk16a == 0) {
                        DAT_801b24e4.unk16a = -DAT_801b24e4.unk12c.vx;
                    }
                }
            }
        } else {
            DAT_801b24e4.unk16a += 3;
            if (DAT_801b24e4.unk16a > 32) {
                DAT_801b24e4.unk16a = 32;
            }
        }
    } else if (DAT_801b24e4.unk178 & DAT_80081868[9]) {
        DAT_801b24e4.unk16a -= 3;
        if (DAT_801b24e4.unk16a < -32) {
            DAT_801b24e4.unk16a = -32;
        }
    } else if (DAT_801b24e4.unk16a > 0) {
        DAT_801b24e4.unk16a -= 3;
        if (DAT_801b24e4.unk16a < 0) {
            DAT_801b24e4.unk16a = 0;
        }
    } else if (DAT_801b24e4.unk16a < 0) {
        DAT_801b24e4.unk16a += 3;
        if (DAT_801b24e4.unk16a > 0) {
            DAT_801b24e4.unk16a = 0;
        }
    }
    if (DAT_801b24e4.unk16a > 0) {
        DAT_801b24e4.unk12c.vx = (DAT_801b24e4.unk12c.vx + DAT_801b24e4.unk16a) & 0xfff;
        if (func_80016a78(DAT_801b24e4.unk12c.vx, 700)) {
            DAT_801b24e4.unk12c.vx = 700;
        }
    } else if (DAT_801b24e4.unk16a < 0) {
        DAT_801b24e4.unk12c.vx = (DAT_801b24e4.unk12c.vx + DAT_801b24e4.unk16a) & 0xfff;
        if (func_80016a78(0xd44, DAT_801b24e4.unk12c.vx)) {
            DAT_801b24e4.unk12c.vx = 0xd44;
        }
    }
}

/* Walking (unk164) and strafing (unk162), then the move along the yaw. */
ADDRESS(0x8002f9bc, 0x420)
void func_8002f9bc(void)
{
    s32 sx;
    s32 sz;
    s16 len;
    s16 side;
    s16 ahead;

    if (DAT_801b24e4.unk178 & DAT_80081868[0]) {
        ahead = DAT_801b24e4.unk164 + (DAT_801b24e4.unk180 >> 2);
        if (ahead > DAT_801b24e4.unk180) {
            DAT_801b24e4.unk164 = DAT_801b24e4.unk180;
        } else {
            DAT_801b24e4.unk164 = ahead;
        }
    } else if (DAT_801b24e4.unk178 & DAT_80081868[1]) {
        ahead = DAT_801b24e4.unk164 - (DAT_801b24e4.unk180 >> 2);
        if (ahead >= -DAT_801b24e4.unk180) {
            DAT_801b24e4.unk164 = ahead;
        } else {
            DAT_801b24e4.unk164 = -DAT_801b24e4.unk180;
        }
    } else if (DAT_801b24e4.unk164 > 0) {
        DAT_801b24e4.unk164 -= DAT_801b24e4.unk180 >> 3;
        if (DAT_801b24e4.unk164 < 0) {
            DAT_801b24e4.unk164 = 0;
        }
    } else if (DAT_801b24e4.unk164 < 0) {
        DAT_801b24e4.unk164 += DAT_801b24e4.unk180 >> 3;
        if (DAT_801b24e4.unk164 > 0) {
            DAT_801b24e4.unk164 = 0;
        }
    }
    if (DAT_801b24e4.unk178 & DAT_80081868[10]) {
        side = DAT_801b24e4.unk162 + (DAT_801b24e4.unk180 >> 2);
        if (side > DAT_801b24e4.unk180) {
            DAT_801b24e4.unk162 = DAT_801b24e4.unk180;
        } else {
            DAT_801b24e4.unk162 = side;
        }
    } else if (DAT_801b24e4.unk178 & DAT_80081868[8]) {
        side = DAT_801b24e4.unk162 - (DAT_801b24e4.unk180 >> 2);
        if (side >= -DAT_801b24e4.unk180) {
            DAT_801b24e4.unk162 = side;
        } else {
            DAT_801b24e4.unk162 = -DAT_801b24e4.unk180;
        }
    } else if (DAT_801b24e4.unk162 > 0) {
        DAT_801b24e4.unk162 -= DAT_801b24e4.unk180 >> 2;
        if (DAT_801b24e4.unk162 < 0) {
            DAT_801b24e4.unk162 = 0;
        }
    } else if (DAT_801b24e4.unk162 < 0) {
        DAT_801b24e4.unk162 += DAT_801b24e4.unk180 >> 2;
        if (DAT_801b24e4.unk162 > 0) {
            DAT_801b24e4.unk162 = 0;
        }
    }
    sx = DAT_801b24e4.unk162 * DAT_801b24e4.unk162;
    sz = DAT_801b24e4.unk164 * DAT_801b24e4.unk164;
    len = SquareRoot0(sx + sz);
    if (len == 0) {
        ahead = 0;
        side = 0;
    } else {
        side = sx / len;
        if (DAT_801b24e4.unk162 < 0) {
            side = -side;
        }
        ahead = sz / len;
        if (DAT_801b24e4.unk164 < 0) {
            ahead = -ahead;
        }
    }
    DAT_801b24e4.unk166 = SquareRoot0(side * side + ahead * ahead);
    func_8002e3f8((DAT_801b24e4.unk12c.vy + 1024) & 0xfff, ahead);
    func_8002e3f8(DAT_801b24e4.unk12c.vy & 0xfff, side);
}

ADDRESS(0x8002fddc, 0x40)
s32 func_8002fddc(s32 id)
{
    switch (id) {
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 119:
    case 120:
        return 1;
    }
    return 0;
}

ADDRESS(0x8002fe1c, 0x660)
void func_8002fe1c(void)
{
    s32 id;
    s32 sum;
    s32 i;
    s32 weight;
    u16 mask;
    u16 *next;
    s32 flag;

    DAT_8009c3cc[3] = 0xffff;
    DAT_8009c3cc[0] = DAT_80081868[7];
    DAT_8009c3cc[1] = DAT_80081868[4];
    DAT_8009c3cc[2] = DAT_80081868[6];
    if (DAT_801b24e4.unkce == 0 && (DAT_801b24e4.unk178 & DAT_80081868[6])
        && !(DAT_801b24e4.unk17a & DAT_80081868[6]) && DAT_801b24e4.unkc6 == 0) {
        func_8002deec(DAT_801b24e4.unkc7);
    }
    if ((DAT_801b24e4.unk178 & DAT_80081868[12]) && !(DAT_801b24e4.unk17a & DAT_80081868[12])) {
        if (DAT_801b24e4.unkc8 != 0xff) {
            func_8002deec(DAT_801b24e4.unkc8);
        }
        if (DAT_801b24e4.unkc9 != 0xff) {
            if (DAT_800c85e8[0][DAT_801b24e4.unkc9] != 0) {
                if (func_8002fddc(DAT_801b24e4.unkc9)) {
                    func_8001b254(DAT_801b24e4.unkc9);
                }
                func_8005cbe0(&DAT_801b24e4.pos, &DAT_801b24e4.unk12c, DAT_801b24e4.unkc9);
            } else {
                func_80041eec(10);
            }
        }
    }
    if (DAT_801b24e4.unk105 != 0xff && --DAT_801b24e4.unk108 == 0) {
        func_8002c30c(DAT_801b24e4.unk105);
        if (--DAT_801b24e4.unk106 == 0) {
            DAT_801b24e4.unk105 = 0xff;
        } else {
            DAT_801b24e4.unk108 = DAT_801b24e4.unk107;
        }
    }
    if ((DAT_801b24e4.unk178 & DAT_80081868[7]) && (DAT_801b24e4.unk17a & DAT_80081868[7])) {
        id = DAT_801b24e4.unkf4;
        if (id != 0x4c) {
            if (DAT_801b24e4.unk11 != 0) {
                DAT_801b24e4.unk11--;
            }
            DAT_801b24e4.unkf = 1;
            DAT_801b24e4.unk1e -= 500;
            if ((s16)DAT_801b24e4.unk1e <= 0) {
                DAT_801b24e4.unk1e = 0;
                if (id == 0x4d || id == 0x4f) {
                    flag = 1;
                } else {
                    flag = 0x3c;
                }
                DAT_801b24e4.unkf = flag;
            }
            DAT_801b24e4.unk10 = 1;
            DAT_801b24e4.unk22 -= 500;
            if ((s16)DAT_801b24e4.unk22 <= 0) {
                DAT_801b24e4.unk22 = 0;
                if (DAT_801b24e4.unkf4 != 0x4d && DAT_801b24e4.unkf4 != 0x4f) {
                    DAT_801b24e4.unk10 = 0x3c;
                } else {
                    DAT_801b24e4.unk10 = 0;
                }
            }
            DAT_801b24e4.unk14 -= 128;
            if (DAT_801b24e4.unk14 < 1024) {
                DAT_801b24e4.unk14 = 0x400;
            }
            return;
        }
    }
    sum = 0;
    for (i = 4; i != -1; i--) {
        sum += DAT_801b24e4.unk42[i];
    }
    sum /= 5;
    if (DAT_801b24e4.unk10 == 0) {
        weight = 0;
        if (DAT_801b24e4.unkac != NULL) {
            weight = DAT_801b24e4.unkac->unk1;
            if (DAT_801b24e4.unkf0 == 0x25) {
                weight -= weight >> 2;
            }
            if (DAT_801b24e4.unkf1 == 0x30) {
                weight <<= 1;
            }
        }
        DAT_801b24e4.unk22 += func_80029390(sum, weight);
    } else {
        DAT_801b24e4.unk10--;
    }
    if (DAT_801b24e4.unk22 > 5000) {
        DAT_801b24e4.unk22 = 5000;
    }
    DAT_801b24e4.unk14 += 128;
    if (DAT_801b24e4.unk14 > 4096) {
        DAT_801b24e4.unk14 = 0x1000;
    }
    DAT_801b24e4.unk11 = 1;
    if (DAT_801b24e4.unkc6 != 0) {
        DAT_801b24e4.unkc6--;
    } else {
        DAT_801b24e4.unka8 = DAT_8009c3cc;
    }
    mask = DAT_80081868[7] | DAT_80081868[4] | DAT_80081868[6];
    if ((DAT_801b24e4.unk178 & mask) && !(DAT_801b24e4.unk17a & mask)) {
        if (DAT_801b24e4.unkb0->unk24 != 0 && func_8002cfe0(1) && DAT_801b24e4.unkb0->unk4 != 0xff
            && (DAT_801b24e4.unk178 & *DAT_801b24e4.unka8)
            && (DAT_801b24e4.unka8 != DAT_8009c3cc
                || (DAT_801b24e4.unk1e == 5000 && DAT_801b24e4.unk22 == DAT_801b24e4.unk1e))) {
            next = DAT_801b24e4.unka8;
            DAT_801b24e4.unka8 = next + 1;
            if (next[1] == 0xffff) {
                func_8002c040(1);
                return;
            }
            DAT_801b24e4.unkc6 = 20;
        } else {
            DAT_801b24e4.unkc6 = 0;
        }
    }
    if (DAT_801b24e4.unkf == 0 && DAT_801b24e4.unkc6 == 0 && (DAT_801b24e4.unk178 & DAT_80081868[4])
        && !(DAT_801b24e4.unk17a & DAT_80081868[4])) {
        func_8002c040(0);
    }
}

ADDRESS(0x8003047c, 0xec)
void func_8003047c(void)
{
    DAT_801b24e4.unk188.knock.push.vx =
        func_80016168(0, DAT_801b24e4.unk134, DAT_801b24e4.unk188.knock.push.vx, 8, 4);
    DAT_801b24e4.unk188.knock.push.vy =
        func_80016168(0, DAT_801b24e4.unk136, DAT_801b24e4.unk188.knock.push.vy, 8, 4);
    DAT_801b24e4.unk188.knock.push.vz =
        func_80016168(0, DAT_801b24e4.unk138, DAT_801b24e4.unk188.knock.push.vz, 8, 4);
    DAT_801b24e4.unk134 += DAT_801b24e4.unk188.knock.push.vx;
    DAT_801b24e4.unk136 += DAT_801b24e4.unk188.knock.push.vy;
    DAT_801b24e4.unk138 += DAT_801b24e4.unk188.knock.push.vz;
}

ADDRESS(0x80030568, 0x70)
void func_80030568(void)
{
    u8 f9 = DAT_801b24e4.unkf9;
    u8 fa = DAT_801b24e4.unkfa;

    DAT_801b24e4.unkf9 = 0;
    DAT_801b24e4.unkfa = 0;
    func_800422b8(NULL, NULL);
    DAT_801b24e4.unkf9 = f9;
    DAT_801b24e4.unkfa = fa;
    func_800438e0();
}

ADDRESS(0x800305d8, 0x1b4)
void func_800305d8(void)
{
    s32 result;

    if (DAT_801b24e4.unk101 == 0x11) {
        return;
    }
    if ((DAT_801b24e4.unk178 & DAT_80081868[7]) && !(DAT_801b24e4.unk17a & DAT_80081868[7])) {
        func_8005e2d0(&DAT_801b24e4.pos, &DAT_801b24e4.unk12c);
    }
    if ((DAT_801b24e4.unk178 & DAT_80081868[5]) && !(DAT_801b24e4.unk17a & DAT_80081868[5])
        && DAT_801b24e4.unkc0 == -1) {
        func_80030568();
        do {
            func_80019538();
            func_80018358();
        } while ((s16)DAT_8018fad4.loading == 1 && DAT_8018fad4.unk17 == 0);
        func_8001bea8();
        func_800290d8(DAT_8018fad4.area[0]);
        result = func_8001a774();
        if (result >= 0) {
            func_800422b8(NULL, NULL);
            func_8005cbe0(&DAT_801b24e4.pos, &DAT_801b24e4.unk12c, result);
        } else if (result == -2) {
            DAT_8009c3f8 = 2;
        } else if (result == -3) {
            func_80028f90();
            func_80029188(DAT_8018fad4.area[0], DAT_8018fad4.area[0], DAT_8018fad4.area[0],
                          DAT_8018fad4.area[3], DAT_8018fad4.area[4], 0xff);
        }
        func_8002b6f0();
    }
}

ADDRESS(0x8003078c, 0x68)
void func_8003078c(void)
{
    DAT_801b24e4.unk101 = 0;
    DAT_801b24e4.unk164 = 0;
    DAT_801b24e4.unk162 = 0;
    DAT_801b24e4.unk138 = 0;
    DAT_801b24e4.unk136 = 0;
    DAT_801b24e4.unk134 = 0;
    DAT_801b24e4.unk140 = 0;
    DAT_801b24e4.unk13e = 0;
    DAT_801b24e4.unk13c = 0;
    DAT_801b24e4.unk148 = 0;
    DAT_801b24e4.unk146 = 0;
    DAT_801b24e4.unk144 = 0;
}

ADDRESS(0x800307f4, 0x1c)
void func_800307f4(u8 id)
{
    DAT_801b24e4.unk101 = 8;
    DAT_801b24e4.unk188.turn.id = id;
}

ADDRESS(0x80030810, 0x54)
void func_80030810(u8 id)
{
    DAT_801b24e4.unk101 = 7;
    DAT_801b24e4.unk188.turn.id = id;
    DAT_801b24e4.unk188.turn.unk1 = 0;
    DAT_801b24e4.unk188.turn.angle = DAT_801b24e4.unk12c;
}

ADDRESS(0x80030864, 0x54)
void func_80030864(u8 id)
{
    DAT_801b24e4.unk101 = 9;
    DAT_801b24e4.unk188.turn.id = id;
    DAT_801b24e4.unk188.turn.unk1 = 0;
    DAT_801b24e4.unk188.turn.angle = DAT_801b24e4.unk12c;
}

ADDRESS(0x800308b8, 0x3c)
void func_800308b8(SVECTOR *dir)
{
    DAT_801b24e4.unk101 = 3;
    DAT_801b24e4.unk188.knock.dir = *dir;
}

ADDRESS(0x800308f4, 0x14)
void func_800308f4(void)
{
    DAT_801b24e4.unk101 = 4;
}

ADDRESS(0x80030908, 0xc0)
void func_80030908(SVECTOR *dir, SVECTOR *push, s16 lift)
{
    if (DAT_801b24e4.unk101 == 7 || DAT_801b24e4.unk101 == 8 || DAT_801b24e4.unk101 == 9) {
        return;
    }
    DAT_801b24e4.unk101 = 0x10;
    DAT_801b24e4.unk188.knock.dir = *dir;
    DAT_801b24e4.unk188.knock.push = *push;
    DAT_801b24e4.unk174 = 3500;
    DAT_801b24e4.unk176 = lift;
    DAT_801b24e4.unk104 = 0x40;
    DAT_801b24e4.unk172 = DAT_801b24e4.unk188.knock.dir.vy;
}

ADDRESS(0x800309c8, 0xa4)
void func_800309c8(SVECTOR *dir, SVECTOR *push, s16 lift)
{
    if (DAT_801b24e4.unk101 == 7 || DAT_801b24e4.unk101 == 8 || DAT_801b24e4.unk101 == 9) {
        return;
    }
    DAT_801b24e4.unk101 = 0x12;
    DAT_801b24e4.unk188.knock.dir = *dir;
    DAT_801b24e4.unk188.knock.push = *push;
    DAT_801b24e4.unk174 = 3500;
    DAT_801b24e4.unk176 = lift;
}

/* Death: mode 0x11, knocked along dir (NULL: in place). */
ADDRESS(0x80030a6c, 0x8c)
void func_80030a6c(SVECTOR *dir)
{
    if (DAT_801b24e4.unk101 == 0x11) {
        return;
    }
    DAT_801b24e4.unk101 = 0x11;
    func_8001576c(6, 0x6e);
    if (dir != NULL) {
        DAT_801b24e4.unk188.knock.dir = *dir;
    }
    DAT_801b24e4.unk188.knock.push.vx = 0;
    DAT_801b24e4.unk13a = 0;
}

ADDRESS(0x80030af8, 0x2c)
void func_80030af8(void)
{
    if (DAT_801b24e4.unk101 == 0) {
        DAT_801b24e4.unk101 = 5;
        DAT_801b24e4.unk188.count = 0;
    }
}

/*
 * Steps an effect counter towards *timer (up while below it, else down with
 * the timer cleared); returns count scaled to scale over range frames, -1 when
 * both are 0.
 */
ADDRESS(0x80030b24, 0xbc)
s16 func_80030b24(s16 *count, s16 *timer, s32 range, s32 scale)
{
    s32 v;
    s16 n = *count;
    s16 t = 0;

    if (timer != NULL) {
        t = *timer;
    }
    if (n != 0 || t != 0) {
        if (n < t) {
            n++;
        } else {
            t = 0;
            n--;
        }
        if (n < range) {
            v = n * scale / range;
        } else {
            v = scale;
        }
        *count = n;
        if (timer != NULL) {
            *timer = t;
        }
        return v;
    }
    return -1;
}

ADDRESS(0x80030be0, 0x6c)
void func_80030be0(s32 amount)
{
    s32 hp = DAT_801b24e4.hp;

    hp += amount;
    if (hp <= 0) {
        DAT_801b24e4.hp = 0;
        func_80030a6c(NULL);
    } else if (hp > DAT_801b24e4.hp_max) {
        DAT_801b24e4.hp = DAT_801b24e4.hp_max;
    } else {
        DAT_801b24e4.hp = hp;
    }
}

ADDRESS(0x80030c4c, 0x20)
void func_80030c4c(s32 amount)
{
    func_80030be0(amount);
}

ADDRESS(0x80030c6c, 0x50)
void func_80030c6c(s32 amount)
{
    s32 v = DAT_801b24e4.unk1c;

    v += amount;
    if (v <= 0) {
        DAT_801b24e4.unk1c = 0;
    } else if (v > DAT_801b24e4.unk1a) {
        DAT_801b24e4.unk1c = DAT_801b24e4.unk1a;
    } else {
        DAT_801b24e4.unk1c = v;
    }
}

ADDRESS(0x80030cbc, 0xac)
void func_80030cbc(Record20 *r)
{
    if (r->unk14 != 0 && DAT_801b24e4.unk9c % r->unk14 == 0) {
        func_80030be0(1);
    }
    if (r->unk16 != 0 && DAT_801b24e4.unk9c % r->unk16 == 0) {
        func_80030be0(-1);
    }
}

ADDRESS(0x80030d68, 0xac)
void func_80030d68(Record20 *r)
{
    if (r->unk18 != 0 && DAT_801b24e4.unk9c % r->unk18 == 0) {
        func_80030c6c(1);
    }
    if (r->unk1a != 0 && DAT_801b24e4.unk9c % r->unk1a == 0) {
        func_80030c6c(-1);
    }
}

ADDRESS(0x80030e14, 0x1b8)
void func_80030e14(void)
{
    s16 v;
    s16 t;

    v = func_80030b24(&DAT_801b24e4.unk7e, &DAT_801b24e4.unk80, 0x40, 0xc00);
    if (v != -1) {
        func_8003456c(10, 10, 10, 5000, 6000, v);
        func_8003da10(0x80, 0x80, 0x80, v >> 2, 2);
    }
    v = func_80030b24(&DAT_801b24e4.unk8c, &DAT_801b24e4.unk8e, 0x40, 0xe10);
    if (v != -1) {
        func_8003456c(0xdc, 0xdc, 0xa0, 0xffff, 18000, v);
    }
    if (DAT_801b24e4.unk174 != 0) {
        DAT_801b24e4.unk174 -= DAT_801b24e4.unk176;
        if (DAT_801b24e4.unk174 < 0) {
            DAT_801b24e4.unk174 = 0;
        }
        if (DAT_801b24e4.unk174 >= 4096) {
            t = 4096;
        } else {
            t = DAT_801b24e4.unk174;
        }
        func_8003d8ec(0x3c, 0, 0, t, 1);
    }
    v = func_80030b24(&DAT_801b24e4.unk7a, &DAT_801b24e4.unk7c, 0x40, 0xc00);
    if (v != -1) {
        if (DAT_801b24e4.unk7a == 0) {
            func_80029500();
        }
        func_8003d8ec(0x1e, 0x1e, 10, v, 1);
    }
}
