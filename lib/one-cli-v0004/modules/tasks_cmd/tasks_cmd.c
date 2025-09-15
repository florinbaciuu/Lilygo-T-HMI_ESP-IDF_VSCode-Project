#include "tasks_cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include "esp_console.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_err.h"
#include <time.h>
#include "argtable3/argtable3.h"

static const char* TAG = "CLI";

#define TASK_MAX_COUNT 40
#define SECONDS_TO_MICROSECONDS(x) ((x) * 1000000)

const char* task_state[] = {"Running", "Ready", "Blocked", "Suspend", "Deleted", "Invalid"};

typedef struct
{
    uint32_t ulRunTimeCounter;
    uint32_t xTaskNumber;
} task_data_t;

static task_data_t previous_snapshot[TASK_MAX_COUNT];
static int         task_top_id   = 0;
static uint32_t    total_runtime = 0;

task_data_t* getPreviousTaskData(uint32_t xTaskNumber) {
    for (int i = 0; i < task_top_id; i++) {
        if (previous_snapshot[i].xTaskNumber == xTaskNumber) {
            return &previous_snapshot[i];
        }
    }  // Try to find the task in the list of tasks
    assert(task_top_id < TASK_MAX_COUNT);  // Allocate a new entry
    task_data_t* result = &previous_snapshot[task_top_id];
    result->xTaskNumber = xTaskNumber;
    task_top_id++;
    return result;
}

const char* getTimestamp() {
    static char timestamp[20];
    time_t      now = time(NULL);
    struct tm*  t   = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    return timestamp;
}

static int tasks_info() {
    ESP_LOGI(TAG, "-----------------Task Dump Start-----------------");
    printf("\n\r");
    uint32_t     totalRunTime              = 0;
    TaskStatus_t taskStats[TASK_MAX_COUNT] = {0};
    uint32_t     taskCount                 = uxTaskGetSystemState(taskStats, TASK_MAX_COUNT, &totalRunTime);
    assert(task_top_id < TASK_MAX_COUNT);
    uint32_t totalDelta = totalRunTime - total_runtime;
    float    f          = 100.0 / totalDelta;
    printf("%.4s\t%.6s\t%.8s\t%.8s\t%.4s\t%-20s\n",
        "Load",
        "Stack",
        "State",
        "CoreID",
        "PRIO",
        "Name");  // Format headers in a more visually appealing way
    for (uint32_t i = 0; i < taskCount; i++) {
        TaskStatus_t* stats            = &taskStats[i];
        task_data_t*  previousTaskData = getPreviousTaskData(stats->xTaskNumber);
        uint32_t      taskRunTime      = stats->ulRunTimeCounter;
        float         load             = f * (taskRunTime - previousTaskData->ulRunTimeCounter);

        char formattedTaskName[19];  // 16 caractere + 1 caracter pt terminatorul '\0' + 2 caractere
                                     // pt paranteze"[]"
        snprintf(formattedTaskName,
            sizeof(formattedTaskName),
            "[%-16s]",
            stats->pcTaskName);  // Format for the task's name with improved visibility
        char core_id_str[16];
        if (stats->xCoreID == -1 || stats->xCoreID == 2147483647) {
            snprintf(core_id_str, sizeof(core_id_str), "1/2");
        } else {
            snprintf(core_id_str, sizeof(core_id_str), "%d", stats->xCoreID);
        }  // Customize how core ID is displayed for better clarity
        printf("%.2f\t%" PRIu32 "\t%-4s\t%-4s\t%-4u\t%-19s\n",
            load,
            stats->usStackHighWaterMark,
            task_state[stats->eCurrentState],
            core_id_str,
            stats->uxBasePriority,
            formattedTaskName);  // Print formatted output
        previousTaskData->ulRunTimeCounter = taskRunTime;
    }
    total_runtime = totalRunTime;
    printf("\n\r");
    ESP_LOGI(TAG, "-----------------Task Dump End-------------------");
    return 0;
}

// ==========================================

#define SPIN_ITER 500000  // Actual CPU cycles used will depend on compiler optimization
#define SPIN_TASK_PRIO 2
#define STATS_TASK_PRIO 3
#define STATS_TICKS pdMS_TO_TICKS(1000)
#define ARRAY_SIZE_OFFSET 5  // Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE

