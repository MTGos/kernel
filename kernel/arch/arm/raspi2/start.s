.arm
.arch armv6k
.fpu vfpv2
.align 4
.global _start
.extern start
.section .text.boot
_start:
    CPSID aif //Disable interrupts
    //check if current cpu is cpu0
1:
    mrc p15, 0, r0, c0, c0, 5;
    ands r0, #3
    wfine
    bne 1b
    ldr sp,  =svc_stack
    push {r0,r1,r2}
    //set other stacks
    mrs r0, cpsr
    bic r2, r0, #0x1F
    mov r1, r2
    orr r1, #0b10001 //FIQ
    msr cpsr, r1
    ldr sp, =fiq_stack
    mov r1, r2
    orr r1, #0b10010 //IRQ
    msr cpsr, r1
    ldr sp, =irq_stack
    mov r1, r2
    orr r1, #0b10111 //Abort
    msr cpsr, r1
    ldr sp, =abt_stack
    mov r1, r2
    orr r1, #0b11011 //Undefined
    msr cpsr, r1
    ldr sp, =und_stack
    orr r1, #0b11111 //SYS
    msr cpsr, r1
    ldr sp, =kernel_stack
    //Enable FPU
    MRC p15, 0, r0, c1, c1, 2
    ORR r0, r0, #3<<10 // enable fpu
    MCR p15, 0, r0, c1, c1, 2
    LDR r0, =(0xF << 20)
    MCR p15, 0, r0, c1, c0, 2
    MOV r3, #0x40000000 
    VMSR FPEXC, r3
    //Start MTGos
    blx start

.section .bss
.align 16
.space 4096
fiq_stack:
.space 4096
irq_stack:
.space 4096
abt_stack:
.space 4096
und_stack:
.space 4096
svc_stack:
.space 16384
kernel_stack:
