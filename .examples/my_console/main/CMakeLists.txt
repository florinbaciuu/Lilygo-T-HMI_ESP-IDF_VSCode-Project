idf_component_register(SRCS "app_main.cpp" "console_settings.c"
                    PRIV_REQUIRES spi_flash esp_wifi nvs_flash console esp_driver_uart esp_driver_usb_serial_jtag fatfs esp_psram log
                    INCLUDE_DIRS ""
                    REQUIRES cmd_nvs cmd_system cmd_wifi)
