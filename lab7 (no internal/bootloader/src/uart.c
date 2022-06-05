/* ref: BCM2837-ARM-Peripherals */
#include "gpio.h"
#include "address.h"
#include "convert.h"

// mini UART registers
// ch 2.2.2
#define AUX_ENB 				((volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO_REG			((volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER_REG			((volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR_REG			((volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR_REG  		((volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR_REG  		((volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR_REG			((volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR_REG			((volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH_REG  	((volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL_REG			((volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT_REG			((volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD_REG			((volatile unsigned int*)(MMIO_BASE+0x00215068))

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    // UART init 
    *AUX_ENB |=1;          	// enable mini uart(UART1)
    *AUX_MU_CNTL_REG = 0;	// disable tx, rx
    *AUX_MU_IER_REG = 0;	// disable interrupt
    *AUX_MU_LCR_REG = 3;       	// the UART works in 8-bit mode
    *AUX_MU_MCR_REG = 0;	// Don’t need auto flow control
    //*AUX_MU_IIR_REG = 0xc6;    	// disable interrupts
    *AUX_MU_BAUD_REG = 270;    	// 115200 baud rate
    *AUX_MU_IIR_REG = 6;	// no FIFO


    // configure GPFSELn register to change alternate function(UART1)
    r = *GPFSEL1;
    r &= ~((7<<12) | (7<<15)); // select gpio14, gpio15, and reset to 0 (ref table 6-3)
    r |= (2<<12) | (2<<15);    // 2(010) means choose alt5(ref 6.2)
    *GPFSEL1 = r;

    // ref: SYNOPSIS of GPPUDCLKn
    // enable pins 14 and 15 
    *GPPUD = 0;            
    r = 150;
    while (r--) { asm volatile("nop"); }		//  Inline Assembly Language in C, nop: do nothing, Wait 150 cycles

    *GPPUDCLK0 = (1<<14) | (1<<15);
    r = 150; 
    while (r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;        	// flush GPIO setup
    
    *AUX_MU_CNTL_REG = 3;     	// enable tx, rx
}

void uart_flush()
{
    while (*AUX_MU_LSR_REG & 0x01)
    	*AUX_MU_IO_REG;
}

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do { asm volatile("nop"); } while ( !(*AUX_MU_LSR_REG & 0x20) );	// see 5th bit
    /* write the character to the buffer */
    *AUX_MU_IO_REG = c;
}

/**
 * Receive a character
 */
char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do { asm volatile("nop"); } while ( !(*AUX_MU_LSR_REG & 0x01) );
    /* read it and return */
    r = (char)(*AUX_MU_IO_REG);
    /* convert carrige return to newline */
    return r == '\r' ? '\n' : r;
    //return r;
}

char uart_getc_raw() {
    char r;
    /* wait until something is in the buffer */
    do { asm volatile("nop"); } while ( !(*AUX_MU_LSR_REG & 0x01) );
    /* read it and return */
    r = (char)(*AUX_MU_IO_REG);
    /* convert carrige return to newline */
    return r;
}

/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d)
{
    unsigned int n;
    int c;
    for(c = 28; c >= 0; c -= 4) {
        // get highest tetrad
        n = (d>>c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x37 : 0x30;
        uart_send(n);
    }
}


void uart_printf(char *fmt, ...)
{
	__builtin_va_list ap;			// pointer to arg
	__builtin_va_start(ap, fmt);	// ap point to the first nameless arg
	
	char *c, *sval;
	int cval;
	int ival;
	unsigned int uival;
	float fval;
	
	for (c = fmt; *c; c++) {
		// print char except printf conversions
		if (*c != '%') {
			uart_send(*c);
			continue;
		}
		// TODO: print printf conversions
		switch (*++c) {
			case 'd':
				ival = __builtin_va_arg(ap, int);
                char buf_int[20];
                char *p1 = itoa(ival, buf_int);
                while (*p1)
                    uart_send(*p1++);	
				break;
			case 'u':
				ival = __builtin_va_arg(ap, unsigned int);
				uart_send(uival);
				break;
			case 'f':
				fval = (float)__builtin_va_arg(ap, double);
                char buf_double[19];  // sign + 10 int + dot + 6 float
                char *p2 = ftoa(fval, buf_double);
                while (*p2)
                    uart_send(*p2++);
				break;
			case 's':
				for (sval = __builtin_va_arg(ap, char *); *sval; sval++)	
					uart_send(*sval);	
				break;
			case 'x':
				uival = __builtin_va_arg(ap, unsigned int);	
				uart_hex(uival);
				break;
			case 'c':
				cval = __builtin_va_arg(ap, int);	
				uart_send(cval);
				break;
			default:
				break;
		}
	}
	__builtin_va_end(ap);
	uart_send('\n');
	uart_send('\r');
}


// uart recive char, then convert to int
// return '\0' if failed
int uart_get_int(){
    int res = 0;
    char c;
    while(1){
        c = uart_getc();
        if (c == '\n' || c == '\r')
        	break;                
        if (c < '0' || c > '9') {
        	uart_printf("ERROR in uart_get_int(): not a number ascii code %c", c);
        	return '\0';
        }
        uart_send(c);
        res = res * 10 + (c - '0');
    }
    return res;
}

void uart_put_int(unsigned long num){
    if(num == 0) uart_send('0');
    else{
        if(num > 10) uart_put_int(num / 10);
        uart_send(num % 10 + '0');
    }
}


void uart_get_string(char *s, int max_length){
	char c;
	int i = 0;
    while(1){
    	i++;	// string length at least equal to 1, because of '\0' 
        c = uart_getc();
        if	((*s = c) == '\n' || *s == '\r')
            break;
        if (i >= max_length) {
        	uart_printf("Exceed string max length %d", max_length);
        	break;
        }
        uart_send(c);
        s++;
    }
    *s = '\0';
}


void uart_puts_bySize(char *s, int size){
    for(int i = 0; i < size ;++i){
        if(*s == '\n') uart_send('\r');
        uart_send(*s++);
    }
}






