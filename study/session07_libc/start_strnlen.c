#include <string.h>

#include <stdint.h>

size_t strnlen(const char *str, size_t maxlen)
{
    /*
     * TODO(session 07-4):
     * Count characters in `str` without reading past `maxlen`.
     */
    (void)str;
    (void)maxlen;
    return 0;
}

size_t strlen(const char *str)
{
    size_t s = 0;
    for (; *str++; s++)
        ;
    return s;
}
