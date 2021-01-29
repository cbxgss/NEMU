#include "prefix/prefix.h"

#include "data-mov/mov.h"
#include "data-mov/xchg.h"
#include "data-mov/cltd.h"
#include "data-mov/leave.h"
#include "data-mov/movext.h"
#include "data-mov/pop.h"
#include "data-mov/push.h"
#include "data-mov/lgdt.h"      // load m into GDTR
#include "data-mov/lidt.h"      // load into LIDR

#include "arith/dec.h"
#include "arith/inc.h"
#include "arith/neg.h"
#include "arith/imul.h"
#include "arith/mul.h"
#include "arith/idiv.h"
#include "arith/div.h"
#include "arith/sub.h"
#include "arith/add.h"
#include "arith/adc.h"
#include "arith/sbb.h"

#include "logic/and.h"
#include "logic/or.h"
#include "logic/not.h"
#include "logic/xor.h"
#include "logic/sar.h"
#include "logic/shl.h"
#include "logic/shr.h"
#include "logic/shrd.h"
#include "logic/setne.h"
#include "logic/sete.h"

#include "string/rep.h"
#include "string/movs.h"
#include "string/lods.h"
#include "string/scas.h"
#include "string/stos.h"
#include "string/cld.h"
#include "string/std.h"

#include "misc/misc.h"

#include "special/special.h"

#include "data-control/call.h"
#include "data-control/jcc/je.h"
#include "data-control/cmp.h"
#include "data-control/ret.h"
#include "data-control/jmp.h"
#include "data-control/jcc/jbe.h"
#include "data-control/test.h"
#include "data-control/jcc/jne.h"
#include "data-control/jcc/jle.h"
#include "data-control/jcc/jg.h"
#include "data-control/jcc/jl.h"
#include "data-control/jcc/jge.h"
#include "data-control/jcc/ja.h"
#include "data-control/jcc/js.h"
#include "data-control/jcc/jns.h"
#include "data-control/jcc/jb.h"

#include "CMOV/cmovns.h"