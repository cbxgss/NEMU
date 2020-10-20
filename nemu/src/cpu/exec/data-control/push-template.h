#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {
        reg_l(R_SP) -= DATA_BYTE;
        swaddr_write(reg_l(R_SP), DATA_BYTE, (DATA_TYPE_S)op_src->val);
        print_asm_template1();
}

make_instr_helper(r)

#include "cpu/exec/template-end.h"
