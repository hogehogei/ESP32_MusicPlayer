file(GLOB SDCARD_SRCS SDCard/*.cpp)
file(GLOB BlueTooth_SRCS bluetooth/*.cpp)

set(COMPONENT_SRCS ${SDCARD_SRCS} ${BlueTooth_SRCS} "hello_world_main.c")
set(COMPONENT_ADD_INCLUDEDIRS "./util/")

register_component()