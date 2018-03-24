#define ASM_FILE 1
#include <config.h>
#include <multiboot.h>
.code32
.section .text.boot
.global _start
_start:
    jmp _start2
.align MULTIBOOT_HEADER_ALIGN
.int MULTIBOOT_HEADER_MAGIC
#ifndef ENABLE_FRAMEBUFFER
.int 0x0
.int -(MULTIBOOT_HEADER_MAGIC)
#else
.int 0x7
.int -(MULTIBOOT_HEADER_MAGIC+0x7)
.int 0,0,0,0,0
.int 0
.int 1024, 768, 24
#endif
.align MULTIBOOT_HEADER_ALIGN
.extern start
_start2:
    mov $mb_ptr, %edi
    stosl
    mov %ebx, %eax
    stosl // Store mb stuff

#ifdef ENABLE_FPU
    finit //Enable FPU
#endif
#ifdef ENABLE_SSE
    //No check for SSE as SSE2+ is always present on x86_64
    mov %cr0, %eax
    and $0xFFFB, %ax
    or $0x2, %ax
    mov %eax, %cr0
    mov %cr4, %eax
    or $3<<9, %ax
    mov %eax, %cr4
#endif
    mov $kernel_stack, %esp
    mov $0x80000001, %eax
    cpuid
    and $0x20000000, %edx //Check if long mode is supported
    jz x86_64_err
    jmp x86_64_OK
x86_64_err:
    cli
    hlt
    jmp x86_64_err
x86_64_OK:
    //Assume PAE is supported
    mov $pmfill, %esi
    mov $pagemapL4, %edi
    movsl
    movsl
    mov $pagedirPT, %edi
    mov $0x87, %eax
    xor %ebx, %ebx
    mov $1023, %ecx
.ptloop:
    stosl
    xchg %eax, %ebx
    stosl
    xchg %eax, %ebx
    add $0x40000000, %eax
    jnc .ptloop_nc
    inc %ebx
.ptloop_nc:
    loop .ptloop
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4 //Activate PAE
    mov $0xC0000080, %ecx
    rdmsr
    or $0x00000100, %eax
    wrmsr //Activate x86_64
    mov $pagemapL4, %eax
    mov %eax, %cr3 //Load page table
    mov %cr0, %eax
    bswap %eax
    or $0x80, %eax
    bswap %eax
    mov %eax, %cr0 //Activate paging
    lgdt gdtr
    mov $0x30, %ax
    ljmp $0x28, $_start3 //Jump into long mode
.code64
_start3:
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss //Load 64-bit data
    mov $0xC0000080, %rax
    rdmsr
    or $(1<<11), %rcx
    wrmsr
    mov $mb_ptr, %rsi
    lodsl
    mov %rax, %rdi
    lodsl
    mov %rax, %rsi
    call start
_stop:
    cli
    hlt
    jmp _stop


.section .data
gdtr:
    .word 9 * 8
    .int gdt
gdt:
    .quad 0 //NULL
    //32-bit kernel code
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x9A
    .byte 0xCF
    .byte 0x00
    //32-bit kernel code
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x92
    .byte 0xCF
    .byte 00
    //32-bit user code
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0xFA
    .byte 0xCF
    .byte 0x00
    //32-bit user data
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0xF2
    .byte 0xCF
    .byte 00
    //64-bit kernel code
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x9B
    .byte 0xAF
    .byte 0x00
    //64-bit kernel code
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x93
    .byte 0xCF
    .byte 00
    //64-bit user code
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0xFB
    .byte 0xAF
    .byte 0x00
    //64-bit user data
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0xF3
    .byte 0xCF
    .byte 00
pmfill:
    .int pagedirPT + 0x7
    .int 0
.section .bss
mb_ptr:
.space 16384
kernel_stack:
    .align 4096
pagemapL4:
    .space 4096
pagedirPT:
    .space 4096
