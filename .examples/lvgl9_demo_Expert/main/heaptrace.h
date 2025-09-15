#include "esp_heap_trace.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

heap_trace_record_t trace_record[100]; // max 100 înregistrări

void start_heap_trace() {
    heap_trace_init_standalone(trace_record, 100);
    heap_trace_start(HEAP_TRACE_LEAKS);
}

void stop_heap_trace() {
    heap_trace_stop();
    heap_trace_dump(); // Asta va printa toate alocările active
}

void print_memory_info() {
    multi_heap_info_t info_psram;
    heap_caps_get_info(&info_psram, MALLOC_CAP_SPIRAM);

    ESP_LOGI("MEM", "🔍 PSRAM usage:");
    ESP_LOGI("MEM", "  - Total:     %d bytes", info_psram.total_size);
    ESP_LOGI("MEM", "  - Free:      %d bytes", info_psram.total_free_bytes);
    ESP_LOGI("MEM", "  - Allocated: %d bytes", info_psram.total_size - info_psram.total_free_bytes);
    ESP_LOGI("MEM", "  - Largest free block: %d bytes", info_psram.largest_free_block);
}
