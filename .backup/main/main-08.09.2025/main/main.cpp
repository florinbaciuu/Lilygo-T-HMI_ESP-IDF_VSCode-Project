
#define BOARD_TFT_DATA0    (48)                // GPIO pin for TFT data line 0
#define BOARD_TFT_DATA1    (47)                // GPIO pin for TFT data line 1
#define BOARD_TFT_DATA2    (39)                // GPIO pin for TFT data line 2
#define BOARD_TFT_DATA3    (40)                // GPIO pin for TFT data line 3
#define BOARD_TFT_DATA4    (41)                // GPIO pin for TFT data line 4
#define BOARD_TFT_DATA5    (42)                // GPIO pin for TFT data line 5
#define BOARD_TFT_DATA6    (45)                // GPIO pin for TFT data line 6
#define BOARD_TFT_DATA7    (46)                // GPIO pin for TFT data line 7
#define BOARD_TFT_RST      (-1)                // GPIO pin for TFT reset, set to -1 if not used
#define BOARD_TFT_CS       (6)                 // GPIO pin for TFT chip select
#define BOARD_TFT_DC       (7)                 // GPIO pin for TFT data/command control
#define BOARD_TFT_WR       (8)                 // GPIO pin for TFT write control
#define LCD_WIDTH          (320)               // Width of the LCD in pixels
#define LCD_HEIGHT         (240)               // Height of the LCD in pixels
#define LCD_PIXEL_CLOCK_HZ (10 * 1000 * 1000)  // LCD pixel clock frequency in Hz
#define BOARD_TFT_BL       (38)                // GPIO pin for backlight control
#define PWR_EN_PIN         (10)                // connected to the battery alone
//---------
#define PWR_ON_PIN \
    (14)                     // if you use an ext 5V power supply, you need to bring a magnet close to the
                             // ReedSwitch and set the PowerOn Pin (GPIO14) to HIGH
#define REED_SWICH_PIN (21)  // connected to the battery and the USB power supply, used to turn off the device
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

#define TICK_INCREMENTATION 5  // in ms(milliseconds) must be equal incrementation with delay
#define LV_TIMER_INCREMENT  TICK_INCREMENTATION * 1000
#define LV_DELAY            TICK_INCREMENTATION
#define LV_NO_OOP           esp_rom_delay_us(100);

//-----------------------------------------------------------
#define LV_TICK_SOURCE_TIMER    0
#define LV_TICK_SOURCE_TASK     1
#define LV_TICK_SOURCE_CALLBACK 2
#ifndef LV_TICK_SOURCE
#    define LV_TICK_SOURCE (LV_TICK_SOURCE_TASK)
#endif /* #ifndef LV_TICK_SOURCE */
//---------
//---------
/*Where flush_ready must to go : in display_flush or in io_trans_done_cb*/
////#define flush_ready_in_disp_flush // nu e asa bun
#define flush_ready_in_io_trans_done  // mult mai bine asa deoarece se da flush ready in momentul cand display face trans_done
//---------

//-----------------------------------------------------------
#if LV_TICK_SOURCE == LV_TICK_SOURCE_TASK
/* LVGL Timer TASK Method block */
#    define USE_MUTEX               1
#    define USE_FREERTOS_TASK_NOTIF 2
#    define LV_TASK_NOTIFY_SIGNAL   0x01
////#define LV_TIMER_TASK_METHOD USE_MUTEX
#    define LV_TIMER_TASK_METHOD (USE_FREERTOS_TASK_NOTIF)  // Chose here
#    if (LV_TIMER_TASK_METHOD == 2)
#        define FPS_DIVIDER 1  // options: 1 max fps, 2 max fps/2 , 3 ,max/3 etcc
#    endif                     /* #if (LV_TIMER_TASK_METHOD == 2) */
#endif                         /* #if  LV_TICK_SOURCE == LV_TICK_SOURCE_TASK */
//---------

//-----------------------------------------------------------

