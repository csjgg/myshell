cmake_minimum_required(VERSION 3.0)
project(myshell)

set(SRC_LIST
    shell.c
    prompt.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g")

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)
add_executable(shell ${SRC_LIST}) 

target_link_libraries(shell readline)

# add_subdirectory(env)
