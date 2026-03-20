#include <stdint.h>

#include <vellum/arch/io.h>
#include <vellum/plat/gdt.h>

static volatile uint16_t *const vga_text = (volatile uint16_t *)0xB8000;

static void debug_putc(char ch)
{
    VlA_Out8(0x00E9, (uint8_t)ch);
}

static void write_line(const char *text, uint8_t attr, int row)
{
    volatile uint16_t *cursor = vga_text + row * 80;

    while (*text) {
        *cursor++ = ((uint16_t)attr << 8) | (uint8_t)*text;
        debug_putc(*text++);
    }

    debug_putc('\n');
}

void _pc_init(void)
{
    _pc_gdt_init();

    write_line("session03: protected mode ok", 0x0F, 0);
    write_line("session03: stage1 loaded VELLUM.X86", 0x0A, 1);

    for (;;) {
        __asm__ volatile("hlt");
    }
}
