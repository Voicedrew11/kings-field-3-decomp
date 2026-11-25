/* Slot 2 (shared fragment, FDAT_077.BIN's claim): item 0x85 only shows message 0x23. */
ADDRESS(0x801e85a8, 0x34)
void area_use_item_notify_no_target(VECTOR *pos, SVECTOR *angle, s32 item)
{
    if (item == 0x85) {
        cur_evt_state.interaction_handled = 1;
        notify_enqueue(0x23);
    }
}
