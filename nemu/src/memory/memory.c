#include "common.h"
#include <stdlib.h>
#include "burst.h"

#define block_bytes 64		// 一个块的字节
#define Cache_ways 8		// 几路组相连
#define Cache_sets 1024/8	// Cache的组数

void ddr3_read (hwaddr_t, void *);
void ddr3_write (hwaddr_t, void *, uint8_t *);

/* Cache的class */
typedef struct block {
	uint32_t tag;					// 标记位(19位)
	uint8_t block[block_bytes];		// 64个字节
	bool valid;					// valid bit
} block;
typedef struct set {
	block blocks[Cache_ways];
} set;
typedef struct Cache {
	/* 成员属性 */
	set sets[Cache_sets];			// 2**10个块
	uint64_t t_sum;					// 计时变量
	/* 成员函数 */
	// void (* init)(struct Cache *this);
} Cache;
Cache cache;		// 实例化
/* 成员函数的实现 */
void init_cache() {
	cache.t_sum = 0;
	int i, j;
	for(i = 0; i < Cache_sets; i++) {
		for(j = 0; j < Cache_ways; j++) {
			cache.sets[i].blocks[j].valid = false;
			cache.sets[i].blocks[j].tag = 0;
		}
	}
}

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

// 返回是set_index的哪个block，如果miss，先处理，再返回
uint32_t cache_read(hwaddr_t addr) {
	// printf("(0x%x", addr);
	// 地址32位 = 19位tags + 7位sets + 6位块内偏移
	uint32_t tag_now = (addr >> 13) & 0x7ffff;
	uint32_t set_now = (addr >> 6) & 0x7f;
	bool hit = false;
	int i;
	for (i = 0; i < Cache_ways; i++) {	//在set中每个block检查
		if ( !cache.sets[set_now].blocks[i].valid ) continue;
		if ( cache.sets[set_now].blocks[i].tag == tag_now ) {
			hit = true; break;
		}
	}
	if(!hit) {	// miss
		// 找到位置
		for (i = 0; i < Cache_ways; i++) {
			if ( !cache.sets[set_now].blocks[i].valid ) break;
		}
		if(i == Cache_ways) {									// 替换算法
			srand(0);
			i = rand() % Cache_ways;
			// 写回，被替换的时候写回ddr3
			uint8_t mask[BURST_LEN * 2] = {};
			memset(mask, 1, BURST_LEN * 2);
			int j = 0;
			for(j = 0; j < block_bytes / BURST_LEN; j++) {
				ddr3_write(((addr >> 6) << 6) + j * BURST_LEN, cache.sets[set_now].blocks[i].block + j*BURST_LEN, mask);
			}
		}
		// 复制到这个块
		cache.sets[set_now].blocks[i].valid = true;
		cache.sets[set_now].blocks[i].tag = tag_now;
		int j;
		for ( j = 0; j < BURST_LEN; j++ )
			ddr3_read(((addr>>6)<<6) + j * BURST_LEN, cache.sets[set_now].blocks[i].block + j*BURST_LEN);
	}
	// printf(",qwq)\t");
	return i;
}

// 读从addr开始的len个字节
uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	// printf("(0x%x) ", addr);
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
	// int i = 0;
	// for(i = 0; i < (int)len; i++) printf("%x ", (int)tmp[i]); puts("");
	return unalign_rw(tmp + qwq, 4) & (~0u >> ((4 - len) << 3));						//	在nemu/include/macro.h
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	/* 原来的代码 */
	// dram_write(addr, len, data);
	/* 加入cache后的代码 */
	uint32_t tag_now = (addr >> 13) & 0x7ffff;
	uint32_t set_now = (addr >> 6) & 0x7f;
	uint32_t imm_now = addr & 0x3f;
	bool hit = false;
	int i;
	for (i = 0; i < Cache_ways; i++) {	//在set中每个block检查
		if ( !cache.sets[set_now].blocks[i].valid ) continue;
		if ( cache.sets[set_now].blocks[i].tag == tag_now ) {
			hit = true; break;
		}
	}
	if(hit == false) dram_write(addr, len, data);
	else {
		memcpy(cache.sets[set_now].blocks[i].block + imm_now, &data, len);
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

