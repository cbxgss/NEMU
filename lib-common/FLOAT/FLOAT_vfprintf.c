#include <stdio.h>
#include <stdint.h>
#include "FLOAT.h"
#include <sys/mman.h>

extern char _vfprintf_internal;				//就算换成int也不是真正的函数的地址（尽管反汇编里是）			这里extern只是引入了一个变量，这个变量的的地址 = 这个函数的地址
extern char _fpmaxtostr;
extern char _ppfs_setargs;

extern int __stdio_fwrite(char *buf, int len, FILE *stream);

__attribute__((used)) static int format_FLOAT(FILE *stream, FLOAT f) {
	/* TODO: Format a FLOAT argument `f' and write the formating
	 * result to `stream'. Keep the precision of the formating
	 * result with 6 by truncating. For example:
	 *              f          result
	 *         0x00010000    "1.000000"
	 *         0x00013333    "1.199996"
	 */
	// printf("steam : %p\n", stream);
	// printf("f     : 0x%x\n", f);
	char buf[80];
	// int len = sprintf(buf, "0x%08x", f);
	//	FLOAT f    转    平时写的小数 到buf
	// 符号
	int len = 0;
	if(f < 0) { buf[len++] = '-'; f = -f; }
	// 整数部分，先是倒序，后面再掉头
	int ret = (f >> 16);
	while(ret != 0) { buf[len++] = '0' + ret % 10; ret /= 10; }
	char tmp; int l = 0; int r = len-1;
	if(buf[0] == '-') l++;
	while(l < r) { tmp = buf[l]; buf[l] = buf[r]; buf[r] = tmp; }
	buf[len++] = '.';	//小数点
	// 小数部分(6位)
	f &= 0xffff; int i = 0;
	for(i = 0; i < 6; i++) {
		f *= 10; buf[len++] = '0' + (f >> 16); f &= 0xffff;
	}
	buf[len++] = 0;
	// printf("%s\n", buf);
	return __stdio_fwrite(buf, len, stream);
}

