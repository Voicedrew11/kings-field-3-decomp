/*
 * FDAT_026.BIN (WIP module): the handler table, the shared slot 0 handler,
 * a facing check (slot 1), an item handler (slot 2), empty handlers, a
 * record handler (slot 19), an event handler (slot 4) and the load handler
 * (slot 5).
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/render.h>
#include <fdat/fdat.h>

RODATA(0x801e8308, 0x88)

void area_handler_noop(void);
void area_update_respawn_actor_out_of_view(void);
void area_use_item_flash_and_set_flag(VECTOR *pos, SVECTOR *angle, s32 item);
void area_adjust_actor_magic_damage_noop(void);
void area_on_actor_removed_disable_actor(Actor *obj);
void area_region_trigger_noop(void);
void area_init_map_object_noop(void);
void area_update_map_object_noop(void);
void area_actor_target_check_noop(void);
void area_update_actor_target_noop(void);
void area_on_item_added_noop(void);
void area_talk_event_grant_magic(Actor *record, s32 event);
void area_on_load_restore_actor_and_item_flags(void);

const FdatHeader fdat_header = {
    0xd,
    {
        /*  0 */ area_interact_item_containers,
        /*  1 */ area_update_respawn_actor_out_of_view,
        /*  2 */ area_use_item_flash_and_set_flag,
        /*  3 */ area_region_trigger_noop,
        /*  4 */ area_talk_event_grant_magic,
        /*  5 */ area_on_load_restore_actor_and_item_flags,
        /*  6 */ area_on_item_added_noop,
        /*  7 */ area_handler_noop,
        /*  8 */ area_init_map_object_noop,
        /*  9 */ area_update_map_object_noop,
        /* 10 */ area_handler_noop,
        /* 11 */ area_handler_noop,
        /* 12 */ area_handler_noop,
        /* 13 */ area_handler_noop,
        /* 14 */ area_handler_noop,
        /* 15 */ area_handler_noop,
        /* 16 */ area_actor_target_check_noop,
        /* 17 */ area_update_actor_target_noop,
        /* 18 */ area_adjust_actor_magic_damage_noop,
        /* 19 */ area_on_actor_removed_disable_actor,
        /* 20 */ area_handler_noop,
        /* 21 */ area_handler_noop,
        /* 22 */ area_handler_noop,
        /* 23 */ area_handler_noop,
        /* 24 */ area_handler_noop,
        /* 25 */ area_handler_noop,
        /* 26 */ area_handler_noop,
        /* 27 */ area_handler_noop,
        /* 28 */ area_handler_noop,
        /* 29 */ area_handler_noop,
        /* 30 */ area_handler_noop,
        /* 31 */ area_handler_noop,
    },
    0x2d,
};

ADDRESS(0x801e8390, 0x8)
void area_handler_noop(void)
{
}

#include "shared/area_interact_item_containers.inc"

/* Slot 1: with flag 0x30 alone set, record 1 stops when it faces away from the player. */
ADDRESS(0x801e85a0, 0xa4)
void area_update_respawn_actor_out_of_view(void)
{
    s32 i;

    if (cur_evt_state.unk2c[4] != 1 || cur_evt_state.unk32[0] != 0) {
        return;
    }
    i = 1;
    if (actor_status.objects[i].lifecycle == 1) {
        return;
    }
    if (angle_within_tolerance(cur_avatar_status.unk124.vy,
                               vector_xz_to_angle(actor_status.objects[i].pos.vx - cur_avatar_status.pos.vx,
                                                  actor_status.objects[i].pos.vz - cur_avatar_status.pos.vz),
                               600) == 0) {
        actor_status.objects[i].lifecycle = 0;
        actor_status.objects[i].slot_state = 2;
    }
}

