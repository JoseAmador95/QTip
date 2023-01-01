set(PROJECT_UT ${PROJECT_NAME}_ut)
file(GLOB TESTS ${CMAKE_CURRENT_SOURCE_DIR}/test_*.c)
add_library(${PROJECT_UT} STATIC ${TESTS})
target_link_libraries(${PROJECT_UT} ${PROJECT_NAME} unity)