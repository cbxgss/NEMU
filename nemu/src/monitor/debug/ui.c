#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/cache.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args){
	if(args == NULL) cpu_exec(1);
	else cpu_exec(atoi(args));
	return 0;
}

static int cmd_info(char *args){
	if(*args == 'r'){
		int i;
		for(i = 0; i < 8; i++){
			printf("%s: 0x%x\t", regsl[i], cpu.gpr[i]._32);
		} puts("");
		for(i = 0; i < 8; i++){
			printf("%s : 0x%x\t", regsw[i], cpu.gpr[i]._16);
		} puts("");
		for(i = 0; i < 4; i++){
			printf("%s : 0x%x\t%s : 0x%x\t", regsb[i], cpu.gpr[i]._8[0], regsb[i+4], cpu.gpr[i]._8[1]);
		} puts("");
		printf("eip: 0x%x\n", cpu.eip);
		printf("|GDTR:\tbase: 0x%x\tlimit: 0x%x\t\t|\t\t", cpu.GDTR.base, cpu.GDTR.limit);
		printf("page dictionary:\tbase: 0x%x|\n", cpu.cr3.page_directory_base);
		printf("selector:\tCS: %d\tDS: %d\tSS: %d\tES: %d\n", cpu.CS.selector, cpu.DS.selector, cpu.SS.selector, cpu.ES.selector);
		printf("base:\t\tCS: %d\tDS: %d\tSS: %d\tES: %d\n", cpu.CS.base, cpu.DS.base, cpu.SS.base, cpu.ES.base);
	}
	else if(*args == 'w'){
		info_w();
	}
	return 0;
}

static int cmd_x(char *args){
	char s[2] = " ";
	char *len, *x0;
	len = strtok(args, s);//第一个参数 长度
    int len_ = atoi(len);
    x0 = strtok(NULL, s);//第二个参数 初始位置
	// int x0_ = strtol(x0, NULL, 16);
	bool success; int x0_ = expr(x0, &success);
	if(!success) printf("x Wrong\n");
	int i;
	for( i=0; i < len_; i++){
		if(i % 8 == 0) printf("0x%06x \t", x0_ + i);
		printf("%02x ", swaddr_read(x0_ + i, 1, R_DS));
		if((i+1) % 8 == 0) printf("\n");
	}
	if((i+1) % 8) printf("\n");
	return 0;
}

static int cmd_p(char *args){
	bool success;
	int rax;
	rax = expr(args, &success);
	if(!success) printf("p Wrong\n");
	printf("0x%x = %d\n", rax, rax);
	return 0;
}

static int cmd_p_cache() {
	p_cache_t();
	return 1;
}

static int cmd_w(char *args){
	new_wp(args);
	return 0;
}
static int cmd_d(char *args){
	free_wp(find_n(atoi(args)));
	return 0;
}

typedef struct {
    swaddr_t prev_ebp;			//ebp的旧值
    swaddr_t ret_addr;			//返回地址
    uint32_t args[4];			//前4个参数
}PartOfStackFrame;

static int cmd_bt(char *args) {
	// 打印： #0 返回地址(最后会返回 右边的函数 的哪里) in 函数名（4个参数）
	// eg:		#0	当前地址	in	当前所在函数（参数，在#0的栈帧）
	//			#1	#0栈帧中的ret_add	in	左边地址所在的函数（参数，在#1的栈帧）
	// 			#x	#x-1栈帧中的ret_add	in	左边地址所在的函数（参数，在#x的栈帧）
	int i = 0;
	PartOfStackFrame now;		//存栈帧信息
	int ebp = reg_l(R_EBP);		//当前栈帧位置
	//第一个栈帧的信息
	//	栈帧（32位）中，最低4字节存旧ebp（prev_ebp），其次4字节存返回地址（ret_addr），上面4个4字节分别为4个参数
	now.ret_addr = cpu.eip;
	bool first = 1;
	while(ebp) {
		// printf("now ebp : %x\n", ebp);
		int j = 0;
		for (j = 0; j < nr_symtab_entry; j++) {	//扫描all符号表里的函数，看看在不在该函数中
			if ((symtab[j].st_info & 0xf) == STT_FUNC){//是函数
				if(symtab[j].st_value <= now.ret_addr && now.ret_addr < symtab[j].st_value + symtab[j].st_size) {//在里面
					if(first) { printf("#%d\t0x%08x in %s", i++, now.ret_addr, strtab + symtab[j].st_name); first = 0; }
					else printf("#%d\t0x%08x in %s", i++, now.ret_addr + 1, strtab + symtab[j].st_name);
					break;
				}
			}
		}
		//读取当前栈帧信息
		now.prev_ebp = swaddr_read(ebp, 4, R_SS);
		now.ret_addr = swaddr_read(ebp + 4 , 4, R_SS);
		int k = 0;	for(k = 0; k < 4; k++) now.args[k] = swaddr_read(ebp + 8 + 4*k, 4, R_SS);
		printf("(%d, %d, %d, %d)\n", now.args[0], now.args[1], now.args[2], now.args[3]);
		//更新ebp
		ebp = now.prev_ebp;		//更旧一层栈帧
	}
	return 0;
}

hwaddr_t cmd_page_translate(lnaddr_t addr);
static int cmd_page(char* args){
	if(args == NULL) { printf("parameter invalid!\n"); return 0; }
	uint32_t addr;
	sscanf(args, "%x", &addr);
	hwaddr_t ans = cmd_page_translate(addr);
	if(ans) printf("Addr is 0x%08x\n",ans);
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	/* TODO: Add more commands */
	{ "si", "单步执行", cmd_si },
	{ "info", "打印程序状态", cmd_info },
	{ "p", "表达式求值", cmd_p },
	{ "p_cache", "cache命中状况", cmd_p_cache },
	{ "x", "扫描内存", cmd_x },
	{ "w", "设置监视点", cmd_w },
	{ "d", "删除监视点", cmd_d },
	{ "bt", "打印栈帧链", cmd_bt },
	{ "page", "打印对应物理地址", cmd_page}
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
