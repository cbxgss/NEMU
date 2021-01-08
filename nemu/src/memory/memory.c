#include "common.h"
#include <stdlib.h>
#include "burst.h"
#include "memory/cache.h"
#include "cpu/reg.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

/* 物理地址 */
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	// printf("addr(r) : 0x%x\teip : 0x%x\n", addr, cpu.eip);													
	/* 原来的代码 */
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	/* 加上 chahe 之后的代码 */
	int32_t set_l1 = (addr >> block_size_bit) & (l1_sets - 1);
	int32_t i = l1_read(addr);
	int32_t imm_l1 = (addr & (block_size - 1));
	int8_t tmp [block_size * 2];	// 把得到的len长度的内容存tmp里（长度为变量len不通过）
	if(imm_l1 + len > block_size) {													/* 跨了两个块 */
		// 第2个块的地址翻译
		// puts("two block");
		memcpy(tmp, l1_cache[set_l1][i].block + imm_l1, block_size - imm_l1);									// 复制第一个块的内容
		int32_t i_last = l1_read(addr + block_size - imm_l1);
		int32_t set_last = ((addr + block_size - imm_l1) >> block_size_bit) & (l1_sets - 1);
		memcpy(tmp + block_size - imm_l1, l1_cache[set_last][i_last].block, len - (block_size - imm_l1));		// 复制剩下的第2个块
	}
	else memcpy(tmp, l1_cache[set_l1][i].block + imm_l1, len);						/* 一个块 */
	// printf("from l1[%x][%x] :tmp: ", set_l1, i); int jkl;																
	// for(jkl = 0; jkl < len; jkl++) {
	// 	printf("%x ", tmp[jkl]);
	// }
	// puts("");
	int qwq = 0; uint32_t ret = unalign_rw(tmp + qwq, 4) & (~0u >> ((4 - len) << 3));					//	在nemu/include/macro.h
	// uint32_t ans = dram_read(addr, len) & (~0u >> ((4 - len) << 3));																	
	// printf("ret is 0x%x.\tAnd it is 0x%x in dram.\n", ret, ans);																				
	return ret;
}
void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	// printf("addr(w) :0x%x\n", addr);																	
	/* 原来的代码 */
	// dram_write(addr, len, data); return;
	/* 加入cache后的代码 */
	l1_write(addr, len, data);
}

/* 线性地址 */
uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}
void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

/* 虚拟地址 */
lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg){
	if (cpu.cr0.protect_enable == 0) return addr;
	return cpu.sreg[sreg].base + addr;
}
uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
	assert(len == 1 || len == 2 || len == 4);
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_read(lnaddr, len);
}
void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
	assert(len == 1 || len == 2 || len == 4);
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	lnaddr_write(lnaddr, len, data);
}