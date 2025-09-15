

# Set usual component variables
set(app_sources "app_main.cpp")
set(app_include_dirs "." "")
set(app_requires driver fatfs spi_flash esp_driver_usb_serial_jtag esp_lcd_touch_xpt2046 esp_lcd esp_lcd_touch lvgl ESP32_Resource_Monitor cmd_nvs cmd_system cmd_wifi)
set(app_priv_requires esp_bootloader_format nvs_flash esp_wifi console)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS ${app_include_dirs}
    REQUIRES  ${app_requires}
    PRIV_REQUIRES ${app_priv_requires}
    WHOLE_ARCHIVE
)

