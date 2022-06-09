#include "uart.h"
#include "mailbox.h"

/**
 * Returns 0 on failure, non-zero on success
 */
int mailbox_call(unsigned char ch, unsigned int *mailbox)
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
	mailbox_call(MBOX_CH_PROP, mailbox); 
	uart_puts("\nboard revision :\t 0x"); // it should be 0xa020d3 for rpi3 b+
	uart_put_hex(mailbox[5]);
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
	mailbox_call(MBOX_CH_PROP, mailbox); 
	uart_puts("\nmemory base address :\t 0x"); 
	uart_put_hex(mailbox[5]);
	uart_puts("\nmemory size :\t 0x"); 
	uart_put_hex(mailbox[6]);
}


void mailbox_init_framebuffer()
{
	unsigned int __attribute__((aligned(16))) mbox[36];
	unsigned int width, height, pitch, isrgb; /* dimensions and channel order */
	unsigned char *lfb;                       /* raw frame buffer address */

	// width = 0;
	// height = 0;
	// pitch = 0;
	// isrgb = 0;
	// lfb = 0;

	mbox[0] = 35 * 4;
	mbox[1] = REQUEST_CODE;

	mbox[2] = 0x48003; // set phy wh
	mbox[3] = 8;
	mbox[4] = 8;
	mbox[5] = 1024; // FrameBufferInfo.width
	mbox[6] = 768;  // FrameBufferInfo.height

	mbox[7] = 0x48004; // set virt wh
	mbox[8] = 8;
	mbox[9] = 8;
	mbox[10] = 1024; // FrameBufferInfo.virtual_width
	mbox[11] = 768;  // FrameBufferInfo.virtual_height

	mbox[12] = 0x48009; // set virt offset
	mbox[13] = 8;
	mbox[14] = 8;
	mbox[15] = 0; // FrameBufferInfo.x_offset
	mbox[16] = 0; // FrameBufferInfo.y.offset

	mbox[17] = 0x48005; // set depth
	mbox[18] = 4;
	mbox[19] = 4;
	mbox[20] = 32; // FrameBufferInfo.depth

	mbox[21] = 0x48006; // set pixel order
	mbox[22] = 4;
	mbox[23] = 4;
	mbox[24] = 1; // RGB, not BGR preferably

	mbox[25] = 0x40001; // get framebuffer, gets alignment on request
	mbox[26] = 8;
	mbox[27] = 8;
	mbox[28] = 4096; // FrameBufferInfo.pointer
	mbox[29] = 0;    // FrameBufferInfo.size

	mbox[30] = 0x40008; // get pitch
	mbox[31] = 4;
	mbox[32] = 4;
	mbox[33] = 0; // FrameBufferInfo.pitch

	mbox[34] = END_TAG;

	// this might not return exactly what we asked for, could be
	// the closest supported resolution instead
	if (mailbox_call(MBOX_CH_PROP, mbox) && mbox[20] == 32 && mbox[28] != 0) {
		mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
		width = mbox[5];        // get actual physical width
		height = mbox[6];       // get actual physical height
		pitch = mbox[33];       // get number of bytes per line
		isrgb = mbox[24];       // get the actual channel order
		lfb = (void *)((unsigned long)mbox[28]);
	} else {
		uart_puts("Unable to set screen resolution to 1024x768x32\n");
	}
}




