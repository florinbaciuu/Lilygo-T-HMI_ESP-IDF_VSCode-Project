#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "driver/gpio.h"

#define MOUNT_POINT "/sdcard"
#define SDIO_DATA0_PIN (13)
#define SDIO_CMD_PIN  (11)
#define SDIO_SCLK_PIN (12)

extern "C" void app_main(void)
{
    esp_err_t ret;
    sdmmc_card_t* card;

    ESP_LOGI("SD_TEST", "Initializing SD card...");

    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 4,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;  // 20 MHz safe mode

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.clk = (gpio_num_t)SDIO_SCLK_PIN;
    slot_config.cmd = (gpio_num_t)SDIO_CMD_PIN;
    slot_config.d0  = (gpio_num_t)SDIO_DATA0_PIN;
    slot_config.width = 1; // single-bit mode

    // Pull-ups
    gpio_set_pull_mode((gpio_num_t)SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);

    // Attempt to mount
    ret = esp_vfs_fat_sdmmc_mount(MOUNT_POINT, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE("SD_TEST", "Failed to mount SD card: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI("SD_TEST", "SD card mounted successfully at %s", MOUNT_POINT);
    sdmmc_card_print_info(stdout, card);

    // Test file write
    FILE* f = fopen(MOUNT_POINT "/test.txt", "w");
    if (f) {
        fprintf(f, "ESP32 SD card test OK\n");
        fclose(f);
        ESP_LOGI("SD_TEST", "File written successfully");
    } else {
        ESP_LOGE("SD_TEST", "Failed to write file");
    }

    // Clean up
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI("SD_TEST", "SD card unmounted");
}
