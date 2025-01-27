cmake_minimum_required(VERSION 3.10)

# Project name
project(Compiler_Project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Include LLVM directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(/usr/lib/llvm-18/include)  # Added LLVM include path
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Include project directories
include_directories(src/lexer src/parser src/utils)

# Source files
file(GLOB_RECURSE SOURCES
    src/lexer/*.cpp
    src/parser/*.cpp
    src/utils/*.cpp
    src/LLVM/*.cpp
    src/main.cpp
)

# Create the executable
add_executable(Compiler_Project ${SOURCES})

# Link LLVM libraries
llvm_map_components_to_libnames(LLVM_LIBS support core irreader)
target_link_libraries(Compiler_Project ${LLVM_LIBS})

# Additional compiler flags
set(CMAKE_CXX_FLAGS "-g -Wall")
