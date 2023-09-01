add_executable(pcroast pcroast.c wifi.c st7735.c graphics.c)
pico_enable_stdio_usb(pcroast 0)
pico_enable_stdio_uart(pcroast 1)

target_link_libraries(pcroast pico_stdlib pico_cyw43_arch_lwip_sys_freertos FreeRTOS-Kernel-Heap4 ugfx st7735 hardware_spi)
pico_add_extra_outputs(pcroast)
