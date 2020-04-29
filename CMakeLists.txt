cmake_minimum_required(VERSION 3.11)
SET(PSL "Paradigm Supplemental Library")
set(PSL_PROJECT "psl")
set(LOCAL_PROJECT ${PSL_PROJECT})
project($LOCAL_PROJECT VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

option(opt_make_tests      "build tests"       TRUE)
option(opt_asan      "enable asan"       FALSE)
option(opt_coverage "enable code coverage" TRUE)

add_compile_options(-Wall -Wextra -pedantic)

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

list(APPEND PSL_INCLUDE_DIRS inc fwd)

if(opt_coverage)
  SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
endif()

add_library(${LOCAL_PROJECT} ${SRC})
target_include_directories(${LOCAL_PROJECT} PUBLIC ${PSL_INCLUDE_DIRS})


if(opt_asan)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer -ggdb -fsanitize=undefined -fsanitize=leak)
  #target_link_libraries(${LOCAL_PROJECT}  -fsanitize=address -ggdb -fsanitize=undefined -fsanitize=leak)
endif()

if(opt_make_tests)
  include(CTest)
  add_subdirectory(tests)
endif()