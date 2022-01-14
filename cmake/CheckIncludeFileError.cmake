if( _CHECK_INCLUDE_FILE_ERROR_CMAKE_ )
	return()
endif()
set( _CHECK_INCLUDE_FILE_ERROR_CMAKE_ TRUE )

include( CheckIncludeFile )

macro( check_include_file_error INCLUDE_FILE HAVE_FILE )
	check_include_file( "${INCLUDE_FILE}" ${HAVE_FILE} )
	if( NOT ${HAVE_FILE} )
		unset( ${HAVE_FILE} CACHE )
		message( FATAL_ERROR "${INCLUDE_FILE} is not found" )
	endif()
endmacro()
