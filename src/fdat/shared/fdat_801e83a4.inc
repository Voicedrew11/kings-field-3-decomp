/*
 * Blends two light records into out by t (4.12), as GAME.EXE's
 * func_800344dc blends its working copy (shared fragment, FDAT_014.BIN's
 * claim; the module's first function, before its empty default handler).
 */
ADDRESS(0x801e83a4, 0xe4)
void fdat_801e83a4(Record30 *from, Record30 *to, Record30 *out, s32 t)
{
    func_800171b0((MATRIX *)&from->unk14, (MATRIX *)&to->unk14, (MATRIX *)&out->unk14, t);
    func_800171b0((MATRIX *)&from->unk0, (MATRIX *)&to->unk0, (MATRIX *)&out->unk0, t);
    out->unk28.r = func_80017158(from->unk28.r, to->unk28.r, t);
    out->unk28.g = func_80017158(from->unk28.g, to->unk28.g, t);
    out->unk28.b = func_80017158(from->unk28.b, to->unk28.b, t);
    out->unk2c = func_80017158(from->unk2c, to->unk2c, t);
    out->unk2e = func_80017158(from->unk2e, to->unk2e, t);
}
