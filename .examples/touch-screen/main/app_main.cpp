

extern "C"
{
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define ESP_LCD_IO_I2C_SKIP_INCLUDE 1
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch_xpt2046.h"
#include "esp_lcd_touch.h"
#include <lv_conf.h>
#include "lvgl.h"
}

#define PIN_NUM_MISO 4
#define PIN_NUM_MOSI 3
#define PIN_NUM_CLK 1
#define PIN_NUM_CS 2
#define PIN_NUM_IRQ 9

#define LCD_WIDTH 320
#define LCD_HEIGHT 240

int16_t touch_map_x1 = 3857;
int16_t touch_map_x2 = 239;
int16_t touch_map_y1 = 213;
int16_t touch_map_y2 = 3693;


esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
esp_lcd_panel_io_handle_t touch_io_handle = NULL;

esp_lcd_touch_handle_t touch_handle = NULL;
esp_lcd_panel_handle_t lcd_handle = NULL;

uint16_t x = 0, y = 0;
uint8_t num_points = 0;

int map_value(int val, int in_min, int in_max, int out_min, int out_max)
{
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void get_calibrated_point(int16_t xraw, int16_t yraw, int16_t *x_out, int16_t *y_out)
{
    // Calibrare pe X (observăm că e inversat)
    *x_out = map_value(xraw, touch_map_x1, touch_map_x2, 0, LCD_WIDTH - 1);
    // Calibrare pe Y (merge direct)
    *y_out = map_value(yraw, touch_map_y1, touch_map_y2, 0, LCD_HEIGHT - 1);

    // Limitare în caz de overshoot
    if (*x_out < 0)
        *x_out = 0;
    if (*x_out >= LCD_WIDTH)
        *x_out = LCD_WIDTH - 1;
    if (*y_out < 0)
        *y_out = 0;
    if (*y_out >= LCD_HEIGHT)
        *y_out = LCD_HEIGHT - 1;
}

extern "C"
{
    void app_main(void)
    {
        // esp_log_level_set("*", ESP_LOG_MAX); //
        //  Configurare SPI
        spi_bus_config_t buscfg = {
            .mosi_io_num = PIN_NUM_MOSI,
            .miso_io_num = PIN_NUM_MISO,
            .sclk_io_num = PIN_NUM_CLK,
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
            .intr_flags = 0};
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

        // Configurare IO pentru touch
        esp_lcd_panel_io_spi_config_t io_config = {
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
            .flags =
                {
                    .dc_high_on_cmd = 0,
                    .dc_low_on_data = 0,
                    .dc_low_on_param = 0,
                    .octal_mode = 0,
                    .quad_mode = 0,
                    .sio_mode = 0,
                    .lsb_first = 0,
                    .cs_high_active = 0}};

        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_config, &io_handle));

        // Configurare driver touch
        esp_lcd_touch_config_t touch_config = {
            .x_max = 4095,
            .y_max = 4095,
            .rst_gpio_num = (gpio_num_t)-1,
            .int_gpio_num = (gpio_num_t)PIN_NUM_IRQ,
            .levels = {
                .reset = 0,
                .interrupt = 0},
            .flags = {.swap_xy = false, .mirror_x = false, .mirror_y = false},
            .process_coordinates = NULL,
            .interrupt_callback = NULL,
            .user_data = NULL,
            .driver_data = NULL};

        ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(io_handle, &touch_config, &touch_handle));

        // Loop touch
        while (true)
        {
            bool touched = false;
            esp_lcd_touch_read_data(touch_handle);
            touched = esp_lcd_touch_get_coordinates(touch_handle, &x, &y, NULL, &num_points, 1);

            if (touched)
            {
                // uint16_t x, y;
                esp_lcd_touch_get_coordinates(touch_handle, &x, &y, NULL, &num_points, 1);
                printf("RAW Touch at: X=%d Y=%d\n", x, y);
                int16_t x_cal, y_cal;
                get_calibrated_point(x, y, &x_cal, &y_cal);
                printf("CALIBRATED Touch at: X=%d Y=%d\n", x_cal, y_cal);
                fflush(stdout);
            }
            fflush(stdout);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
