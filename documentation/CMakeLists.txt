#######################################################################################################################
### Definitions																										###
#######################################################################################################################

cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

SET(DOCUMENTATION "PSL Documentation Generation")
set(DOC_PROJECT "Documentation")
set(LOCAL_PROJECT ${DOC_PROJECT})
project(${LOCAL_PROJECT} VERSION 1.0.0)

find_package(Python3 REQUIRED)

set(mcss_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/externals/mcss")

set(DOXYGEN_PROJECT_NAME ${PSL})
set(DOXYGEN_PROJECT_VERSION ${${PSL_PROJECT}_VERSION})
set(DOXYGEN_PROJECT_ROOT  ${${PSL_PROJECT}_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

configure_file(${CMAKE_CURRENT_LIST_DIR}/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
configure_file(${CMAKE_CURRENT_LIST_DIR}/configuration.py ${CMAKE_CURRENT_BINARY_DIR}/configuration.py)

add_custom_target(${LOCAL_PROJECT}_COMPILE_CSS 
    COMMAND ${Python3_EXECUTABLE} "postprocess.py" m-dark.css -o "m-dark.compiled.css"
    COMMAND ${Python3_EXECUTABLE} "postprocess.py" m-dark.css m-documentation.css -o "m-dark+documentation.compiled.css"
    COMMAND ${Python3_EXECUTABLE} "postprocess.py" m-dark.css m-theme-dark.css m-documentation.css --no-import -o "m-dark.documentation.compiled.css"
    WORKING_DIRECTORY "${mcss_SOURCE_DIR}/css")

add_custom_target(
    ${LOCAL_PROJECT}
    COMMAND cmake -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
    COMMAND ${Python3_EXECUTABLE} "${mcss_SOURCE_DIR}/documentation/doxygen.py" "${CMAKE_CURRENT_BINARY_DIR}/configuration.py"
    COMMAND echo "Generated documentation to: ${DOXYGEN_OUTPUT_DIRECTORY}"
    DEPENDS ${PSL_GENERATED_INC} ${LOCAL_PROJECT}_COMPILE_CSS 
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_dependencies(${LOCAL_PROJECT} ${PSL_PROJECT}_generator)