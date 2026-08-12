#include "kstdlib.h"

static int cursor_x = 0;
static int cursor_y = 0;

void putchar_at(int x, int y, char c, unsigned char color)
{
    int pos = (y * VGA_WIDTH + x) * 2;

    VGA_MEMORY[pos] = c;
    VGA_MEMORY[pos + 1] = color;
}

void kprint_at(int x, int y, const char *str, unsigned char color)
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

void putchar(char c, unsigned char color)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        return;
    }

    putchar_at(cursor_x, cursor_y, c, color);

    cursor_x++;

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        cursor_y = 0;
    }
}

void kprint(const char *str, unsigned char color)
{
    while (*str) {
        putchar(*str++, color);
    }
}



void color_test(void)
{
    unsigned char colors[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };

    for (int i = 0; i < 16; i++) {
        putchar_at(
            5 + i * 4,
            5,
            'A' + i,
            colors[i]
        );
    }
}

void progress_bar(int x, int y, int width)
{
    for (int i = 0; i < width; i++) {
        putchar_at(x + i, y, '#', 0x0A);
    }
}

void delay(void)
{
    for (volatile long i = 0; i < 100000000; i++) {
    }
}

void animated_progress(void)
{
    for (int i = 0; i < 40; i++) {
        putchar_at(20 + i, 20, '#', 0x0A);
        delay();
    }
}


unsigned int read_eflags(void)
{
    unsigned int flags;

    __asm__ volatile (
        "pushfl\n"
        "popl %0"
        : "=r"(flags)
    );

    return flags;
}


void kprint_hex(unsigned int value, unsigned char color)
{
    const char *hex = "0123456789ABCDEF";

    kprint("0x", color);

    for (int i = 7; i >= 0; i--) {
        unsigned int digit = (value >> (i * 4)) & 0xF;

        putchar(hex[digit], color);
    }
}

void kprint_dec(unsigned int value, unsigned char color)
{
    char buffer[11];
    int i = 0;

    if (value == 0) {
        putchar('0', color);
        return;
    }

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        putchar(buffer[--i], color);
    }
}

void crash(void)
{
    int *ptr = (int *)0;
    *ptr = 1234;
}
