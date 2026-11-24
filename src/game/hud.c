/*
 * GAME.EXE: 2D overlay sprites and on-screen messages (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <stdarg.h>
#include <LIBGTE.H>
#include <LIBGPU.H>
#include <game/data.h>
#include <game/render.h>

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
    if (DAT_8018fad4.unk4 == 10 && mode == 2) {
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
