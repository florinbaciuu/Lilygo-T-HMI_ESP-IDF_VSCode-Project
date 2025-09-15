/**
 * @file      app_main.cpp
 * @author    Baciu Aurel Florin
 * @brief     Main application file for the Lilygo T HMI development board.
 * @brief     Demo
 * @license   MIT
 * @copyright Copyright (c) 2025 Baciu Aurel Florin
 * @date      2025-08-07
 * @time      22:19 (PM)
 * ..
 */

/*********************
 *      DEFINES
 *********************/
#define BOARD_TFT_DATA0            (48)                // GPIO pin for TFT data line 0
#define BOARD_TFT_DATA1            (47)                // GPIO pin for TFT data line 1
#define BOARD_TFT_DATA2            (39)                // GPIO pin for TFT data line 2
#define BOARD_TFT_DATA3            (40)                // GPIO pin for TFT data line 3
#define BOARD_TFT_DATA4            (41)                // GPIO pin for TFT data line 4
#define BOARD_TFT_DATA5            (42)                // GPIO pin for TFT data line 5
#define BOARD_TFT_DATA6            (45)                // GPIO pin for TFT data line 6
#define BOARD_TFT_DATA7            (46)                // GPIO pin for TFT data line 7
#define BOARD_TFT_RST              (-1)                // GPIO pin for TFT reset, set to -1 if not used
#define BOARD_TFT_CS               (6)                 // GPIO pin for TFT chip select
#define BOARD_TFT_DC               (7)                 // GPIO pin for TFT data/command control
#define BOARD_TFT_WR               (8)                 // GPIO pin for TFT write control
#define LCD_WIDTH                  (320)               // Width of the LCD in pixels
#define LCD_HEIGHT                 (240)               // Height of the LCD in pixels
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000)  // LCD pixel clock frequency in Hz
#define BOARD_TFT_BL               (38)                // GPIO pin for backlight control
#define PWR_EN_PIN                 (10)                // connected to the battery alone
//---------
#define PWR_ON_PIN    (14)  // if you use an ext 5V power supply, you need to bring a magnet close to the ReedSwitch and set the PowerOn Pin (GPIO14) to HIGH
#define Dellp_OFF_PIN (21)  // connected to the battery and the USB power supply, used to turn off the device
//---------
#define PIN_NUM_IRQ  (9)  // IRQ pin for touch controller
#define BAT_ADC_PIN  (5)  // ADC pin for battery voltage measurement
#define PIN_NUM_MISO (4)  // MISO pin for touch controller
#define PIN_NUM_MOSI (3)  // MOSI pin for touch controller
#define PIN_NUM_CLK  (1)  // CLK pin for touch controller
#define PIN_NUM_CS   (2)  // CS pin for touch controller

/*********************
 *    LVGL DEFINES
 *********************/
/* LVGL TASK NOTIFICATION */
#define USE_MUTEX               0
#define USE_FREERTOS_TASK_NOTIF 1     // cica e mai rapid cu 20 %
#define LV_TASK_NOTIFY_SIGNAL   0x01  // Semnalul pentru notificarea LVGL
////#define LV_TASK_NOTIFY_SIGNAL_MODE USE_MUTEX
#define LV_TASK_NOTIFY_SIGNAL_MODE (USE_FREERTOS_TASK_NOTIF)
//---------
/* BUFFER MODE */
#define BUFFER_20LINES     0
#define BUFFER_40LINES     1
#define BUFFER_60LINES     2  // merge
#define BUFFER_DEVIDED4    3
#define BUFFER_FULL        4              // merge super ok
#define BUFFER_MODE        (BUFFER_FULL)  // selecteaza modul de buffer , defaut este BUFFER_FULL
#define DOUBLE_BUFFER_MODE (true)
//---------
/* BUFFER MEMORY TYPE AND DMA */
#define BUFFER_INTERNAL 0
#define BUFFER_SPIRAM   1
#define BUFFER_MEM      (BUFFER_SPIRAM)
#if (BUFFER_MEM == BUFFER_INTERNAL)
#define DMA_ON (true)
#endif
//---------
/* RENDER MODE */
#define RENDER_MODE_PARTIAL 0                      // Modul recomandat pt dual buffer and no canvas and no direct mode
#define RENDER_MODE_FULL    1                      //
#define RENDER_MODE         (RENDER_MODE_PARTIAL)  // selecteaza modul de randare
//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include <stdio.h>

#include "esp_bootloader_desc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define ESP_LCD_IO_I2C_SKIP_INCLUDE 1
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_lcd_touch.h"
#include <lv_conf.h>
#include "lvgl.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"  // Sau driverul real folosit de tine
#include "ui.h"
//#include "ResourceMonitor/ResourceMonitor.h"
#include "ResourceMonitor.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "esp_system.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/soc_caps.h"
#include "cmd_system.h"
#include "cmd_nvs.h"

