#include "uart.h"
#include "convert.h"

int uart_interrupt = 0;

/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

    // UART init 
    *AUX_ENB |=1;          		// enable mini uart(UART1)
    *AUX_MU_CNTL_REG = 0;		// disable tx, rx
    *AUX_MU_IER_REG = 0;		// disable interrupt
    *AUX_MU_LCR_REG = 3;    	// the UART works in 8-bit mode
    *AUX_MU_MCR_REG = 0;		// Don’t need auto flow control
    *AUX_MU_BAUD_REG = 270;    	// 115200 baud rate
    *AUX_MU_IIR_REG = 6;		// no FIFO(buffer)


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

	if (uart_interrupt)
	{
		*AUX_MU_IER_REG = 1;   // enable receive interrupts
	} else {
    	*AUX_MU_IIR_REG = 0xc6;   // disable interrupts
	}
	
	//#endif

	*AUX_MU_CNTL_REG = 3;  // enable transmitter and receiver back
	
	if (uart_interrupt)
	{
		// asynch read/write
		read_buf_start = read_buf_end = 0;
		write_buf_start = write_buf_end = 0;
		enable_uart_interrupt();
	}

	//disable_uart_interrupt();

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

/**
 * Receive a character
 */
char uart_getc_cannot_type() {
    char r;
	int delay = 100;
    /* wait until something is in the buffer */
    do { delay--; } while ( !(*AUX_MU_LSR_REG & 0x01) && delay > 0);
	if (delay > 0)
	{
		/* read it and return */
    	r = (char)(*AUX_MU_IO_REG);
	} else
	{
		return '\0';
	}
	

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
void uart_put_hex(unsigned int d)
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
    //uart_send('\r');
    //uart_send('\n');
}


void uart_puts(char *s){
    while(*s){
    	if(*s == '\n') uart_send('\r');
		uart_send(*s++);
    }
}

/*
unsigned int uart_printf(char* fmt,...){
	char dst[100];
    //__builtin_va_start(args, fmt): "..." is pointed by args
    //__builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args,fmt);
    unsigned int ret=vsprintf(dst,fmt,args);
    uart_puts(dst);
    return ret;
}
*/

/*
// my printf, error: make kernel interrupt??
void uart_printf(char *fmt, ...)
{
	__builtin_va_list ap;			// pointer to arg
	__builtin_va_start(ap, fmt);	// ap point to the first nameless arg
	
	char *c, *sval;
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
				uart_put_hex(uival);
				break;
			default:
				break;
		}
	}
	__builtin_va_end(ap);
	uart_send('\n');
	uart_send('\r');
}
*/

void uart_printf(char *fmt, ...)
{
	//uart_puts("uart_printf");
	//*AUX_MU_IIR_REG = 0xc6;   // disable interrupts

	__builtin_va_list ap;			// pointer to arg
	__builtin_va_start(ap, fmt);	// ap point to the first nameless arg
	
	char *c, *sval;
	int ival;
	unsigned int uival;
	float fval;
	
	for (c = fmt; *c; c++) {
		// print char except printf conversions
		if (*c != '%') {
			uart_async_putc(*c);
			continue;
		}
		// TODO: print printf conversions
		switch (*++c) {
			case 'd':
				ival = __builtin_va_arg(ap, int);
                char buf_int[20];
                char *p1 = itoa(ival, buf_int);
                while (*p1)
                    uart_async_putc(*p1++);	
				break;
			case 'f':
				fval = (float)__builtin_va_arg(ap, double);
                char buf_double[19];  // sign + 10 int + dot + 6 float
                char *p2 = ftoa(fval, buf_double);
                while (*p2)
                    uart_async_putc(*p2++);
				break;
			case 's':
				for (sval = __builtin_va_arg(ap, char *); *sval; sval++)	
					uart_async_putc(*sval);	
				break;
			case 'x':
				uival = __builtin_va_arg(ap, unsigned int);	
				//uart_put_hex(uival);
				break;
			default:
				break;
		}
	}
	__builtin_va_end(ap);
	//uart_async_putc('\n');
	//uart_async_putc('\r');
	//*AUX_MU_IER_REG = 1;   // enable receive interrupts
}

