

#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "STACK";

void print_task_stack_info(void) {
    UBaseType_t   numTasks        = uxTaskGetNumberOfTasks();
    TaskStatus_t* taskStatusArray = malloc(numTasks * sizeof(TaskStatus_t));

    if (!taskStatusArray) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        return;
    }
    // clang-format off
    UBaseType_t arraySize = uxTaskGetSystemState(taskStatusArray,
                                                       numTasks,
                                                   NULL);
    // clang-format on
    printf("╔═════════════════════════════════════════════════════╗\n");
    printf("║ %-20s │ %-12s │ %-5s │ %-5s ║\n", "Task", "High Water", "State", "Prio");
    printf("╟──────────────────────┼──────────────┼───────┼───────╢\n");
    for (UBaseType_t i = 0; i < arraySize; i++) {
        printf("║ %-20s │ %-12lu │ %-5d │ %-5d ║\n",
            taskStatusArray[i].pcTaskName,
            (unsigned long) (taskStatusArray[i].usStackHighWaterMark * sizeof(StackType_t)),
            taskStatusArray[i].eCurrentState,
            taskStatusArray[i].uxCurrentPriority);
    }
    printf("╚═════════════════════════════════════════════════════╝\n");
    free(taskStatusArray);
}