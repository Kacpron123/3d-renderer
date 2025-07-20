file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp") # This will find VecTest.cpp

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(renderer_test_runner ${TEST_SOURCES})

target_link_libraries(renderer_test_runner PRIVATE
    gtest_main
    gtest       
    renderer_lib
)

target_include_directories(renderer_test_runner PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_test(NAME RunRendererTests COMMAND renderer_test_runner)