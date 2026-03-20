#include <vellum/device.h>
#include <vellum/interface/char.h>

/*
 * Starter source for session 05.
 * The device registry is already present in this stage; students fill in the
 * UART driver's probe/write/get_interface path.
 */

static status_t write(struct device *dev, const char *buf, size_t len, size_t *result)
{
    /*
     * TODO(session 05-6):
     * Push `len` bytes from `buf` into the UART transmit register.
     */
    (void)dev;
    (void)buf;
    (void)len;

    if (result) {
        *result = 0;
    }
    return STATUS_NOT_IMPLEMENTED;
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
     * Validate the I/O resource, create the device, assign a generated name,
     * and keep the UART base port in `dev->data`.
     */
    (void)devout;
    (void)drv;
    (void)parent;
    (void)rsrc;
    (void)rsrc_cnt;
    return STATUS_NOT_IMPLEMENTED;
}

static status_t get_interface(struct device *dev, const char *name, const void **result)
{
    /*
     * TODO(session 05-8):
     * Return `charif` when the caller asks for the `"char"` interface.
     */
    (void)dev;

    if (name && name[0] == 'c' && name[1] == 'h' && name[2] == 'a' && name[3] == 'r' &&
        name[4] == '\0') {
        if (result) {
            *result = &charif;
        }
        return STATUS_SUCCESS;
    }

    return STATUS_NOT_IMPLEMENTED;
}

static status_t remove(struct device *dev)
{
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
