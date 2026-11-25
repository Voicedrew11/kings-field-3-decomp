#ifndef GAME_MISC_H
#define GAME_MISC_H

/*
 * GAME.EXE functions after the Object4c modules (src/game/misc_5bd34.c,
 * object44_move.c, ...; WIP names and module edges).
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <psyq/sdk.h>
#include <psyq/cd.h>
#include <game/data.h>

void func_8005bd34(void);
void func_8005bd58(s32 se);
s32 func_8005bd78(VECTOR *pos, SVECTOR *angle);
void func_8005be18(void);
void func_8005be20(void);
void func_8005be28(s32 x, s32 y, s32 z, s32 pitch, s32 yaw, s32 dy, s32 dz, VECTOR *out);
void func_8005bee0(Object44 *obj, VECTOR *from, VECTOR *to, SVECTOR *afrom, SVECTOR *ato, s32 t);
void func_8005bfc4(Object44 *obj, VECTOR *from, VECTOR *to, s32 t);
void func_8005c04c(Object44 *obj, SVECTOR *afrom, SVECTOR *ato, s32 t);
void func_8005c0d4(Object88 *obj, u8 anim, u16 time, s32 target, u16 step);
u8 func_8005c18c(u8 *script, u8 id);
void func_8005c308(Object88 *obj);
u8 *func_8005c1e8(Object88 *obj);
void func_8005c7e8(Object44 *obj, s32 id);

/* A start position of func_8005c864's table: areas, map cell and height, yaw. */
typedef struct Record08 {
    u8 area;        /* resource_request_transition's third area (0xff: none) */
    u8 area3;
    u8 area4;
    u8 x;           /* map cell x */
    u8 z;           /* map cell z */
    u8 y;           /* height / 128 */
    u16 yaw;        /* negated */
} Record08;

void func_8005c864(s32 n);
s32 func_8005d7bc(s32 id);
s32 func_8005d7f8(s32 id);
s32 func_8005d898(s32 id);
void func_8005d948(s32 step, s32 r0, s32 g0, s32 b0, s32 r1, s32 g1, s32 b1);
void func_8005da94(void);
void func_8005ea64(void);
void func_8005ee58(u8 **out);
void func_8005eea8(s32 delta);
void func_8005ef18(u8 **in);
void func_8005ef64(s32 delta);

void func_80060800(void);
void func_800608ec(u8 id, u8 slot);
u16 func_80060a10(u8 n, u8 slot);
u8 func_80060b0c(void);
u8 func_80060ba8(u8 n);
u8 func_80060c54(u8 n);
u8 func_80060cd0(u8 id);
void func_80060d20(void);
void func_80061364(void);
void func_800614d4(CdlLOC *loc);
void func_80061550(void);
s32 func_80061650(void);
u_long *func_800616dc(DECENV *dec);
void func_8006179c(DECENV *dec);
void func_8006182c(CdlLOC *loc);
void func_80061878(void);
void func_80061894(void);
void func_80061918(void);
void func_80061bc4(void);

/* Game loop exit word (defined in src/game/main.c). */
extern s32 DAT_8009c3f8;

#endif
