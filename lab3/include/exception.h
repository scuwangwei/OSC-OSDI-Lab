#ifndef EXCEPTION_H
#define EXCEPTION_H
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(0x40000060))
#define IRQ_PENDING_1 ((volatile unsigned int*)(MMIO_BASE + 0x0000b204))
#endif