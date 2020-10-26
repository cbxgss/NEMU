#include "cpu/exec/template-start.h"

#define instr or

static void do_execute () {
	DATA_TYPE result = op_dest->val | op_src->val;
	OPERAND_W(op_dest, result);

	/* TODO: Update EFLAGS. */
	//检查无符号数溢出
    cpu.CF = 0;
    // = 结果的符号位
    cpu.SF = result >> (DATA_BYTE*8 -1);
    //检查有符号数溢出
    cpu.OF = 0;
    // 结果==0 为1
    cpu.ZF = !result;
    // 结果的最低有效字节包含偶数个1 为1
    result ^= result >>4;
	result ^= result >>2;
	result ^= result >>1;
	cpu.PF = !(result & 1);
	// panic("please implement me");

	print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"
