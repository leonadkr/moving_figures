cmake_minimum_required( VERSION 4.1 )

project( ${PROGRAM_NAME} LANGUAGES C )

include( GNUInstallDirs )

find_package( PkgConfig REQUIRED )
pkg_check_modules( GOBJECT2 REQUIRED gobject-2.0 )
pkg_check_modules( GLIB2 REQUIRED glib-2.0 )
pkg_check_modules( GIO2 REQUIRED gio-2.0 )
pkg_check_modules( GTK4 REQUIRED gtk4 )
pkg_check_modules( GRAPHENE REQUIRED graphene-1.0 )
pkg_check_modules( GRAPHENE_GOBJECT REQUIRED graphene-gobject-1.0 )
pkg_check_modules( EPOXY REQUIRED epoxy )

add_subdirectory( resource )

add_executable( ${PROJECT_NAME} )
target_compile_features( ${PROJECT_NAME} PRIVATE c_std_17 )

target_sources( ${PROJECT_NAME}
	PRIVATE
		mffigure.c
		mflayout.c
		mfmodelasset.c
		mfrender.c
		mfswitchbutton.c
		mfdrawarea.c
		mfcustomfigure.c
		mffigureinfo.c
		mfwindow.c
		mfapplication.c
		main.c

	PRIVATE
		FILE_SET privateHeaders
		TYPE HEADERS
		FILES
			mffigure.h
			mflayout.h
			mfmodelasset.h
			mfrender.h
			mfswitchbutton.h
			mfdrawarea.h
			mfcustomfigure.h
			mffigureinfo.h
			mfwindow.h
			mfapplication.h
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE
		${GOBJECT2_INCLUDE_DIRS}
		${GLIB2_INCLUDE_DIRS}
		${GIO2_INCLUDE_DIRS}
		${GTK4_INCLUDE_DIRS}
		${GRAPHENE_INCLUDE_DIRS}
		${GRAPHENE_GOBJECT_INCLUDE_DIRS}
		${EPOXY_INCLUDE_DIRS}
)

target_link_directories( ${PROJECT_NAME}
	PRIVATE
		${GOBJECT2_LIBRARY_DIRS}
		${GLIB2_LIBRARY_DIRS}
		${GIO2_LIBRARY_DIRS}
		${GTK4_LIBRARY_DIRS}
		${GRAPHENE_LIBRARY_DIRS}
		${GRAPHENE_GOBJECT_LIBRARY_DIRS}
		${EPOXY_LIBRARY_DIRS}
)

target_link_libraries( ${PROJECT_NAME}
	PRIVATE
		${GOBJECT2_LIBRARIES}
		${GLIB2_LIBRARIES}
		${GIO2_LIBRARIES}
		${GTK4_LIBRARIES}
		${GRAPHENE_LIBRARIES}
		${GRAPHENE_GOBJECT_LIBRARIES}
		${EPOXY_LIBRARIES}
		resource
)

install( TARGETS ${PROJECT_NAME}
	RUNTIME
)
