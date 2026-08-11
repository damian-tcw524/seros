.section .multiboot
.align 4

.long 0x1BADB002
.long 0
.long -(0x1BADB002)

.section .bss
.align 16

stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.extern kernel_main

_start:
    mov $stack_top, %esp

    call kernel_main

hang:
    cli
    hlt
    jmp hang