cmake_minimum_required(VERSION 3.22.1)
project(qtip VERSION 0.0.0)
enable_language(C CXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)
add_library(${PROJECT_NAME} STATIC ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC include)

option(QTIP_REDUCED_API "Reduce the public API to save memory" OFF)
option(QTIP_DISABLE_LOCK "Disable the queue lock" OFF)
option(QTIP_DISABLE_TELEMETRY "Disable queue telemetry to save memory" OFF)
set(QTIP_SIZE_TYPE size_t CACHE STRING "Type of the max number of items in the queue")

if(QTIP_REDUCED_API)
    target_compile_definitions(${PROJECT_NAME} PUBLIC REDUCED_API)
endif()

if(QTIP_DISABLE_LOCK)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_LOCK)
endif()

if(QTIP_DISABLE_TELEMETRY)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_TELEMETRY)
endif()

if(DEFINED QTIP_SIZE_TYPE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SIZE_TYPE=${QTIP_SIZE_TYPE})
endif()

set(STANDALONE_BUILD CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

if(${STANDALONE_BUILD})
    add_subdirectory(thirdparty/unity EXCLUDE_FROM_ALL)
    add_subdirectory(test EXCLUDE_FROM_ALL)
endif()