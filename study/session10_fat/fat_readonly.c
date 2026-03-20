#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vellum/device.h>
#include <vellum/filesystem.h>
#include <vellum/interface/block.h>
#include <vellum/status.h>

#include "../../vellum/filesystem/fat/fat.h"

/*
 * Skeleton source for session 10.
 * Reference implementation:
 * vellum/filesystem/fat/fat.c
 */

struct fat_dir_data {
    fatcluster_t head_cluster, current_cluster;
    int root_cluster;
    unsigned int current_entry_index;
    struct fat_direntry_file direntry;
};

struct fat_file_data {
    struct fat_direntry_file direntry;
    fatcluster_t head_cluster, current_cluster;
    uint32_t cursor;
};

struct fat_data {
    struct device *blkdev;
    const struct block_interface *blkif;

    uint16_t reserved_sectors;
    uint16_t sector_size;
    uint8_t sectors_per_cluster;
    uint8_t fat_type;
    uint32_t data_area_begin;
    uint32_t fat_size;
    uint32_t root_cluster;
    uint16_t root_entry_count;
    uint16_t root_sector_count;

    lba_t fatbuf_lba;
    uint8_t fatbuf[FAT_SECTOR_SIZE];
    lba_t databuf_lba_start;
    uint8_t *databuf;
};

static status_t probe(struct device *dev, struct fs_driver *drv)
{
    /*
     * TODO(session 10-1):
     * Read sector 0, validate the BPB/FAT signature, and reject devices that do
     * not look like FAT.
     */
    (void)dev;
    (void)drv;
    return STATUS_NOT_IMPLEMENTED;
}

static status_t mount(
    struct filesystem **fsout, struct fs_driver *drv, struct device *dev, const char *name
)
{
    /*
     * TODO(session 10-2):
     * Allocate filesystem state, parse the BPB, determine FAT12/16/32 layout,
     * and create a mounted filesystem instance.
     */
    (void)fsout;
    (void)drv;
    (void)dev;
    (void)name;
    return STATUS_NOT_IMPLEMENTED;
}

static status_t open(struct fs_directory *dir, const char *name, struct fs_file **fileout)
{
    /*
     * TODO(session 10-3):
     * Search the current directory, match the requested 8.3 name, and create a
     * file object bound to the discovered directory entry.
     */
    (void)dir;
    (void)name;
    (void)fileout;
    return STATUS_NOT_IMPLEMENTED;
}

static status_t read(struct fs_file *file, void *buf, size_t len, size_t *result)
{
    /*
     * TODO(session 10-4):
     * Follow the cluster chain, read data into `buf`, respect EOF, and advance
     * the file cursor.
     */
    (void)file;
    (void)buf;
    (void)len;
    (void)result;
    return STATUS_NOT_IMPLEMENTED;
}

static status_t open_root_directory(struct filesystem *fs, struct fs_directory **dirout)
{
    /*
     * TODO(session 10-5):
     * Create a root directory iterator for FAT12/16 root or FAT32 root cluster.
     */
    (void)fs;
    (void)dirout;
    return STATUS_NOT_IMPLEMENTED;
}

static status_t iter_directory(struct fs_directory *dir, struct fs_directory_entry *entry)
{
    /*
     * TODO(session 10-6):
     * Iterate directory entries, skip deleted/LFN metadata, and fill `entry`
     * with a printable filename and size.
     */
    (void)dir;
    (void)entry;
    return STATUS_NOT_IMPLEMENTED;
}
