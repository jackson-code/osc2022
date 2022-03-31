.section ".text"
.global _start
_start:
    mov 	x0, 0
1:
    add 	x0, x0, 1
    svc 	0			// take an exception,  call exception handler in vector_table.S
    cmp 	x0, 5
    blt 	1b
    b		2f
2:
    b		2b
