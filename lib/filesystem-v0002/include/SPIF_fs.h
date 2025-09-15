#pragma once
#ifndef SPIF_FS_H
#define SPIF_FS_H

#include "stdbool.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//PROTOTYPES
esp_err_t initialize_filesystem_littlefs() ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SPIF_FS_H */