#ifndef __OPERAND_H__
#define __OPERAND_H__

enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM, OP_TYPE_NO };			//加了一个没有参数OP_TYPE_NO

#define OP_STR_SIZE 40

typedef struct {
	uint32_t type;
	size_t size;
	union {
		uint32_t reg;		//寄存器
		swaddr_t addr;		//地址
		uint32_t imm;		//u偏移量
		int32_t simm;		//偏移量
	};
	uint32_t val;			//值
	char str[OP_STR_SIZE];
} Operand;

typedef struct {
	uint32_t opcode;
	bool is_operand_size_16;
	Operand src, dest, src2;
} Operands;

#endif
