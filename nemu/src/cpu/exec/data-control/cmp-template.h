#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute() {
	DATA_TYPE result = op_dest->val - op_src->val;    //前一个操作数 - 后一个（无符号数）
    //检查无符号数溢出
    if(op_dest->val < op_src->val) cpu.CF = 1;
    else cpu.CF = 0;
    // = 结果的符号位
    cpu.SF = result >> (DATA_BYTE*8 -1);
    //检查有符号数溢出
    int a = op_dest->val >> (DATA_BYTE*8 - 1);
    int b = op_src->val >> (DATA_BYTE*8 - 1);
    // a - b  = result
    // +   -        -
    // -   +        +
    //因此，当a和b符号不同，结果和b的符号相同时，有符号数溢出
    cpu.OF = ((a != b) && (b == cpu.SF));
    // 结果==0 为1
    cpu.ZF = !result;
    // 结果的最低有效字节包含偶数个1 为1
    result ^= result >>4;
	result ^= result >>2;
	result ^= result >>1;
	cpu.PF = !(result & 1);
    //D3到D4是否进位
    if(( (op_dest->val & 0xf) - (op_src->val & 0xf) ) >> 4) cpu.AF = 1;
    else cpu.AF = 0;
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
