#include <vellum/arch/io.h>

#include <vellum/device.h>
#include <vellum/interface/char.h>

/*
 * Skeleton source for session 05.
 * Reference implementation:
 * vellum/device/char/uart_isa.c
 */

#define UART_DEVICE_CAPACITY 4

struct uart_isa_data {
    uint16_t ioport;
};

static struct uart_isa_data uart_data_pool[UART_DEVICE_CAPACITY];
static uint8_t uart_data_used[UART_DEVICE_CAPACITY];

static int string_equal(const char *lhs, const char *rhs)
{
    if (!lhs || !rhs) return 0;

    while (*lhs && *rhs) {
        if (*lhs != *rhs) return 0;
        ++lhs;
        ++rhs;
    }

    return *lhs == '\0' && *rhs == '\0';
}

static struct uart_isa_data *allocate_uart_data(void)
{
    size_t i;

    for (i = 0; i < UART_DEVICE_CAPACITY; ++i) {
        if (!uart_data_used[i]) {
            uart_data_used[i] = 1;
            uart_data_pool[i] = (struct uart_isa_data){0};
            return &uart_data_pool[i];
        }
    }

    return NULL;
}

static void release_uart_data(struct uart_isa_data *data)
{
    size_t i;

    for (i = 0; i < UART_DEVICE_CAPACITY; ++i) {
        if (&uart_data_pool[i] == data) {
            uart_data_used[i] = 0;
            uart_data_pool[i] = (struct uart_isa_data){0};
            return;
        }
    }
}

static status_t write(struct device *dev, const char *buf, size_t len, size_t *result)
{
    /*
     * TODO(session 05-6):
     * Write each byte to the UART base port and return the transferred size.
     */
    size_t i;
    struct uart_isa_data *data;

    if (!dev || !buf) return STATUS_INVALID_VALUE;

    data = (struct uart_isa_data *)dev->data;
    if (!data) return STATUS_CONFLICTING_STATE;

    for (i = 0; i < len; ++i) {
        VlA_Out8(data->ioport, (uint8_t)buf[i]);
    }

    if (result) *result = len;
    return STATUS_SUCCESS;
}

static const struct char_interface charif = {
    .write = write,
};

static status_t probe(
    struct device **devout,
    struct device_driver *drv,
    struct device *parent,
    struct resource *rsrc,
    int rsrc_cnt
)
{
    /*
     * TODO(session 05-7):
     * Validate resources, create a device, assign a generated name, store the
     * I/O base, and publish the new device.
     */
    status_t status;
    struct device *dev = NULL;
    struct uart_isa_data *data = NULL;

    if (!rsrc || rsrc_cnt < 1 || rsrc[0].type != RT_IOPORT) {
        return STATUS_INVALID_RESOURCE;
    }

    status = VlDev_Create(&dev, drv, parent);
    if (!CHECK_SUCCESS(status)) return status;

    status = VlDev_GenerateName("ser", dev->name, sizeof(dev->name));
    if (!CHECK_SUCCESS(status)) {
        VlDev_Remove(dev);
        return status;
    }

    data = allocate_uart_data();
    if (!data) {
        VlDev_Remove(dev);
        return STATUS_INSUFFICIENT_MEMORY;
    }

    data->ioport = (uint16_t)rsrc[0].base;
    dev->data = data;

    if (devout) *devout = dev;

    return STATUS_SUCCESS;
}

static status_t get_interface(struct device *dev, const char *name, const void **result)
{
    /*
     * TODO(session 05-8):
     * Return `charif` when `name` is `"char"`.
     */
    (void)dev;

    if (string_equal(name, "char")) {
        if (result) *result = &charif;
        return STATUS_SUCCESS;
    }

    return STATUS_ENTRY_NOT_FOUND;
}

static status_t remove(struct device *dev)
{
    if (dev && dev->data) {
        release_uart_data((struct uart_isa_data *)dev->data);
    }

    VlDev_Remove(dev);
    return STATUS_SUCCESS;
}

static void uart_isa_init(void)
{
    status_t status;
    struct device_driver *drv;

    status = VlDev_CreateDriver(&drv);
    if (!CHECK_SUCCESS(status)) {
        return;
    }

    drv->name = "uart_isa";
    drv->probe = probe;
    drv->remove = remove;
    drv->get_interface = get_interface;
}

REGISTER_DEVICE_DRIVER(uart_isa, uart_isa_init)