/**
 * @brief   Function to print the CPU usage of tasks over a given duration.
 *
 * This function will measure and print the CPU usage of tasks over a specified
 * number of ticks (i.e. real time stats). This is implemented by simply calling
 * uxTaskGetSystemState() twice separated by a delay, then calculating the
 * differences of task run times before and after the delay.
 *
 * @note    If any tasks are added or removed during the delay, the stats of
 *          those tasks will not be printed.
 * @note    This function should be called from a high priority task to minimize
 *          inaccuracies with delays.
 * @note    When running in dual core mode, each core will correspond to 50% of
 *          the run time.
 *
 * @param   xTicksToWait    Period of stats measurement
 *
 * @return
 *  - ESP_OK                Success
 *  - ESP_ERR_NO_MEM        Insufficient memory to allocated internal arrays
 *  - ESP_ERR_INVALID_SIZE  Insufficient array size for uxTaskGetSystemState. Trying increasing
 * ARRAY_SIZE_OFFSET
 *  - ESP_ERR_INVALID_STATE Delay duration too short
 */
static esp_err_t print_real_time_stats(TickType_t xTicksToWait) {
    TaskStatus_t *              start_array = NULL, *end_array = NULL;
    UBaseType_t                 start_array_size, end_array_size;
    configRUN_TIME_COUNTER_TYPE start_run_time, end_run_time;
    esp_err_t                   ret;

    // Allocate array to store current task states
    start_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    start_array      = malloc(sizeof(TaskStatus_t) * start_array_size);
    if (start_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    // Get current task states
    start_array_size = uxTaskGetSystemState(start_array, start_array_size, &start_run_time);
    if (start_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    vTaskDelay(xTicksToWait);

    // Allocate array to store tasks states post delay
    end_array_size = uxTaskGetNumberOfTasks() + ARRAY_SIZE_OFFSET;
    end_array      = malloc(sizeof(TaskStatus_t) * end_array_size);
    if (end_array == NULL) {
        ret = ESP_ERR_NO_MEM;
        goto exit;
    }
    // Get post delay task states
    end_array_size = uxTaskGetSystemState(end_array, end_array_size, &end_run_time);
    if (end_array_size == 0) {
        ret = ESP_ERR_INVALID_SIZE;
        goto exit;
    }

    // Calculate total_elapsed_time in units of run time stats clock period.
    uint32_t total_elapsed_time = (end_run_time - start_run_time);
    if (total_elapsed_time == 0) {
        ret = ESP_ERR_INVALID_STATE;
        goto exit;
    }

    printf("| Task | Run Time | Percentage\n");
    // Match each task in start_array to those in the end_array
    for (int i = 0; i < start_array_size; i++) {
        int k = -1;
        for (int j = 0; j < end_array_size; j++) {
            if (start_array[i].xHandle == end_array[j].xHandle) {
                k = j;
                // Mark that task have been matched by overwriting their handles
                start_array[i].xHandle = NULL;
                end_array[j].xHandle   = NULL;
                break;
            }
        }
        // Check if matching task found
        if (k >= 0) {
            uint32_t task_elapsed_time = end_array[k].ulRunTimeCounter - start_array[i].ulRunTimeCounter;
            uint32_t percentage_time   = (task_elapsed_time * 100UL) / (total_elapsed_time * CONFIG_FREERTOS_NUMBER_OF_CORES);
            printf("| %s | %" PRIu32 " | %" PRIu32 "%%\n",
                start_array[i].pcTaskName,
                task_elapsed_time,
                percentage_time);
        }
    }

    // Print unmatched tasks
    for (int i = 0; i < start_array_size; i++) {
        if (start_array[i].xHandle != NULL) {
            printf("| %s | Deleted\n", start_array[i].pcTaskName);
        }
    }
    for (int i = 0; i < end_array_size; i++) {
        if (end_array[i].xHandle != NULL) {
            printf("| %s | Created\n", end_array[i].pcTaskName);
        }
    }
    ret = ESP_OK;

exit:  // Common return path
    free(start_array);
    free(end_array);
    return ret;
}

// ==========================================

// -------------------------------------------------------------

/***
 * Structura necesara functiei principale
 * Structura care contine alte 2 structuri
 */
static struct
{
    struct arg_str* subcommand;
    struct arg_lit* list;  // <-- opțiunea nouă
    struct arg_lit* help;  // ⬅️ NOU
    struct arg_end* end;
} tasks_args;

typedef void (*info_func_t)(void);
typedef struct
{
    const char* name;
    void (*function)(void);
    const char* description;  // nou!
} tasks_command_entry_t;

// -------------------------------------

void printTasksCommandList();

void printTasksInfo() {
    tasks_info();
}
void printTasksStats() {
    print_real_time_stats(1000);
}

// -------------------------------------

static const tasks_command_entry_t tasks_cmds[] = {
    {"info", printTasksInfo, "Display chip model, cores, and revision"},
    {"stats", printTasksStats, "Display chip model, cores, and revision"},
    {"--list", printTasksCommandList, "List all available subcommands"},
};

// -------------------------------------

void printTasksCommandList() {
    printf("╔═══════════════════════ AVAILABLE TASKS COMMANDS ══════════════════════════╗\n");
    printf("║ %-10s │ %-60s ║\n", "Command", "Description");
    printf("╟─────────┼─────────────────────────────────────────────────────────────────╢\n");
    for (size_t i = 0; i < sizeof(tasks_cmds) / sizeof(tasks_cmds[0]); ++i) {
        printf("║ %-10s │ %-60s ║\n",
            tasks_cmds[i].name,
            tasks_cmds[i].description ? tasks_cmds[i].description : "No description");
    }
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
}

// -------------------------------------

#define TASKS_CMD_COUNT (sizeof(tasks_cmds) / sizeof(tasks_cmds[0]))

static char tasks_cmds_help[128] = {0};

static void generate_tasks_cmds_help_text(void) {
    strcpy(tasks_cmds_help, ":   ");
    for (size_t i = 0; i < TASKS_CMD_COUNT; i++) {
        strcat(tasks_cmds_help, tasks_cmds[i].name);
        if (i < TASKS_CMD_COUNT - 1)
            strcat(tasks_cmds_help, "; ");
    }
}

// -------------------------------------

static int tasks_command(int argc, char** argv) {
    int nerrors = arg_parse(argc, argv, (void**) &tasks_args);

    // Dacă nu are niciun argument sau a cerut help global
    if (argc == 1 || tasks_args.help->count > 0) {
        printf("╔═══════════════════════════ TASKS COMMAND HELP ═════════════════════════════╗\n");
        printf("║ Usage: info <subcommand> [--help]                                          ║\n");
        printf("║                                                                            ║\n");
        printf("║ Available subcommands:                                                     ║\n");
        for (size_t i = 0; i < TASKS_CMD_COUNT; i++) {
            printf("║   %-10s - %-60s║\n", tasks_cmds[i].name, tasks_cmds[i].description);
        }
        printf("║                                                                            ║\n");
        printf("║ Use 'info <subcommand> --help' for more information.                       ║\n");
        printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
        return 0;
    }

    // Afișare listă dacă s-a cerut explicit
    if (tasks_args.list->count > 0) {
        printTasksCommandList();
        return 0;
    }

    if (nerrors != 0) {
        arg_print_errors(stderr, tasks_args.end, argv[0]);
        return 1;
    }

    // Verificare subcommand valid
    if (!tasks_args.subcommand || tasks_args.subcommand->count == 0 || !tasks_args.subcommand->sval[0]) {
        printf("No subcommand provided. Use `info --help`.\n");
        return 1;
    }

    const char* subcommand = tasks_args.subcommand->sval[0];
    size_t      num_cmds   = sizeof(tasks_cmds) / sizeof(tasks_cmds[0]);

    for (size_t i = 0; i < num_cmds; ++i) {
        if (strcmp(subcommand, tasks_cmds[i].name) == 0) {
            if (argc > 2 && (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "-h") == 0)) {
                printf("Help for '%s': %s\n", tasks_cmds[i].name, tasks_cmds[i].description);
                return 0;
            }
            tasks_cmds[i].function();
            return 0;
        }
    }

    printf("Unknown subcommand: %s\n", subcommand);
    printf("Type `info --list` to see available subcommands.\n");
    return 1;
}

// -------------------------------------

void cli_register_tasks_command(void) {
    generate_tasks_cmds_help_text();
    tasks_args.subcommand       = arg_str1(NULL,  // nu are flag scurt, gen `-s
        NULL,                               // nu are flag lung, gen `--subcmd`
        "<subcommand>",                     // numele argumentului (pentru help/usage)
        tasks_cmds_help);                   // descrierea lui
    tasks_args.list             = arg_lit0("l", "list", "List all available subcommands");
    tasks_args.help             = arg_lit0("h", "help", "Show help for 'info' command");
    tasks_args.end              = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command  = "tasks",
        .help     = "Tasks",
        .hint     = NULL,
        .func     = &tasks_command,
        .argtable = &tasks_args,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI(TAG, "'%s' command registered.", cmd.command);
    return;
}