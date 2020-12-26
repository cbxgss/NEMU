#ifndef __REG_H__
#define __REG_H__

#include "common.h"
#include "../../lib-common/x86-inc/cpu.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };
enum { R_CS, R_DS, R_SS, R_ES};

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct{
	uint16_t selector;				// 段描述符
	uint32_t base, limit, type;		// 隐藏信息
} S_reg;

typedef struct {
	// reg
	union {
		union {//小端序
			uint32_t _32;//32位==4bytes
			struct {
				union {//后16位
					uint16_t _16;
					uint8_t _8[2];
				};
				uint16_t _16_0;//前16位
			};
		} gpr[8];

		/* Do NOT change the order of the GPRs' definitions. */
		struct {
			uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
		};
	};
	swaddr_t eip;
	//EFLAGS寄存器
	union {
		struct {
			uint32_t CF:	1;
			uint32_t :		1;
			uint32_t PF:	1;
			uint32_t :		1;
			uint32_t AF:	1;
			uint32_t :		1;
			uint32_t ZF:	1;
			uint32_t SF:	1;
			uint32_t TF:	1;
			uint32_t IF:	1;
			uint32_t DF:	1;
			uint32_t OF:	1;
			uint32_t IOPL:	1;
			uint32_t NT:	1;
			uint32_t :		1;
			uint32_t RF:	1;
			uint32_t VM:	1;
			uint32_t :		14;
		};
		uint32_t eflags;
	};
	// 段
	struct{
		uint32_t base, limit;	// GDT的 首地址 和 长度
	} GDTR;
	CR0 cr0;
	union{
		struct{
			S_reg sreg[4];	// 为了方便swaddr_read和seg_translate
		};
		struct{
			S_reg CS, DS, SS, ES;
			// CS是代码段寄存器
			// DS是数据段寄存器
			// SS是堆栈段寄存器
			// ES是扩展段寄存器
		};
	};
} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
	assert(index >= 0 && index < 8);
	return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])

extern const char* regsl[];
extern const char* regsw[];
extern const char* regsb[];

#endif
