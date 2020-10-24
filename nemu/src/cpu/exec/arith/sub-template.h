#include "cpu/exec/template-start.h"

#define instr sub

static void do_execute() {
	print_asm_template2();
}

make_instr_helper(i2rm)

#include "cpu/exec/template-end.h"
