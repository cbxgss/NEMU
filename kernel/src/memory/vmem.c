#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)
#define HAS_DEVICE
/* Use the function to get the start address of user page directory. */
PDE* get_updir();
static PTE ptable[1024] align_to_page;

void create_video_mapping() {
	/* TODO: create an identical mapping from virtual memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area 
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */
	// panic("please implement me");
	PDE *pdir = (PDE *)va_to_pa(get_updir());
	pdir[0].val = make_pde(va_to_pa(ptable));
	uint32_t i, start, end;
	start = VMEM_ADDR / PAGE_SIZE;
	end = start + SCR_SIZE / PAGE_SIZE;
	if(SCR_SIZE % PAGE_SIZE) end++;
	// fill PTEs
	for (i = start; i <= end; i++) {
		ptable[i].val = make_pte(i << 12);
	}
}

void video_mapping_write_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		buf[i] = i;
	}
}

void video_mapping_read_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		assert(buf[i] == i);
	}
}

void video_mapping_clear() {
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}

