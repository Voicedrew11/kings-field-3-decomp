/*
 * Slot 16 (shared fragment, FDAT_077.BIN's claim): GAME.EXE's actor_score_target_candidate
 * asks whether a script record applies at dist; record 0x85 applies only
 * between its two distances (u16 at +0x12 and +0x10).
 */
ADDRESS(0x801e875c, 0x44)
s32 fdat_801e875c(u8 *script, s32 dist)
{
    if (script[0] != 0x85) {
        return 1;
    }
    if (dist >= ((u16 *)script)[8] || ((u16 *)script)[9] >= dist) {
        return 0;
    }
    return 1;
}
