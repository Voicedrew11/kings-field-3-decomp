/*
 * GAME.EXE: the cur_evt_state block arena's 32 saved pointers (WIP module):
 * pointers to and from arena offsets, moving the blocks' owner pointers by
 * a delta, and saving and restoring an area's object states in its block
 * (event_world_state_save_slot/event_world_state_restore_slot). The .rdata is their two jump tables.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/libc.h>
#include <game/funcs.h>
#include <game/memory.h>
#include <game/data.h>
#include <game/misc.h>
#include <game/object44.h>

RODATA(0x80013298, 0x3e8)

/* out[i] = the arena address of saved offset i, NULL for 0xffff. */
ADDRESS(0x8005ee58, 0x50)
void event_saved_offsets_decode(u8 **out)
{
    u16 *off;
    u16 o;
    s32 i;

    off = cur_evt_state.saved_offsets;
    for (i = 31; i != -1; i--) {
        o = *off++;
        if (o == 0xffff) {
            *out = NULL;
        } else {
            *out = cur_evt_state.arena + o;
        }
        out++;
    }
}

/* Adds delta to the owner pointer of every used block. */
ADDRESS(0x8005eea8, 0x70)
void event_arena_owner_pointers_add_delta(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)cur_evt_state.arena;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner + delta);
        }
        block = HEAP_NEXT(block);
    }
}

/* Saves in[i] as an arena offset, 0xffff for NULL. */
ADDRESS(0x8005ef18, 0x4c)
void event_saved_offsets_encode(u8 **in)
{
    u16 *off;
    u8 *p;
    s32 i;

    off = cur_evt_state.saved_offsets;
    for (i = 31; i != -1; i--) {
        p = *in++;
        if (p == NULL) {
            *off = 0xffff;
        } else {
            *off = p - cur_evt_state.arena;
        }
        off++;
    }
}

/* Subtracts delta from the owner pointer of every used block. */
ADDRESS(0x8005ef64, 0x70)
void event_arena_owner_pointers_subtract_delta(s32 delta)
{
    HeapBlock *block;
    s32 state;

    block = (HeapBlock *)cur_evt_state.arena;
    while (block->state != HEAP_END) {
        state = block->state;
        if (state < 4 && state != HEAP_FREE) {
            block->owner = (u8 **)((u8 *)block->owner - delta);
        }
        block = HEAP_NEXT(block);
    }
}

/*
 * Saves area's state in its saved block: the live Actor records, the talk
 * script positions and each MapObject record by its MapObjectDef type.
 */
ADDRESS(0x8005efd4, 0x470)
void event_world_state_save_slot(s32 area)
{
    u8 *ptrs[32];
    u8 buf[0xc00];
    Actor *obj;
    ActorGroup *def;
    MapObject *rec;
    u8 *script;
    u8 *p;
    u8 *block;
    s32 size;
    s32 i;

    p = buf;
    obj = actor_status.objects;
    for (i = 0; i < 200; i++) {
        if (obj->slot_state != 0xff && obj->slot_state == 1) {
            *p++ = i;
            if (obj->lifecycle == 3) {
                *p = 3;
            } else {
                *p = 0;
            }
            p++;
        }
        obj++;
    }
    *p++ = 0xff;
    def = actor_status.groups;
    for (i = 0; i < 40; i++) {
        if (def->definition_id == 0xff) {
            break;
        }
        script = def->targets[0];
        def++;
        if (script != NULL && script[0] == 0x70) {
            *p++ = i;
            *p++ = script[16];
            *p++ = script[19];
        }
    }
    *p++ = 0xff;
    rec = map_objects_slots.objects;
    for (i = 0; i < 396; i++) {
        if (rec->object_id == 0xff) {
            *p++ = 0xff;
        } else {
            switch (map_objects_slots.defs[rec->object_id].action) {
            case 0x40:
                switch (rec->action) {
                case 0x60:
                    *p++ = 0xf0;
                    *p++ = rec->object_id;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    *p++ = rec->angle.vy >> 4;
                    break;
                case 0x61:
                    *p++ = 0xf1;
                    *p++ = rec->object_id;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    break;
                case 0x62:
                    *p++ = 0xf2;
                    *p++ = rec->object_id;
                    *p++ = rec->pos.vx >> 2;
                    *p++ = rec->pos.vx >> 10;
                    *p++ = rec->pos.vz >> 2;
                    *p++ = rec->pos.vz >> 10;
                    *p++ = rec->pos.vy;
                    *p++ = rec->pos.vy >> 8;
                    *p++ = rec->arg.h.unk3a >> 2;
                    break;
                case 0x70:
                    *p++ = 0xf3;
                    *p++ = rec->object_id;
                    *p++ = rec->arg.b.unk38;
                    break;
                default:
                    *p++ = 0xfd;
                    *p++ = rec->arg.b.unk38;
                    break;
                }
                break;
            case 0x53:
                if (rec->arg.b.unk38 < 2) {
                    break;
                }
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x17:
            case 0x1b:
            case 0x51:
            case 0x52:
            case 0x54:
            case 0x55:
            case 0x57:
            case 0x58:
            case 0x5a:
            case 0x5f:
            case 0xa0:
            case 0xa1:
            case 0xa3:
            case 0xa4:
            case 0xa6:
            case 0xe8:
                *p++ = 0xfd;
                *p++ = rec->arg.b.unk38;
                break;
            case 0x0f:
            case 0x11:
            case 0x18:
            case 0x1a:
            case 0xa7:
                *p++ = 0xf4;
                *p++ = rec->arg.b.unk38;
                *p++ = rec->arg.b.unk39;
                break;
            case 0x56:
                *p++ = 0xf5;
                *p++ = rec->arg.b.unk39;
                break;
            default:
                *p++ = 0xfe;
                break;
            }
        }
        rec++;
    }
    event_saved_offsets_decode(ptrs);
    event_arena_owner_pointers_add_delta((s32)ptrs);
    block = ptrs[area];
    if (block != NULL) {
        memory_block_release(block);
    }
    size = (p - buf + 3) & ~3;
    block = memory_arena_allocate_block((HeapBlock *)cur_evt_state.arena, size, &ptrs[area]);
    if (block != NULL) {
        resource_copy_words((u32 *)block, (u32 *)buf, size >> 2);
        event_saved_offsets_encode(ptrs);
        event_arena_owner_pointers_subtract_delta((s32)ptrs);
    }
}

