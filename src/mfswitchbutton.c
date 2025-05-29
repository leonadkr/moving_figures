#include "mfswitchbutton.h"

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

struct _MfSwitchButton
{
	GtkWidget parent_instance;

	gchar *first_label;
	gchar *second_label;

	gboolean is_first;
	GtkButton *button;
};
typedef struct _MfSwitchButton MfSwitchButton;

enum _MfSwitchButtonPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_FIRST_LABEL,
	PROP_SECOND_LABEL,

	N_PROPS
};
typedef enum _MfSwitchButtonPropertyID MfSwitchButtonPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

enum _MfSwitchButtonSignalID
{
	SIGNAL_FIRST_CLICKED,
	SIGNAL_SECOND_CLICKED,

	N_SIGNALS
};
typedef enum _MfSwitchButtonSignalID MfSwitchButtonSignalID;

static guint mf_switch_button_signals[N_SIGNALS] = { 0, };

G_DEFINE_TYPE( MfSwitchButton, mf_switch_button, GTK_TYPE_WIDGET )

static void
on_button_clicked(
	GtkButton *self,
	gpointer user_data )
{
	MfSwitchButton *switch_button = MF_SWITCH_BUTTON( user_data );

	if( switch_button->is_first )
	{
		g_signal_emit( switch_button, mf_switch_button_signals[SIGNAL_FIRST_CLICKED], 0 );
		gtk_button_set_label( self, switch_button->second_label );
	}
	else
	{
		g_signal_emit( switch_button, mf_switch_button_signals[SIGNAL_SECOND_CLICKED], 0 );
		gtk_button_set_label( self, switch_button->first_label );
	}

	switch_button->is_first = !switch_button->is_first;
}

static void
mf_switch_button_init(
	MfSwitchButton *self )
{
	self->first_label = g_strdup( "First" );
	self->second_label = g_strdup( "Second" );
	self->is_first = TRUE;

	/* setup widget */
	self->button = GTK_BUTTON( gtk_button_new() );
	gtk_button_set_label( self->button, self->first_label );
	g_signal_connect( G_OBJECT( self->button ), "clicked", G_CALLBACK( on_button_clicked ), self );

	/* layout widgets */
	gtk_widget_set_parent( GTK_WIDGET( self->button ), GTK_WIDGET( self ) );
}

static void
mf_switch_button_get_property(
	GObject *object,	
	guint prop_id,	
	GValue *value,	
	GParamSpec *pspec )
{
	MfSwitchButton *self = MF_SWITCH_BUTTON( object );

	switch( (MfSwitchButtonPropertyID)prop_id )
	{
		case PROP_FIRST_LABEL:
			g_value_set_string( value, self->first_label );
			break;
		case PROP_SECOND_LABEL:
			g_value_set_string( value, self->second_label );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_switch_button_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	MfSwitchButton *self = MF_SWITCH_BUTTON( object );

	switch( (MfSwitchButtonPropertyID)prop_id )
	{
		case PROP_FIRST_LABEL:
			mf_switch_button_set_first_label( self, g_value_get_string( value ) );
			break;
		case PROP_SECOND_LABEL:
			mf_switch_button_set_second_label( self, g_value_get_string( value ) );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_switch_button_finalize(
	GObject *object )
{
	MfSwitchButton *self = MF_SWITCH_BUTTON( object );

	g_free( self->first_label );
	g_free( self->second_label );

	G_OBJECT_CLASS( mf_switch_button_parent_class )->finalize( object );
}

static void
mf_switch_button_dispose(
	GObject *object )
{
	MfSwitchButton *self = MF_SWITCH_BUTTON( object );

	gtk_widget_unparent( GTK_WIDGET( self->button ) );

	G_OBJECT_CLASS( mf_switch_button_parent_class )->dispose( object );
}

static void
mf_switch_button_class_init(
	MfSwitchButtonClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS( klass );

	object_class->get_property = mf_switch_button_get_property;
	object_class->set_property = mf_switch_button_set_property;
	object_class->dispose = mf_switch_button_finalize;
	object_class->dispose = mf_switch_button_dispose;

	object_props[PROP_FIRST_LABEL] = g_param_spec_string(
		"first-label",
		"First label",
		"The first label of the button",
		"First",
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_SECOND_LABEL] = g_param_spec_string(
		"second-label",
		"Second label",
		"The second label of the button",
		"Second",
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	mf_switch_button_signals[SIGNAL_FIRST_CLICKED] = g_signal_new(
		"first-clicked",
		G_TYPE_FROM_CLASS( klass ),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0 );
	mf_switch_button_signals[SIGNAL_SECOND_CLICKED] = g_signal_new(
		"second-clicked",
		G_TYPE_FROM_CLASS( klass ),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0 );

	gtk_widget_class_set_layout_manager_type( widget_class, GTK_TYPE_BIN_LAYOUT );
}

MfSwitchButton*
mf_switch_button_new(
	void )
{
	return MF_SWITCH_BUTTON( g_object_new( MF_TYPE_SWITCH_BUTTON, NULL ) );
}

gchar*
mf_switch_button_get_first_label(
	MfSwitchButton *self )
{
	g_return_val_if_fail( MF_IS_SWITCH_BUTTON( self ), NULL );

	return self->first_label;
}

void
mf_switch_button_set_first_label(
	MfSwitchButton *self,
	const gchar *label )
{
	g_return_if_fail( MF_IS_SWITCH_BUTTON( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	g_free( self->first_label );
	self->first_label = g_strdup( label );

	if( self->is_first )
		gtk_button_set_label( GTK_BUTTON( self->button ), self->first_label );

	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_FIRST_LABEL] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gchar*
mf_switch_button_get_second_label(
	MfSwitchButton *self )
{
	g_return_val_if_fail( MF_IS_SWITCH_BUTTON( self ), NULL );

	return self->second_label;
}

void
mf_switch_button_set_second_label(
	MfSwitchButton *self,
	const gchar *label )
{
	g_return_if_fail( MF_IS_SWITCH_BUTTON( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	g_free( self->second_label );
	self->second_label = g_strdup( label );

	if( !self->is_first )
		gtk_button_set_label( GTK_BUTTON( self->button ), self->second_label );

	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_SECOND_LABEL] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

