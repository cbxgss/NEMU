#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_n_, SUFFIX)){//ae ; 和lods类似
	// 将寄存器（AL，AX或EAX）的内容与内存中的项目的内容进行比较
	DATA_TYPE dest = reg_b(R_AL); DATA_TYPE src = swaddr_read (reg_l(R_EDI),1);
	DATA_TYPE result = dest - src;
	reg_b(R_AL) = result;
	if (cpu.DF == 0) reg_l (R_EDI) += DATA_BYTE; 
	else reg_l (R_EDI) -= DATA_BYTE;
	print_asm("scas");

	//检查无符号数溢出
	if(dest < src) cpu.CF = 1;
    else cpu.CF = 0;
	// = 结果的符号位
    cpu.SF = result >> (DATA_BYTE*8 - 1);
    //检查有符号数溢出
    int a = op_dest->val >> (DATA_BYTE*8 - 1);
    int b = op_src->val >> (DATA_BYTE*8 - 1);
		// a - b  = result
		// +   -        -
		// -   +        +
		//因此，当a和b符号不同，结果和b的符号相同时，有符号数溢出
    cpu.OF = (a != b) && (b == cpu.SF);
    // 结果==0 为1
    cpu.ZF = !result;
	// 结果的最低有效字节包含偶数个1 为1
    int qwq = 0;
    int i;
    for(i = 0; i < 8; i++) qwq += (result >> i) & 1;
    if(qwq % 2) cpu.PF = 0;
    else cpu.PF = 1;
    //D3到D4是否进位
    if(( (op_dest->val & 0xf) - (op_src->val & 0xf) ) >> 4) cpu.AF = 1;
    else cpu.AF = 0;

    return 1;
}

#include "cpu/exec/template-end.h"
