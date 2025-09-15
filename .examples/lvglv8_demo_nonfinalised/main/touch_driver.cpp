/**
 * @file      touch_driver.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2024-01-07
 *
 */
#include <sdkconfig.h>
#include "esp_err.h"
#include "esp_log.h"
#include "product_pins.h"
#include "freertos/FreeRTOS.h"

#if BOARD_HAS_TOUCH

static const char *TAG = "touch";


extern "C" {
    void board_hmi_touch_init();
    uint8_t board_hmi_get_point(uint16_t *x, uint16_t *y);
}


static bool _init_success = false;


void touch_home_button_callback(void *args)
{
    ESP_LOGI(TAG, "Pressed Home button");
}

bool touch_init()
{
    ESP_LOGI(TAG, "Initialize Touchpanle");
    board_hmi_touch_init();
    _init_success = true;
    return true;
}

uint8_t touch_get_data(int16_t *x, int16_t *y, uint8_t point_num)
{
    uint8_t touched = 0;
    uint16_t pointX;
    uint16_t pointY;
    touched = board_hmi_get_point(&pointX, &pointY);
    if (touched) {
        *x = pointX;
        *y = pointY;

    }
    return touched;
}
#else

bool touch_init()
{
    return true;
}
#endif