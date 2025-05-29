#include "config.h"
#include "mfapplication.h"

#include <locale.h>
#include <glib.h>
#include <gio/gio.h>

int
main(
	int argc,
	char *argv[] )
{
	MfApplication *app;
	gint ret = EXIT_SUCCESS;

	setlocale( LC_ALL, "" );

	app = mf_application_new( PROGRAM_APP_ID );
	if( app == NULL )
		return EXIT_FAILURE;

	ret = g_application_run( G_APPLICATION( app ), argc, argv );
	g_object_unref( G_OBJECT( app ) );

	return ret;
}

