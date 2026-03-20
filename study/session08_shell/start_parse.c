#include <vellum/shell.h>

#include <ctype.h>
#include <stdlib.h>

#define PUSH_CHAR_E(buf, buflen, ch)                                                               \
    do {                                                                                           \
        if (buflen > 0) {                                                                          \
            *buf++ = ch;                                                                           \
            buflen--;                                                                              \
        } else {                                                                                   \
            return STATUS_BUFFER_TOO_SMALL;                                                        \
        }                                                                                          \
    } while (0)

#define PUSH_CHAR(buf, buflen, ch)                                                                 \
    do {                                                                                           \
        if (buflen > 0) {                                                                          \
            *buf++ = ch;                                                                           \
            buflen--;                                                                              \
        } else {                                                                                   \
            return NULL;                                                                           \
        }                                                                                          \
    } while (0)

status_t VlShell_Expand(struct shell_instance *inst, const char *line, char *buf, long buflen)
{
    /*
     * TODO(session 08-1):
     * Implement:
     * - plain character copy
     * - escaped `$`
     * - `$VAR` expansion through `VlShell_GetVariable`
     */
    (void)inst;
    (void)line;
    (void)buf;
    (void)buflen;
    return STATUS_NOT_IMPLEMENTED;
}

const char *VlShell_Parse(const char *line, char *buf, long buflen)
{
    /*
     * TODO(session 08-2):
     * Parse one shell token with quote handling, backslash escapes, and
     * whitespace-delimited arguments.
     */
    (void)line;
    (void)buf;
    (void)buflen;
    return NULL;
}
