#ifndef GAME_FUNCS_H
#define GAME_FUNCS_H

/*
 * GAME.EXE functions band A calls that are not reconstructed yet, plus two
 * unnamed SDK-band members (sdk-unmatched-version). Signatures follow the
 * band A call sites only.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

void func_80017664(void);
void func_8001796c(void);
void func_80018358(void);
void func_80018cd0(void);
u16 *func_80018f8c(u16 *dst, u16 *src, s32 count);
u32 *func_80018fbc(u32 *dst, u32 value, s32 count);
void func_8001937c(void *arena, s32 size);
void *func_800194f0(u32 size);
void func_80019ab4(void);
void func_8001a438(void);
void func_8001a5c0(void);
s32 func_8001fa60(void);
void func_80027f2c(void);
void func_80028f90(void);
void func_8002b330(VECTOR *pos, SVECTOR *angle);
void func_8002b760(void);
void func_80030e14(void);
void func_80030fcc(void);
void func_800324f0(s32 x, s32 y, s32 z);
void func_80034180(void);
void func_800341e8(void);
void func_80034300(void);
void func_800350fc(RECT *rect, s32 a1, s32 a2, s32 a3);
void func_80035394(void);
void func_8003c9dc(void);
void func_800422b8(VECTOR *pos, SVECTOR *angle);
void func_8004485c(void);
void func_80046c00(s32 a0, s32 a1, s32 a2, s32 a3);
void func_80047010(void);
void func_8004b8fc(void);
void func_80052e5c(void);
void func_8005bbf4(void);
void func_8005bc50(void);
void func_8005ea64(void);
void func_8005eb20(void);
void func_80060800(void);
void func_80061918(void);
void func_80061940(void);
void func_80061bc4(void);

/* LIBSND-shaped SDK members: SsVabOpenHeadSticky(addr, vab_id, spu_addr) and
 * SsUtKeyOn(vab, prog, tone, note, fine, voll, volr) by their call shape only. */
short func_80068f7c(unsigned char *addr, short vab_id, unsigned long spu_addr);
short func_8006dce8(short vab_id, short program, short tone, short note, short fine,
                    short left, short right);

#endif
