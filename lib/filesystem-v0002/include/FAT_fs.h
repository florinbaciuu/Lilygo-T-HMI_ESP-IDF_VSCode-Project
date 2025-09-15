#pragma once
#ifndef FAT_FS_H
#define FAT_FS_H

#include "stdbool.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// PROTOTYPES
esp_err_t initialize_internal_fat_filesystem(); // init internat FAT Partition Filesystem

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FAT_FS_H */