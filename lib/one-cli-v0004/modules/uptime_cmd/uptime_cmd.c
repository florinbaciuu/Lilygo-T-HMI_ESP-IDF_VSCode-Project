


#include "uptime_cmd.h"

#include <stdio.h>
#include "esp_timer.h"
#include "esp_console.h"
#include "esp_log.h"


static const char *TAG = "CLI";

static int uptime_command(int argc, char **argv)
{
    int64_t uptime_us = esp_timer_get_time();
    int64_t uptime_s = uptime_us / 1000000;

    int days = uptime_s / (24 * 3600);
    uptime_s %= (24 * 3600);
    int hours = uptime_s / 3600;
    uptime_s %= 3600;
    int minutes = uptime_s / 60;
    int seconds = uptime_s % 60;

    printf("Uptime: %d days, %02d:%02d:%02d\n", days, hours, minutes, seconds);
    return 0;
}

static void register_uptime(void)
{
    const esp_console_cmd_t cmd = {
        .command = "uptime",
        .help = "Show system uptime",
        .hint = NULL,
        .func = &uptime_command,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI(TAG, "'%s' command registered.", cmd.command);
}

void cli_register_uptime_command(void)
{
    register_uptime();
}
