#ifndef GAME_DATA_H
#define GAME_DATA_H

/*
 * GAME.EXE globals outside the sound, CD and render headers (WIP), mostly
 * the BSS blocks the game loop clears word by word at start-up. Each extent
 * is the word count main passes to repeat_store_word; whether a block is one
 * object or a run of several is not yet known, so the names stay
 * address-derived and only the fields the matched code touches are named.
 * The block at 0x80199154 is GraphicsRuntime and the effect records at
 * 0x80182968 are MapPlacedEntry, both in game/render.h; the word at 0x801c12f0 is
 * CdState.unkc (game/cd.h).
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <kf/decenv.h>

/* PlayerState.unk188 in modes 3, 0x10, 0x11 and 0x12 (player_begin_moving_damage_reaction). */
typedef struct PlayerDamageReaction {
    SVECTOR vel;    /* +0x0 */
    SVECTOR angular_vel; /* +0x8: added to unk134..unk138 (player_update_reaction_rotation_offsets) */
} PlayerDamageReaction;

/* PlayerState.unk188 in modes 7, 8 and 9 (player_begin_movie_approach). */
typedef struct PlayerMovieReaction {
    u8 movie_id;    /* +0x0 */
    u8 step;
    SVECTOR start_rotation; /* +0x2: copy of PlayerState.unk12c */
} PlayerMovieReaction;

/* PlayerState.unk188: one view per unk101 mode (mode 5 counts in count). */
typedef union PlayerReaction {
    PlayerDamageReaction damage;
    PlayerMovieReaction movie;
    s16 bob_phase;
} PlayerReaction;

typedef struct PlayerState {
    s32 experience; /* +0x00: the save title shows it as six digits */
    s32 next_level_experience;
    s32 weapon_growth_experience;
    u8 level;       /* +0x0c: the save title shows it as two digits */
    u8 unkd;
    u8 force_actor_lifecycle_refresh; /* +0x0e: cleared every frame by the game loop */
    u8 weapon_charge_delay;
    u8 magic_charge_delay;
    u8 movement_speed_adjustment_decay_latch;
    s16 movement_speed_adjustment_q12;
    s16 damage_scale;
    u16 hp_max;     /* +0x16: 0..999, from the level table */
    u16 hp;         /* +0x18: 0 is death (player_adjust_hp_unclamped) */
    u16 mp_max;     /* +0x1a: maximum of unk1c; 0..999, from the level table */
    u16 mp;
    u16 attack_charge_current; /* +0x1e: out of 5000 */
    u16 attack_charge_committed; /* +0x20: copy of unk1e; scales the swing hit (player_update_weapon_attack) */
    u16 magic_charge; /* +0x22: out of 5000 */
    u16 physical_power_training; /* +0x24: counts up to 100 for a point of unk32 */
    u16 magic_training[5]; /* +0x26: count up to 400 for a point of unk34[n] */
    s16 unk30;
    u16 base_physical_power; /* +0x32: player_recalculate_combat_stats copies it to unk40 */
    u16 base_magic[5]; /* +0x34: player_recalculate_combat_stats copies them to unk42 */
    s16 unk3e;
    u16 physical_power; /* +0x40: 0..999 */
    u16 magic[5];   /* +0x42: 0..999 each */
    u16 wisdom;     /* +0x4c: the menu shows it plus one (menu_draw_player_status, lhu) */
    u8 unk4e[0x2];
    s32 gold;
    u16 attack_components[7]; /* +0x54: player_recalculate_combat_stats rebuilds them; never above 0x7fff */
    u16 attack_water;
    u8 unk64[0x2];
    u16 defense_slash; /* +0x66..+0x76: player_recalculate_combat_stats rebuilds them from Record20s */
    u16 defense_blow;
    u16 defense_stab;
    u16 defense_holy;
    u16 defense_fire;
    u16 defense_earth;
    u16 defense_wind;
    u16 defense_water;
    u16 poison_resistance;
    s16 poison_timer;
    s16 curse_strength;
    s16 curse_phase_limit;
    s16 darkness_phase;
    s16 darkness_phase_limit;
    s16 slow_timer;
    s16 paralysis_timer;
    s16 fire_defense_boost_timer;
    s16 damage_immunity_timer;
    s16 physical_defense_boost_timer;
    s16 magic_tint_phase;
    s16 magic_tint_phase_limit;
    s16 map_marker_visual_effect_timer;
    s16 full_mp_timer;
    s16 magic_boost_timer;
    u8 unk96[0x6];
    u32 equipment_effect_ticks; /* +0x9c: frame counter (player_apply_equipment_hp_tick periods) */
    s32 unka0;
    u32 play_minutes; /* +0xa4: the VSync handler adds one every 3600 VSyncs */
    u16 *magic_attack_mask_cursor; /* +0xa8: next pad bit of the player_magic_attack_masks sequence (player_update_actions_and_charge) */
    struct MagicRecord *selected_magic_record; /* +0xac: &effect_status.unk0[id] (player_dispatch_weapon_magic) */
    struct WeaponRecord *equipped_weapon_record; /* +0xb0: &weapon_table[unkcb] */
    u8 *weapon_asset_buffer; /* +0xb4 */
    s32 weapon_animation_cache;
    struct Effect *weapon_effect; /* +0xbc: arrow of a bow shot (player_update_weapon_attack) */
    s16 weapon_attack_phase; /* +0xc0: -1 blocks the Actor script records of ids 19 and 20 */
    s16 weapon_attack_window; /* +0xc2: swing time of the next hit (player_update_weapon_attack) */
    s16 weapon_next_sound_phase; /* +0xc4: swing time of the next swing sound */
    u8 weapon_magic_shots_remaining;
    u8 primary_magic_shortcut_id;
    u8 secondary_magic_shortcut_id;
    u8 secondary_item_shortcut_id;
    u8 weapon_attack_mode;
    u8 equipped_weapon_id;
    u8 unkcc;
    u8 unkcd[0x1];
    u8 weapon_magic_shots_configured;
    u8 weapon_attack_fully_charged;
    u8 weapon_guard_active;
    u8 unkd1[0x3];
    struct EquipmentRecord *equipped_head_record; /* +0xd4..+0xec: &map_world.unk11c14[] of unkf0..unkf6 */
    struct EquipmentRecord *equipped_body_record;
    struct EquipmentRecord *equipped_shield_record;
    struct EquipmentRecord *equipped_arm_record;
    struct EquipmentRecord *equipped_leg_record;
    struct EquipmentRecord *equipped_accessory_record;
    struct EquipmentRecord *equipped_extra_record;
    u8 equipped_head_id; /* +0xf0..+0xf6: EquipmentRecord indices, 0xff for none */
    u8 equipped_body_id;
    u8 equipped_shield_id;
    u8 equipped_arm_id;
    u8 equipped_leg_id;
    u8 equipped_accessory_id;
    u8 equipped_extra_id;
    u8 se_volume;   /* +0xf7: 0 mutes sound effects; scales them by n/128 */
    u8 bgm_volume;  /* +0xf8: 0 keeps the sequence off */
    u8 vitals_display; /* +0xf9: status overlay layout, bits 0-1 show its two halves */
    u8 compass_display; /* +0xfa: compass, bits 0-1 show its needles */
    u8 item_preview_enabled;
    u8 walking_bob_enabled;
    u8 brightness;  /* +0xfd: brightness of the status overlay */
    u8 button_style;
    u8 button_layout;
    u8 panel_translucent;
    u8 reaction_mode;
    u8 unk102[0x2];
    u8 vertical_motion_state;
    u8 queued_magic_id;
    u8 queued_magic_casts_remaining;
    u8 queued_magic_repeat_interval;
    u8 queued_magic_countdown;
    u8 fatal_fall_latch;
    u8 warp_arrived;
    u8 unk10b[0x1];
    VECTOR pos;     /* +0x10c: translate_active_world_positions moves it with the objects */
    SVECTOR frame_displacement; /* +0x11c */
    SVECTOR camera_rotation; /* +0x124 */
    SVECTOR camera_rotation_target; /* +0x12c */
    s16 reaction_rotation_offset_x;
    s16 reaction_rotation_offset_y;
    s16 reaction_rotation_offset_z;
    s16 death_transition_frame;
    s16 view_rotation_offset_x;
    s16 view_rotation_offset_y;
    s16 view_rotation_offset_z;
    u8 unk142[0x2];
    s16 vertical_motion_pitch_offset;
    s16 camera_yaw_offset;
    s16 camera_roll_offset;
    u8 unk14a[0x2];
    SVECTOR magic_origin_offset; /* +0x14c: player_probe_view_target_and_vectors rotates it by unk124 */
    s32 collision_lower_clearance; /* +0x154: top edge of the full-screen overlay pair; >= 240 hides it */
    s32 collision_upper_clearance; /* +0x158: top edge of a single overlay; >= 240 hides it */
    s32 collision_second_upper_clearance; /* +0x15c: top edge of a single overlay; >= 240 hides it */
    u16 map_layer_offset;
    s16 strafe_velocity;
    s16 forward_velocity;
    s16 movement_speed;
    s16 yaw_step;
    s16 pitch_step;
    s16 camera_vertical_offset;
    s16 walking_bob_phase;
    s16 landing_vertical_offset;
    s16 vertical_velocity;
    s16 damage_red_overlay_scale;
    s16 damage_red_overlay_decay;
    u16 pad_buttons; /* +0x178: pad bits held (action_button_masks) */
    u16 pad_buttons_previous; /* +0x17a: pad bits held the frame before */
    u8 unk17c[0x4];
    s32 movement_step_limit;
    s32 turn_step_limit;
    PlayerReaction reaction; /* +0x188: parameters of the unk101 mode */
    u8 unk198[0x4];
} PlayerState;

