/*
 * GAME.EXE: MapObject records (WIP module): the 396 records after the
 * MapObjectDef table in map_objects_slots, their set-up, per-kind state and the map
 * cell bytes some kinds keep in step with their state.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <stdarg.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/sound.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/collision.h>
#include <game/render.h>
#include <game/map_patch.h>
#include <game/object44.h>

RODATA(0x80011c54, 0x400)

/* Plays sound effect se at the record. */
ADDRESS(0x8004482c, 0x30)
void map_object_play_spatial_sound(MapObject *obj, s32 se)
{
    audio_play_spatial_default_range(se, &obj->pos, 0x78, 0);
}

/* Frees every record. */
ADDRESS(0x8004485c, 0x5c)
void map_object_pool_reset(void)
{
    MapObject *obj;
    u16 i;

    obj = map_objects_slots.objects;
    i = 396;
    while (i--) {
        obj->action = obj->object_id = 0xff;
        obj->arg.w.animation_cache = obj->arg.w.unk38 = obj->arg.w.unk3c = 0;
        obj++;
    }
    map_objects_slots.scatter_pool_sequence = map_objects_slots.definition_drop_sequence = map_objects_slots.placement_drop_sequence = 0;
}

/* Resets a record's state, angle and scale. */
ADDRESS(0x800448b8, 0x48)
void map_object_reset(MapObject *obj)
{
    obj->animation_id = 0x80;
    obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
    obj->animation_phase = 0;
    obj->render_mode = 0;
    obj->layer_mask = 0;
    obj->angle.vy = obj->angle.vx = obj->angle.vz = 0;
    obj->action = 0xff;
    obj->render_depth_offset = 0;
    obj->lighting_override_index = 0xff;
    obj->lighting_blend = 0;
}

/*
 * Sets the state of record id by the type of its MapObjectDef: mode 0 stops
 * it, 1 sets layer_mask to the byte argument, 2 marks it, 3 sets render_depth_offset to the
 * halfword argument.
 */
ADDRESS(0x80044900, 0x240)
void map_object_set_property(s32 id, s32 mode, ...)
{
    va_list args;
    MapObject *obj;
    MapObjectDef *def;

    va_start(args, mode);
    if (id == 0xffff) {
        return;
    }
    obj = &map_objects_slots.objects[id];
    def = &map_objects_slots.defs[obj->object_id];
    switch (mode) {
    case 0:
        switch (def->action) {
        case 0xd:
        case 0x53:
            obj->layer_mask = 0;
            /* fall through */
        case 0x14:
            obj->arg.b.unk3d[1] = 0;
            break;
        case 9:
        case 0x15:
            obj->layer_mask = 0;
            obj->arg.b.unk3c = 0;
            break;
        case 0x40:
            obj->layer_mask = 0;
            obj->arg.b.unk38 = 0;
            if (obj->object_id != 0x95) {
                if (obj->arg.b.unk3a.bytes[0] != 0xff) {
                    obj->angle.vx = obj->arg.b.unk3a.bytes[0] << 6;
                }
                if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                    obj->angle.vy = obj->arg.b.unk3a.bytes[1] << 6;
                }
                if (obj->arg.b.unk3c != 0xff) {
                    obj->angle.vz = obj->arg.b.unk3c << 6;
                }
                if (obj->arg.b.unk39 == 0) {
                    obj->action = 0x10;
                    obj->arg.w.unk40 = obj->pos.vy;
                }
            }
            break;
        default:
            obj->layer_mask = 0;
            obj->arg.b.unk38 = 0;
            break;
        }
        break;
    case 1:
        switch (def->action) {
        case 0x14:
        case 0x15:
            break;
        default:
            obj->layer_mask = va_arg(args, u8);
            break;
        }
        break;
    case 2:
        switch (def->action) {
        case 0xd:
        case 0x14:
        case 0x53:
            obj->arg.b.unk3d[1] = 0xff;
            break;
        case 9:
        case 0x15:
            obj->arg.b.unk3c = 0xff;
            break;
        default:
            obj->arg.b.unk38 = 0xff;
            break;
        }
        break;
    case 3:
        obj->render_depth_offset = va_arg(args, u16);
        break;
    }
    va_end(args);
}

/*
 * Writes the record's map cell byte for its area layer: value with no scale
 * when mode is 0 and PlayerState.map_marker_visual_effect_timer is clear, else 0xfc with full scale.
 */
