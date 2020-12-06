#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

#define block_bytes 64		// 一个块的字节
#define Cache_ways 8		// 几路组相连
#define Cache_sets 1024/8	// Cache的组数

/* Cache的class */
typedef struct block {
	uint32_t tag;					// 标记位(19位)
	uint8_t block[block_bytes];		// 64个字节
	bool valid;					    // valid bit
} block;
typedef struct set {
	block blocks[Cache_ways];
} set;
typedef struct Cache {
	/* 成员属性 */
	set sets[Cache_sets];			// 2**10个块
	uint64_t t_sum;					// 计时变量
} Cache;
Cache cache;		        // 实例化

void init_cache();
void p_cache_t();

// 地址32位 = 19位tags + 7位sets + 6位块内偏移
uint32_t cache_read(hwaddr_t addr);

#endif