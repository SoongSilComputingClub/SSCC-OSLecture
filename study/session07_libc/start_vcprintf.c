#include <stdio.h>

#include <ctype.h>
#include <stdint.h>

enum placeholder_length {
    LEN_DEFAULT = 0,
    LEN_CHAR,
    LEN_SHORT,
    LEN_LONG,
    LEN_LLONG,
    LEN_INTMAX,
    LEN_SIZE,
    LEN_PTRDIFF,
    LEN_LONG_DOUBLE,
};

struct placeholder_spec {
    enum placeholder_length length;
    char conv;
    const char *next;
};

static int emit_text(int (*func)(void *, char), void *farg, const char *text)
{
    int written = 0;

    while (*text) {
        if (func(farg, *text++)) {
            return written;
        }
        ++written;
    }

    return written;
}

static int is_flag_char(char ch)
{
    return ch == '-' || ch == '+' || ch == ' ' || ch == '#' || ch == '0';
}

static struct placeholder_spec decode_placeholder(const char *fmt)
{
    struct placeholder_spec spec = {
        .length = LEN_DEFAULT,
        .conv = '\0',
        .next = fmt,
    };

    while (*fmt && is_flag_char(*fmt)) {
        ++fmt;
    }

    while (*fmt && isdigit((unsigned char)*fmt)) {
        ++fmt;
    }

    if (*fmt == '.') {
        ++fmt;

        while (*fmt && isdigit((unsigned char)*fmt)) {
            ++fmt;
        }
    }

    switch (*fmt) {
    case 'h':
        spec.length = LEN_SHORT;
        if (fmt[1] == 'h') {
            spec.length = LEN_CHAR;
            ++fmt;
        }
        ++fmt;
        break;
    case 'l':
        spec.length = LEN_LONG;
        if (fmt[1] == 'l') {
            spec.length = LEN_LLONG;
            ++fmt;
        }
        ++fmt;
        break;
    case 'j':
        spec.length = LEN_INTMAX;
        ++fmt;
        break;
    case 'z':
        spec.length = LEN_SIZE;
        ++fmt;
        break;
    case 't':
        spec.length = LEN_PTRDIFF;
        ++fmt;
        break;
    case 'L':
        spec.length = LEN_LONG_DOUBLE;
        ++fmt;
        break;
    default:
        break;
    }

    if (*fmt) {
        spec.conv = *fmt++;
    }
    spec.next = fmt;

    return spec;
}

static void consume_argument(struct placeholder_spec spec, va_list *args)
{
    switch (spec.conv) {
    case 'c':
        (void)va_arg(*args, int);
        break;
    case 's':
        (void)va_arg(*args, const char *);
        break;
    case 'p':
    case 'n':
        (void)va_arg(*args, void *);
        break;
    case 'd':
    case 'i':
    case 'u':
    case 'o':
    case 'x':
    case 'X':
        switch (spec.length) {
        case LEN_LONG:
            (void)va_arg(*args, unsigned long);
            break;
        case LEN_LLONG:
            (void)va_arg(*args, unsigned long long);
            break;
        case LEN_INTMAX:
            (void)va_arg(*args, uintmax_t);
            break;
        case LEN_SIZE:
            (void)va_arg(*args, size_t);
            break;
        case LEN_PTRDIFF:
            (void)va_arg(*args, ptrdiff_t);
            break;
        default:
            (void)va_arg(*args, unsigned int);
            break;
        }
        break;
    case 'f':
    case 'F':
    case 'e':
    case 'E':
    case 'g':
    case 'G':
    case 'a':
    case 'A':
        if (spec.length == LEN_LONG_DOUBLE) {
            (void)va_arg(*args, long double);
        } else {
            (void)va_arg(*args, double);
        }
        break;
    default:
        break;
    }
}

int vcprintf(int (*func)(void *, char), void *farg, const char *fmt, va_list args)
{
    int write_count = 0;

    while (*fmt) {
        if (*fmt != '%') {
            if (func(farg, *fmt++)) {
                break;
            }
            ++write_count;
            continue;
        }

        ++fmt;
        if (*fmt == '%') {
            if (func(farg, *fmt++)) {
                break;
            }
            ++write_count;
            continue;
        }

        /*
         * TODO(session 07-5):
         * Decode flags, width, precision, and the conversion type.
         * Then consume the argument from `args` and render the formatted text
         * through `func`.
         */
        {
            struct placeholder_spec spec = decode_placeholder(fmt);
            consume_argument(spec, &args);
            write_count += emit_text(func, farg, "<todo>");
            fmt = spec.next;
        }
    }

    func(farg, 0);
    return write_count;
}
