#include "cpu/exec/template-start.h"

#define instr lods

make_helper(concat(lods_n_, SUFFIX)){//ac; 和movs类似
	// 内存加载。如果操作数为1字节，则将其装载到AL寄存器中，如果操作数为一个字，则它将被加载到AX寄存器中，并将一个双字加载到EAX寄存器中。 
	// printf("%c", swaddr_read (reg_l(R_ESI), 1));
	REG(R_EAX) = swaddr_read (reg_l(R_ESI), DATA_BYTE);
	if (cpu.DF == 0) {
		// reg_b (R_AL) += DATA_BYTE;		//gdb出来的答案，这里不能直接抄movs的，从汇编上可以看出来应该是$al直接存值，而不是val
		reg_l (R_ESI) += DATA_BYTE;
	}
	else {
		// reg_b (R_AL) -= DATA_BYTE;
		reg_l (R_ESI) -= DATA_BYTE;
	}
	print_asm("lods");
    return 1;
}

#include "cpu/exec/template-end.h"
