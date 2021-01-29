#include "cpu/exec/template-start.h"
#include <nemu.h>

#define instr iret
#if DATA_BYTE == 2
int popfromstack_w(){
    DATA_TYPE ret = swaddr_read(reg_l(R_ESP), DATA_BYTE, R_SS);
    swaddr_write(reg_l(R_ESP), DATA_BYTE, 0, R_SS);
    reg_l(R_ESP) += DATA_BYTE;
    return ret;
}
#endif

#if DATA_BYTE == 4
int popfromstack_l(){
    DATA_TYPE ret = swaddr_read(reg_l(R_ESP), DATA_BYTE, R_SS);
    swaddr_write(reg_l(R_ESP), DATA_BYTE, 0, R_SS);
    reg_l(R_ESP) += DATA_BYTE;
    return ret;
}
#endif

make_helper(concat(iret_,SUFFIX)) {
	if (cpu.cr0.protect_enable == 0){
		cpu.eip = concat(popfromstack_,SUFFIX)();
		cpu.CS.selector = concat(popfromstack_,SUFFIX)();
		cpu.eflags = concat(popfromstack_,SUFFIX)();
	}else{
		cpu.eip = concat(popfromstack_,SUFFIX)();
		cpu.CS.selector = concat(popfromstack_,SUFFIX)();
		cpu.eflags = concat(popfromstack_,SUFFIX)();
		sreg_set(R_CS);
	}
	print_asm("iret");
	return 0;
}

#include "cpu/exec/template-end.h"