cmake_minimum_required(VERSION 3.12...3.26)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release default")
endif()

project(3d-renderer LANGUAGES CXX CUDA)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CUDA_SEPARABLE_COMPILATION ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|Intel")
  add_compile_options(-Wall)
endif()

find_package(CUDAToolkit REQUIRED)
aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCES)

find_package(OpenMP COMPONENTS CXX)

set(INCLUDES ${PROJECT_SOURCE_DIR}/include)

add_library(renderer_lib STATIC ${SOURCES})
target_link_libraries(renderer_lib PRIVATE CUDA::cudart)
target_include_directories(renderer_lib PUBLIC ${INCLUDES})

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE renderer_lib)

enable_testing()
add_subdirectory(tests)