typedef struct EventState {
    s32 interaction_handled;
    u8 flags[9];    /* +0x04: script bytes from here (event_target_stream_resolve_cursor tests
                     * unk4[n]); [0] the highest area entered (resource_request_transition) */
    u8 unkd;       /* +0x0d: the game loop sets it to 1 on a continued game */
    u8 unke[0x1d];
    u8 unk2b;
    u8 unk2c[5];
    u8 unk31;
    u8 unk32[0x2f];
    u8 unk61;       /* +0x61: 1: the movie ends at start position 12 (movie_update_state) */
    u8 unk62[3];
    u8 unk65;
    u8 unk66[3];
    u8 ending_state; /* +0x69: 1 when the ending is due (game_update_ending sets 2) */
    u8 ending_variant; /* +0x6a: picks the ending (game_update_ending) */
    u8 unk6b[6];
    u8 element_unlocked[5]; /* +0x71: per magic element 0..3; element 4 needs none */
    u8 unk76[7];
    u8 first_kill_flag; /* +0x7d: set to 1 by the first Actor kill (actor_apply_magic_to_actor) */
    u8 kill_count;  /* +0x7e: Actor kills (actor_apply_magic_to_actor) */
    u8 element_magic_learned[5]; /* +0x7f: per magic element */
    u8 unk84[6];
    u8 area_visited[0x20]; /* +0x8a: per area, 1 once resource_request_transition loaded it */
    u8 stream_actor_definition_id; /* +0xaa: Actor.unk1 of the record whose script
                    * event_target_stream_resolve_cursor entered */
    u8 unkab[0x59];
    u8 arena[0x5000]; /* +0x104: block arena (game/memory.h), cleared by event_state_initialize */
    u16 saved_offsets[32]; /* +0x5104: arena offsets of 32 pointers, 0xffff for NULL (event_saved_offsets_encode) */
} EventState;

