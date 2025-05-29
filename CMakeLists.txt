cmake_minimum_required( VERSION 3.16 )

project( moving_figures VERSION 1.0.0 )

set( PROGRAM_NAME ${CMAKE_PROJECT_NAME} )
set( PROGRAM_VERSION ${CMAKE_PROJECT_VERSION} )
set( PROGRAM_APP_ID "com.github.leonadkr.moving_figures" )
set( PROGRAM_AUTHOR "leonadkr@gmail.com" )

set( PROGRAM_WINDOW_WIDTH "1000" )
set( PROGRAM_WINDOW_HEIGHT "900" )

configure_file( config.h.in config.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory( src )
