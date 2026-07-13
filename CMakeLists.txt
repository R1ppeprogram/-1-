cmake_minimum_required(VERSION 3.10)
project(ByteReplacer)


set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)


option(BLOCK_SIZE "Block size for file reading" 4096)


add_compile_definitions(BLOCK_SIZE=${BLOCK_SIZE})


add_library(byte_replacer STATIC src/byte_replacer.c)
target_include_directories(byte_replacer PUBLIC include)


add_executable(byte_replace src/main.c)
target_link_libraries(byte_replace byte_replacer)
target_include_directories(byte_replace PRIVATE include)


enable_testing()

add_executable(test_byte_replacer tests/test_byte_replacer.c)
target_link_libraries(test_byte_replacer byte_replacer)
target_include_directories(test_byte_replacer PRIVATE include)

add_test(NAME ByteReplacerTests COMMAND test_byte_replacer)