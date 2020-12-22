cmake_minimum_required(VERSION 3.18)

project(crypto)

file(GLOB source_files code/*.c)

if(MSVC)
else()
    add_compile_options(-Wall -Werror -Wno-discarded-qualifiers)
endif()
add_executable(crypto ${source_files})
