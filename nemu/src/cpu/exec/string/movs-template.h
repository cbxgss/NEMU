#include "cpu/exec/template-start.h"

#define instr movs

static void do_execute () {
	 // 描述:
    // 移动字符串数据，复制由ESI寄存器寻址的内存地址处的数据至EDI寻址的内存地址处   将1字节，Word或双字数据从存储器位置移动到另一个。 
    // 执行的操作：
    // 1) ((DI))←((SI))
    // 2) 字节操作：
    // (SI)←(SI)±1,(DI)←(DI)±1
    // 当方向标志DF=0时用+，当方向标志DF=1时用-
    // 3) 字操作：          //没用到
    // (SI)←(SI)±2,(DI)←(DI)±2
    // 当方向标志DF=0时用+，当方向标志DF=1时用-
    // 该指令不影响条件码。
	if ( ops_decoded.is_operand_size_16 ) {
		swaddr_write (reg_w(R_DI), 2,swaddr_read (reg_w(R_SI),4));
		if (cpu.DF == 0) { reg_w (R_DI) += DATA_BYTE; reg_w (R_SI) += DATA_BYTE; }  //DF表示方向
		else { reg_w (R_DI) -= DATA_BYTE; reg_w (R_SI) -= DATA_BYTE; }
	}
	else {
		swaddr_write (reg_l(R_EDI), DATA_BYTE, swaddr_read (reg_l(R_ESI),4));
		if (cpu.DF == 0) { reg_l (R_EDI) += DATA_BYTE; reg_l (R_ESI) += DATA_BYTE; }
		else { reg_l (R_EDI) -= DATA_BYTE; reg_l (R_ESI) -= DATA_BYTE; }
	}
	print_asm("movs");
}

make_instr_helper(n)

#include "cpu/exec/template-end.h"
