#include "common.h"
#include <stdlib.h>
#include "burst.h"
#include "memory/cache.h"
#include "cpu/reg.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

// 读从addr开始的len个字节
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	printf("addr(r) : 0x%x\teip : 0x%x\n", addr, cpu.eip);													
	/* 原来的代码 */
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	/* 加上 chahe 之后的代码 */
	int32_t set_l2 = (addr >> l2_sets_bit) & (l2_sets - 1);
	int32_t i = l2_read(addr);
	int32_t imm_l2 = (addr & (block_size - 1));
	int8_t tmp [block_size * 2];	// 把得到的len长度的内容存tmp里（长度为变量len不通过）
	if(imm_l2 + len > block_size) {													/* 跨了两个块 */
		// 第2个块的地址翻译
		memcpy(tmp, l2_cache[set_l2][i].block + imm_l2, block_size - imm_l2);									// 复制第一个块的内容
		int32_t i_last = l2_read(addr + len);
		int32_t set_last = ((addr + block_size - imm_l2) >> l2_sets_bit) & (l2_sets - 1);
		memcpy(tmp + block_size - imm_l2, l2_cache[set_last][i_last].block, len - (block_size - imm_l2));		// 复制剩下的第2个块
	}
	else memcpy(tmp, l2_cache[set_l2][i].block + imm_l2, len);						/* 一个块 */
	int jkl;																
	for(jkl = 0; jkl < len; jkl++) {
		printf("%x ", tmp[jkl]);
	}
	puts("");
	int qwq = 0; uint32_t ret = unalign_rw(tmp + qwq, 4) & (~0u >> ((4 - len) << 3));					//	在nemu/include/macro.h
	uint32_t ans = dram_read(addr, len) & (~0u >> ((4 - len) << 3));																	
	printf("ret is %u.\tAnd it should be %d\n", ret, ans);																				
	return ret;
}
void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	// printf("addr(w) :0x%x\n", addr);																	
	/* 原来的代码 */
	// dram_write(addr, len, data); return;
	/* 加入cache后的代码 */
	l2_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}
void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}
void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

// addr : 0x100000
// addr : 0x100001
// addr : 0x100000
// addr : 0x100001
// addr : 0x100002
// addr : 0x100003
// addr : 0x100004
//   100000:   bd 00 00 00 00                        movl $0x0,%ebp

// 0X100000        BD 00 00 00 00 BC 00 00 
// 0X100008        00 08 E9 91 07 00 00 90 