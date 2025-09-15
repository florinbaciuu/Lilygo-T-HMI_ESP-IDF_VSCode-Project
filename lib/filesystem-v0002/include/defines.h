#pragma once

/**********************
 * INTERNAL MMC DEFINES
 **********************/
#define BASE_PATH "/spiflash"
#define PARTITION_LABEL "ffat"
#define FAT_MOUNT_PATH "/spiflash"

//-------------------------

/**********************
 *   SD MMC DEFINES
 **********************/
#define SD_CS_PIN (15)
#define SD_MISO_PIN (13)
#define SD_MOSI_PIN (11)
#define SD_SCLK_PIN (12)
#define SDIO_DATA0_PIN (13)
#define SDIO_CMD_PIN (11)
#define SDIO_SCLK_PIN (12)
//---
#define SD_FREQ_DEFAULT 20000   /*!< SD/MMC Default speed (limited by clock divider) */
#define SD_FREQ_HIGHSPEED 40000 /*!< SD High speed (limited by clock divider) */

#define SD_MOUNT_PATH "/sdcard"

//-------------------------

/**
 * @brief Default sdmmc_host_t structure initializer for SDMMC peripheral
 *
 * Uses SDMMC peripheral, with 4-bit mode enabled, and max frequency set to 20MHz
 */
#define SDMMC_HOST_DEF()                                                                           \
    {                                                                                              \
        .flags = SDMMC_HOST_FLAG_8BIT | SDMMC_HOST_FLAG_4BIT | SDMMC_HOST_FLAG_1BIT |              \
            SDMMC_HOST_FLAG_DDR | SDMMC_HOST_FLAG_DEINIT_ARG,                                      \
        .slot                   = SDMMC_HOST_SLOT_1,                                               \
        .max_freq_khz           = SDMMC_FREQ_DEFAULT,                                              \
        .io_voltage             = 3.3f,                                                            \
        .driver_strength        = SDMMC_DRIVER_STRENGTH_B,                                         \
        .current_limit          = SDMMC_CURRENT_LIMIT_200MA,                                       \
        .init                   = &sdmmc_host_init,                                                \
        .set_bus_width          = &sdmmc_host_set_bus_width,                                       \
        .get_bus_width          = &sdmmc_host_get_slot_width,                                      \
        .set_bus_ddr_mode       = &sdmmc_host_set_bus_ddr_mode,                                    \
        .set_card_clk           = &sdmmc_host_set_card_clk,                                        \
        .set_cclk_always_on     = &sdmmc_host_set_cclk_always_on,                                  \
        .do_transaction         = &sdmmc_host_do_transaction,                                      \
        .deinit_p               = &sdmmc_host_deinit_slot,                                         \
        .io_int_enable          = sdmmc_host_io_int_enable,                                        \
        .io_int_wait            = sdmmc_host_io_int_wait,                                          \
        .command_timeout_ms     = 0,                                                               \
        .get_real_freq          = &sdmmc_host_get_real_freq,                                       \
        .input_delay_phase      = SDMMC_DELAY_PHASE_0,                                             \
        .set_input_delay        = &sdmmc_host_set_input_delay,                                     \
        .dma_aligned_buffer     = NULL,                                                            \
        .pwr_ctrl_handle        = NULL,                                                            \
        .get_dma_info           = NULL,                                                            \
        .check_buffer_alignment = &sdmmc_host_check_buffer_alignment,                              \
        .is_slot_set_to_uhs1    = &sdmmc_host_is_slot_set_to_uhs1,                                 \
    }

#define GPIO_NO (-1)
#define SDMMC_NO_CD GPIO_NO ///< indicates that card detect line is not used
#define SDMMC_NO_WP GPIO_NO ///< indicates that write protect line is not used

#define SDMMC_SLOT_CONFIG_DEF()                                                                    \
    {                                                                                              \
        .clk   = SDIO_SCLK_PIN,                                                                    \
        .cmd   = SDIO_CMD_PIN,                                                                     \
        .d0    = SDIO_DATA0_PIN,                                                                   \
        .d1    = GPIO_NO,                                                                          \
        .d2    = GPIO_NO,                                                                          \
        .d3    = GPIO_NO,                                                                          \
        .d4    = GPIO_NO,                                                                          \
        .d5    = GPIO_NO,                                                                          \
        .d6    = GPIO_NO,                                                                          \
        .d7    = GPIO_NO,                                                                          \
        .cd    = SDMMC_NO_CD,                                                                      \
        .wp    = SDMMC_NO_WP,                                                                      \
        .width = 1,                                                                                \
        .flags = 0,                                                                                \
    }