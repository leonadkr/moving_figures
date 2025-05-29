#ifndef MFAPPLICATION_H
#define MFAPPLICATION_H

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MF_TYPE_APPLICATION ( mf_application_get_type() )
G_DECLARE_FINAL_TYPE( MfApplication, mf_application, MF, APPLICATION, GtkApplication )

MfApplication* mf_application_new( const gchar *application_id );

G_END_DECLS

#endif