/* 0x20-byte records player_add_equipment_bonuses adds to the player's halfwords +0x66..+0x76. */
typedef struct EquipmentRecord {
    u8 unk0[2];
    u16 defense_slash;
    u16 defense_blow;
    u16 defense_stab;
    u16 poison_resistance;
    u16 defense_holy;
    u16 defense_fire;
    u16 defense_earth;
    u16 defense_wind;
    u16 defense_water;
    u16 hp_regen_interval; /* +0x14..+0x1a: frame periods of the hp/unk1c gain and loss */
    u16 hp_drain_interval;
    u16 mp_regen_interval;
    u16 mp_drain_interval;
    u8 unk1c[0x4];
} EquipmentRecord;

/*
 * One layer of a map cell; render_map_cell_object draws one (field names WIP).
 */
typedef struct MapLayer {
    u8 object_index; /* +0x00: below 240 the layer is drawn; 0xfd..0xff are walls */
    u8 elevation;   /* +0x01: floor height / -128 */
    u8 quarter_turns; /* +0x02: layer 0: bits 2-7 count the Actor records on the
                     * cell (map_cell_add_layer_occupancy) */
    u8 collision_shape_id;
    u8 lighting_index;
} MapLayer;

/*
 * One 2048-unit square of the area map (80 x 80 of them start map_world,
 * indexed [z >> 11][x >> 11]): layer 0 for area 1, layer 1 for area 2.
 */
typedef struct MapCell {
    MapLayer layer[2];
} MapCell;

typedef struct MapWorld {
    MapCell cells[80][80];
    u8 unkfa00[0x600];
    u8 shape_bank[0x1c14]; /* +0x10000: resource_advance_transition copies an area file's second table
                          * (0x2000 bytes, through unk11ff4) here */
    EquipmentRecord equipment_records[31]; /* +0x11c14: WIP count */
    u8 unk11ff4[0xc];
    MapCell *collision_cell; /* +0x12000: cell of the last collision test */
    u8 *collision_layer; /* +0x12004: record of the last collision test; [2] bits 0-1 its slope */
    u8 unk12008[2];
    u16 collision_layer_offset; /* +0x1200a: copied to SoundState.unk24 with the listener */
    s32 collision_layer_height; /* +0x1200c: height given to Actor records with flag 0x400 */
    s32 collision_floor_height; /* +0x12010: floor height of the last collision test */
    s32 collision_height_limit; /* +0x12014: second height of the last collision test (collision_evaluate_shape_records);
                     * player_move_horizontal lets the player through when it is 1700 above unk12010 */
    s32 collision_bounds[3]; /* +0x12018: heights player_update_collision_bounds measures the player against; [0] the
                      * floor the Effect spawns of effect_spawn_at_lower_bound must reach */
    u32 collision_hit_flags; /* +0x12024: hit bits of the last collision_query_world test */
    s32 collision_actor_index; /* +0x12028: Actor index it hit, -1 for none (effect_dispatch_magic_impact reads it) */
    s32 collision_object_index; /* +0x1202c: MapObject index it hit, -1 for none */
    u8 unk12030[4];
    s32 collision_slope_push; /* +0x12034: push of the last collision test (collision_evaluate_shape_records); from 64
                     * up player_move_horizontal moves the player by it */
    s32 collision_slope_angle; /* +0x12038: angle of that push */
    VECTOR collision_hit_position; /* +0x1203c: point of the last wall hit (collision_cache_load_hit_bounds) */
    u16 collision_hit_radius; /* +0x1204c: radius of what was hit (collision_cache_load_hit_bounds) */
    u16 collision_hit_height; /* +0x1204e: height of what was hit */
    u16 collision_hit_half_width;
    u8 unk12052[2];
} MapWorld;

/* An in-game movie record (WIP: 52 bytes; movie_state holds 16, indexed by movie id). */
typedef struct MovieRecord {
    u8 file;        /* +0x00: STR file number; 16 and up: a random DRM file */
    u8 skip_flag;   /* +0x01: script flag that skips the movie when set (0x80: when clear),
                     * 0xff for none */
    u16 last_frame; /* +0x02: last frame */
    s16 approach_angle_x; /* +0x04..+0x08: view angles the player turns to before the movie
                     * (player_update_frame mode 7), -1 to keep one */
    s16 approach_angle_y;
    s16 approach_angle_z;
    s16 fade_back;  /* +0x0a: nonzero fades tint unk22 back in after the movie */
    s32 approach_x; /* +0x0c..+0x14: position the player moves to before it, -1 to keep */
    s32 approach_y;
    s32 approach_z;
    u8 unk18[4];
    s16 return_angle_x; /* +0x1c..+0x20: view angles after the movie (mode 9), -1 to keep */
    s16 return_angle_y;
    s16 return_angle_z;
    s16 overlay_mode; /* +0x22: render_frames_with_color_overlay tint mode around the movie */
    s32 return_x;   /* +0x24..+0x2c: position after it, -1 to keep */
    s32 return_y;
    s32 return_z;
    u8 unk30[4];
} MovieRecord;

typedef struct MovieState {
    MovieRecord movies[16]; /* +0x000: by movie id (unk380 points into them) */
    DECENV dec;     /* +0x340: the in-game movie player's decode environment */
    s16 state;
    s16 stream_status; /* +0x372: 99 once the stream passed the movie's last frame */
    u_long *vlc_buffers[2]; /* +0x374: VLC buffers strSetDefDecEnv hands to dec */
    u_long *slice_buffer; /* +0x37c: slice buffer */
    MovieRecord *current_movie; /* +0x380: the record of movie unk384 */
    u8 movie_id;    /* +0x384: movie id, 0xff for none */
    u8 unk385[3];
    s32 stream_frame; /* +0x388: frame number of the last sector (strNext) */
} MovieState;

/*
 * Object records translate_active_world_positions moves when the area origin shifts (WIP: named
 * by size; 0xff/0xffff in the first field marks a free record). The 24-byte
 * records it also moves are the placed effects (MapPlacedEntry, game/render.h).
 */
