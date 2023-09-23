cmake_minimum_required(VERSION 3.20)

set(HOST_BUILD ${HOST_BUILD})
if (NOT DEFINED HOST_BUILD)
    set(FREERTOS_PORT GCC_RP2040)
    set(PICO_BOARD pico_w)
    include(vendor/pico-sdk/pico_sdk_init.cmake)
    include(vendor/free-rtos/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
endif()

project(pcroast
    VERSION 0.2.0
    DESCRIPTION "Control for Reflow Ovens"
    HOMEPAGE_URL https://github.com/zoftko/pcroast
    LANGUAGES C CXX ASM
)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/vendor/ugfx/cmake")
find_package(ugfx REQUIRED)
add_library(st7735 INTERFACE)
target_sources(st7735 INTERFACE vendor/ugfx/drivers/gdisp/ST7735/gdisp_lld_ST7735.c)


add_compile_definitions("DEBUG_BUILD=$<CONFIG:Debug>")
add_compile_definitions("PROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")
add_compile_definitions("PROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")

include_directories(include)
add_subdirectory(src)

if (NOT DEFINED HOST_BUILD)
    pico_sdk_init()
    include_directories(pico config vendor/ugfx/drivers/gdisp/ST7735)
    add_subdirectory(pico)
endif()

if (DEFINED HOST_BUILD)
    add_subdirectory(vendor/googletest)
    enable_testing()
    add_subdirectory(tests)
endif()
