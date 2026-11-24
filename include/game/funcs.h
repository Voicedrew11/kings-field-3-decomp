#ifndef GAME_FUNCS_H
#define GAME_FUNCS_H

/*
 * GAME.EXE functions the reconstructed units call that are not reconstructed
 * yet, plus two unnamed SDK-band members (sdk-unmatched-version). Signatures
 * follow the reconstructed call sites only.
 */
#include <kf/lib/types.h>
#include <sys/types.h>
#include <LIBGTE.H>
#include <LIBGPU.H>

void func_80017c78(u8 area0, u8 area1, u8 area2, u8 area3, u8 area4, s8 shift0, s8 shift1,
                   s8 shift2);
void func_80018358(void);
void func_8001f4c0(void);
s32 func_8001fa60(void);
void func_80030a6c(SVECTOR *knockback);
void func_80030e14(void);
void func_80030fcc(void);
void func_800324f0(s32 x, s32 y, s32 z);
void func_8004485c(void);
void func_80046c00(s32 a0, s32 a1, s32 a2, s32 a3);
void func_80047010(void);
void func_8004b8fc(void);
void func_80052e5c(void);
/* Called with five or six arguments (no prototype). */
void func_80053c84();
void func_8005bbf4(void);
void func_8005bc50(void);
s32 func_8005d7bc(s32 n);
void func_8005d7f8(s32 n);
void func_8005d898(s32 n);
void func_8005ea64(void);
void func_8005eb20(void);
void func_8005f7bc(u8 *data);
void func_8005ffd0(u8 *data);
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
