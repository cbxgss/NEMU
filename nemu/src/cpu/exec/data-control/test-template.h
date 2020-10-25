#include "cpu/exec/template-start.h"

#define instr test

static void do_execute() {
    DATA_TYPE result = op_src->val & op_dest->val;      //两个操作数的&
    cpu.CF = 0;                                         //检查无符号数溢出
    cpu.OF = 0;                                         //检查有符号数溢出
    cpu.SF = result >> (DATA_BYTE*8 -1);                // = 结果的符号位
    cpu.ZF = !result;                                   // 结果==0 为1
    int qwq = 0;
    int i;
    for(i = 0; i < 8; i++) qwq += (result >> i) & 1;// 结果的最低有效字节包含偶数个1 为1
    if(qwq % 2) cpu.PF = 0;
    else cpu.PF = 1;

    print_asm_template2();
}

// make_instr_helper(i2rm)
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"
