set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")


#add_definitions(-DLV_CONF_INCLUDE_SIMPLE)

idf_component_register(SRCS "display_hmi.c" "initSequence.c" "power_driver.cpp" "touch_driver.cpp" "app_main.cpp" "lv_ui.cpp"
                    PRIV_REQUIRES esp_lcd
                    INCLUDE_DIRS ""
                    REQUIRES lvgl esp_lcd_touch_xpt2046)

#add_compile_definitions(LV_CONF_PATH="${LV_CONF_PATH}")
#add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
