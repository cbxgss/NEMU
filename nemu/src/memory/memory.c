#include "common.h"
#include <stdlib.h>
#include "burst.h"
#include "memory/cache.h"
#include "cpu/reg.h"
#include "memory/tlb.h"

// #define DEBUG_page

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
int read_tlb(lnaddr_t addr);
void write_tlb(lnaddr_t addr, hwaddr_t haaddr);
hwaddr_t cmd_page_translate(lnaddr_t addr) {	// 简易调试器
	if(!cpu.cr0.protect_enable || !cpu.cr0.paging) return addr;
	/* addr = 10 dictionary + 10 page + 12 offset */
	uint32_t dictionary = addr >> 22, page = (addr >> 12) & 0x3ff, offset = addr & 0xfff;
	/* 读取页表信息 */
	uint32_t tmp = (cpu.cr3.page_directory_base << 12) + dictionary * 4;		// 页目录基地址 + 页目录号 * 页表项大小
	Page_info dictionary_, page_;
	dictionary_.val = hwaddr_read(tmp, 4);
	tmp = (dictionary_.addr << 12) + page * 4;									// 二级页表基地址 + 页号 + 页表项大小
	page_.val = hwaddr_read(tmp, 4);
	if(dictionary_.p != 1) {
		printf("dirctionary present != 1\n");
		return 0;
	}
	if(page_.p != 1) {
		printf("second page table present != 1\n");
		return 0;
	}
	return (page_.addr << 12) + offset;
}
hwaddr_t page_translate(lnaddr_t addr) {	// 线性地址 -> 物理地址
	if(!cpu.cr0.protect_enable || !cpu.cr0.paging) return addr;
	/* addr = 10 dictionary + 10 page + 12 offset */
	uint32_t dictionary = addr >> 22, page = (addr >> 12) & 0x3ff, offset = addr & 0xfff;
	/* 先看TLB */
	int index = read_tlb(addr);
	if (index != -1) return (tlb[index].data << 12) + offset;
	/* 读取页表信息 */
	uint32_t tmp = (cpu.cr3.page_directory_base << 12) + dictionary * 4;		// 页目录基地址 + 页目录号 * 页表项大小
	Page_info dictionary_, page_;
	dictionary_.val = hwaddr_read(tmp, 4);
	tmp = (dictionary_.addr << 12) + page * 4;									// 二级页表基地址 + 页号 + 页表项大小
	page_.val = hwaddr_read(tmp, 4);
#ifdef DEBUG_page
	printf("eip:0x%x\taddr 0x%x\n", cpu.eip, addr);
#endif
	Assert(dictionary_.p == 1, "dirctionary present != 1");
	Assert(page_.p == 1, "second page table present != 1");
#ifdef DEBUG_page
	printf("0x%x\n", (page_.addr << 12) + offset);
#endif
	hwaddr_t addr_ = (page_.addr << 12) + offset;
	write_tlb(addr, addr_);
	return addr_;
}
uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	assert(len == 1 || len == 2 || len ==4);
	uint32_t offset = addr & 0xfff;
	if((int64_t)(offset + len) > 0x1000) {	// 跨页
		// assert(0);
		size_t l = 0xfff - offset + 1;		// 低位最多几个字节同页
		uint32_t down_val = lnaddr_read(addr, l);			// 低位
		uint32_t up_val = lnaddr_read(addr + l, len - l);	//高位
		return (up_val << (l * 8)) | down_val;
	}
	else {
		hwaddr_t hwaddr = page_translate(addr);
		return hwaddr_read(hwaddr, len);
	}
}
void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	assert(len == 1 || len == 2 || len ==4);
	uint32_t offset = addr & 0xfff;
	if((int64_t)(offset + len) > 0x1000) {	// 跨页
		// assert(0);
		size_t l = 0xfff - offset + 1;		// 低位最多几个字节同页
		lnaddr_write(addr, l, data & ((1 << (l * 8)) - 1) );			// 低位
		lnaddr_write(addr + l, len - l, data >> (l * 8));				//高位
	}
	else {
		hwaddr_t hwaddr = page_translate(addr);
		hwaddr_write(hwaddr, len, data);
	}
}

/* 虚拟地址 */
lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) { // 虚拟地址 -> 线性地址
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
