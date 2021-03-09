#######################################################################################################################
### Definitions																										###
#######################################################################################################################

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
SET(PSL "Paradigm Standard Library")
set(PSL_PROJECT "psl")
set(LOCAL_PROJECT ${PSL_PROJECT})
project(${LOCAL_PROJECT} VERSION 0.0.1 LANGUAGES CXX)
include(CMakeDependentOption)

set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DRELEASE")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(avx2 "support avx2" TRUE)
option(tests "build tests" TRUE)
option(documentation "build documentation" TRUE)
cmake_dependent_option(coverage "show the code coverage of the tests" FALSE "tests" FALSE)


if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

#######################################################################################################################
### Externals 																										###
#######################################################################################################################

SET(Python_ADDITIONAL_VERSIONS 3 3.6 3.5 3.4 3.3 3.2 3.1 3.0)
find_package(PythonInterp REQUIRED)

add_subdirectory(externals/fmt)

#######################################################################################################################
### Includes 																										###
#######################################################################################################################

list(APPEND INC_IMPL
	allocator
	
	serialization/format
	)

list(APPEND PSL_GENERATED_INC
	config
	psl
	)

list(TRANSFORM PSL_GENERATED_INC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/include/psl/)
list(TRANSFORM PSL_GENERATED_INC APPEND .hpp)

list(APPEND PSL_INC
	${INC_IMPL}
	algorithms
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

	containers/variant.hpp
	containers/string_view
	
	serialization/property
	serialization/serializer

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
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate.py
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/settings.json ${CMAKE_CURRENT_SOURCE_DIR}/tools/config.template
	COMMENT "Generating headers for PSL")
add_custom_target(${LOCAL_PROJECT}_generator ALL DEPENDS ${PSL_GENERATED_INC})

add_library(${LOCAL_PROJECT} ${PSL_SRC})
target_include_directories(${LOCAL_PROJECT} PUBLIC ${PSL_INCLUDE_DIRECTORIES} ${fmt_INCLUDE_DIRS})
target_compile_features(${LOCAL_PROJECT} PUBLIC cxx_std_20)
target_link_libraries(${LOCAL_PROJECT} fmt)

add_dependencies(${LOCAL_PROJECT} ${LOCAL_PROJECT}_generator)

# -Wno-error=terminate is silenced globally due to constent expressions not figuring out that they are not runtime, and 
# so warn of throws in noexcept marked functions.


target_compile_options(${LOCAL_PROJECT} PUBLIC 
	$<$<CXX_COMPILER_ID:MSVC>:/permissive- /W3 /Zc:preprocessor /wd4146 /wd4308>
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/WX>
	$<$<CXX_COMPILER_ID:CLANG>:-Wno-error=terminate -Wall -Wextra -pedantic -Wno-unknown-pragmas>
	$<$<CXX_COMPILER_ID:GNU>:-Wno-error=terminate -Wall -Wextra -pedantic -Wno-unknown-pragmas>
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${coverage}>>:--coverage -O0>
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${avx2}>>:-mavx2>
	)

target_link_options(${LOCAL_PROJECT} PUBLIC
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${coverage}>>:--coverage>
)

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE $<BOOL:${coverage}>)

if(tests)
	add_subdirectory(tests)
endif()

if(documentation)
	add_subdirectory(documentation)
endif()
