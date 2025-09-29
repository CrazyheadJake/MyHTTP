cmake_minimum_required(VERSION 3.26.3)
project(MyHTTP)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# we define MY_SOURCES to be a list of all the source files for my project
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(MyHTTP STATIC ${MY_SOURCES})
target_include_directories(MyHTTP PUBLIC include)

add_executable(SimpleServer examples/SimpleServer.cpp)
target_link_libraries(SimpleServer PRIVATE MyHTTP)

# Enable testing
enable_testing()
add_subdirectory(tests)
