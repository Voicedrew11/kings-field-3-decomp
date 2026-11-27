/*
 * GAME.EXE: map cell patches (KF2's map_cell_pattern_place.c): records that
 * rewrite the layer bytes of map cells around a point, turned by an angle, and
 * a block copy of cells to a turned destination. Callers: the MapObject set-up
 * map_object_initialize_from_placements and the big switch
 * map_object_update_actions.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/data.h>
#include <game/map_cell_pattern.h>

/*
 * Applies the patch records at rec (variant index) to the cells around
 * (x, z) turned by angle: the area's own layer takes bytes 0 and 2 of the
 * variant, the other layer bytes 1 and 3 (0xfc / 0xff keep a byte); a flags
 * value other than 0xff replaces bits 0-6 of lighting_index on both layers.
 */
ADDRESS(0x800443c8, 0x1f0)
void map_cell_apply_rotated_pattern(u8 area, s32 x, s32 z, s32 angle, MapCellPattern *rec, s32 index,
                                    s32 flags)
{
    s32 c;
    s32 s;
    s32 px;
    s32 pz;
    MapCell *cell;
    MapLayer *own;
    MapLayer *other;
    u8 *bytes;
    s32 near;
    s32 far;

    x >>= 11;
    z >>= 11;
    c = rcos(angle);
    s = rsin(angle);
    near = area == 1 ? 0 : 5;
    far = near == 0 ? 5 : 0;
    while (rec->variant[0][0] != 0xff) {
        px = ((rec->dx * c - rec->dz * s) >> 12) + x;
        pz = ((rec->dz * c + rec->dx * s) >> 12) + z;
        cell = &map_world.cells[pz][px];
        bytes = rec->variant[index];
        own = (MapLayer *)((u8 *)cell + near);
        other = (MapLayer *)((u8 *)cell + far);
        if (bytes[0] != 0xfc) {
            own->collision_shape_id = bytes[0];
        }
        if (bytes[1] != 0xfc) {
            other->collision_shape_id = bytes[1];
        }
        if (bytes[2] != 0xff) {
            own->object_index = bytes[2];
        }
        if (bytes[3] != 0xff) {
            other->object_index = bytes[3];
        }
        if (flags != 0xff) {
            own->lighting_index = (own->lighting_index & 0x7f) | flags;
            other->lighting_index = (other->lighting_index & 0x7f) | flags;
        }
        rec++;
    }
}

/*
 * Copies a w x h block of map cells from (sx, sz) to (dx, dz), turned by
 * angle, which is reduced to quarter turns in place; mask picks the layer 1
 * bytes and bits copied (bits 0-1 of quarter_turns are turned with the
 * block). Bit 1 of flags enables the copy; the test is inside the loops.
 */
ADDRESS(0x800445b8, 0x274)
void map_cell_copy_rotated_fields(u8 flags, s32 sx, s32 sz, s32 dx, s32 dz, s32 w, s32 h, s32 angle,
                                  s32 mask)
{
    s32 col;
    s32 row;
    MapCell *src_row;
    MapCell *dst_row;
    MapCell *src;
    MapCell *dst;
    s32 i;
    s32 j;

    if (sx == 0xff || dx == 0xff || w == 0xff) {
        return;
    }
    angle = -(angle >> 10) & 3;
    switch (angle) {
    case 0:
        col = 1;
        row = 80;
        break;
    case 1:
        col = -80;
        row = 1;
        dz += w - 1;
        break;
    case 2:
        col = -1;
        row = -80;
        dx += w - 1;
        dz += h - 1;
        break;
    case 3:
        col = 80;
        row = -1;
        dx += h - 1;
        break;
    }
    src_row = &map_world.cells[sz][sx];
    dst_row = &map_world.cells[dz][dx];
    for (i = h - 1; i != -1; i--) {
        src = src_row;
        dst = dst_row;
        src_row += 80;
        dst_row += row;
        j = w;
        while (--j != -1) {
            if (flags & 2) {
                if (mask & 1) {
                    dst->layer[1].object_index = src->layer[1].object_index;
                }
                if (mask & 2) {
                    dst->layer[1].elevation = src->layer[1].elevation;
                }
                if (mask & 4) {
                    dst->layer[1].quarter_turns = (dst->layer[1].quarter_turns & 0xfc)
                        | ((src->layer[1].quarter_turns + angle) & 3);
                }
                if (mask & 8) {
                    dst->layer[1].collision_shape_id = src->layer[1].collision_shape_id;
                }
                if (mask & 0x10) {
                    dst->layer[1].lighting_index = (src->layer[1].lighting_index & 0x3f)
                        | (dst->layer[1].lighting_index & 0xc0);
                }
                if (mask & 0x20) {
                    dst->layer[1].lighting_index = (src->layer[1].lighting_index & 0x40)
                        | (dst->layer[1].lighting_index & 0xbf);
                }
                if (mask & 0x40) {
                    dst->layer[1].lighting_index = (src->layer[1].lighting_index & 0x80)
                        | (dst->layer[1].lighting_index & 0x7f);
                }
            }
            src++;
            dst += col;
        }
    }
}
