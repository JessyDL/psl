#######################################################################################################################
### Definitions																										###
#######################################################################################################################

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
SET(PSL_TESTS "tests")
set(LOCAL_PROJECT ${PSL_TESTS})
project(${LOCAL_PROJECT} VERSION 0.0.1 LANGUAGES CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

#######################################################################################################################
### Externals 																										###
#######################################################################################################################

include(FetchContent)

FetchContent_Declare(
	googletest
	GIT_REPOSITORY
	https://github.com/google/googletest.git
	GIT_TAG
	v1.10.x
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
	FetchContent_Populate(googletest)
	set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "")
	add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
	unset(CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
endif()

#######################################################################################################################
### Includes 																										###
#######################################################################################################################

list(APPEND PSL_TESTS_INC_SRC)

list(APPEND PSL_TESTS_INC ${PSL_TESTS_INC_SRC})

list(APPEND PSL_TESTS_SRC
	${PSL_TESTS_INC_SRC}
	tests
	allocator
	iterators
	span
	)

list(TRANSFORM PSL_TESTS_INC PREPEND include)
list(TRANSFORM PSL_TESTS_INC APPEND .hpp)
list(TRANSFORM PSL_TESTS_SRC PREPEND source/)
list(TRANSFORM PSL_TESTS_SRC APPEND .cpp)


#######################################################################################################################
### Setup	 																										###
#######################################################################################################################

add_executable(${LOCAL_PROJECT} ${PSL_TESTS_SRC})
target_include_directories(${LOCAL_PROJECT} PUBLIC include)

target_compile_options(${LOCAL_PROJECT} PUBLIC
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/MT>
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/MTd>
	)
	
target_compile_options(${LOCAL_PROJECT} PUBLIC 
	$<$<CXX_COMPILER_ID:MSVC>:/permissive- /W4>
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/WX>
	$<$<CXX_COMPILER_ID:CLANG>:-Wno-error=terminate -Wall -Wextra -pedantic -Wno-unknown-pragmas>
	$<$<CXX_COMPILER_ID:GNU>:-Wno-error=terminate -Wall -Wextra -pedantic -Wno-unknown-pragmas>
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${coverage}>>:--coverage -O0>
	)

target_link_options(${LOCAL_PROJECT} PUBLIC
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${coverage}>>:--coverage>
)

include(GoogleTest)

target_link_libraries(${LOCAL_PROJECT} ${PSL_PROJECT} gtest gmock gtest_main)
gtest_discover_tests(${LOCAL_PROJECT})