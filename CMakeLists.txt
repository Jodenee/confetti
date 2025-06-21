cmake_minimum_required(VERSION 3.10)
project(confetti VERSION 1.0.0 LANGUAGES C)

# Set C standards for the project
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)

# Enables option for building either static or dynamic library files
option(BUILD_SHARED_LIBS "Build confetti as a shared library instead of static" OFF)

# Add the src sub directory
add_subdirectory("src")
