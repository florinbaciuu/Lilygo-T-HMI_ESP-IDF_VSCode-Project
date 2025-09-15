/**
 * @file      power_driver.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2024-01-08
 *
 */
#include <stdio.h>
#include <cstring>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "product_pins.h"
#include "driver/gpio.h"

static const char *TAG = "POWER";

bool power_driver_init()
{
#ifdef BOARD_POWERON
    ESP_LOGI(TAG, "Turn on board power pin");
    gpio_config_t poweron_gpio_config = {(int)0};
    poweron_gpio_config.pin_bit_mask = 1ULL << BOARD_POWERON;
    poweron_gpio_config.mode = GPIO_MODE_OUTPUT;
    ESP_ERROR_CHECK(gpio_config(&poweron_gpio_config));
    gpio_set_level(BOARD_POWERON, 1);
#endif

#ifdef BOARD_POWERON1
    ESP_LOGI(TAG, "Turn on board power pin");
    gpio_config_t poweron1_gpio_config = {(int)0};
    poweron1_gpio_config.pin_bit_mask = 1ULL << BOARD_POWERON1;
    poweron1_gpio_config.mode = GPIO_MODE_OUTPUT;
    ESP_ERROR_CHECK(gpio_config(&poweron1_gpio_config));
    gpio_set_level(BOARD_POWERON1, 1);
#endif

    return true;
}