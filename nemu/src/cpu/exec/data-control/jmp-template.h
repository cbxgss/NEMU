#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
    DATA_TYPE_S imm = op_src->val;
    if( op_src->type == OP_TYPE_IMM ){
        cpu.eip += imm;
        print_asm("jmp %x",cpu.eip + DATA_BYTE+1);
    }
    else {
        printf("imm:%x\t", imm);
        cpu.eip = imm - concat (decode_rm_,SUFFIX)(cpu.eip+1) - 1;
		print_asm_no_template1();
    }
}

make_instr_helper(i)
make_instr_helper(rm)

#include "cpu/exec/template-end.h"
