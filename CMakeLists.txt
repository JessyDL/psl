#######################################################################################################################
### Definitions																										###
#######################################################################################################################

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
SET(PSL "Paradigm Standard Library")
set(PSL_PROJECT "psl")
set(LOCAL_PROJECT ${PSL_PROJECT})
project(${LOCAL_PROJECT} VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

option(opt_make_tests "build tests" TRUE)


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
	fmt
	GIT_REPOSITORY
	https://github.com/fmtlib/fmt.git
	GIT_TAG
	6.2.0
)

FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
	FetchContent_Populate(fmt)
	set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE BOOL "")
	add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR})
	unset(CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
endif()

#######################################################################################################################
### Includes 																										###
#######################################################################################################################

list(APPEND INC_IMPL)

list(APPEND PSL_INC uid
	${INC_IMPL}
	allocator
	enum
	)

list(TRANSFORM PSL_INC PREPEND include/psl/)
list(TRANSFORM PSL_INC APPEND .h)

list(APPEND PSL_SRC 
	${INC_IMPL}
	psl
	)
list(TRANSFORM PSL_SRC PREPEND source/)
list(TRANSFORM PSL_SRC APPEND .cpp)

list(APPEND PSL_INCLUDE_DIRECTORIES include)

#######################################################################################################################
### Setup	 																										###
#######################################################################################################################

add_library(${LOCAL_PROJECT} ${PSL_SRC})
target_include_directories(${LOCAL_PROJECT} PUBLIC ${PSL_INCLUDE_DIRECTORIES} ${fmt_INCLUDE_DIRS})
target_link_libraries(${LOCAL_PROJECT} fmt)

target_compile_options(${LOCAL_PROJECT} PUBLIC 
	$<$<CXX_COMPILER_ID:MSVC>:/permissive- /W4>
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/WX>
	)

if(opt_make_tests)
	include(CTest)
	add_subdirectory(tests)
endif()
