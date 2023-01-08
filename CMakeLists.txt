cmake_minimum_required(VERSION 3.22.1)
project(qtip VERSION 0.0.0)
enable_language(C CXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)
add_library(${PROJECT_NAME} STATIC ${SRC})
target_include_directories(${PROJECT_NAME} PUBLIC include)

if(DEFINED QTIP_REDUCED_API)
    target_compile_definitions(${PROJECT_NAME} PUBLIC REDUCED_API)
endif()

if(DEFINED QTIP_DISABLE_LOCK)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_LOCK)
endif()

if(DEFINED QTIP_DISABLE_TELEMETRY)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DISABLE_TELEMETRY)
endif()

if(DEFINED QTIP_SIZE_TYPE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SIZE_TYPE=${QTIP_SIZE_TYPE})
endif()

add_subdirectory(thirdparty/unity EXCLUDE_FROM_ALL)
add_subdirectory(test EXCLUDE_FROM_ALL)