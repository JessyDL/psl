cmake_minimum_required(VERSION 3.11)
SET(PSL "Paradigm Supplemental Library")
set(PSL_NAMESPACE "psl")
set(LOCAL_PROJECT $PSL)
project($LOCAL_PROJECT VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

option(opt_make_executable "create testable executable" TRUE)

add_compile_options(-Wall -Wextra -pedantic -fsanitize=address -fno-omit-frame-pointer -ggdb -fsanitize=undefined -fsanitize=leak)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

list(APPEND INC_IMPL uid)

list(APPEND INC uid ${INC_IMPL} details/fixed_string details/metaprogramming allocator algorithms array concepts config enum exceptions literals range static_array string)
list(TRANSFORM INC PREPEND inc/psl/)
list(TRANSFORM INC APPEND .h)

list(APPEND FWD)
list(TRANSFORM FWD PREPEND fwd/psl/fwd/)
list(TRANSFORM FWD APPEND .h)

list(APPEND SRC ${INC_IMPL})
list(TRANSFORM SRC PREPEND src/psl/)
list(TRANSFORM SRC APPEND .cpp)

if(opt_make_executable)
    list(APPEND SRC src/main.cpp)
    add_executable(psl ${SRC})
    target_include_directories(psl PUBLIC inc fwd)
else()
    add_library(psl ${SRC} ${INC} ${FWD})
endif()

target_link_libraries(psl
  -fsanitize=address -ggdb -fsanitize=undefined -fsanitize=leak
)