#include "sdmmc_cmd.h"
}

// ----------------------------------------------------------

#define SD_CS_PIN      (15)  // Chip Select pentru SPI
#define SD_MISO_PIN    (13)
#define SD_MOSI_PIN    (11)
#define SD_SCLK_PIN    (12)
#define SDIO_DATA0_PIN (13)
#define SDIO_CMD_PIN   (11)
#define SDIO_SCLK_PIN  (12)

#define SD_FREQ_DEFAULT   20000 /*!< SD/MMC Default speed (limited by clock divider) */
#define SD_FREQ_HIGHSPEED 40000 /*!< SD High speed (limited by clock divider) */

#define CONSOLE_MAX_CMDLINE_ARGS   8
#define CONSOLE_MAX_CMDLINE_LENGTH 256
#define CONSOLE_PROMPT_MAX_LEN     (32)

#define CONFIG_CONSOLE_STORE_HISTORY      1
#define CONFIG_CONSOLE_IGNORE_EMPTY_LINES 1
#define PROMPT_STR                        CONFIG_IDF_TARGET

void initialize_console_peripheral(void);
void initialize_console_library(const char *history_path);
char *setup_prompt(const char *prompt_str);

/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_CONSOLE_STORE_HISTORY

// #define MOUNT_PATH   "/data"
#define MOUNT_PATH   "/sdcard"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

char prompt[CONSOLE_PROMPT_MAX_LEN];

static void initialize_filesystem_sdmmc(void) {
  esp_vfs_fat_mount_config_t mount_config = {
    .format_if_mount_failed = false, .max_files = 4, .allocation_unit_size = 16 * 1024, .disk_status_check_enable = false, .use_one_fat = false
  };

  sdmmc_card_t *card;
  const char mount_point[] = MOUNT_PATH;

  // Configurare SDMMC host
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

  // Configurare pini slot SDMMC
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.clk = (gpio_num_t)SDIO_SCLK_PIN;
  slot_config.cmd = (gpio_num_t)SDIO_CMD_PIN;
  slot_config.d0 = (gpio_num_t)SDIO_DATA0_PIN;
  slot_config.width = 1;  // 1-bit mode

  // (daca ai pull-up externi, poți comenta linia de mai jos)
  gpio_set_pull_mode((gpio_num_t)SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);

  esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
  if (ret != ESP_OK) {
    ESP_LOGE("SD", "Failed to mount SDMMC (%s)", esp_err_to_name(ret));
    return;
  }

  ESP_LOGI("SD", "SD card mounted at %s", mount_point);
  sdmmc_card_print_info(stdout, card);
}

#else
#define HISTORY_PATH NULL
#endif  // CONFIG_CONSOLE_STORE_HISTORY

static void initialize_nvs(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

void initialize_console_peripheral(void) {
  /* Drain stdout before reconfiguring it */
  fflush(stdout);
  fsync(fileno(stdout));

#if defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
  /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
  usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
  /* Move the caret to the beginning of the next line on '\n' */
  usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

  /* Enable blocking mode on stdin and stdout */
  fcntl(fileno(stdout), F_SETFL, 0);
  fcntl(fileno(stdin), F_SETFL, 0);

  usb_serial_jtag_driver_config_t jtag_config = {
    .tx_buffer_size = 256,
    .rx_buffer_size = 256,
  };

  /* Install USB-SERIAL-JTAG driver for interrupt-driven reads and writes */
  ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&jtag_config));

  /* Tell vfs to use usb-serial-jtag driver */
  usb_serial_jtag_vfs_use_driver();

#else
#error Unsupported console type
#endif

  /* Disable buffering on stdin */
  setvbuf(stdin, NULL, _IONBF, 0);
}

#define MY_ESP_CONSOLE_CONFIG_DEFAULT() \
  {.max_cmdline_length = 256, .max_cmdline_args = 32, .heap_alloc_caps = MALLOC_CAP_DEFAULT, .hint_color = 39, .hint_bold = 0}