/* BUFFER MODE */
#define BUFFER_20LINES     1
#define BUFFER_40LINES     2
#define BUFFER_60LINES     3  // merge
#define BUFFER_DEVIDED4    4
#define BUFFER_FULL        5            // merge super ok
#define BUFFER_MODE        BUFFER_FULL  // selecteaza modul de buffer , defaut este BUFFER_FULL
#define DOUBLE_BUFFER_MODE true
//---------
/* BUFFER MEMORY TYPE AND DMA */
#define BUFFER_INTERNAL 1
#define BUFFER_SPIRAM   2
#define BUFFER_MEM      BUFFER_SPIRAM
#if (BUFFER_MEM == BUFFER_INTERNAL)
#    define DMA_ON (true)
#endif /* #if (BUFFER_MEM == BUFFER_INTERNAL) */
//---------

//-----------------------------------------------------------
/* RENDER MODE */
// Aici e mod mai special pt ca se transmite direct functiei....
#define RENDER_MODE_PARTIAL (LV_DISPLAY_RENDER_MODE_PARTIAL)
#define RENDER_MODE_FULL    (LV_DISPLAY_RENDER_MODE_FULL)
#define RENDER_MODE_DIRECT  (LV_DISPLAY_RENDER_MODE_DIRECT)

#define RENDER_MODE (RENDER_MODE_PARTIAL)
//-----------------------------------------------------------

//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "esp_bootloader_desc.h"
#include "esp_rom_sys.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl.h"
#include <lv_conf.h>

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_st7789.h"  // Sau driverul real folosit de tine
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_xpt2046.h"

// my include
#include "one-cli.h"
#include "ui.h"
}
/**********************
 *   GLOBAL VARIABLES
 **********************/
RTC_DATA_ATTR static uint32_t boot_count      = 0;
esp_lcd_panel_io_handle_t     touch_io_handle = NULL;
esp_lcd_panel_io_handle_t     lcd_io_handle   = NULL;
esp_lcd_i80_bus_handle_t      i80_bus         = NULL;
esp_lcd_touch_handle_t        touch_handle    = NULL;
esp_lcd_panel_handle_t        panel_handle    = NULL;
//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
void gpio_extra_set_init(uint32_t mode) {
    // Setăm ambii pini ca output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWR_EN_PIN) | (1ULL << PWR_ON_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) PWR_EN_PIN, mode);
    gpio_set_level((gpio_num_t) PWR_ON_PIN, mode);  // nu e nevoie de el daca alimentam usb
}
//---------
void power_latch_init() {
    gpio_config_t io_conf = {.pin_bit_mask = 1ULL << PWR_EN_PIN,
        .mode                              = GPIO_MODE_OUTPUT,
        .pull_up_en                        = GPIO_PULLUP_DISABLE,
        .pull_down_en                      = GPIO_PULLDOWN_DISABLE,
        .intr_type                         = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) PWR_EN_PIN, 1);  // ⚡ ține placa aprinsă
}
//---------
void gfx_set_backlight(uint32_t mode) {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BOARD_TFT_BL,
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) BOARD_TFT_BL, mode);
}
//---------

/**********************
 *   TOUCH VARIABLES
 **********************/
int16_t  touch_map_x1 = 3857;
int16_t  touch_map_x2 = 239;
int16_t  touch_map_y1 = 213;
int16_t  touch_map_y2 = 3693;
uint16_t x            = 0;
uint16_t y            = 0;
uint8_t  num_points   = 0;

/**********************
 *   TOUCH FUNCTIONS
 **********************/
