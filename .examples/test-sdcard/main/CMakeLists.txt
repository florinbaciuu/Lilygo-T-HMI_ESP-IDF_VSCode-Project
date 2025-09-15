
# File: main/CMakeLists.txt
# LilyGO THMI IDF Template Project
cmake_minimum_required(VERSION 3.16)

# Set usual component variables
set(app_sources "app_main.cpp" "init.cpp")
set(app_include_dirs "." "")
set(app_requires driver fatfs spi_flash esp_driver_usb_serial_jtag esp_system heap esp_common esp_psram esp_hw_support)
set(app_priv_requires ${app_requires} esp_bootloader_format nvs_flash esp_wifi console )

set(my_components_add
    ESP32_Resource_Monitor
    ESP32_Command_Line_Interface)

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS ${app_include_dirs}
    REQUIRES ${app_requires} ${my_components_add}
    PRIV_REQUIRES ${app_priv_requires}
    WHOLE_ARCHIVE
)

