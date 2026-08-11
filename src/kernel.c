#define VGA_MEMORY ((volatile unsigned char *)0xB8000)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

void putchar_at(int x, int y, char c, unsigned char color)
{
    int pos = (y * VGA_WIDTH + x) * 2;

    VGA_MEMORY[pos] = c;
    VGA_MEMORY[pos + 1] = color;
}

void print_at(int x, int y, const char *str, unsigned char color)
{
    while (*str) {
        putchar_at(x++, y, *str++, color);
    }
}

void clear_screen(unsigned char color)
{
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            putchar_at(x, y, ' ', color);
        }
    }
}

void kernel_main(void)
{
    clear_screen(0x07);

    print_at(30, 5, "====================", 0x0B);
    print_at(30, 6, "      DAMO OS       ", 0x0B);
    print_at(30, 7, "====================", 0x0B);

    print_at(25, 10, "Kernel booted successfully!", 0x0A);
    print_at(25, 12, "CPU: i386 protected mode", 0x07);
    print_at(25, 13, "VGA: 80x25 text mode", 0x07);
    print_at(25, 14, "Status: ONLINE", 0x0A);

    print_at(25, 17, ">", 0x0E);
    print_at(27, 17, "Welcome, kernel developer.", 0x07);

    while (1) {
        __asm__ volatile ("hlt");
    }
}