void initialize_console_library(const char *history_path) {
  /* Initialize the console */
  esp_console_config_t console_config = {
    .max_cmdline_length = CONSOLE_MAX_CMDLINE_LENGTH,
    .max_cmdline_args = CONSOLE_MAX_CMDLINE_ARGS,
    .heap_alloc_caps = 4096,
#if CONFIG_LOG_COLORS
    .hint_color = atoi(LOG_COLOR_CYAN),
#endif
    .hint_bold = 1,
  };
  ESP_ERROR_CHECK(esp_console_init(&console_config));

  /* Configure linenoise line completion library */
  /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
  linenoiseSetMultiLine(1);

  /* Tell linenoise where to get command completions and hints */
  linenoiseSetCompletionCallback(&esp_console_get_completion);
  linenoiseSetHintsCallback((linenoiseHintsCallback *)&esp_console_get_hint);

  /* Set command history size */
  linenoiseHistorySetMaxLen(100);

  /* Set command maximum length */
  linenoiseSetMaxLineLen(console_config.max_cmdline_length);

  /* Don't return empty lines */
  linenoiseAllowEmpty(false);

#if CONFIG_CONSOLE_STORE_HISTORY
  /* Load command history from filesystem */
  linenoiseHistoryLoad(history_path);
#endif  // CONFIG_CONSOLE_STORE_HISTORY

  /* Figure out if the terminal supports escape sequences */
  const int probe_status = linenoiseProbe();
  if (probe_status) { /* zero indicates success */
    linenoiseSetDumbMode(1);
  }
}

char *setup_prompt(const char *prompt_str) {
  /* set command line prompt */
  const char *prompt_temp = "esp>";
  if (prompt_str) {
    prompt_temp = prompt_str;
  }
  snprintf(prompt, CONSOLE_PROMPT_MAX_LEN - 1, LOG_COLOR_I "%s " LOG_RESET_COLOR, prompt_temp);

  if (linenoiseIsDumbMode()) {
#if CONFIG_LOG_COLORS
    /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the s_prompt.
         */
    snprintf(prompt, CONSOLE_PROMPT_MAX_LEN - 1, "%s ", prompt_temp);
#endif  //CONFIG_LOG_COLORS
  }
  return prompt;
}

// ----------------------------------------------------------

/**********************
 *   GLOBAL VARIABLES
 **********************/
