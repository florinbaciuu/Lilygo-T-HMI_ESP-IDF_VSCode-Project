
# ==================================== #
set(hello_cmd_srcs # Se adauga modulul hello
    "modules/hello_cmd/hello_cmd.c")
set(hello_cmd_includes
    "modules/hello_cmd")
# ==================================== #
set(restart_srcs # Se adauga restart
    "modules/restart_cmd/restart_cmd.c")
set(restart_includes "modules/restart_cmd")
# ==================================== #
set(tasks_srcs # Se adauga task info
    "modules/tasks_cmd/tasks_cmd.c")
set(tasks_includes
    "modules/tasks_cmd")
# ==================================== #
set(uptime_srcs # Se adauga uptime
    "modules/uptime_cmd/uptime_cmd.c")
set(uptime_includes
    "modules/uptime_cmd")
# ==================================== #
set(info_srcs # Se adauga modulul info
    "modules/info_cmd/info_cmd.c"
    "modules/info_cmd/funct.c"
    "modules/info_cmd/stack.c")
set(info_includes
    "modules/info_cmd")
# ==================================== #
set(nvs_cmd_srcs # Se adauga modulul nvs
    "modules/nvs_cmd/nvs_cmd.c")
set(nvs_cmd_includes
    "modules/nvs_cmd")
# ==================================== #
set(wifi_cmd_srcs # Se adauga modulul wifi
    "modules/wifi_cmd/wifi_join_cmd.c")
set(wifi_cmd_includes
    "modules/wifi_cmd")
# ==================================== #
set(set_cmd_srcs # Se adauga modulul set
    "modules/set_cmd/set_cmd.c"
    "modules/set_cmd/set_log.c")
set(set_cmd_includes
    "modules/set_cmd")
# ==================================== #
set(perfmon_cmd_srcs # Se adauga modulul wifi
    "modules/perfmon_cmd/perfmon_cmd.c")
set(perfmon_cmd_includes
    "modules/perfmon_cmd")
# ==================================== #

# ------------------------------ #

# Aduna toate modulele intr-un singur #
set(modules_srcs
    ${hello_cmd_srcs}
    ${restart_srcs}
    ${tasks_srcs}
    ${uptime_srcs}
    ${info_srcs}
    ${nvs_cmd_srcs}
    ${wifi_cmd_srcs}
    ${set_cmd_srcs}
    ${perfmon_cmd_srcs}
)
## ------------------
set(modules_includes
    ${hello_cmd_includes}
    ${restart_includes}
    ${tasks_includes}
    ${uptime_includes}
    ${info_includes}
    ${nvs_cmd_includes}
    ${wifi_cmd_includes}
    ${set_cmd_includes}
    ${perfmon_cmd_includes}
)
## ------------------
set(modules_priv_includes
    ${hello_cmd_includes}
    ${restart_includes}
    ${tasks_includes}
    ${uptime_includes}
    ${info_includes}
    ${nvs_cmd_includes}
    ${wifi_cmd_includes}
    ${set_cmd_includes}
    ${perfmon_cmd_includes}
)
## ------------------

# ------------------------------- #

idf_component_register(
    SRCS
    "src/one-cli.c"
    "src/init.c"
    "src/config.c"
    ${modules_srcs}
    ## ------------------
    INCLUDE_DIRS
    "include"
    "modules"
    "src"
    "."
    ${modules_includes}
    ## ------------------
    PRIV_INCLUDE_DIRS
    ${modules_priv_includes}
    ## ------------------
    REQUIRES
    perfmon
    esp_common
    driver
    console
    log

    ## ------------------
    PRIV_REQUIRES
    perfmon
    esp_timer
    driver
    freertos
    console
    esp_hw_support
    spi_flash
    esp_system
    heap
    esp_common
    esp_psram
    esp_hw_support
    nvs_flash
    esp_wifi
)

# ------------------------------- #

set( # Asta adaug-o separat:
    KCONFIG_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/Kconfig"
)

# ------------------------------- #


