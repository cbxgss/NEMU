#include "common.h"
#include <stdlib.h>
#include "burst.h"
#include "memory/cache.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

// 读从addr开始的len个字节
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	/* 原来的代码 */
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	/* 加上 chahe 之后的代码 */
	int32_t set_l1 = (addr >> l1_sets_bit) & (l1_sets - 1);
	int32_t i = l1_read(addr);
	int32_t imm_l1 = (addr & (block_size - 1));
	int8_t tmp[block_size] = {};			// 把得到的len长度的内容存tmp里（长度为变量len不通过）
	if(imm_l1 + len > block_size) {													/* 跨了两个块 */
		// 第2个块的地址翻译
		memcpy(tmp, l1_cache[set_l1][i].block + imm_l1, block_size - imm_l1);									// 复制第一个块的内容
		int32_t i_last = l1_read(addr + len);
		int32_t set_last = ((addr + len) >> l1_sets_bit) & (l1_sets - 1);
		memcpy(tmp + block_size - imm_l1, l1_cache[set_last][i_last].block, len - (block_size - imm_l1));		// 复制剩下的第2个块
	}
	else memcpy(tmp, l1_cache[set_l1][i].block + imm_l1, len);						/* 一个块 */
	int qwq = 0; return unalign_rw(tmp + qwq, 4) & (~0u >> ((4 - len) << 3));					//	在nemu/include/macro.h
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	/* 原来的代码 */
	// dram_write(addr, len, data);
	/* 加入cache后的代码 */
	l1_write(addr, len, data);
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