/* MapObject +0x3a as the FDAT module handlers read it (WIP). */
typedef union MapObjectHalfword {
    u16 index;      /* slot 0: a linked object, 0xffff for none; kind 7 (map_object_refresh_cell_markers) */
    u8 bytes[2];    /* slot 3 compares bytes[1] with a per-module value; map_object_set_property reads
                     * bytes[0] and bytes[1] as angles */
} MapObjectHalfword;

/*
 * Per-kind state of an MapObject from +0x34 (WIP views, by MapObject.unk4): the
 * GAME and FDAT code read it as bytes or halfwords by kind; map_object_pool_reset
 * clears the first three words.
 */
typedef struct MapObjectBytes {
    u8 unk34[4];
    u8 unk38;       /* +0x38: radius scale, 0x80 = 1 (flag 0x10) */
    u8 unk39;       /* +0x39: event_spawn_effect_object stores the index - 124 of the record it spawns */
    MapObjectHalfword unk3a; /* +0x3a */
    u8 unk3c;       /* +0x3c: FDAT slot 0 acts only when set */
    u8 unk3d[2];
    u8 unk3f;       /* +0x3f: FDAT slot 0 shows this message when below 35 */
    u8 unk40;       /* +0x40: kind 0x70 driver: steps taken (map_object_step_offset_motion); FDAT slot 0
                     * copies it to the linked object's unk0 */
    u8 unk41[3];
} MapObjectBytes;

typedef struct MapObjectHalves {
    u16 unk34[2];
    u16 unk38;
    u16 unk3a;
    u16 unk3c;      /* +0x3c: age stamp map_object_effect_pool_acquire replaces the oldest by */
    u16 unk3e;      /* +0x3e: kind 0x1f: delay in units of 6 VSyncs; kinds 0/0x1b: the
                     * other wing's index, 0xffff for none */
    u16 unk40;      /* +0x40: kind 0x70: index of its driver (map_object_step_offset_motion); kinds 0/1/0x1b:
                     * frames since the door started opening */
    u16 base_yaw;   /* +0x42: kinds 0/1/0x1b: yaw when shut */
} MapObjectHalves;

/* Signed views of the same bytes and halfwords (kinds 6, 0x60..0x62, 0xe0, 0xea). */
typedef struct MapObjectSBytes {
    s8 unk34[4];
    s8 unk38;
    s8 unk39;
    s8 unk3a;
    s8 unk3b;
    s8 cell_dx;     /* +0x3c: kind 0xea: x shift in map cells (translate_player_effects_and_actors) */
    s8 cell_dz;     /* +0x3d: kind 0xea: z shift */
    s8 unk3e;
    s8 unk3f;
    s8 shift_x;     /* +0x40: kind 0xe0: shift passed to resource_request_transition (with unk41, unk42) */
    s8 shift_z;
    s8 shift_y;
    s8 unk43;
} MapObjectSBytes;

typedef struct MapObjectSHalves {
    s16 unk34[2];
    s16 unk38;
    s16 unk3a;
    s16 unk3c;
    s16 motion_velocity; /* +0x3e: kinds 0x60..0x62: fall speed (map_object_spawn_effect,
                     * map_object_spawn_scattered_effect) */
    s16 unk40;      /* +0x40: kind 6: turn speed (angle_velocity_step) */
    s16 base_yaw;
} MapObjectSHalves;

typedef struct MapObjectWords {
    s32 animation_cache;
    s32 unk38;
    s32 unk3c;
    s32 unk40;      /* +0x40: kind 0x10: height it started from */
} MapObjectWords;

/* Kinds render_scene_and_update_resources draws with a model (0xf2 and the
 * default kinds): pose slot first. */
typedef struct MapObjectModel {
    struct AnimSlot *animation_cache; /* +0x34: pose slot (animation_prepare_asset_vertices) */
    u8 radius_x;    /* +0x38: kind 0xf2: x range in map cells it is drawn from */
    u8 radius_z;    /* +0x39: z range */
    s16 cell_offset_x; /* +0x3a: kind 0xf2: x offset of the drawn position in map cells */
    s16 cell_offset_z; /* +0x3c: z offset */
    u8 cell_height; /* +0x3e: height of the drawn position in map cells */
    u8 unk3f[5];
} MapObjectModel;

/* Kind 0xf0, drawn in view space by render_animated_object. */
typedef struct MapObjectAnimated {
    struct AnimSlot *animation_cache; /* +0x34: pose slot */
    u8 radius_x;    /* +0x38: x range in map cells it is drawn from */
    u8 radius_z;    /* +0x39: z range */
    u8 depth_code;  /* +0x3a: depth below 0x1fff */
    u8 lighting_index; /* +0x3b: light record */
    u8 blend_mode;  /* +0x3c: shade */
    u8 unk3d[7];
} MapObjectAnimated;

/* Kind 0x1f: a sound played by distance from the player. */
typedef struct MapObjectAmbientSound {
    u8 unk34[4];
    u8 region_width; /* +0x38: half width (1024 units) */
    u8 region_depth; /* +0x39: half depth */
    u8 sound_id;    /* +0x3a: sound effect - 496 */
    u8 maximum_volume; /* +0x3b: volume */
    u8 fade_distance; /* +0x3c: fade-in distance (2048 units) */
    u8 vertical_attenuation_flags; /* +0x3d: bit 0 fades by height too */
    u16 repeat_delay_units; /* +0x3e: interval in 6-frame units */
    s32 next_sound_frame; /* +0x40: frame of the next play */
} MapObjectAmbientSound;

typedef union MapObjectArg {
    MapObjectBytes b;
    MapObjectHalves h;
    MapObjectWords w;
    MapObjectSBytes sb;
    MapObjectSHalves sh;
    MapObjectModel m;
    MapObjectAnimated v;
    MapObjectAmbientSound s;
} MapObjectArg;

