#ifndef EXCEPTION_H
#define EXCEPTION_H
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(0x40000060))
#define IRQ_PENDING_1 ((volatile unsigned int*)(MMIO_BASE + 0x0000b204))

#define TASK_PRIORITY_Q_NUM 3//current it has 2 levels,level 0,level 1
#define TASK_MAX_PRIORITY (TASK_PRIORITY_Q_NUM - 1)//level 0,1

void irq_init();
void task_dispatcher(int prior_level);
#endif