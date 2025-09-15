



/**********************
 *   INCLUDES
 **********************/
#include "defines.h"

#include "LITTLE_fs.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_system.h"

#include "esp_console.h"
#include "esp_log.h"

#include "esp_littlefs.h"



/**********************
 *   FILESYSTEM
 **********************/


static const char* LITTLEFS_TAG = "LITTLEFS";

esp_err_t initialize_filesystem_littlefs() {
    ESP_LOGI(LITTLEFS_TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
        .base_path              = "/littlefs",
        .partition_label        = "littlefs",
        .format_if_mount_failed = true,
        .dont_mount             = false,
    };

    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(LITTLEFS_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(LITTLEFS_TAG, "Failed to find LittleFS partition");
        } else
        {
            ESP_LOGE(LITTLEFS_TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LITTLEFS_TAG,
            "Failed to get LittleFS partition information (%s)",
            esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else
    {
        ESP_LOGI(LITTLEFS_TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(LITTLEFS_TAG, "Opening file");
    FILE *f = fopen("/littlefs/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(LITTLEFS_TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "Hello World!\n");
    fclose(f);
    ESP_LOGI(LITTLEFS_TAG, "File written");

    // Rename original file
    // ESP_LOGI(TAG, "Renaming file");
    // if (rename("/littlefs/hello.txt", "/littlefs/foo.txt") != 0) {
    //     ESP_LOGE(TAG, "Rename failed");
    //     return;
    // }

    // Open renamed file for reading
    ESP_LOGI(LITTLEFS_TAG, "Reading file");
    f = fopen("/littlefs/hello.txt", "r");
    if (f == NULL) {
        ESP_LOGE(LITTLEFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }

    char line[128] = {0};
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strpbrk(line, "\r\n");
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(LITTLEFS_TAG, "Read from file: '%s'", line);

    ESP_LOGI(LITTLEFS_TAG, "Reading from flashed filesystem example.txt");
    f = fopen("/littlefs/example.txt", "r");
    if (f == NULL) {
        ESP_LOGE(LITTLEFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    pos = strpbrk(line, "\r\n");
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(LITTLEFS_TAG, "Read from file: '%s'", line);
    return ESP_OK;
}



