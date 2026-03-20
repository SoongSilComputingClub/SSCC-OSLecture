#include <vellum/shell.h>

#include <vellum/device.h>
#include <vellum/interface/video.h>

/*
 * Skeleton source for session 06.
 * Reference implementation:
 * vellum/shell/commands/vmode.c
 */

static int list_modes(char *argv0)
{
    /*
     * TODO(session 06-4):
     * - Find `video0`
     * - Query its `video` interface
     * - Walk every mode using `get_mode_info`
     * - Print a compact mode list to stdout
     */
    (void)argv0;
    return 1;
}

static int set_mode(char *argv0, int text, int width, int height, int bpp)
{
    /*
     * TODO(session 06-5):
     * Scan available modes, find the requested mode, and switch to it.
     */
    (void)argv0;
    (void)text;
    (void)width;
    (void)height;
    (void)bpp;
    return 1;
}
