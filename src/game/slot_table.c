/*
 * GAME.EXE: the slot tables of conversation_records (WIP module): building the
 * ConversationRecord records and their runs of slot bytes from the copied ConversationSource
 * table, marking a slot used, and the searches and counts over the runs.
 * movie_play_stream (the in-game movie player) follows.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <sys/types.h>
#include <game/data.h>
#include <game/misc.h>

/* Builds the ConversationRecord records and an empty run of count slots for each, then marks slots 0-3 of id
 * 0x20. */
ADDRESS(0x80060800, 0xec)
void conversation_records_initialize(void)
{
    ConversationSource *src;
    ConversationRecord *dst;
    u8 *p;
    s32 i;
    s32 n;

    p = conversation_records.page_states;
    dst = conversation_records.records;
    src = conversation_records.sources;
    for (i = 47; i != -1; i--) {
        if (src->id == 0xff) {
            break;
        }
        dst->id = src->id;
        dst->first_entry = src->first_entry;
        dst->offset = p - conversation_records.page_states;
        n = src->count;
        while (--n != -1) {
            *p++ = 0xff;
        }
        *p++ = 0xff;
        dst++;
        src++;
    }
    dst->id = 0xff;
    conversation_record_mark_page(0x20, 0);
    conversation_record_mark_page(0x20, 1);
    conversation_record_mark_page(0x20, 2);
    conversation_record_mark_page(0x20, 3);
}

/*
 * Marks slot of id's run used (3) and frees (0) the 0xff slots before it.
 *
 * Not exact: retail leaves the constants 3 and -1 inside the record loop.
 * loop.c moves a constant out only while threshold times savings times
 * lifetime is at least the loop's insn count, with the threshold at 52 less
 * 3 per move; retail's loop had 41 to 43 insns when loop.c ran and this
 * source has 36, so both are moved and three registers differ.
 */
ADDRESS(0x800608ec, 0x124)
void conversation_record_mark_page(u8 id, u8 slot)
{
    ConversationRecord *t;
    u8 *p;
    s32 n;
    u8 k;
    u8 c;

    t = conversation_records.records;
    n = 48;
    if (id == 1 || id == 2) {
        id = 1;
    }
    if (id == 7 || id == 0x25) {
        id = 7;
    }
    if (id == 0xf || id == 0x24) {
        id = 0xf;
    }
    if (id == 0x20 || id == 0x21 || id == 0x22) {
        id = 0x20;
    }
    while (--n != -1) {
        if (t->id == 0xff) {
            return;
        }
        if (t->id == id) {
            p = conversation_records.page_states + slot + t->offset;
            *p = 3;
            k = 0;
            c = slot;
            while (c--) {
                k++;
                if (p[-k] == 0xff) {
                    p[-k] = 0;
                }
            }
            return;
        }
        t++;
    }
}

/* Index in the run of the n-th id's slot after slot (0xfffd/0xfffe/0xffff: none). */
ADDRESS(0x80060a10, 0xfc)
u16 conversation_record_page_entry(u8 n, u8 slot)
{
    ConversationRecord *t;
    u8 *p;

    t = &conversation_records.records[conversation_records_find_character(conversation_records_nth_character(n))];
    if (t->id == 0xff) {
        return 0xfffe;
    }
    if (t->offset != 0xffff) {
        p = conversation_records.page_states + t->offset;
        if (*p != 0xff) {
            do {
                if ((*p & 2) && (*p & 1)) {
                    if (slot == 0) {
                        return t->first_entry + (p - conversation_records.page_states - t->offset);
                    }
                } else if (slot == 0) {
                    return 0xfffd;
                }
                p++;
                slot--;
            } while (*p != 0xff);
            return 0xffff;
        }
    }
    return 0xffff;
}

/* Number of runs with a used slot. */
ADDRESS(0x80060b0c, 0x9c)
u8 conversation_records_count_characters(void)
{
    ConversationRecord *t;
    u8 *p;
    u8 n;

    n = 0;
    for (t = conversation_records.records; t->id != 0xff; t++) {
        if (t->offset == 0xffff) {
            continue;
        }
        p = conversation_records.page_states + t->offset;
        while (*p++ != 0xff) {
            if ((*p & 2) && (*p & 1)) {
                n++;
                break;
            }
        }
    }
    return n;
}

/* Used slots in the n-th id's run. */
ADDRESS(0x80060ba8, 0xac)
u8 conversation_record_count_pages(u8 n)
{
    ConversationRecord *t;
    u8 *p;
    u8 count;

    count = 0;
    t = &conversation_records.records[conversation_records_find_character(conversation_records_nth_character(n))];
    if (t->id != 0xff && t->offset != 0xffff) {
        p = conversation_records.page_states + t->offset;
        while (*p++ != 0xff) {
            if ((*p & 2) && (*p & 1)) {
                count++;
            }
        }
    }
    return count;
}

/* Id of the n-th record whose run is in use, 0xff for none. */
ADDRESS(0x80060c54, 0x7c)
u8 conversation_records_nth_character(u8 n)
{
    ConversationRecord *t;
    u8 *p;
    u8 none;

    none = 0xff;
    for (t = conversation_records.records; t->id != 0xff; t++) {
        if (t->offset != 0xffff) {
            p = conversation_records.page_states + t->offset;
            if (*p != 0xff) {
                if (n == 0) {
                    return t->id;
                }
                n--;
            }
        }
    }
    return none;
}

/* Index of id's record, 0xff for none. */
ADDRESS(0x80060cd0, 0x50)
u8 conversation_records_find_character(u8 id)
{
    ConversationRecord *t;
    u8 i;

    i = 0;
    for (t = conversation_records.records; t->id != 0xff; t++) {
        if (t->id == id) {
            return i;
        }
        i++;
    }
    return 0xff;
}
