#ifndef __TLB_H__
#define __TLB_H__

#include "common.h"
/******************************
 * TLB 总共有 64 项
 * fully associative
 * 标志位只需要 valid bit
 * 替换算法采用随机方式
 * tag有20位, 即虚拟页号
 ******************************/

#define TLB_SIZE 64

typedef struct{
    bool valid;         // 有效位
    uint32_t tag, data; // tag是虚拟页号 data是物理页号
} TLB;

TLB tlb[TLB_SIZE];
void init_tlb();

int read_tlb(lnaddr_t addr);
void write_tlb(lnaddr_t addr, hwaddr_t haaddr);

#endif