/*
 * Handler slot 0 of 17 modules (shared fragment, FDAT_002.BIN's claim):
 * GAME.EXE's func_8005e2d0 calls it with the point it tests (the position
 * raised by 500) and the facing. Each object func_80045c7c finds in front
 * whose definition kind is 9 or 0x15 and that is armed (unk3c) either
 * triggers its linked object (unk3a) or, with none, shows its message.
 */
ADDRESS(0x801e83b0, 0x208)
void fdat_801e83b0(VECTOR *pos, SVECTOR *angle)
{
    s32 i;
    Object44 *obj;
    Object44 *link;
    Object44Def *def;

    for (i = 0; (i = func_80045c7c(i, pos, 800, 2500, angle->vy, 0x200)) != -1; i++) {
        obj = &DAT_8018fb3c.objects[i];
        def = &DAT_8018fb3c.defs[obj->unk6];
        DAT_801ba984.unk0 = 1;
        if ((def->unk0 == 9 || def->unk0 == 0x15) && obj->arg.b.unk3c != 0) {
            if (obj->arg.b.unk3a.index == 0xffff) {
            message:
                if (obj->arg.b.unk3f != 0) {
                    if (obj->arg.b.unk3f < 35) {
                        func_80041eec(obj->arg.b.unk3f);
                    }
                    if (obj->unk6 == 212 || obj->unk6 == 213 || obj->unk6 == 0xf7) {
                        func_80041eec(0x1a);
                    } else if (obj->unk6 == 0xb7) {
                        func_80041eec(0x1e);
                    } else if (obj->unk6 == 0xf5) {
                        func_80041eec(0xe);
                    } else if (obj->unk6 == 0xba) {
                        func_80041eec(0x22);
                    } else if (obj->unk6 >= 190 && obj->unk6 < 198) {
                        func_80041eec(8);
                    }
                }
            } else {
                link = &DAT_8018fb3c.objects[obj->arg.b.unk3a.index];
                if (link->unk6 == 0xff) {
                    goto message;
                }
                link->unk0 = obj->arg.b.unk40;
                link->arg.b.unk38 = 0xff;
                func_8005db30(link);
                link->unk0 = 0;
                link->arg.b.unk38 = 0;
                if (link->unk6 == 0xff) {
                    obj->arg.b.unk3a.index = 0xffff;
                }
            }
        }
    }
}
