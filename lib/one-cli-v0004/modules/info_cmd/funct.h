#pragma once

#define USE_PRINTF 1
#define CONFIG_DEBUG_ESP_TIMERS 1

#pragma once

void printSysInfo(void);
void printFlashInfo(void);
void printCPUInfo(void);
void printInfoAboutMemory(void);
void print_esp_timers(void);
void printVersion();
void printInfoTasks();