//
//  m3_exec_defs.h
//
//  Created by Steven Massey on 5/1/19.
//  Copyright © 2019 Steven Massey. All rights reserved.
//

#ifndef m3_exec_defs_h
#define m3_exec_defs_h

#include "m3_core.h"

#if defined(__cplusplus)
extern "C" {
#endif

#   define d_m3OpSig                pc_t _pc, u64 * _sp, M3MemoryHeader * _mem, m3reg_t _r0, f64 _fp0
#   define d_m3OpArgs               _sp, _mem, _r0, _fp0
#   define d_m3OpAllArgs            _pc, _sp, _mem, _r0, _fp0
#   define d_m3OpDefaultArgs        0, 0.
#	define d_m3ClearRegisters		_r0 = 0; _fp0 = 0.;

#   define m3MemData(mem)           (u8*)((M3MemoryHeader*)(mem)+1)

typedef m3ret_t (vectorcall * IM3Operation) (d_m3OpSig);

#if defined(__cplusplus)
}
#endif

#endif // m3_exec_defs_h
