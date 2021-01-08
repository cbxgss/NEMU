#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
    DATA_TYPE_S imm = op_src->val;
    if( op_src->type == OP_TYPE_IMM ){
        //是立即数的时候
        cpu.eip += imm;
        print_asm("jmp %x",cpu.eip + DATA_BYTE+1);
    }
    else {
        // printf("imm:%x\t", imm);
        // 通过上面的printf可以推断，似乎imm直接就是要Jum的地址，所以下面要减去该指令的长度
        cpu.eip = imm - (concat (decode_rm_,SUFFIX)(cpu.eip+1) + 1);
		print_asm_no_template1();
    }
}

make_instr_helper(i)
make_instr_helper(rm)

#if DATA_BYTE == 4
extern Sreg_info sreg_info;
Sreg_info tmp;
make_helper(ljmp){
    sreg_info = tmp;            // 初始化sreg_info，全0
    cpu.eip = instr_fetch(cpu.eip + 1, 4) - 7;                  // 后面eip会+7，所以先-7
    cpu.CS.selector = instr_fetch(cpu.eip + 1 + 4, 2);          // 设置CS寄存器
    // sreg_set(R_CS);             // 设置CS寄存器
    print_asm("ljmp 0x%x 0x%x",instr_fetch(cpu.eip + 1 + 4, 2), instr_fetch(cpu.eip+1, 4));
    return 7;
}
#endif

#include "cpu/exec/template-end.h"
