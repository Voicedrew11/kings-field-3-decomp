/*
 * GAME.EXE: point relative to the player and Object44 position/angle
 * interpolation helpers (WIP module).
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/data.h>
#include <game/misc.h>

/* (x, y + dy, z + dz) turned by (-pitch, yaw, 0), relative to the player's eye. */
ADDRESS(0x8005be28, 0xb8)
void func_8005be28(s32 x, s32 y, s32 z, s32 pitch, s32 yaw, s32 dy, s32 dz, VECTOR *out)
{
    SVECTOR v;
    SVECTOR rotation;

    v.vx = x;
    rotation.vx = -pitch;
    rotation.vz = 0;
    v.vy = y + dy;
    v.vz = z + dz;
    rotation.vy = yaw;
    func_800168a8(&rotation, &v, out);
    out->vx += DAT_801b24e4.pos.vx;
    out->vy += DAT_801b24e4.pos.vy + dy - 1600;
    out->vz += DAT_801b24e4.pos.vz;
}

/* Moves obj between two positions and two angles by t (either pair may be NULL). */
ADDRESS(0x8005bee0, 0xe4)
void func_8005bee0(Object44 *obj, VECTOR *from, VECTOR *to, SVECTOR *afrom, SVECTOR *ato, s32 t)
{
    if (from != NULL) {
        obj->x = func_80017158(from->vx, to->vx, t);
        obj->y = func_80017158(from->vy, to->vy, t);
        obj->z = func_80017158(from->vz, to->vz, t);
    }
    if (afrom != NULL) {
        obj->angle.vx = func_80017170(afrom->vx, ato->vx, t);
        obj->angle.vz = func_80017170(afrom->vz, ato->vz, t);
        obj->angle.vy = func_80017170(afrom->vy, ato->vy, t);
    }
}

ADDRESS(0x8005bfc4, 0x88)
void func_8005bfc4(Object44 *obj, VECTOR *from, VECTOR *to, s32 t)
{
    if (from != NULL) {
        obj->x = func_80017158(from->vx, to->vx, t);
        obj->y = func_80017158(from->vy, to->vy, t);
        obj->z = func_80017158(from->vz, to->vz, t);
    }
}

ADDRESS(0x8005c04c, 0x88)
void func_8005c04c(Object44 *obj, SVECTOR *afrom, SVECTOR *ato, s32 t)
{
    if (afrom != NULL) {
        obj->angle.vx = func_80017170(afrom->vx, ato->vx, t);
        obj->angle.vz = func_80017170(afrom->vz, ato->vz, t);
        obj->angle.vy = func_80017170(afrom->vy, ato->vy, t);
    }
}
