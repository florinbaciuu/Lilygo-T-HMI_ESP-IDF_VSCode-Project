
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

extern TaskHandle_t lvgl_task_handle;
extern TaskHandle_t touch_task_handle;

static const char *TAG = "STACK_MONITOR";

void stack_monitor_task(void *param)
{
    while (1) {
        UBaseType_t main_stack = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "Main stack free: %d bytes", main_stack * sizeof(StackType_t));

        if (lvgl_task_handle) {
            UBaseType_t lvgl_stack = uxTaskGetStackHighWaterMark(lvgl_task_handle);
            ESP_LOGI(TAG, "LVGL task stack free: %d bytes", lvgl_stack * sizeof(StackType_t));
        }

        if (touch_task_handle) {
            UBaseType_t touch_stack = uxTaskGetStackHighWaterMark(touch_task_handle);
            ESP_LOGI(TAG, "Touch task stack free: %d bytes", touch_stack * sizeof(StackType_t));
        }

        vTaskDelay(pdMS_TO_TICKS(3000)); // La fiecare 3 secunde
    }
}

void start_stack_monitor()
{
    xTaskCreate(stack_monitor_task, "stack_monitor", 4096, NULL, 1, NULL);
}


// ----------------------------------------------------------------------------------------

void check_esp_timer_stack() {
    TaskHandle_t timer_task = NULL;

    // Căutăm taskul după nume — implicit e "esp_timer"
    // (Poate varia în versiunile viitoare, dar în ESP-IDF v5.4 e sigur asta)
    UBaseType_t num_tasks = uxTaskGetNumberOfTasks();
    TaskStatus_t *task_status_array = (TaskStatus_t *)malloc(num_tasks * sizeof(TaskStatus_t));
    if (task_status_array == NULL) {
        ESP_LOGE("STACK", "Eșec malloc pentru task status array");
        return;
    }

    // Luăm toate taskurile
    num_tasks = uxTaskGetSystemState(task_status_array, num_tasks, NULL);
    for (int i = 0; i < num_tasks; i++) {
        if (strcmp(task_status_array[i].pcTaskName, "esp_timer") == 0) {
            timer_task = task_status_array[i].xHandle;
            break;
        }
    }

    if (timer_task) {
        UBaseType_t stack_left = uxTaskGetStackHighWaterMark(timer_task);
        ESP_LOGI("STACK", "esp_timer stack liber: %lu bytes (~%lu words)", stack_left * sizeof(StackType_t), stack_left);
    } else {
        ESP_LOGW("STACK", "Taskul esp_timer nu a fost găsit.");
    }

    free(task_status_array);
}