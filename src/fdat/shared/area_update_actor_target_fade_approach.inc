/*
 * Slot 17 (shared fragment, FDAT_077.BIN's claim): script record 0x85 of the
 * current actor. Its light blend (+0x16) is raised to 0xfff (phase 0), it
 * moves toward the player while further than the record's range (phase 1)
 * and the blend is lowered again (phase 2); the including module defines
 * FADE_APPROACH_BLEND_STEP, the blend change per frame.
 */
ADDRESS(0x801e87a0, 0x350)
void area_update_actor_target_fade_approach(void)
{
    Actor *obj;
    ActorGroup *def;
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

    obj = actor_status.current;
    def = actor_status.current_group;
    script = obj->target;
    if (obj->target_type != 0x85) {
        return;
    }
    if (obj->target_action_state == 0) {
        obj->target_action_state = 0xf0;
        obj->lighting_blend = -12000;
        obj->lighting_override_index = 0x32;
        obj->target_phase = 0;
        obj->unk72.vy = 0;
        obj->animation_phase = 0;
        obj->render_mode = 7;
        actor_set_animation(script[1]);
    }
    dx = cur_avatar_status.pos.vx - obj->pos.vx;
    dz = cur_avatar_status.pos.vz - obj->pos.vz;
    angle = ratan2(dz, dx);
    step_x = ((u16 *)script)[6] * rcos(angle) >> 12;
    step_z = ((u16 *)script)[6] * rsin(angle) >> 12;
    obj->movement_yaw = vector_xz_to_angle(cur_avatar_status.pos.vx - obj->pos.vx,
                                           cur_avatar_status.pos.vz - obj->pos.vz);
    mode = 5;
    if (obj->unk72.vy == 0) {
        turned = actor_turn_and_move_along_heading(obj->movement_yaw, ((u16 *)script)[6], ((u16 *)script)[7],
                                                   def->unk1[2], def->unk1[3], mode);
    } else {
        turned = actor_move_along_heading(obj->movement_yaw + 2048, ((u16 *)script)[6], def->unk1[2], mode);
    }
    if (turned != 0) {
        obj->unk72.vy = obj->unk72.vy == 0;
    }
    dist = dx * dx + dz * dz;
    range = ((u16 *)script)[10] * ((u16 *)script)[10];
    switch (obj->target_phase) {
    case 0:
        obj->lighting_blend = obj->lighting_blend < 4095 ? obj->lighting_blend + FADE_APPROACH_BLEND_STEP : 0xfff;
        actor_advance_animation_clamped(obj, ((u16 *)script)[4]);
        if (dist > 0x19000000) {
            obj->target_phase = 1;
            obj->animation_phase = 0;
        }
        if (obj->animation_phase >= 4095) {
            obj->target_phase = 1;
            obj->animation_phase = 0;
        }
        break;
    case 1:
        actor_advance_animation_clamped(obj, ((u16 *)script)[4]);
        obj->pos.vx += step_x;
        obj->pos.vz += step_z;
        obj->lighting_blend += (rand() >> 6) - 256;
        if (dist < range) {
            obj->target_phase = 2;
            obj->animation_phase = 0;
        }
        if (obj->animation_phase >= 4095) {
            obj->target_phase = 2;
            obj->animation_phase = 0;
        }
        break;
    case 2:
        actor_advance_animation_clamped(obj, ((u16 *)script)[4]);
        obj->lighting_blend -= FADE_APPROACH_BLEND_STEP;
        if (obj->animation_phase >= 4095 && obj->lighting_blend <= 1024) {
            obj->lighting_override_index = 0x37;
            obj->render_mode = 0;
            obj->lighting_blend = 0x800;
            actor_reset_target_and_reselect();
        }
        break;
    default:
        return;
    }
    actor_damp_horizontal_motion(def->unk1[2], 10);
}