typedef struct MapObject {
    u8 layer_mask;  /* +0x00: area (1 or 2): picks the map cell layer */
    u8 animation_id; /* +0x01: 0x80 after set-up */
    u8 render_mode;
    u8 flags;       /* +0x03: flags; 0x10 scales its radius by arg.b.unk38, 0x4 takes MapObjectDef.unke/unk10 */
    u8 action;      /* +0x04: kind, 0xff when free */
    u8 lighting_override_index;
    u16 object_id;  /* +0x06: index of its MapObjectDef; 0xff when free */
    u16 action_state; /* +0x08: FDAT_023 clears it when the key is used */
    u16 animation_phase; /* +0x0a: FDAT_017 tests it for 0 or >= 4095 */
    u16 collision_height; /* +0x0c: reach below its position (collision tests) */
    s16 render_depth_offset;
    u16 lighting_blend; /* +0x10: FDAT_047 raises it by 256 a frame up to 0xfff; light
                     * blend toward light2 (render_world_model) */
    u8 unk12[2];
    VECTOR pos;     /* +0x14 */
    SVECTOR angle;  /* +0x24 (scene_pose_interpolate) */
    SVECTOR scale;  /* +0x2c: 0x1000 = 1; FDAT_017 clears vx, vy and vz */
    MapObjectArg arg; /* +0x34 */
} MapObject;

/*
 * MapObjectDef +0x0c..+0x17 by type (WIP views): most types read bytes; the
 * moving types 0x51, 0x52 and 0x55 (map_object_update_actions) and the collision tests read
 * +0x0c..+0x11 as halfwords.
 */
typedef struct MapObjectDefBytes {
    u8 unkc;        /* +0x0c: map layer byte kind 7 sets, 0xff for none; kind 0x51: first
                     * vertex map_object_sample_world_vertex tests */
    u8 unkd;        /* +0x0d: kind 0x51: second vertex */
    u8 unke;
    u8 sound_id;    /* +0x0f: sound effect when it starts moving (map_object_play_spatial_sound) */
    u8 unk10[2];
    u8 impact_magic_values[4]; /* +0x12: kinds 0x51/0x52/0x55: passed to effect_dispatch_magic_impact on a hit */
    u8 collision_sound_id; /* +0x16: kinds 0x51/0x52/0x55: sound effect (- 496), 0xff for none */
    u8 unk17;       /* +0x17: map layer byte kinds 0x51/0x52 set; kind 0x55: speed / 16 */
} MapObjectDefBytes;

typedef struct MapObjectDefHalves {
    u16 vertex_index; /* +0x0c: kinds 0x52/0x55: vertex map_object_sample_world_vertex tests */
    u16 unke;       /* +0x0e: collision radius */
    u16 unk10;      /* +0x10: collision height */
    u8 unk12[6];
} MapObjectDefHalves;

/* Signed view of the same halfwords: the pick-up offsets of the item kinds (event_map_object_interact). */
typedef struct MapObjectDefSHalves {
    s16 height_offset; /* +0x0c: items: height offset of the held position
                     * (scene_position_from_camera_offset dy) */
    s16 depth_offset; /* +0x0e: items: distance of the held position (dz) */
    s16 held_depth_offset; /* +0x10: items: MapObject.unke while held */
    u8 unk12[6];
} MapObjectDefSHalves;

typedef union MapObjectDefArg {
    MapObjectDefBytes b;
    MapObjectDefHalves h;
    MapObjectDefSHalves sh;
} MapObjectDefArg;

/* The 0x18-byte records MapObject.unk6 indexes (WIP). */
typedef struct MapObjectDef {
    u8 action;      /* +0x00: type (map_object_set_property) */
    u8 kind;        /* +0x01: picks the kind map_object_spawn_effect gives a spawned record */
    u8 vab_resource_index;
    u8 flags;       /* +0x03: MapObject.unk3 flags at set-up */
    u16 collision_radius; /* +0x04: radius */
    u16 interaction_radius; /* +0x06: radius of the turned kinds (map_object_find_interaction_target) */
    u16 collision_height; /* +0x08: height */
    u16 initial_render_depth_offset;
    MapObjectDefArg arg; /* +0x0c */
} MapObjectDef;

/* One of the 24 positions kinds 5 and 16 record behind them (effect_construct_record). */
typedef struct EffectTrailRow {
    VECTOR pos;
    SVECTOR angle;
} EffectTrailRow;

/* Effect+0x44..+0x4b: effect_construct_record stores bytes, halfwords or words there by id (WIP views). */
typedef struct EffectBytes {
    u8 unk44;
    u8 unk45;
    u8 unk46[2];
    u8 trail_frame_index; /* +0x48: ids 5 and 16: cursor into their 24 trail records */
    u8 trail_phase_counter; /* +0x49: ids 5 and 16: step counter of the current phase */
    u8 trail_actor_index; /* +0x4a: ids 5 and 16: index of the Actor record they hold */
    u8 unk4b;
} EffectBytes;

/* Signed views of the payload: effect_update_dispatch reads some ids' bytes with lb, halfwords with lh. */

typedef struct EffectSBytes {
    s8 unk44;
    s8 unk45;
    s8 unk46[2];
    s8 unk48;
    s8 unk49;
    s8 unk4a[2];
} EffectSBytes;

typedef struct EffectHalves {
    u16 unk44;      /* +0x44: a shot arrow keeps its starting height here (player_update_weapon_attack) */
    u16 unk46;
    u16 unk48;
    u16 unk4a;
} EffectHalves;

/* Signed halfword view (effect_update_dispatch). */
typedef struct EffectSHalves {
    s16 unk44;
    s16 unk46;
    s16 unk48;
    s16 unk4a;
} EffectSHalves;

typedef struct EffectWords {
    EffectTrailRow *trail_rows; /* +0x44: ids 5 and 16: their 24 trail records */
    s32 origin_y;   /* +0x48: id 95: the height it was made at */
} EffectWords;

typedef union EffectArg {
    EffectBytes b;
    EffectSBytes c;
    EffectHalves h;
    EffectSHalves s;
    EffectWords w;
} EffectArg;

