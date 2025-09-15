
/*********************
 *      DEFINES
 *********************/
#define BOARD_TFT_BL               (38)
#define BOARD_TFT_DATA0            (48)
#define BOARD_TFT_DATA1            (47)
#define BOARD_TFT_DATA2            (39)
#define BOARD_TFT_DATA3            (40)
#define BOARD_TFT_DATA4            (41)
#define BOARD_TFT_DATA5            (42)
#define BOARD_TFT_DATA6            (45)
#define BOARD_TFT_DATA7            (46)
#define BOARD_TFT_RST              (-1)
#define BOARD_TFT_CS               (6)
#define BOARD_TFT_DC               (7)
#define BOARD_TFT_WR               (8)
#define LCD_WIDTH  (320)
#define LCD_HEIGHT (240)
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000)
#define GFX_BL                     (38)  // pin
#define GFX_RRR                    (10)  // pin
#define GFX_RRR1                   (14)  // pin
#define PIN_NUM_MISO               4
#define PIN_NUM_MOSI               3
#define PIN_NUM_CLK                1
#define PIN_NUM_CS                 2
#define PIN_NUM_IRQ                9
//--------------------------
#define USE_MUTEX                  0
#define USE_FREERTOS_TASK_NOTIF    1     // cica e mai rapid cu 20 %
#define LV_TASK_NOTIFY_SIGNAL      0x01  // Semnalul pentru notificarea LVGL
#define LV_TASK_NOTIFY_SIGNAL_MODE USE_MUTEX
//--------------------------
/**
 * BUFFER MODE:
 */
#define BUFFER_20LINES     0
#define BUFFER_40LINES     1
#define BUFFER_60LINES     2  // merge
#define BUFFER_DEVIDED4    3
#define BUFFER_FULL        4            // merge super ok
//--------------------------
#define BUFFER_MODE        BUFFER_FULL  // selecteaza modul de buffer , defaut este BUFFER_FULL
#define DOUBLE_BUFFER_MODE 1
//--------------------------
/**
 * BUFFER MEMORY TYPE AND DMA :
 */
#define BUFFER_INTERNAL 0
#define BUFFER_SPIRAM   1
#define BUFFER_MEM      BUFFER_SPIRAM
#if (BUFFER_MEM == BUFFER_INTERNAL)
#define DMA_ON 1
#endif
//--------------------------
/**
 * RENDER MODE :
 */
#define RENDER_MODE_PARTIAL 0  // Modul recomandat pt dual buffer and no canvas and no direct mode
#define RENDER_MODE_FULL    1  //
///--------------------------
#define RENDER_MODE RENDER_MODE_PARTIAL  // selecteaza modul de randare
///--------------------------

//--------------------------------------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include <stdio.h>
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
#include "task_monitor.h"
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

//--------------------------------------

void vApplicationIdleHook(void) {
  // Codul tău aici. Rulează când ESP-ul nu face nimic altceva.
}

//--------------------------------------

void vApplicationTickHook(void) {
  // Codul tău aici, executat la fiecare tick (ex. 1ms)
}

//--------------------------------------

void vApplicationTaskManager(void *parameter) {
  (void)parameter;
  while (true) {
    task_monitor_cb();
    vTaskDelay(4000);
  }
}

//--------------------------------------

int16_t touch_map_x1 = 3857;
int16_t touch_map_x2 = 239;
int16_t touch_map_y1 = 213;
int16_t touch_map_y2 = 3693;

uint16_t x = 0, y = 0;
uint8_t num_points = 0;

esp_lcd_panel_io_handle_t touch_io_handle = NULL;
esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
esp_lcd_i80_bus_handle_t i80_bus = NULL;
esp_lcd_touch_handle_t touch_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;

uint32_t bufSize;              // Dimensiunea buffer-ului
lv_color_t *disp_draw_buf;     // Buffer LVGL
lv_color_t *disp_draw_buf_II;  // Buffer LVGL secundar
lv_display_t *disp;            // Display LVGL

//--------------------------------------

void gpio_extra_set_init(uint32_t mode) {
  // Setăm ambii pini ca output
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << GFX_RRR) | (1ULL << GFX_RRR1),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  // Dăm HIGH pe ambii pini
  gpio_set_level((gpio_num_t)GFX_RRR, mode);
  gpio_set_level((gpio_num_t)GFX_RRR1, mode);
}

//--------------------------------------

void gfx_set_backlight(uint32_t mode) {
  gpio_config_t io_conf = {
    .pin_bit_mask = 1ULL << GFX_BL,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);
  gpio_set_level((gpio_num_t)GFX_BL, mode);
}

//--------------------------------------

int touch_map_value(int val, int in_min, int in_max, int out_min, int out_max) {
  return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//--------------------------------------

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

//--------------------------------------

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

//--------------------------------------

bool touch_panel_is_touched(void) {
  uint16_t x_raw = 0, y_raw = 0;
  uint8_t point_count = 0;
  esp_lcd_touch_read_data(touch_handle);
  bool is_touched = esp_lcd_touch_get_coordinates(touch_handle, &x_raw, &y_raw, NULL, &point_count, 1);
  return is_touched && point_count > 0;
}

//--------------------------------------

/* Display flushing function callback */
void lv_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (const void *)px_map);
}

//--------------------------------------

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

//--------------------------------------

