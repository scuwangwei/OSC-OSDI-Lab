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
#define ENB_IRQs1 ((volatile unsigned int*)(MMIO_BASE + 0x0000b210))
#define DISABLE_IRQs1 	 ((volatile unsigned int*)(MMIO_BASE + 0x0000b21c))

#define RBUF_SIZE 256

typedef struct {
    char buf[RBUF_SIZE];
    int head; // write point
    int tail; // read point
} ringBuffer;

void mini_uart_init();
char mini_uart_read();
void mini_uart_write(char tmp);
void mini_uart_read_string(char *buffer, int max_len);
void mini_uart_send_string(const char *str);
void mini_uart_send_hex(unsigned int num);
char mini_uart_read_raw();
void aux_irq_enable();
void aux_irq_disable();
void mini_uart_trans_irq_enable();
void mini_uart_trans_irq_disable();
void mini_uart_recv_irq_enable();
void mini_uart_recv_irq_disable();
int mini_uart_write_async(const char tmp, int isAsycn);
void mini_uart_send_string_async(const char *str);
int mini_uart_read_async(char *c);
void mini_uart_read_string_async(char *buffer, int max_len);
void rbuf_init(ringBuffer *rbuf);
int rbuf_is_empty(ringBuffer *rbuf);
int rbuf_is_full(ringBuffer *rbuf);
int rbuf_put(ringBuffer *rbuf, char c);
int rbuf_get(ringBuffer *rbuf, char *c);
void mini_uart_handler();


#endif