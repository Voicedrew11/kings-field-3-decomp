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
void scene_position_from_camera_offset(s32 x, s32 y, s32 z, s32 pitch, s32 yaw, s32 dy, s32 dz, VECTOR *out)
{
    SVECTOR v;
    SVECTOR rotation;

    v.vx = x;
    rotation.vx = -pitch;
    rotation.vz = 0;
    v.vy = y + dy;
    v.vz = z + dz;
    rotation.vy = yaw;
    vector_rotate_yxz(&rotation, &v, out);
    out->vx += cur_avatar_status.pos.vx;
    out->vy += cur_avatar_status.pos.vy + dy - 1600;
    out->vz += cur_avatar_status.pos.vz;
}

/* Moves obj between two positions and two angles by t (either pair may be NULL). */
ADDRESS(0x8005bee0, 0xe4)
void scene_pose_interpolate(Object44 *obj, VECTOR *from, VECTOR *to, SVECTOR *afrom, SVECTOR *ato, s32 t)
{
    if (from != NULL) {
        obj->pos.vx = fixed_lerp_q12(from->vx, to->vx, t);
        obj->pos.vy = fixed_lerp_q12(from->vy, to->vy, t);
        obj->pos.vz = fixed_lerp_q12(from->vz, to->vz, t);
    }
    if (afrom != NULL) {
        obj->angle.vx = angle_lerp_shortest_q12(afrom->vx, ato->vx, t);
        obj->angle.vz = angle_lerp_shortest_q12(afrom->vz, ato->vz, t);
        obj->angle.vy = angle_lerp_shortest_q12(afrom->vy, ato->vy, t);
    }
}

ADDRESS(0x8005bfc4, 0x88)
void scene_position_interpolate(Object44 *obj, VECTOR *from, VECTOR *to, s32 t)
{
    if (from != NULL) {
        obj->pos.vx = fixed_lerp_q12(from->vx, to->vx, t);
        obj->pos.vy = fixed_lerp_q12(from->vy, to->vy, t);
        obj->pos.vz = fixed_lerp_q12(from->vz, to->vz, t);
    }
}

ADDRESS(0x8005c04c, 0x88)
void scene_angles_interpolate(Object44 *obj, SVECTOR *afrom, SVECTOR *ato, s32 t)
{
    if (afrom != NULL) {
        obj->angle.vx = angle_lerp_shortest_q12(afrom->vx, ato->vx, t);
        obj->angle.vz = angle_lerp_shortest_q12(afrom->vz, ato->vz, t);
        obj->angle.vy = angle_lerp_shortest_q12(afrom->vy, ato->vy, t);
    }
}
