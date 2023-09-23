set(API_IPV4_ADDR "" CACHE STRING "IPV4 address to use for IOT functionality")
set(API_PORT "8000" CACHE STRING "TCP port used to connect with the IOT service")
set(WIFI_TIMEOUT "10000" CACHE STRING "Milliseconds to wait for a WiFi connection to be established")
set(DEBUG_NO_HTTP_API "0" CACHE STRING "Print out would be HTTP body instead of performing HTTP call to the API")

if (NOT API_IPV4_ADDR)
    message(FATAL_ERROR "The API's IPV4 address must be specified")
endif ()

if (NOT WIFI_SSID)
    message(FATAL_ERROR "WiFi's SSID must be specified")
endif ()

if (NOT WIFI_PSK)
    message(FATAL_ERROR "WiFi's PSK must be specified")
endif ()

string(REGEX MATCH "([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)" IPV4_ADDR_OUT ${API_IPV4_ADDR})
add_compile_definitions("API_IPV4_B1=${CMAKE_MATCH_1}")
add_compile_definitions("API_IPV4_B2=${CMAKE_MATCH_2}")
add_compile_definitions("API_IPV4_B3=${CMAKE_MATCH_3}")
add_compile_definitions("API_IPV4_B4=${CMAKE_MATCH_4}")

add_compile_definitions("WIFI_TIMEOUT=${WIFI_TIMEOUT}")
add_compile_definitions("WIFI_SSID=\"${WIFI_SSID}\"")
add_compile_definitions("WIFI_PSK=\"${WIFI_PSK}\"")

add_compile_definitions("API_IPV4=\"${API_IPV4_ADDR}\"")
add_compile_definitions("API_PORT=${API_PORT}")

add_executable(pcroast pcroast.c wifi.c st7735.c graphics.c control.c)
pico_enable_stdio_usb(pcroast 0)
pico_enable_stdio_uart(pcroast 1)

target_link_libraries(
        pcroast
        pico_stdlib
        pico_cyw43_arch_lwip_sys_freertos
        FreeRTOS-Kernel-Heap4
        ugfx
        st7735
        hardware_spi
        max6675
        hardware_pwm
        beeper
        pid
)
pico_add_extra_outputs(pcroast)
