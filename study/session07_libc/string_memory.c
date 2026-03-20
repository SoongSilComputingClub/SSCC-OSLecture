#include <string.h>

/*
 * Skeleton source for session 07.
 * Reference implementation:
 * vellum/stdc/string/memset.c
 * vellum/stdc/string/memcpy.c
 * vellum/stdc/string/memmove.c
 * vellum/stdc/string/strnlen.c
 */

void *memset(void *dest, int c, size_t count)
{
    /*
     * TODO(session 07-1):
     * Fill `count` bytes of `dest` with the low 8 bits of `c`.
     */
    (void)dest;
    (void)c;
    (void)count;
    return dest;
}

void *memcpy(void *__restrict dest, const void *__restrict src, size_t len)
{
    /*
     * TODO(session 07-2):
     * Copy `len` bytes from `src` to `dest`.
     */
    (void)dest;
    (void)src;
    (void)len;
    return dest;
}

void *memmove(void *dest, const void *src, size_t len)
{
    /*
     * TODO(session 07-3):
     * Handle overlapping regions correctly.
     */
    (void)dest;
    (void)src;
    (void)len;
    return dest;
}

size_t strnlen(const char *str, size_t maxlen)
{
    /*
     * TODO(session 07-4):
     * Stop at either the first NUL or `maxlen`, whichever comes first.
     */
    (void)str;
    (void)maxlen;
    return 0;
}

size_t strlen(const char *str)
{
    /*
     * TODO(session 07-5):
     * Return the number of bytes before the terminating NUL.
     */
    (void)str;
    return 0;
}
