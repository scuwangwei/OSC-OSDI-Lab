#ifndef PM_H
#define PM_H

#define PM_WDOG     ((volatile unsigned int*)(0x3F100024))
#define PM_RSTC     ((volatile unsigned int*)(0x3F10001c))
#define PM_WDOG_MAGIC 0x5a000000
#define PM_RSTC_WRCFG_FULL_RESET 0x00000020

#endif