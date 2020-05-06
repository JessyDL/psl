cmake_minimum_required(VERSION 3.11)
SET(PSL_TESTS "tests")
set(LOCAL_PROJECT ${PSL_TESTS})
project($LOCAL_PROJECT VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

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

list(APPEND PSL_TESTS_INC_SRC)

list(APPEND PSL_TESTS_INC ${PSL_TESTS_INC_SRC})

list(APPEND PSL_TESTS_FWD)

list(APPEND PSL_TESTS_SRC 
	${PSL_TESTS_INC_SRC}
	tests
	algorithms
	array
	allocators
	dequeue
	queue)

list(TRANSFORM PSL_TESTS_INC PREPEND inc/tests/)
list(TRANSFORM PSL_TESTS_INC APPEND .h)
list(TRANSFORM PSL_TESTS_FWD PREPEND fwd/tests/fwd/)
list(TRANSFORM PSL_TESTS_FWD APPEND .h)
list(TRANSFORM PSL_TESTS_SRC PREPEND src/)
list(TRANSFORM PSL_TESTS_SRC APPEND .cpp)

add_executable(${LOCAL_PROJECT} ${PSL_TESTS_SRC})
target_include_directories(${LOCAL_PROJECT} PUBLIC inc fwd)

include(GoogleTest) 

target_link_libraries(${LOCAL_PROJECT} ${PSL_PROJECT} gtest gmock gtest_main)

#gtest_discover_tests(${LOCAL_PROJECT} TEST_LIST tests_target)