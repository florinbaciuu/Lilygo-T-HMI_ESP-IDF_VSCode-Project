/*
 * SPDX-FileCopyrightText: 2025 baciuaurelflorin@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "task_monitor.h"
#include <time.h>

// -------------------------------------------------------------------------

typedef struct {
  uint32_t ulRunTimeCounter;
  uint32_t xTaskNumber;
} task_data_t;

// -------------------------------------------------------------------------

#define M2T(X)         ((unsigned int)(X) / portTICK_PERIOD_MS)
#define TASK_MAX_COUNT CONFIG_TASKMONITOR_MAX_TASKS
#define DUMP_INTERVAL  (CONFIG_TASKMONITOR_DUMP_INTERVAL * 1000)

// -------------------------------------------------------------------------

const static char *TAG = "TASK MON";

// -------------------------------------------------------------------------

static task_data_t previous_snapshot[TASK_MAX_COUNT];
static int task_top_id = 0;
static uint32_t total_runtime = 0;

// -------------------------------------------------------------------------

const char *task_state[] = {"Running", "Ready", "Blocked", "Suspend", "Deleted", "Invalid"};
const char *task_state_ro[] = {"Ruleaza", "Gata", "Blocat", "Suspendat", "Sters", "Invalid"};

// -------------------------------------------------------------------------

/**
  * @brief Function to get the previous task data
  * @param xTaskNumber Task number
  * @return task_data_t* Pointer to the task data
  * @note This function is used to get the previous task data
  * @note If the task is not found, a new entry is allocated
  * @note The function is used to calculate the task load
  */
task_data_t *getPreviousTaskData(uint32_t xTaskNumber) {
  for (int i = 0; i < task_top_id; i++) {
    if (previous_snapshot[i].xTaskNumber == xTaskNumber) {
      return &previous_snapshot[i];
    }
  }  // Try to find the task in the list of tasks
  assert(task_top_id < TASK_MAX_COUNT);  // Allocate a new entry
  task_data_t *result = &previous_snapshot[task_top_id];
  result->xTaskNumber = xTaskNumber;
  task_top_id++;
  return result;
}

// -------------------------------------------------------------------------

/**
  * @brief Function to monitor the tasks
  * @param xTimer Timer handle
  * @note This function is called by the timer
  * @note It calls the task_monitor_cb_() function
  */
#if (TIME_STAMP_ON == 1)
const char *getTimestamp() {
  static char timestamp[20];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
  return timestamp;
}
#endif /* #if (TIME_STAMP_ON == 1) */

// -------------------------------------------------------------------------

/***
 * @brief Function to print the memory information
 * @note This function is used to calculates and print the memory information.
 * It prints the total PSRAM, free PSRAM, used PSRAM, total heap, free heap,
 * used heap, total RAM, free RAM. This function is called by the task_monitor_cb_() function
 */
void printMemoryInfo() {
  // Obținerea informațiilor despre memorie
  size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
  size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  size_t used_psram = total_psram - free_psram;
  size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
  size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  size_t used_heap = total_heap - free_heap;
  size_t total_ram = total_psram + total_heap;  // Suma PSRAM și heap
  size_t free_ram = free_psram + free_heap;     // Suma memoriei libere
  ESP_LOGI("MemoryInfo", "Total PSRAM: %zu bytes \t Free PSRAM: %zu bytes \t Used PSRAM: %zu bytes", total_psram, free_psram, used_psram);
  ESP_LOGI("MemoryInfo", "Total Heap: %zu bytes  \t Free Heap: %zu bytes  \t Used Heap: %zu bytes", total_heap, free_heap, used_heap);
  ESP_LOGI("MemoryInfo", "Total RAM: %zu bytes   \t Free RAM: %zu bytes", total_ram, free_ram);
}

// -------------------------------------------------------------------------

/***
  * task_monitor_cb_()
  * @note Function to print the task information
  * @note This function is called by the task_monitor_cb() function. It prints the task information, calculates the task load,
  * prints the total free heap, prints the total run time, prints the total run time delta, prints the total run time delta in percentage
  * and prints the task information.
  */
void task_monitor_cb_() {
  uint32_t totalRunTime = 0;
  TaskStatus_t taskStats[TASK_MAX_COUNT] = {0};
  uint32_t taskCount = uxTaskGetSystemState(taskStats, TASK_MAX_COUNT, &totalRunTime);
  assert(task_top_id < TASK_MAX_COUNT);
  uint32_t totalDelta = totalRunTime - total_runtime;
  float f = 100.0 / totalDelta;
  ESP_LOGI(TAG, "-----------------Task Dump Start-----------------");

#if (TIME_STAMP_ON == 1)
  const char *timestamp = getTimestamp();  // Get current timestamp and print it at the start of the task dump
  ESP_LOGI(TAG, "Timestamp: [%s]\n", timestamp);
#endif /* #if TIME_STAMP_ON = 1 */

  printf("%.4s\t%.6s\t%.8s\t%.8s\t%.4s\t%-20s\n", "Load", "Stack", "State", "CoreID", "PRIO", "Name");  // Format headers in a more visually appealing way
  for (uint32_t i = 0; i < taskCount; i++) {
    TaskStatus_t *stats = &taskStats[i];
    task_data_t *previousTaskData = getPreviousTaskData(stats->xTaskNumber);
    uint32_t taskRunTime = stats->ulRunTimeCounter;
    float load = f * (taskRunTime - previousTaskData->ulRunTimeCounter);

    char formattedTaskName[19];  // 16 caractere + 1 caracter pt terminatorul '\0' + 2 caractere pt paranteze"[]"
    snprintf(formattedTaskName, sizeof(formattedTaskName), "[%-16s]", stats->pcTaskName);  // Format for the task's name with improved visibility
    char core_id_str[16];
    if (stats->xCoreID == -1 || stats->xCoreID == 2147483647) {
      snprintf(core_id_str, sizeof(core_id_str), "1/2");
    } else {
      snprintf(core_id_str, sizeof(core_id_str), "%d", stats->xCoreID);
    }  // Customize how core ID is displayed for better clarity
    printf(
      "%.2f\t%" PRIu32 "\t%-4s\t%-4s\t%-4u\t%-19s\n", load, stats->usStackHighWaterMark, task_state[stats->eCurrentState], core_id_str, stats->uxBasePriority,
      formattedTaskName
    );  // Print formatted output
    previousTaskData->ulRunTimeCounter = taskRunTime;
  }
  printf("\n\r");
#if MEMORY_DEBUG == 1
  printMemoryInfo();
#endif /* #if MEMORY_DEBUG == 1 */

  ESP_LOGI(TAG, "-----------------Task Dump End-------------------");
  printf("\n\r");
  total_runtime = totalRunTime;
}

// -------------------------------------------------------------------------

/**
 * @brief Function to monitor the tasks
 * @note This function is called by the APP
 */
void task_monitor_cb() {
  task_monitor_cb_();
}

// -------------------------------------------------------------------------
