file(GLOB SDCARD_SRCS SDCard/*.cpp)
file(GLOB Audio_SRCS Audio/*.cpp)
file(GLOB BlueTooth_SRCS Bluetooth/*.cpp)
file(GLOB Input_SRCS Input/*.cpp)
file(GLOB Task_SRCS Task/*.cpp)
file(GLOB Util_SRCS Util/*.cpp)

set(COMPONENT_SRCS ${SDCARD_SRCS} ${Audio_SRCS} ${BlueTooth_SRCS} ${Input_SRCS} ${Task_SRCS} ${Util_SRCS} "hello_world_main.c")
set(COMPONENT_ADD_INCLUDEDIRS "./Audio/" "./Bluetooth/" "./Input/" "./Task/" "./Util/")

register_component()