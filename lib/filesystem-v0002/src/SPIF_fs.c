
#include "defines.h"
#include "SPIF_fs.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_system.h"

#include "esp_console.h"
#include "esp_log.h"

#include "esp_spiffs.h"



/**********************
 *   FILESYSTEM
 **********************/


static const char * SPIFFS_TAG = "SPIFFS";

esp_err_t initialize_filesystem_spiffs(){
    ESP_LOGI(SPIFFS_TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SPIFFS_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(SPIFFS_TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(SPIFFS_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    ESP_LOGI(SPIFFS_TAG, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFFS_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    } else {
        ESP_LOGI(SPIFFS_TAG, "SPIFFS_check() successful");
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFFS_TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return ESP_FAIL;
    } else {
        ESP_LOGI(SPIFFS_TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partition size info.
    if (used > total) {
        ESP_LOGW(SPIFFS_TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(SPIFFS_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return ESP_FAIL;
        } else {
            ESP_LOGI(SPIFFS_TAG, "SPIFFS_check() successful");
        }
    }

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(SPIFFS_TAG, "Opening file");
    FILE* f = fopen("/spiffs/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "Hello World!\n");
    fclose(f);
    ESP_LOGI(SPIFFS_TAG, "File written");

    // // Check if destination file exists before renaming
    // struct stat st;
    // if (stat("/spiffs/foo.txt", &st) == 0) {
    //     // Delete it if it exists
    //     unlink("/spiffs/foo.txt");
    // }

    // // Rename original file
    // ESP_LOGI(SPIFFS_TAG, "Renaming file");
    // if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0) {
    //     ESP_LOGE(SPIFFS_TAG, "Rename failed");
    //     return;
    // }

    // Open renamed file for reading
    ESP_LOGI(SPIFFS_TAG, "Reading file");
    f = fopen("/spiffs/hello.txt", "r");
    if (f == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(SPIFFS_TAG, "Read from file: '%s'", line);
    
    vTaskDelay(100);

    ESP_LOGI(SPIFFS_TAG, "Reading from flashed filesystem example.txt");
    f = fopen("/spiffs/example.txt", "r");
    if (f == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    pos = strpbrk(line, "\r\n");
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(SPIFFS_TAG, "Read from file: '%s'", line);

    return ESP_OK;
}