RTC_DATA_ATTR static uint32_t boot_count = 0;
esp_lcd_panel_io_handle_t touch_io_handle = NULL;
esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
esp_lcd_i80_bus_handle_t i80_bus = NULL;
esp_lcd_touch_handle_t touch_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;
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
  gpio_set_level((gpio_num_t)PWR_ON_PIN, mode);  // nu e nevoie de el daca alimentam usb
}
//---------
void power_latch_init() {
  gpio_config_t io_conf = {
    .pin_bit_mask = 1ULL << PWR_EN_PIN,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };
  gpio_config(&io_conf);
  gpio_set_level((gpio_num_t)PWR_EN_PIN, 1);  // ⚡ ține placa aprinsă
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
//---------

/**********************
 *   TOUCH VARIABLES
 **********************/
int16_t touch_map_x1 = 3857;
int16_t touch_map_x2 = 239;
int16_t touch_map_y1 = 213;
int16_t touch_map_y2 = 3693;
uint16_t x = 0, y = 0;
uint8_t num_points = 0;

/**********************
 *   TOUCH FUNCTIONS
 **********************/
int touch_map_value(int val, int in_min, int in_max, int out_min, int out_max) {
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//---------
void touch_get_calibrated_point(int16_t xraw, int16_t yraw, int16_t *x_out, int16_t *y_out) {
  *x_out = touch_map_value(xraw, touch_map_x1, touch_map_x2, 0, LCD_WIDTH - 1);
  *y_out = touch_map_value(yraw, touch_map_y1, touch_map_y2, 0, LCD_HEIGHT - 1);
  if (*x_out < 0) {
    *x_out = 0;
  }
  if (*x_out >= LCD_WIDTH) {
    *x_out = LCD_WIDTH - 1;
  }
  if (*y_out < 0) {
    *y_out = 0;
  }
  if (*y_out >= LCD_HEIGHT) {
    *y_out = LCD_HEIGHT - 1;
  }
}
//---------
bool touch_read(uint16_t *x_out, uint16_t *y_out) {
  uint16_t x_raw = 0, y_raw = 0;
  uint8_t point_count = 0;
  esp_lcd_touch_read_data(touch_handle);
  bool touched = esp_lcd_touch_get_coordinates(touch_handle, &x_raw, &y_raw, NULL, &point_count, 1);
  if (touched && point_count > 0) {
    int16_t x_cal, y_cal;
    touch_get_calibrated_point(x_raw, y_raw, &x_cal, &y_cal);
    if (x_out) {
      *x_out = x_cal;
    }
    if (y_out) {
      *y_out = y_cal;
    }
    return true;
  }
  return false;
}
//---------
bool touch_panel_is_touched(void) {
  uint16_t x_raw = 0, y_raw = 0;
  uint8_t point_count = 0;
  esp_lcd_touch_read_data(touch_handle);
  bool is_touched = esp_lcd_touch_get_coordinates(touch_handle, &x_raw, &y_raw, NULL, &point_count, 1);
  return is_touched && point_count > 0;
}
//---------

/**********************
 *   LVGL VARIABLES
 **********************/
uint32_t bufSize;              // Dimensiunea buffer-ului
lv_color_t *disp_draw_buf;     // Buffer LVGL
lv_color_t *disp_draw_buf_II;  // Buffer LVGL secundar
lv_display_t *disp;            // Display LVGL

/**********************
 *   LVGL FUNCTIONS
 **********************/
/* Display flushing function callback */
void lv_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (const void *)px_map);
  lv_disp_flush_ready(disp);
}
//---------
void lv_touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data) {
  static uint16_t last_x = 0;
  static uint16_t last_y = 0;
  uint16_t x, y;
  if (touch_read(&x, &y)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
    last_x = x;
    last_y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;  // Nu e apăsat → eliberat
    data->point.x = last_x;                 // Păstrăm ultima poziție x (LVGL o cere chiar și în RELEASED)
    data->point.y = last_y;                 // Păstrăm ultima poziție y (LVGL o cere chiar și în RELEASED)
  }
}
//---------
void lv_touchpad_read_v2(lv_indev_t *indev_drv, lv_indev_data_t *data) {
  static uint16_t last_x = 0;                       // Ultima poziție X
  static uint16_t last_y = 0;                       // Ultima poziție Y
  static uint16_t stable_x = 0;                     // Poziția stabilă X
  static uint16_t stable_y = 0;                     // Poziția stabilă Y
  static uint8_t touch_cnt = 0;                     // Numărul de atingeri stabile
  static const uint8_t touch_tolerance = 8;         // Poți crește sau micșora după feeling // Distanța permisă între citiri succesive
  static const uint8_t TOUCH_STABLE_THRESHOLD = 0;  // Threshold pentru stabilitate  // De câte ori trebuie să fie stabil ca să fie considerat apăsat
  uint16_t x, y;
  if (touch_read(&x, &y)) {
    if (abs(x - last_x) < touch_tolerance && abs(y - last_y) < touch_tolerance) {
      if (touch_cnt < 255) {
        touch_cnt++;  // Incrementăm numărul de atingeri stabile
      }
    } else {
      touch_cnt = 0;  // Resetăm dacă mișcarea e prea mare
    }
    last_x = x;
    last_y = y;
    if (touch_cnt >= TOUCH_STABLE_THRESHOLD) {
      data->state = LV_INDEV_STATE_PRESSED;
      stable_x = x;
      stable_y = y;
    } else {
      data->state = LV_INDEV_STATE_RELEASED;  // Nu trimitem touch activ până nu e stabil
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
  data->point.x = stable_x;  // Trimitem ultima poziție stabilă
  data->point.y = stable_y;  // Trimitem ultima poziție stabilă
}
//---------
bool panel_io_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
  if (disp != NULL) {
  } else {
    esp_rom_printf("[lv_trans_done_cb] - disp este NULL!\n");
  }
  return false;  // false înseamnă: nu mai face nimic după
}
//---------
uint32_t lv_get_rtos_tick_count_callback(void) {
  return xTaskGetTickCount();
}  // Callback pentru a obține numărul de tick-uri RTOS
//--------------------------------------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
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
/*********************
 *  rtos variables
 *********************/
TaskHandle_t xHandle_lv_main_task;
TaskHandle_t xHandle_lv_main_tick_task;
TaskHandle_t xHandle_ResourceMonitor;
TaskHandle_t xHandle_v_check_0_pin_state_task;
//---------
#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
SemaphoreHandle_t lvgl_mutex;
bool lv_port_sem_take(void) {
  return (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE);
}
//---------
bool lv_port_sem_give(void) {
  return (xSemaphoreGive(lvgl_mutex) == pdTRUE);
}
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)

void console_app(void *parameter) {
  (void)parameter;
  while (true) {
    char *line = linenoise(prompt);

#if CONFIG_CONSOLE_IGNORE_EMPTY_LINES
    if (line == NULL) { /* Ignore empty lines */
      continue;
      ;
    }
#else
    if (line == NULL) { /* Break on EOF or error */
      break;
    }
#endif  // CONFIG_CONSOLE_IGNORE_EMPTY_LINES

    /* Add the command to the history if not empty*/
    if (strlen(line) > 0) {
      linenoiseHistoryAdd(line);
#if CONFIG_CONSOLE_STORE_HISTORY
      /* Save command history to filesystem */
      linenoiseHistorySave(HISTORY_PATH);
#endif  // CONFIG_CONSOLE_STORE_HISTORY
    }

    /* Try to run the command */
    int ret;
    esp_err_t err = esp_console_run(line, &ret);
    if (err == ESP_ERR_NOT_FOUND) {
      printf("Unrecognized command\n");
    } else if (err == ESP_ERR_INVALID_ARG) {
      // command was empty
    } else if (err == ESP_OK && ret != ESP_OK) {
      printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
    } else if (err != ESP_OK) {
      printf("Internal error: %s\n", esp_err_to_name(err));
    }
    /* linenoise allocates line buffer on the heap, so need to free it */
    linenoiseFree(line);
  }

  ESP_LOGE("CONSOLE", "Error or end-of-input, terminating console");
  esp_console_deinit();
}