int touch_map_value(int val, int in_min, int in_max, int out_min, int out_max) {
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//---------
void touch_get_calibrated_point(int16_t xraw, int16_t yraw, int16_t* x_out, int16_t* y_out) {
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
bool touch_read(uint16_t* x_out, uint16_t* y_out) {
    uint16_t x_raw = 0, y_raw = 0;
    uint8_t  point_count = 0;
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
    uint8_t  point_count = 0;
    esp_lcd_touch_read_data(touch_handle);
    bool is_touched = esp_lcd_touch_get_coordinates(touch_handle, &x_raw, &y_raw, NULL, &point_count, 1);
    return is_touched && point_count > 0;
}
//---------

#define LVGL_BENCH_TEST

#ifdef LVGL_BENCH_TEST
// --- stats flush (ISR-safe) ---
volatile uint64_t g_flush_tstart_us = 0;
volatile uint32_t g_flush_bytes     = 0;
volatile uint32_t g_flush_last_us   = 0;
volatile uint64_t g_flush_total_us  = 0;
volatile uint32_t g_flush_count     = 0;

// pentru log la 1s (din task, nu din ISR)
static uint32_t g_log_last_tick = 0;

#endif /* #if LVGL_BENCH_TEST */

/**********************
 *   LVGL VARIABLES
 **********************/
uint32_t      bufSize;           // Dimensiunea buffer-ului
lv_color_t*   disp_draw_buf;     // Buffer LVGL
lv_color_t*   disp_draw_buf_II;  // Buffer LVGL secundar
lv_display_t* disp;              // Display LVGL

/**********************
 *   LVGL FUNCTIONS
 **********************/

/* Display flushing function callback */
void lv_disp_flush(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
#ifdef LVGL_BENCH_TEST
    // dimensiune reală a zonei în bytes
    g_flush_bytes     = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) * sizeof(lv_color_t);
    g_flush_tstart_us = esp_timer_get_time();  // ISR-safe
#endif                                         /* #if LVGL_BENCH_TEST */
    esp_lcd_panel_draw_bitmap(
        panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (const void*) px_map);
#ifdef flush_ready_in_disp_flush
    lv_disp_flush_ready(disp);
#endif /* #ifdef (flush_ready_in_disp_flush) */
}
//---------
void lv_touchpad_read(lv_indev_t* indev_drv, lv_indev_data_t* data) {
    static uint16_t last_x = 0;
    static uint16_t last_y = 0;
    uint16_t        x, y;
    if (touch_read(&x, &y)) {
        data->state   = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
        last_x        = x;
        last_y        = y;
    } else {
        data->state   = LV_INDEV_STATE_RELEASED;  // Nu e apăsat → eliberat
        data->point.x = last_x;                   // Păstrăm ultima poziție x (LVGL o cere chiar și în RELEASED)
        data->point.y = last_y;                   // Păstrăm ultima poziție y (LVGL o cere chiar și în RELEASED)
    }
}
//---------
void lv_touchpad_read_v2(lv_indev_t* indev_drv, lv_indev_data_t* data) {
    static uint16_t      last_x          = 0;         // Ultima poziție X
    static uint16_t      last_y          = 0;         // Ultima poziție Y
    static uint16_t      stable_x        = 0;         // Poziția stabilă X
    static uint16_t      stable_y        = 0;         // Poziția stabilă Y
    static uint8_t       touch_cnt       = 0;         // Numărul de atingeri stabile
    static const uint8_t touch_tolerance = 8;         // Poți crește sau micșora după feeling //
                                                      // Distanța permisă între citiri succesive
    static const uint8_t TOUCH_STABLE_THRESHOLD = 0;  // Threshold pentru stabilitate  // De câte ori trebuie
                                                      // să fie stabil ca să fie considerat apăsat
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
            stable_x    = x;
            stable_y    = y;
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
bool panel_io_trans_done_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
    // old rau.
    // if (disp != NULL) {
    // } else {
    //     esp_rom_printf("[lv_trans_done_cb] - disp este NULL!\n");
    // }
    // return false;  // false înseamnă: nu mai face nimic după
#ifdef LVGL_BENCH_TEST
    // calcule rapide, ISR-safe
    uint32_t end_us     = (uint32_t) esp_timer_get_time();
    uint32_t elapsed_us = end_us - (uint32_t) g_flush_tstart_us;

    g_flush_last_us     = elapsed_us;
    g_flush_total_us += elapsed_us;
    g_flush_count++;
#endif /* #ifdef LVGL_BENCH_TEST */
#ifdef flush_ready_in_io_trans_done
    lv_display_t* d = (lv_display_t*) user_ctx;
    if (d)
        lv_disp_flush_ready(d);  // <— mutat aici
#endif                           /* #ifdef (flush_ready_in_io_trans_done) */
    return false;                // nu mai face nimic după
}
//---------
#if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK
uint32_t lv_get_rtos_tick_count_callback(void) {
    return xTaskGetTickCount();
}  // Callback pentru a obține numărul de tick-uri RTOS
#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK */
//--------------------------------------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/

/*********************
 *  rtos variables
 *********************/
TaskHandle_t xHandle_lv_main_task;
TaskHandle_t xHandle_lv_main_tick_task;
TaskHandle_t xHandle_chechButton0State;

// -------------------------------

// lbgl semafor principal (draw)
SemaphoreHandle_t s_lvgl_mutex;

bool s_lvgl_port_init_locking(void) {
    s_lvgl_mutex = xSemaphoreCreateRecursiveMutex();
    return (s_lvgl_mutex != NULL);
}

bool s_lvgl_lock(uint32_t timeout_ms) {
    if (!s_lvgl_mutex)
        return false;
    return xSemaphoreTakeRecursive(s_lvgl_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void s_lvgl_unlock(void) {
    if (s_lvgl_mutex)
        xSemaphoreGiveRecursive(s_lvgl_mutex);
}

// -------------------------------
#if LV_TICK_SOURCE == LV_TICK_SOURCE_TASK && (LV_TIMER_TASK_METHOD == USE_MUTEX)

SemaphoreHandle_t lvgl_timer_mutex;

bool lv_timer_sem_init() {
    ESP_LOGI("LVGL", "Se creaza LVGL mutex!");
    lvgl_timer_mutex = xSemaphoreCreateMutex();
    if (lvgl_timer_mutex == NULL) {
        ESP_LOGE("LVGL", "Eroare: Mutex-ul LVGL nu a fost creat!");
        return 0;
    }
    return (lvgl_timer_mutex != NULL);
}

bool lv_timer_sem_take(void) {
    return (xSemaphoreTake(lvgl_timer_mutex, portMAX_DELAY) == pdTRUE);
}

bool lv_timer_sem_give(void) {
    return (xSemaphoreGive(lvgl_timer_mutex) == pdTRUE);
}
#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_TASK && (LV_TIMER_TASK_METHOD == USE_MUTEX) */
// -------------------------------

#ifdef LVGL_BENCH_TEST
/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_bench_task(void* parameter) {
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();
    g_log_last_tick        = lv_tick_get();
    while (true) {
        // --- log la 1s, din task (NU din ISR) ---
        uint32_t now = lv_tick_get();
        if (now - g_log_last_tick >= 1000 && g_flush_count) {
            double avg_us   = (double) g_flush_total_us / (double) g_flush_count;        // medii
            double fps_avg  = 1e6 / avg_us;                                              // medii
            double fps_inst = g_flush_last_us ? (1e6 / (double) g_flush_last_us) : 0.0;  // medii

            // MB/s (atenție: dacă nu randezi full-screen, asta e pe “ultima zonă”)
            double mbps_inst = g_flush_last_us
                                   ? ((double) g_flush_bytes / (1024.0 * 1024.0)) / ((double) g_flush_last_us / 1e6)
                                   : 0.0;

            ESP_LOGI("STATS", "flush last=%.2f ms | avg=%.2f ms | FPS(inst)=%.1f | FPS(avg)=%.1f | MB/s(inst)=%.2f", g_flush_last_us / 1000.0, avg_us / 1000.0, fps_inst, fps_avg, mbps_inst);

            g_log_last_tick = now;
        }
        // ----------------------------------------
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(100));
    }
}
#endif /* #ifdef LVGL_BENCH_TEST */

/************************************************** */

#if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER

static void lv_tick_timer_callback_func(void* arg) {
    lv_tick_inc((TICK_INCREMENTATION));  // 5 ms ticklv_timer_handler();
}

static void lv_tick_start_timer(void) {
    const esp_timer_create_args_t timer_args = {
        .callback              = &lv_tick_timer_callback_func,
        .dispatch_method       = ESP_TIMER_TASK,
        .name                  = "lv_tick",
        .skip_unhandled_events = true};
    esp_timer_handle_t tick_timer;

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, LV_TIMER_INCREMENT));  // 1000 us = 1 ms
}