/* Restores area's state from its saved block (event_world_state_save_slot's format). */
ADDRESS(0x8005f444, 0x378)
void event_world_state_restore_slot(s32 area)
{
    u8 *ptrs[32];
    MapObject *rec;
    u8 *script;
    u8 *p;
    s32 i;
    s32 x;
    s32 z;
    s16 y;

    event_saved_offsets_decode(ptrs);
    p = ptrs[area];
    if (p == NULL) {
        return;
    }
    for (;;) {
        s32 n = *p++;

        if (n == 0xff) {
            break;
        }
        (actor_status.objects + n)->lifecycle = *p++;
    }
    for (;;) {
        s32 n = *p++;

        if (n == 0xff) {
            break;
        }
        script = (actor_status.groups + n)->targets[0];
        script[16] = *p++;
        script[19] = *p++;
    }
    rec = map_objects_slots.objects;
    for (i = 0; i < 396; i++) {
        switch (*p++) {
        case 0xff:
            rec->object_id = 0xff;
            break;
        case 0xf4:
            rec->arg.b.unk38 = *p++;
        case 0xf5:
            rec->arg.b.unk39 = *p++;
            break;
        case 0xf0:
            map_object_reset(rec);
            rec->action = 0x60;
            rec->object_id = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            rec->angle.vy = *p++ << 4;
            rec->angle.vz = 0x400;
        place:
            rec->pos.vx = x << 2;
            rec->pos.vz = z << 2;
            rec->pos.vy = y;
            rec->action_state = 99;
            collision_sample_map_cell_layer(rec->pos.vx, rec->pos.vy, rec->pos.vz);
            rec->layer_mask = map_world.collision_layer_offset == 0 ? 1 : 2;
            rec->arg.b.unk38 = 0xff;
            break;
        case 0xf1:
            map_object_reset(rec);
            rec->action = 0x61;
            rec->object_id = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            goto place;
        case 0xf2:
            map_object_reset(rec);
            rec->action = 0x62;
            rec->object_id = *p++;
            x = *p++;
            x |= *p++ << 8;
            z = *p++;
            z |= *p++ << 8;
            y = *p++;
            y |= *p++ << 8;
            rec->arg.h.unk3a = (*p++ << 2) + (rand() >> 13);
            if (rec->object_id == 0x79) {
                rec->angle.vx = 0x400;
            }
            goto place;
        case 0xf3:
            map_object_reset(rec);
            rec->action = 0x70;
            rec->object_id = *p++;
        case 0xfd:
            rec->arg.b.unk38 = *p++;
            break;
        }
        rec++;
    }
}
