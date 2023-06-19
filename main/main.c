#include "config.h"
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include "window.h"

struct _AppPrivate
{
	GtkWindow *window;
};
typedef struct _AppPrivate AppPrivate;

static void
on_app_startup(
	GApplication *self,
	gpointer user_data )
{
	AppPrivate *priv = (AppPrivate*)user_data;

	g_return_if_fail( G_IS_APPLICATION( self ) );

	g_application_hold( self );
	priv->window = mf_window_new( GTK_APPLICATION( self ) );
	g_application_release( self );
}

static void
on_app_activate(
	GApplication *self,
	gpointer user_data )
{
	AppPrivate *priv = (AppPrivate*)user_data;

	g_return_if_fail( G_IS_APPLICATION( self ) );

	g_application_hold( self );
	gtk_widget_set_visible( GTK_WIDGET( priv->window ), TRUE );
	g_application_release( self );
}

int
main(
	int argc,
	char *argv[] )
{
	GApplication *app;
	AppPrivate priv;
	gint ret;

	g_return_val_if_fail( g_application_id_is_valid( APP_ID ), EXIT_FAILURE );

	app = G_APPLICATION( gtk_application_new( APP_ID, G_APPLICATION_DEFAULT_FLAGS ) );
	if( app == NULL )
		return EXIT_FAILURE;

	g_signal_connect( G_OBJECT( app ), "startup", G_CALLBACK( on_app_startup ), &priv );
	g_signal_connect( G_OBJECT( app ), "activate", G_CALLBACK( on_app_activate ), &priv );

	ret = g_application_run( app, argc, argv );
	g_object_unref( G_OBJECT( app ) );

	return ret;
}

