#include <stdint.h>

#include <vellum/arch/io.h>
#include <vellum/plat/gdt.h>

static volatile uint16_t *const vga_text = (volatile uint16_t *)0xB8000;
static int cursor = 0;

static void debug_putc(char ch)
{
    VlA_Out8(0x00E9, (uint8_t)ch);
}

static void puts_debug(const char *text)
{
    while (*text) {
        debug_putc(*text++);
    }
}

void _pc_init(void)
{
    const char *msg = "session06: BIOS video helpers live in stage1\n";

    _pc_gdt_init();

    while (*msg) {
        char ch = *msg++;

        if (ch == '\n') {
            cursor = ((cursor / 80) + 1) * 80;
            debug_putc('\n');
            continue;
        }

        vga_text[cursor++] = ((uint16_t)0x0F << 8) | (uint8_t)ch;
        debug_putc(ch);
    }

    for (;;) {
        __asm__ volatile("hlt");
    }
}