void lv_main_task(void* parameter) {
    xHandle_lv_main_task   = xTaskGetCurrentTaskHandle();
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();
    while (true) {
        if (s_lvgl_lock(portMAX_DELAY)) {
            lv_timer_handler();
            // xTaskGenericNotifyFromISR(xHandle_lv_main_task, tskDEFAULT_INDEX_TO_NOTIFY, 0x01, eSetBits);
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));
    }
}

#elif LV_TICK_SOURCE == LV_TICK_SOURCE_TASK

#    if (LV_TIMER_TASK_METHOD == USE_MUTEX)

/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_main_tick_task(void* parameter) {
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();  // Inițializare corectă
    while (true) {
        if (lv_timer_sem_take())  // Protejeaza accesul la LVGL
        {
            lv_tick_inc(TICK_INCREMENTATION);  // Incrementeaza tick-urile la fiecare 5ms
            lv_timer_sem_give();
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));  // Delay precis mult mai rapid asa
    }
}

void lv_main_task(void* parameter) {
    xHandle_lv_main_task   = xTaskGetCurrentTaskHandle();
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();  // Inițializare corectă
    while (true) {
        if (s_lvgl_lock(portMAX_DELAY)) {
            if (lv_timer_sem_take()) {
                lv_timer_handler();   /* let the GUI do its work */
                lv_timer_sem_give();  // Eliberam mutex-ul
            }
            s_lvgl_unlock();
        }
        vTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));  // Delay precis mult mai rapid asa
    }
}