ADDRESS(0x80044b40, 0xf4)
void map_object_set_cell_marker(MapObject *obj, s32 mode, u8 value)
{
    MapCell *cell;
    MapLayer *layer;

    if (mode == 0 && cur_avatar_status.map_marker_visual_effect_timer == 0) {
        {
            MapCell *row = map_world.cells[obj->pos.vz >> 11];

            cell = &row[obj->pos.vx >> 11];
        }
        layer = cell->layer;
        if (obj->layer_mask != 1) {
            layer++;
        }
        layer->object_index = value;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
    } else {
        {
            MapCell *row = map_world.cells[obj->pos.vz >> 11];

            cell = &row[obj->pos.vx >> 11];
        }
        layer = cell->layer;
        if (obj->layer_mask != 1) {
            layer++;
        }
        layer->object_index = 0xfc;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
    }
}

/* Brings the map cell bytes of kinds 7, 0x51 and 0x52 in line with mode. */
ADDRESS(0x80044c34, 0x168)
void map_object_refresh_cell_markers(s32 mode)
{
    MapObject *obj;
    MapObjectDef *def;
    s32 i;

    obj = map_objects_slots.objects;
    for (i = 396; i != 0; i--) {
        switch (obj->action) {
        case 0x51:
            if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                def = &map_objects_slots.defs[obj->object_id];
                map_object_set_cell_marker(obj, mode, def->arg.b.unk17);
            }
            break;
        case 0x52:
            if (obj->arg.b.unk3d[0] != 0xff) {
                def = &map_objects_slots.defs[obj->object_id];
                map_object_set_cell_marker(obj, mode, def->arg.b.unk17);
            }
            break;
        case 7:
            if (obj->arg.b.unk38 != 0xfe) {
                map_object_set_property(obj->arg.h.unk3a, mode, obj->layer_mask);
                def = &map_objects_slots.defs[obj->object_id];
                if (def->arg.b.unkc != 0xff) {
                    map_object_set_cell_marker(obj, mode, def->arg.b.unkc);
                }
            }
            break;
        }
        obj++;
    }
}

/*
 * Sets up the area's 350 MapObject records from its placement table: def,
 * position on the map cell, radius on the cell grid, then per MapObjectDef
 * type the kind and its state (map patches, timers, cell bytes); types
 * without a case go to FDAT handler 8.
 * Not exact: retail adds the cell row (base + z * 800) before the column
 * (+ x * 10) where GCC here forms z * 800 + (x * 10 + base) (as in
 * map_object_set_cell_marker), and loads def->arg.h.unke before def->arg.h.unk10 for the larger radius.
 */
