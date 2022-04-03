#include "uart.h"
#include "address.h"

#define MAILBOX_BASE    (MMIO_BASE + 0xB880)

#define MAILBOX_READ    ((volatile unsigned int*)MAILBOX_BASE)
#define MAILBOX_STATUS  ((volatile unsigned int*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE   ((volatile unsigned int*)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY		0x00010005

#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

int mailbox_call(unsigned int *mailbox, unsigned char ch);

void mailbox_get_board_revision() 
{
  	unsigned int __attribute__((aligned(16))) mailbox[7];
  	/* buffer content (ref: mailbox property interface in github) */
  	mailbox[0] = 7 * 4; // buffer size in bytes
	mailbox[1] = REQUEST_CODE;
	// tags begin
	mailbox[2] = GET_BOARD_REVISION; // tag identifier
	mailbox[3] = 4; // maximum of request and response value buffer's length.
	mailbox[4] = TAG_REQUEST_CODE;
	mailbox[5] = 0; // value buffer
	// tags end
	mailbox[6] = END_TAG;

	// get raspi board revision
	mailbox_call(mailbox, 8); 
	uart_printf("board revision :\t 0x%x", mailbox[5]); // it should be 0xa020d3 for rpi3 b+
}

void mailbox_get_arm_memory() 
{
  	unsigned int __attribute__((aligned(16))) mailbox[7];
  	/* buffer content (ref: mailbox property interface in github) */
  	mailbox[0] = 8 * 4; // buffer size in bytes
	mailbox[1] = REQUEST_CODE;
	// tags begin
	mailbox[2] = GET_ARM_MEMORY; // tag identifier
	mailbox[3] = 8; // maximum of request and response value buffer's length.
	mailbox[4] = TAG_REQUEST_CODE;
	mailbox[5] = 0; // value buffer
	mailbox[6] = 0; // value buffer
	// tags end
	mailbox[7] = END_TAG;

	// get memory base address and size
	mailbox_call(mailbox, 8); 
	uart_printf("memory base address :\t 0x%x", mailbox[5]); 
	uart_printf("memory size :\t 0x%x", mailbox[6]); 
}


/**
 * Returns 0 on failure, non-zero on success
 */
int mailbox_call(unsigned int *mailbox, unsigned char ch)
{
    // 1. Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    //unsigned int r = (((unsigned int)((unsigned long)mailbox) & ~0xF) | (ch & 0xF)); 
	unsigned int r = (unsigned int)(((unsigned long)mailbox) & (~0xF)) | (ch & 0xF);

    /* 2. Check if Mailbox 0 status register’s full flag is set.
   		If not, then you can write to Mailbox 1 Read/Write register. */
    do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRITE = r;

    // wait for the response
    while(1) {
    	/* 3. Check if Mailbox 0 status register’s empty flag is set.
    		If not, then you can read from Mailbox 0 Read/Write register. */
        do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_EMPTY);

        // 4. Check if the value is the same as you wrote in step 1.
        if(r == *MAILBOX_READ) {
            /* is it a valid successful response? */
            return mailbox[1] == REQUEST_SUCCEED;
        }
    }
    return 0;
}

