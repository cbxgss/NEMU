#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
	DATA_TYPE_S imm = op_src->val;
    printf("eip:%d\tDATA_BYTE:%d\timm:%d\t", cpu.eip, DATA_BYTE, imm);
    print_asm("je %x",cpu.eip + DATA_BYTE+1 + imm);
    if(cpu.ZF == 1) cpu.eip += imm;
    // print_asm("je %x",cpu.eip + DATA_BYTE+1);   //注意上面已经+imm了     这是原来的写法，实际上不对，因为不管怎样，输出指令的时候都应该加上imm
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
