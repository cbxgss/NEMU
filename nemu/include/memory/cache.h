#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

/* block */
#define block_size_bit 6		// addr : 6位块内偏移
#define block_size 64			// 一个块的字节
typedef struct {
	uint32_t tag;					// 标记位(19位)
	uint8_t block[block_size];		// 64个字节
	bool valid, dirty;			    // 有效位 和 脏位
} block;

/* L1 cache
 * write through	&	not write allocate
 */
#define l1_ways 8					// 8路组相连	1个set = 8个块
#define l1_sets_bit 7				// addr : 组号的位数
#define l1_sets (1024/8)			// cache的组数	块数/8
block l1_cache[l1_sets][l1_ways];
uint64_t l1_t;			// 计时变量

/* L2 cache
 * write back	&	write allocate
 */
#define l2_ways 16					// 16路组相连
#define l2_sets_bit 12				// addr : 组号的位数
#define l2_sets (64*1024/16)		//l2组数 块数/16
block l2_cache[l2_sets][l2_ways];
uint64_t l2_t;			// 计时变量

void init_cache();
void p_cache_t();

// l1 addr : 32位 = 19位tags + 7位sets + 6位块内偏移
#define l1_tag_bit 19
// l2 addr : 32位 = 14位tags + 12位sets + 6位块内偏移
#define l2_tag_bit 14

int32_t l1_read(hwaddr_t addr);
int32_t l2_read(hwaddr_t addr);
void l1_write(hwaddr_t addr,size_t len, uint32_t data);
void l2_write(hwaddr_t addr,size_t len, uint32_t data);

#endif