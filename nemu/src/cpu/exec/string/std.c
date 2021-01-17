#include "cpu/exec/helper.h"

make_helper(std) {
    cpu.DF = 1;
    print_asm("std\n");
    return 1;
}
