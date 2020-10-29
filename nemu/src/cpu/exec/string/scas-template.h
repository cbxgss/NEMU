#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_n_, SUFFIX)){//ae ; 和lods类似
	// 将寄存器（AL，AX或EAX）的内容与内存中的项目的内容进行比较
	swaddr_write (reg_b(R_AL), 1, swaddr_read(reg_b(R_AL), 1) - swaddr_read (reg_w(R_SI),1));
	if (cpu.DF == 0) { reg_b (R_AL) += DATA_BYTE; reg_w (R_SI) += DATA_BYTE; }
	else { reg_b (R_AL) -= DATA_BYTE; reg_w (R_SI) -= DATA_BYTE; }
	print_asm("scas");
    return 1;
}

#include "cpu/exec/template-end.h"
