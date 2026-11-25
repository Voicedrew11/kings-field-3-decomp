/*
 * GAME.EXE: Actor script record scanning (WIP module): stepping an
 * Actor's animation time to a target while rendering, and finding the
 * entry point of its first script record. The 0xf1/0xf2/0xfe/0xff bytes
 * after +0x14 are the record's markers; +0x10 is the saved entry offset.
 * event_target_stream_execute (switch) follows.
 */
#include <kf/lib/address.h>
#include <kf/lib/null.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <game/math.h>
#include <game/data.h>
#include <game/render.h>
#include <game/misc.h>

/* Sets obj's animation and time, then steps the time by step until it is within step / 2 of target. */
ADDRESS(0x8005c0d4, 0xb8)
void actor_animation_seek_phase(Actor *obj, u8 anim, u16 time, s32 target, u16 step)
{
    u32 s;
    s16 half;
    s16 low;

    if (step != 0) {
        s = step & 0xfffe;
        half = s >> 1;
        low = target - half;
        obj->animation_id = anim;
        obj->animation_phase = time;
        while (!angle_within_tolerance(obj->animation_phase, low, half)) {
            obj->animation_phase = (s + obj->animation_phase) & 0xfff;
            render_game_frame(NULL, NULL);
        }
        obj->animation_phase = low & 0xfff;
        render_game_frame(NULL, NULL);
    }
}

/* Offset after the 0xf2 marker for id, or script[16] at the 0xff end. */
ADDRESS(0x8005c18c, 0x5c)
u8 event_target_stream_find_marker(u8 *script, u8 id)
{
    u8 *p;

    p = script + 20;
    for (;;) {
        switch (*p++) {
        case 0xff:
            return script[16];
        case 0xf2:
            if (*p == id) {
                return p - 19 - script;
            }
            p++;
            break;
        }
    }
}

/* Entry point of obj's first script record: the first 0xf1 entry whose flag matches, else the saved one. */
ADDRESS(0x8005c1e8, 0x120)
u8 *event_target_stream_resolve_cursor(Actor *obj)
{
    u8 *script;
    u8 *p;
    u8 n;

    script = actor_status.groups[obj->group_index].targets[0];
    p = script + 20;
    for (;;) {
        switch (*p) {
        case 0xfe:
            p++;
            if (script[16] == 0) {
                script[16] = p - (script + 20);
                return p;
            }
        found:
            return script + (script[16] + 20);
        case 0xf1:
            if (cur_evt_state.flags[p[1]] == p[2]) {
                n = event_target_stream_find_marker(script, p[3]);
                if (script[16] < n) {
                    script[16] = n;
                }
                cur_evt_state.stream_actor_definition_id = obj->definition_id;
                script[19] = 0;
                goto found;
            }
            p += 4;
            break;
        }
    }
}