/* Slot 2: item 0x81 used on object 0x34 in front fades the screen and sets flag 0x30. */
ADDRESS(0x801e8644, 0x134)
void area_use_item_flash_and_set_flag(VECTOR *pos, SVECTOR *angle, s32 item)
{
    s32 i;

    if (item != 0x81) {
        return;
    }
    if (cur_evt_state.unk2c[4] != 0) {
        return;
    }
    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 0x6a4, angle->vy, 0x200)) != -1; i++) {
        if (i == 0x34) {
            color_overlay_transition(0x200, 0, 0, 0, 0x80, 0x80, 200);
            color_overlay_transition(0x200, 0x80, 0x80, 200, 0, 0, 0);
            set_accumulated_color_overlay(0, 0, 0, 0, 1);
            cur_evt_state.unk2c[4] = 1;
            game_counter_decrement(item);
            cur_evt_state.unk0 = 1;
        }
    }
}

ADDRESS(0x801e8778, 0x8)
void area_adjust_actor_magic_damage_noop(void)
{
}

/* Slot 19: once flag 0x30 is set, actor 1 is disabled (state 8) when removed. */
ADDRESS(0x801e8780, 0x68)
void area_on_actor_removed_disable_actor(Actor *obj)
{
    if (obj - actor_status.objects == 1) {
        if (cur_evt_state.unk2c[4] == 1) {
            cur_evt_state.unk2c[4] = 2;
            actor_status.objects[1].slot_state = 8;
        }
    }
}

ADDRESS(0x801e87e8, 0x8)
void area_region_trigger_noop(void)
{
}

ADDRESS(0x801e87f0, 0x8)
void area_init_map_object_noop(void)
{
}

ADDRESS(0x801e87f8, 0x8)
void area_update_map_object_noop(void)
{
}

ADDRESS(0x801e8800, 0x8)
void area_actor_target_check_noop(void)
{
}

ADDRESS(0x801e8808, 0x8)
void area_update_actor_target_noop(void)
{
}

ADDRESS(0x801e8810, 0x8)
void area_on_item_added_noop(void)
{
}

/* Slot 4: script events 0 and 1 of an object record. */
ADDRESS(0x801e8818, 0x100)
void area_talk_event_grant_magic(Actor *record, s32 event)
{
    switch (event) {
    case 0:
        color_overlay_transition(0x200, 0, 0, 0, 0x80, 200, 0x80);
        color_overlay_transition(0x200, 0x80, 200, 0x80, 0, 0, 0);
        cur_evt_state.unk31 = 1;
        effect_status.unk0[54].menu_available = 1;
        effect_status.unk0[55].menu_available = 1;
        effect_status.unk0[66].menu_available = 1;
        effect_status.unk0[67].menu_available = 1;
        effect_status.unk0[68].menu_available = 1;
        effect_status.unk0[69].menu_available = 1;
        effect_status.unk0[70].menu_available = 1;
        effect_status.unk0[71].menu_available = 1;
        effect_status.unk0[72].menu_available = 1;
        effect_status.unk0[73].menu_available = 1;
        effect_status.unk0[74].menu_available = 1;
        effect_status.unk0[75].menu_available = 1;
        break;
    case 1:
        cur_evt_state.unk32[0] = 1;
        actor_status.objects[1].slot_state = 8;
        break;
    }
}

/* Slot 5, called after the module loads. */
ADDRESS(0x801e8918, 0xb0)
void area_on_load_restore_actor_and_item_flags(void)
{
    if (cur_evt_state.unk2c[4] != 0 && cur_evt_state.unk32[0] == 0) {
        actor_status.objects[1].slot_state = 1;
    } else {
        actor_status.objects[1].slot_state = 8;
    }
    if (game_counter_is_nonzero(0x85) > 0) {
        cur_evt_state.unk32[1] = 1;
    } else {
        cur_evt_state.unk32[1] = 0;
    }
    if (game_counter_is_nonzero(0x10) > 0) {
        cur_evt_state.unk32[2] = 1;
    }
    if (game_counter_is_nonzero(0x7d) > 0) {
        cur_evt_state.unk32[3] = 1;
    }
    cur_evt_state.unk84[5] = 8;
}
