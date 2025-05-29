#ifndef MFSWITCHBUTTON_H
#define MFSWITCHBUTTON_H

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MF_TYPE_SWITCH_BUTTON ( mf_switch_button_get_type() )
G_DECLARE_FINAL_TYPE( MfSwitchButton, mf_switch_button, MF, SWITCH_BUTTON, GtkWidget )

MfSwitchButton* mf_switch_button_new( void );
gchar* mf_switch_button_get_first_label( MfSwitchButton *self );
void mf_switch_button_set_first_label( MfSwitchButton *self, const gchar *label );
gchar* mf_switch_button_get_second_label( MfSwitchButton *self );
void mf_switch_button_set_second_label( MfSwitchButton *self, const gchar *label );

G_END_DECLS

#endif
