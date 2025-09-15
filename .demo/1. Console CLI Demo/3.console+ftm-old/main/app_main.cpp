/**
 * @file      app_main.cpp.
 * @author    Baciu Aurel Florin
 * @brief     Main application file for the Lilygo T HMI development board.
 * @brief     Demo
 * @license   MIT
 * @copyright Copyright (c) 2025 Baciu Aurel Florin
 * @copyright Oltean East
 * @version   1.0.0
 *
 * @date      10 July 2025
 * @time      20:40 (PM)
 * @computer  Mac Mini M4
 * ..
 * @details   This file contains the main application logic, including initialization of hardware
 * components,
 */

/*********************
 *      DEFINES
 *********************/
#define BOARD_TFT_DATA0 (48) // GPIO pin for TFT data line 0
#define BOARD_TFT_DATA1 (47) // GPIO pin for TFT data line 1
#define BOARD_TFT_DATA2 (39) // GPIO pin for TFT data line 2
#define BOARD_TFT_DATA3 (40) // GPIO pin for TFT data line 3
#define BOARD_TFT_DATA4 (41) // GPIO pin for TFT data line 4
#define BOARD_TFT_DATA5 (42) // GPIO pin for TFT data line 5
#define BOARD_TFT_DATA6 (45) // GPIO pin for TFT data line 6
#define BOARD_TFT_DATA7 (46) // GPIO pin for TFT data line 7
#define BOARD_TFT_RST (-1)   // GPIO pin for TFT reset, set to -1 if not used
#define BOARD_TFT_CS (6)     // GPIO pin for TFT chip select
#define BOARD_TFT_DC (7)     // GPIO pin for TFT data/command control
#define BOARD_TFT_WR (8)     // GPIO pin for TFT write control
#define LCD_WIDTH (320)      // Width of the LCD in pixels
#define LCD_HEIGHT (240)     // Height of the LCD in pixels
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000) // LCD pixel clock frequency in Hz
#define BOARD_TFT_BL (38)                             // GPIO pin for backlight control
#define PWR_EN_PIN (10)                               // connected to the battery alone
//---------
#define PWR_ON_PIN                                                                                 \
    (14) // if you use an ext 5V power supply, you need to bring a magnet close to the ReedSwitch
         // and set the PowerOn Pin (GPIO14) to HIGH
#define Dellp_OFF_PIN                                                                              \
    (21) // connected to the battery and the USB power supply, used to turn off the device
//---------
#define PIN_NUM_IRQ (9)  // IRQ pin for touch controller
#define BAT_ADC_PIN (5)  // ADC pin for battery voltage measurement
#define PIN_NUM_MISO (4) // MISO pin for touch controller
#define PIN_NUM_MOSI (3) // MOSI pin for touch controller
#define PIN_NUM_CLK (1)  // CLK pin for touch controller
#define PIN_NUM_CS (2)   // CS pin for touch controller

/*********************
 *    LVGL DEFINES
 *********************/
/* LVGL TASK NOTIFICATION */
#define USE_MUTEX 0
#define USE_FREERTOS_TASK_NOTIF 1  // cica e mai rapid cu 20 %
#define LV_TASK_NOTIFY_SIGNAL 0x01 // Semnalul pentru notificarea LVGL
////#define LV_TASK_NOTIFY_SIGNAL_MODE USE_MUTEX
#define LV_TASK_NOTIFY_SIGNAL_MODE (USE_FREERTOS_TASK_NOTIF)
//---------
/* BUFFER MODE */
#define BUFFER_20LINES 0
#define BUFFER_40LINES 1
#define BUFFER_60LINES 2 // merge
#define BUFFER_DEVIDED4 3
#define BUFFER_FULL 4             // merge super ok
#define BUFFER_MODE (BUFFER_FULL) // selecteaza modul de buffer , defaut este BUFFER_FULL
#define DOUBLE_BUFFER_MODE (true)
//---------
/* BUFFER MEMORY TYPE AND DMA */
#define BUFFER_INTERNAL 0
#define BUFFER_SPIRAM 1
#define BUFFER_MEM (BUFFER_SPIRAM)
#if (BUFFER_MEM == BUFFER_INTERNAL)
#define DMA_ON (true)
#endif
//---------
/* RENDER MODE */
#define RENDER_MODE_PARTIAL 0 // Modul recomandat pt dual buffer and no canvas and no direct mode
#define RENDER_MODE_FULL 1    //
#define RENDER_MODE (RENDER_MODE_PARTIAL) // selecteaza modul de randare
//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include "esp_bootloader_desc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define ESP_LCD_IO_I2C_SKIP_INCLUDE 1
#include "command_line_interface.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "command_line_interface.h"
}

#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// ----------------------------------------------------------

/**
 * Prototypes for functions
 */

// ----------------------------------------------------------

// ----------------------------------------------------------

/**********************
 *   GLOBAL VARIABLES
 **********************/
RTC_DATA_ATTR static uint32_t boot_count = 0;
//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
void gpio_extra_set_init(uint32_t mode) {
    // Setăm ambii pini ca output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWR_EN_PIN) | (1ULL << PWR_ON_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)PWR_EN_PIN, mode);
    gpio_set_level((gpio_num_t)PWR_ON_PIN, mode); // nu e nevoie de el daca alimentam usb
}
//---------
void power_latch_init() {
    gpio_config_t io_conf = {.pin_bit_mask = 1ULL << PWR_EN_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)PWR_EN_PIN, 1); // ⚡ ține placa aprinsă
}
//---------
void gfx_set_backlight(uint32_t mode) {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BOARD_TFT_BL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t)BOARD_TFT_BL, mode);
}

/****************************/

//--------------------------------------

/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ 
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  * This is the main entry point of the application.
  * It initializes the hardware, sets up the display, and starts the LVGL tasks.
  * The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
    power_latch_init(); // Inițializare latch pentru alimentare
    gfx_set_backlight(1);
    esp_log_level_set("*", ESP_LOG_INFO);

    esp_bootloader_desc_t bootloader_desc;
    printf("\n");
    ESP_LOGI("Bootloader description", "\tESP-IDF version from 2nd stage bootloader: %s\n",
        bootloader_desc.idf_ver);
    ESP_LOGI("Bootloader description", "\tESP-IDF version from app: %s\n", IDF_VER);


    // Use the ftm component
    // the ftm component use {} for the parameter enter point
    fmt::print("Hello, world from <ftm> Component!\n");
    std::string string1 = fmt::format("The nr is {}.\n", 1);
    fmt::print("{}", string1);
    std::string string2 = fmt::format("I'd rather be {1} than {0}.\n", "right", "happy");
    fmt::print("{}\n", string2);
    std::string string3 = fmt::format("{1}{0}\n", "Hello", "World");
    fmt::print("{}\n", string3);
    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "Hello, {}!\n", "world");
    fmt::print(fg(fmt::color::floral_white) | bg(fmt::color::slate_gray) | fmt::emphasis::underline,"Olá, {}!\n", "Mundo");
    fmt::print(fg(fmt::color::steel_blue) | fmt::emphasis::italic, "Hello{}！\n", "World");

    StartCLI();
} // app_main

/********************************************** */

/**********************
 *   END OF FILE
 **********************/
