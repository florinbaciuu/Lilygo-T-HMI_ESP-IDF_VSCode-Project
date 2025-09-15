extern "C"
{
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_heap_caps.h" // pentru SPIRAM
#include "esp_log.h"
}

#define LOG_COLOR_D           LOG_ANSI_COLOR_REGULAR(LOG_COLOR_CYAN)
#define LOG_COLOR_V           LOG_ANSI_COLOR_REGULAR(LOG_COLOR_MAGENTA)

extern "C"
{
    void app_main(void)
    {
        esp_log_level_set("*", ESP_LOG_MAX); //
        printf("Hello world!\n");
        fflush(stdout);
    }
}