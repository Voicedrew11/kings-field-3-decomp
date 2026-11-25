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
u32 DAT_8009c3fc;

ADDRESS(0x80017664, 0x308)
void resource_initialize_game_assets(void)
{
    u32 *p = (u32 *)DAT_800c8954;

    DAT_8018fad4.area[1] = DAT_8018fad4.area[2] = DAT_8018fad4.area[3] = AREA_KEEP;
    DAT_8018fad4.loading = 0;
    DAT_8018fad4.step = 0;
    DAT_8018fad4.area[0] = 0;
    DAT_8018fad4.area[1] = 0;
    DAT_8018fad4.area[2] = 0;
    DAT_8018fad4.area[3] = 0;
    DAT_8018fad4.area[4] = 0;
    DAT_8018fad4.unk9 = 0;
    DAT_8009c3fc = DAT_800136e8[2] + 4;
    DAT_8009c400[0] = DAT_800136e8[2];
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
    resource_copy_words((u32 *)&DAT_8018fb3c, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)DAT_801d37a4, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words(DAT_801e64b8, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)DAT_8009f114, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)&DAT_801b77ec, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)DAT_80081c8c, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)&DAT_801e7edc, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)DAT_801b09fc.effects, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words((u32 *)&DAT_801983a8, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    resource_copy_words(DAT_80116154, p + 1, *p >> 2);
    p = (u32 *)((u8 *)p + (*p + 4));
    func_80042ba8(0, (u8 *)DAT_80116154);
    resource_copy_words(DAT_800aedc4, p + 1, *p >> 2);
    func_80042ba8(0x28, (u8 *)DAT_800aedc4);
    func_8002b64c();
}

ADDRESS(0x8001796c, 0x138)
void resource_run_initial_transition(void)
{
    DAT_8018fad4.loading = 1;
    DAT_8018fad4.step = 0;
    DAT_8018fad4.previous[0] = DAT_8018fad4.area[0];
    DAT_8018fad4.previous[1] = DAT_8018fad4.area[1];
    DAT_8018fad4.previous[2] = DAT_8018fad4.area[2];
    DAT_8018fad4.previous[3] = DAT_8018fad4.area[3];
    DAT_8018fad4.previous[4] = DAT_8018fad4.area[4];
    DAT_8018fad4.area[0] = AREA_RELOAD;
    DAT_8018fad4.area[1] = AREA_RELOAD;
    DAT_8018fad4.area[2] = AREA_RELOAD;
    DAT_8018fad4.area[3] = AREA_RELOAD;
    DAT_8018fad4.area[4] = AREA_RELOAD;
    DAT_8018fad4.shift[0] = SHIFT_NONE;
    DAT_8018fad4.shift[1] = SHIFT_NONE;
    DAT_8018fad4.shift[2] = SHIFT_NONE;
    do {
        cd_request_yield();
        resource_advance_transition();
    } while (DAT_8018fad4.loading != 0);
    func_80035c84(0, DAT_8014f164);
    func_80061918();
    DAT_8018fad4.handlers[5]();
}

