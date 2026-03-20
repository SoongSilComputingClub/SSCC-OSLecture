#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <vellum/arch/interrupt.h>
#include <vellum/arch/io.h>

#include <vellum/plat/bios/bootinfo.h>
#include <vellum/plat/bios/keyboard.h>
#include <vellum/plat/bios/mem.h>
#include <vellum/plat/bios/video.h>
#include <vellum/plat/gdt.h>
#include <vellum/plat/isr.h>
#include <vellum/plat/pic.h>

#include <vellum/compiler.h>
#include <vellum/device.h>
#include <vellum/filesystem.h>
#include <vellum/interface/block.h>
#include <vellum/macros.h>
#include <vellum/mm.h>
#include <vellum/panic.h>
#include <vellum/status.h>

extern void main(void);

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

static ssize_t early_stderr_write(void *cookie, const char *buf, size_t count)
{
    (void)cookie;

    for (size_t i = 0; i < count && buf[i]; i++) {
        if (buf[i] == '\n') {
            VlBiosP_WriteVideoTty('\r');
        }

        VlBiosP_WriteVideoTty((uint8_t)buf[i]);
        VlA_Out8(0x00E9, (uint8_t)buf[i]);
    }

    return (ssize_t)count;
}

static const struct cookie_io_functions early_stderr_io = {
    .write = early_stderr_write,
};

struct mbr_partition_entry_fallback {
    uint8_t status;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t base_lba;
    uint32_t sector_count;
} __packed;

struct mbr_sector_fallback {
    uint8_t boot_code[446];
    struct mbr_partition_entry_fallback partition_entries[4];
    uint16_t boot_signature;
} __packed;

#define MBR_BOOT_SIGNATURE 0xAA55

static status_t find_boot_partition_device(
    struct device *diskdev,
    const struct block_interface *blki,
    const uint8_t *sect0,
    struct device **partdevout
)
{
    status_t status;
    struct device_driver *partdrv;
    const struct mbr_sector_fallback *mbr = (const struct mbr_sector_fallback *)sect0;

    (void)blki;

    if (mbr->boot_signature != MBR_BOOT_SIGNATURE) {
        return STATUS_INVALID_SIGNATURE;
    }

    status = VlDev_FindDriver("part", &partdrv);
    if (!CHECK_SUCCESS(status)) return status;

    for (size_t i = 0; i < ARRAY_SIZE(mbr->partition_entries); ++i) {
        const struct mbr_partition_entry_fallback *entry = &mbr->partition_entries[i];
        struct resource res[] = {
            {
                .type = RT_LBA,
                .base = entry->base_lba,
                .limit = entry->base_lba + entry->sector_count - 1,
                .flags = 0,
            },
        };

        if (!entry->type || !entry->sector_count) continue;
        if (entry->base_lba != _pc_boot_part_base) continue;

        return partdrv->probe(partdevout, partdrv, diskdev, res, ARRAY_SIZE(res));
    }

    return STATUS_ENTRY_NOT_FOUND;
}

static status_t init_pma(void)
{
    status_t status;
    uint32_t smap_cursor;
    struct smap_entry smap_entry;
    uint64_t smap_base, smap_size;
    uintptr_t base_paddr, limit_paddr;

    smap_cursor = 0;
    base_paddr = 0;
    limit_paddr = 0;
    do {
        status = VlBiosP_QueryMemoryMap(&smap_cursor, &smap_entry, sizeof(smap_entry));
        if (!CHECK_SUCCESS(status)) return status;

        smap_base = (uint64_t)smap_entry.base_addr_high << 32 | smap_entry.base_addr_low;
        smap_size = (uint64_t)smap_entry.length_high << 32 | smap_entry.length_low;

        if (smap_base < 0x100000) continue;
        if (smap_entry.type != 0x00000001) continue;

        if (!base_paddr) {
            base_paddr = smap_base;
        }
        if (limit_paddr < smap_base + smap_size - 1) {
            limit_paddr = smap_base + smap_size - 1;
        }
    } while (smap_cursor);

    status = mm_pma_init(base_paddr, limit_paddr);
    if (!CHECK_SUCCESS(status)) return status;

    smap_cursor = 0;
    do {
        status = VlBiosP_QueryMemoryMap(&smap_cursor, &smap_entry, sizeof(smap_entry));
        if (!CHECK_SUCCESS(status)) return status;

        smap_base = (uint64_t)smap_entry.base_addr_high << 32 | smap_entry.base_addr_low;
        smap_size = (uint64_t)smap_entry.length_high << 32 | smap_entry.length_low;

        if (smap_base + smap_size <= 0x100000) continue;
        if (smap_base > limit_paddr) continue;
        if (smap_entry.type == 0x00000001) continue;

        if (smap_base < 0x100000) {
            smap_size -= 0x100000 - smap_base;
            smap_base = 0x100000;
        }

        base_paddr = smap_base;
        limit_paddr = smap_base + smap_size - 1;

        status = mm_pma_mark_reserved(base_paddr, limit_paddr);
        if (!CHECK_SUCCESS(status)) return status;
    } while (smap_cursor);

    return STATUS_SUCCESS;
}

static volatile uint64_t global_tick = 0;
int config_rtc_century_offset = 0;

uint64_t get_global_tick(void)
{
    return global_tick;
}

static void pit_isr(void *data, struct VlA_InterruptFrame *frame, struct trap_regs *regs, int num)
{
    (void)data;
    (void)frame;
    (void)regs;
    (void)num;

    global_tick++;
}

static void init_pit(void)
{
    static const uint16_t pit_value = 1193182 / 20;

    VlA_Out8(0x0043, 0x34);
    VlA_Out8(0x0040, pit_value & 0xFF);
    VlA_Out8(0x0040, (pit_value >> 8) & 0xFF);

    VlIntP_Unmask(0x20);
}

