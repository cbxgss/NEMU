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
	// 地址处理
	uint32_t set_now = (addr >> 6) & 0x7f;
	uint32_t block_now = cache_read(addr);
	uint32_t imm_now = addr & 0x3f;
	// 读
	uint8_t tmp[block_bytes] = {};														// 把得到的len长度的内容存tmp里（长度为变量len不通过）
	if(imm_now + len >= block_bytes) {													// 跨了两个块
		// 第2个块的地址翻译
		uint32_t set_last = ((addr + len) >> 6) & 0x7f;
		uint32_t block_last = cache_read(addr + len);
		memcpy(tmp, cache.sets[set_now].blocks[block_now].block + imm_now, block_bytes - imm_now);							// 复制第一个块的内容
		memcpy(tmp + block_bytes - imm_now, cache.sets[set_last].blocks[block_last].block, len - (block_bytes - imm_now));	// 复制剩下的第2个块
	}
	else memcpy(tmp, cache.sets[set_now].blocks[block_now].block + imm_now, len);		//一个块
	int qwq = 0;
	// p_cache_t();
	return unalign_rw(tmp + qwq, 4) & (~0u >> ((4 - len) << 3));						//	在nemu/include/macro.h
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	/* 原来的代码 */
	// dram_write(addr, len, data);
	/* 加入cache后的代码 */
	// if(((addr >> 6) << 6) == 0x801000) puts("5555");							
	// printf("%u\t%u\n", cache_read(addr), cache_read(addr));
	uint32_t tag_now = (addr >> 13) & 0x7ffff;	//19
	uint32_t set_now = (addr >> 6) & 0x7f;		//7
	uint32_t imm_now = addr & 0x3f;				//6
	bool hit = false;
	int i;
	for (i = 0; i < Cache_ways; i++) {	//在set中每个block检查
		if ( !cache.sets[set_now].blocks[i].valid ) continue;
		if ( cache.sets[set_now].blocks[i].tag == tag_now ) {
			hit = true; break;
		}
	}
	if(hit == false) {
		// if(((addr >> 6) << 6) == 0x801000) { printf("%x\t%x \t\t", addr, data); puts("55555555555"); }					
		if(imm_now + len <= block_bytes) dram_write(addr, len, data);
		else dram_write(addr, block_bytes - imm_now, data);
		cache.t_sum += 200;
	}
	else {
		memcpy(cache.sets[set_now].blocks[i].block + imm_now, &data, len);	cache.t_sum += 2;
	}
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

