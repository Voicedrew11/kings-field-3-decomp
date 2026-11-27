/*
 * GAME.EXE: area set-up (WIP module). resource_initialize_game_assets opens the archive
 * files, loads the first FDAT.T entry into the arena and copies its tables
 * out; resource_run_initial_transition asks for a full area load and steps the loader until it
 * finishes; translate_active_world_positions shifts the player and every live object;
 * resource_request_transition asks for a load of some area parts; resource_advance_transition is the
 * loader's step machine (FDAT module, map, objects, image, sequence), whose
 * CD requests complete through the five callbacks before it. Its unit is
 * -G8: the handler-table pointer is a $gp-relative .sbss word, while the
 * FDAT window base is reached absolutely (declared without a size here).
 * The RODATA claim holds the COM names and resource_advance_transition's jump table.
 */
/* The area step is volatile only here (see game/data.h). */
#define GAME_SCENE_MODULE
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <psyq/kernel.h>
#include <game/sound.h>
#include <game/cd.h>
#include <game/memory.h>
#include <game/image.h>
#include <game/scene.h>
#include <game/render.h>
#include <game/data.h>
#include <game/misc.h>
#include <game/funcs.h>
#include <game/player.h>
#include <game/menu.h>
#include <game/entity.h>
#include <game/collision.h>
#include <game/object44.h>

/* archive file numbers (cd_archive_open table slots) */
#define FILE_FDAT 4

/* area[] value that asks resource_advance_transition to load the part again. */
#define AREA_RELOAD 99
#define AREA_KEEP 0xff
#define SHIFT_NONE 0x7f
/* step while a CD request is outstanding; its callback sets the next one. */
#define STEP_WAIT 0xf0

RODATA(0x80011000, 0x8c)

DATA(0x8009c3fc, 0x4, ".sbss")
u32 fdat_window_handlers;

ADDRESS(0x80017664, 0x308)
void resource_initialize_game_assets(void)
{
    u32 *p = (u32 *)resource_space;

    asset_status.active_resource_ids[1] = asset_status.active_resource_ids[2] =
        asset_status.active_resource_ids[3] = AREA_KEEP;
    asset_status.loading = 0;
    asset_status.step = 0;
    asset_status.active_resource_ids[0] = 0;
    asset_status.active_resource_ids[1] = 0;
    asset_status.active_resource_ids[2] = 0;
    asset_status.active_resource_ids[3] = 0;
    asset_status.active_resource_ids[4] = 0;
    asset_status.current_area = 0;
    fdat_window_handlers = fdat_window_bounds[2] + 4;
    fdat_window_base[0] = fdat_window_bounds[2];
    cd_archive_open(0, "COM\\MO.T");
    cd_archive_open(1, "COM\\MOF.T");
    cd_archive_open(2, "COM\\VAB.T");
    cd_archive_open(3, "COM\\RTIM.T");
    cd_archive_open(4, "COM\\FDAT.T");
    cd_archive_open(5, "COM\\RTMD.T");
    cd_archive_open(6, "COM\\ITEM.T");
    cd_archive_open(7, "COM\\TALK.T");
    cd_archive_open(8, "COM\\STALK.T");
    cd_archive_read(FILE_FDAT, 0x61, (u8 *)p);
    cd_map_stream_read(FILE_FDAT, 0x60);
    audio_queue_vab_stream(2, 0, 0);
    resource_copy_words((u32 *)&map_objects_slots, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)weapon_table, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words(item_gear_recs, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)player_lv_grow_list, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)&effect_status, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)light_default_records, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)&movie_state, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)sound_status.effects, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)&conversation_records, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words(tmd_0_buffer, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    asset_registry_load_tmd_archive(0, (u8 *)tmd_0_buffer);
    resource_copy_words(tmd_archive_28_buffer, p + 1, *p >> 2);
    asset_registry_load_tmd_archive(0x28, (u8 *)tmd_archive_28_buffer);
    game_initialize_session();
}

