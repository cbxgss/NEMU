#include "cpu/exec/template-start.h"

#define instr pop

// static void do_execute() {
//     swaddr_write(op_src->addr, 4, MEM_R(reg_l(R_SP)));
//     reg_l(R_SP) += 4;
//     print_asm_no_template1();
// }

static void do_execute () {			// SS 2
	OPERAND_W (op_src, MEM_R(REG (R_ESP), 2));
	MEM_W(REG (R_ESP) , 0, 2);
	REG (R_ESP) += 4;
	print_asm_no_template1();
}

make_instr_helper(r)

#include "cpu/exec/template-end.h"