typedef struct Effect {
    u8 type;        /* +0x00: 0xff when free; bits 0-2 pick the collision mask,
                     * 0x40 averages the MagicRecord.unk5 stats (effect_magic_power) */
    u8 kind;        /* +0x01: index of its MagicRecord record */
    u8 base_render_id;
    u8 render_id;   /* +0x03: model id - 40 (effect_sample_rotated_vertex) */
    u8 animation_id; /* +0x04: animation */
    u8 unk5;
    u8 damage_multiplier_tenths;
    u8 phase;
    u8 render_flags; /* +0x08: both of bits 2 and 3 set keeps it in place */
    u8 render_mode;
    u8 layer_mask;
    u8 cooldown;    /* +0x0b: frames before the collision test (effect_probe_collision_by_type) */
    u8 lighting_override_index;
    u8 midpoint_collision_enabled;
    u16 clut_offset;
    s16 updates_remaining;
    s16 lighting_blend; /* +0x12: light blend toward light2 (render_world_model); id 6 compares it signed */
    u16 animation_phase; /* +0x14: animation time */
    u8 unk16[2];
    VECTOR pos;     /* +0x18 */
    SVECTOR angle;  /* +0x28 */
    SVECTOR scale;  /* +0x30: 0x1000 = 1 */
    SVECTOR vel;    /* +0x38 */
    struct AnimSlot *animation_cache; /* +0x40: pose slot (animation_prepare_asset_vertices) */
    EffectArg arg; /* +0x44: per-id payload (effect_construct_record) */
} Effect;

typedef struct Actor {
    u8 slot_state;  /* +0x00: 0xff when free */
    u8 definition_id; /* +0x01: ActorGroup.unk0 of its record */
    u8 group_index; /* +0x02: index of its ActorGroup record */
    u8 layer_mask;
    u8 unk4;
    u8 placement_flags; /* +0x05: bit 0 keeps its initial yaw */
    u8 home_layer_mask; /* +0x06: area passed to collision_sample_map_layer_height */
    u8 home_cell_z; /* +0x07: map cell z */
    u8 home_cell_x; /* +0x08: map cell x */
    u8 lifecycle;   /* +0x09: state, 1 active */
    u8 spawn_chance;
    u8 death_drop_object_id;
    u8 animation_id; /* +0x0c: animation */
    u8 vertical_motion_state;
    u8 target_type; /* +0x0e: id of the running script record, 0xff none */
    u8 target_action_state; /* +0x0f: script step, 0xff restart */
    u8 previous_target_type; /* +0x10: previous unke */
    u8 unk11;
    u8 unk12;
    u8 render_mode;
    u8 lighting_override_index; /* +0x14: second light record (render_world_model light2) */
    s8 render_depth_offset; /* +0x15: depth offset (render_world_model) */
    s16 lighting_blend; /* +0x16: FDAT slot 17 raises and lowers it (-12000 at the start) */
    u16 animation_phase; /* +0x18: animation time */
    u16 health;
    u16 collision_radius;
    u16 collision_height;
    u16 unk20;      /* +0x20: initial yaw */
    s16 unk22;      /* +0x22: z offset in its cell */
    s16 unk24;      /* +0x24: x offset in its cell */
    s16 vertical_anchor_offset; /* +0x26: height above the floor */
    u32 flags;      /* +0x28: flags (ActorGroup.unk34) */
    VECTOR pos;     /* +0x2c */
    s32 ballistic_origin_y;
    SVECTOR angle;  /* +0x40 */
    SVECTOR scale;  /* +0x48: 0x1000 = 1 */
    SVECTOR vel;    /* +0x50 */
    s16 turn_rate;
    u8 unk5a[2];
    struct AnimSlot *animation_cache; /* +0x5c: pose slot (animation_prepare_asset_vertices) */
    u8 *target;     /* +0x60: running script record */
    s16 movement_yaw; /* +0x64: FDAT slot 17 stores the angle to the player */
    s16 animation_step;
    s16 ballistic_horizontal_speed;
    s16 ballistic_launch_speed_y;
    s16 ballistic_acceleration;
    u8 unk6e[2];
    s16 target_phase; /* +0x70: script phase (FDAT slot 17: 0..2); script 5 keeps two
                     * byte flags here (ACTOR_B70/B71) */
    SVECTOR unk72;  /* +0x72: script direction; vx/vy also counters and indexes, pad a
                     * height (script 12); FDAT slot 17 toggles vy when the turn completes */
    u8 unk7a[6];
    s16 fade_out_phase; /* +0x80: phase of script 31 */
    u8 unk82[2];
    u8 hit_flash_phase; /* +0x84: cleared by a hit below the hurt threshold (actor_apply_magic_to_actor) */
    u8 unk85[3];
} Actor;

/*
 * The record an Actor is made from (its unk2 indexes actor_status.defs);
 * player_probe_view_target_and_vectors hands one to actor_find_target_of_type.
 */
typedef struct ActorGroup {
    u8 definition_id;
    u8 unk1[4];
    u8 vertical_acceleration; /* +0x05: fall acceleration */
    u8 landing_damage;
    u8 vab_resource_indices[2];
    u8 render_depth_offset;
    u8 activation_range_cells; /* +0x0a: wake range in map cells */
    u8 deactivation_range_cells; /* +0x0b: sleep range in map cells */
    s16 position_offset_x; /* +0x0c: offset of the record's point (Actor.unk28 bits 0-1) */
    s16 position_offset_y;
    s16 position_offset_z;
    u16 collision_radius;
    u16 collision_height;
    s16 player_facing_tolerance; /* +0x16: yaw range of its front (player_update_weapon_attack) */
    s16 actor_facing_tolerance; /* +0x18: yaw range (effect_dispatch_magic_impact) */
    u16 unk1a;
    u16 default_vertical_anchor_offset;
    s16 experience_reward; /* +0x1e: passed to player_add_experience when the record dies */
    u16 defense_components[8]; /* +0x20: resistances to the eight attack stats (actor_apply_magic_to_actor) */
    u16 gold_drop_center; /* +0x30: drop id (map_object_spawn_scattered_effect) when script 3 ends */
    u16 initial_model_scale_q12; /* +0x32: scale */
    u32 initial_actor_flags; /* +0x34: flags copied to Actor.unk28 */
    u8 *targets[16]; /* +0x38: script records, the first byte their id; 0 ends */
} ActorGroup;

