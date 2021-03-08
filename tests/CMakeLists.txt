#######################################################################################################################
### Definitions																										###
#######################################################################################################################

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
SET(PSL_TESTS ${PSL_PROJECT}_tests)
set(LOCAL_PROJECT ${PSL_TESTS})
project(${LOCAL_PROJECT} VERSION 0.0.1 LANGUAGES CXX)

include(CTest)
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

add_subdirectory(externals/litmus)

#######################################################################################################################
### Includes 																										###
#######################################################################################################################

list(APPEND PSL_TESTS_INC_SRC)

list(APPEND PSL_TESTS_INC 
	${PSL_TESTS_INC_SRC}
	types
)

list(APPEND PSL_TESTS_SRC
	${PSL_TESTS_INC_SRC}
	tests
	algorithms
	allocator
	array
	expected
	iterators
	optional
	span
	random
	#uid

	containers/string_view
	
	serialization/property
	serialization/format
	)

list(TRANSFORM PSL_TESTS_INC PREPEND include/tests/)
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

target_link_libraries(${LOCAL_PROJECT} ${PSL_PROJECT} litmus)

if(coverage)
	include(CodeCov)
	set(LCOV_BASE_DIR "'${CMAKE_SOURCE_DIR}'")
	set(LCOV_REMOVE_EXTRA "'${CMAKE_SOURCE_DIR}/tests/*'" "'${CMAKE_SOURCE_DIR}/include/psl/fwd/*'" "'${CMAKE_SOURCE_DIR}/include/psl/details/*'")
	code_coverage(${LOCAL_PROJECT}_coverage ${LOCAL_PROJECT} coverage)
endif()
