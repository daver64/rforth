#ifndef CONFIG_H
#define CONFIG_H

/* RForth Configuration Constants */

/* Version Information */
#define RFORTH_VERSION_MAJOR 1
#define RFORTH_VERSION_MINOR 0
#define RFORTH_VERSION_PATCH 0

/* Stack Configuration */
#define DEFAULT_STACK_SIZE 256
#define DEFAULT_RETURN_STACK_SIZE 256
#define MAX_STACK_SIZE 8192
#define MIN_STACK_SIZE 16

/* Dictionary and Parser Configuration */
#define MAX_WORD_LENGTH 64
#define MAX_INPUT_LENGTH 1024
#define MAX_DEFINITION_LENGTH 4096

/* Buffer Sizes */
#define MAX_FILENAME_LENGTH 256
#define MAX_NUMBER_STRING_LENGTH 32
#define COMPILE_BUFFER_INITIAL_SIZE 1024
#define COMPILE_BUFFER_GROWTH_FACTOR 2
#define IO_BUFFER_SIZE 1024

/* File Extensions */
#define C_FILE_EXTENSION ".c"
#define EXECUTABLE_EXTENSION ""

/* Compiler Configuration */
#define DEFAULT_COMPILER "gcc"
#define COMPILER_FLAGS "-O2", "-std=c99", "-Wall", "-Wextra"
#define MAX_COMPILER_ARGS 16

/* Memory Management */
#define INITIAL_DICT_CAPACITY 128
#define DICT_GROWTH_FACTOR 2

/* I/O Configuration */
#define DEFAULT_IO_TIMEOUT_MS 1000
#define MAX_IO_RETRY_COUNT 3

/* Debugging and Development */
#ifndef NDEBUG
#define RFORTH_DEBUG 1
#define ENABLE_MEMORY_TRACKING 1
#else
#define RFORTH_DEBUG 0
#define ENABLE_MEMORY_TRACKING 0
#endif

/* Platform-specific configurations */
#ifdef _WIN32
    #define PATH_SEPARATOR "\\"
    #define EXECUTABLE_EXTENSION ".exe"
#else
    #define PATH_SEPARATOR "/"
    #define EXECUTABLE_EXTENSION ""
#endif

#endif /* CONFIG_H */