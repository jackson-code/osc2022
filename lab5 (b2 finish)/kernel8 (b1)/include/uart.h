/* ref: BCM2837-ARM-Peripherals */

#ifndef UART_H
#define UART_H

#include "gpio.h"
#include "address.h"

// mini UART registers
// ch 2.2.2
#define AUX_ENB 				((volatile unsigned int*)(MMIO_BASE + 0x00215004))
#define AUX_MU_IO_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215040))
#define AUX_MU_IER_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215044))
#define AUX_MU_IIR_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215048))
#define AUX_MU_LCR_REG  		((volatile unsigned int*)(MMIO_BASE + 0x0021504C))
#define AUX_MU_MCR_REG  		((volatile unsigned int*)(MMIO_BASE + 0x00215050))
#define AUX_MU_LSR_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215054))
#define AUX_MU_MSR_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215058))
#define AUX_MU_SCRATCH_REG  	((volatile unsigned int*)(MMIO_BASE + 0x0021505C))
#define AUX_MU_CNTL_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215060))
#define AUX_MU_STAT_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215064))
#define AUX_MU_BAUD_REG			((volatile unsigned int*)(MMIO_BASE + 0x00215068))



#define UART0_BASE      (MMIO_BASE + 0x201000)

#define UART0_DR        ((unsigned int*)(UART0_BASE))
#define UART0_FR        ((unsigned int*)(UART0_BASE + 0x18))
#define UART0_IBRD      ((unsigned int*)(UART0_BASE + 0x24))
#define UART0_FBRD      ((unsigned int*)(UART0_BASE + 0x28))
#define UART0_LCRH      ((unsigned int*)(UART0_BASE + 0x2C))
#define UART0_CR        ((unsigned int*)(UART0_BASE + 0x30))
#define UART0_IMSC      ((unsigned int*)(UART0_BASE + 0x38))
#define UART0_RIS       ((unsigned int*)(UART0_BASE + 0x3C))
#define UART0_MIS       ((unsigned int*)(UART0_BASE + 0x40))
#define UART0_ICR       ((unsigned int*)(UART0_BASE + 0x44))

#define UART_QUEUE_MAX_SIZE 2048
#define UART0_BUF_MAX_SIZE  128

struct uart_queue {  // circular queue
    int front;
    int rear;
    int max;
    char buf[UART_QUEUE_MAX_SIZE];
};

//#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(0x40000060))

// ch 7.5
//#define ARM_IRQ_REG_BASE 		((volatile unsigned int*)(MMIO_BASE + 0x0000b000))
#define ENB_IRQS1 		 		((volatile unsigned int*)(MMIO_BASE + 0x0000b210))
#define DISABLE_IRQS1 	 		((volatile unsigned int*)(MMIO_BASE + 0x0000b21c))
#define AUX_IRQ (1 << 29)		// Auxiliary peripherals: One mini UART and two SPI masters, ch7.5-table2

#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int *)(0x40000060))

#define MAX_BUFFER_LEN 128
char read_buf[MAX_BUFFER_LEN];
char write_buf[MAX_BUFFER_LEN];
int read_buf_start, read_buf_end;
int write_buf_start, write_buf_end;


void	uart_init();
void	uart_flush();
void	uart_send(unsigned int c);
char	uart_getc();
char	uart_getc_raw();
void 	uart_put_hex(unsigned int d);
//unsigned int 	uart_printf(char *fmt, ...);
void uart_printf(char *fmt, ...);
int 	uart_get_int();
//void 	uart_get_string(char *s, int max_length);
unsigned long uart_get_string(char *s, int max_length);
void 	uart_puts_bySize(char *s, int size);
void 	uart_puts(char *s);
void	uart_put_int(unsigned long num);
// interrupt
void 	enable_uart_interrupt();
void 	disable_uart_interrupt();
void 	assert_receive_interrupt();
void 	assert_transmit_interrupt();
void 	uart_handler();
char	uart_async_getc();
void 	uart_async_putc(char c);
void	uart_async_puts(char *str);
void	test_uart_async();

#endif
