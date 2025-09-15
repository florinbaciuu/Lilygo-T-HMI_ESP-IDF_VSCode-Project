#include "esp_heap_caps.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "LV_MEM";

// Inițializare LVGL mem – nu e nevoie în ESP-IDF
void lv_mem_init(void)
{
    ESP_LOGI(TAG, "LVGL memory init: PSRAM & Internal heap");
    return;
}

void lv_mem_deinit(void)
{
    ESP_LOGI(TAG, "LVGL memory deinit");
    return;
}

// Alocare în PSRAM (dacă e disponibil)
void *lv_malloc_core(size_t size) {
    void *ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (ptr == NULL) {
        ESP_LOGW(TAG, "PSRAM malloc failed for size: %zu. Trying INTERNAL.", size);
        ptr = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (ptr == NULL) {
            ESP_LOGE(TAG, "Internal malloc also failed for size: %zu", size);
        }
    }
    return ptr;
}

// Realocare în PSRAM sau fallback pe heap intern
void *lv_realloc_core(void *p, size_t new_size) {
    void *new_ptr = heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (new_ptr == NULL) {
        ESP_LOGW(TAG, "PSRAM realloc failed. Trying INTERNAL for size: %zu", new_size);
        new_ptr = heap_caps_realloc(p, new_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (new_ptr == NULL) {
            ESP_LOGE(TAG, "Internal realloc also failed for size: %zu", new_size);
        }
    }
    return new_ptr;
}

// Eliberare memorie (nu contează sursa)
void lv_free_core(void *p) {
    if (p != NULL) {
        heap_caps_free(p);
    }
}