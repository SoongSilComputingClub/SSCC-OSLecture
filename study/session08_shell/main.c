#include <stdio.h>
#include <time.h>

#include <vellum/compiler.h>
#include <vellum/panic.h>
#include <vellum/shell.h>
#include <vellum/status.h>

struct json_value;

struct json_value *config_data = NULL;
const char *config_password = NULL;
time_t config_timezone_offset = 0;
int config_rtc_utc = 1;

static void setup_tty(void)
{
    if (freopendevice("kbd0", stdin)) {
        VlP_Panic(STATUS_UNKNOWN_ERROR, "failed to reopen essential file");
    }
}

static void read_config(void)
{
    /* boot.json parsing is introduced after the interactive shell is in place. */
}

__noreturn void main(void)
{
    setup_tty();
    read_config();

    for (;;) {
        VlShell_Execute(NULL, "shell");
    }
}
