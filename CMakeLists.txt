cmake_minimum_required(VERSION 3.10)

# Project name
project(Compiler_Project)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(src/lexer src/parser src/utils)

# Source files
file(GLOB_RECURSE SOURCES
    src/lexer/*.cpp
    src/parser/*.cpp
    src/main.cpp
)

# Create the executable
add_executable(LexerParser ${SOURCES})

set(CMAKE_CXX_FLAGS "-g -Wall")

