cmake_minimum_required(VERSION 3.16.0)
project(MyHTTP)

set(CMAKE_CXX_STANDARD 23)

# we define MY_SOURCES to be a list of all the source files for my project
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(MyHTTP STATIC ${MY_SOURCES})
set_target_properties(MyHTTP PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
target_include_directories(MyHTTP PUBLIC include)

add_executable(SimpleServer examples/SimpleServer.cpp)
set_target_properties(SimpleServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)
target_link_libraries(SimpleServer PRIVATE MyHTTP)

# Enable testing
enable_testing()
add_subdirectory(tests)
