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
set(DOXYGEN_PROJECT_ROOT  ${${PSL_PROJECT}_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

configure_file(${CMAKE_CURRENT_LIST_DIR}/doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doxyfile)
configure_file(${CMAKE_CURRENT_LIST_DIR}/configuration.py ${CMAKE_CURRENT_BINARY_DIR}/configuration.py)

file(GLOB CSS
  "css/*"
)
set(commands)
foreach(file ${CSS})
  list(APPEND commands
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file} "${mcss_SOURCE_DIR}/css")
endforeach()

add_custom_target(${LOCAL_PROJECT}_COPY ${commands} WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
add_custom_target(${LOCAL_PROJECT}_COMPILE 
    COMMAND "${mcss_SOURCE_DIR}/css/postprocess.py" m-dark.css
    COMMAND "${mcss_SOURCE_DIR}/css/postprocess.py" m-dark.css m-documentation.css -o m-dark+documentation.compiled.css
    COMMAND "${mcss_SOURCE_DIR}/css/postprocess.py" m-dark.css m-theme-dark.css m-documentation.css --no-import -o m-dark.documentation.compiled.css
    DEPENDS ${LOCAL_PROJECT}_COPY WORKING_DIRECTORY "${mcss_SOURCE_DIR}/css")

add_custom_target(
    ${LOCAL_PROJECT}
    COMMAND cmake -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
    COMMAND "${mcss_SOURCE_DIR}/documentation/doxygen.py" "${CMAKE_CURRENT_BINARY_DIR}/configuration.py"
    COMMAND echo "Generated documentation to: ${DOXYGEN_OUTPUT_DIRECTORY}"
    DEPENDS ${PSL_GENERATED_INC} ${LOCAL_PROJECT}_COMPILE
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_dependencies(${LOCAL_PROJECT} ${PSL_PROJECT}_generator)