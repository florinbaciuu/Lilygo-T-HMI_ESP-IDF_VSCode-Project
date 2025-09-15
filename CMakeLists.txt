# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly   -Wunused-function
cmake_minimum_required(VERSION 3.16)

set(
   LV_CONF_PATH
   "${CMAKE_SOURCE_DIR}/main/lv_conf.h"
)

add_compile_definitions(LV_CONF_PATH="${LV_CONF_PATH}")
add_compile_definitions(LVGL_VERSION_MAJOR=9)

add_compile_options(
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-function>
   $<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-variable>
   $<$<COMPILE_LANGUAGE:C>:-Wno-unused-function>
   $<$<COMPILE_LANGUAGE:C>:-Wno-unused-variable>
)

# Seteaza folder extra pt componente
set(
   EXTRA_COMPONENT_DIRS
   lib
)

# chestii generale ccache
find_program(CCACHE_PROGRAM ccache)

if(CCACHE_PROGRAM)
   message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
   set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
   set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# aici se încarcă magia IDF
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# acum poți folosi funcțiile ESP-IDF deoarece ai incarcat magia
idf_build_set_property(MINIMAL_BUILD ON)

project(esp32s3-console-project)
