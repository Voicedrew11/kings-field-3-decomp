/*
 * GAME.EXE: angle, vector and matrix helpers (WIP module). Angles are in
 * 4096ths of a turn; GTE fixed point is 4.12.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <psyq/libc.h>
#include <game/math.h>

ADDRESS(0x80015f18, 0xc4)
s16 angle_approach(s16 angle, s16 target, s16 step)
{
    angle &= 0xfff;
    target &= 0xfff;
    if (target == angle) {
        return target;
    }
    if (angle < target) {
        if (target - angle <= 2048) {
            angle += step;
            if (angle > target) {
                return target;
            }
        } else {
            angle = (angle - step) & 0xfff;
            if (angle >= 2048 && angle <= target) {
                return target;
            }
        }
    } else {
        if (angle - target <= 2048) {
            angle -= step;
            if (angle < target) {
                return target;
            }
        } else {
            angle = (angle + step) & 0xfff;
            if (angle <= 2048 && angle >= target) {
                return target;
            }
        }
    }
    return angle;
}

ADDRESS(0x80015fdc, 0x68)
s32 value_approach(s32 value, s32 target, s32 step)
{
    if (step > 0) {
        if (value < target) {
            value += step;
            if (value > target) {
                value = target;
            }
        } else if (value > target) {
            value -= step;
            if (value < target) {
                value = target;
            }
        }
    } else {
        if (value > target) {
            value += step;
            if (value < target) {
                value = target;
            }
        } else if (value < target) {
            value -= step;
            if (value > target) {
                value = target;
            }
        }
    }
    return value;
}

ADDRESS(0x80016044, 0x124)
void vector_direction_scaled(VECTOR *from, VECTOR *to, s32 length, SVECTOR *out)
{
    s32 dx = to->vx - from->vx;
    s32 dy = to->vy - from->vy;
    s32 dz = to->vz - from->vz;
    s32 distance = fixed_vector3_length(dx, dy, dz);

    out->vx = dx * length / distance;
    out->vy = dy * length / distance;
    out->vz = dz * length / distance;
}

ADDRESS(0x80016168, 0xd8)
s32 angle_velocity_step(s32 angle, s32 target, s32 speed, s32 accel, s32 drag)
{
    if (angle_within_tolerance(angle, target, accel >> 1) && speed <= accel && speed >= -accel) {
        return angle - target;
    }
    if (target != angle) {
        if (angle_mod_delta_le_half_turn(target, angle)) {
            speed -= accel;
        } else {
            speed += accel;
        }
    }
    if (speed > 0) {
        speed -= drag;
    } else if (speed < 0) {
        speed += drag;
    }
    return speed;
}

ADDRESS(0x80016240, 0x50)
void angle_to_forward_xz(s16 angle, s32 *out)
{
    out[0] = -rsin(angle);
    out[1] = rcos(angle);
}

ADDRESS(0x80016290, 0x224)
void matrix_rotate_quarter_turns(MATRIX *src, MATRIX *dst, s32 quarter)
{
    switch (quarter) {
    case 0:
        dst->m[0][0] = src->m[0][0];
        dst->m[0][1] = src->m[0][1];
        dst->m[0][2] = src->m[0][2];
        dst->m[1][0] = src->m[1][0];
        dst->m[1][1] = src->m[1][1];
        dst->m[1][2] = src->m[1][2];
        dst->m[2][0] = src->m[2][0];
        dst->m[2][1] = src->m[2][1];
        dst->m[2][2] = src->m[2][2];
        break;
    case 1:
        dst->m[0][0] = -src->m[0][2];
        dst->m[0][1] = src->m[0][1];
        dst->m[0][2] = src->m[0][0];
        dst->m[1][0] = -src->m[1][2];
        dst->m[1][1] = src->m[1][1];
        dst->m[1][2] = src->m[1][0];
        dst->m[2][0] = -src->m[2][2];
        dst->m[2][1] = src->m[2][1];
        dst->m[2][2] = src->m[2][0];
        break;
    case 2:
        dst->m[0][0] = -src->m[0][0];
        dst->m[0][1] = src->m[0][1];
        dst->m[0][2] = -src->m[0][2];
        dst->m[1][0] = -src->m[1][0];
        dst->m[1][1] = src->m[1][1];
        dst->m[1][2] = -src->m[1][2];
        dst->m[2][0] = -src->m[2][0];
        dst->m[2][1] = src->m[2][1];
        dst->m[2][2] = -src->m[2][2];
        break;
    case 3:
        dst->m[0][0] = src->m[0][2];
        dst->m[0][1] = src->m[0][1];
        dst->m[0][2] = -src->m[0][0];
        dst->m[1][0] = src->m[1][2];
        dst->m[1][1] = src->m[1][1];
        dst->m[1][2] = -src->m[1][0];
        dst->m[2][0] = src->m[2][2];
        dst->m[2][1] = src->m[2][1];
        dst->m[2][2] = -src->m[2][0];
        break;
    }
}

ADDRESS(0x800164b4, 0xe4)
void svector_rotate_quarter_turns(SVECTOR *src, SVECTOR *dst, s32 quarter)
{
    switch (quarter) {
    case 0:
        dst->vx = src->vx;
        dst->vy = src->vy;
        dst->vz = src->vz;
        break;
    case 1:
        dst->vx = -src->vz;
        dst->vy = src->vy;
        dst->vz = src->vx;
        break;
    case 2:
        dst->vx = -src->vx;
        dst->vy = src->vy;
        dst->vz = -src->vz;
        break;
    case 3:
        dst->vx = src->vz;
        dst->vy = src->vy;
        dst->vz = -src->vx;
        break;
    }
}

ADDRESS(0x80016598, 0x74)
void matrix_set_rotation_x(s16 angle, MATRIX *m)
{
    s32 s = rsin(angle);
    s32 c = rcos(angle);

    m->m[0][0] = 0x1000;
    m->m[0][1] = 0;
    m->m[0][2] = 0;
    m->m[1][0] = 0;
    m->m[1][1] = c;
    m->m[1][2] = -s;
    m->m[2][0] = 0;
    m->m[2][1] = s;
    m->m[2][2] = c;
}

ADDRESS(0x8001660c, 0x74)
void matrix_set_rotation_y(s16 angle, MATRIX *m)
{
    s32 s = rsin(angle);
    s32 c = rcos(angle);

    m->m[0][0] = c;
    m->m[0][1] = 0;
    m->m[0][2] = -s;
    m->m[1][0] = 0;
    m->m[1][1] = 0x1000;
    m->m[1][2] = 0;
    m->m[2][0] = s;
    m->m[2][1] = 0;
    m->m[2][2] = c;
}

ADDRESS(0x80016680, 0x74)
void matrix_set_rotation_z(s16 angle, MATRIX *m)
{
    s32 s = rsin(angle);
    s32 c = rcos(angle);

    m->m[0][0] = c;
    m->m[0][1] = -s;
    m->m[0][2] = 0;
    m->m[1][0] = s;
    m->m[1][1] = c;
    m->m[1][2] = 0;
    m->m[2][0] = 0;
    m->m[2][1] = 0;
    m->m[2][2] = 0x1000;
}

ADDRESS(0x800166f4, 0x6c)
void matrix_set_rotation_yxz(SVECTOR *rotation, MATRIX *m)
{
    MATRIX work;

    matrix_set_rotation_z(rotation->vz, &work);
    matrix_set_rotation_x(rotation->vx, m);
    MulMatrix(m, &work);
    matrix_set_rotation_y(rotation->vy, &work);
    MulMatrix2(&work, m);
}

ADDRESS(0x80016760, 0x6c)
void matrix_set_rotation_xzy(SVECTOR *rotation, MATRIX *m)
{
    MATRIX work;

    matrix_set_rotation_y(rotation->vy, &work);
    matrix_set_rotation_z(rotation->vz, m);
    MulMatrix(m, &work);
    matrix_set_rotation_x(rotation->vx, &work);
    MulMatrix2(&work, m);
}

ADDRESS(0x800167cc, 0xdc)
void pitch_yaw_to_forward_vector(SVECTOR *rotation, SVECTOR *out)
{
    MATRIX pitch;
    MATRIX yaw;
    SVECTOR v;
    VECTOR result;

    v.vx = 0;
    v.vy = 0;
    v.vz = 0x1000;
    matrix_set_rotation_x(-rotation->vx, &pitch);
    ApplyMatrix(&pitch, &v, &result);
    v.vx = result.vx;
    v.vy = result.vy;
    v.vz = result.vz;
    matrix_set_rotation_y(rotation->vy, &yaw);
    ApplyMatrix(&yaw, &v, &result);
    out->vx = result.vx;
    out->vy = result.vy;
    out->vz = result.vz;
}

ADDRESS(0x800168a8, 0x48)
void vector_rotate_yxz(SVECTOR *rotation, SVECTOR *v, VECTOR *out)
{
    MATRIX m;

    matrix_set_rotation_yxz(rotation, &m);
    ApplyMatrix(&m, v, out);
}

ADDRESS(0x800168f0, 0x38)
void vector2i_scale_shift11(s16 scale, s32 *v)
{
    v[0] = scale * v[0] >> 11;
    v[1] = scale * v[1] >> 11;
}

ADDRESS(0x80016928, 0x50)
void vector3s_scale_shift12(s16 scale, SVECTOR *v)
{
    v->vx = v->vx * scale >> 12;
    v->vy = v->vy * scale >> 12;
    v->vz = v->vz * scale >> 12;
}

ADDRESS(0x80016978, 0x38)
void vector2i_scale_shift12(s16 scale, s32 *v)
{
    v[0] = scale * v[0] >> 12;
    v[1] = scale * v[1] >> 12;
}

ADDRESS(0x800169b0, 0x50)
void func_800169b0(s16 scale, SVECTOR *v)
{
    v->vx = v->vx * scale >> 12;
    v->vy = v->vy * scale >> 12;
    v->vz = v->vz * scale >> 12;
}

ADDRESS(0x80016a00, 0x2c)
void vector3i_add_xz(VECTOR *v, s32 *delta)
{
    v->vx += delta[0];
    v->vz += delta[1];
}

ADDRESS(0x80016a2c, 0x3c)
s32 angle_within_tolerance(s16 angle, s16 target, s16 range)
{
    s32 within = 0;
    s32 difference = (u16)(angle - target) & 0xfff;

    if (range >= difference || difference >= 0x1000 - range) {
        within = 1;
    }
    return within;
}

ADDRESS(0x80016a68, 0x10)
s32 angle_mod_delta_le_half_turn(s16 angle, s16 target)
{
    s16 difference = (angle - target) & 0xfff;

    return difference <= 2048;
}

ADDRESS(0x80016a78, 0x20)
s32 angle_signed_greater_than(s32 angle, s32 target)
{
    return (s16)(((angle + 2048) & 0xfff) - ((target + 2048) & 0xfff)) > 0;
}

ADDRESS(0x80016a98, 0x20)
s32 angle_shortest_delta(s32 angle, s32 target)
{
    s32 difference = (target - angle) & 0xfff;

    return difference > 2048 ? difference - 0x1000 : difference;
}

ADDRESS(0x80016ab8, 0x150)
s32 vector_xz_to_angle(s32 dx, s32 dz)
{
    s32 abs_x = dx < 0 ? -dx : dx;
    s32 abs_z = dz < 0 ? -dz : dz;

    if (abs_z >= abs_x) {
        if (dz > 0) {
            return -catan((dx << 12) / dz) & 0xfff;
        }
        if (dz < 0) {
            return 0x800 - catan((dx << 12) / dz);
        }
        return 0;
    }
    if (dx >= 0) {
        return catan((dz << 12) / dx) + 0xc00;
    }
    return catan((dz << 12) / dx) + 0x400;
}

ADDRESS(0x80016c08, 0x3c)
s32 fixed_vector2_length(s32 dx, s32 dz)
{
    dx >>= 3;
    dz >>= 3;
    return SquareRoot0(dx * dx + dz * dz) << 3;
}

ADDRESS(0x80016c44, 0x4c)
s32 fixed_vector3_length(s32 dx, s32 dy, s32 dz)
{
    dx >>= 3;
    dy >>= 3;
    dz >>= 3;
    return SquareRoot0(dx * dx + dy * dy + dz * dz) << 3;
}

ADDRESS(0x80016c90, 0x7c)
void vector_displacement_to_pitch_yaw(s32 dx, s32 dy, s32 dz, SVECTOR *out)
{
    out->vy = vector_xz_to_angle(dx, dz);
    out->vx = -vector_xz_to_angle(dy, fixed_vector2_length(dx, dz)) & 0xfff;
    out->vz = 0;
}

ADDRESS(0x80016d0c, 0x30)
s32 directed_intervals_overlap(s32 a, s32 b, s32 c, s32 d)
{
    if (c < a) {
        if (c < a - b) {
            return 0;
        }
    } else {
        if (a < c - d) {
            return 0;
        }
    }
    return 1;
}

ADDRESS(0x80016d3c, 0x18c)
s32 vector_distance_to_point_in_rotated_box(SVECTOR *rotation, VECTOR *pos, s32 x, s32 y, s32 z,
                                            s32 half_width, s32 half_depth, s32 reach_down, s32 reach_up)
{
    s32 dx = (pos->vx - x) >> 3;
    s32 dz = (pos->vz - z) >> 3;
    s32 distance = SquareRoot0(dx * dx + dz * dz) << 3;
    u16 angle = vector_xz_to_angle(x - pos->vx, z - pos->vz);

    dx = rcos(angle - rotation->vy + 1024) * distance >> 12;
    dz = rsin(angle - rotation->vy + 1024) * distance >> 12;
    if (dx < -half_width || half_width < dx || dz < -half_depth || half_depth < dz) {
        return -1;
    }
    if (y == 0xffff) {
        return distance;
    }
    if (pos->vy < y) {
        if (pos->vy < y - reach_up) {
            return -1;
        }
    } else if (y < pos->vy - reach_down) {
        goto reject;
    }
    return distance;
reject:
    return -1;
}

ADDRESS(0x80016ec8, 0xec)
s32 vector_distance_to_point(VECTOR *pos, s32 x, s32 y, s32 z, s32 radius, s32 reach_down, s32 reach_up)
{
    s32 dx;
    s32 dz;
    s32 distance;

    dx = pos->vx - x;
    if (dx < -radius || radius < dx) {
        return -1;
    }
    dz = pos->vz - z;
    if (dz < -radius || radius < dz) {
        return -1;
    }
    if (y != 0xffff) {
        if (pos->vy < y) {
            if (pos->vy < y - reach_up) {
                return -1;
            }
        } else if (y < pos->vy - reach_down) {
            goto reject;
        }
    }
    dx >>= 3;
    dz >>= 3;
    distance = SquareRoot0(dx * dx + dz * dz) << 3;
    if (radius < distance) {
        goto reject;
    }
    return distance;
reject:
    return -1;
}

ADDRESS(0x80016fb4, 0x10c)
s32 vector_distance_between_with_reach(VECTOR *a, s32 radius_a, VECTOR *b, s32 radius_b, s32 height)
{
    s32 reach = radius_a + radius_b;
    s32 dx = b->vx - a->vx;
    s32 dz;
    s32 dy;
    s32 distance;

    if (dx < -reach || reach < dx) {
        return -9999999;
    }
    dz = b->vz - a->vz;
    if (dz < -reach || reach < dz) {
        return -9999999;
    }
    dy = b->vy - a->vy - (height >> 1);
    if (dy < -reach || reach < dy) {
        return -9999999;
    }
    dx >>= 3;
    dz >>= 3;
    dy >>= 3;
    distance = SquareRoot0(dx * dx + dz * dz + dy * dy) << 3;
    if (reach >= distance) {
        if (radius_b < distance) {
            return distance - radius_b;
        }
        return 0;
    }
    return -9999999;
}

ADDRESS(0x800170c0, 0x48)
s32 random_triangular_scaled(s32 range)
{
    s32 r = rand();

    r += rand();
    return r * range >> 15;
}

ADDRESS(0x80017108, 0x50)
s32 random_centered_triangular_scaled(s32 range)
{
    s32 r = rand();

    r += rand();
    r -= 32768;
    return r * range >> 15;
}

ADDRESS(0x80017158, 0x18)
s32 fixed_lerp_q12(s32 from, s32 to, s32 t)
{
    return ((to - from) * t >> 12) + from;
}

ADDRESS(0x80017170, 0x40)
s32 angle_lerp_shortest_q12(s32 from, s32 to, s32 t)
{
    s32 difference = ((s16)to - (s16)from) & 0xfff;

    if (difference > 2048) {
        difference -= 0x1000;
    }
    return (from + (difference * t >> 12)) & 0xfff;
}

ADDRESS(0x800171b0, 0x60)
void fixed_lerp_nine_halfwords_q12(MATRIX *from, MATRIX *to, MATRIX *out, s16 t)
{
    s16 *a = &from->m[0][0];
    s16 *b = &to->m[0][0];
    s16 *c = &out->m[0][0];
    s32 i;

    for (i = 8; i != -1; i--) {
        s16 va = *a++;
        s16 vb = *b++;

        *c++ = va + ((vb - va) * t >> 12);
    }
}

ADDRESS(0x80017210, 0x28c)
s32 trajectory_solve_time_angle(s32 high, s32 distance, s32 height, s32 gravity, s32 speed, s32 *time,
                                s32 *angle)
{
    s32 speed2 = speed * speed;
    s32 gravity2 = gravity * gravity;
    s32 v = speed2 >> 6;
    s32 discriminant = v * v - ((gravity2 * distance >> 10) * (distance >> 2))
                     - ((gravity * v >> 3) * (height >> 2));
    s32 b;
    s32 t1;
    s32 t2;
    s32 t;
    s32 sqrt_t;
    s32 across;
    s32 up;

    if (discriminant < 0) {
        return -1;
    }
    discriminant = SquareRoot0(discriminant << 4) << 2;
    b = (speed2 >> 2) - (height * gravity >> 2);
    t1 = ((b + discriminant) << 1) / gravity2;
    t2 = ((b - discriminant) << 1) / gravity2;
    if (t1 <= 0 && t2 <= 0) {
        return -1;
    }
    if (high == 0) {
        t = t2;
        if (t1 > 0 && t1 <= t2) {
            t = t1;
        }
    } else {
        t = t2;
        if (t2 <= t1) {
            t = t1;
        }
    }
    sqrt_t = SquareRoot0(t << 14);
    across = ((distance << 14) / speed) / sqrt_t;
    up = (((height + ((gravity * t) << 1)) << 12) / (speed >> 1)) / (sqrt_t >> 1);
    *time = sqrt_t;
    *angle = vector_xz_to_angle(up, across);
    return 0;
}

ADDRESS(0x8001749c, 0x10c)
s32 trajectory_solve_motion_between_points(s32 high, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1,
                                           s32 gravity, s32 speed, s16 *time, s16 *horizontal, s16 *vertical)
{
    s32 t;
    s32 angle;
    s32 result = trajectory_solve_time_angle(high, fixed_vector2_length(x1 - x0, z1 - z0), y0 - y1, gravity,
                                             speed, &t, &angle);

    if (result == 0) {
        *horizontal = speed * rcos(angle) >> 12;
        *vertical = speed * -rsin(angle) >> 12;
        *time = t;
    }
    return result;
}

ADDRESS(0x800175a8, 0x58)
void vector_add_scaled_delta(VECTOR *base, SVECTOR *dir, s32 scale, VECTOR *out)
{
    out->vx = dir->vx * scale + base->vx;
    out->vy = dir->vy * scale + base->vy;
    out->vz = dir->vz * scale + base->vz;
}

ADDRESS(0x80017600, 0x5c)
s32 func_80017600(SVECTOR *normal, SVECTOR *point, s32 *d)
{
    s32 r = normal->vx * (*d - point->vx) + normal->vy * (*d - point->vy)
          + normal->vz * (*d - point->vz);

    return r < 0 ? -r : r;
}

ADDRESS(0x8001765c, 0x8)
void resource_noop_callback(void)
{
}
