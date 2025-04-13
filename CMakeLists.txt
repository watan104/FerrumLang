cmake_minimum_required(VERSION 3.15)
project(Ferrum LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Derleyici ayarları
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror -pedantic)
endif()

# Platform spesifik ayarlar
if(WIN32)
    add_definitions(-DFERRUM_WINDOWS)
    list(APPEND EXTRA_LIBS kernel32)
elseif(UNIX AND NOT APPLE)
    add_definitions(-DFERRUM_LINUX)
    list(APPEND EXTRA_LIBS pthread dl)
elseif(APPLE)
    add_definitions(-DFERRUM_MACOS)
endif()

# Bootstrap kütüphaneleri
add_library(bootstrap STATIC
    bootstrap/minimal_libc.c
    bootstrap/start.s
)

# Runtime kütüphaneleri
add_library(runtime STATIC
    src/runtime/memory.c
    src/runtime/io.c
    src/runtime/sys.c
)

# Derleyici bileşenleri
add_executable(ferrumc
    src/compiler/main.c
    src/compiler/lexer.c
    src/compiler/parser.c
    src/compiler/ast.c
    src/compiler/codegen.c
    src/compiler/semantic.c
)

# Kütüphane bağlantıları
target_link_libraries(ferrumc PRIVATE bootstrap runtime ${EXTRA_LIBS})

# Include dizinleri
target_include_directories(ferrumc PRIVATE include)
target_include_directories(bootstrap PRIVATE include)
target_include_directories(runtime PRIVATE include)

# Testler
enable_testing()

add_executable(test_lexer
    tests/unit/test_lexer.c
    src/compiler/lexer.c
)
target_link_libraries(test_lexer PRIVATE bootstrap)

add_executable(test_parser
    tests/unit/test_parser.c
    src/compiler/parser.c
    src/compiler/ast.c
)
target_link_libraries(test_parser PRIVATE bootstrap runtime)

add_test(NAME lexer_test COMMAND test_lexer)
add_test(NAME parser_test COMMAND test_parser)

# Kurulum ayarları
install(TARGETS ferrumc DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)