#include "cpu/exec/template-start.h"

#define instr lods

make_helper(concat(lods_n_, SUFFIX)){//和movs类似
    if ( ops_decoded.is_operand_size_16 ) {
		swaddr_write (reg_w(R_AL),2,swaddr_read (reg_w(R_SI),4));
		if (cpu.DF == 0) { reg_w (R_AL) += DATA_BYTE; reg_w (R_SI) += DATA_BYTE; }
		else { reg_w (R_AL) -= DATA_BYTE; reg_w (R_SI) -= DATA_BYTE; }
	}
	else {
		swaddr_write (reg_l(R_EAX),4,swaddr_read (reg_l(R_ESI),4));
		if (cpu.DF == 0) { reg_l (R_EAX) += DATA_BYTE; reg_l (R_ESI) += DATA_BYTE; }
		else { reg_l (R_EAX) -= DATA_BYTE; reg_l (R_ESI) -= DATA_BYTE; }
	}
	print_asm("lods");
    return 1;
}

#include "cpu/exec/template-end.h"
