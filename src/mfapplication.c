#include "mfapplication.h"

#include "config.h"
#include "mfwindow.h"

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

struct _MfApplication
{
	GtkApplication parent_instance;

	MfWindow *window;
};
typedef struct _MfApplication MfApplication;

G_DEFINE_TYPE( MfApplication, mf_application, GTK_TYPE_APPLICATION )

static void
mf_application_init(
	MfApplication *self )
{
	g_application_set_option_context_description( G_APPLICATION( self ), PROGRAM_APPLICATION_DESCRIPTION );
	g_application_set_option_context_summary( G_APPLICATION( self ), PROGRAM_APPLICATION_SUMMARY );
	g_application_set_resource_base_path( G_APPLICATION( self ), NULL );

	self->window = NULL;
}

static void
mf_application_startup(
	GApplication *app )
{
	MfApplication *self = MF_APPLICATION( app );

	G_APPLICATION_CLASS( mf_application_parent_class )->startup( app );

	self->window = mf_window_new( self );
}

static void
mf_application_activate(
	GApplication *app )
{
	MfApplication *self = MF_APPLICATION( app );

	G_APPLICATION_CLASS( mf_application_parent_class )->activate( app );

	gtk_window_present( GTK_WINDOW( self->window ) );
}

static void
mf_application_class_init(
	MfApplicationClass *klass )
{
	GApplicationClass *app_class = G_APPLICATION_CLASS( klass );

	app_class->startup = mf_application_startup;
	app_class->activate = mf_application_activate;
}

MfApplication*
mf_application_new(
	const gchar *application_id )
{
	g_return_val_if_fail( g_application_id_is_valid( application_id ), NULL );

	return MF_APPLICATION( g_object_new( MF_TYPE_APPLICATION, "application-id", application_id, "flags", G_APPLICATION_DEFAULT_FLAGS, NULL ) );
}

