function(jas_check_type name valid_out)

	set(verbose FALSE)
	#set(verbose TRUE)

	set(single_arg_options "SIZE")
	cmake_parse_arguments(PARSE_ARGV 2 parsed "" "${single_arg_options}" "")

	if(verbose)
		message("name: ${name}")
		message("size: ${result}")
		message("parsed_SIZE: ${parsed_SIZE}")
	endif()

	# Note:
	# The check_type_size function/macro places its results in cache variables.
	# So, we need to use a name that is unlikely to collide with another name.
	# Also, we need to clear the cache so that we do not obtain a cached
	# result from a previous invocation.
	unset(jas_check_type_name_result CACHE)
	unset(HAVE_jas_check_type_name_result CACHE)
	check_type_size("${name}" jas_check_type_name_result)

	if(verbose)
		message("valid: ${HAVE_jas_check_type_name_result}")
		message("size: ${jas_check_type_name_result}")
	endif()

	if(HAVE_jas_check_type_name_result)
		set(${valid} TRUE)
	else()
		set(${valid} FALSE)
	endif()

	set(${valid_out} ${valid} CACHE INTERNAL "is_valid ${name}")

	if (DEFINED parsed_SIZE)
		set(${parsed_SIZE} ${jas_check_type_name_result} CACHE INTERNAL
		  "sizeof ${name}")
	endif()
	
endfunction()
