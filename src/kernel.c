
void kernel_main(void)
{
    clear_screen(0x07);

    kprint_at(30, 5,
             "====================",
             0x0B);

    kprint_at(30, 6,
             "        SEROS       ",
             0x0B);

    kprint_at(30, 7,
             "====================",
             0x0B);

    kprint_at(18, 10,
             "Initializing IDT and PIC...",
             0x07);

    /*
     * The interrupt descriptor table tells the CPU where to jump when an
     * exception or IRQ happens.
     *
     * The PIC (Programmable Interrupt Controller) is the chip that routes
     * hardware events like timer ticks and keyboard presses into those IRQ
     * vectors.
     */
    idt_init();

    __asm__ volatile ("sti");

    kprint_at(18, 11,
             "Interrupts enabled. Press keys on the keyboard.",
             0x0A);

    kprint_at(2, 22,
             ">",
             0x0E);

    while (1) {
        __asm__ volatile ("hlt");
    }
}