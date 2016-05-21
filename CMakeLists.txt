
cmake_minimum_required (VERSION 2.6)
project (Selena)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -g")


file(GLOB scc_SRC
    "source/*.cpp"
)

file(GLOB driver_SRC
    "driver/*.cpp"
)

add_library(selena ${scc_SRC})

add_executable(caelinacc ${driver_SRC})
target_link_libraries(caelinacc selena)
target_include_directories (selena PUBLIC include)
