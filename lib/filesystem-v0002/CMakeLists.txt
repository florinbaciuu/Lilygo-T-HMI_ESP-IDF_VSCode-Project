

set(
    srcs 
    "src/filesystem-os.c"
    "src/eMMC_fs.c"
    "src/FAT_fs.c"
    "src/LITTLE_fs.c"
    "src/SPIF_fs.c"
)

set(
    include_dirs
    "include"
)

set(
    requires
    esp_common
)

set(
    priv_requires
    esp_common
    driver
    esp_timer
    console
    log
    spi_flash
    fatfs
    littlefs
    spiffs
)

idf_component_register(
    SRCS 
    ${srcs}
    INCLUDE_DIRS 
    ${include_dirs}
    REQUIRES
    ${requires}
    PRIV_REQUIRES
    ${priv_requires}
)