typedef struct ActorState {
    Actor objects[200];
    ActorGroup groups[40];  /* +0x6a40 */
    u8 target_candidate_blob[0x2000];
    u8 actor_overlap_exclusion_flags; /* +0x9d00: Actor.unk28 flags the searches skip */
    u8 unk9d01[3];
    s32 actor_collision_query_flags; /* +0x9d04: mask passed to the collision tests */
    ActorGroup *current_group; /* +0x9d08: record of current */
    Actor *current;         /* +0x9d0c: the record being updated */
    ActorGroup *other_group; /* +0x9d10: record of target */
    Actor *other_actor;     /* +0x9d14: objects[current->unk22] when flag 0x10 */
    s32 current_index;      /* +0x9d18: index of current */
    s32 current_group_index; /* +0x9d1c: current->unk2 */
    s32 active_actor_count; /* +0x9d20: active records updated this frame */
    s32 actor_update_frame_count; /* +0x9d24: frame counter */
    Actor *player_view_target; /* +0x9d28: player_probe_view_target_and_vectors keeps its last object here */
} ActorState;

/* 0x18-byte records at the start of effect_status (WIP). */
typedef struct MagicRecord {
    u8 menu_available; /* +0x00: player_learn_element_magic sets it to 1 */
    u8 charge_rate;
    u8 sound_id;    /* +0x02: SoundState.effects[unk2 + 144], 0xff for none */
    u8 training_gain;
    u8 player_status_flags;
    u8 element_mask; /* +0x05: bits 0-4 pick PlayerState.unk42[0..4] */
    u16 damage_slash;
    u16 damage_blow;
    u16 damage_stab;
    u16 damage_holy;
    u16 damage_fire;
    u16 damage_earth;
    u16 damage_wind;
    u16 damage_water;
    u16 mp_cost;    /* +0x16: unk1c cost of the spell (player_dispatch_weapon_magic);
                     * menu_collect_available_magic_rows lists it */
} MagicRecord;

typedef struct EffectState {
    MagicRecord magic_records[96];
    Effect objects[128];    /* +0x900 */
    MagicRecord *current_magic; /* +0x2f00: record of unk2f04 */
    Effect *current;        /* +0x2f04: the Effect record being updated */
    s32 current_index;      /* +0x2f08: index of unk2f04 (effect_pool_sweep) */
} EffectState;

/* 12-byte level records (player_lv_grow_list, one per level). */
typedef struct PlayerLevelGrowth {
    u16 hp_max;     /* new PlayerState.hp_max */
    u16 mp_max;     /* new PlayerState.unk1a */
    u16 physical_power_step; /* added to PlayerState.unk32 */
    u16 magic_step;
    s32 experience_threshold; /* experience for the next level */
} PlayerLevelGrowth;

/* 0x44-byte records player_recalculate_combat_stats adds to the player's unk54[]/unk62 (WIP). */
typedef struct WeaponRecord {
    u8 unk0[4];
    u8 alternate_magic_id;
    u8 magic_shots;
    u16 attack_components[8];
    u16 hp_regen_interval; /* +0x16: frame period of an hp point (player_update_frame), 0 for none */
    u16 mp_regen_interval; /* +0x18: frame period of a PlayerState.unk1c point, 0 for none */
    u16 attack_reach; /* +0x1a: reach of the swing probe (player_update_weapon_attack) */
    u16 attack_phase_step; /* +0x1c..+0x32: per hand the swing speed, hit time, limits and steps
                     * player_update_weapon_attack runs the swing with (WIP names) */
    u16 normal_attack_end_phase;
    u16 magic_window_start;
    u16 magic_window_end;
    u16 alternate_attack_phase_step;
    u16 alternate_attack_window_start;
    u16 alternate_attack_end_phase;
    u16 magic_phase_step;
    u16 normal_attack_sound_phase;
    u16 alternate_attack_sound_start_phase;
    u16 alternate_attack_sound_end_phase;
    u16 alternate_attack_sound_phase_step;
    u16 position_offset_x; /* +0x34..+0x42: model offsets of the weapon tip (player_sample_weapon_world_vertex) */
    u16 position_offset_y;
    u16 position_offset_z;
    s16 initial_vertex_index;
    u16 rotation_offset_x;
    u16 rotation_offset_y;
    u16 rotation_offset_z;
    s16 final_vertex_index;
} WeaponRecord;

/* One entry of a loaded FDAT module's handler table. */
typedef s32 (*FdatHandler)();

/*
 * Slots of that table (FdatHeader.handlers, asset_status.handlers) GAME.EXE calls; the
 * slots not listed are empty in every module.
 */
#define FDAT_SLOT_INTERACT                  0
#define FDAT_SLOT_UPDATE                    1
#define FDAT_SLOT_USE_ITEM                  2
#define FDAT_SLOT_REGION_TRIGGER            3
#define FDAT_SLOT_TALK_EVENT                4
#define FDAT_SLOT_ON_LOAD                   5
#define FDAT_SLOT_ON_ITEM_ADDED             6
#define FDAT_SLOT_ON_MOVIE_END              7
#define FDAT_SLOT_INIT_MAP_OBJECT           8
#define FDAT_SLOT_UPDATE_MAP_OBJECT         9
#define FDAT_SLOT_ON_MOVIE_START            10
#define FDAT_SLOT_ON_ITEM_REMOVED           11
#define FDAT_SLOT_ACTOR_TARGET_CHECK        16
#define FDAT_SLOT_UPDATE_ACTOR_TARGET       17
#define FDAT_SLOT_ADJUST_ACTOR_MAGIC_DAMAGE 18
#define FDAT_SLOT_ON_ACTOR_REMOVED          19

