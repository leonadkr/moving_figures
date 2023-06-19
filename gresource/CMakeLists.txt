project( gresource LANGUAGES C )

find_package( PkgConfig REQUIRED )
pkg_check_modules( GLIB2 REQUIRED glib-2.0 )

find_program( GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED )

configure_file( gresource.xml.in gresource.xml )

execute_process(
	COMMAND ${GLIB_COMPILE_RESOURCES} --generate-dependencies gresource.xml
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	OUTPUT_VARIABLE	GRESOURCE_DEPENDENCIES )
string( REPLACE "\n" ";" GRESOURCE_DEPENDENCIES ${GRESOURCE_DEPENDENCIES} )
list( POP_BACK GRESOURCE_DEPENDENCIES )
list( TRANSFORM GRESOURCE_DEPENDENCIES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/ )

add_custom_command( OUTPUT gresource.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${GLIB_COMPILE_RESOURCES}
	ARGS --sourcedir=${CMAKE_CURRENT_SOURCE_DIR} --generate-source --target=gresource.c gresource.xml
	VERBATIM
	MAIN_DEPENDENCY	gresource.xml
	DEPENDS ${GRESOURCE_DEPENDENCIES} )

add_custom_command( OUTPUT gresource.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${GLIB_COMPILE_RESOURCES}
	ARGS --sourcedir=${CMAKE_CURRENT_SOURCE_DIR} --generate-header --target=gresource.h gresource.xml
	VERBATIM
	MAIN_DEPENDENCY	gresource.xml
	DEPENDS ${GRESOURCE_DEPENDENCIES} )

add_custom_target( gresource_source
	DEPENDS
	gresource.c )

add_custom_target( gresource_header
	DEPENDS
	gresource.h )

add_library( gresource
	STATIC
	${CMAKE_CURRENT_BINARY_DIR}/gresource.c )

add_dependencies( gresource
	gresource_source
	gresource_header )

target_include_directories( gresource
	PRIVATE
	${GLIB2_INCLUDE_DIRS}
	PUBLIC
	${CMAKE_CURRENT_BINARY_DIR} )

target_link_directories( gresource
	PRIVATE
	${GLIB2_LIBRARY_DIRS} )

target_link_libraries( gresource
	PRIVATE
	${GLIB2_LIBRARIES} )

set_target_properties( gresource PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED TRUE )
