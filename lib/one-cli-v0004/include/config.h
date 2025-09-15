
#pragma once


#define CONSOLE_MAX_CMDLINE_ARGS (8)
#define CONSOLE_MAX_CMDLINE_LENGTH (256)
#define CONSOLE_PROMPT_MAX_LEN (32)

#define CONFIG_CONSOLE_STORE_HISTORY (1)
#define CONFIG_CONSOLE_IGNORE_EMPTY_LINES (1)
#define PROMPT_STR CONFIG_IDF_TARGET

/**
 * SETTINGS
 */
#define SDCARD_USE
/**********
 * END SETTINGS
 */

#ifdef SDCARD_USE
#define MOUNT_PATH "/sdcard"
#define HISTORY_PATH MOUNT_PATH "/history.txt"
#else
#define BASE_PATH "/spiflash"
#define PARTITION_LABEL "ffat"
#define MOUNT_PATH "/spiflash"
#define HISTORY_PATH MOUNT_PATH "/history.txt"
#endif /* #if SDCARD_USE */

extern char prompt[CONSOLE_PROMPT_MAX_LEN];