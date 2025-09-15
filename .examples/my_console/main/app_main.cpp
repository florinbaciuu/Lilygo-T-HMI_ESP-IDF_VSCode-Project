/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

extern "C"
{
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/soc_caps.h"
#include "cmd_system.h"
#include "cmd_wifi.h"
#include "cmd_nvs.h"
#include "console_settings.h"
}

// ========================================================================== //

#define PWR_EN (10)
#define PWR_ON (14)
#define SD_CS_PIN (15) // Chip Select pentru SPI
#define SD_MISO_PIN (13)
#define SD_MOSI_PIN (11)
#define SD_SCLK_PIN (12)
#define SDIO_DATA0_PIN (13)
#define SDIO_CMD_PIN (11)
#define SDIO_SCLK_PIN (12)

#define SD_FREQ_DEFAULT 20000   /*!< SD/MMC Default speed (limited by clock divider) */
#define SD_FREQ_HIGHSPEED 40000 /*!< SD High speed (limited by clock divider) */

#define CONFIG_CONSOLE_STORE_HISTORY 1      // Linenoise line editing library provides functions to save and load command history. If this option is enabled, initializes a FAT filesystem and uses it to store command history.
#define CONFIG_CONSOLE_IGNORE_EMPTY_LINES 1 // If an empty line is inserted to the console, the Console can eitherignore empty lines (the example would continue), or break on empty lines (the example would stop after an empty line).

static const char *TAG = "example";
#define PROMPT_STR CONFIG_IDF_TARGET

/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_CONSOLE_STORE_HISTORY

// #define MOUNT_PATH "/data"
#define MOUNT_PATH "/sdcard"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

// ========================================================================== //

static void initialize_filesystem(void)
{
    static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4,
        .allocation_unit_size = 0,
        .disk_status_check_enable = false,
        .use_one_fat = false};
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, "ffat", &mount_config, &wl_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
}
#else
#define HISTORY_PATH NULL
#endif // CONFIG_CONSOLE_STORE_HISTORY

// ========================================================================== //

static void initialize_filesystem_sdmmc(void)
{
    esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 4,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false};

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_PATH;

    // Configurare SDMMC host
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // Configurare pini slot SDMMC
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.clk = (gpio_num_t)SDIO_SCLK_PIN;
    slot_config.cmd = (gpio_num_t)SDIO_CMD_PIN;
    slot_config.d0 = (gpio_num_t)SDIO_DATA0_PIN;
    slot_config.width = 1; // 1-bit mode

    // (daca ai pull-up externi, poți comenta linia de mai jos)
    gpio_set_pull_mode((gpio_num_t)SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t)SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE("SD", "Failed to mount SDMMC (%s)", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI("SD", "SD card mounted at %s", mount_point);
    sdmmc_card_print_info(stdout, card);
}

// ========================================================================== //

static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

// ========================================================================== //

/************************************************************************************ */

/***
                            ███
                           ░░░
 █████████████    ██████   ████  ████████
░░███░░███░░███  ░░░░░███ ░░███ ░░███░░███
 ░███ ░███ ░███   ███████  ░███  ░███ ░███
 ░███ ░███ ░███  ███░░███  ░███  ░███ ░███
 █████░███ █████░░████████ █████ ████ █████
░░░░░ ░░░ ░░░░░  ░░░░░░░░ ░░░░░ ░░░░ ░░░░░
 */
extern "C"
{
    void app_main(void)
    {
        gpio_set_direction((gpio_num_t)PWR_EN, GPIO_MODE_OUTPUT);
        gpio_set_direction((gpio_num_t)PWR_ON, GPIO_MODE_OUTPUT);
        gpio_set_level((gpio_num_t)PWR_EN, 1);
        gpio_set_level((gpio_num_t)PWR_ON, 1);

        initialize_nvs();

#if CONFIG_CONSOLE_STORE_HISTORY
        // initialize_filesystem();
        initialize_filesystem_sdmmc();
        ESP_LOGI(TAG, "Command history enabled");
#else
        ESP_LOGI(TAG, "Command history disabled");
#endif

        /* Initialize console output periheral (UART, USB_OTG, USB_JTAG) */
        initialize_console_peripheral();

        /* Initialize linenoise library and esp_console*/
        initialize_console_library(HISTORY_PATH);

        // esp_console_set_help_verbose_level((esp_console_help_verbose_level_e)1);

        /* Prompt to be printed before each line.
         * This can be customized, made dynamic, etc.
         */
        const char *prompt = setup_prompt(PROMPT_STR ">");

        /* Register commands */
        esp_console_register_help_command();
        register_system_common();
#if SOC_LIGHT_SLEEP_SUPPORTED
        register_system_light_sleep();
#endif
#if SOC_DEEP_SLEEP_SUPPORTED
        register_system_deep_sleep();
#endif
#if (CONFIG_ESP_WIFI_ENABLED || CONFIG_ESP_HOST_WIFI_ENABLED)
        // register_wifi();
#endif
        // register_nvs();

        printf("\n"
               "This is an example of ESP-IDF console component.\n"
               "Type 'help' to get the list of commands.\n"
               "Use UP/DOWN arrows to navigate through command history.\n"
               "Press TAB when typing command name to auto-complete.\n"
               "Ctrl+C will terminate the console environment.\n");

        if (linenoiseIsDumbMode())
        {
            printf("\n"
                   "Your terminal application does not support escape sequences.\n"
                   "Line editing and history features are disabled.\n"
                   "On Windows, try using Putty instead.\n");
        }

        /* Main loop */
        while (true)
        {
            /* Get a line using linenoise.
             * The line is returned when ENTER is pressed.
             */
            char *line = linenoise(prompt);

#if CONFIG_CONSOLE_IGNORE_EMPTY_LINES
            if (line == NULL)
            { /* Ignore empty lines */
                continue;
                ;
            }
#else
            if (line == NULL)
            { /* Break on EOF or error */
                break;
            }
#endif // CONFIG_CONSOLE_IGNORE_EMPTY_LINES

            /* Add the command to the history if not empty*/
            if (strlen(line) > 0)
            {
                linenoiseHistoryAdd(line);
#if CONFIG_CONSOLE_STORE_HISTORY
                /* Save command history to filesystem */
                linenoiseHistorySave(HISTORY_PATH);
#endif // CONFIG_CONSOLE_STORE_HISTORY
            }

            /* Try to run the command */
            int ret;
            esp_err_t err = esp_console_run(line, &ret);
            if (err == ESP_ERR_NOT_FOUND)
            {
                printf("Unrecognized command\n");
            }
            else if (err == ESP_ERR_INVALID_ARG)
            {
                // command was empty
            }
            else if (err == ESP_OK && ret != ESP_OK)
            {
                printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
            }
            else if (err != ESP_OK)
            {
                printf("Internal error: %s\n", esp_err_to_name(err));
            }
            /* linenoise allocates line buffer on the heap, so need to free it */
            linenoiseFree(line);
        }

        ESP_LOGE(TAG, "Error or end-of-input, terminating console");
        esp_console_deinit();
    }
}

/************************************************************************************ */