static void modify_vfprintf() {
	/* TODO: Implement this function to hijack the formating of "%f"
	 * argument during the execution of `_vfprintf_internal'. Below
	 * is the code section in _vfprintf_internal() relative to the
	 * hijack.
	 */
	// printf("_vfprintf_internal : %p\n", &_vfprintf_internal);
	void *p = (void *)(int)&_vfprintf_internal + (0x80497f9-0x80494f3);	//指向call指令
	// printf("call <_fpmaxtostr> : %p\n", p);
	// printf("mprotect           : %p, %p\n", (void *)(((int)p - 100) & 0xfffff000), (void *)(((int)p - 100) & 0xfffff000) + 4096*2);
	// mprotect((void *)(((int)p - 100) & 0xfffff000), 4096*2, PROT_READ | PROT_WRITE | PROT_EXEC);			//nemu还不支持系统调用
	p++;													//指向imm
	// printf("imm                : %p\n", p);
	// printf("format_FLOAT       : %p\n", format_FLOAT);
	// printf("_fpmaxtostr        : %p\n", &_fpmaxtostr);
	// printf("p - >                %p\n", (int *)p);
	// printf("imm_prev           : %x = %d\n", *(int *)p, *(int *)p);
	*(int *)p += ((int)format_FLOAT - (int)&_fpmaxtostr);				//修改
	p--;	//指向call
	// 修改成push
	p -= (0xb4 - 0xaa);		//指向fstpt
	*(char *)p = 0xff; p++; *(char *)p = 0x32; p++; *(char *)p = 0x90;//改三个字节
	p -= 2;					//指向fstpt
	// 修改栈空间
	p -= 3;		//指向上一次sub esp
	p += 2;	*(char *)p -= 0x4; p -= 2;
	// 修改浮点指令
	p -= (0x8049de4 - 0x8049dcf);
	*(char *)p = 0x90; *(char *)(p+1) = 0x90;
	*(char *)(p+4) = 0x90; *(char *)(p+5) = 0x90;

#if 0
	else if (ppfs->conv_num <= CONV_A) {  /* floating point */
		ssize_t nf;
		nf = _fpmaxtostr(stream,
				(__fpmax_t)
				(PRINT_INFO_FLAG_VAL(&(ppfs->info),is_long_double)
				 ? *(long double *) *argptr
				 : (long double) (* (double *) *argptr)),
				&ppfs->info, FP_OUT );
		if (nf < 0) {
			return -1;
		}
		*count += nf;

		return 0;
	} else if (ppfs->conv_num <= CONV_S) {  /* wide char or string */
#endif

	/* You should modify the run-time binary to let the code above
	 * call `format_FLOAT' defined in this source file, instead of
	 * `_fpmaxtostr'. When this function returns, the action of the
	 * code above should do the following:
	 */

#if 0
	else if (ppfs->conv_num <= CONV_A) {  /* floating point */
		ssize_t nf;
		nf = format_FLOAT(stream, *(FLOAT *) *argptr);
		if (nf < 0) {
			return -1;
		}
		*count += nf;

		return 0;
	} else if (ppfs->conv_num <= CONV_S) {  /* wide char or string */
#endif

}

static void modify_ppfs_setargs() {
	/* TODO: Implement this function to modify the action of preparing
	 * "%f" arguments for _vfprintf_internal() in _ppfs_setargs().
	 * Below is the code section in _vfprintf_internal() relative to
	 * the modification.
	 */

	void *p = &_ppfs_setargs;
	p += (0x804a0f1 - 0x804a080);		//7的case
	*(char *)p = 0xe9;
	p++;
	*(int *)p = (0x804a123 - 0x804a0f1 - 5);

#if 0
	enum {                          /* C type: */
		PA_INT,                       /* int */
		PA_CHAR,                      /* int, cast to char */
		PA_WCHAR,                     /* wide char */
		PA_STRING,                    /* const char *, a '\0'-terminated string */
		PA_WSTRING,                   /* const wchar_t *, wide character string */
		PA_POINTER,                   /* void * */
		PA_FLOAT,                     /* float */
		PA_DOUBLE,          //7          /* double */
		__PA_NOARG,                   /* non-glibc -- signals non-arg width or prec */
		PA_LAST
	};

	/* Flag bits that can be set in a type returned by `parse_printf_format'.  */
	/* WARNING -- These differ in value from what glibc uses. */
#define PA_FLAG_MASK		(0xff00)
#define __PA_FLAG_CHAR		(0x0100) /* non-gnu -- to deal with hh */
#define PA_FLAG_SHORT		(0x0200)
#define PA_FLAG_LONG		(0x0400)
#define PA_FLAG_LONG_LONG	(0x0800)	//800
#define PA_FLAG_LONG_DOUBLE	PA_FLAG_LONG_LONG
#define PA_FLAG_PTR		(0x1000) /* TODO -- make dynamic??? */

	while (i < ppfs->num_data_args) {
		switch(ppfs->argtype[i++]) {
			case (PA_INT|PA_FLAG_LONG_LONG):
				GET_VA_ARG(p,ull,unsigned long long,ppfs->arg);
				break;
			case (PA_INT|PA_FLAG_LONG):
				GET_VA_ARG(p,ul,unsigned long,ppfs->arg);
				break;
			case PA_CHAR:	/* TODO - be careful */
				/* ... users could use above and really want below!! */
			case (PA_INT|__PA_FLAG_CHAR):/* TODO -- translate this!!! */
			case (PA_INT|PA_FLAG_SHORT):
			case PA_INT:
				GET_VA_ARG(p,u,unsigned int,ppfs->arg);
				break;
			case PA_WCHAR:	/* TODO -- assume int? */
				/* we're assuming wchar_t is at least an int */
				GET_VA_ARG(p,wc,wchar_t,ppfs->arg);
				break;
				/* PA_FLOAT */
			case PA_DOUBLE:
				GET_VA_ARG(p,d,double,ppfs->arg);
				break;
			case (PA_DOUBLE|PA_FLAG_LONG_DOUBLE):
				GET_VA_ARG(p,ld,long double,ppfs->arg);
				break;
			default:
				/* TODO -- really need to ensure this can't happen */
				assert(ppfs->argtype[i-1] & PA_FLAG_PTR);
			case PA_POINTER:
			case PA_STRING:
			case PA_WSTRING:
				GET_VA_ARG(p,p,void *,ppfs->arg);
				break;
			case __PA_NOARG:
				continue;
		}
		++p;
	}
#endif

	/* You should modify the run-time binary to let the `PA_DOUBLE'
	 * branch execute the code in the `(PA_INT|PA_FLAG_LONG_LONG)'
	 * branch. Comparing to the original `PA_DOUBLE' branch, the
	 * target branch will also prepare a 64-bit argument, without
	 * introducing floating point instructions. When this function
	 * returns, the action of the code above should do the following:
	 */

#if 0
	while (i < ppfs->num_data_args) {
		switch(ppfs->argtype[i++]) {
			case (PA_INT|PA_FLAG_LONG_LONG):
			here:
				GET_VA_ARG(p,ull,unsigned long long,ppfs->arg);
				break;
			// ......
				/* PA_FLOAT */
			case PA_DOUBLE:
				goto here;
				GET_VA_ARG(p,d,double,ppfs->arg);
				break;
			// ......
		}
		++p;
	}
#endif

}

void init_FLOAT_vfprintf() {
	modify_vfprintf();
	modify_ppfs_setargs();
}