/*
uart recive char, then convert to int
*/
int uart_get_int(){
    int res = 0;
    char c;
    while(1){
        c = uart_getc();
        if(c == '\0' || c == '\n')
            break;
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


unsigned long uart_get_string_error(char *s, int max_length){
	char c;
	int i = 1;	// 1 for '\0'
    while(1){
    	i++;	// string length at least equal to 1, because of '\0' 
        c = uart_getc();
        if	((*s = c) == '\n' || *s == '\r') {
	        uart_send('\n');
	        uart_send('\r');
			break;
		}
        if (i >= max_length) {
        	uart_puts("Exceed string max length");
			uart_put_int(max_length);
			break;;
        }
        uart_send(c);
        s++;
    }
	s++;
	*s = '\0';
	return i;
}

unsigned long uart_get_string_block_error(char *s, int max_length){
	char c;
	int i = 0;
    while(1){
    	i++;
        c = uart_getc();
		*s = c;
        s++;
		if (i >= max_length) {
			break;
        }
    }
	return i;
}

unsigned long uart_get_string_error_unknown(char *s, int max_length){
	char c;
	int i = 0;
    while((c = uart_getc())){
    	i++;
		*s = c;
        s++;
		if (i >= max_length) {
			break;
        }
    }
	return i;
}


unsigned long uart_get_string(char *s, int max_length){
	char c;
	int i = 0;
    while(i < max_length){
		*s = uart_getc_raw();
        s++;
		i++;
    }
	return i;
}

void uart_puts_bySize(char *s, int size){
    for(int i = 0; i < size ;++i){
        if(*s == '\n') uart_send('\r');
        uart_send(*s++);
    }
}



/******************************************/
/*                  irq                   */
/******************************************/

void enable_uart_interrupt() { *ENB_IRQS1 = AUX_IRQ; }			// enable uart interrupt
void disable_uart_interrupt() { *DISABLE_IRQS1 = AUX_IRQ; }		// disable uart interrupt, all interrupts remain asserted until disabled or the interrupt source is cleared.

void assert_receive_interrupt() { *AUX_MU_IER_REG |= 0x1; }		// If this bit is set the interrupt line is asserted whenever the receive FIFO holds at least 1 byte
void clear_receive_interrupt() { *AUX_MU_IER_REG &= ~(0x1); }	// If this bit is clear no receive interrupts are generated

void assert_transmit_interrupt() { *AUX_MU_IER_REG |= 0x2; }	// If this bit is set the interrupt line is asserted whenever the transmit FIFO is empty.
void clear_transmit_interrupt() { *AUX_MU_IER_REG &= ~(0x2); }	// If this bit is clear no transmit interrupts are generated

void uart_handler()
{
	disable_uart_interrupt();
	
	//uart_puts("(uart handler)");		// infinite uart interrupt, why???
	
	// decide interrupt issued by rx/tx (p13)
	int rx = (*AUX_MU_IIR_REG & 0x4);			// Receiver holds valid byte		(condition of rx issued interrupt)
	int tx = (*AUX_MU_IIR_REG & 0x2);			// Transmit holding register empty	(condition of tx issued interrupt)
	int no_irq = (*AUX_MU_IIR_REG & 0x6);
	
	if (rx) {
		//uart_puts("(rx)");
    	read_buf[read_buf_end++] = (char)(*AUX_MU_IO_REG);
    	//uart_async_puts(read_buf + read_buf_end - 1);		// show what you type immediately
    	
    	if (read_buf_end == MAX_BUFFER_LEN) 
    		read_buf_end = 0;
	}
	else if (tx) {
	    while (*AUX_MU_LSR_REG & 0x20) {				// the transmit FIFO can accept at least one byte
			if (write_buf_start == write_buf_end) {
				clear_transmit_interrupt();				// no stuff to write, disable transmit interrupts
				break;
			}
			//uart_puts("(tx)");
			*AUX_MU_IO_REG = write_buf[write_buf_start++];
			if (write_buf_start == MAX_BUFFER_LEN) 
				write_buf_start = 0;
		}
	}
	else if(no_irq) {
	}
	enable_uart_interrupt();
}


char uart_async_getc() {
	// wait until there are new data
	while (read_buf_start == read_buf_end) {
		asm volatile("nop");
	}
	char c = read_buf[read_buf_start++];
	if (read_buf_start == MAX_BUFFER_LEN) read_buf_start = 0;
	// '\r' => '\n'
	return c == '\r' ? '\n' : c;
}

void uart_async_putc(char c) {
	/*if (c == '\r')
		write_buf[write_buf_end++] = '\n';
	else
	*/	
		write_buf[write_buf_end++] = c;
	
	if (write_buf_end == MAX_BUFFER_LEN)
		write_buf_end = 0;
	//uart_puts(write_buf);
	assert_transmit_interrupt();
}

void uart_async_puts(char *str) {
	for (int i = 0; str[i]; i++) {
		if (str[i] == '\r')
			write_buf[write_buf_end++] = '\n';
			
		write_buf[write_buf_end++] = str[i];
		
		if (write_buf_end == MAX_BUFFER_LEN)
			write_buf_end = 0;
	}
	//uart_puts(write_buf);
	assert_transmit_interrupt();
}

void test_uart_async(){
	/*  need time */
    int reg=1500;
    while ( reg-- )
    { 
        asm volatile("nop"); 
    }

	uart_async_puts("test test");
}