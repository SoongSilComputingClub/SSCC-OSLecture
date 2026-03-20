#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    char buf[16];
    unsigned int len_all;
    unsigned int len_prefix;
    const uint8_t title_attr = 0x0F;
    const uint8_t body_attr = 0x0A;

    _pc_gdt_init();

    memset(buf, 0, sizeof(buf));
    memcpy(buf, "HELLO", 6);
    memmove(buf + 1, buf, 5);
    len_all = (unsigned int)strnlen(buf, sizeof(buf));
    len_prefix = (unsigned int)strnlen(buf, 3);

    cprintf(write_screen, (void *)&title_attr, "session07: libc formatter ready\n");
    cprintf(
        write_screen,
        (void *)&body_attr,
        "buf=%s len=%u prefix=%u hex=%08X\n",
        buf,
        len_all,
        len_prefix,
        0x1234ABCDu
    );

    for (;;) {
        __asm__ volatile("hlt");
    }
}
