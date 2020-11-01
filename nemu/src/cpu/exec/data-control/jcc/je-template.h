#include "cpu/exec/template-start.h"

#define instr je

static void do_execute() {
	DATA_TYPE_S imm = op_src->val;
    // printf("eip:%x\tDATA_BYTE:%x\timm:%x\t", cpu.eip, DATA_BYTE, imm);
    // 因为上面修改了exec.c文件里的0f开头的函数，应该先cpu.eip++，不然这里je输出的时候总是少1
    print_asm("je %x",cpu.eip + DATA_BYTE+1 + imm);
    if(cpu.ZF == 1) cpu.eip += imm;
    // print_asm("je %x",cpu.eip + DATA_BYTE+1);   //注意上面已经+imm了     这是原来的写法，实际上不对，因为不管怎样，输出指令的时候都应该加上imm
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
