

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"
#include "esp_rom_uart.h"
}

/* HELP */
//! need CONFIG_FREERTOS_CHECK_STACKOVERFLOW_CANARY=y
//! need CONFIG_FREERTOS_WATCHPOINT_END_OF_STACK=y

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    esp_rom_printf("💀 Stack overflow în task: %s\n", pcTaskName);
    abort();
}

//----

extern "C" void vApplicationMallocFailedHook(void) 
{
    esp_rom_printf("💀💥 Malloc failed!");
    abort();
}

//----

extern "C" void vApplicationIdleHook(void) {
  // Codul tău aici sau lasă gol
  // ex: __asm__("nop");
  // for (int i = 0; i < 100; i++) {
  //       asm volatile("nop");
  //   }
}
//---------
extern "C" void vApplicationTickHook(void) {
  // Codul tău aici sau lasă gol
  // ex: __asm__("nop");
  // for (int i = 0; i < 100; i++) {
  //       asm volatile("nop");
  //   }
}