ADDRESS(0x8001796c, 0x138)
void resource_run_initial_transition(void)
{
    asset_status.loading = 1;
    asset_status.step = 0;
    asset_status.requested_resource_ids[0] = asset_status.active_resource_ids[0];
    asset_status.requested_resource_ids[1] = asset_status.active_resource_ids[1];
    asset_status.requested_resource_ids[2] = asset_status.active_resource_ids[2];
    asset_status.requested_resource_ids[3] = asset_status.active_resource_ids[3];
    asset_status.requested_resource_ids[4] = asset_status.active_resource_ids[4];
    asset_status.active_resource_ids[0] = AREA_RELOAD;
    asset_status.active_resource_ids[1] = AREA_RELOAD;
    asset_status.active_resource_ids[2] = AREA_RELOAD;
    asset_status.active_resource_ids[3] = AREA_RELOAD;
    asset_status.active_resource_ids[4] = AREA_RELOAD;
    asset_status.shift[0] = SHIFT_NONE;
    asset_status.shift[1] = SHIFT_NONE;
    asset_status.shift[2] = SHIFT_NONE;
    do {
        cd_request_yield();
        resource_advance_transition();
    } while (asset_status.loading != 0);
    tmd_set_slot(0, resource_tmd_arc_work);
    movie_state_reset();
    asset_status.handlers->on_load();
}

ADDRESS(0x80017aa4, 0x1d4)
void translate_active_world_positions(s32 dx, s32 dy, s32 dz)
{
    Effect *a;
    MapObject *b;
    Actor *c;
    MapPlacedEntry *d;
    u16 i;
    u16 j;
    u16 k;
    u16 l;

    cur_avatar_status.pos.vx += dx;
    cur_avatar_status.pos.vz += dz;
    cur_avatar_status.pos.vy += dy;
    a = effect_status.objects;
    i = 128;
    while (i--) {
        if (a->type != 0xff && (a->render_flags & 0xc) != 0xc) {
            a->pos.vx += dx;
            a->pos.vz += dz;
            a->pos.vy += dy;
        }
        a++;
    }
    b = map_objects_slots.objects;
    j = 0x18c;
    while (j--) {
        if (b->object_id != 0xff) {
            b->pos.vx += dx;
            b->pos.vz += dz;
            b->pos.vy += dy;
        }
        b++;
    }
    c = actor_status.objects;
    k = 200;
    while (k--) {
        if (c->slot_state != 0xff) {
            c->pos.vx += dx;
            c->pos.vz += dz;
            c->pos.vy += dy;
        }
        c++;
    }
    d = map_placed_entries;
    l = 128;
    while (l--) {
        if (d->model_index != 0xffff) {
            d->pos.vx += dx;
            d->pos.vz += dz;
            d->pos.vy += dy;
        }
        d++;
    }
}

/*
 * Asks for an area load: area0..area4 name the parts (0xff keeps a part, 200
 * in area4 only restarts the sequence), shift0..shift2 the origin shift.
 * While a load runs it waits for it (or for its CD request) first.
 * Not exact: want1/next3 and want2/next4 trade registers (t0/t1, a2/a3);
 * the code is otherwise instruction-identical.
 */