bool lv_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
  if (disp != NULL) {
    lv_disp_flush_ready(disp);
  } else {
    esp_rom_printf("[lv_trans_done_cb] - disp este NULL!\n");
  }
  // esp_rom_printf("Transfer de culoare finalizat!\n");
  return false;  // false înseamnă: nu mai face nimic după
}

//--------------------------------------

uint32_t lv_get_rtos_tick_count_callback(void) {
  return xTaskGetTickCount();
}

//--------------------------------------

/*  _____              ____ _____ ___  ____   */
/* |  ___| __ ___  ___|  _ \_   _/ _ \/ ___|  */
/* | |_ | '__/ _ \/ _ \ |_) || || | | \___ \  */
/* |  _|| | |  __/  __/  _ < | || |_| |___) | */
/* |_|  |_|  \___|\___|_| \_\|_| \___/|____/  */

TaskHandle_t xHandle_lv_main_task;
TaskHandle_t xHandle_lv_main_tick_task;
TaskHandle_t xHandle_TaskManager;

//--------------------------------------

#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
SemaphoreHandle_t lvgl_mutex;
// static SemaphoreHandle_t lvgl_mutex = NULL;
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)

#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
bool lv_port_sem_take(void) {
  return (xSemaphoreTake(lvgl_mutex, portMAX_DELAY) == pdTRUE);
}
#endif /* #if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX) */

///--------------------------------------

#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
bool lv_port_sem_give(void) {
  return (xSemaphoreGive(lvgl_mutex) == pdTRUE);
}
#endif /* #if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX) */

/**************************************************
 * Task 1
 **************************************************/
#if (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)
void lv_main_tick_task(void *parameter) {
  static TickType_t tick = 0;
  tick = xTaskGetTickCount();  // Inițializare corectă
  while (true) {
    lv_tick_inc(5);  // Incrementeaza tick-urile la fiecare 5ms
    // vTaskDelay(5 / portTICK_PERIOD_MS);
    vTaskDelayUntil(&tick,
                    pdMS_TO_TICKS(5));  // Delay precis mult mai rapid asa
  }
}
#elif (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)
void lv_main_tick_task(void *parameter) {
  static TickType_t tick = 0;
  tick = xTaskGetTickCount();  // Inițializare corectă
  while (true) {
    lv_tick_inc(5);  // Incrementeaza tick-urile LVGL

    xTaskNotify(xHandle_lv_main_task, LV_TASK_NOTIFY_SIGNAL,
                eSetBits);  // Notifica task-ul principal
    // vTaskDelay(5 / portTICK_PERIOD_MS);
    vTaskDelayUntil(&tick,
                    pdMS_TO_TICKS(5));  // Delay precis mult mai rapid asa
  }
}
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)

/**************************************************
 * Task 2
 **************************************************/
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
    vTaskDelayUntil(&tick,
                    pdMS_TO_TICKS(5));  // Delay precis mult mai rapid asa
  }
}
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_MUTEX)

//--------------------------------------

#if LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF
void lv_main_task(void *parameter) {
  static TickType_t tick = 0;
  tick = xTaskGetTickCount();  // Inițializare corectă
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
    vTaskDelayUntil(&tick,
                    pdMS_TO_TICKS(5));  // Delay precis mult mai rapid asa
  }
}
#endif  // (LV_TASK_NOTIFY_SIGNAL_MODE == USE_FREERTOS_TASK_NOTIF)

//--------------------------------------

extern "C" {
void app_main(void) {
  gpio_extra_set_init(1);
  gfx_set_backlight(1);
  // esp_log_level_set("*", ESP_LOG_MAX); //
  esp_log_level_set("*", ESP_LOG_INFO);

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
  esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus);

  esp_lcd_panel_io_i80_config_t lcd_io_config = {
    .cs_gpio_num = BOARD_TFT_CS,
    .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
    .trans_queue_depth = 10,
    .on_color_trans_done = lv_trans_done_callback,
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
  lv_color_t *disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
#else
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
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
#else   // !(DOUBLE_BUFFER_MODE == 1)
  lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, (lv_display_render_mode_t)RENDER_MODE);
#endif  // !(DOUBLE_BUFFER_MODE == 1)

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
  lv_indev_set_read_cb(indev, lv_touchpad_read);
  ESP_LOGI("LVGL", "LVGL Setup done");

  ESP_LOGI("SYSTEM", "Total RAM       memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
  ESP_LOGI("SYSTEM", "Free RAM        memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  ESP_LOGI("SYSTEM", "Total RAM-DMA   memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_DMA));
  ESP_LOGI("SYSTEM", "Free RAM-DMA    memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_DMA));
  ESP_LOGI("SYSTEM", "Total PSRAM     memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI("SYSTEM", "Free PSRAM      memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI("SYSTEM", "Total PSRAM-DMA memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA));
  ESP_LOGI("SYSTEM", "Free PSRAM-DMA  memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA));
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

  xTaskCreatePinnedToCore(
    vApplicationTaskManager,                // Functia care ruleaza task-ul
    (const char *)"Task Manager",           // Numele task-ului
    (uint32_t)(4096 + 1024),                // Dimensiunea stack-ului
    (NULL),                                 // Parametri
    (UBaseType_t)configMAX_PRIORITIES - 1,  // Prioritatea task-ului
    &xHandle_TaskManager,                   // Handle-ul task-ului
    ((1))                                   // Nucleul pe care ruleaza (ESP32 e dual-core)
  );
}
}

//--------------------------------------
