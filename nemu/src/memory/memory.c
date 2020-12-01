#include "common.h"

#define block_bytes 64		// 一个块的字节
#define Cache_ways 8			// 几路组相连
#define Cache_sets 1024/8	// Cache的组数

/* Cache的class */
typedef struct block {
	uint32_t tag;					// 标记位
	uint8_t block[block_bytes];		// 64个字节
	bool valid;					// valid bit
} block;
typedef struct set {
	block blocks[Cache_ways];
} set;
typedef struct Cache {
	/* 成员属性 */
	set sets[Cache_sets];			// 2**10个块
	/* 成员函数 */
	// void (* init)(struct Cache *this);
} Cache;
Cache cache;
/* 成员函数的实现 */
void init_cache() {
	int i, j;
	for(i = 0; i < Cache_sets; i++) {
		for(j = 0; j < Cache_ways; j++) {
			cache.sets[i].blocks[j].valid = false;
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

