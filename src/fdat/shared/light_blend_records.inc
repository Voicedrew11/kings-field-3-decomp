/*
 * Blends two light records into out by t (4.12), as GAME.EXE's
 * light_interpolate_records blends its working copy (shared fragment, FDAT_014.BIN's
 * claim; the module's first function, before its empty default handler).
 */
ADDRESS(0x801e83a4, 0xe4)
void light_blend_records(LightSourceRecord *from, LightSourceRecord *to, LightSourceRecord *out, s32 t)
{
    fixed_lerp_nine_halfwords_q12((MATRIX *)&from->color_matrix, (MATRIX *)&to->color_matrix,
                                  (MATRIX *)&out->color_matrix, t);
    fixed_lerp_nine_halfwords_q12((MATRIX *)&from->light_matrix, (MATRIX *)&to->light_matrix,
                                  (MATRIX *)&out->light_matrix, t);
    out->back_color.r = fixed_lerp_q12(from->back_color.r, to->back_color.r, t);
    out->back_color.g = fixed_lerp_q12(from->back_color.g, to->back_color.g, t);
    out->back_color.b = fixed_lerp_q12(from->back_color.b, to->back_color.b, t);
    out->fog_near = fixed_lerp_q12(from->fog_near, to->fog_near, t);
    out->fog_far = fixed_lerp_q12(from->fog_far, to->fog_far, t);
}
