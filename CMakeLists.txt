cmake_minimum_required(VERSION 3.10)
project(rforth VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Include directories
include_directories(include)

# Compiler flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -Wextra -DNDEBUG")

# Source files
set(RFORTH_SOURCES
    src/main.c
    src/interpreter.c
    src/compiler.c
    src/dict.c
    src/stack.c
    src/parser.c
    src/builtins.c
    src/codegen.c
    src/runtime.c
)

# Header files
set(RFORTH_HEADERS
    include/rforth.h
    include/dict.h
    include/stack.h
    include/parser.h
    include/compiler.h
)

# Main executable
add_executable(rforth ${RFORTH_SOURCES} ${RFORTH_HEADERS})

# Runtime library for compiled programs
set(RUNTIME_SOURCES
    src/stack.c
    src/dict.c
    src/runtime.c
    src/builtins.c
)

add_library(rforth_runtime STATIC ${RUNTIME_SOURCES})

# Install targets
install(TARGETS rforth DESTINATION bin)
install(TARGETS rforth_runtime DESTINATION lib)
install(DIRECTORY include/ DESTINATION include/rforth)

# Enable testing
enable_testing()

# Add test if test files exist
if(EXISTS "${CMAKE_SOURCE_DIR}/tests")
    add_subdirectory(tests)
endif()

# Custom target to clean bin and lib directories
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/lib
    COMMENT "Cleaning bin and lib directories"
)

# Custom target to run rforth in REPL mode
add_custom_target(repl
    COMMAND ${CMAKE_SOURCE_DIR}/bin/rforth -r
    DEPENDS rforth
    COMMENT "Running rforth in REPL mode"
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")