#    elif (LV_TIMER_TASK_METHOD == USE_FREERTOS_TASK_NOTIF)

/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_main_tick_task(void* parameter) {
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();
    uint8_t fps_divider    = 0;
    while (true) {
        lv_tick_inc(TICK_INCREMENTATION);  // Incrementeaza tick-urile LVGL
        fps_divider++;
        if (fps_divider >= FPS_DIVIDER) {
            fps_divider = 0;
            xTaskNotify(xHandle_lv_main_task, LV_TASK_NOTIFY_SIGNAL,
                eSetBits);  // Notifica task-ul principal
        }
        xTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY));  // Delay precis mult mai rapid asa
    }
}

/********************************************** */
/*                   TASK                       */
/********************************************** */
void lv_main_task(void* parameter) {
    xHandle_lv_main_task   = xTaskGetCurrentTaskHandle();
    static TickType_t tick = 0;
    tick                   = xTaskGetTickCount();  // Inițializare corectă
    while (true) {
        uint32_t   notificationValue;
        BaseType_t notified = xTaskNotifyWait(
            0x00,       // nu ignoră nimic
            ULONG_MAX,  // curăță toate biturile
            &notificationValue,
            portMAX_DELAY  // așteaptă cât trebuie
        );
        if (notified == pdTRUE && (notificationValue & LV_TASK_NOTIFY_SIGNAL)) {
            if (s_lvgl_lock(portMAX_DELAY)) {  // <— ADD
                lv_timer_handler();
                s_lvgl_unlock();  // <— ADD
            }
        }
        ////vTaskDelayUntil(&tick, pdMS_TO_TICKS(LV_DELAY_NO_OOP));  // delay doar aici
    }
}

#    endif  // (LV_TIMER_TASK_METHOD == USE_FREERTOS_TASK_NOTIF)
#endif      /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER */

/********************************************** */
/*                   TASK                       */
/********************************************** */
static void IRAM_ATTR chechButton0State_isr_handler(void* arg) {
    // NOTĂ: NU face log sau delay aici
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR((TaskHandle_t) arg, 0x01, eSetBits, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
//---------
void chechButton0State(void* parameter) {
    (void) parameter;
    xHandle_chechButton0State = xTaskGetCurrentTaskHandle();  // Încoronarea oficială
    uint32_t      notificationValue;
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << 0,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,  // activăm pull-up intern
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,  // întrerupere pe orice schimbare de stare
    };
    gpio_config(&io_conf);
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);  // doar o dată în tot proiectul
    gpio_isr_handler_add((gpio_num_t) 0, chechButton0State_isr_handler, (void*) xHandle_chechButton0State);
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
    power_latch_init();  // Inițializare latch pentru alimentare
    gfx_set_backlight(1);
    esp_log_level_set("*", ESP_LOG_INFO);

    boot_count++;
    ESP_LOGI("RTC", "Boot count (from RTC RAM): %lu", boot_count);
    esp_sleep_enable_timer_wakeup(5000000);  // 5 secunde în microsecunde

    esp_bootloader_desc_t bootloader_desc;

    printf("\n");

    ESP_LOGI("Bootloader description", "\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
    ESP_LOGI("Bootloader description", "\tESP-IDF version from app: %s\n", IDF_VER);

    // printf("\tESP-IDF version from 2nd stage bootloader: %s\n",
    // bootloader_desc.idf_ver); printf("\tESP-IDF version from app: %s\n", IDF_VER);

    lv_init();

#if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK
    // Next function comment because create problems with lvgl timers and esp32 timers
    lv_tick_set_cb(lv_get_rtos_tick_count_callback);
#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_CALLBACK */

    disp = lv_display_create(
        (int32_t) LCD_WIDTH,
        (int32_t) LCD_HEIGHT);

    esp_lcd_i80_bus_config_t lcd_bus_config = {.dc_gpio_num = BOARD_TFT_DC,
        .wr_gpio_num                                        = BOARD_TFT_WR,
        .clk_src                                            = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums                                     = {
            BOARD_TFT_DATA0,
            BOARD_TFT_DATA1,
            BOARD_TFT_DATA2,
            BOARD_TFT_DATA3,
            BOARD_TFT_DATA4,
            BOARD_TFT_DATA5,
            BOARD_TFT_DATA6,
            BOARD_TFT_DATA7,
        },
        .bus_width          = 8,
        .max_transfer_bytes = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t),
        .psram_trans_align  = 64,
        .sram_trans_align   = 4};
    // esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus);
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&lcd_bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t lcd_io_config = {
        .cs_gpio_num = BOARD_TFT_CS,
        //.pclk_hz             = LCD_PIXEL_CLOCK_HZ,
        //.pclk_hz             = 30000000,
        //.pclk_hz             = 26000000,
        .pclk_hz             = 20000000,
        .trans_queue_depth   = 10,
        .on_color_trans_done = panel_io_trans_done_callback,
        //.user_ctx            = NULL,
        .user_ctx       = disp,
        .lcd_cmd_bits   = 8,
        .lcd_param_bits = 8,
        .dc_levels      = {
                 .dc_idle_level  = 0,
                 .dc_cmd_level   = 0,
                 .dc_dummy_level = 0,
                 .dc_data_level  = 1,
        },
        .flags = {
            .cs_active_high     = 0,
            .reverse_color_bits = 0,
            .swap_color_bytes   = 1,
            .pclk_active_neg    = 0,
            .pclk_idle_low      = 0,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &lcd_io_config, &lcd_io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BOARD_TFT_RST,
        .rgb_ele_order  = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian    = LCD_RGB_DATA_ENDIAN_BIG,
        .bits_per_pixel = 16,
        .flags          = {
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
    spi_bus_config_t buscfg = {.mosi_io_num = (int) PIN_NUM_MOSI,
        .miso_io_num                        = (int) PIN_NUM_MISO,
        .sclk_io_num                        = (int) PIN_NUM_CLK,
        .quadwp_io_num                      = (int) (-1),
        .quadhd_io_num                      = (int) (-1),
        .data4_io_num                       = (int) (-1),
        .data5_io_num                       = (int) (-1),
        .data6_io_num                       = (int) (-1),
        .data7_io_num                       = (int) (-1),
        .data_io_default_level              = 0,
        .max_transfer_sz                    = (int) 4096,
        .flags                              = 0,
        .isr_cpu_id                         = (esp_intr_cpu_affinity_t) 0,
        .intr_flags                         = 0};
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI("LVGL", "SPI bus initialized");

    // Configurare IO pentru touch
    esp_lcd_panel_io_spi_config_t touch_io_config = {.cs_gpio_num = (gpio_num_t) PIN_NUM_CS,
        .dc_gpio_num                                              = GPIO_NUM_NC,
        .spi_mode                                                 = 0,
        .pclk_hz                                                  = ESP_LCD_TOUCH_SPI_CLOCK_HZ,
        .trans_queue_depth                                        = 3,
        .on_color_trans_done                                      = NULL,
        .user_ctx                                                 = NULL,
        .lcd_cmd_bits                                             = 8,
        .lcd_param_bits                                           = 8,
        .cs_ena_pretrans                                          = 0,
        .cs_ena_posttrans                                         = 0,
        .flags                                                    = {.dc_high_on_cmd = 0,
                                                               .dc_low_on_data       = 0,
                                                               .dc_low_on_param      = 0,
                                                               .octal_mode           = 0,
                                                               .quad_mode            = 0,
                                                               .sio_mode             = 0,
                                                               .lsb_first            = 0,
                                                               .cs_high_active       = 0}};
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &touch_io_config, &touch_io_handle));
    ESP_LOGI("LVGL", "Touch panel IO created");

    // Configurare driver touch
    esp_lcd_touch_config_t touch_config = {.x_max = 4095,
        .y_max                                    = 4095,
        .rst_gpio_num                             = (gpio_num_t) -1,
        .int_gpio_num                             = (gpio_num_t) PIN_NUM_IRQ,
        .levels                                   = {.reset = 0, .interrupt = 0},
        .flags                                    = {.swap_xy = true, .mirror_x = false, .mirror_y = false},
        .process_coordinates                      = NULL,
        .interrupt_callback                       = NULL,
        .user_data                                = NULL,
        .driver_data                              = NULL};
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(touch_io_handle, &touch_config, &touch_handle));
    ESP_LOGI("LVGL", "Touch panel created");

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
#    if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf    = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
    disp_draw_buf_II = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#    else
    disp_draw_buf = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
#    endif
#elif (BUFFER_MEM == BUFFER_INTERNAL)
#    if (DMA_ON == 1)
#        if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf    = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    disp_draw_buf_II = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#        else
    disp_draw_buf = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);
