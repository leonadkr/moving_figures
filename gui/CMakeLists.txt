project( gui LANGUAGES C )

find_package( PkgConfig REQUIRED )
pkg_check_modules( GLIB2 REQUIRED glib-2.0 )
pkg_check_modules( GOBJECT2 REQUIRED gobject-2.0 )
pkg_check_modules( GTK4 REQUIRED gtk4 )
pkg_check_modules( EPOXY REQUIRED epoxy )

# figures
add_library( figures
	STATIC
	gfigure.c
	gpoint.c
	gcircle.c
	gpolygon.c
	gstar.c
	glrenderer.c
	gtkmovingfiguresarea.c )

set_target_properties( figures PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED TRUE )

target_include_directories( figures
	PRIVATE
	${GLIB2_INCLUDE_DIRS}
	${GOBJECT2_INCLUDE_DIRS}
	${GTK4_INCLUDE_DIRS}
	${EPOXY_INCLUDE_DIRS}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR} )

target_link_directories( figures
	PRIVATE
	${GLIB2_LIBRARY_DIRS}
	${GOBJECT2_LIBRARY_DIRS}
	${GTK4_LIBRARY_DIRS}
	${EPOXY_LIBRARY_DIRS} )

target_link_libraries( figures
	PRIVATE
	gresource
	m
	${GLIB2_LIBRARIES}
	${GOBJECT2_LIBRARIES}
	${GTK4_LIBRARIES}
	${EPOXY_LIBRARIES} )

# gui
add_library( ${PROJECT_NAME}
	STATIC
	window.c )

set_target_properties( ${PROJECT_NAME} PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED TRUE )

target_include_directories( ${PROJECT_NAME}
	PRIVATE
	${GLIB2_INCLUDE_DIRS}
	${GTK4_INCLUDE_DIRS}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR} )

target_link_directories( ${PROJECT_NAME}
	PRIVATE
	${GLIB2_LIBRARY_DIRS}
	${GTK4_LIBRARY_DIRS} )

target_link_libraries( ${PROJECT_NAME}
	PRIVATE
	${GLIB2_LIBRARIES}
	${GTK4_LIBRARIES}
	figures )