/********************************************** */
/*                   TASK                       */
/********************************************** */
#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
void lv_main_tick_task(void *parameter) {
  static TickType_t tick = 0;
  tick = xTaskGetTickCount();  // Inițializare corectă
  while (true) {
    lv_tick_inc(5);                            // Incrementeaza tick-urile la fiecare 5ms
    vTaskDelayUntil(&tick, pdMS_TO_TICKS(5));  // Delay precis mult mai rapid asa
  }
}
#elif (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)
void lv_main_tick_task(void *parameter) {
  static TickType_t tick = 0;
  tick = xTaskGetTickCount();
  while (true) {
    lv_tick_inc(5);                                                      // Incrementeaza tick-urile LVGL
    xTaskNotify(xHandle_lv_main_task, LV_TASK_NOTIFY_SIGNAL, eSetBits);  // Notifica task-ul principal
    vTaskDelayUntil(&tick, pdMS_TO_TICKS(5));                            // Delay precis mult mai rapid asa
  }
}
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)

/********************************************** */
/*                   TASK                       */
/********************************************** */
#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
void lv_main_task(void *parameter) {
  static TickType_t tick = 0;
  tick = xTaskGetTickCount();  // Inițializare corectă
  while (true) {
    if (lv_port_sem_take())  // Protejeaza accesul la LVGL
    {
      lv_timer_handler();  /* let the GUI do its work */
      lv_port_sem_give();  // Eliberam mutex-ul
    }
    vTaskDelayUntil(&tick, pdMS_TO_TICKS(5));  // Delay precis mult mai rapid asa
  }
}
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
#if LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF
void lv_main_task(void *parameter) {
  static TickType_t tick = 0;
  tick = xTaskGetTickCount();                          // Inițializare corectă
  xHandle_lv_main_task = xTaskGetCurrentTaskHandle();  // Încoronarea oficială
  while (true) {
    uint32_t notificationValue;  // Așteapta notificarea
    BaseType_t notified = xTaskNotifyWait(
      0x00,                // Niciun bit de ignorat
      ULONG_MAX,           // Curata toate biturile
      &notificationValue,  // Primește valoarea notificarii
      portMAX_DELAY
    );  // Așteapta notificarea pe termen nelimitat
    if (notified == pdTRUE && (notificationValue & LV_TASK_NOTIFY_SIGNAL)) {
      lv_timer_handler();  // LVGL își face treaba
    }
    vTaskDelayUntil(&tick, pdMS_TO_TICKS(5));  // Delay precis mult mai rapid asa
  }
}
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)

