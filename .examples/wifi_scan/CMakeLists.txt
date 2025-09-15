idf_component_register(SRCS "app_main.cpp"
                    PRIV_REQUIRES spi_flash esp_wifi nvs_flash
                    INCLUDE_DIRS ""
                    REQUIRES esp_psram log)
