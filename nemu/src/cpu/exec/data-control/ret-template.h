#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_n_, SUFFIX)){//没有相应的decode，所以自己写
    //ret
    cpu.eip = MEM_R(reg_l(R_SP), 2);                        // 2表示SS
    if(DATA_BYTE == 2) cpu.eip &= 0xffff;
    reg_l(R_SP) += DATA_BYTE;

    print_asm("ret");
    return 1;
}

make_helper(concat(ret_i_, SUFFIX)){//c2
    //ret
	cpu.eip = MEM_R (REG (R_ESP), 2);
	if (DATA_BYTE == 2) cpu.eip &= 0xffff;
	REG (R_ESP) += DATA_BYTE;
    //pop immm个字节
    int val = instr_fetch(eip + 1, 2);
	int i;
	for (i = 0; i < val; i += DATA_BYTE) MEM_W (REG (R_ESP) + i,0, 2);//初始化成0
	REG (R_ESP) += val;

	print_asm("ret $0x%x",val);
	return 1;   //尽管实际长度是3字节，但是有关eip的赋值，还是应该返回1
}

#include "cpu/exec/template-end.h"
