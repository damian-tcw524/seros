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


/*
 * Global Descriptor Table
 *
 * Entry 0:
 *   Null descriptor
 *
 * Entry 1:
 *   Kernel code
 *   Selector = 0x08
 *
 * Entry 2:
 *   Kernel data
 *   Selector = 0x10
 */

.section .rodata
.align 8

gdt:

    /*
     * Entry 0: NULL
     */
    .quad 0x0000000000000000


    /*
     * Entry 1: Kernel Code
     *
     * Base  = 0
     * Limit = 4 GB
     * Ring  = 0
     * Code  = executable/readable
     * 32-bit
     */
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x9A
    .byte 0xCF
    .byte 0x00


    /*
     * Entry 2: Kernel Data
     *
     * Base  = 0
     * Limit = 4 GB
     * Ring  = 0
     * Read/write
     * 32-bit
     */
    .word 0xFFFF
    .word 0x0000
    .byte 0x00
    .byte 0x92
    .byte 0xCF
    .byte 0x00


gdt_end:


gdt_descriptor:
    .word gdt_end - gdt - 1
    .long gdt


.section .text

.global _start
.extern kernel_main


_start:

    /*
     * Set up our own stack.
     */
    mov $stack_top, %esp


    /*
     * Load our GDT.
     */
    lgdt gdt_descriptor


    /*
     * Reload CS.
     *
     * 0x08 = GDT entry 1 = kernel code.
     */
    ljmp $0x08, $reload_segments


reload_segments:

    /*
     * 0x10 = GDT entry 2 = kernel data.
     */
    mov $0x10, %ax

    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss


    /*
     * Now we're using OUR GDT.
     */

    call kernel_main


hang:

    cli
    hlt
    jmp hang