/********************************************** */
/*                   TASK                       */
/********************************************** */
static void IRAM_ATTR v_check_0_pin_state_isr_handler(void *arg) {
  // NOTĂ: NU face log sau delay aici
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xTaskNotifyFromISR((TaskHandle_t)arg, 0x01, eSetBits, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}
//---------
void v_check_0_pin_state_task(void *parameter) {
  (void)parameter;
  xHandle_v_check_0_pin_state_task = xTaskGetCurrentTaskHandle();  // Încoronarea oficială
  uint32_t notificationValue;
  gpio_config_t io_conf = {
    .pin_bit_mask = 1ULL << 0,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,  // activăm pull-up intern
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_ANYEDGE,  // întrerupere pe orice schimbare de stare
  };
  gpio_config(&io_conf);
  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);  // doar o dată în tot proiectul
  gpio_isr_handler_add((gpio_num_t)0, v_check_0_pin_state_isr_handler, (void *)xHandle_v_check_0_pin_state_task);
  while (true) {
    xTaskNotifyWait(0x00, 0xFFFFFFFF, &notificationValue, portMAX_DELAY);
    if (notificationValue & 0x01) {
      ESP_LOGW("BUTTON", "Button ACTIVAT pe GPIO0");
      //// Acțiune custom (PUNE COD AICI)
    }
    vTaskDelay(200);
  }
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
  //// gpio_extra_set_init(1);
  power_latch_init();  // Inițializare latch pentru alimentare
  gfx_set_backlight(1);
  //// esp_log_level_set("*", ESP_LOG_MAX); //
  esp_log_level_set("*", ESP_LOG_INFO);

  initialize_nvs();

#if CONFIG_CONSOLE_STORE_HISTORY
  //initialize_filesystem();
  initialize_filesystem_sdmmc();
  ESP_LOGI("CONSOLE", "Command history enabled");
#else
  ESP_LOGI("CONSOLE", "Command history disabled");
#endif
  /* Initialize console output periheral (UART, USB_OTG, USB_JTAG) */
  initialize_console_peripheral();

  /* Initialize linenoise library and esp_console*/
  initialize_console_library(HISTORY_PATH);

  /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
  const char *prompt = setup_prompt(PROMPT_STR ">");

  /* Register commands */
  esp_console_register_help_command();
  register_system_common();
  register_nvs();

  boot_count++;
  ESP_LOGI("RTC", "Boot count (from RTC RAM): %lu", boot_count);
  esp_sleep_enable_timer_wakeup(5000000);  // 5 secunde în microsecunde

  esp_bootloader_desc_t bootloader_desc;
  printf("\n");
  ESP_LOGI("Bootloader description", "\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
  ESP_LOGI("Bootloader description", "\tESP-IDF version from app: %s\n", IDF_VER);
  // printf("\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
  // printf("\tESP-IDF version from app: %s\n", IDF_VER);

  esp_lcd_i80_bus_config_t lcd_bus_config = {
    .dc_gpio_num = BOARD_TFT_DC,
    .wr_gpio_num = BOARD_TFT_WR,
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .data_gpio_nums =
      {
        BOARD_TFT_DATA0,
        BOARD_TFT_DATA1,
        BOARD_TFT_DATA2,
        BOARD_TFT_DATA3,
        BOARD_TFT_DATA4,
        BOARD_TFT_DATA5,
        BOARD_TFT_DATA6,
        BOARD_TFT_DATA7,
      },
    .bus_width = 8,
    .max_transfer_bytes = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
    .psram_trans_align = 64,
    .sram_trans_align = 4
  };
  // esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus);
  ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus));

  esp_lcd_panel_io_i80_config_t lcd_io_config = {
    .cs_gpio_num = BOARD_TFT_CS,
    .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
    .trans_queue_depth = 10,
    .on_color_trans_done = panel_io_trans_done_callback,
    .user_ctx = NULL,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .dc_levels =
      {
        .dc_idle_level = 0,
        .dc_cmd_level = 0,
        .dc_dummy_level = 0,
        .dc_data_level = 1,
      },
    .flags = {
      .cs_active_high = 0,
      .reverse_color_bits = 0,
      .swap_color_bytes = 1,
      .pclk_active_neg = 0,
      .pclk_idle_low = 0,
    },
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &lcd_io_config, &lcd_io_handle));

  esp_lcd_panel_dev_config_t panel_config = {
    .reset_gpio_num = BOARD_TFT_RST,
    .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
    .bits_per_pixel = 16,
    .flags =
      {
        .reset_active_high = 1,
      },
    .vendor_config = NULL,
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io_handle, &panel_config, &panel_handle));
  ESP_LOGI("LVGL", "ST7789 panel created");
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
  ESP_LOGI("LVGL", "ST7789 panel reset done");
  ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 0));
  ESP_LOGI("LVGL", "ST7789 panel gap set");
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  ESP_LOGI("LVGL", "ST7789 panel initialized");
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, false));
  ESP_LOGI("LVGL", "ST7789 panel color inversion set");
  ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, true));
  ESP_LOGI("LVGL", "ST7789 panel mirror set");
  ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
  ESP_LOGI("LVGL", "ST7789 panel swap xy set %bool", true);
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
  ESP_LOGI("LVGL", "ST7789 panel display on");

  //  Configurare SPI Touch IO
  spi_bus_config_t buscfg = {
    .mosi_io_num = (int)PIN_NUM_MOSI,
    .miso_io_num = (int)PIN_NUM_MISO,
    .sclk_io_num = (int)PIN_NUM_CLK,
    .quadwp_io_num = (int)(-1),
    .quadhd_io_num = (int)(-1),
    .data4_io_num = (int)(-1),
    .data5_io_num = (int)(-1),
    .data6_io_num = (int)(-1),
    .data7_io_num = (int)(-1),
    .data_io_default_level = 0,
    .max_transfer_sz = (int)4096,
    .flags = 0,
    .isr_cpu_id = (esp_intr_cpu_affinity_t)0,
    .intr_flags = 0
  };
  ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
  ESP_LOGI("LVGL", "SPI bus initialized");

  // Configurare IO pentru touch
  esp_lcd_panel_io_spi_config_t touch_io_config = {
    .cs_gpio_num = (gpio_num_t)PIN_NUM_CS,
    .dc_gpio_num = GPIO_NUM_NC,
    .spi_mode = 0,
    .pclk_hz = ESP_LCD_TOUCH_SPI_CLOCK_HZ,
    .trans_queue_depth = 3,
    .on_color_trans_done = NULL,
    .user_ctx = NULL,
    .lcd_cmd_bits = 8,
    .lcd_param_bits = 8,
    .cs_ena_pretrans = 0,
    .cs_ena_posttrans = 0,
    .flags = {
      .dc_high_on_cmd = 0, .dc_low_on_data = 0, .dc_low_on_param = 0, .octal_mode = 0, .quad_mode = 0, .sio_mode = 0, .lsb_first = 0, .cs_high_active = 0
    }
  };
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &touch_io_config, &touch_io_handle));
  ESP_LOGI("LVGL", "Touch panel IO created");

  // Configurare driver touch
  esp_lcd_touch_config_t touch_config = {
    .x_max = 4095,
    .y_max = 4095,
    .rst_gpio_num = (gpio_num_t)-1,
    .int_gpio_num = (gpio_num_t)PIN_NUM_IRQ,
    .levels = {.reset = 0, .interrupt = 0},
    .flags = {.swap_xy = true, .mirror_x = false, .mirror_y = false},
    .process_coordinates = NULL,
    .interrupt_callback = NULL,
    .user_data = NULL,
    .driver_data = NULL
  };
  ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(touch_io_handle, &touch_config, &touch_handle));
  ESP_LOGI("LVGL", "Touch panel created");

  lv_init();
  lv_tick_set_cb(lv_get_rtos_tick_count_callback);
  disp = lv_display_create((int32_t)LCD_WIDTH, (int32_t)LCD_HEIGHT);

