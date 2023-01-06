cmake_minimum_required(VERSION 3.22.1)
project(qlib VERSION 0.0.0)
enable_language(C CXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)
add_library(${PROJECT_NAME} STATIC ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(thirdparty/unity EXCLUDE_FROM_ALL)
add_subdirectory(test EXCLUDE_FROM_ALL)