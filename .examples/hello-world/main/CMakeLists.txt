set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")


idf_component_register(SRCS  "app_main.cpp" 
                    PRIV_REQUIRES 
                    INCLUDE_DIRS ""
                    REQUIRES lvgl)

