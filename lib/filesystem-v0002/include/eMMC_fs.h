#pragma once
#ifndef EMMC_FS_H
#define EMMC_FS_H

#include "stdbool.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

esp_err_t initialize_filesystem_sdmmc();        // init SD MMC FAT Partition Filesystem
esp_err_t deinitialize_filesystem_sdmmc();      // deinit SD MMC FAT Partition Filesystem


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EMMC_FS_H */