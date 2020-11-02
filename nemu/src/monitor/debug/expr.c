#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
	NOTYPE = 256, EQ, Number, Hex, Reg, NEQ, Deref, LY, LH, X 		//X是变量或符号

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
	{"0[xX][A-Fa-f0-9]{1,8}", Hex},	//16进制
	{"[0-9]{1,10}", Number},		//数字
	{"[a-zA-Z_0-9]+", X},			// 变量
	{"\\$(e?(ax|dx|cx|bx|si|di|sp|bp|ip)|[a-d][hl])", Reg},	//寄存器

	{"\\+", '+'},					// plus
	{"\\-", '-'},					// 减
	{"\\*", '*'},					// 乘
	{"/", '/'},						// 除
	{"\\(", '('},					//	(
	{"\\)", ')'},					//	)

	{"==", EQ},						// equal
	{"!=", NEQ},					//不等
	{"&&", LY},						//逻辑与
	{"\\|\\|", LH},					//逻辑或

	{"!", '!'}						//逻辑非
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

bool is_Deref_or_(int i){
	if(i == 0) return 1;
	if(tokens[i-1].type == '+') return 1;
	if(tokens[i-1].type == '-') return 1;
	if(tokens[i-1].type == '*') return 1;
	if(tokens[i-1].type == '/') return 1;
	if(tokens[i-1].type == '(') return 1;
	if(tokens[i-1].type == '!') return 1;
	if(tokens[i-1].type == EQ) return 1;
	if(tokens[i-1].type == NEQ) return 1;
	if(tokens[i-1].type == LY) return 1;
	if(tokens[i-1].type == LH) return 1;
	if(tokens[i-1].type == Deref) return 1;
	return 0;
}

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;
	int index = 0;								//当前存到tockens[index]

	for(i=0; i < 32; i++){
		tokens[i].type = 666;
		memset(tokens[i].str, 0, sizeof(tokens[i].str));
	}

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				// Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);			//太烦了，不要了
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */				

				switch(rules[i].token_type) {
					case NOTYPE:{index--; break;}
					case Number: {
						tokens[index].type = Number;
						strncpy(tokens[index].str, substr_start, substr_len);
						break;
					}
					case Hex: {
						tokens[index].type = Hex;
						strncpy(tokens[index].str, substr_start, substr_len);
						break;
					}
					case Reg: {
						tokens[index].type = Reg;
						strncpy(tokens[index].str, substr_start, substr_len);
						break;
					}
					case X: {
						tokens[index].type = X;
						strncpy(tokens[index].str, substr_start, substr_len);
						break;
					}
					case '!': {tokens[index].type = (int)'!'; break;}
					case '+': {tokens[index].type = (int)'+'; break;}
					case '-': {tokens[index].type = (int)'-'; break;}
					case '*': {tokens[index].type = (int)'*'; break;}
					case '/': {tokens[index].type = (int)'/'; break;}
					case '(': {tokens[index].type = (int)'('; break;}
					case ')': {tokens[index].type = (int)')'; break;}
					case EQ : {tokens[index].type = EQ; break;}
					case NEQ: {tokens[index].type = NEQ; break;}
					case LY : {tokens[index].type = LY; break;}
					case LH : {tokens[index].type = LH; break;}
					default: panic("please implement me");
				}
				// printf("index = %d\n", index);
				index++;									//++
				break;
			}
		}

		if(i == NR_REGEX) {printf("i = %d\n", i);
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
		// printf("index_now = %d\n", index);
	}
	nr_token = index;				//个数
	return true; 
}

int check_parentheses(int p, int q) {	//对为1，括号匹配但两边无括号为2，括号非法为0
	int l = p; int num = 0;		//num = '('的个数 - ')'的个数
	for(l = p; l <= q; l++) {
		if(tokens[l].type == (int)'(') num++;	//'('
		else if(tokens[l].type == (int)')') {	//')'
			if(!num) return 0;
			else num--;
		}
	}
	if(num) return 0;
	if(tokens[p].type == (int)'(' && tokens[q].type == (int)')') {		//两边有()
		int l = p+1; int num = 0;		//num = '('的个数 - ')'的个数
		for(l = p+1; l < q; l++) {
			if(tokens[l].type == (int)'(') num++;		//'('
			else if(tokens[l].type == (int)')') num--;	//')'
			if(num < 0) return 2;
		}
		return 1;
	}
	return 2;
}

