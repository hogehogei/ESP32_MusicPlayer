file(GLOB SDCARD_SRCS SDCard/*.cpp)

set(COMPONENT_SRCS "hello_world_main.c" "http_sample.cpp")
set(COMPONENT_SRCS ${SDCARD_SRCS})
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()