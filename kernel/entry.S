	# qemu -kernel loads the kernel at 0x80000000
        # and causes each CPU to jump there.
        # kernel.ld causes the following code to
        # be placed at 0x80000000.
.section .text
.global _entry
_entry:
	# set up a stack for C.
        # stack0 is declared in start.c,
        # with a 4096-byte stack per CPU.
        # sp = stack0 + (hartid * 4096)
        la sp, stack0
        li a0, 1024*4
	csrr a1, mhartid     # 例如mhartid=0(CPU0),则 a1 = 0
        addi a1, a1, 1       # a1 = a1 + 1, a1 = 1
        mul a0, a0, a1       # a0 = a0 * a1 = 4096 * 1 = 4096
        add sp, sp, a0       # sp = sp + a0 = stack0 + 4096
	# jump to start() in start.c
        call start
spin:
        j spin
