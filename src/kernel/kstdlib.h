#ifndef KSTDLIB_H
#define KSTDLIB_H

#define VGA_MEMORY ((volatile unsigned char *)0xB8000)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

void kprint_at(int x, int y, const char *str, unsigned char color);
void kprint(const char *str, unsigned char color);
void clear_screen(unsigned char color);
void delay(void);


#endif