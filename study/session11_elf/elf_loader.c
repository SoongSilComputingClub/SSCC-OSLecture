#include <vellum/elf.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vellum/mm.h>

/*
 * Skeleton source for session 11.
 * Reference implementation:
 * vellum/elf/elf.c
 */

status_t VlElf_Open(const char *path, struct elf_file **elfout)
{
    /*
     * TODO(session 11-1):
     * - Allocate `struct elf_file`
     * - Open the file
     * - Read and validate the ELF ident header
     * - Load the main ELF header for the supported class
     */
    (void)path;
    (void)elfout;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_GetProgramHeader(struct elf_file *elf, unsigned int index, void *buf, size_t len)
{
    /*
     * TODO(session 11-2):
     * Seek to the requested program header and copy it into `buf`.
     */
    (void)elf;
    (void)index;
    (void)buf;
    (void)len;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_LoadProgram(struct elf_file *elf, unsigned int index, void *paddr)
{
    /*
     * TODO(session 11-3):
     * - Read one PT_LOAD segment
     * - Allocate enough pages at the requested load address
     * - Copy file data and zero-fill the BSS tail
     */
    (void)elf;
    (void)index;
    (void)paddr;
    return STATUS_NOT_IMPLEMENTED;
}
