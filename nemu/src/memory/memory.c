#include "common.h"

#define block_bytes 64		// 一个块的字节
#define cache_ways 8			// 几路组相连
#define cache_sets 1024/8	// cache的组数

/* cache的class */
typedef struct block {
	uint32_t tag;					// 标记位
	uint8_t block[block_bytes];		// 64个字节
	uint8_t valid;					// valid bit
} block;
typedef struct set {
	block blocks[cache_ways];
} set;
typedef struct cache {
	/* 成员属性 */
	set sets[cache_sets];			// 2**10个块
	/* 成员函数 */
	void (* init) (cache *this);
} cache;
void init_(cache x) {
	int i, j;
	for(i = 0; i < cache_sets; i++) {
		for(j = 0; j < cache_ways; j++) {
			x.sets[i].blocks[j].valid = 0;
		}
	}
}
uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	dram_write(addr, len, data);
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

