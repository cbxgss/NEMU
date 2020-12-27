#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {      // SS 2
        if (DATA_BYTE == 1) op_src->val = (int8_t)op_src->val;
        reg_l(R_SP) -= 4;                       //为了pop方便，统一-4
        swaddr_write(reg_l(R_SP), 4, (DATA_TYPE_S)op_src->val, 2);
        print_asm_template1();
}

make_instr_helper(i)
make_instr_helper(r)
make_instr_helper(rm)

#include "cpu/exec/template-end.h"
