.section .text

.global isr0
.global isr1
.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

.extern divide_error_handler
.extern debug_handler
.extern interrupt_handler


/*
 * Common interrupt entry point.
 *
 * The CPU automatically pushes the return address, code segment, and flags
 * before jumping here. We then push the CPU registers onto the stack so a
 * C function can inspect the saved state and handle the interrupt.
 */
common_interrupt:
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %esp, %eax
    push %eax
    call interrupt_handler
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    add $8, %esp
    iret


/*
 * ISR 0
 *
 * CPU exception:
 * Divide Error (#DE)
 *
 * Interrupt vector: 0
 */
isr0:
    cli
    push $0
    push $0
    jmp common_interrupt


/*
 * ISR 1
 *
 * CPU exception:
 * Debug (#DB)
 *
 * Interrupt vector: 1
 */
isr1:
    cli
    push $0
    push $1
    jmp common_interrupt


/*
 * IRQ handlers.
 *
 * Hardware interrupts come from the PIC, not the CPU. We remap them into
 * the 32..47 range so they don't collide with the CPU exceptions.
 */
.macro IRQ_HANDLER irq_num
irq\irq_num:
    cli
    push $0
    push $(32 + \irq_num)
    jmp common_interrupt
.endm

IRQ_HANDLER 0
IRQ_HANDLER 1
IRQ_HANDLER 2
IRQ_HANDLER 3
IRQ_HANDLER 4
IRQ_HANDLER 5
IRQ_HANDLER 6
IRQ_HANDLER 7
IRQ_HANDLER 8
IRQ_HANDLER 9
IRQ_HANDLER 10
IRQ_HANDLER 11
IRQ_HANDLER 12
IRQ_HANDLER 13
IRQ_HANDLER 14
IRQ_HANDLER 15