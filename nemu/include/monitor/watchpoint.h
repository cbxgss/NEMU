#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	/* TODO: Add more members if necessary */
	char type;		//w or b
	int using;		//正在使用
	int value;		//旧值
	int new_value;	//新值
	char str[32];	//被监视的表达式
} WP;

int change();
void info_w();
WP* new_wp(char *str_);
void free_wp(WP* wp);
WP* find_n(int n);

#endif
