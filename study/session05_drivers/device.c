#include <vellum/device.h>

#include <stddef.h>

/*
 * Skeleton source for session 05.
 * Reference implementation:
 * vellum/device/device.c
 */

#define DEVICE_POOL_CAPACITY 16
#define DRIVER_POOL_CAPACITY 16

static struct device *device_list_head = NULL;
static struct device_driver *driver_list_head = NULL;
static struct device device_pool[DEVICE_POOL_CAPACITY];
static uint8_t device_pool_used[DEVICE_POOL_CAPACITY];
static struct device_driver driver_pool[DRIVER_POOL_CAPACITY];
static uint8_t driver_pool_used[DRIVER_POOL_CAPACITY];

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

static int has_prefix(const char *str, const char *prefix)
{
    if (!str || !prefix) return 0;

    while (*prefix) {
        if (*str++ != *prefix++) return 0;
    }

    return 1;
}

static int ascii_is_digit(char ch)
{
    return ch >= '0' && ch <= '9';
}

static unsigned int parse_decimal(const char *str)
{
    unsigned int value = 0;

    while (*str && ascii_is_digit(*str)) {
        value = value * 10u + (unsigned int)(*str - '0');
        ++str;
    }

    return value;
}

static void write_decimal(char *buf, size_t len, unsigned int value)
{
    char tmp[16];
    size_t digits = 0;
    size_t pos = 0;

    if (!buf || !len) return;

    if (value == 0) {
        if (len > 1) {
            buf[0] = '0';
            buf[1] = '\0';
        } else {
            buf[0] = '\0';
        }
        return;
    }

    while (value && digits < sizeof(tmp)) {
        tmp[digits++] = (char)('0' + (value % 10u));
        value /= 10u;
    }

    while (digits && pos + 1 < len) {
        buf[pos++] = tmp[--digits];
    }

    buf[pos] = '\0';
}

static struct device *allocate_device(void)
{
    size_t i;

    for (i = 0; i < DEVICE_POOL_CAPACITY; ++i) {
        if (!device_pool_used[i]) {
            device_pool_used[i] = 1;
            device_pool[i] = (struct device){0};
            return &device_pool[i];
        }
    }

    return NULL;
}

static void release_device(struct device *dev)
{
    size_t i;

    for (i = 0; i < DEVICE_POOL_CAPACITY; ++i) {
        if (&device_pool[i] == dev) {
            device_pool_used[i] = 0;
            device_pool[i] = (struct device){0};
            return;
        }
    }
}

static struct device_driver *allocate_driver(void)
{
    size_t i;

    for (i = 0; i < DRIVER_POOL_CAPACITY; ++i) {
        if (!driver_pool_used[i]) {
            driver_pool_used[i] = 1;
            driver_pool[i] = (struct device_driver){0};
            return &driver_pool[i];
        }
    }

    return NULL;
}

static void append_device(struct device *dev)
{
    struct device *current;

    if (!device_list_head) {
        device_list_head = dev;
        return;
    }

    for (current = device_list_head; current->next; current = current->next) {
    }

    current->next = dev;
}

static void append_child(struct device *parent, struct device *dev)
{
    struct device *current;

    if (!parent) return;

    if (!parent->first_child) {
        parent->first_child = dev;
        return;
    }

    for (current = parent->first_child; current->sibling; current = current->sibling) {
    }

    current->sibling = dev;
}

static void unlink_device_from_parent(struct device *dev)
{
    struct device *current;

    if (!dev || !dev->parent) return;

    if (dev->parent->first_child == dev) {
        dev->parent->first_child = dev->sibling;
        return;
    }

    for (current = dev->parent->first_child; current && current->sibling; current = current->sibling) {
        if (current->sibling == dev) {
            current->sibling = dev->sibling;
            return;
        }
    }
}

struct device *VlDev_GetFirst(void)
{
    return device_list_head;
}

status_t VlDev_Create(struct device **devout, struct device_driver *drv, struct device *parent)
{
    /*
     * TODO(session 05-1):
     * - Allocate and zero a device object
     * - Link it into the global device list
     * - Link it into the parent's child/sibling list if `parent` is not NULL
     */
    struct device *dev;

    dev = allocate_device();
    if (!dev) return STATUS_INSUFFICIENT_MEMORY;

    dev->driver = drv;
    dev->parent = parent;
    append_device(dev);
    append_child(parent, dev);

    if (devout) *devout = dev;

    return STATUS_SUCCESS;
}

void VlDev_Remove(struct device *dev)
{
    struct device *current;
    struct device *prev = NULL;

    if (!dev) return;

    unlink_device_from_parent(dev);

    for (current = device_list_head; current; current = current->next) {
        if (current == dev) {
            if (prev) {
                prev->next = dev->next;
            } else {
                device_list_head = dev->next;
            }
            break;
        }
        prev = current;
    }

    release_device(dev);
}

status_t VlDev_Find(const char *id, struct device **dev)
{
    /*
     * TODO(session 05-2):
     * Walk the global device list and match against `device->name`.
     */
    struct device *current;

    for (current = device_list_head; current; current = current->next) {
        if (string_equal(current->name, id)) {
            if (dev) *dev = current;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ENTRY_NOT_FOUND;
}

status_t VlDev_GenerateName(const char *basename, char *buf, size_t len)
{
    /*
     * TODO(session 05-3):
     * Scan existing devices whose name starts with `basename`, find the largest
     * numeric suffix, and generate the next one into `buf`.
     */
    struct device *current;
    unsigned int max_suffix = 0;
    size_t base_len = 0;
    size_t pos = 0;
    char suffix[16];
    size_t i;

    if (!basename || !buf || !len) return STATUS_INVALID_VALUE;

    while (basename[base_len]) {
        if (pos + 1 >= len) return STATUS_BUFFER_TOO_SMALL;
        buf[pos++] = basename[base_len++];
    }

    for (current = device_list_head; current; current = current->next) {
        unsigned int suffix_value;

        if (!has_prefix(current->name, basename)) continue;
        if (!ascii_is_digit(current->name[base_len])) continue;

        suffix_value = parse_decimal(&current->name[base_len]);
        if (suffix_value >= max_suffix) {
            max_suffix = suffix_value + 1u;
        }
    }

    write_decimal(suffix, sizeof(suffix), max_suffix);

    for (i = 0; suffix[i]; ++i) {
        if (pos + 1 >= len) return STATUS_BUFFER_TOO_SMALL;
        buf[pos++] = suffix[i];
    }

    buf[pos] = '\0';
    return STATUS_SUCCESS;
}

status_t VlDev_CreateDriver(struct device_driver **drvout)
{
    /*
     * TODO(session 05-4):
     * Allocate a driver object and append it to the global driver list.
     */
    struct device_driver *drv;
    struct device_driver *current;

    drv = allocate_driver();
    if (!drv) return STATUS_INSUFFICIENT_MEMORY;

    if (!driver_list_head) {
        driver_list_head = drv;
    } else {
        for (current = driver_list_head; current->next; current = current->next) {
        }
        current->next = drv;
    }

    if (drvout) *drvout = drv;

    return STATUS_SUCCESS;
}

status_t VlDev_FindDriver(const char *name, struct device_driver **drv)
{
    /*
     * TODO(session 05-5):
     * Find a driver by its string name.
     */
    struct device_driver *current;

    for (current = driver_list_head; current; current = current->next) {
        if (string_equal(current->name, name)) {
            if (drv) *drv = current;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ENTRY_NOT_FOUND;
}
