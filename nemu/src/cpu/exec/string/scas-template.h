#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_n_, SUFFIX)){//ae ; 和lods类似, 改几个字母就ok
	// 将寄存器（AL，AX或EAX）的内容与内存中的项目的内容进行比较
    if ( ops_decoded.is_operand_size_16 ) {
		swaddr_write (reg_w(R_AL), 2, swaddr_read (reg_w(R_AL),4) - swaddr_read (reg_w(R_DI),4));
		if (cpu.DF == 0) { reg_w (R_AL) += DATA_BYTE; reg_w (R_DI) += DATA_BYTE; }
		else { reg_w (R_AL) -= DATA_BYTE; reg_w (R_DI) -= DATA_BYTE; }
	}
	else {
		swaddr_write (reg_l(R_EAX), 4, swaddr_read (reg_w(R_EAX),4) - swaddr_read (reg_l(R_EDI),4));
		if (cpu.DF == 0) { reg_l (R_EAX) += DATA_BYTE; reg_l (R_EDI) += DATA_BYTE; }
		else { reg_l (R_EAX) -= DATA_BYTE; reg_l (R_EDI) -= DATA_BYTE; }
	}
	print_asm("scas");
    return 1;
}

#include "cpu/exec/template-end.h"
