#include "cpu/exec/helper.h"

#include <setjmp.h>
extern jmp_buf jbuf;
static inline void push_r2stack(int val){
	reg_l(R_ESP) -= 4;
	swaddr_write(reg_l(R_ESP), 4, val, R_SS);
}
void raise_intr (uint8_t NO) {
    /* 触发一个编号是NO的异常 */
    // printf("%d %d\n",NO,cpu.IDTR.limit);
	Assert((NO << 3) <= cpu.IDTR.limit, "Wrong Int id!");
	Gate_Descriptor now_gate;
	idt_desc = &now_gate;

	lnaddr_t addr = cpu.IDTR.base + (NO << 3);
	idt_desc -> part1 = lnaddr_read(addr,4);
	idt_desc -> part2 = lnaddr_read(addr+4,4);
	
	push_r2stack(cpu.eflags);
	if (cpu.cr0.protect_enable == 0){
		cpu.IF = 0;
		cpu.TF = 0;
	}
	push_r2stack(cpu.CS.selector);
	push_r2stack(cpu.eip);

	cpu.CS.selector = idt_desc -> selector;

	sreg_set(R_CS);
	cpu.eip = cpu.CS.base + idt_desc -> offset1 + (idt_desc -> offset2 << 16);
    /* 回来 */
    longjmp(jbuf, 1);
}

/* for instruction encoding overloading */

make_helper(intr) {
    int NO = instr_fetch(eip + 1, 1);   // 得到NO
    cpu.eip += 2;                       // 提前处理PC
    print_asm("int %x", NO);
    raise_intr(NO);                     // 异常
    return 0;
}