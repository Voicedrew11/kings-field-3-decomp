/*
 * Handler slot 0 of 17 modules (shared fragment, FDAT_002.BIN's claim):
 * GAME.EXE's event_world_dispatch_interaction calls it with the point it tests (the position
 * raised by 500) and the facing. Each object map_object_find_interaction_target finds in front
 * whose definition kind is 9 or 0x15 and that is armed (unk3c) either
 * triggers its linked object (unk3a) or, with none, shows its message.
 */
ADDRESS(0x801e83b0, 0x208)
void area_interact_item_containers(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    MapObject *obj;
    MapObject *link;
    MapObjectDef *def;

    for (i = 0; (i = map_object_find_interaction_target(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &map_objects_slots.objects[i];
        def = &map_objects_slots.defs[obj->object_id];
        cur_evt_state.unk0 = 1;
        if ((def->action == 9 || def->action == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        notify_enqueue(obj->arg.b.unk3f);
                    }
                    if (obj->object_id == 212 || obj->object_id == 213 || obj->object_id == 0xf7) {
                        notify_enqueue(0x1a);
                    } else if (obj->object_id == 0xb7) {
                        notify_enqueue(0x1e);
                    } else if (obj->object_id == 0xf5) {
                        notify_enqueue(0xe);
                    } else if (obj->object_id == 0xba) {
                        notify_enqueue(0x22);
                    } else if (obj->object_id >= 190 && obj->object_id < 198) {
                        notify_enqueue(8);
                    }
                }
            } else {
                link = &map_objects_slots.objects[obj->arg.b.unk3a.index];
                if (link->object_id == 0xff) {
                    goto message;
                }
                link->layer_mask = obj->arg.b.unk40;
                link->arg.b.unk38 = 0xff;
                event_map_object_interact(link);
                link->layer_mask = 0;
                link->arg.b.unk38 = 0;
                if (link->object_id == 0xff) {
                    obj->arg.b.unk3a.index = 0xffff;
                }
            }
        }
    }
}
