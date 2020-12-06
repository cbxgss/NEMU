#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>

void ddr3_read (hwaddr_t, void *);
void ddr3_write (hwaddr_t, void *, uint8_t *);

void init_cache() {
	cache.t_sum = 0;
	int i, j;
	for(i = 0; i < Cache_sets; i++) {
		for(j = 0; j < Cache_ways; j++) {
			cache.sets[i].blocks[j].valid = false;
			cache.sets[i].blocks[j].tag = 0;
			memset(cache.sets[i].blocks[j].block, 0, block_bytes);
		}
	}
}
void p_cache_t() {
	printf("%lu\n", cache.t_sum);
}

// 地址32位 = 19位tags + 7位sets + 6位块内偏移
uint32_t cache_read(hwaddr_t addr) { // 返回是set_index的哪个block，如果miss，先处理，再返回
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
		// if(((addr >> 6) << 6) == 0x801000) puts("5555");
		// 找到位置
		for (i = 0; i < Cache_ways; i++) {
			if ( !cache.sets[set_now].blocks[i].valid ) break;
		}
		if(i == Cache_ways) {									// 替换算法
			srand(0);
			i = rand() % Cache_ways;
			// 写回，被替换的时候写回ddr3
			// uint8_t mask[BURST_LEN * 2] = {};
			// memset(mask, 1, BURST_LEN * 2);
			// int j = 0;
			// for(j = 0; j < block_bytes / BURST_LEN; j++) {
			// 	ddr3_write(((addr >> 6) << 6) + j * BURST_LEN, cache.sets[set_now].blocks[i].block + j * BURST_LEN, mask);
			// }
		}
		// 复制到这个块
		cache.sets[set_now].blocks[i].valid = true;
		cache.sets[set_now].blocks[i].tag = tag_now;
		int j;
		for ( j = 0; j < block_bytes / BURST_LEN; j++ )
			ddr3_read(((addr >> 6) << 6) + j * BURST_LEN, cache.sets[set_now].blocks[i].block + j * BURST_LEN);
		cache.t_sum += 200;
		// 打印
		// printf("(0x%x)", ((addr >> 6) << 6)); int k;																							
		// for(k = 0; k < block_bytes; k++) printf("%x ", (int)cache.sets[set_now].blocks[i].block[k]); puts("");									
	}
	else cache.t_sum += 2;
	// printf(",qwq)\t");
	return i;
}
