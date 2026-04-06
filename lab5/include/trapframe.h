#ifndef TRAPFRAME_H
#define TRAPFRAME_H
#include <stdint.h>

/*
User code
   ↓
x0, x1, x2.. 放參數
x8 放 syscall number
   ↓
svc 0
   ↓
Exception handler (EL1)
   ↓
讀 trap frame
   ↓
dispatch syscall
   ↓
改 x0（return value）
   ↓
eret 回 user
*/
struct trap_frame {
   uint64_t spsr_el1;   // Saved processor state
   uint64_t elr_el1;    // Return address
   uint64_t x[31];      // General purpose registers x0-x30
};

#endif