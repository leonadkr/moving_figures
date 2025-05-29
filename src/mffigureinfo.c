#include "mffigureinfo.h"

#include "mffigure.h"

#include <gtk/gtk.h>
#include <graphene.h>
#include <graphene-gobject.h>

#define BOX_SPACING 2

struct _MfFigureInfo
{
	GtkWidget parent_instance;

	GtkLabel *label;
	GtkColorDialogButton *color_button;

	GtkBox *main_box;
};
typedef struct _MfFigureInfo MfFigureInfo;

enum _MfFigureInfoPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_NAME,
	PROP_COLOR,

	N_PROPS
};
typedef enum _MfFigureInfoPropertyID MfFigureInfoPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

G_DEFINE_FINAL_TYPE( MfFigureInfo, mf_figure_info, GTK_TYPE_WIDGET )

static void
mf_figure_info_init(
	MfFigureInfo *self )
{
	GtkLabel *label;
	GtkColorDialogButton *color_button;
	GtkBox *main_box;

	/* create widgets */
	main_box = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, BOX_SPACING ) );

	label = GTK_LABEL( gtk_label_new( NULL ) );
	gtk_widget_set_halign( GTK_WIDGET( label ), GTK_ALIGN_START );
	gtk_label_set_single_line_mode( label, TRUE );
	gtk_label_set_selectable( label, FALSE );
	self->label = label;

	color_button = GTK_COLOR_DIALOG_BUTTON( gtk_color_dialog_button_new( NULL ) );
	gtk_widget_set_hexpand( GTK_WIDGET( color_button ), TRUE );
	gtk_widget_set_sensitive( GTK_WIDGET( color_button ), FALSE );
	self->color_button = color_button;

	/* layout widgets */
	gtk_box_append( main_box, GTK_WIDGET( label ) );
	gtk_box_append( main_box, GTK_WIDGET( color_button ) );

	/* main layout */
	self->main_box = main_box;
	gtk_widget_set_parent( GTK_WIDGET( self->main_box ), GTK_WIDGET( self ) );
}

static void
mf_figure_info_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	MfFigureInfo *self = MF_FIGURE_INFO( object );
	GdkRGBA *rgba;
	graphene_vec4_t color;

	switch( (MfFigureInfoPropertyID)prop_id )
	{
		case PROP_NAME:
			g_value_set_string( value, gtk_label_get_label( self->label ) );
			break;
		case PROP_COLOR:
			rgba = (GdkRGBA*)gtk_color_dialog_button_get_rgba( self->color_button );
			graphene_vec4_init( &color, rgba->red, rgba->green, rgba->blue, rgba->alpha );
			g_value_set_boxed( value, &color );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_figure_info_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	MfFigureInfo *self = MF_FIGURE_INFO( object );

	switch( (MfFigureInfoPropertyID)prop_id )
	{
		case PROP_NAME:
			mf_figure_info_set_name( self, g_value_get_string( value ) );
			break;
		case PROP_COLOR:
			mf_figure_info_set_color( self, g_value_get_boxed( value ) );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_figure_info_dispose(
	GObject *object )
{
	MfFigureInfo *self = MF_FIGURE_INFO( object );

	gtk_widget_unparent( GTK_WIDGET( self->main_box ) );

	G_OBJECT_CLASS( mf_figure_info_parent_class )->dispose( object );
}

static void
mf_figure_info_class_init(
	MfFigureInfoClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS( klass );

	object_class->get_property = mf_figure_info_get_property;
	object_class->set_property = mf_figure_info_set_property;
	object_class->dispose = mf_figure_info_dispose;
	object_props[PROP_NAME] = g_param_spec_string(
		"name",
		"Name",
		"Name of the layout",
		NULL,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_COLOR] = g_param_spec_boxed(
		"color",
		"Color",
		"Color of the layout as graphene_vec4_t",
		GRAPHENE_TYPE_VEC4,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	gtk_widget_class_set_layout_manager_type( widget_class, GTK_TYPE_BIN_LAYOUT );
}

MfFigureInfo*
mf_figure_info_new(
	void )
{
	return MF_FIGURE_INFO( g_object_new( MF_TYPE_FIGURE_INFO, NULL ) );
}

MfFigureInfo*
mf_figure_info_new_full(
	const gchar *name,
	graphene_vec4_t *color )
{
	return MF_FIGURE_INFO( g_object_new( MF_TYPE_FIGURE_INFO, "name", name, "color", color, NULL ) );
}

gchar*
mf_figure_info_get_name(
	MfFigureInfo *self )
{
	g_return_val_if_fail( MF_IS_FIGURE_INFO( self ), NULL );

	return g_strdup( gtk_label_get_label( self->label ) );
}

void
mf_figure_info_set_name(
	MfFigureInfo *self,
	const gchar *name )
{
	g_return_if_fail( MF_IS_FIGURE_INFO( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	gtk_label_set_label( self->label, name );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_NAME] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

void
mf_figure_info_get_color(
	MfFigureInfo *self,
	graphene_vec4_t *color )
{
	GdkRGBA *rgba;

	g_return_if_fail( MF_IS_FIGURE_INFO( self ) );
	g_return_if_fail( color != NULL );

	rgba = (GdkRGBA*)gtk_color_dialog_button_get_rgba( self->color_button );
	graphene_vec4_init( color, rgba->red, rgba->green, rgba->blue, rgba->alpha );
}

void
mf_figure_info_set_color(
	MfFigureInfo *self,
	const graphene_vec4_t *color )
{
	GdkRGBA rgba;

	g_return_if_fail( MF_IS_FIGURE_INFO( self ) );
	g_return_if_fail( color != NULL );

	rgba.red = graphene_vec4_get_x( color );
	rgba.green = graphene_vec4_get_y( color );
	rgba.blue = graphene_vec4_get_z( color );
	rgba.alpha = graphene_vec4_get_w( color );

	gtk_color_dialog_button_set_rgba( self->color_button, &rgba );
}

