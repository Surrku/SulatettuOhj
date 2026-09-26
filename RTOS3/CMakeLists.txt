cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(Debugging)

target_sources(app PRIVATE 
    src/main.c
    src/leds.c
    src/btn.c
    src/uart.c
    src/dispatch.c
    src/TimeParser.c
    )
