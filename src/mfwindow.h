#ifndef MFWINDOW_H
#define MFWINDOW_H

#include "mfapplication.h"

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MF_TYPE_WINDOW ( mf_window_get_type() )
G_DECLARE_FINAL_TYPE( MfWindow, mf_window, MF, WINDOW, GtkApplicationWindow )

MfWindow* mf_window_new( MfApplication *app );

G_END_DECLS

#endif
