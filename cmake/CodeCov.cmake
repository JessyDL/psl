find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

if(NOT CMAKE_COMPILER_IS_GNUCXX)
	message(FATAL_ERROR "requires GNU compiler")
endif()

if ( NOT (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Coverage"))
  message( FATAL_ERROR "Code coverage should be run with non-optimized builds" )
endif()

function(code_coverage _name _runner _output)
	if(NOT DEFINED LCOV_BASE_DIR)
		message("setting LCOV_BASE_DIR to '${CMAKE_SOURCE_DIR}")
		set(LCOV_BASE_DIR "'${CMAKE_SOURCE_DIR}'")
	endif()

	if(NOT LCOV_PATH)
		message(FATAL_ERROR "missing lcov, please install it")
	endif()

	if(NOT GENHTML_PATH)
		message(FATAL_ERROR "missing genhtml, please install it")
	endif()

	set(coverage_info "${CMAKE_BINARY_DIR}/${_output}_raw.info")
	set(coverage_filtered  "${CMAKE_BINARY_DIR}/${_output}.info")

	separate_arguments(runner_command UNIX_COMMAND "${_runner}")

	add_custom_target(${_name}
		${LCOV_PATH} --directory . --zerocounters
		COMMAND ${runner_command}
		COMMAND ${LCOV_PATH} --directory . --base-directory ${LCOV_BASE_DIR} --no-external --capture --output-file ${coverage_info} -rc lcov_branch_coverage=1
		COMMAND ${LCOV_PATH} --remove ${coverage_info} "'${CMAKE_BINARY_DIR}/*'" ${LCOV_REMOVE_EXTRA} --output-file ${coverage_filtered}
		COMMAND ${GENHTML_PATH} --prefix ${LCOV_BASE_DIR} -o ${_output} ${coverage_filtered} --branch-coverage --highlight --legend --keep-descriptions
		COMMAND ${CMAKE_COMMAND} -E remove ${coverage_info} ${coverage_filtered}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "generating code coverage report"
	)
endfunction()
