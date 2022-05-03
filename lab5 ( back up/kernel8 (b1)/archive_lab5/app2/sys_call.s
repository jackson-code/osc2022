	.arch armv8-a
	.file	"sys_call.c"
	.text
	.align	2
	.global	uart_printf
	.type	uart_printf, %function
uart_printf:
.LFB0:
	.cfi_startproc
	stp	x29, x30, [sp, -400]!
	.cfi_def_cfa_offset 400
	.cfi_offset 29, -400
	.cfi_offset 30, -392
	mov	x29, sp
	str	x0, [sp, 56]
	str	x1, [sp, 344]
	str	x2, [sp, 352]
	str	x3, [sp, 360]
	str	x4, [sp, 368]
	str	x5, [sp, 376]
	str	x6, [sp, 384]
	str	x7, [sp, 392]
	str	q0, [sp, 208]
	str	q1, [sp, 224]
	str	q2, [sp, 240]
	str	q3, [sp, 256]
	str	q4, [sp, 272]
	str	q5, [sp, 288]
	str	q6, [sp, 304]
	str	q7, [sp, 320]
	add	x0, sp, 400
	str	x0, [sp, 72]
	add	x0, sp, 400
	str	x0, [sp, 80]
	add	x0, sp, 336
	str	x0, [sp, 88]
	mov	w0, -56
	str	w0, [sp, 96]
	mov	w0, -128
	str	w0, [sp, 100]
	add	x2, sp, 16
	add	x3, sp, 72
	ldp	x0, x1, [x3]
	stp	x0, x1, [x2]
	ldp	x0, x1, [x3, 16]
	stp	x0, x1, [x2, 16]
	add	x1, sp, 16
	add	x0, sp, 104
	mov	x2, x1
	ldr	x1, [sp, 56]
	bl	vsprintf
	str	w0, [sp, 204]
	ldr	w1, [sp, 204]
	add	x0, sp, 104
	bl	uartwrite
	ldr	w0, [sp, 204]
	ldp	x29, x30, [sp], 400
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE0:
	.size	uart_printf, .-uart_printf
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
