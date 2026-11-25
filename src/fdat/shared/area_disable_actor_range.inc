/*
 * Sets state 8 in count object records from index on (shared fragment,
 * FDAT_047.BIN's claim).
 */
ADDRESS(0x801e86d8, 0x34)
void area_disable_actor_range(s32 index, s32 count)
{
    Object88 *obj = &DAT_80185da8.objects[index];

    do {
        obj->unk0 = 8;
        obj++;
    } while (--count != 0);
}
