
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g -mfloat-abi=hard")
SET(CMAKE_C_COMPILER $ENV{DEVKITARM}/bin/arm-none-eabi-gcc)
SET(CMAKE_CXX_COMPILER $ENV{DEVKITARM}/bin/arm-none-eabi-g++)
SET(CMAKE_AR $ENV{DEVKITARM}/bin/arm-none-eabi-ar)
SET(CMAKE_RANLIB $ENV{DEVKITARM}/bin/arm-none-eabi-ranlib)

file(GLOB scc_SRC
    "*.cpp"
)

file(GLOB scc_INC
    "../include/*.h"
)

add_library(selena ${scc_SRC})

target_include_directories (selena PUBLIC ../include)
install(TARGETS selena DESTINATION $ENV{DEVKITPRO}/libctru/lib)
install(FILES ${scc_INC} DESTINATION $ENV{DEVKITPRO}/libctru/include/selena)
