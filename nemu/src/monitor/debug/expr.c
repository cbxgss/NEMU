#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
	NOTYPE = 256, EQ, Number

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"[0-9]{1,10}", Number},		//数字
	{"\\+", '+'},					// plus
	{"\\-", '-'},					// 减
	{"\\*", '*'},					// 乘
	{"/", '/'},						// 除
	{"\\（", '('},					//	( 
	{"\\）", ')'},					//	)
	{"==", EQ}						// equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		int index = 0;								//当前存到tockens[index]
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */				
				switch(rules[i].token_type) {
					case NOTYPE:{index--; break;}
					case Number: {
						tokens[index].type = Number;
						assert(substr_len <= 31);
						strncpy(tokens[index].str, substr_start, substr_len);
						break;
					}
					case '+': {tokens[index].type = (int)'+'; break;}
					case '-': {tokens[index].type = (int)'-'; break;}
					case '*': {tokens[index].type = (int)'*'; break;}
					case '/': {tokens[index].type = (int)'/'; break;}
					case '(': {tokens[index].type = (int)'('; break;}
					case ')': {tokens[index].type = (int)')'; break;}
					case EQ : {tokens[index].type = EQ; break;}
					default: panic("please implement me");
				}
				index++;									//++
				break;
			}
		}

		if(i == NR_REGEX) {printf("i = %d\n", i);
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}

		nr_token = index;				//个数
	}

	return true; 
}

int check_parentheses(int p, int q) {	//对为1，括号匹配但两边无括号为2，括号非法为0
	int l = p; int num = 0;		//num = '('的个数 - ')'的个数
	for(l = p; l < q; l++) {
		if(tokens[l].type == (int)'(') num++;	//'('
		else if(tokens[l].type == (int)')') {	//')'
			if(!num) return 0;
			else num--;
		}
	}
	if(num) return 0;
	if(tokens[p].type == (int)'(' && tokens[q].type == (int)')') return 1;
	return 2;
}

int find_dp(int p, int q) {				//找到dominant operator
	//在check_parentheses的if语句过滤后，p和q的地方应该都是数字(即使未来考虑了!，q也是数字)
	int index = q;					//index为当前找到的dp
	int i = p; bool flag = 0;		//当身处()时, flag == 1
	int fff = 0;					//没有dp为0，dp为+-为1，dp为*/为2
	for(i = p; i < q; i++){
		if(flag) continue;				//身处括号【二】
		switch (tokens[i].type) {
			case '(': {flag = 1; break;}
			case ')': {flag = 0; break;}
			case '+': {index = i; fff = 1; break;}	//[优先级最低 + 最后]【三，四】
			case '-': {index = i; fff = 1; break;}
			case '*': {
				if(fff == 0) index = i;
				else if(fff == 2) index = i;
				fff = 2; break;
			}
			case '/': {
				if(fff == 0) index = i;
				else if(fff == 2) index = i;
				fff = 2; break;
			}
			default: break;				//不是运算符【一】
		}
	}
	return index;
}

int eval(int p, int q) {
	// printf("%d %d\n", p, q);
	if(p > q) {printf("p Wrong1\n"); return 0;}
	else if (p == q) {							//一个数字?
		if(tokens[p].type != Number) {printf("p Wrong2\n"); return 0;}
		return atoi(tokens[p].str);
	}else if (check_parentheses(p, q) == 0) {printf("p Wrong3\n"); return 0;}
	else if (check_parentheses(p, q) == 1)	{	//一个()
		return eval(p+1, q-1);
	}else {										//原式 = 左式 dp 右式
		int dp = find_dp(p, q);
		if(dp == q) {printf("p Wrong4\n"); return 0;}
		int val1 = eval(p, dp - 1); int val2 = eval(dp + 1, q);
		printf("val1 = %d    val2 = %d\n", val1, val2);
		switch (tokens[dp].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			default : {printf("p Wrong5\n"); return 0;}
		}
	}
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	/* TODO: Insert codes to evaluate the expression. */
	*success = true;
	return eval(0, nr_token - 1);
}