/*
 * PSX.EXE: the boot program. It loads and executes the three other programs
 * from the disc, starting with index 0 (OPEN.EXE), and after each one returns
 * reads the next index from the byte at 0x800102f0. Which program writes
 * that byte, and what reads the byte at 0x800102f8, is not yet traced.
 */
#include <kf/lib/address.h>
#include <kf/lib/types.h>
#include <psyq/kernel.h>

/*
 * 0x800102f0 and 0x800102f8 lie just past PSX.EXE's linked bss, inside its
 * load window and below the 0x80011000 load address of the other programs.
 * Only these fixed addresses are evidenced; whether the original spelled
 * numbers or link-time symbols is unknown.
 */
#define NEXT_PROGRAM_BYTE ((u8 *)0x800102f0)
#define MAILBOX_BYTE_8 ((u8 *)0x800102f8)

RODATA(0x80010000, 0x35)

/* GCC 2.6.0 emits the literals in reverse, so .rdata holds END, GAME, OPEN. */
static char *program_paths[] = {
    "cdrom:OPEN.EXE;1", "cdrom:GAME.EXE;1", "cdrom:END.EXE;1",
};
int program_index = 0;
u8 *next_program = NEXT_PROGRAM_BYTE;
struct EXEC program_header;

ADDRESS(0x80010038, 0xe0)
int main(void)
{
    SetMem(2);
    _96_remove();
    _96_init();
    *next_program = 0;
    *MAILBOX_BYTE_8 = 0;
    for (;;) {
        if (Load(program_paths[program_index], &program_header) == 1) {
            _96_remove();
            program_header.s_addr = 0;
            program_header.s_size = 0;
            EnterCriticalSection();
            Exec(&program_header, 0, 0);
            program_index = *next_program;
            _96_init();
        }
    }
}
