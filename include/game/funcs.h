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

s32 func_8001fa60(void);
void func_80030fcc(void);
s32 func_800324f0(s32 x, s32 y, s32 z);
void func_80047010(void);
void func_8005eb20(void);
void func_8005efd4(s32 area);
void func_8005f444(s32 area);
void func_8005f7bc(u8 *data);
void func_8005ffd0(u8 *data);
void func_80061940(void);

/* LIBSND-shaped SDK members: SsVabOpenHeadSticky(addr, vab_id, spu_addr) and
 * SsUtKeyOn(vab, prog, tone, note, fine, voll, volr) by their call shape only. */
short func_80068f7c(unsigned char *addr, short vab_id, unsigned long spu_addr);
short func_8006dce8(short vab_id, short program, short tone, short note, short fine,
                    short left, short right);

#endif