int find_dp(int p, int q) {				//找到dominant operator
	//在check_parentheses的if语句过滤后，p和q的地方应该都是数字(即使未来考虑了!，q也是数字)
	int index = q;					//index为当前找到的dp
	int i = p; int flag = 0;		// flag = (的个数 - )的个数
	int fff = 0;					// 当前没有dp :0; 正负号,!* :1;	*/ :2;	+- :3;
	/// fff表示作为dp的优先级，越大越优先	==,!= :4,	&& :5;	|| :6

	for(i = p; i < q; i++){
		// printf("%c ", tokens[i].type);
		if(flag && (tokens[i].type != '(') && (tokens[i].type != ')')) continue;	//身处括号【二】
		switch (tokens[i].type) {
			case '(': {flag++; break;}
			case ')': {flag--; break;}

			case '!': {}
			case Deref: {
				if(fff < 1) {index = i; fff = 1;}
				break;
			}
			case '*': {}
			case '/': {
				if(fff <= 2) {index = i; fff = 2;}
				break;
			}
			case '+': {}					//[优先级最低 + 最后]【三，四】
			case '-': {
				if(i == p) {//第一个符号是正负号
					index = i; fff = 1; break;
				}
				if(is_Deref_or_(i)){//正负号
					if(fff < 1) {index = 1; fff = 1;}
					break;
				}
				if(fff <= 3) {index = i; fff = 3;}
				break;
			}
			case EQ : {}
			case NEQ: {
				if(fff <= 4) {index = i; fff = 4;}
				break;
			}
			case LY : {
				if(fff <= 5) {index = i; fff = 4;}
				break;
			}
			case LH : {
				if(fff <= 6) {index = i; fff = 4;}
				break;
			}
			default: break;							//不是运算符【一】
		}
	}
	// printf("\tdp = %d\n", index);
	return index;
}

int eval(int p, int q) {
	// printf("nr_tocken = %d\n", nr_token);
	// printf("eval : %d %d\n", p, q);
	if(p > q) {printf("p Wrong1\n"); return 0;}
	else if (p == q) {							//一个数字?
		// printf("nuber's type : %d or %c\n", tokens[p].type, tokens[p].type);
		if(tokens[p].type == Number) return atoi(tokens[p].str);
		if(tokens[p].type == Hex) return strtol(tokens[p].str, NULL, 16);
		if(tokens[p].type == Reg) {		//寄存器
			// printf("eax = 0x%X\n", cpu.gpr[0]._32);
			int i;
			for(i = 0; i < 8; i++){
				if(strcmp(tokens[p].str + 1, regsl[i]) == 0) return cpu.gpr[i]._32;
				if(strcmp(tokens[p].str + 1, regsw[i]) == 0) return cpu.gpr[i]._16;
			}
			for(i = 0; i < 4; i++){
				if(strcmp(tokens[p].str + 1, regsb[i]) == 0) return cpu.gpr[i]._8[0];
				if(strcmp(tokens[p].str + 1, regsb[i+4]) == 0) return cpu.gpr[i]._8[1];
			}
			if(strcmp(tokens[p].str +1, "eip") == 0) return cpu.eip;
		}
		if(tokens[p].type == X) {				//变量或符号
		printf("%s :", tokens[p].str);
			int i;
			for (i = 0; i < nr_symtab_entry; i++) {
				if ((symtab[i].st_info & 0xf) == STT_OBJECT){		//在OBJECT里找
					char tmp [50];
					int tmplen = symtab[i+1].st_name - symtab[i].st_name - 1;
					strncpy (tmp, strtab + symtab[i].st_name, tmplen);
					// printf("%s :", tmp);
					tmp [tmplen] = 0;
					printf("%s :", tmp);
					// if (strcmp (tmp, tokens[p].str) == 0) return symtab[i].st_value;
					return symtab[i+1].st_name - symtab[i].st_name - 1;
				}
			}
		}
		printf("p Wrong2\n"); return 0;
	}
	else if (check_parentheses(p, q) == 1)	{	//一个()
		return eval(p+1, q-1);
	}
	else if (check_parentheses(p, q) == 0) {printf("p Wrong3\n"); return 0;}
	else {										//原式 = 左式 dp 右式
		int dp = find_dp(p, q); int val1 = 0, val2 = 0;
		if(dp == q) {printf("p Wrong4\n"); return 0;}
		if(dp == p){
			if(tokens[dp].type == '+') {val1 = 0; val2 = eval(p+1, q);}
			else if(tokens[dp].type == '-') {val1 = 0; val2 = eval(p+1, q);}
			else if(tokens[dp].type == '!') return !eval(p+1, q);
			else if(tokens[dp].type == Deref)  return swaddr_read(eval(p+1, q), 4);
		}else{
			val1 = eval(p, dp - 1); val2 = eval(dp + 1, q);
		}
		// printf("val1 = %d    val2 = %d\n", val1, val2);
		switch (tokens[dp].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case EQ : return (val1 == val2);
			case NEQ: return (val1 != val2);
			case LY : return (val1 && val2);
			case LH : return (val1 || val2);
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
	int i=0;
	for(i = 0; i < nr_token; i++){
		if(tokens[i].type == '*'){
			if(is_Deref_or_(i)) tokens[i].type = Deref;
		}
	}
	*success = 1;
	return eval(0, nr_token - 1);
}