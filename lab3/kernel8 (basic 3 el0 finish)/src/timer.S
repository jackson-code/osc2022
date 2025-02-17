#define CORE0_TIMER_IRQ_CTRL 0x40000040

.global core_timer_enable
core_timer_enable:    
    mov x0, 1
    msr cntp_ctl_el0, x0 // enable
    mrs x0, cntfrq_el0
    msr cntp_tval_el0, x0 // set expired time
    mov x0, 2
    ldr x1, =CORE0_TIMER_IRQ_CTRL
    str w0, [x1] // unmask timer interrupt
    ret
    
.global el0_timer_handler
el0_timer_handler:
    mrs x0, cntpct_el0
    mrs x0, cntfrq_el0
    add x0, x0, x0       // 2 seconds
    msr cntp_tval_el0, x0
    eret
