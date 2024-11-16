function(jas_get_stdc_version status_out stdc_version_out)

	#set(verbose TRUE)
	set(verbose FALSE)

	if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
		set(source_dir ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../src)
	else()
		set(source_dir ${CMAKE_CURRENT_SOURCE_DIR}/build/cmake/src)
	endif()
	set(bin_dir ${CMAKE_CURRENT_BINARY_DIR}/build/cmake/src)

	try_run(print_stdc_run_status print_stdc_compile_status
	    ${bin_dir}
	    ${source_dir}/print_stdc.c
	    COMPILE_OUTPUT_VARIABLE print_stdc_compile_output
	    RUN_OUTPUT_VARIABLE stdc_version
	)

	# For testing.
	#set(stdc_version "199901L") # C99
	#set(stdc_version "201112L") # C11
	#set(stdc_version "201710L") # C17

	if(verbose)
		message("print_stdc_run_status ${print_stdc_run_status}")
		message("print_stdc_compile_status ${print_stdc_compile_status}")
		message("stdc_version ${stdc_version}")
	endif()

	if(print_stdc_compile_status AND NOT print_stdc_run_status)
		set(status TRUE)
	else()
		set(status FALSE)
		set(stdc_version "")
	endif()

	if(verbose)
		message("status ${status}")
		message("stdc_version ${stdc_version}")
	endif()

	set(${status_out} "${status}" PARENT_SCOPE)
	set(${stdc_version_out} "${stdc_version}" PARENT_SCOPE)

endfunction()
