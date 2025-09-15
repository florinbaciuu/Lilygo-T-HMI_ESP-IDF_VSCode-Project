#pragma once
#ifndef FILESYSTEM_OS_H
#define FILESYSTEM_OS_H

#include "stdbool.h"
#include "esp_err.h"

#include "eMMC_fs.h"
#include "FAT_fs.h"
#include "LITTLE_fs.h"
#include "SPIF_fs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// PROTOTYPES
bool init_filesystem_sys();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* FILESYSTEM_OS_H */

/***
 * Troubleshooting
 * Card fails to initialize with sdmmc_check_scr: send_scr returned 0xffffffff error
Connections between the eMMC chip and the ESP chip are too long for the frequency used. Try using
shorter connections, or try reducing the clock speed of the bus. Failure to mount filesystem
example: Failed to mount filesystem. If you want the card to be formatted, set the
EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option. The app will be able to mount only cards formatted
using FAT32 filesystem. If the card is formatted as exFAT or some other filesystem, you have an
option to format it in the example code. Enable the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig
option, then build and flash the example.
 */