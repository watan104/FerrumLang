cmake_minimum_required(VERSION 3.15)
project(Ferrum LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler settings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Platform specific settings
if(WIN32)
    add_definitions(-DFERRUM_WINDOWS)
    list(APPEND EXTRA_LIBS kernel32)
elseif(UNIX AND NOT APPLE)
    add_definitions(-DFERRUM_LINUX)
    list(APPEND EXTRA_LIBS pthread dl)
elseif(APPLE)
    add_definitions(-DFERRUM_MACOS)
endif()

# Runtime library
add_library(runtime STATIC
    src/runtime/memory.c
    src/runtime/io.c
    src/runtime/sys.c
)

# Compiler components
add_executable(ferrumc
    src/compiler/main.c
    src/compiler/lexer.c
    src/compiler/parser.c
    src/compiler/parser_concurrency.c
    src/compiler/ast.c
    src/compiler/codegen.c
    src/compiler/common.c
    src/compiler/ferror.c
)

# Library links
target_link_libraries(ferrumc PRIVATE runtime ${EXTRA_LIBS})

# Include directories
target_include_directories(ferrumc PRIVATE include)
target_include_directories(runtime PRIVATE include)

# Installation settings
install(TARGETS ferrumc DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)