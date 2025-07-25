#ifndef MINI_UART_H
#define MINI_UART_H

#include "base.h"

#define AUX_ENABLE  ((volatile unsigned int *)(MMIO_BASE + 0x00215004))
#define AUX_MU_CNTL_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215060))
#define AUX_MU_IER_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215044))
#define AUX_MU_LCR_REG ((volatile unsigned int *)(MMIO_BASE + 0x0021504C))
#define AUX_MU_MCR_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215050))
#define AUX_MU_BAUD ((volatile unsigned int *)(MMIO_BASE + 0x00215068))
#define AUX_MU_IIR_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215048))
#define AUX_MU_IO_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215040))
#define AUX_MU_LSR_REG ((volatile unsigned int *)(MMIO_BASE + 0x00215054))

void mini_uart_init();
char mini_uart_read();
void mini_uart_write(char tmp);
void mini_uart_read_string(char *buffer, int max_len);
void mini_uart_send_string(const char *str);
void mini_uart_send_hex(unsigned int num);

#endif