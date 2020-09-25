#######################################################################################################################
### Definitions																										###
#######################################################################################################################

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

SET(DOCUMENTATION "PSL Documentation Generation")
set(DOC_PROJECT "Documentation")
set(LOCAL_PROJECT ${DOC_PROJECT})
project(${LOCAL_PROJECT} VERSION 1.0.0)

include(FetchContent)

FetchContent_Declare(
	mcss
	GIT_REPOSITORY
	https://github.com/mosra/m.css.git
)

FetchContent_GetProperties(mcss)
if(NOT mcss_POPULATED)
	FetchContent_Populate(mcss)
endif()

set(DOXYGEN_PROJECT_NAME ${PSL})
set(DOXYGEN_PROJECT_VERSION ${${PSL_PROJECT}_VERSION})
set(DOXYGEN_PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

configure_file(${CMAKE_CURRENT_LIST_DIR}/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
configure_file(${CMAKE_CURRENT_LIST_DIR}/configuration.py ${CMAKE_CURRENT_BINARY_DIR}/configuration.py)

add_custom_target(
    ${LOCAL_PROJECT}
    ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
    COMMAND "${mcss_SOURCE_DIR}/documentation/doxygen.py" "${CMAKE_CURRENT_BINARY_DIR}/configuration.py"
    COMMAND echo "Generated documentation to: ${DOXYGEN_OUTPUT_DIRECTORY}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)