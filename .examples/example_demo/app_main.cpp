extern "C"
{
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_heap_caps.h" // pentru SPIRAM
#include "esp_psram.h"
#include "esp_log.h"
}

#define LOG_COLOR_D           LOG_ANSI_COLOR_REGULAR(LOG_COLOR_CYAN)
#define LOG_COLOR_V           LOG_ANSI_COLOR_REGULAR(LOG_COLOR_MAGENTA)

extern "C"
{
    void app_main(void)
    {
        // esp_log_level_set("*", ESP_LOG_DEBUG); //
        esp_log_level_set("*", ESP_LOG_MAX); //
        printf("Hello world!\n");

        /* Print chip information */
        esp_chip_info_t chip_info;
        uint32_t flash_size;
        esp_chip_info(&chip_info);
        printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
               CONFIG_IDF_TARGET,
               chip_info.cores,
               (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
               (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
               (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
               (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

        unsigned major_rev = chip_info.revision / 100;
        unsigned minor_rev = chip_info.revision % 100;
        printf("silicon revision v%d.%d, ", major_rev, minor_rev);
        if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
        {
            printf("Get flash size failed");
            return;
        }

        printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
               (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

        printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

        // 🧠 SPIRAM Check
        if (esp_psram_is_initialized())
        {
            size_t spiram_size = esp_psram_get_size();
            printf("SPIRAM initialized. Size: %d bytes (%.2f MB)\n", (int)spiram_size, spiram_size / 1048576.0f);

            // 🧪 Test allocare buffer mare
            char *buffer = (char *)heap_caps_malloc(512 * 1024, MALLOC_CAP_SPIRAM); // 512 Kb
            if (buffer != NULL)
            {
                printf("Allocated 512KB in SPIRAM at address: %p\n", buffer);
                snprintf(buffer, 100, "Salut, The One, bufferul din SPIRAM merge!\n");
                printf("%s", buffer);
                free(buffer);
            }
            else
            {
                printf("Eșec la alocarea bufferului în SPIRAM!\n");
            }
        }
        else
        {
            printf("⚠️ SPIRAM not initialized! Poate n-ai activat din menuconfig.\n");
        }

        for (int i = 30; i >= 0; i--)
        {
            printf("Restarting in %d seconds...\n", i);
            printf("Hello World!...\n");
            ESP_LOGE("*", "Hello world");
            ESP_LOGI("*", "Hello world");
            ESP_LOGW("*", "Hello world");
            ESP_LOGD("*", "Hello world");
            ESP_LOGV("*", "Hello world");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        printf("Restarting now.\n");
        fflush(stdout);
        esp_restart();
    }
}