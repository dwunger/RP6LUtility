cmake_minimum_required(VERSION 3.13)

# Normally you would set a static project name:
# project(markdown-project)
# This will dynamically accomplish the same:
get_filename_component(ROOT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME "${ROOT_DIR_NAME}-project")
project(${PROJECT_NAME})


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)

if(MSVC)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()
# Configuration for GoogleTest
configure_file(GoogleTestLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
enable_testing()

include_directories("${PROJECT_SOURCE_DIR}/include")
#include_directories("${PROJECT_SOURCE_DIR}/../../../include")
message("Include directory: ${PROJECT_SOURCE_DIR}/include")
message("Include directory: ${PROJECT_SOURCE_DIR}/../../../include")

file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.c")
list(REMOVE_ITEM sources "${PROJECT_SOURCE_DIR}/src/main.c")

file(GLOB tests "${PROJECT_SOURCE_DIR}/test/unit/*.cpp")
add_executable("all_tests" ${sources} ${tests})
target_link_libraries("all_tests" gtest_main ${CMAKE_THREAD_LIBS_INIT})
add_test(NAME all_tests COMMAND "all_tests")
