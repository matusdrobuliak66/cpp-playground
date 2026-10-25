cmake_minimum_required(VERSION 3.16)
project(cpp_course LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 00 - Setup and the Compile Step
add_executable(00_setup 00_setup/hello.cpp)
target_compile_options(00_setup PRIVATE -Wall -Wextra)

# 01 - Headers, Translation Units, Linking
add_executable(01_headers_linking
    01_headers_linking/main.cpp
    01_headers_linking/mathutils.cpp
)
target_compile_options(01_headers_linking PRIVATE -Wall -Wextra)

# 02 - Static Types
add_executable(02_static_types 02_static_types/main.cpp)
target_compile_options(02_static_types PRIVATE -Wall -Wextra)

# 03 - Stack and Heap
add_executable(03_stack_heap 03_stack_heap/main.cpp)
target_compile_options(03_stack_heap PRIVATE -Wall -Wextra)
