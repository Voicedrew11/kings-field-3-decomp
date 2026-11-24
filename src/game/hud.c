/*
 * GAME.EXE: 2D overlay sprites and on-screen messages (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <stdarg.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>

/* Brightness the 3D overlays share with the status sprites (.sdata). */
DATA(0x8009c2e0, 0x4, ".sdata")
CVECTOR DAT_8009c2e0 = { 0x80, 0x80, 0x80, 0 };
DATA(0x8009c2e4, 0x8, ".sdata")
SVECTOR DAT_8009c2e4 = { 0x1000, 0x1000, 0x1000, 0 };
DATA(0x8009c2ec, 0x4, ".sdata")
s32 DAT_8009c2ec = 0;

ADDRESS(0x80041ad4, 0x2c8)
void func_80041ad4(SpriteRec *sprite, u16 mode, u8 *rgb)
{
    DR_MODE *dr;
    SPRT *p;

    dr = (DR_MODE *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(DR_MODE);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    p = (SPRT *)DAT_80199154.pool->next;
    DAT_80199154.pool->next += sizeof(SPRT);
    if (DAT_80199154.pool->next > DAT_80199154.pool->end) {
        return;
    }
    DAT_80199154.prim_count++;
    if (DAT_8018fad4.area[0] == 10 && mode == 2) {
        p->x0 = sprite->x + 1;
        p->y0 = sprite->y + 1;
    } else {
        p->x0 = sprite->x;
        p->y0 = sprite->y;
    }
    p->clut = sprite->clut;
    p->u0 = sprite->u;
    p->v0 = sprite->v;
    p->w = sprite->w;
    p->h = sprite->h;
    setRGB0(p, rgb[0], rgb[1], rgb[2]);
    setSprt(p);
    if (mode != 0xff) {
        SetDrawMode(dr, 0, 1, sprite->tpage | ((mode & 3) << 5), NULL);
        setSemiTrans(p, 1);
    }
    LINK_PRIM(sprite->z, p);
    LINK_PRIM(sprite->z, dr);
}

ADDRESS(0x80041d9c, 0xcc)
void func_80041d9c(void)
{
    SpriteRec *sprite;
    u8 bright[4];
    u8 dim[4];

    sprite = DAT_80081928;
    bright[0] = bright[1] = bright[2] = DAT_80199154.message_level;
    dim[0] = dim[2] = dim[1] = DAT_80199154.message_level >> 2;
    for (; sprite->visible != 0xff; sprite++) {
        if (sprite->visible & 1) {
            func_80041ad4(sprite, 1, bright);
        }
        if (sprite->visible & 2) {
            func_80041ad4(sprite, 2, bright);
        }
        if (sprite->visible & 4) {
            func_80041ad4(sprite, 2, dim);
        }
    }
}

ADDRESS(0x80041e68, 0x84)
void func_80041e68(void)
{
    SpriteRec *sprite;
    u8 rgb[4];

    sprite = DAT_800819b4;
    rgb[0] = rgb[1] = rgb[2] = DAT_801b24e4.unkfd;
    for (; sprite->visible != 0xff; sprite++) {
        if (sprite->visible != 0) {
            func_80041ad4(sprite, sprite->mode, rgb);
        }
    }
}

ADDRESS(0x80041eec, 0xa0)
void func_80041eec(s32 message, ...)
{
    va_list args;

    va_start(args, message);
    if (message < 240) {
        if (DAT_80199154.message_queue[DAT_80199154.message_head] == 0xff) {
            DAT_80199154.message_queue[DAT_80199154.message_head] = message;
            if (message == 25) {
                DAT_80199154.message_arg[DAT_80199154.message_head] = va_arg(args, s32);
            }
            DAT_80199154.message_head = (DAT_80199154.message_head + 1) & 7;
        }
    }
    va_end(args);
}

ADDRESS(0x80041f8c, 0x10)
void func_80041f8c(SpriteRec *sprite, u8 digit)
{
    sprite->u = digit * 8;
}

ADDRESS(0x80041f9c, 0x31c)
void func_80041f9c(void)
{
    u32 message;
    u8 digits[20];

    switch (DAT_80199154.message_state) {
    case 0:
        message = DAT_80199154.message_queue[DAT_80199154.message_tail];
        if (message == 0xff) {
            break;
        }
        DAT_80199154.message_state = 1;
        DAT_80199154.message_timer = 15;
        DAT_80199154.message_level = 0;
        if (message == 25) {
            DAT_80081928[0].visible = 0;
            DAT_80081928[1].visible = 3;
            DAT_80081928[1].u = 0x80;
            DAT_80081928[1].v = 0x62;
            func_800277c0(DAT_80199154.message_arg[DAT_80199154.message_tail], 4, 0, 0, digits);
            DAT_80081928[2].visible = 3;
            func_80041f8c(&DAT_80081928[2], digits[3]);
            DAT_80081928[3].visible = 3;
            func_80041f8c(&DAT_80081928[3], digits[2]);
            DAT_80081928[4].visible = 3;
            func_80041f8c(&DAT_80081928[4], digits[1]);
            DAT_80081928[5].visible = 3;
            func_80041f8c(&DAT_80081928[5], digits[0]);
        } else {
            DAT_80081928[0].visible = 3;
            DAT_80081928[1].visible = DAT_80081928[2].visible = DAT_80081928[3].visible =
                DAT_80081928[4].visible = DAT_80081928[5].visible = 0;
            DAT_80081928[0].u = message / 18 * 128;
            DAT_80081928[0].v = (u8)(message % 18) * 14;
        }
        break;
    case 1:
        DAT_80199154.message_level += 20;
        if (DAT_80199154.message_level >= 100) {
            DAT_80199154.message_state = 2;
        }
        break;
    case 2:
        if (--DAT_80199154.message_timer == 0) {
            DAT_80199154.message_state = 3;
        }
        break;
    case 3:
        DAT_80199154.message_level -= 20;
        if (DAT_80199154.message_level == 0) {
            message = DAT_80081928[0].visible = DAT_80081928[1].visible =
                DAT_80081928[2].visible = DAT_80081928[3].visible = DAT_80081928[4].visible =
                    DAT_80081928[5].visible =
                        DAT_80199154.message_queue[DAT_80199154.message_tail];
            do {
                DAT_80199154.message_queue[DAT_80199154.message_tail] = 0xff;
                DAT_80199154.message_tail = (DAT_80199154.message_tail + 1) & 7;
            } while (message == DAT_80199154.message_queue[DAT_80199154.message_tail]
                     && message != 25);
            DAT_80199154.message_state = 0;
        }
        break;
    }
}

ADDRESS(0x800422b8, 0x654)
void func_800422b8(VECTOR *pos, SVECTOR *angle)
{
    s32 i;

    DAT_8009c2e0.b = DAT_8009c2e0.g = DAT_8009c2e0.r = DAT_801b24e4.unkfd;
    func_800357e8();
    func_800351fc();
    func_80041f9c();
    func_80034bf4();
    func_80035630();
    func_80043858();
    func_8003df50();
    DAT_800819b4[13].visible = DAT_800819b4[14].visible = 0;
    DAT_80081c20[0].visible = DAT_801b24e4.unkfa & 1;
    DAT_80081c20[1].visible = (DAT_801b24e4.unkfa & 2) >> 1;
    DAT_800819b4[0].visible = DAT_800819b4[1].visible = DAT_800819b4[2].visible =
        DAT_800819b4[3].visible = DAT_800819b4[4].visible = DAT_800819b4[5].visible =
            DAT_800819b4[6].visible = DAT_800819b4[7].visible = DAT_800819b4[8].visible =
                DAT_800819b4[9].visible = DAT_800819b4[10].visible = DAT_800819b4[11].visible =
                    DAT_800819b4[12].visible = DAT_801b24e4.unkf9 & 1;
    DAT_800819b4[15].visible = DAT_800819b4[16].visible = DAT_800819b4[17].visible =
        DAT_800819b4[18].visible = DAT_800819b4[19].visible = DAT_800819b4[20].visible =
            DAT_800819b4[21].visible = DAT_800819b4[22].visible = DAT_800819b4[23].visible =
                DAT_800819b4[24].visible = DAT_800819b4[25].visible = DAT_800819b4[26].visible =
                    DAT_800819b4[27].visible = DAT_800819b4[28].visible =
                        DAT_800819b4[29].visible = (DAT_801b24e4.unkf9 & 2) >> 1;
    i = DAT_801b24e4.unkfa - 1;
    if (i >= 0) {
        DAT_8009c2ec += func_80016a98(DAT_80081c20[i].unk1a, DAT_80199154.unk15b0a);
        if (DAT_8009c2ec > 0) {
            DAT_8009c2ec -= (DAT_8009c2ec + 7) >> 3;
        } else if (DAT_8009c2ec < 0) {
            DAT_8009c2ec -= (DAT_8009c2ec - 7) >> 3;
        }
        DAT_80081c20[i == 0].unk1a = DAT_80081c20[i].unk1a =
            DAT_80081c20[i].unk1a + (DAT_8009c2ec >> 6);
        DAT_80081c20[i == 0].unk18 = DAT_80081c20[i].unk18 = DAT_80199154.unk15b08;
    }
    if (DAT_801b24e4.unkf9 == 1) {
        DAT_800819b4[4].unka = DAT_800819b4[4].w = DAT_801b24e4.hp * 48 / DAT_801b24e4.hp_max;
        DAT_800819b4[5].unka = DAT_800819b4[5].w = DAT_801b24e4.unk1e * 48 / 5000;
        DAT_800819b4[6].unka = DAT_800819b4[6].w = DAT_801b24e4.unk1c * 48 / DAT_801b24e4.unk1a;
        DAT_800819b4[7].unka = DAT_800819b4[7].w = DAT_801b24e4.unk22 * 48 / 5000;
    } else if (DAT_801b24e4.unkf9 == 2) {
        u16 hp;
        u16 mp;
        s32 n;

        hp = DAT_801b24e4.hp % 1000;
        n = hp;
        DAT_800819b4[17].u = n / 100 * 12 + 0x80;
        DAT_800819b4[18].u = n % 100 / 10 * 12 + 0x80;
        DAT_800819b4[19].u = n % 10 * 12 + 0x80;
        mp = DAT_801b24e4.unk1c % 1000;
        n = mp;
        DAT_800819b4[22].u = n / 100 * 12 + 0x80;
        DAT_800819b4[23].u = n % 100 / 10 * 12 + 0x80;
        DAT_800819b4[24].u = n % 10 * 12 + 0x80;
        DAT_800819b4[25].unka = DAT_800819b4[25].w = DAT_801b24e4.unk1e * 64 / 5000;
        DAT_800819b4[26].unka = DAT_800819b4[26].w = DAT_801b24e4.unk22 * 64 / 5000;
    }
    func_8003c35c();
    func_80041e68();
    func_80041d9c();
    func_8003bfd0();
    func_80040ae4();
    func_8003d280();
    func_8003d38c();
    func_8003d41c();
    func_8003d568();
    func_8003d64c();
    func_8003d79c();
    func_80035700();
    func_80019614();
    func_80043940();
}