ADDRESS(0x80017c78, 0x668)
void resource_request_transition(u8 area0, u8 area1, u8 area2, u8 area3, u8 area4, s8 shift0, s8 shift1,
                                 s8 shift2)
{
    u8 want0;
    u8 want1;
    u8 want2;
    u8 want3;
    u8 want4;
    u8 next0;
    u8 next1;
    u8 next2;
    u8 next3;
    u8 next4;

    if (area4 == 200) {
        if (sound_status.seq_playing == 0) {
            audio_start_sequence();
        }
        return;
    }
    if (area0 == AREA_KEEP) {
        want0 = asset_status.active_resource_ids[0];
        next0 = asset_status.requested_resource_ids[0];
        if (area1 == AREA_KEEP) {
            want1 = asset_status.active_resource_ids[1];
            next1 = asset_status.requested_resource_ids[1];
        } else {
            want1 = next1 = area1;
        }
        if (area2 == AREA_KEEP) {
            want2 = asset_status.active_resource_ids[2];
            next2 = asset_status.requested_resource_ids[2];
        } else {
            want2 = next2 = area2;
        }
        if (area3 == AREA_KEEP) {
            want3 = asset_status.active_resource_ids[3];
            next3 = asset_status.requested_resource_ids[3];
        } else {
            want3 = next3 = area3;
        }
        if (area4 == AREA_KEEP) {
            want4 = asset_status.active_resource_ids[4];
            next4 = asset_status.requested_resource_ids[4];
        } else {
            want4 = next4 = area4;
        }
    } else {
        next1 = area1;
        next2 = area2;
        next3 = area3;
        next4 = area4;
        next0 = area0;
        want0 = area0;
        want1 = asset_status.active_resource_ids[1];
        want2 = asset_status.active_resource_ids[2];
        want3 = asset_status.active_resource_ids[3];
        want4 = asset_status.active_resource_ids[4];
    }
    if (asset_status.loading != 0) {
        goto busy;
    }
    if (asset_status.active_resource_ids[0] == want0 && asset_status.active_resource_ids[1] == want1
        && asset_status.active_resource_ids[2] == want2 && asset_status.active_resource_ids[3] == want3
        && asset_status.active_resource_ids[4] == want4) {
        return;
    }
start:
    if (asset_status.active_resource_ids[0] != AREA_RELOAD && asset_status.active_resource_ids[0] != want0) {
        map_reveal_visited_rects();
        repeat_store_word((u32 *)map_visited_cells, 0, 200);
        event_world_state_save_slot(asset_status.active_resource_ids[0]);
    }
    if (cur_evt_state.flags[0] < want0) {
        cur_evt_state.flags[0] = want0;
    }
    cur_evt_state.area_visited[want0] = 1;
    asset_status.loading = 1;
    asset_status.step = 0;
    asset_status.requested_resource_ids[0] = area0;
    asset_status.requested_resource_ids[1] = area1;
    asset_status.requested_resource_ids[2] = area2;
    asset_status.requested_resource_ids[3] = area3;
    asset_status.requested_resource_ids[4] = area4;
    asset_status.shift[0] = shift0;
    asset_status.shift[1] = shift1;
    asset_status.shift[2] = shift2;
    asset_status.world_shift_applied = 0;
    if (area1 == AREA_KEEP) {
        asset_status.tmd_object_limit_active = 0;
    } else {
        asset_status.tmd_object_limit_active = 1;
    }
    return;
busy:
    if (((s16)asset_status.loading != 1 || asset_status.requested_resource_ids[0] == next0)
        && asset_status.requested_resource_ids[1] == next1 && asset_status.requested_resource_ids[2] == next2
        && asset_status.requested_resource_ids[3] == next3
        && asset_status.requested_resource_ids[4] == next4) {
        return;
    }
    if (asset_status.requested_resource_ids[0] == AREA_KEEP && asset_status.active_resource_ids[0] == want0) {
        return;
    }
    if (asset_status.requested_resource_ids[1] == AREA_KEEP && asset_status.active_resource_ids[1] == area1) {
        return;
    }
    if (asset_status.requested_resource_ids[2] == AREA_KEEP && asset_status.active_resource_ids[2] == area2) {
        return;
    }
    if (asset_status.requested_resource_ids[3] == AREA_KEEP && asset_status.active_resource_ids[3] == area3) {
        return;
    }
    if (asset_status.requested_resource_ids[4] == AREA_KEEP && asset_status.active_resource_ids[4] == area4) {
        return;
    }
    if ((asset_status.requested_resource_ids[0] != AREA_KEEP && area0 == AREA_KEEP)
        || (asset_status.requested_resource_ids[1] != AREA_KEEP && area1 == AREA_KEEP)
        || (asset_status.requested_resource_ids[2] != AREA_KEEP && area2 == AREA_KEEP)
        || (asset_status.requested_resource_ids[3] != AREA_KEEP && area3 == AREA_KEEP)
        || (asset_status.requested_resource_ids[4] != AREA_KEEP && area4 == AREA_KEEP)) {
        while (asset_status.loading != 0) {
            cd_request_yield();
            resource_advance_transition();
        }
    } else {
        while (asset_status.step == STEP_WAIT) {
            cd_request_yield();
        }
    }
    ExitCriticalSection();
    if (asset_status.world_shift_applied != 0 && area0 != AREA_KEEP && shift0 == SHIFT_NONE
        && asset_status.requested_resource_ids[0] != area0) {
        shift0 = -asset_status.shift[0];
        shift2 = -asset_status.shift[2];
        shift1 = -asset_status.shift[1];
    }
    goto start;
}

/* CD request callbacks of the area loader: each sets the next step. */
ADDRESS(0x800182e0, 0x18)
void resource_transition_set_phase_1(void)
{
    asset_status.step = 1;
}

ADDRESS(0x800182f8, 0x18)
void resource_transition_set_phase_2(void)
{
    asset_status.step = 2;
}

ADDRESS(0x80018310, 0x18)
void resource_transition_set_phase_3(void)
{
    asset_status.step = 3;
}

