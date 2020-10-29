#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_n_, SUFFIX)){//没有相应的decode，所以自己写
    cpu.eip = MEM_R(reg_l(R_SP));
    if(DATA_BYTE == 2) cpu.eip &= 0xffff;
    reg_l(R_SP) += DATA_BYTE;
    print_asm("ret");
    return 1;
}

#include "cpu/exec/template-end.h"