ADDRESS(0x80044d9c, 0xd2c)
void map_object_initialize_from_placements(MapObjectPlacement *place)
{
    MapObject *obj;
    MapObjectDef *def;
    u32 radius;
    s32 i;
    u32 frames;

    frames = cdrom_ctl.frames;
    obj = map_objects_slots.objects;
    for (i = 349; i != -1; place++, obj++, i--) {
        if (place->object_id == 0xffff) {
            obj->object_id = 0xff;
        } else {
            obj->object_id = place->object_id;
        }
        obj->action = 0xff;
        if (obj->object_id == 0xff) {
            continue;
        }
        def = &map_objects_slots.defs[obj->object_id];
        obj->action_state = 0;
        obj->animation_id = 0x80;
        obj->animation_phase = 0;
        obj->render_mode = 0;
        obj->angle.vz = 0;
        obj->angle.vx = 0;
        obj->angle.vy = -place->rotation_y & 0xfff;
        obj->scale.vx = obj->scale.vy = obj->scale.vz = 0x1000;
        obj->layer_mask = place->layer_mask;
        obj->flags = def->flags;
        obj->render_depth_offset = def->initial_render_depth_offset;
        obj->lighting_override_index = 0xff;
        obj->lighting_blend = 0;
        if (obj->flags & 0x20) {
            obj->render_mode |= 0x80;
        }
        if (obj->flags & 0x10) {
            obj->collision_height = (def->collision_height * place->arg_bytes[0]) >> 7;
            obj->scale.vx = obj->scale.vy = obj->scale.vz = place->arg_bytes[0] << 5;
        } else {
            obj->collision_height = def->collision_height;
        }
        obj->pos.vx = (place->cell_x << 11) + place->local_x;
        obj->pos.vz = (place->cell_z << 11) + place->local_z;
        {
            MapCell *row = map_world.cells[place->cell_z];
            MapLayer *layer = row[place->cell_x].layer;

            if (obj->layer_mask != 1) {
                layer++;
            }
            obj->pos.vy = place->height - (layer->elevation << 7);
        }
        resource_copy_words((u32 *)&obj->arg.b.unk38, (u32 *)place->arg_bytes, 2);
        repeat_store_word((u32 *)&obj->arg.w.unk40, 0xff, 1);
        radius = def->collision_radius;
        if (radius != 0) {
            if (obj->flags & 0x10) {
                map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, radius * obj->arg.b.unk38 >> 7, 1);
            } else {
                map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, radius, 1);
            }
        } else if (obj->flags & 4) {
            radius = def->arg.h.unk10;
            if (def->arg.h.unke > radius) {
                radius = def->arg.h.unke;
            }
            map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, radius, 1);
        }
        switch (def->action) {
        case 1:
            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + 1, obj->arg.b.unk3c,
                                         obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], def->arg.b.unkd,
                                         def->arg.b.unke, obj->angle.vy, 45);
            obj->action = 1;
            obj->action_state = 2;
            obj->arg.h.unk40 = 999;
            obj->arg.h.base_yaw = obj->angle.vy;
            map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, 1500, 1);
            break;
        case 27:
            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + 3, obj->arg.b.unk3c,
                                         obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2, obj->angle.vy,
                                         45);
            obj->action = 0x1b;
            obj->action_state = 2;
            obj->arg.h.unk40 = 999;
            obj->arg.h.base_yaw = obj->angle.vy;
            map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, 3000, 1);
            break;
        case 0:
            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + 3, obj->arg.b.unk3c,
                                         obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0], 3, 2, obj->angle.vy,
                                         45);
            obj->action = 0;
            obj->action_state = 2;
            obj->arg.h.unk40 = 999;
            obj->arg.h.base_yaw = obj->angle.vy;
            map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, 3000, 1);
            break;
        case 3:
            obj->action = 3;
            obj->animation_id = 0;
            map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                           map_object_door_shape_pattern, 0, 0xff);
            map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                           map_object_door3_object_pattern, 0, 0xff);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 4:
            obj->action = 4;
            obj->animation_id = 0;
            map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                           map_object_door_shape_pattern, 0, 0xff);
            map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                           map_object_door4_object_pattern, 0, 0xff);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 5:
            obj->action = 5;
            obj->animation_id = 0;
            map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                           map_object_door5_shape_pattern, 0, 0xff);
            map_cell_apply_rotated_pattern(obj->layer_mask, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                           map_object_door5_object_pattern, 0, 0xff);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 2:
            obj->action = 2;
            obj->animation_id = 0;
            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + def->arg.b.unkd * 2,
                                         obj->arg.b.unk3c, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                         def->arg.b.unkd, def->arg.b.unke, obj->angle.vy, 45);
            map_cell_add_layer_occupancy(obj->pos.vx, obj->pos.vz, 4400, 1);
            break;
        case 23:
            obj->action = 0x17;
            break;
        case 6:
            obj->action = 6;
            break;
        case 7:
            obj->action = 7;
            obj->animation_id = 0;
            if (def->arg.b.unkc != 0xff) {
                map_object_set_cell_marker(obj, 0, def->arg.b.unkc);
            }
            break;
        case 8:
            obj->action = 8;
            break;
        case 9:
            obj->action = 9;
            obj->arg.b.unk40 = obj->layer_mask;
            break;
        case 21:
            obj->action = 9;
            obj->arg.b.unk40 = obj->layer_mask;
            obj->layer_mask = 0;
            break;
        case 83:
            obj->action = 0x53;
            obj->animation_id = 0;
            obj->action_state = 9;
            break;
        case 84:
            obj->animation_id = 0;
            obj->arg.b.unk40 = obj->layer_mask;
            obj->layer_mask = 2;
            obj->action = 0x54;
            map_cell_apply_rotated_pattern(obj->arg.b.unk40, obj->pos.vx, obj->pos.vz, obj->angle.vy,
                                           map_object_cell_patterns[def->arg.b.unke * 2 + (obj->arg.b.unk3d[1] & 1)], 0,
                                           0);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 95:
            obj->layer_mask = 0;
            obj->action = 0x5f;
            obj->action_state = 1;
            break;
        case 81:
            obj->animation_id = 0;
            obj->action = 0x51;
            obj->arg.b.unk40 = 0;
            if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                map_object_set_cell_marker(obj, 0, def->arg.b.unk17);
            }
            break;
        case 87:
            obj->animation_id = 0;
            obj->layer_mask = 2;
            obj->action = 0x57;
            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0],
                                         obj->arg.b.unk3c, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                         obj->arg.b.unk3d[0], obj->arg.b.unk3d[1], obj->angle.vy, 45);
            obj->scale.vx = obj->scale.vy = obj->scale.vz = 0;
            break;
        case 90:
            obj->animation_id = 0;
            obj->layer_mask = 2;
            obj->action = 0x5a;
            map_cell_copy_rotated_fields(obj->layer_mask, obj->arg.b.unk3a.bytes[1] + obj->arg.b.unk3d[0],
                                         obj->arg.b.unk3c, obj->arg.b.unk39, obj->arg.b.unk3a.bytes[0],
                                         obj->arg.b.unk3d[0], obj->arg.b.unk3d[1], obj->angle.vy, 45);
            break;
        case 82:
            obj->animation_id = 0;
            obj->action = 0x52;
            obj->arg.b.unk40 = 0;
            if (obj->arg.b.unk3d[0] != 0xff) {
                map_object_set_cell_marker(obj, 0, def->arg.b.unk17);
            }
            break;
        case 85:
            obj->animation_id = 0;
            obj->action = 0x55;
            obj->arg.b.unk40 = 0;
            if (obj->arg.b.unk3a.bytes[1] == obj->arg.b.unk3d[0]) {
                if (obj->arg.b.unk3c < obj->arg.b.unk3d[1]) {
                    obj->arg.b.unk41[1] = 0;
                } else {
                    u8 t;

                    obj->arg.b.unk41[1] = 1;
                    t = obj->arg.b.unk3c;
                    obj->arg.b.unk3c = obj->arg.b.unk3d[1];
                    obj->arg.b.unk3d[1] = t;
                }
            } else if (obj->arg.b.unk3c == obj->arg.b.unk3d[1]) {
                if (obj->arg.b.unk3a.bytes[1] < obj->arg.b.unk3d[0]) {
                    obj->arg.b.unk41[1] = 2;
                } else {
                    u8 t;

                    obj->arg.b.unk41[1] = 3;
                    t = obj->arg.b.unk3a.bytes[1];
                    obj->arg.b.unk3a.bytes[1] = obj->arg.b.unk3d[0];
                    obj->arg.b.unk3d[0] = t;
                }
            }
            obj->arg.b.unk41[2] = 1;
            break;
        case 86:
            obj->action = 0x56;
            obj->action_state = 0;
            break;
        case 231:
            obj->layer_mask = 0;
            obj->action = 0xe7;
            obj->arg.b.unk40 = 0;
            break;
        case 235:
            obj->action = 0xeb;
            obj->arg.b.unk40 = 1;
            break;
        case 234:
            obj->layer_mask = 0;
            obj->action = 0xea;
            obj->arg.b.unk40 = 0;
            break;
        case 232:
            obj->animation_id = 0x80;
            obj->action = 0xe8;
            obj->animation_phase = 0;
            obj->arg.b.unk40 = 0xff;
            break;
        case 224:
            obj->layer_mask = 0;
            obj->action = 0xe0;
            obj->pos.vx -= place->local_x;
            obj->pos.vz -= place->local_z;
            obj->pos.vy -= place->height;
            if (place->local_x == 0xff) {
                obj->arg.b.unk40 = obj->arg.b.unk41[0] = obj->arg.b.unk41[1] = 0x7f;
            } else {
                obj->arg.b.unk40 = (u8)place->local_x - place->cell_x;
                obj->arg.b.unk41[0] = (u8)place->local_z - place->cell_z;
                obj->arg.b.unk41[1] = (u8)place->height - (-obj->pos.vy >> 7);
            }
            break;
        case 225:
            obj->layer_mask = 0;
            obj->action = 0xe1;
            obj->arg.b.unk40 = 0;
            break;
        case 227:
            obj->layer_mask = 0;
            obj->action = 0xe3;
            if (player_camera_within_map_region(obj->pos.vx >> 11, obj->pos.vz >> 11, obj->arg.b.unk38,
                                                obj->arg.b.unk39, 0x8000) != 0) {
                scene_display_sys.view_range = obj->arg.b.unk3a.bytes[0];
                scene_display_sys.unk15996 = obj->arg.b.unk3a.bytes[1];
            }
            break;
        case 228:
            obj->layer_mask = 0;
            obj->action = 0xe4;
            break;
        case 229:
            obj->action = 0xe5;
            break;
        case 230:
            obj->layer_mask = 0;
            obj->action = 0xe6;
            obj->arg.b.unk40 = 0;
            break;
        case 31:
            obj->layer_mask = 0;
            obj->action = 0x1f;
            obj->arg.w.unk40 = frames + (u16)obj->arg.h.unk3e * 6;
            break;
        case 240:
            obj->action = 0xf0;
            break;
        case 242:
            obj->action = 0xf2;
            break;
        case 233:
            obj->action = 0xe9;
            break;
        case 13:
            if (obj->arg.b.unk3a.bytes[0] != 0xff) {
                obj->angle.vx = obj->arg.b.unk3a.bytes[0] << 6;
            }
            if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                obj->angle.vy = obj->arg.b.unk3a.bytes[1] << 6;
            }
            if (obj->arg.b.unk3c != 0xff) {
                obj->angle.vz = obj->arg.b.unk3c << 6;
            }
            break;
        case 226:
            obj->arg.b.unk40 = obj->layer_mask;
        case 11:
        case 20:
        clear_area:
            obj->layer_mask = 0;
            break;
        case 88:
            obj->animation_id = 0;
            obj->action = 0x58;
            obj->action_state = 1;
            break;
        case 89:
            obj->action = 0x59;
            obj->lighting_override_index = 0x32;
            obj->lighting_blend = 0x1000;
            obj->render_mode = (obj->render_mode & 0xf8) | 5;
            obj->pos.vy += 256;
            {
                MapCell *row = map_world.cells[obj->pos.vz >> 11];
                MapLayer *layer = row[obj->pos.vx >> 11].layer;

                if (obj->layer_mask != 1) {
                    layer++;
                }
                layer->collision_shape_id = 0x75;
            }
            obj->arg.b.unk41[1] = obj->layer_mask;
            goto clear_area;
        case 15:
            obj->action = 0xf;
            break;
        case 17:
            obj->action = 0x11;
            break;
        case 24:
            obj->action = 0x18;
            break;
        case 26:
            obj->action = 0x1a;
            break;
        case 25:
            obj->action = 0x19;
            break;
        case 49:
            obj->action = 0x31;
            break;
        case 50:
            obj->action = 0x32;
            obj->animation_id = 0;
            obj->action_state = 0;
            break;
        case 64:
            if (def->kind != 0x20) {
                if (obj->arg.b.unk3a.bytes[0] != 0xff) {
                    obj->angle.vx = obj->arg.b.unk3a.bytes[0] << 6;
                }
                if (obj->arg.b.unk3a.bytes[1] != 0xff) {
                    obj->angle.vy = obj->arg.b.unk3a.bytes[1] << 6;
                }
                if (obj->arg.b.unk3c != 0xff) {
                    obj->angle.vz = obj->arg.b.unk3c << 6;
                }
                if (obj->arg.b.unk39 == 0) {
                    obj->action = 0x10;
                    obj->arg.w.unk40 = obj->pos.vy;
                }
            }
            break;
        case 19:
            obj->action = 0x13;
            break;
        case 48:
            obj->action = 0x30;
            obj->render_mode = (obj->render_mode & 0xf8) | 5;
            obj->pos.vy -= obj->arg.b.unk3c << 8;
            break;
        case 22:
            obj->action = 0x16;
            obj->arg.w.unk40 = frames + 30;
            break;
        case 18:
            obj->action = 0x12;
            obj->arg.w.unk40 = frames + 30;
            break;
        default:
            ((void (*)(MapObject *, MapObjectDef *))
                 asset_status.handlers[FDAT_SLOT_INIT_MAP_OBJECT])(obj, def);
            break;
        case 33:
        case 255:
            break;
        }
    }
}