static void probe_ide_channel(uint16_t io_base0, uint16_t io_base1, uint8_t irq_num)
{
    status_t status;
    struct device_driver *drv;
    struct device *dev;
    struct resource res[] = {
        {
            .type = RT_IOPORT,
            .base = io_base0,
            .limit = io_base0 + 7,
            .flags = 0,
        },
        {
            .type = RT_IOPORT,
            .base = io_base1,
            .limit = io_base1 + 1,
            .flags = 0,
        },
        {
            .type = RT_IRQ,
            .base = irq_num,
            .limit = irq_num,
            .flags = 0,
        },
    };

    status = VlDev_FindDriver("ide_isa", &drv);
    if (!CHECK_SUCCESS(status)) {
        return;
    }

    (void)drv->probe(&dev, drv, NULL, res, ARRAY_SIZE(res));
}

static status_t init_nonpnp_devices(void)
{
    status_t status;
    struct device_driver *drv;
    struct device *dev;
    struct resource ps2_res[] = {
        {
            .type = RT_IOPORT,
            .base = 0x0060,
            .limit = 0x0060,
            .flags = 0,
        },
        {
            .type = RT_IOPORT,
            .base = 0x0064,
            .limit = 0x0064,
            .flags = 0,
        },
        {
            .type = RT_IRQ,
            .base = 0x21,
            .limit = 0x21,
            .flags = 0,
        },
        {
            .type = RT_IRQ,
            .base = 0x2C,
            .limit = 0x2C,
            .flags = 0,
        },
    };

    status = VlDev_FindDriver("i8042", &drv);
    if (!CHECK_SUCCESS(status)) return status;

    status = drv->probe(&dev, drv, NULL, ps2_res, ARRAY_SIZE(ps2_res));
    if (!CHECK_SUCCESS(status)) return status;

    status = VlDev_FindDriver("fdc_isa", &drv);
    if (CHECK_SUCCESS(status)) {
        struct resource res[] = {
            {
                .type = RT_IOPORT,
                .base = 0x03F0,
                .limit = 0x03F7,
                .flags = 0,
            },
            {
                .type = RT_IRQ,
                .base = 0x26,
                .limit = 0x26,
                .flags = 0,
            },
            {
                .type = RT_DMA,
                .base = 0,
                .limit = 0,
                .flags = 0,
            },
        };

        (void)drv->probe(&dev, drv, NULL, res, ARRAY_SIZE(res));
    }

    probe_ide_channel(0x01F0, 0x03F6, 0x2E);
    probe_ide_channel(0x0170, 0x0376, 0x2F);

    status = VlDev_FindDriver("vga", &drv);
    if (!CHECK_SUCCESS(status)) return status;

    status = drv->probe(&dev, drv, NULL, NULL, 0);
    if (!CHECK_SUCCESS(status)) return status;

    return STATUS_SUCCESS;
}

static status_t mount_boot_filesystem(void)
{
    status_t status;
    struct device *bootdisk;
    const struct block_interface *blki;
    uint8_t sect0[512];
    size_t sect_size;

    bootdisk = VlDev_GetFirst();

    for (; bootdisk; bootdisk = bootdisk->next) {
        status = bootdisk->driver->get_interface(bootdisk, "block", (const void **)&blki);
        if (!CHECK_SUCCESS(status)) continue;

        status = blki->get_block_size(bootdisk, &sect_size);
        if (!CHECK_SUCCESS(status) || sect_size > sizeof(sect0)) continue;

        status = blki->read(bootdisk, 0, sect0, 1, NULL);
        if (!CHECK_SUCCESS(status)) continue;

        if (memcmp(_pc_boot_sector, sect0, sizeof(sect0)) == 0) break;

        struct device *partdev = NULL;
        status = find_boot_partition_device(bootdisk, blki, sect0, &partdev);
        if (CHECK_SUCCESS(status) && partdev) {
            bootdisk = partdev;
            break;
        }
    }

    if (!bootdisk) return STATUS_BOOT_DEVICE_INACCESSIBLE;

    return VlFs_MountAuto(bootdisk, "boot");
}

__noreturn void _pc_init(void)
{
    status_t status;
    int i;

    freopencookie(NULL, "w", early_stderr_io, stderr);

    _pc_gdt_init();
    VlIntP_Init();
    _pc_pic_remap_int(0x20, 0x28);

    status = init_pma();
    if (!CHECK_SUCCESS(status)) {
        VlP_Panic(status, "failed to initialize physical memory allocator");
    }

    status = mm_init();
    if (!CHECK_SUCCESS(status)) {
        VlP_Panic(status, "failed to initialize memory management");
    }

    status = VlIntP_AddInterruptHandler(0x20, NULL, pit_isr, NULL);
    if (!CHECK_SUCCESS(status)) {
        VlP_Panic(status, "failed to initialize PIT interrupt");
    }

    init_pit();

    for (i = 0; &(&__init_array_start)[i] != &__init_array_end; i++) {
        (&__init_array_start)[i]();
    }

    VlA_EnableInterrupt();

    status = init_nonpnp_devices();
    if (!CHECK_SUCCESS(status)) {
        fprintf(stderr, "init_nonpnp_devices() failed: 0x%08lX\n", status);
        VlP_Panic(status, "failed to initialize essential non-PnP devices");
    }

    status = mount_boot_filesystem();
    if (!CHECK_SUCCESS(status)) {
        fprintf(stderr, "mount_boot_filesystem() failed: 0x%08lX\n", status);
    }

    main();

    for (;;) {
        __asm__ volatile("hlt");
    }
}
