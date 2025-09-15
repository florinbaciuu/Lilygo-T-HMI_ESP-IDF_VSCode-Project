/**
 * @file      product_pins.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2024-01-07
 *
 */

#pragma once

#include <sdkconfig.h>
#include "initSequence.h"

#define BOARD_NONE_PIN      (-1)

#define BOARD_POWERON         (gpio_num_t)(14)
#define BOARD_POWERON1        (gpio_num_t)(10)

#define BOARD_TFT_BL         (38)
#define BOARD_TFT_DATA0      (48)
#define BOARD_TFT_DATA1      (47)
#define BOARD_TFT_DATA2      (39)
#define BOARD_TFT_DATA3      (40)
#define BOARD_TFT_DATA4      (41)
#define BOARD_TFT_DATA5      (42)
#define BOARD_TFT_DATA6      (45)
#define BOARD_TFT_DATA7      (46)
#define BOARD_TFT_RST        (-1)
#define BOARD_TFT_CS         (6)
#define BOARD_TFT_DC         (7)
#define BOARD_TFT_WR         (8)


#define BOARD_TOUCH_SPI_MISO (4)
#define BOARD_TOUCH_SPI_MOSI (3)
#define BOARD_TOUCH_SPI_SCLK (1)
#define BOARD_TOUCH_SPI_CS   (2)
#define BOARD_TOUCH_IRQ      (gpio_num_t)(9)

#define AMOLED_WIDTH         (240)
#define AMOLED_HEIGHT        (320)

#define BOARD_HAS_TOUCH      1

#define DISPLAY_BUFFER_SIZE  (AMOLED_WIDTH * 100)

#define DISPLAY_FULLRESH     false






