#        endif
#    else
#        if (DOUBLE_BUFFER_MODE == 1)
    disp_draw_buf    = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
    disp_draw_buf_II = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
    ESP_LOGI("LVGL", "LVGL buffers created in SPIRAM");
#        else
    disp_draw_buf = (lv_color_t*) heap_caps_malloc(bufSize, MALLOC_CAP_INTERNAL);
#        endif
#    endif
#endif /* #if (BUFFER_MEM == BUFFER_SPIRAM) */

    // --- memset pentru curățare frame-uri ---
    // Asta îți garantează că primul frame e complet „negru”

    if (disp_draw_buf) {
        memset(disp_draw_buf, 0, bufSize);
    }
    if (disp_draw_buf_II) {
        memset(disp_draw_buf_II, 0, bufSize);
    }

    if (!disp_draw_buf) {  // VERIFICA DACA PRIMUL BUFFER ESTE CREAT
        ESP_LOGE("LVGL", "LVGL disp_draw_buf allocate failed!");
    }
#if (DOUBLE_BUFFER_MODE == 1)
    if (!disp_draw_buf_II) {  // VERIFICA DACA AL DOILEA BUFFER ESTE CREAT
        ESP_LOGE("LVGL", "LVGL disp_draw_buf_II allocate failed!");
    }
#endif

#if (DOUBLE_BUFFER_MODE == 1)
    lv_display_set_buffers(
        disp, disp_draw_buf, disp_draw_buf_II, bufSize, (lv_display_render_mode_t) RENDER_MODE);
    ESP_LOGI("LVGL", "LVGL buffers set");
#else
    lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, (lv_display_render_mode_t) RENDER_MODE);
