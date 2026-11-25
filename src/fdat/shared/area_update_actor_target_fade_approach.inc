/*
 * Slot 17 (shared fragment, FDAT_077.BIN's claim): script record 0x85 of the
 * current object record. It rises (phase 0), moves toward the player while
 * further than the record's range (phase 1) and sinks again (phase 2); the
 * including module defines FDAT_801E87A0_STEP, the rise and sink per frame.
 */
ADDRESS(0x801e87a0, 0x350)
void area_update_actor_target_fade_approach(void)
{
    Object88 *obj;
    Object78 *def;
    u8 *script;
    s32 dx;
    s32 dz;
    s16 angle;
    s32 step_x;
    s32 step_z;
    s32 turned;
    s32 dist;
    s32 range;
    s32 mode;

    obj = DAT_80185da8.current;
    def = DAT_80185da8.def;
    script = obj->unk60;
    if (obj->unke != 0x85) {
        return;
    }
    if (obj->unkf == 0) {
        obj->unkf = 0xf0;
        obj->unk16 = -12000;
        obj->unk14 = 0x32;
        obj->unk70 = 0;
        obj->unk72.vy = 0;
        obj->unk18 = 0;
        obj->unk13 = 7;
        actor_set_animation(script[1]);
    }
    dx = DAT_801b24e4.pos.vx - obj->pos.vx;
    dz = DAT_801b24e4.pos.vz - obj->pos.vz;
    angle = ratan2(dz, dx);
    step_x = ((u16 *)script)[6] * rcos(angle) >> 12;
    step_z = ((u16 *)script)[6] * rsin(angle) >> 12;
    obj->unk64 = vector_xz_to_angle(DAT_801b24e4.pos.vx - obj->pos.vx, DAT_801b24e4.pos.vz - obj->pos.vz);
    mode = 5;
    if (obj->unk72.vy == 0) {
        turned = actor_turn_and_move_along_heading(obj->unk64, ((u16 *)script)[6], ((u16 *)script)[7],
                                                   def->unk1[2], def->unk1[3], mode);
    } else {
        turned = actor_move_along_heading(obj->unk64 + 2048, ((u16 *)script)[6], def->unk1[2], mode);
    }
    if (turned != 0) {
        obj->unk72.vy = obj->unk72.vy == 0;
    }
    dist = dx * dx + dz * dz;
    range = ((u16 *)script)[10] * ((u16 *)script)[10];
    switch (obj->unk70) {
    case 0:
        obj->unk16 = obj->unk16 < 4095 ? obj->unk16 + FDAT_801E87A0_STEP : 0xfff;
        actor_advance_animation_clamped(obj, ((u16 *)script)[4]);
        if (dist > 0x19000000) {
            obj->unk70 = 1;
            obj->unk18 = 0;
        }
        if (obj->unk18 >= 4095) {
            obj->unk70 = 1;
            obj->unk18 = 0;
        }
        break;
    case 1:
        actor_advance_animation_clamped(obj, ((u16 *)script)[4]);
        obj->pos.vx += step_x;
        obj->pos.vz += step_z;
        obj->unk16 += (rand() >> 6) - 256;
        if (dist < range) {
            obj->unk70 = 2;
            obj->unk18 = 0;
        }
        if (obj->unk18 >= 4095) {
            obj->unk70 = 2;
            obj->unk18 = 0;
        }
        break;
    case 2:
        actor_advance_animation_clamped(obj, ((u16 *)script)[4]);
        obj->unk16 -= FDAT_801E87A0_STEP;
        if (obj->unk18 >= 4095 && obj->unk16 <= 1024) {
            obj->unk14 = 0x37;
            obj->unk13 = 0;
            obj->unk16 = 0x800;
            actor_reset_target_and_reselect();
        }
        break;
    default:
        return;
    }
    actor_damp_horizontal_motion(def->unk1[2], 10);
}
