#include "../FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
	// nemu_assert(0);
	// a * b = A >> 16 * B >> 16 = (A * B >> 16) >> 16
	// 2^32 * 2^32 = 2^64	乘法后最多需要64位存
	long long x = (long long)a * (long long)b;	//防止溢出
	return (FLOAT)(x >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
	/* Dividing two 64-bit integers needs the support of another library
	 * `libgcc', other than newlib. It is a dirty work to port `libgcc'
	 * to NEMU. In fact, it is unnecessary to perform a "64/64" division
	 * here. A "64/32" division is enough.
	 *
	 * To perform a "64/32" division, you can use the x86 instruction
	 * `div' or `idiv' by inline assembly. We provide a template for you
	 * to prevent you from uncessary details.
	 *
	 *     asm volatile ("??? %2" : "=a"(???), "=d"(???) : "r"(???), "a"(???), "d"(???));
	 *
	 * If you want to use the template above, you should fill the "???"
	 * correctly. For more information, please read the i386 manual for
	 * division instructions, and search the Internet about "inline assembly".
	 * It is OK not to use the template above, but you should figure
	 * out another way to perform the division.
	 */

	// nemu_assert(0);
	// 符号(直接在整数部分处理的话，可能有1的误差)：
	int qwq = 1;
	if(a < 0) { qwq *= -1; a = -a; }
	if(b < 0) { qwq *= -1; b = -b; }
	// 整数部分：a/b
	FLOAT x = (a / b);
	// 小数部分：
	// for 每一位小数：		x = x * 10 + 1 ? 0 : 还有
	a = a % b;
	int i = 0;
	for(i; i < 16; i++){
		a <<= 1; x <<= 1;
		if(a >= b){	//（a<<i / b） >= 1时
			a -= b; x++;
		}
	}
	return x * qwq;
}

FLOAT f2F(float a) {
	int b = *(int *)&a;
	int sign = b >> 31;
	int exp = (b >> 23) & 0xff;
	FLOAT k = b & 0x7fffff;
	if (exp != 0) k += 1 << 23;
	exp -= 150;
	if (exp < -16) k >>= -16 - exp;
	if (exp > -16) k <<= exp + 16;
	return sign == 0 ? k : -k;
}
FLOAT Fabs(FLOAT a) {	//返回浮点数的绝对值
	// nemu_assert(0);
	if(a < 0) return -a;
	else return a;
}

/* Functions below are already implemented */

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

