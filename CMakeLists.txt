cmake_minimum_required(VERSION 3.11)
SET(PSL "Paradigm Supplemental Library")
set(PSL_PROJECT "psl")
set(LOCAL_PROJECT ${PSL_PROJECT})
project(${LOCAL_PROJECT} VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

option(opt_make_tests      "build tests"       TRUE)
option(opt_asan      "enable asan"       FALSE)
option(opt_coverage "enable code coverage" TRUE)
option(opt_iwyu "enable include what you use" FALSE)

add_compile_options(-Wall -Wextra -pedantic)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

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

list(APPEND INC_IMPL uid ecs/state)

list(APPEND INC uid
	${INC_IMPL}
	details/fixed_string
	details/metaprogramming
	serialization/property
	serialization/serialization
	serialization/versioning
	ecs/entity
	ecs/details/staged_sparse_component_array
	ecs/details/component_info
	allocator
	algorithms
	array
	concepts
	config
	dequeue
	enum
	exceptions
	literals
	queue
	range
	static_array
	vec
	string)

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

if(opt_asan)
	add_compile_options(-fsanitize=address -fno-omit-frame-pointer -ggdb -fsanitize=undefined -fsanitize=leak)
	add_link_options(-fsanitize=address -fno-omit-frame-pointer -ggdb -fsanitize=undefined -fsanitize=leak)
endif()

add_library(${LOCAL_PROJECT} ${SRC})
target_include_directories(${LOCAL_PROJECT} PUBLIC ${PSL_INCLUDE_DIRS} ${fmt_INCLUDE_DIRS})
target_link_libraries(${LOCAL_PROJECT} fmt)


if(opt_make_tests)
	include(CTest)
	add_subdirectory(tests)
endif()

if(opt_iwyu)
	find_program(iwyu_path NAMES include-what-you-use iwyu)
	if(NOT iwyu_path)
	message(FATAL_ERROR "Could not find the program include-what-you-use")
	endif()

	set_target_properties(${LOCAL_PROJECT} PROPERTIES CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()

add_subdirectory(assembly)