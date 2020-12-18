#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>
#include <time.h>

void dram_write(hwaddr_t addr, size_t len, uint32_t data);
void ddr3_read (hwaddr_t, void *);
void ddr3_write (hwaddr_t, void *, uint8_t *);

void init_cache() {
	l1_t = l2_t = 0;
	int i, j;
	for(i = 0; i < l1_sets; i++) {
		for(j = 0; j < l1_ways; j++) {
			l1_cache[i][j].valid = false;
			l1_cache[i][j].tag = 0;
			memset(l1_cache[i][j].block, 0, block_size);
		}
	}
	for(i = 0; i < l2_sets; i++) {
		for(j = 0; j < l2_ways; j++) {
			l2_cache[i][j].valid = l2_cache[i][j].dirty = false;
			l2_cache[i][j].tag = 0;
			memset(l2_cache[i][j].block, 0, block_size);
		}
	}
	srand(clock());
}
void p_cache_t() {
	printf("(l1: %lu)\t(l2: %lu)\n", l1_t, l2_t);
}

int32_t l1_read(hwaddr_t addr) { // 返回是set_index的哪个block，如果miss，先处理，再返回
	int32_t tag_l1 = (addr >> (l1_sets_bit + block_size_bit)) & ((1 << l1_tag_bit) - 1);
	int32_t set_l1 = (addr >> l1_sets_bit) & (l1_sets - 1);
	int i;
	for (i = 0; i < l1_ways; i++) {	//在set中每个block检查
		if ( !l1_cache[set_l1][i].valid ) continue;
		if ( l1_cache[set_l1][i].tag == tag_l1 ) {
			l1_t += 2;
			return i;
		}
	}
	/* miss */
	// 找到位置
	for (i = 0; i < l1_ways; i++) {			// 有空位
		if ( !l1_cache[set_l1][i].valid ) break;
	}
	if(i == l1_ways) i = rand() % l1_ways;	// 替换算法 write through 不用写回
	// 复制到这个块
	l1_cache[set_l1][i].valid = true; l1_cache[set_l1][i].tag = tag_l1;
	int ii = l2_read(addr);
	int set_l2 = (addr >> l2_sets_bit) & (l2_sets - 1);
	memcpy(l1_cache[set_l1][i].block, l2_cache[set_l2][ii].block, block_size);
	l1_t += 200;
	return i;
}
int32_t l2_read(hwaddr_t addr) {
	int32_t tag_l2 = (addr >> (l2_sets_bit + block_size_bit)) & ((1 << l2_tag_bit) - 1);
	int32_t set_l2 = (addr >> l2_sets_bit) & (l2_sets - 1);
	int i;
	for (i = 0; i < l2_ways; i++) {	//在set中每个block检查
		if ( !l2_cache[set_l2][i].valid ) continue;
		if ( l2_cache[set_l2][i].tag == tag_l2 ) {
			l2_t += 2;
			return i;
		}
	}
	/* miss */
	// 找到位置
	for (i = 0; i < l2_ways; i++) {			// 有空位
		if ( !l2_cache[set_l2][i].valid ) break;
	}
	if(i == l2_ways) i = rand() % l2_ways;	// 替换算法 write back 需要写回
	if(l2_cache[set_l2][i].valid && l2_cache[set_l2][i].dirty) { // 被改动了，需要写回
		uint8_t tmp[BURST_LEN * 2];
        memset(tmp, 1, sizeof(tmp));
		int j;
        for (j = 0; j < block_size / BURST_LEN; j++) {
            ddr3_write(((addr >> block_size_bit) << block_size_bit) + BURST_LEN * j, l2_cache[set_l2][i].block + BURST_LEN * j, tmp);
        }
	}
	// 复制到这个块
	l2_cache[set_l2][i].valid = true; l2_cache[set_l2][i].tag = tag_l2; l2_cache[set_l2][i].dirty = false;
	int j;
	for(j = 0; j < block_size / BURST_LEN; j++) {
        ddr3_read(((addr >> block_size_bit) << block_size_bit) + BURST_LEN * j,  l2_cache[set_l2][i].block + BURST_LEN * j);
    }
	l2_t += 200;
	return i;
}

void l1_write(hwaddr_t addr,size_t len, uint32_t data) {
	/* write through	&	not write allocate */
	int32_t tag_l1 = (addr >> (l1_sets_bit + block_size_bit)) & ((1 << l1_tag_bit) - 1);
	int32_t set_l1 = (addr >> l1_sets_bit) & (l1_sets - 1);
	int32_t imm_l1 = (addr & (block_size - 1));
	bool hit = false;
	int i;
	for (i = 0; i < l1_ways; i++) {	//在set中每个block检查
		if ( !l1_cache[set_l1][i].valid ) continue;
		if ( l1_cache[set_l1][i].tag == tag_l1 ) {
			hit = true; break;
		}
	}
	if(hit) {	// write through 都改
		if(imm_l1 + len < block_size) {
			memcpy(l1_cache[set_l1][i].block + imm_l1, &data, len);	// l1
			l2_write(addr, len, data);											// l2
			dram_write(addr, len, data);										// dram
		}
		else {	// 两个块
			memcpy(l1_cache[set_l1][i].block + imm_l1, &data, block_size - imm_l1);				// 低位低地址
			l2_write(addr, block_size - imm_l1, data);
			dram_write(addr, block_size - imm_l1, data);
			l1_write(addr + block_size - imm_l1, len - block_size + imm_l1, data >> (block_size - imm_l1)); // 高位高地址
		}
	}
	else { // not write allocate 直接往l2写
		l2_write(addr, len, data);
	}
}
void l2_write(hwaddr_t addr,size_t len, uint32_t data) {
	/* write back	&	write allocate */
	int32_t tag_l2 = (addr >> (l2_sets_bit + block_size_bit)) & ((1 << l2_tag_bit) - 1);
	int32_t set_l2 = (addr >> l2_sets_bit) & (l2_sets - 1);
	int32_t imm_l2 = (addr & (block_size - 1));
	bool hit = false;
	int i;
	for (i = 0; i < l2_ways; i++) {	//在set中每个block检查
		if ( !l2_cache[set_l2][i].valid ) continue;
		if ( l2_cache[set_l2][i].tag == tag_l2 ) {
			hit = true; break;
		}
	}
	if(hit) {	// write back 只改l2, 不改dram
		l2_cache[set_l2][i].dirty = true;
		if(imm_l2 + len < block_size) {
			memcpy(l2_cache[set_l2][i].block + imm_l2, &data, len);	// l2
		}
		else {	// 两个块
			memcpy(l2_cache[set_l2][i].block + imm_l2, &data, block_size - imm_l2);							// 低位低地址
			l2_write(addr + block_size - imm_l2, len - block_size + imm_l2, data >> (block_size - imm_l2)); // 高位高地址
		}
	}
	else {	// write allocate, 调入再写入
		i = l2_read(addr);
		l2_write(addr, len, data);
	}
}