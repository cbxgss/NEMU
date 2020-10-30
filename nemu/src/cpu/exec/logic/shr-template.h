#include "cpu/exec/template-start.h"

#define instr shr

static void do_execute () {
	DATA_TYPE src = op_src->val;
	DATA_TYPE dest = op_dest->val;

	uint8_t count = src & 0x1f;
	dest >>= count;
	OPERAND_W(op_dest, dest);

	/* TODO: Update EFLAGS. */
	//检查无符号数溢出
    cpu.CF = 0;
    // = 结果的符号位
    cpu.SF = dest >> (DATA_BYTE*8 -1);
    //检查有符号数溢出
    cpu.OF = 0;
    // 结果==0 为1
    cpu.ZF = !dest;
    // 结果的最低有效字节包含偶数个1 为1
    dest ^= dest >>4;
	dest ^= dest >>2;
	dest ^= dest >>1;
	cpu.PF = !(dest & 1);
	// panic("please implement me");

	print_asm_template2();
}

make_instr_helper(rm_1)
make_instr_helper(rm_cl)
make_instr_helper(rm_imm)

#include "cpu/exec/template-end.h"
