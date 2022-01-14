project( gui LANGUAGES C )

find_package( PkgConfig REQUIRED )
pkg_check_modules( GTK3 REQUIRED gtk+-3.0 )
pkg_check_modules( GLIB2 REQUIRED glib-2.0 )
pkg_check_modules( GOBJECT2 REQUIRED gobject-2.0 )
pkg_check_modules( CAIRO REQUIRED cairo )

# figures
add_library( figures
	STATIC
	gfigure.c
	gpoint.c
	gcircle.c
	gpolygon.c
	gstar.c
	moving_figures.c )

target_include_directories( figures
	PRIVATE
	${GTK3_INCLUDE_DIRS}
	${GLIB2_INCLUDE_DIRS}
	${GOBJECT2_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR} )

target_link_directories( figures
	PRIVATE
	${GTK3_LIBRARY_DIRS}
	${GLIB2_LIBRARY_DIRS}
	${GOBJECT2_LIBRARY_DIRS}
	${CAIRO_LIBRARY_DIRS} )

target_link_libraries( figures
	PRIVATE
	m
	${GTK3_LIBRARIES}
	${GLIB2_LIBRARIES}
	${GOBJECT2_LIBRARIES}
	${CAIRO_LIBRARIES} )

set_target_properties( figures PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED TRUE )

# gui
add_library( gui
	STATIC
	gui.c
	window.c )

target_include_directories( gui
	PRIVATE
	${GTK3_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR} )

target_link_directories( gui
	PRIVATE
	${GTK3_LIBRARY_DIRS}
	${CAIRO_LIBRARY_DIRS} )

target_link_libraries( gui
	PRIVATE
	${GTK3_LIBRARIES}
	${CAIRO_LIBRARIES}
	figures )

set_target_properties( gui PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED TRUE )
