#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "esp_timer.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "perfmon.h"

#include "perfmon_cmd.h"

static const char* TAG = "CLI";

static void exec_test_function(void* params) {
    // for (int i = 0 ; i < 100 ; i++) {
    //     __asm__ __volatile__("   nop");
    // }

    // sau

    volatile int sum = 0;
    for (int i = 0; i < 100; i++)
    {
        sum += i * 3;
    }
}

// Table with dedicated performance counters
static uint32_t pm_check_table[] = {
    XTPERF_CNT_CYCLES,
    XTPERF_MASK_CYCLES, // total cycles
    XTPERF_CNT_INSN,
    XTPERF_MASK_INSN_ALL, // total instructions
    XTPERF_CNT_D_LOAD_U1,
    XTPERF_MASK_D_LOAD_LOCAL_MEM, // Mem read
    XTPERF_CNT_D_STORE_U1,
    XTPERF_MASK_D_STORE_LOCAL_MEM, // Mem write
    XTPERF_CNT_BUBBLES,
    XTPERF_MASK_BUBBLES_ALL & (~XTPERF_MASK_BUBBLES_R_HOLD_REG_DEP), // wait for other reasons
    XTPERF_CNT_BUBBLES,
    XTPERF_MASK_BUBBLES_R_HOLD_REG_DEP, // Wait for register dependency
    XTPERF_CNT_OVERFLOW,
    XTPERF_MASK_OVERFLOW, // Last test cycle
};

#define TOTAL_CALL_AMOUNT 200 //
#define PERFMON_TRACELEVEL -1 // -1 - will ignore trace level

bool app_main_perfmon(void) {
    ESP_LOGI(TAG, "Start");
    ESP_LOGI(TAG, "Start test with printing all available statistic");
    xtensa_perfmon_config_t pm_config = {};
    pm_config.counters_size   = sizeof(xtensa_perfmon_select_mask_all) / sizeof(uint32_t) / 2;
    pm_config.select_mask     = xtensa_perfmon_select_mask_all;
    pm_config.repeat_count    = TOTAL_CALL_AMOUNT;
    pm_config.max_deviation   = 1;
    pm_config.call_function   = exec_test_function;
    pm_config.callback        = xtensa_perfmon_view_cb;
    pm_config.callback_params = stdout;
    pm_config.tracelevel      = PERFMON_TRACELEVEL;
    xtensa_perfmon_exec(&pm_config);

    ESP_LOGI(TAG, "Start test with user defined statistic");
    pm_config.counters_size   = sizeof(pm_check_table) / sizeof(uint32_t) / 2;
    pm_config.select_mask     = pm_check_table;
    pm_config.repeat_count    = TOTAL_CALL_AMOUNT;
    pm_config.max_deviation   = 1;
    pm_config.call_function   = exec_test_function;
    pm_config.callback        = xtensa_perfmon_view_cb;
    pm_config.callback_params = stdout;
    pm_config.tracelevel      = PERFMON_TRACELEVEL;

    xtensa_perfmon_exec(&pm_config);

    ESP_LOGI(TAG, "The End");
    return 1;
}

static int perfmon_command(int argc, char** argv) {
    app_main_perfmon();
    return 0;
}

static void register_perfmon(void) {
    const esp_console_cmd_t cmd = {
        .command = "perfmon",
        .help    = "Performance Monitor",
        .hint    = NULL,
        .func    = &perfmon_command,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI(TAG, "'%s' command registered.", cmd.command);
}

void cli_register_perfmon_command(void) {
    register_perfmon();
}
