/*
 * Blends two light records into out by t (4.12), as GAME.EXE's
 * light_interpolate_records blends its working copy (shared fragment, FDAT_014.BIN's
 * claim; the module's first function, before its empty default handler).
 */
ADDRESS(0x801e83a4, 0xe4)
void fdat_801e83a4(Record30 *from, Record30 *to, Record30 *out, s32 t)
{
    fixed_lerp_nine_halfwords_q12((MATRIX *)&from->unk14, (MATRIX *)&to->unk14, (MATRIX *)&out->unk14, t);
    fixed_lerp_nine_halfwords_q12((MATRIX *)&from->unk0, (MATRIX *)&to->unk0, (MATRIX *)&out->unk0, t);
    out->unk28.r = fixed_lerp_q12(from->unk28.r, to->unk28.r, t);
    out->unk28.g = fixed_lerp_q12(from->unk28.g, to->unk28.g, t);
    out->unk28.b = fixed_lerp_q12(from->unk28.b, to->unk28.b, t);
    out->unk2c = fixed_lerp_q12(from->unk2c, to->unk2c, t);
    out->unk2e = fixed_lerp_q12(from->unk2e, to->unk2e, t);
}
