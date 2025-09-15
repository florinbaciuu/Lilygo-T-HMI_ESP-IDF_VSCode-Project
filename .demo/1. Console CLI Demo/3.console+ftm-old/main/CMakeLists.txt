

cmake_minimum_required(VERSION 3.16)
  
# --------------------------------------------------- #
set(app_sources         "app_main.cpp" 
                        "init.cpp")
# --------------------------------------------------- #
set(app_include_dirs    "." 
                        "")
# --------------------------------------------------- #
set(app_requires        driver 
                        fatfs 
                        spi_flash 
                        esp_driver_usb_serial_jtag 
                        esp_lcd_touch_xpt2046 
                        esp_lcd esp_lcd_touch 
                        lvgl 
                        esp_system 
                        heap 
                        esp_common 
                        esp_psram 
                        esp_hw_support
                        esp_lvgl_port
                        esp-file-iterator
                        button
                        ccomp_timer
                        fmt
                        cmake_utilities
                        #esp32_display_panel
                        )
# --------------------------------------------------- #
set(app_priv_requires   ${app_requires} 
                        esp_bootloader_format
                        nvs_flash
                        esp_wifi
                        console
                        )
# --------------------------------------------------- #
# Set my components
# These are components that are not part of the IDF 
# but are used in this project.
# Put this components in /lib after set 
# set(EXTRA_COMPONENT_DIRS lib) in root/CMakeLists.txt
set(my_components_add   ESP32_Command_Line_Interface
)
# --------------------------------------------------- #
idf_component_register(
    SRCS                ${app_sources}
    INCLUDE_DIRS        ${app_include_dirs}
    REQUIRES            ${app_requires} 
                        ${my_components_add}
    PRIV_REQUIRES       ${app_priv_requires}
    WHOLE_ARCHIVE       True
    )

# --------------------------------------------------- #
