/**
 * @file : app_main.cpp.
 * @author : baf
 * board : t-hmi
 */

// test

/*********************
 *      DEFINES
 *********************/
#define PWR_EN_PIN (10) // connected to the battery alone
#define PWR_ON_PIN (14)
#define Dellp_OFF_PIN (21) // ? IDK what it is ???
//---------
//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_bootloader_desc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_console.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "soc/soc_caps.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

// my includes (from lib folder)
#include "one-cli.h"
#include "filesystem-os.h"
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// ----------------------------------------------------------
// ----------------------------------------------------------

/**********************
 *   CLI History
 **********************/
/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 TODO AICI TREBUIE SA PUN DETECTIE DACA S-A MAI CHEMAT INITIALIZAREA CATRE SDMMC CA SA POT SA FAC
 INITIALIZARE IN ORICE MOMENT.. ISTORIA TREBUIE
 TODO DOAR SA STIE DACA S-A INITIALIZAT SI APOI SA ISI FACA TREABA, ATAT.. ORICUM SUNT APROAPE DE
 ASTA DAR SA NU UITI!!!!
 */
void init_cli_filesystem_history() {
#if CONFIG_CONSOLE_STORE_HISTORY
    esp_err_t history_fs_ok = ESP_OK; // FLAG
#ifdef SDCARD_USE
    history_fs_ok = initialize_filesystem_sdmmc(); // call the init function
    if (history_fs_ok == ESP_OK)
    {
        ESP_LOGI("File System", "File system enabled on SDCARD");
    } else
    {
        ESP_LOGW("File System",
            "Failed to enable file system on SDCARD (%s)",
            esp_err_to_name(history_fs_ok));
    }
#else  /*  !(SDCARD_USE) */
    history_fs_ok = initialize_internal_fat_filesystem(); // call the init function
    if (history_fs_ok == ESP_OK)
    {
        ESP_LOGI("File System", "File system enabled on internal FAT partition.");
    } else
    {
        ESP_LOGW("File System",
            "Failed to enable file system on FFAT (%s)",
            esp_err_to_name(history_fs_ok));
    }
#endif /* (SDCARD_USE) */
    if (history_fs_ok == ESP_OK)
    {
        cli_set_history_path(MOUNT_PATH "/history.txt"); // set path
        ESP_LOGI("CLI", "Command history enabled on " MOUNT_PATH);
    } else
    {
        ESP_LOGW("CLI", "⚠️ Filesystem not mounted, disabling command history");
        cli_set_history_path(NULL); // fără history
    }
#else  /* !(CONFIG_CONSOLE_STORE_HISTORY)  */
    ESP_LOGI("CONSOLE", "Command history disabled");
    //// #define HISTORY_PATH NULL
#endif /* if (CONFIG_CONSOLE_STORE_HISTORY) */
}

static void initialize_nvs(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

// ----------------------------------------------------------
/*
 TODO SI AICI TREBUIE SA FAC CEVA CU EEPROOM SA MA GANDESC DACA IL BAG IN FILESYSTEM SAU II FAC
 COMPONENTA. MA MAI GANDESC
 */
esp_err_t initialize_eeproom() {
    ESP_LOGI("eeproom", "🔧 Initializing NVS partition 'eeproom'...");
    esp_err_t err = nvs_flash_init_partition("eeproom");
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW("eeproom", "⚠️ NVS partition is full or version mismatch. Erasing...");
        err = nvs_flash_erase_partition("eeproom");
        if (err != ESP_OK)
        {
            ESP_LOGE("eeproom", "❌ Failed to erase 'eeproom' partition: %s", esp_err_to_name(err));
            return err;
        }
        err = nvs_flash_init_partition("eeproom");
        if (err != ESP_OK)
        {
            ESP_LOGE("eeproom",
                "❌ Failed to re-initialize 'eeproom' after erase: %s",
                esp_err_to_name(err));
            return err;
        }
    }
    if (err == ESP_OK)
    {
        ESP_LOGI("eeproom", "✅ NVS partition 'eeproom' initialized successfully");
    } else
    {
        ESP_LOGE("eeproom", "❌ Failed to initialize NVS: %s", esp_err_to_name(err));
        return err;
    }
    nvs_handle_t handle; // Deschidem un handle ca să verificăm spațiul
    err = nvs_open_from_partition("eeproom", "diagnostic", NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("eeproom", "❌ Can't open NVS handle for diagnostics: %s", esp_err_to_name(err));
        return err;
    }
    nvs_close(handle);
    return ESP_OK;
}

//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
void power_latch_init_5V(uint32_t mode) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWR_EN_PIN) | (1ULL << PWR_ON_PIN), // Setăm ambii pini ca output
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) PWR_EN_PIN, mode);
    gpio_set_level((gpio_num_t) PWR_ON_PIN, mode); // nu e nevoie de el daca alimentam usb
}
//---------
void power_latch_init_Battery() {
    gpio_config_t io_conf = {.pin_bit_mask = 1ULL << PWR_EN_PIN,
        .mode                              = GPIO_MODE_OUTPUT,
        .pull_up_en                        = GPIO_PULLUP_DISABLE,
        .pull_down_en                      = GPIO_PULLDOWN_DISABLE,
        .intr_type                         = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) PWR_EN_PIN, 1); // ⚡ ține placa aprinsă
}
//---------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
'// ⚡
*/

/*********************
 *  ...
 *********************/
//---------
/****************************/
//--------------------------------------

/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ 
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  ! This is the main entry point of the application.
  ! The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(150));
    power_latch_init_5V(true);
    vTaskDelay(pdMS_TO_TICKS(150));
    esp_log_level_set("*", ESP_LOG_INFO);
    vTaskDelay(pdMS_TO_TICKS(150));
    esp_bootloader_desc_t bootloader_desc;
    ESP_LOGI("Bootloader", "Details about bootloader:\n");
    esp_rom_printf("\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
    esp_rom_printf("\tESP-IDF version from app: %s\n", IDF_VER);
    vTaskDelay(pdMS_TO_TICKS(150));
    //// initialize_nvs();
    //// ESP_ERROR_CHECK(initialize_eeproom());
    heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
    vTaskDelay(pdMS_TO_TICKS(150));
    initialize_internal_fat_filesystem();
    vTaskDelay(pdMS_TO_TICKS(250));
    initialize_filesystem_littlefs(); // aici e aici sa vedem ce se intampla
    vTaskDelay(pdMS_TO_TICKS(250));
    initialize_filesystem_spiffs();
    vTaskDelay(pdMS_TO_TICKS(250));
    init_cli_filesystem_history();
    vTaskDelay(pdMS_TO_TICKS(250));
    StartCLI();
    vTaskDelay(pdMS_TO_TICKS(250));
    // Aici app main trebuie sa dea return.
} // app_main
/********************************************** */
/**********************
 *   END OF FILE
 **********************/
