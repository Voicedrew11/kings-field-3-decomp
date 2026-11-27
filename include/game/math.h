#ifndef GAME_MATH_H
#define GAME_MATH_H

/*
 * GAME.EXE angle, vector and matrix helpers (src/game/matrix_rotation.c). Angles are in
 * 4096ths of a turn; GTE fixed point is 4.12 (0x1000 = 1.0).
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>

s16 angle_approach(s16 angle, s16 target, s16 step);
s32 value_approach(s32 value, s32 target, s32 step);
void vector_direction_scaled(VECTOR *from, VECTOR *to, s32 length, SVECTOR *out);
s32 angle_velocity_step(s32 angle, s32 target, s32 speed, s32 accel, s32 drag);
void angle_to_forward_xz(s16 angle, s32 *out);
void matrix_rotate_quarter_turns(MATRIX *src, MATRIX *dst, s32 quarter);
void svector_rotate_quarter_turns(SVECTOR *src, SVECTOR *dst, s32 quarter);
void matrix_set_rotation_x(s16 angle, MATRIX *m);
void matrix_set_rotation_y(s16 angle, MATRIX *m);
void matrix_set_rotation_z(s16 angle, MATRIX *m);
void matrix_set_rotation_yxz(SVECTOR *rotation, MATRIX *m);
void matrix_set_rotation_xzy(SVECTOR *rotation, MATRIX *m);
void pitch_yaw_to_forward_vector(SVECTOR *rotation, SVECTOR *out);
void vector_rotate_yxz(SVECTOR *rotation, SVECTOR *v, VECTOR *out);
void vector2i_scale_shift11(s16 scale, s32 *v);
void vector3s_scale_shift12(s16 scale, SVECTOR *v);
void vector2i_scale_shift12(s16 scale, s32 *v);
void func_800169b0(s16 scale, SVECTOR *v);
void vector3i_add_xz(VECTOR *v, s32 *delta);
s32 angle_within_tolerance(s16 angle, s16 target, s16 range);
s32 angle_mod_delta_le_half_turn(s16 angle, s16 target);
s32 angle_signed_greater_than(s32 angle, s32 target);
s32 angle_shortest_delta(s32 angle, s32 target);
s32 vector_xz_to_angle(s32 dx, s32 dz);
s32 fixed_vector2_length(s32 dx, s32 dz);
s32 fixed_vector3_length(s32 dx, s32 dy, s32 dz);
void vector_displacement_to_pitch_yaw(s32 dx, s32 dy, s32 dz, SVECTOR *out);
s32 directed_intervals_overlap(s32 a, s32 b, s32 c, s32 d);
s32 vector_distance_to_point_in_rotated_box(SVECTOR *rotation, VECTOR *pos, s32 x, s32 y, s32 z,
                                            s32 half_width, s32 half_depth, s32 reach_down, s32 reach_up);
s32 vector_distance_to_point(VECTOR *pos, s32 x, s32 y, s32 z, s32 radius, s32 reach_down, s32 reach_up);
s32 vector_distance_between_with_reach(VECTOR *a, s32 radius_a, VECTOR *b, s32 radius_b, s32 height);
s32 random_triangular_scaled(s32 range);
s32 random_centered_triangular_scaled(s32 range);
s32 fixed_lerp_q12(s32 from, s32 to, s32 t);
s32 angle_lerp_shortest_q12(s32 from, s32 to, s32 t);
void fixed_lerp_nine_halfwords_q12(MATRIX *from, MATRIX *to, MATRIX *out, s16 t);
s32 trajectory_solve_time_angle(s32 high, s32 distance, s32 height, s32 gravity, s32 speed, s32 *time,
                                s32 *angle);
s32 trajectory_solve_motion_between_points(s32 high, s32 x0, s32 y0, s32 z0, s32 x1, s32 y1, s32 z1,
                                           s32 gravity, s32 speed, s16 *time, s16 *horizontal, s16 *vertical);
void vector_add_scaled_delta(VECTOR *base, SVECTOR *dir, s32 scale, VECTOR *out);
s32 func_80017600(SVECTOR *normal, SVECTOR *point, s32 *d);
void resource_noop_callback(void);

#endif
