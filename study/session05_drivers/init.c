#include <stddef.h>
#include <stdint.h>

#include <vellum/arch/io.h>
#include <vellum/device.h>
#include <vellum/interface/char.h>
#include <vellum/plat/gdt.h>

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

static volatile uint16_t *const vga_text = (volatile uint16_t *)0xB8000;
static int cursor_row = 0;
static int cursor_col = 0;

static void debug_putc(char ch)
{
    VlA_Out8(0x00E9, (uint8_t)ch);
}

static void puts_screen(const char *text, uint8_t attr)
{
    while (*text) {
        if (*text == '\n') {
            cursor_col = 0;
            ++cursor_row;
            debug_putc('\n');
            ++text;
            continue;
        }

        vga_text[cursor_row * 80 + cursor_col] = ((uint16_t)attr << 8) | (uint8_t)*text;
        debug_putc(*text++);
        ++cursor_col;
    }
}

static void run_constructors(void)
{
    void (**ctor)(void);

    for (ctor = &__init_array_start; ctor < &__init_array_end; ++ctor) {
        (*ctor)();
    }
}

void _pc_init(void)
{
    status_t status;
    struct device_driver *drv;
    struct device *dev;
    const struct char_interface *charif;
    struct resource res[] = {
        {
            .type = RT_IOPORT,
            .base = 0x00E9,
            .limit = 0x00E9,
            .flags = 0,
        },
    };
    static const char msg[] = "session05: uart driver write path ok\n";

    _pc_gdt_init();
    run_constructors();

    puts_screen("session05: probing uart_isa\n", 0x0F);

    status = VlDev_FindDriver("uart_isa", &drv);
    if (!CHECK_SUCCESS(status)) {
        puts_screen("driver lookup failed\n", 0x0C);
        for (;;) {
            __asm__ volatile("hlt");
        }
    }

    status = drv->probe(&dev, drv, NULL, res, 1);
    status = CHECK_SUCCESS(status) ? dev->driver->get_interface(dev, "char", (const void **)&charif)
                                   : status;
    if (!CHECK_SUCCESS(status)) {
        puts_screen("probe/get_interface failed\n", 0x0C);
        for (;;) {
            __asm__ volatile("hlt");
        }
    }

    charif->write(dev, msg, sizeof(msg) - 1, NULL);
    puts_screen("device registry and char interface ready\n", 0x0A);

    for (;;) {
        __asm__ volatile("hlt");
    }
}