#if (BUFFER_MODE == BUFFER_FULL)
  bufSize = ((LCD_WIDTH * LCD_HEIGHT) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_60LINES)
  bufSize = ((LCD_WIDTH * 60) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_40LINES)
  bufSize = ((LCD_WIDTH * 40) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_20LINES)
  bufSize = ((LCD_WIDTH * 20) * lv_color_format_get_size(lv_display_get_color_format(disp)));
#elif (BUFFER_MODE == BUFFER_DEVIDED4)
  bufSize = ((LCD_WIDTH * LCD_HEIGHT) * lv_color_format_get_size(lv_display_get_color_format(disp)) / 4);
#endif
#if (BUFFER_MEM == BUFFER_SPIRAM)
#if (DOUBLE_BUFFER_MODE == 1)
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
  disp_draw_buf_II = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
  ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#else
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
#endif
#elif (BUFFER_MEM == BUFFER_INTERNAL)
#if (DMA_ON == 1)
#if (DOUBLE_BUFFER_MODE == 1)
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
  disp_draw_buf_II = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
  ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#else
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
#endif
#else
#if (DOUBLE_BUFFER_MODE == 1)
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
  disp_draw_buf_II = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
  ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#else
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
#endif
#endif
#endif

  if (!disp_draw_buf) {  // VERIFICA DACA PRIMUL BUFFER ESTE CREAT
    ESP_LOGE("LVGL", "LVGL disp_draw_buf allocate failed!");
  }
#if (DOUBLE_BUFFER_MODE == 1)
  if (!disp_draw_buf_II) {  // VERIFICA DACA AL DOILEA BUFFER ESTE CREAT
    ESP_LOGE("LVGL", "LVGL disp_draw_buf_II allocate failed!");
  }
#endif

#if (DOUBLE_BUFFER_MODE == 1)
  lv_display_set_buffers(disp, disp_draw_buf, disp_draw_buf_II, bufSize, (lv_display_render_mode_t)RENDER_MODE);
  ESP_LOGI("LVGL", "LVGL buffers set");
#else
  lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, (lv_display_render_mode_t)RENDER_MODE);
