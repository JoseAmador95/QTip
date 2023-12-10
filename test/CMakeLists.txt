include(FetchContent)

FetchContent_Declare(
    unity_repo
    GIT_REPOSITORY  https://github.com/ThrowTheSwitch/Unity.git
    GIT_TAG         v2.5.2
)
FetchContent_MakeAvailable(unity_repo)

set(
    SANITIZER_FLAGS
    -fsanitize=address
    -fsanitize=leak
    -fsanitize=undefined
    -fno-sanitize-recover
    -fno-omit-frame-pointer
)

add_executable(test_qtip ${CMAKE_CURRENT_LIST_DIR}/test_qtip.c)
target_compile_options(test_qtip PUBLIC ${SANITIZER_FLAGS})
target_link_options(test_qtip PUBLIC ${SANITIZER_FLAGS})
target_link_libraries(test_qtip PUBLIC unity qtip)
add_test(NAME qtip COMMAND test_qtip)