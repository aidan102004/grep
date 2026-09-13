cmake_minimum_required(VERSION 3.10)
project(grep C CXX)
 
# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
 
# Source files
set(SOURCES
    src/main.cpp
    src/grep.cpp
    src/regexengine.cpp
    # src/matcher.cpp
    # src/util.cpp
)
 
# Create executable
add_executable(grep ${SOURCES})
 
# Compiler flags
if(APPLE)
    target_compile_options(grep PRIVATE -Wall -Wextra -Wpedantic)
elseif(UNIX)
    target_compile_options(grep PRIVATE -Wall -Wextra -Wpedantic)
endif()
 
# Output directory
set_target_properties(grep PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)