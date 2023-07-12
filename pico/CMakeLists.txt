add_executable(pcroast pcroast.c)
target_link_libraries(pcroast pico_stdlib pico_stdio_usb)
pico_add_extra_outputs(pcroast)
