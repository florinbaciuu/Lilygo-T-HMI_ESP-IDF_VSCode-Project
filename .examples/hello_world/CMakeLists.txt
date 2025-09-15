# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly   -Wunused-function
cmake_minimum_required(VERSION 3.16)

set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/main/lv_conf.h")
add_compile_definitions(LV_CONF_PATH="${LV_CONF_PATH}")
add_compile_definitions(LVGL_VERSION_MAJOR=9)

add_compile_options(
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-function>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-variable>
   $<$<COMPILE_LANGUAGE:C>:-Wno-unused-function>
   $<$<COMPILE_LANGUAGE:C>:-Wno-unused-variable>
)

set(EXTRA_COMPONENT_DIRS lib)


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(project-test-espidf)
