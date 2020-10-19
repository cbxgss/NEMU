#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_rel_, SUFFIX)) {
    return 5;
}

#include "cpu/exec/template-end.h"