#endif

    lv_display_set_resolution(disp, LCD_WIDTH, LCD_HEIGHT);           // Seteaza rezolutia software
    lv_display_set_physical_resolution(disp, LCD_WIDTH, LCD_HEIGHT);  // Actualizeaza rezolutia reala
    lv_display_set_rotation(disp, (lv_display_rotation_t) 0);         // Seteaza rotatia lvgl
    lv_display_set_render_mode(disp,
        (lv_display_render_mode_t) RENDER_MODE);  // Seteaza (lv_display_render_mode_t)
    lv_display_set_antialiasing(disp, true);      // Antialiasing DA sau NU
    ESP_LOGI("LVGL", "LVGL display settings done");

    lv_display_set_flush_cb(disp, lv_disp_flush);  // Set the flush callback which will be called to
                                                   // copy the rendered image to the display.
    ESP_LOGI("LVGL", "LVGL display flush callback set");

    lv_indev_t* indev = lv_indev_create();           /*Initialize the (dummy) input device driver*/
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    ////lv_indev_set_read_cb(indev, lv_touchpad_read);    // old version
    lv_indev_set_read_cb(indev, lv_touchpad_read_v2);
    ESP_LOGI("LVGL", "LVGL Setup done");

    vTaskDelay(500);

    StartCLI();

#if (LV_TICK_SOURCE == LV_TICK_SOURCE_TASK) && (LV_TIMER_TASK_METHOD == USE_MUTEX)
    lv_timer_sem_init();
#endif  // (LV_TIMER_TASK_METHOD == USE_MUTEX)
    s_lvgl_port_init_locking();
    esp_rom_delay_us(100);
    s_lvgl_lock(0);
    create_tabs_ui();  // Creeaza interfata grafica
    s_lvgl_unlock();
    esp_rom_delay_us(100);

    xTaskCreatePinnedToCore(lv_main_task,        // Functia task-ului
        (const char*) "LVGL Main Task",          // Numele task-ului
        (uint32_t) (4096 + 4096),                // Dimensiunea stack-ului
        (NULL),                                  // Parametri (daca exista)
        (UBaseType_t) configMAX_PRIORITIES - 4,  // Prioritatea task-ului // 3
        &xHandle_lv_main_task,                   // Handle-ul task-ului
        ((1))                                    // Nucleul pe care ruleaza task-ul
    );

#if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER
    lv_tick_start_timer();
#elif LV_TICK_SOURCE == LV_TICK_SOURCE_TASK
    xTaskCreatePinnedToCore(lv_main_tick_task,   // Functia care ruleaza task-ul
        (const char*) "LVGL Tick Task",          // Numele task-ului
        (uint32_t) (2048 + 1024),                // Dimensiunea stack-ului
        (NULL),                                  // Parametri
        (UBaseType_t) configMAX_PRIORITIES - 2,  // Prioritatea task-ului // 1
        &xHandle_lv_main_tick_task,              // Handle-ul task-ului
        ((1))                                    // Nucleul pe care ruleaza (ESP32 e dual-core)
    );
#endif /* #if LV_TICK_SOURCE == LV_TICK_SOURCE_TIMER */

    vTaskDelay(500);
    printf("HAH\n");

#ifdef LVGL_BENCH_TEST
    esp_rom_delay_us(100);
    xTaskCreatePinnedToCore(lv_bench_task, "lvBench", 4096, NULL, tskIDLE_PRIORITY + 1, NULL, 1);
#endif /* #ifdef LVGL_BENCH_TEST */

    xTaskCreatePinnedToCore(chechButton0State,   // Functia care ruleaza task-ul
        (const char*) "v_check_0_pin_state",     // Numele task-ului
        (uint32_t) (4096),                       // Dimensiunea stack-ului
        (NULL),                                  // Parametri
        (UBaseType_t) configMAX_PRIORITIES - 7,  // Prioritatea task-ului // 6
        &xHandle_chechButton0State,              // Handle-ul task-ului
        ((1))                                    // Nucleul pe care ruleaza (ESP32 e dual-core)
    );

    printf("T\n");
    esp_rom_delay_us(100);
    printf("Aici aplicatia ar trebui sa returneze.Meh\n");
    vTaskDelay(100);
    printf("Aici aplicatia ar trebui sa returneze.Meh\n");
}
