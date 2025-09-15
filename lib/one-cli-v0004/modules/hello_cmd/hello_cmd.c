#include <stdio.h>
#include "esp_timer.h"
#include "esp_console.h"
#include "esp_log.h"
#include "hello_cmd.h"

static const char *TAG = "CLI";

static int hello_command(int argc, char **argv)
{
    printf("Hello World!\n");
    return 0;
}

static void register_hello(void)
{
    const esp_console_cmd_t cmd = {
        .command = "hello",
        .help = "Print Hello World",
        .hint = NULL,
        .func = &hello_command,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI(TAG, "'%s' command registered.", cmd.command);
}

void cli_register_hello_command(void)
{
    register_hello();
}
