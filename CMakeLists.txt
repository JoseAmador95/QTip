cmake_minimum_required(VERSION 3.22.1)
project(qtip VERSION 0.0.0)
enable_language(C)
set(CMAKE_C_STANDARD 17)

find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
)

add_library(
    ${PROJECT_NAME}
    STATIC
        ${CMAKE_CURRENT_LIST_DIR}/source/qtip.c
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

if(CLANG_TIDY_EXE)
    message("Clang tidy found")
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE}"
    )
endif()

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

if(PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(test)
endif()