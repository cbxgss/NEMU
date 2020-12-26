# NEMU2020 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System) in College of Intelligence and Computing, Tianjin Univerisity.

This project is introduced from Nanjin University in 2016. Thank you for  Prof. Chunfeng Yuan of NJU and Dr. Zihao Yu of ICT.

The following subprojects/components are included. Some of them are not fully implemented.
* NEMU
* testcase
* uClibc
* kernel
* typing game
* NEMU-PAL

## NEMU

NEMU(NJU Emulator) is a simple but complete full-system x86 emulator designed for teaching. It is the main part of this programming assignment. Small x86 programs can run under NEMU. The main features of NEMU include
* a small monitor with a simple debugger
 * single step
 * register/memory examination
 * expression evaluation with the support of symbols
 * watch point
 * backtrace
* CPU core with support of most common used x86 instructions in protected mode
 * real mode is not supported
 * x87 floating point instructions are not supported
* DRAM with row buffer and burst
* two-level unified cache
* IA-32 segmentation and paging with TLB
 * protection is not supported
* IA-32 interrupt and exception
 * protection is not supported
* 6 devices
 * timer, keyboard, VGA, serial, IDE, i8259 PIC
 * most of them are simplified and unprogrammable
* 2 types of I/O
 * port-mapped I/O and memory-mapped I/O

## testcase

Some small C programs to test the implementation of NEMU.

## uClibc

uClibc(https://www.uclibc.org/) is a C library for embedding systems. It requires much fewer run-time support than glibc and is very friendly to NEMU.

## kernel

This is the simplified version of Nanos(http://cslab.nju.edu.cn/opsystem). It is a uni-tasking kernel with the following features.
* 2 device drivers
 * Ramdisk
 * IDE
* ELF32 loader
* memory management with paging
* a simple file system
 * with fix number and size of files
 * without directory
* 6 system calls
 * open, read, write, lseek, close, brk

## typing game

This is a fork of the demo of NJU 2013 oslab0(the origin repository has been deleted, but we have a fork of it -- https://github.com/nju-ics/os-lab0). It is ported to NEMU.

## NEMU-PAL

This is a fork of Wei Mingzhi's SDLPAL(https://github.com/CecilHarvey/sdlpal). It is obtained by refactoring the original SDLPAL, as well as porting to NEMU.





# PA3

## 分段

### 16位的8086

$$
physical\_address = (seg\_reg<<4) + offset
$$

- seg_reg： 段寄存器的值

- offset：偏移量

- seg_reg和offset的搭配：

	|              |                     seg_reg寄存器 16位                     | offset寄存器 |                             说明                             |
	| :----------: | :--------------------------------------------------------: | :----------: | :----------------------------------------------------------: |
	|    取指令    |                             CS                             |   IP(eip)    |                                                              |
	| 内存数据访问 |  DS(Data Segment)，也可以显式使用ES(Extra Segment)<br />   |              | $$mov\quad\%ax,(\%bx)$$<br />用寄存器传输语言（RTL）描述就是：$$M[DS:BX]<-R[AX]$$ |
	|     堆栈     | SS(stack segment) 或 SP(stack pointer) 和 BP(base pointer) |              |                                                              |
	|    字符串    |                             ES                             |              |                                                              |

#### 问题：

1. 1MB内存容量的瓶颈
2. 恶意程序(段寄存器随便改)

### 32位的80386

- CR0 寄存器：

	32位4GB足够了，理论上可以去掉段寄存器，但为了兼容，设置了CR0寄存器的PE位作为开关，为1时进入保护模式

- GDT **全局**描述符表：

  一个数组，数组元素是段描述符，在80386里，段寄存器存其索引

  Segment Descriptor 段描述符：64位，需要放内存里

  ![image-20201225200318999](README.assets/image-20201225200318999.png)

- GDTR 寄存器：

	存放GDT的首地址（线性地址，无需再经过分段机制的地址翻译）和长度

- LDT 每个进程的描述符表

- LDTR 寄存器：

	存放LDT的位置(实际上是LDT段在GDT中的索引)

- 段寄存器：

	- 16位段选择符

		index：段描述符的索引

		![image-20201225192350055](README.assets/image-20201225192350055.png)

	- 隐藏描述：

		从描述符表中获取：base addr, limit, type, other information

	- ![image-20201226113956639](README.assets/image-20201226113956639.png)

#### 80386的分段机制

1. 通过段寄存器的段选择符TI位，决定在哪个表里查找（在LDT还是GDT）
2. 读GDTR或LDTR
3. 读段选择符的index，找到段描述符
4. 在段描述符中读段的基地址，和虚拟地址相加得线性地址

### 80286就有了的进程特权等级

| 进程等级（0最高） |      进程       |
| :---------------: | :-------------: |
|         0         | 内核，GDT，页表 |
|         1         |                 |
|         2         |   一般的程序    |
|         3         |                 |

- DPL域：在段描述符中，描述一个段所在的特权级
- RPL域：段选择符中，描述了请求者所在的特权级
- CPL：指示当前进程的特权级，一般与当前CS寄存器指向的段描述符的DPL相等

$$
数据段切换操作合法\quad iff\quad 请求者和当前进程比目标段权限高
$$

