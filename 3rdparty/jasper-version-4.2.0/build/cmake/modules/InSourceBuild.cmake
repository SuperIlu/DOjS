option(ALLOW_IN_SOURCE_BUILD "Allow an in-source build" OFF)

function(prevent_in_source_build)

	# Determine if an in-source build is in progress.
	file(REAL_PATH "${CMAKE_SOURCE_DIR}" source_dir)
	file(REAL_PATH "${CMAKE_BINARY_DIR}" binary_dir)
	cmake_path(IS_PREFIX source_dir "${binary_dir}" result)

	# If an in-source build is in progress, and the build directory is not
	# chosen in a very specific way, then stop the build.
	if(result)
		cmake_path(RELATIVE_PATH binary_dir BASE_DIRECTORY "${source_dir}"
		  OUTPUT_VARIABLE cur_path)
		#message("cur_path ${cur_path}")
		while(true)
			cmake_path(HAS_PARENT_PATH cur_path has_parent)
			if(NOT has_parent)
				break()
			endif()
			cmake_path(GET cur_path PARENT_PATH cur_path)
			#message("cur_path ${cur_path}")
		endwhile()
		cmake_path(GET cur_path FILENAME top_dir_name)
		#message("top_dir_name ${top_dir_name}")
		if(NOT (top_dir_name MATCHES "^tmp"))
			message(FATAL_ERROR
				"The use of an in-source build has been detected "
				"(i.e., the binary directory specified to CMake is located "
				"in or under the source directory).  "
				"This can potentially trash the source tree.  "
				"In fact, if you are seeing this message, you may have already "
				"partially trashed the source tree.  "
				"The use of an in-source build is not officially supported and "
				"is therefore disallowed by default.  "
				"If you like to live dangerously and would like to override "
				"this default behavior, this can be accomplished via the "
				"CMake option ALLOW_IN_SOURCE_BUILD.\n"
				"CMake source directory: ${CMAKE_SOURCE_DIR}\n"
				"CMake binary directory: ${CMAKE_BINARY_DIR}\n"
				"CMake binary directory root: ${CMAKE_SOURCE_DIR}/${top_dir_name}\n"
			)
		endif()
	endif()

endfunction()

if(NOT ALLOW_IN_SOURCE_BUILD)
	prevent_in_source_build()
endif()
