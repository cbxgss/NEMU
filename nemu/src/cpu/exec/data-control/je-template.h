#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
	DATA_TYPE_S imm = op_src->val;
    if(cpu.ZF == 1) cpu.eip += imm;
    print_asm("je %x",cpu.eip + DATA_BYTE+1);   //注意上面已经+imm了
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
