

set(
    srcs 
    "src/button.cpp"
)

set(
    include_dirs
    "."
    "include"
)


idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS ${include_dirs}
    REQUIRES driver esp_timer
)
