#include <stdint.h>

#include <vellum/arch/io.h>
#include <vellum/mm.h>
#include <vellum/plat/gdt.h>

static volatile uint16_t *const vga_text = (volatile uint16_t *)0xB8000;
static int cursor_row = 0;
static int cursor_col = 0;

static void debug_putc(char ch)
{
    VlA_Out8(0x00E9, (uint8_t)ch);
}

static void putc_screen(char ch, uint8_t attr)
{
    if (ch == '\n') {
        cursor_col = 0;
        ++cursor_row;
        debug_putc('\n');
        return;
    }

    vga_text[cursor_row * 80 + cursor_col] = ((uint16_t)attr << 8) | (uint8_t)ch;
    debug_putc(ch);
    ++cursor_col;

    if (cursor_col >= 80) {
        cursor_col = 0;
        ++cursor_row;
        debug_putc('\n');
    }
}

static void puts_screen(const char *text, uint8_t attr)
{
    while (*text) {
        putc_screen(*text++, attr);
    }
}

static void put_hex32(uint32_t value, uint8_t attr)
{
    static const char hex[] = "0123456789ABCDEF";
    int shift;

    for (shift = 28; shift >= 0; shift -= 4) {
        putc_screen(hex[(value >> shift) & 0xF], attr);
    }
}

void _pc_init(void)
{
    status_t status;
    size_t free_frames = 0;
    pfn_t pfn = 0;

    _pc_gdt_init();

    puts_screen("session04: physical memory allocator\n", 0x0F);

    status = mm_pma_init(0x00100000, 0x007FFFFF);
    if (!CHECK_SUCCESS(status)) {
        puts_screen("mm_pma_init failed: 0x", 0x0C);
        put_hex32(status, 0x0C);
        for (;;) {
            __asm__ volatile("hlt");
        }
    }

    mm_pma_get_free_frame_count(&free_frames);
    status = mm_pma_allocate_frame(2, &pfn);
    puts_screen("free frames ok, first allocation status=0x", 0x0A);
    put_hex32(status, 0x0A);
    putc_screen('\n', 0x0A);
    puts_screen("allocated PFN=0x", 0x0A);
    put_hex32((uint32_t)pfn, 0x0A);
    putc_screen('\n', 0x0A);

    status = mm_init();
    puts_screen("paging enable status=0x", 0x0F);
    put_hex32(status, 0x0F);
    putc_screen('\n', 0x0F);

    for (;;) {
        __asm__ volatile("hlt");
    }
}
