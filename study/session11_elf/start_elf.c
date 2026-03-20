#include <vellum/elf.h>

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

void VlElf_Close(struct elf_file *elf)
{
    (void)elf;
}

status_t VlElf_GetHeader(struct elf_file *elf, void *buf, size_t len)
{
    (void)elf;
    (void)buf;
    (void)len;
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

status_t VlElf_GetSectionHeader(struct elf_file *elf, unsigned int index, void *buf, size_t len)
{
    (void)elf;
    (void)index;
    (void)buf;
    (void)len;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_GetSectionName(struct elf_file *elf, unsigned int index, const char **name)
{
    (void)elf;
    (void)index;
    (void)name;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_FindSection(struct elf_file *elf, const char *name, unsigned int *idx)
{
    (void)elf;
    (void)name;
    (void)idx;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_LoadSection(struct elf_file *elf, unsigned int index, void *buf, size_t len)
{
    (void)elf;
    (void)index;
    (void)buf;
    (void)len;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_FindSymbol(struct elf_file *elf, const char *name, unsigned int *index)
{
    (void)elf;
    (void)name;
    (void)index;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_GetSymbol(struct elf_file *elf, unsigned int index, void *buf, size_t len)
{
    (void)elf;
    (void)index;
    (void)buf;
    (void)len;
    return STATUS_NOT_IMPLEMENTED;
}

status_t VlElf_GetSymbolCount(struct elf_file *elf, unsigned int *count)
{
    (void)elf;
    (void)count;
    return STATUS_NOT_IMPLEMENTED;
}
