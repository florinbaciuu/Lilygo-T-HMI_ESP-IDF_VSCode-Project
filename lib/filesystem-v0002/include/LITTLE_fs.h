#pragma once
#ifndef LITTLE_FS_H
#define LITTLE_FS_H

#include "stdbool.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// PROTOTYPES
esp_err_t initialize_filesystem_spiffs();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LITTLE_FS_H */