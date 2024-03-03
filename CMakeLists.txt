# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Dazzle)

# Set the c++ standard version
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add all the required sub directories
# TODO: Add an option() check to whether build the examples or not.
add_subdirectory(examples)