ADDRESS(0x80018328, 0x18)
void resource_transition_set_phase_4(void)
{
    asset_status.step = 4;
}

ADDRESS(0x80018340, 0x18)
void resource_transition_set_phase_6(void)
{
    asset_status.step = 6;
}

ADDRESS(0x80018358, 0x8fc)
void resource_advance_transition(void)
{
    u32 *data = (u32 *)stream_work_buffer;
    u32 *part;
    s32 i;
    u16 j;
    MapObject *obj44;
    Actor *obj88;
    MapCell *cell;
    u8 *block;

    switch ((s16)asset_status.loading) {
    case 0:
        return;
    case 1:
        break;
    default:
        return;
    }
    switch (asset_status.step) {
    case 0:
        if (asset_status.requested_resource_ids[0] == AREA_KEEP) {
            goto parts;
        }
        asset_status.step = STEP_WAIT;
        obj44 = map_objects_slots.objects;
        for (i = 395; i != -1; i--) {
            obj44->action = 0xff;
            obj44++;
        }
        cd_archive_queue_read(FILE_FDAT, asset_status.requested_resource_ids[0] * 3, stream_work_buffer,
                              resource_transition_set_phase_1);
        break;
    case 1:
        asset_status.step = STEP_WAIT;
        asset_status.handlers = &callback_default_table;
        cd_archive_queue_read(FILE_FDAT, asset_status.requested_resource_ids[0] * 3 + 2,
                              (u8 *)fdat_window_base[0], resource_transition_set_phase_2);
        resource_copy_words((u32 *)map_world.cells, data + 1, 0x3e80);
        data = (u32 *)((u8 *)data + (*data + 4));
        resource_copy_words((u32 *)map_world.shape_bank, data + 1, 0x800);
        repeat_store_word((u32 *)map_visited_cells, 0, 200);
        if (asset_status.shift[0] != SHIFT_NONE) {
            asset_status.world_shift_applied = 1;
            translate_active_world_positions(asset_status.shift[0] << 11, -(asset_status.shift[2] << 7),
                                             asset_status.shift[1] << 11);
        }
        asset_status.current_area = asset_status.requested_resource_ids[0];
        break;
    case 2:
        asset_status.step = STEP_WAIT;
        cd_archive_queue_read(FILE_FDAT, asset_status.requested_resource_ids[0] * 3 + 1, stream_work_buffer,
                              resource_transition_set_phase_3);
        asset_status.handlers = (FdatHandlers *)fdat_window_handlers;

        break;
    case 3:
    parts:
        if (asset_status.requested_resource_ids[1] != AREA_KEEP) {
            asset_status.step = STEP_WAIT;
            cd_archive_queue_read(5, asset_status.requested_resource_ids[1], resource_tmd_arc_work,
                                  resource_transition_set_phase_4);
        }
        if (asset_status.requested_resource_ids[0] != AREA_KEEP) {
            part = (u32 *)stream_work_buffer;
            obj88 = actor_status.objects;
            for (i = 199; i != -1; i--) {
                if (obj88->animation_cache != 0) {
                    pool_record_release(obj88->animation_cache);
                }
                obj88->slot_state = 0xff;
                obj88++;
            }
            obj44 = map_objects_slots.objects;
            for (i = 395; i != -1; i--) {
                if (obj44->arg.w.animation_cache != 0) {
                    pool_record_release((AnimSlot *)obj44->arg.w.animation_cache);
                }
                switch (obj44->action) {
                case 0x20:
                    memory_free((void *)obj44->arg.w.unk40);
                    obj44->action = 0xff;
                    break;
                case 0x22:
                    memory_free((void *)obj44->arg.w.unk40);
                    obj44->action = 0xff;
                    break;
                }
                obj44->object_id = 0xff;
                obj44++;
            }
            if (asset_status.shift[0] != SHIFT_NONE) {
                cell = map_world.cells[0];
                for (i = 80 * 80; i != 0; i--) {
                    cell->layer[0].quarter_turns &= 3;
                    cell->layer[1].quarter_turns &= 3;
                    cell++;
                }
                map_cell_add_layer_occupancy(cur_avatar_status.pos.vx, cur_avatar_status.pos.vz, 800, 1);
            }
            resource_copy_words((u32 *)actor_status.groups, part + 1, 0xcb0);
            actor_fixup_group_targets();
            part = (u32 *)((u8 *)part + (*part + 4));
            actor_load_records((ActorPlacement *)(part + 1));
            part = (u32 *)((u8 *)part + (*part + 4));
            resource_copy_words((u32 *)&map_objects_slots.defs[300], part + 1, 0xc0);
            part = (u32 *)((u8 *)part + (*part + 4));
            map_object_initialize_from_placements((MapObjectPlacement *)(part + 1));
            part = (u32 *)((u8 *)part + (*part + 4));
            map_placed_expand_sources((MapPlacedSource *)(part + 1));
            part = (u32 *)((u8 *)part + (*part + 4));
            resource_copy_words((u32 *)map_rects, part + 1, 0xa0);
            event_world_state_restore_slot(asset_status.requested_resource_ids[0]);
            repeat_store_word((u32 *)map_visited_cells, 0, 200);
            if (asset_status.active_resource_ids[0] != asset_status.requested_resource_ids[0]
                && asset_status.active_resource_ids[0] != AREA_RELOAD) {
                for (j = asset_status.active_resource_ids[0] * 32
                     + 556; j < asset_status.active_resource_ids[0] * 32 + 588;
                     j++) {
                    block = scene_display_sys.models[j];
                    if (block != NULL && memory_block_kind(block) != 3) {
                        memory_block_set_kind(block, 1);
                    }
                }
            }
            movie_state_reset();
            asset_status.handlers->on_load();
            cur_avatar_status.force_actor_lifecycle_refresh = 1;
            asset_status.area_records_loaded = 1;
        }
        if (asset_status.requested_resource_ids[1] != AREA_KEEP) {
            break;
        }
    case 4:
        if (asset_status.requested_resource_ids[2] != AREA_KEEP) {
            cd_map_stream_read(3, asset_status.requested_resource_ids[2]);
        }
        asset_status.tmd_object_limit_active = 0;
        if (asset_status.requested_resource_ids[3] == AREA_KEEP
            && asset_status.requested_resource_ids[4] == AREA_KEEP) {
        done:
            asset_status.area_records_loaded = 0;
            if (asset_status.requested_resource_ids[0] != AREA_KEEP) {
                asset_status.active_resource_ids[0] = asset_status.requested_resource_ids[0];
            }
            if (asset_status.requested_resource_ids[1] != AREA_KEEP) {
                asset_status.active_resource_ids[1] = asset_status.requested_resource_ids[1];
            }
            if (asset_status.requested_resource_ids[2] != AREA_KEEP) {
                asset_status.active_resource_ids[2] = asset_status.requested_resource_ids[2];
            }
            if (asset_status.requested_resource_ids[3] != AREA_KEEP) {
                asset_status.active_resource_ids[3] = asset_status.requested_resource_ids[3];
            }
            if (asset_status.requested_resource_ids[4] != AREA_KEEP) {
                asset_status.active_resource_ids[4] = asset_status.requested_resource_ids[4];
            }
            asset_status.loading = 0;
            break;
        }
        asset_status.step = 5;
        sound_status.seq_ready = 0;
        asset_status.sequence_fade_volume = cur_avatar_status.bgm_volume;
    case 5:
        if (sound_status.seq_playing != 0) {
            asset_status.sequence_fade_volume -= 2;
            if (asset_status.sequence_fade_volume <= 0) {
                asset_status.sequence_fade_volume = 0;
            }
            SsSeqSetVol(sound_status.seq_id, asset_status.sequence_fade_volume,
                        asset_status.sequence_fade_volume);
            if (asset_status.sequence_fade_volume != 0) {
                break;
            }
            SsSeqStop(sound_status.seq_id);
            SsSeqClose(sound_status.seq_id);
            sound_status.seq_playing = 0;
        }
        if (sound_status.banks[1].vab_id != -1) {
            sound_status.banks[1].slot = NULL;
        }
        if (asset_status.requested_resource_ids[3] != AREA_KEEP) {
            audio_queue_vab_stream(2, asset_status.requested_resource_ids[3] + 1, 1);
        }
        if (asset_status.requested_resource_ids[4] == AREA_KEEP) {
            goto done;
        }
        cd_archive_queue_read(2, (u8)(asset_status.requested_resource_ids[4] % 100) + 546,
                              (u8 *)sound_status.seq_buffer, resource_transition_set_phase_6);
        asset_status.step = STEP_WAIT;
        break;
    case 6:
        if (asset_status.requested_resource_ids[4] < 100) {
            sound_status.seq_ready = 1;
            audio_start_sequence();
        }
        goto done;
    }
}
