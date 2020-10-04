#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	/* TODO: Add more members if necessary */
	int value;		//旧值
	int new_value;	//新值
	char str[32];	//被监视的表达式
} WP;

#endif
