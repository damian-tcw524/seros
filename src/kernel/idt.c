#include "idt.h"
#include "kstdlib.h"


struct idt_entry {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  type_attr;
    unsigned short offset_high;
} __attribute__((packed));


struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));


/*
 * The CPU exception handlers are mapped to vectors 0..31.
 * Hardware interrupts from the Programmable Interrupt Controller are
 * mapped to vectors 32..47.
 *
 * That is why the timer and keyboard IRQs live in the 0x20..0x2F range:
 * they are not CPU exceptions, they are external hardware signals that
 * have been routed through the PIC.
 */
static struct idt_entry idt[256];
static struct idt_ptr idt_descriptor;

static int keyboard_x = 0;
static int keyboard_y = 0;


/*
 * ISR functions defined in isr.S.
 *
 * The CPU will jump into these handlers when a specific interrupt vector
 * is triggered. The assembly stubs do the low-level register save/restore.
 */
extern void isr0(void);
extern void isr1(void);
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);


/*
 * This matches the stack layout produced by the assembler interrupt stub.
 *
 * After `pusha`, we push the segment registers, then CPU interrupt number,
 * error code, and the return address/flags. The interrupt handler can use
 * this structure to know which interrupt triggered and which CPU state was
 * saved.
 */
struct interrupt_frame {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no;
    unsigned int err_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int useresp;
    unsigned int ss;
} __attribute__((packed));


static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}


static inline unsigned char inb(unsigned short port)
{
    unsigned char value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}


/*
 * Load IDT into the CPU.
 */
static void idt_load(void)
{
    __asm__ volatile (
        "lidt (%0)"
        :
        : "r" (&idt_descriptor)
    );
}


/*
 * This is the classic PIC remap.
 *
 * The 8259 PIC starts at IRQ 0..15 by default, which conflicts with CPU
 * exceptions 0..31. We move the IRQs to 32..47 so the CPU can distinguish
 * between internal traps and external device interrupts.
 */
static void pic_init(void)
{
    /*
     * Start the master and slave PICs in initialization mode.
     */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    /*
     * Remap master PIC to 0x20..0x27 and slave PIC to 0x28..0x2F.
     */
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    /*
     * Tell the master that the slave is connected to IRQ2.
     */
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    /*
     * Set 8086 mode and enable both PICs.
     */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    /*
     * Mask all IRQs except keyboard IRQ1.
     *
     * Bit 1 is cleared, which means IRQ1 is unmasked and enabled.
     * All other bits are set to 1, so they stay masked off.
     */
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
}


static void pic_send_eoi(int irq)
{
    /*
     * When the PIC receives an interrupt, the CPU must acknowledge it.
     * This is called End Of Interrupt (EOI). Without it, the same IRQ can
     * fire again and again and the keyboard would stop working correctly.
     */
    if (irq >= 8) {
        outb(0xA0, 0x20);
    }

    outb(0x20, 0x20);
}


/*
 * Divide Error handler.
 *
 * CPU exception #0.
 */
void divide_error_handler(void)
{
    kprint_at(
        20,
        18,
        "!!! DIVIDE ERROR (#DE) !!!",
        0x0C
    );
}


/*
 * Debug exception handler.
 *
 * CPU exception #1.
 */
void debug_handler(void)
{
    kprint_at(
        20,
        19,
        "!!! DEBUG EXCEPTION (#DB) !!!",
        0x0B
    );
}


static unsigned char scancode_to_ascii(unsigned char scancode)
{
    /*
     * A scan code is the raw key identifier from the keyboard controller.
     * We convert it into ASCII for a simple terminal-style echo.
     *
     * This table covers the common keys you'll use while learning.
     */
    static const char map[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0
    };

    if (scancode >= 0x80) {
        return 0;
    }

    return map[scancode];
}


static void keyboard_handler(void)
{
    /*
     * IRQ1 is the keyboard interrupt.
     * The keyboard controller stores the scan code in port 0x60.
     * We read it, convert it to ASCII, and then print it on the screen.
     */
    unsigned char scancode = inb(0x60);
    unsigned char c = scancode_to_ascii(scancode);

    if (c != 0) {
        char buf[2];

        buf[0] = c;
        buf[1] = '\0';

        kprint_at(2 + keyboard_x, 22 + keyboard_y, buf, 0x0E);

        keyboard_x++;
        if (keyboard_x >= 70) {
            keyboard_x = 0;
            keyboard_y++;
        }
    }

    /*
     * Tell the PIC that we handled the interrupt.
     */
    pic_send_eoi(1);
}


/*
 * This is the common interrupt entry point called from the assembly stubs.
 *
 * The CPU pushes the current execution state, then jumps here. We inspect
 * the interrupt number and dispatch to the right C handler.
 */
void interrupt_handler(struct interrupt_frame *frame)
{
    switch (frame->int_no) {
    case 0:
        divide_error_handler();
        break;

    case 1:
        debug_handler();
        break;

    case 33:
        keyboard_handler();
        break;

    default:
        break;
    }

    if (frame->int_no >= 32) {
        pic_send_eoi(frame->int_no - 32);
    }
}


/*
 * Create an IDT gate.
 */
static void idt_set_gate(
    int number,
    unsigned int handler
)
{
    idt[number].offset_low =
        handler & 0xFFFF;

    /*
     * 0x08 = kernel code segment.
     */
    idt[number].selector = 0x08;

    idt[number].zero = 0;

    /*
     * 0x8E:
     *
     * Present
     * Ring 0
     * 32-bit interrupt gate
     */
    idt[number].type_attr = 0x8E;

    idt[number].offset_high =
        (handler >> 16) & 0xFFFF;
}


/*
 * Initialize the IDT.
 */
void idt_init(void)
{
    /*
     * Clear all 256 entries.
     */
    for (int i = 0; i < 256; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }

    /*
     * CPU exception vectors.
     */
    idt_set_gate(0, (unsigned int)isr0);
    idt_set_gate(1, (unsigned int)isr1);

    /*
     * Hardware interrupt vectors.
     *
     * IRQ0 timer -> vector 32
     * IRQ1 keyboard -> vector 33
     * ...
     * IRQ15 -> vector 47
     */
    idt_set_gate(32, (unsigned int)irq0);
    idt_set_gate(33, (unsigned int)irq1);
    idt_set_gate(34, (unsigned int)irq2);
    idt_set_gate(35, (unsigned int)irq3);
    idt_set_gate(36, (unsigned int)irq4);
    idt_set_gate(37, (unsigned int)irq5);
    idt_set_gate(38, (unsigned int)irq6);
    idt_set_gate(39, (unsigned int)irq7);
    idt_set_gate(40, (unsigned int)irq8);
    idt_set_gate(41, (unsigned int)irq9);
    idt_set_gate(42, (unsigned int)irq10);
    idt_set_gate(43, (unsigned int)irq11);
    idt_set_gate(44, (unsigned int)irq12);
    idt_set_gate(45, (unsigned int)irq13);
    idt_set_gate(46, (unsigned int)irq14);
    idt_set_gate(47, (unsigned int)irq15);

    /*
     * Configure IDT descriptor.
     */
    idt_descriptor.limit = sizeof(idt) - 1;
    idt_descriptor.base = (unsigned int)&idt;

    /*
     * Initialize the Programmable Interrupt Controller.
     */
    pic_init();

    /*
     * Load the IDT into the processor.
     */
    idt_load();
}