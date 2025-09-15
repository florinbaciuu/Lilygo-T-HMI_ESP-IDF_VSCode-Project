
idf_component_register(
    SRCS "app_main.cpp"
    INCLUDE_DIRS "." ""
    REQUIRES driver fatfs esp_lcd_touch_xpt2046 esp_lcd esp_lcd_touch lvgl
    PRIV_REQUIRES
)


