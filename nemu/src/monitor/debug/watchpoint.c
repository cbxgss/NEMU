#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *str_){
	if(!free_) assert(0);

	WP* now_wp = free_;		//第一个空闲wp
	free_ = free_->next;

	now_wp->type = 'w'; now_wp->using = 1;
	strcpy(now_wp->str, str_);
	bool success; now_wp->value = expr(str_,&success);
	now_wp->next = NULL;

	if(!head){				//head为空
		head = now_wp;
	}
	else {
		WP *qwq = head;
		while (qwq->next) {	//qwq是上一个监视点
			qwq = qwq->next;
		}
		qwq->next = now_wp;
	}
	printf("Number of watch is : %d\n", now_wp->NO);
	return now_wp;
}

void free_wp(WP* wp){
	if(wp == 0) return ;
	wp->using = 0;
	if(head == wp){			//wp 是 第一个监视点
		head = wp->next; wp->next = 0;

		if(!free_) free_ = wp;
		else {
			WP* qwq = free_;
			while (qwq->next) qwq = qwq->next;		//qwq是当前最后一个free_的wp
			qwq->next = wp;
		}
	}
	else {
		WP *qwq = head;
		while (qwq->next != wp) qwq = qwq->next; 	//qwq->next == wp
		qwq->next = wp->next; wp->next = 0;

		if(!free_) free_ = wp;
		else {
			qwq = free_;
			while (qwq->next) qwq = qwq->next;		//qwq是当前最后一个free_的wp
			qwq->next = wp;
		}
	}
}

int change(){				//判断value是否变化

	WP *i = head; int num=0;//num为变了的个数
	bool success;
	while(i){
		i->new_value = expr(i->str, &success);
		if(i->new_value != i->value) num++;
		i = i->next;
	}
	return num;
}

void info_w(){
	printf("Num\tType\tValue\tWhat\n");
	int i;
	for(i=0; i < NR_WP; i++){
		if(wp_pool[i].using) printf("%2d\t%c\t0x%x\t%s\n", wp_pool[i].NO, wp_pool[i].type, wp_pool[i].value, wp_pool[i].str);
	}
}

WP* find_n(int n){
	if((n < 0) || (NR_WP <= n)) {printf("this wp was not existed\n"); return 0;}
	if(wp_pool[n].using == 0) {printf("this wp was not used\n"); return 0;}
	return &wp_pool[n];
}