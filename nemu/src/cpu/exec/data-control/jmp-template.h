#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
    DATA_TYPE_S imm = op_src->val;
    cpu.eip += imm;
    print_asm("jmp %x",cpu.eip + DATA_BYTE+1);
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
