#######################################################################################################################
### Definitions																										###
#######################################################################################################################

cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
SET(PSL "Paradigm Standard Library")
set(PSL_PROJECT "psl")
set(LOCAL_PROJECT ${PSL_PROJECT})
project(${LOCAL_PROJECT} VERSION 0.0.1 LANGUAGES CXX)
include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 23)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(PSL_AVX2 "support avx2" TRUE)
option(PSL_TESTS "build tests" TRUE)
option(PSL_DOCUMENTATION "build documentation" TRUE)
cmake_dependent_option(PSL_COVERAGE "show the code coverage of the tests" FALSE "PSL_TESTS" FALSE)


if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

#######################################################################################################################
### Externals 																										###
#######################################################################################################################

find_package(Python3 REQUIRED)
add_subdirectory(externals/fmt)

#######################################################################################################################
### Includes 																										###
#######################################################################################################################

list(APPEND INC_IMPL
	allocator
	)

list(APPEND PSL_GENERATED_INC
	config
	psl
	)

list(TRANSFORM PSL_GENERATED_INC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/include/psl/)
list(TRANSFORM PSL_GENERATED_INC APPEND .hpp)

list(APPEND PSL_INC
	${INC_IMPL}
	array
	algorithms
	chunked_array
	enum
	exceptions
	expected
	iterators
	optional
	random
	span
	type_concepts
	types
	uid

	details/optional_value_storage
	details/source_location
	details/fixed_ascii_string
	)

list(TRANSFORM PSL_INC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/include/psl/)
list(TRANSFORM PSL_INC APPEND .hpp)
list(APPEND PSL_INC ${PSL_GENERATED_INC})

list(APPEND PSL_SRC 
	${INC_IMPL}
	psl
	)
list(TRANSFORM PSL_SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/source/)
list(TRANSFORM PSL_SRC APPEND .cpp)

list(APPEND PSL_INCLUDE_DIRECTORIES include)

#######################################################################################################################
### Setup	 																										###
#######################################################################################################################

set_source_files_properties(${PSL_GENERATED_INC} PROPERTIES GENERATED TRUE)

add_custom_command(OUTPUT ${PSL_GENERATED_INC}
	COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate.py
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/settings.json ${CMAKE_CURRENT_SOURCE_DIR}/tools/config.template
	COMMENT "Generating headers for PSL")
add_custom_target(${LOCAL_PROJECT}_generator ALL DEPENDS ${PSL_GENERATED_INC})

add_library(${LOCAL_PROJECT} ${PSL_SRC})
target_include_directories(${LOCAL_PROJECT} PUBLIC ${PSL_INCLUDE_DIRECTORIES} ${fmt_INCLUDE_DIRS})
target_link_libraries(${LOCAL_PROJECT} fmt)
add_dependencies(${LOCAL_PROJECT} ${LOCAL_PROJECT}_generator)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	list(APPEND PSL_COMPILE_OPTIONS
		-Wall
		-Wextra
		-pedantic
		-Wno-unknown-pragmas
		-Wno-unqualified-std-cast-call
	)
	if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
		if(${PSL_COVERAGE})
			list(APPEND PSL_COMPILE_OPTIONS
				--coverage
				-O0
			)
			list(APPEND PSL_LINK_OPTIONS
				--coverage
			)
		endif()
		if(${PSL_AVX2})
			list(APPEND PSL_COMPILE_OPTIONS
				-mavx2
			)
		endif()
	endif()
else()
	list(APPEND PSL_COMPILE_OPTIONS
		/permissive-
		/W
	)
	if("${CONFIG}" MATCHES "Release")
		list(APPEND PSL_COMPILE_OPTIONS
			/WX
		)
	endif()
endif()

target_compile_options(${LOCAL_PROJECT} PUBLIC ${PSL_COMPILE_OPTIONS})

target_link_options(${LOCAL_PROJECT} PUBLIC ${PSL_LINK_OPTIONS})

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE $<BOOL:${PSL_COVERAGE}>)

if(PSL_TESTS)
	add_subdirectory(tests)
endif()

if(PSL_DOCUMENTATION)
	add_subdirectory(documentation)
endif()