ADDRESS(0x80017aa4, 0x1d4)
void translate_active_world_positions(s32 dx, s32 dy, s32 dz)
{
    Object4c *a;
    Object44 *b;
    Object88 *c;
    Effect *d;
    u16 i;
    u16 j;
    u16 k;
    u16 l;

    DAT_801b24e4.pos.vx += dx;
    DAT_801b24e4.pos.vz += dz;
    DAT_801b24e4.pos.vy += dy;
    a = DAT_801b77ec.objects;
    i = 128;
    while (i--) {
        if (a->unk0 != 0xff && (a->unk8 & 0xc) != 0xc) {
            a->pos.vx += dx;
            a->pos.vz += dz;
            a->pos.vy += dy;
        }
        a++;
    }
    b = DAT_8018fb3c.objects;
    j = 0x18c;
    while (j--) {
        if (b->unk6 != 0xff) {
            b->pos.vx += dx;
            b->pos.vz += dz;
            b->pos.vy += dy;
        }
        b++;
    }
    c = DAT_80185da8.objects;
    k = 200;
    while (k--) {
        if (c->unk0 != 0xff) {
            c->pos.vx += dx;
            c->pos.vz += dz;
            c->pos.vy += dy;
        }
        c++;
    }
    d = DAT_80182968;
    l = 128;
    while (l--) {
        if (d->id != 0xffff) {
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
        if (DAT_801b09fc.seq_playing == 0) {
            audio_start_sequence();
        }
        return;
    }
    if (area0 == AREA_KEEP) {
        want0 = DAT_8018fad4.area[0];
        next0 = DAT_8018fad4.previous[0];
        if (area1 == AREA_KEEP) {
            want1 = DAT_8018fad4.area[1];
            next1 = DAT_8018fad4.previous[1];
        } else {
            want1 = next1 = area1;
        }
        if (area2 == AREA_KEEP) {
            want2 = DAT_8018fad4.area[2];
            next2 = DAT_8018fad4.previous[2];
        } else {
            want2 = next2 = area2;
        }
        if (area3 == AREA_KEEP) {
            want3 = DAT_8018fad4.area[3];
            next3 = DAT_8018fad4.previous[3];
        } else {
            want3 = next3 = area3;
        }
        if (area4 == AREA_KEEP) {
            want4 = DAT_8018fad4.area[4];
            next4 = DAT_8018fad4.previous[4];
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
        want1 = DAT_8018fad4.area[1];
        want2 = DAT_8018fad4.area[2];
        want3 = DAT_8018fad4.area[3];
        want4 = DAT_8018fad4.area[4];
    }
    if (DAT_8018fad4.loading != 0) {
        goto busy;
    }
    if (DAT_8018fad4.area[0] == want0 && DAT_8018fad4.area[1] == want1
        && DAT_8018fad4.area[2] == want2 && DAT_8018fad4.area[3] == want3
        && DAT_8018fad4.area[4] == want4) {
        return;
    }
start:
    if (DAT_8018fad4.area[0] != AREA_RELOAD && DAT_8018fad4.area[0] != want0) {
        map_reveal_visited_rects();
        repeat_store_word((u32 *)DAT_801b594c, 0, 200);
        func_8005efd4(DAT_8018fad4.area[0]);
    }
    if (DAT_801ba984.unk4[0] < want0) {
        DAT_801ba984.unk4[0] = want0;
    }
    DAT_801ba984.unk8a[want0] = 1;
    DAT_8018fad4.loading = 1;
    DAT_8018fad4.step = 0;
    DAT_8018fad4.previous[0] = area0;
    DAT_8018fad4.previous[1] = area1;
    DAT_8018fad4.previous[2] = area2;
    DAT_8018fad4.previous[3] = area3;
    DAT_8018fad4.previous[4] = area4;
    DAT_8018fad4.shift[0] = shift0;
    DAT_8018fad4.shift[1] = shift1;
    DAT_8018fad4.shift[2] = shift2;
    DAT_8018fad4.unk15 = 0;
    if (area1 == AREA_KEEP) {
        DAT_8018fad4.unk16 = 0;
    } else {
        DAT_8018fad4.unk16 = 1;
    }
    return;
busy:
    if (((s16)DAT_8018fad4.loading != 1 || DAT_8018fad4.previous[0] == next0)
        && DAT_8018fad4.previous[1] == next1 && DAT_8018fad4.previous[2] == next2
        && DAT_8018fad4.previous[3] == next3 && DAT_8018fad4.previous[4] == next4) {
        return;
    }
    if (DAT_8018fad4.previous[0] == AREA_KEEP && DAT_8018fad4.area[0] == want0) {
        return;
    }
    if (DAT_8018fad4.previous[1] == AREA_KEEP && DAT_8018fad4.area[1] == area1) {
        return;
    }
    if (DAT_8018fad4.previous[2] == AREA_KEEP && DAT_8018fad4.area[2] == area2) {
        return;
    }
    if (DAT_8018fad4.previous[3] == AREA_KEEP && DAT_8018fad4.area[3] == area3) {
        return;
    }
    if (DAT_8018fad4.previous[4] == AREA_KEEP && DAT_8018fad4.area[4] == area4) {
        return;
    }
    if ((DAT_8018fad4.previous[0] != AREA_KEEP && area0 == AREA_KEEP)
        || (DAT_8018fad4.previous[1] != AREA_KEEP && area1 == AREA_KEEP)
        || (DAT_8018fad4.previous[2] != AREA_KEEP && area2 == AREA_KEEP)
        || (DAT_8018fad4.previous[3] != AREA_KEEP && area3 == AREA_KEEP)
        || (DAT_8018fad4.previous[4] != AREA_KEEP && area4 == AREA_KEEP)) {
        while (DAT_8018fad4.loading != 0) {
            cd_request_yield();
            resource_advance_transition();
        }
    } else {
        while (DAT_8018fad4.step == STEP_WAIT) {
            cd_request_yield();
        }
    }
    ExitCriticalSection();
    if (DAT_8018fad4.unk15 != 0 && area0 != AREA_KEEP && shift0 == SHIFT_NONE
        && DAT_8018fad4.previous[0] != area0) {
        shift0 = -DAT_8018fad4.shift[0];
        shift2 = -DAT_8018fad4.shift[2];
        shift1 = -DAT_8018fad4.shift[1];
    }
    goto start;
}

/* CD request callbacks of the area loader: each sets the next step. */
ADDRESS(0x800182e0, 0x18)
void resource_transition_set_phase_1(void)
{
    DAT_8018fad4.step = 1;
}

ADDRESS(0x800182f8, 0x18)
void resource_transition_set_phase_2(void)
{
    DAT_8018fad4.step = 2;
}

ADDRESS(0x80018310, 0x18)
void resource_transition_set_phase_3(void)
{
    DAT_8018fad4.step = 3;
}

ADDRESS(0x80018328, 0x18)
void resource_transition_set_phase_4(void)
{
    DAT_8018fad4.step = 4;
}

ADDRESS(0x80018340, 0x18)
void resource_transition_set_phase_6(void)
{
    DAT_8018fad4.step = 6;
}

ADDRESS(0x80018358, 0x8fc)
void resource_advance_transition(void)
{
    u32 *data = (u32 *)DAT_801c17a4;
    u32 *part;
    s32 i;
    u16 j;
    Object44 *obj44;
    Object88 *obj88;
    MapCell *cell;
    u8 *block;

    switch ((s16)DAT_8018fad4.loading) {
    case 0:
        return;
    case 1:
        break;
    default:
        return;
    }
    switch (DAT_8018fad4.step) {
    case 0:
        if (DAT_8018fad4.previous[0] == AREA_KEEP) {
            goto parts;
        }
        DAT_8018fad4.step = STEP_WAIT;
        obj44 = DAT_8018fb3c.objects;
        for (i = 395; i != -1; i--) {
            obj44->unk4 = 0xff;
            obj44++;
        }
        cd_archive_queue_read(FILE_FDAT, DAT_8018fad4.previous[0] * 3, DAT_801c17a4,
                              resource_transition_set_phase_1);
        break;
    case 1:
        DAT_8018fad4.step = STEP_WAIT;
        DAT_8018fad4.handlers = DAT_8007e4e4;
        cd_archive_queue_read(FILE_FDAT, DAT_8018fad4.previous[0] * 3 + 2, (u8 *)DAT_8009c400[0],
                              resource_transition_set_phase_2);
        resource_copy_words((u32 *)DAT_801d4464.cells, data + 1, 0x3e80);
        data = (u32 *)((u8 *)data + (*data + 4));
        resource_copy_words((u32 *)DAT_801d4464.unk10000, data + 1, 0x800);
        repeat_store_word((u32 *)DAT_801b594c, 0, 200);
        if (DAT_8018fad4.shift[0] != SHIFT_NONE) {
            DAT_8018fad4.unk15 = 1;
            translate_active_world_positions(DAT_8018fad4.shift[0] << 11, -(DAT_8018fad4.shift[2] << 7),
                                             DAT_8018fad4.shift[1] << 11);
        }
        DAT_8018fad4.unk9 = DAT_8018fad4.previous[0];
        break;
    case 2:
        DAT_8018fad4.step = STEP_WAIT;
        cd_archive_queue_read(FILE_FDAT, DAT_8018fad4.previous[0] * 3 + 1, DAT_801c17a4,
                              resource_transition_set_phase_3);
        DAT_8018fad4.handlers = (FdatHandler *)DAT_8009c3fc;
        break;
    case 3:
    parts:
        if (DAT_8018fad4.previous[1] != AREA_KEEP) {
            DAT_8018fad4.step = STEP_WAIT;
            cd_archive_queue_read(5, DAT_8018fad4.previous[1], DAT_8014f164, resource_transition_set_phase_4);
        }
        if (DAT_8018fad4.previous[0] != AREA_KEEP) {
            part = (u32 *)DAT_801c17a4;
            obj88 = DAT_80185da8.objects;
            for (i = 199; i != -1; i--) {
                if (obj88->unk5c != 0) {
                    func_80043894(obj88->unk5c);
                }
                obj88->unk0 = 0xff;
                obj88++;
            }
            obj44 = DAT_8018fb3c.objects;
            for (i = 395; i != -1; i--) {
                if (obj44->arg.w.unk34 != 0) {
                    func_80043894((AnimSlot *)obj44->arg.w.unk34);
                }
                switch (obj44->unk4) {
                case 0x20:
                    memory_free((void *)obj44->arg.w.unk40);
                    obj44->unk4 = 0xff;
                    break;
                case 0x22:
                    memory_free((void *)obj44->arg.w.unk40);
                    obj44->unk4 = 0xff;
                    break;
                }
                obj44->unk6 = 0xff;
                obj44++;
            }
            if (DAT_8018fad4.shift[0] != SHIFT_NONE) {
                cell = DAT_801d4464.cells[0];
                for (i = 80 * 80; i != 0; i--) {
                    cell->layer[0].unk2 &= 3;
                    cell->layer[1].unk2 &= 3;
                    cell++;
                }
                func_80033c4c(DAT_801b24e4.pos.vx, DAT_801b24e4.pos.vz, 800, 1);
            }
            resource_copy_words((u32 *)DAT_80185da8.defs, part + 1, 0xcb0);
            func_80053084();
            part = (u32 *)((u8 *)part + (*part + 4));
            func_800530f8((Object88Place *)(part + 1));
            part = (u32 *)((u8 *)part + (*part + 4));
            resource_copy_words((u32 *)&DAT_8018fb3c.defs[300], part + 1, 0xc0);
            part = (u32 *)((u8 *)part + (*part + 4));
            func_80044d9c((Object44Place *)(part + 1));
            part = (u32 *)((u8 *)part + (*part + 4));
            func_80043a08((EffectSpawn *)(part + 1));
            part = (u32 *)((u8 *)part + (*part + 4));
            resource_copy_words((u32 *)DAT_801ba6fc, part + 1, 0xa0);
            func_8005f444(DAT_8018fad4.previous[0]);
            repeat_store_word((u32 *)DAT_801b594c, 0, 200);
            if (DAT_8018fad4.area[0] != DAT_8018fad4.previous[0]
                && DAT_8018fad4.area[0] != AREA_RELOAD) {
                for (j = DAT_8018fad4.area[0] * 32 + 556; j < DAT_8018fad4.area[0] * 32 + 588;
                     j++) {
                    block = DAT_80199154.models[j];
                    if (block != NULL && memory_block_kind(block) != 3) {
                        memory_block_set_kind(block, 1);
                    }
                }
            }
            func_80061918();
            DAT_8018fad4.handlers[5]();
            DAT_801b24e4.unke = 1;
            DAT_8018fad4.unk17 = 1;
        }
        if (DAT_8018fad4.previous[1] != AREA_KEEP) {
            break;
        }
    case 4:
        if (DAT_8018fad4.previous[2] != AREA_KEEP) {
            cd_map_stream_read(3, DAT_8018fad4.previous[2]);
        }
        DAT_8018fad4.unk16 = 0;
        if (DAT_8018fad4.previous[3] == AREA_KEEP && DAT_8018fad4.previous[4] == AREA_KEEP) {
        done:
            DAT_8018fad4.unk17 = 0;
            if (DAT_8018fad4.previous[0] != AREA_KEEP) {
                DAT_8018fad4.area[0] = DAT_8018fad4.previous[0];
            }
            if (DAT_8018fad4.previous[1] != AREA_KEEP) {
                DAT_8018fad4.area[1] = DAT_8018fad4.previous[1];
            }
            if (DAT_8018fad4.previous[2] != AREA_KEEP) {
                DAT_8018fad4.area[2] = DAT_8018fad4.previous[2];
            }
            if (DAT_8018fad4.previous[3] != AREA_KEEP) {
                DAT_8018fad4.area[3] = DAT_8018fad4.previous[3];
            }
            if (DAT_8018fad4.previous[4] != AREA_KEEP) {
                DAT_8018fad4.area[4] = DAT_8018fad4.previous[4];
            }
            DAT_8018fad4.loading = 0;
            break;
        }
        DAT_8018fad4.step = 5;
        DAT_801b09fc.seq_ready = 0;
        DAT_8018fad4.unk1c = DAT_801b24e4.bgm_volume;
    case 5:
        if (DAT_801b09fc.seq_playing != 0) {
            DAT_8018fad4.unk1c -= 2;
            if (DAT_8018fad4.unk1c <= 0) {
                DAT_8018fad4.unk1c = 0;
            }
            SsSeqSetVol(DAT_801b09fc.seq_id, DAT_8018fad4.unk1c, DAT_8018fad4.unk1c);
            if (DAT_8018fad4.unk1c != 0) {
                break;
            }
            SsSeqStop(DAT_801b09fc.seq_id);
            SsSeqClose(DAT_801b09fc.seq_id);
            DAT_801b09fc.seq_playing = 0;
        }
        if (DAT_801b09fc.banks[1].vab_id != -1) {
            DAT_801b09fc.banks[1].slot = NULL;
        }
        if (DAT_8018fad4.previous[3] != AREA_KEEP) {
            audio_queue_vab_stream(2, DAT_8018fad4.previous[3] + 1, 1);
        }
        if (DAT_8018fad4.previous[4] == AREA_KEEP) {
            goto done;
        }
        cd_archive_queue_read(2, (u8)(DAT_8018fad4.previous[4] % 100) + 546, (u8 *)DAT_801b09fc.seq_data,
                              resource_transition_set_phase_6);
        DAT_8018fad4.step = STEP_WAIT;
        break;
    case 6:
        if (DAT_8018fad4.previous[4] < 100) {
            DAT_801b09fc.seq_ready = 1;
            audio_start_sequence();
        }
        goto done;
    }
}
