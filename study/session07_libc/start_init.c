#include <stdint.h>
#include <stdio.h>

#include <vellum/arch/io.h>
#include <vellum/plat/gdt.h>

static volatile uint16_t *const vga_text = (volatile uint16_t *)0xB8000;
static int cursor = 0;

static int write_screen(void *ctx, char ch)
{
    uint8_t attr = *(const uint8_t *)ctx;

    if (ch == '\0') {
        return 0;
    }

    if (ch == '\n') {
        cursor = ((cursor / 80) + 1) * 80;
        VlA_Out8(0x00E9, '\n');
        return 0;
    }

    vga_text[cursor++] = ((uint16_t)attr << 8) | (uint8_t)ch;
    VlA_Out8(0x00E9, (uint8_t)ch);
    return 0;
}

void _pc_init(void)
{
    const uint8_t title_attr = 0x0F;
    const uint8_t body_attr = 0x0E;

    _pc_gdt_init();

    cprintf(write_screen, (void *)&title_attr, "session07: libc primitives and vcprintf\n");
    cprintf(
        write_screen,
        (void *)&body_attr,
        "format demo -> len=%d hex=%08X name=%s\n",
        5,
        0x1234ABCDu,
        "student"
    );

    for (;;) {
        __asm__ volatile("hlt");
    }
}