/*
 * Area state resource_initialize_game_assets and resource_run_initial_transition set up (WIP). Only the scene
 * module, the render module (render_scene_and_update_resources reads step lhu + sll) and the
 * player frame (player_update_frame reloads it for each of five compares) see the
 * step the CD callbacks set as volatile: the menu module's accesses to the
 * same object show none of that (a volatile member changes how GCC 2.6
 * reaches every member of the aggregate; see game/cd.h).
 */
#if defined(GAME_SCENE_MODULE) || defined(GAME_RENDER_MODULE) || defined(GAME_FLOOR_SELECT_MODULE)
#define SCENE_STEP_SHARED volatile
#else
#define SCENE_STEP_SHARED
#endif

typedef struct ResourceState {
    u16 loading;    /* +0x00: resource_run_initial_transition steps resource_advance_transition
                     * until it clears */
    SCENE_STEP_SHARED s16 step; /* +0x02: the CD callbacks resource_transition_set_phase_1.. set it */
    u8 active_resource_ids[5]; /* +0x04: 99 asks for a reload, 0xff keeps a part; [0] picks
                     * the model id bank (map_object_asset_index) */
    u8 current_area;
    u8 unka[2];
    FdatHandler *handlers; /* +0x0c: the loaded FDAT module's handler table */
    u8 requested_resource_ids[5]; /* +0x10: [0] is the model id bank while loading, 0xff for none */
    u8 world_shift_applied; /* +0x15: set once the loader shifted the origin (resource_advance_transition) */
    u8 tmd_object_limit_active; /* +0x16: render_map_cell_object limits the parts drawn while loading */
    u8 area_records_loaded; /* +0x17 */
    u8 unk18;
    s8 shift[3];    /* +0x19: 0x7f is no shift */
    s16 sequence_fade_volume; /* +0x1c: sequence volume while the loader fades it out */
    u8 unk1e[2];
    MapObject *animated_object; /* +0x20: unk24 once a kind 0xf0 record was drawn, NULL when
                      * the view hides them; drawn while an area loads */
    MapObject animated_object_copy; /* +0x24: copy of the last kind 0xf0 record drawn */
} ResourceState;

typedef struct MapObjectState {
    MapObjectDef defs[332]; /* WIP count: fills the space before objects */
    MapObject objects[0x18c];
    u8 unk8850[4];
    MapObjectDef *current_def; /* +0x8854: the def of unk8858 (map_object_update_actions) */
    MapObject *current;     /* +0x8858: the record map_object_find_collision_at_point does not test;
                             * the object FDAT slot 9 handlers test */
    u8 unk885c[2];
    u16 scatter_pool_sequence;
    u16 definition_drop_sequence;
    u16 placement_drop_sequence;
    u8 unk8864[4];
} MapObjectState;

/*
 * FDAT window words in the load image (owner unit unknown): [2] the window
 * base, [4] the end of the loaded module, [5] the heap base main stores.
 */
extern u32 fdat_window_bounds[6];

/* Arena memory_arena_initialize_blocks formats with a 0x4d800-byte free block. */
extern u8 resource_space[0x4d800];

extern ActorState actor_status;
extern ResourceState asset_status;
extern MapObjectState map_objects_slots;
extern PlayerState cur_avatar_status;
/* Area map bits: rooms seen per area (32 areas x 160 bits) and the 80 x 80 grid of
 * visited cells map_reveal_visited_rects marks rooms from. */
extern u8 map_revealed_rects[32][20];
extern u8 map_visited_cells[80][10];
extern EffectState effect_status;

/* A record of the table resource_initialize_game_assets copies in (conversation_records.unk0). */
typedef struct ConversationSource {
    u8 id;          /* 0xff ends the table */
    u8 count;       /* slots in its run of conversation_records.unk1e0 */
    u16 first_entry;
} ConversationSource;

/* A record conversation_records_initialize builds from a ConversationSource (conversation_records.unkc0). */
typedef struct ConversationRecord {
    u8 id;          /* 0xff ends the table */
    u8 unk1;
    u16 first_entry; /* ConversationSource.unk2 */
    u16 offset;     /* start of its run in conversation_records.unk1e0, 0xffff for none */
} ConversationRecord;

/*
 * Slot tables (WIP extent, up to actor_animation_shift_target): the copied records, the
 * records conversation_records_initialize builds from them and their runs of slot bytes
 * (bits 0-1 set: in use; 0xff: free or the end of a run). The save code
 * copies the 0x3c0 run bytes with word-aligned block moves, so the object
 * is word-aligned; the rest of the extent is unread (WIP words).
 */
typedef struct ConversationRecords {
    ConversationSource sources[48];
    ConversationRecord records[48]; /* +0xc0 */
    u8 page_states[0x3c0]; /* +0x1e0: the runs (SaveData.unk5148) */
    u32 unk5a0[0x200];   /* +0x5a0 */
} ConversationRecords;
extern ConversationRecords conversation_records;
extern EventState cur_evt_state;
/* Two counts per item id (0..149; game_counter_increment caps each at 99), cleared by
 * event_state_initialize. */
extern u8 event_item_counters_table[2][150];
extern MapWorld map_world;
extern MovieState movie_state;

/*
 * Tables resource_initialize_game_assets copies out of the first FDAT.T entry: the level
 * records (one per level; 100 fill the gap to player_weapon_asset_bufs), the 0x44-byte
 * records (48 fill the gap to map_world), and the rest (WIP extents).
 */
extern PlayerLevelGrowth player_lv_grow_list[100];
extern WeaponRecord weapon_table[48];
extern u32 tmd_archive_28_buffer[];
extern u32 tmd_0_buffer[];
extern u32 item_gear_recs[];
extern u8 resource_tmd_arc_work[];

/* What game_initialize_session points PlayerState.unkb4 at (WIP extent). */
extern u8 player_weapon_asset_bufs[];

#endif
