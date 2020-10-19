#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)) {
    //decode_i_v 译码, len = 操作数的长度 = 本指令长度 - 1
    int len = concat(decode_i_, SUFFIX) (eip + 1);
    //开栈
    reg_l (R_ESP) -= DATA_BYTE;
    //*rsp = eip + len
    swaddr_write (reg_l (R_ESP) , 4 , cpu.eip + len);
    //偏移量
    DATA_TYPE_S imm = op_src->val;
    //打印汇编
    print_asm("call %x",cpu.eip + 1 + len + imm);
    //修改eip
    cpu.eip += imm;
    return len + 1;
    //函数返回该指令长度之后，eip再加上该指令的长度
}

#include "cpu/exec/template-end.h"