#endif

  lv_display_set_resolution(disp, LCD_WIDTH, LCD_HEIGHT);                   // Seteaza rezolutia software
  lv_display_set_physical_resolution(disp, LCD_WIDTH, LCD_HEIGHT);          // Actualizeaza rezolutia reala
  lv_display_set_rotation(disp, (lv_display_rotation_t)0);                  // Seteaza rotatia lvgl
  lv_display_set_render_mode(disp, (lv_display_render_mode_t)RENDER_MODE);  // Seteaza (lv_display_render_mode_t)
  lv_display_set_antialiasing(disp, true);                                  // Antialiasing DA sau NU
  ESP_LOGI("LVGL", "LVGL display settings done");

  lv_display_set_flush_cb(disp, lv_disp_flush);  // Set the flush callback which will be called to copy the rendered image to the display.
  ESP_LOGI("LVGL", "LVGL display flush callback set");

  lv_indev_t *indev = lv_indev_create();           /*Initialize the (dummy) input device driver*/
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  ////lv_indev_set_read_cb(indev, lv_touchpad_read);    // old version
  lv_indev_set_read_cb(indev, lv_touchpad_read_v2);
  ESP_LOGI("LVGL", "LVGL Setup done");

  ESP_LOGI("SYSTEM", "Total RAM          memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
  ESP_LOGI("SYSTEM", "Free RAM           memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  ESP_LOGI("SYSTEM", "Total RAM-DMA      memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_DMA));
  ESP_LOGI("SYSTEM", "Free RAM-DMA       memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_DMA));

  ESP_LOGI("SYSTEM", "Total RAM 8 bit    memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  ESP_LOGI("SYSTEM", "Free RAM 8 bit     memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

  ESP_LOGI("SYSTEM", "Total RAM 32 bit   memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
  ESP_LOGI("SYSTEM", "Free RAM 32 bit    memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));

  ESP_LOGI("SYSTEM", "Total RTC RAM      memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_RTCRAM));
  ESP_LOGI("SYSTEM", "Free RTC RAM       memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_RTCRAM));

  ESP_LOGI("SYSTEM", "Total PSRAM        memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI("SYSTEM", "Free PSRAM         memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  ESP_LOGI("SYSTEM", "Total PSRAM 8 bit  memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  ESP_LOGI("SYSTEM", "Free PSRAM 8 bit   memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));

  ESP_LOGI("SYSTEM", "Total PSRAM 32 bit memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT));
  ESP_LOGI("SYSTEM", "Free PSRAM 32 bit  memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT));

  ESP_LOGI("STACK", "Main task stack left: %d bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

#if LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX
  ESP_LOGI("LVGL", "Se creaza LVGL mutex!");
  lvgl_mutex = xSemaphoreCreateMutex();
  if (lvgl_mutex == NULL) {
    ESP_LOGE("LVGL", "Eroare: Mutex-ul LVGL nu a fost creat!");
    while (1);  // Blocheaza executia daca mutex-ul nu poate fi creat
  }
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)

  create_tabs_ui();  // Creeaza interfata grafica

  printf(
    "\n"
    "This is an example of ESP-IDF console component.\n"
    "Type 'help' to get the list of commands.\n"
    "Use UP/DOWN arrows to navigate through command history.\n"
    "Press TAB when typing command name to auto-complete.\n"
    "Ctrl+C will terminate the console environment.\n"
  );

  if (linenoiseIsDumbMode()) {
    printf(
      "\n"
      "Your terminal application does not support escape sequences.\n"
      "Line editing and history features are disabled.\n"
      "On Windows, try using Putty instead.\n"
    );
  }

  xTaskCreatePinnedToCore(
    lv_main_task,                           // Functia task-ului
    (const char *)"LVGL Main Task",         // Numele task-ului
    (uint32_t)(4096 + 4096),                // Dimensiunea stack-ului
    (NULL),                                 // Parametri (daca exista)
    (UBaseType_t)configMAX_PRIORITIES - 3,  // Prioritatea task-ului
    &xHandle_lv_main_task,                  // Handle-ul task-ului
    ((1))                                   // Nucleul pe care ruleaza task-ul
  );

  xTaskCreatePinnedToCore(
    lv_main_tick_task,                      // Functia care ruleaza task-ul
    (const char *)"LVGL Tick Task",         // Numele task-ului
    (uint32_t)(2048 + 1024),                // Dimensiunea stack-ului
    (NULL),                                 // Parametri
    (UBaseType_t)configMAX_PRIORITIES - 1,  // Prioritatea task-ului
    &xHandle_lv_main_tick_task,             // Handle-ul task-ului
    ((1))                                   // Nucleul pe care ruleaza (ESP32 e dual-core)
  );

  // start_resource_monitor();

  xTaskCreatePinnedToCore(
    v_check_0_pin_state_task,               // Functia care ruleaza task-ul
    (const char *)"v_check_0_pin_state",    // Numele task-ului
    (uint32_t)(4096),                       // Dimensiunea stack-ului
    (NULL),                                 // Parametri
    (UBaseType_t)configMAX_PRIORITIES - 6,  // Prioritatea task-ului
    &xHandle_v_check_0_pin_state_task,      // Handle-ul task-ului
    ((1))                                   // Nucleul pe care ruleaza (ESP32 e dual-core)
  );

  xTaskCreatePinnedToCore(
    console_app,                            // Functia care ruleaza task-ul
    (const char *)"Console",                // Numele task-ului
    (uint32_t)(10000),                      // Dimensiunea stack-ului
    (NULL),                                 // Parametri
    (UBaseType_t)configMAX_PRIORITIES - 6,  // Prioritatea task-ului
    &xHandle_v_check_0_pin_state_task,      // Handle-ul task-ului
    ((1))                                   // Nucleul pe care ruleaza (ESP32 e dual-core)
  );
}  // app_main

/********************************************** */

/**********************
 *   END OF FILE
 **********************/
