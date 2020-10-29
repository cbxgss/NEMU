#include "cpu/exec/template-start.h"

#define instr sbb

static void do_execute() {
	DATA_TYPE qwq;
	if (op_src->size == 1 && op_dest->size != 1) {
        qwq = (int8_t)op_src->val + cpu.CF;
    }
    else {
        qwq = op_src->val + cpu.CF;
    }
	DATA_TYPE result = op_dest->val - qwq;
    OPERAND_W(op_dest, result);
    //flags
	cpu.CF = op_dest->val < qwq;
	cpu.SF=result >> (DATA_BYTE*8 - 1);
    int a, b;
    //a - b = result
    //-   +     +
    //+   -     -
    //ab不同号，b和result相同
	a = op_dest->val >> (DATA_BYTE*8 - 1);
	b = qwq >> (DATA_BYTE*8 - 1);
    cpu.OF = (a != b && b == cpu.SF) ;
    cpu.ZF = !result;
	result ^= result >>4;
	result ^= result >>2;
	result ^= result >>1;
	cpu.PF=!(result & 1);
	print_asm_no_template2();
}
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)


#include "cpu/exec/template-end.h"