#include "mfcustomfigure.h"

#include "mffigure.h"

#include <gtk/gtk.h>
#include <graphene.h>
#include <graphene-gobject.h>

#define BOX_SPACING 3

#define DEFAULT_RGBA ( (GdkRGBA){ .red = 0.0f, .green = 0.0f, .blue = 0.0f, .alpha = 1.0f } )

#define DEFAULT_SPIN_STEP ( 1.0 )
#define SPIN_DIGITS 0

#define DEFAULT_X ( DEFAULT_X_MIN )
#define DEFAULT_X_MIN ( 0.0 )
#define DEFAULT_X_MAX ( 0.0 )

#define DEFAULT_Y ( DEFAULT_Y_MIN )
#define DEFAULT_Y_MIN ( 0.0 )
#define DEFAULT_Y_MAX ( 0.0 )

#define DEFAULT_VELOCITY_X ( 0.0 )
#define DEFAULT_VELOCITY_X_MIN ( -200.0 )
#define DEFAULT_VELOCITY_X_MAX ( 200.0 )

#define DEFAULT_VELOCITY_Y ( 0.0 )
#define DEFAULT_VELOCITY_Y_MIN ( -200.0 )
#define DEFAULT_VELOCITY_Y_MAX ( 200.0 )

#define DEFAULT_ANGLE ( 0.0 )

#define DEFAULT_ANGULAR_VELOCITY ( 0.0 )
#define DEFAULT_ANGULAR_VELOCITY_MIN ( -180.0 )
#define DEFAULT_ANGULAR_VELOCITY_MAX ( 180.0 )

#define DEFAULT_SCALE ( DEFAULT_SCALE_MIN )
#define DEFAULT_SCALE_MIN ( 10.0 )
#define DEFAULT_SCALE_MAX ( 100.0 )

struct _MfCustomFigure
{
	GtkWidget parent_instance;

	GtkCheckButton *drop_check;
	GtkDropDown *drop_down;

	GtkCheckButton *color_check;
	GtkColorDialogButton *color_button;

	GtkCheckButton *coord_check;
	GtkSpinButton *x_spin;
	GtkSpinButton *y_spin;

	GtkCheckButton *vel_check;
	GtkSpinButton *velx_spin;
	GtkSpinButton *vely_spin;

	GtkCheckButton *a_check;
	GtkSpinButton *a_spin;

	GtkCheckButton *va_check;
	GtkSpinButton *vela_spin;

	GtkCheckButton *scale_check;
	GtkSpinButton *scale_spin;

	GtkBox *main_box;

	GListStore *layouts;
};
typedef struct _MfCustomFigure MfCustomFigure;

enum _MfCustomFigurePropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_LAYOUTS,

	N_PROPS
};
typedef enum _MfCustomFigurePropertyID MfCustomFigurePropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

enum _MfCustomFigureSignalID
{
	SIGNAL_CREATE,

	N_SIGNALS
};
typedef enum _MfCustomFigureSignalID MfCustomFigureSignalID;

static guint mf_custom_figure_signals[N_SIGNALS] = { 0, };

G_DEFINE_FINAL_TYPE( MfCustomFigure, mf_custom_figure, GTK_TYPE_WIDGET )

static void
on_factory_setup(
	GtkSignalListItemFactory *self,
	GObject *object,
	gpointer user_data )
{
	GtkLabel *label;

	label = GTK_LABEL( gtk_label_new( NULL ) );
	gtk_widget_set_halign( GTK_WIDGET( label ), GTK_ALIGN_START );
	gtk_label_set_single_line_mode( label, TRUE );
	gtk_label_set_selectable( label, FALSE );

	gtk_list_item_set_child( GTK_LIST_ITEM( object ), GTK_WIDGET( label ) );
}

static void
on_factory_bind(
	GtkSignalListItemFactory *self,
	GtkListItem *list_item,
	gpointer user_data )
{
	gchar *name;

	g_object_get( gtk_list_item_get_item( list_item ), "name", &name, NULL );
	gtk_label_set_text( GTK_LABEL( gtk_list_item_get_child( list_item ) ), name );
	g_free( name );
}

static void
on_drop_check_toggled(
	GtkCheckButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );

	if( gtk_widget_get_sensitive( GTK_WIDGET( custom->drop_down ) ) )
		gtk_widget_set_sensitive( GTK_WIDGET( custom->drop_down ), FALSE );
	else
		gtk_widget_set_sensitive( GTK_WIDGET( custom->drop_down ), TRUE );
}

static void
on_color_check_toggled(
	GtkCheckButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );

	if( gtk_widget_get_sensitive( GTK_WIDGET( custom->color_button ) ) )
		gtk_widget_set_sensitive( GTK_WIDGET( custom->color_button ), FALSE );
	else
		gtk_widget_set_sensitive( GTK_WIDGET( custom->color_button ), TRUE );
}

static void
on_coordinate_check_toggled(
	GtkCheckButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );

	if( gtk_widget_get_sensitive( GTK_WIDGET( custom->x_spin ) ) )
	{
		gtk_widget_set_sensitive( GTK_WIDGET( custom->x_spin ), FALSE );
		gtk_widget_set_sensitive( GTK_WIDGET( custom->y_spin ), FALSE );
	}
	else
	{
		gtk_widget_set_sensitive( GTK_WIDGET( custom->x_spin ), TRUE );
		gtk_widget_set_sensitive( GTK_WIDGET( custom->y_spin ), TRUE );
	}
}

static void
on_velocity_check_toggled(
	GtkCheckButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );

	if( gtk_widget_get_sensitive( GTK_WIDGET( custom->velx_spin ) ) )
	{
		gtk_widget_set_sensitive( GTK_WIDGET( custom->velx_spin ), FALSE );
		gtk_widget_set_sensitive( GTK_WIDGET( custom->vely_spin ), FALSE );
	}
	else
	{
		gtk_widget_set_sensitive( GTK_WIDGET( custom->velx_spin ), TRUE );
		gtk_widget_set_sensitive( GTK_WIDGET( custom->vely_spin ), TRUE );
	}
}

static void
on_angle_check_toggled(
	GtkCheckButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );

	if( gtk_widget_get_sensitive( GTK_WIDGET( custom->a_spin ) ) )
		gtk_widget_set_sensitive( GTK_WIDGET( custom->a_spin ), FALSE );
	else
		gtk_widget_set_sensitive( GTK_WIDGET( custom->a_spin ), TRUE );
}

static void
on_angular_velocity_check_toggled(
	GtkCheckButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );

	if( gtk_widget_get_sensitive( GTK_WIDGET( custom->vela_spin ) ) )
		gtk_widget_set_sensitive( GTK_WIDGET( custom->vela_spin ), FALSE );
	else
		gtk_widget_set_sensitive( GTK_WIDGET( custom->vela_spin ), TRUE );
}

static void
on_scale_check_toggled(
	GtkCheckButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );

	if( gtk_widget_get_sensitive( GTK_WIDGET( custom->scale_spin ) ) )
		gtk_widget_set_sensitive( GTK_WIDGET( custom->scale_spin ), FALSE );
	else
		gtk_widget_set_sensitive( GTK_WIDGET( custom->scale_spin ), TRUE );
}

static void
on_create_button_clicked(
	GtkButton *self,
	gpointer user_data )
{
	MfCustomFigure *custom = MF_CUSTOM_FIGURE( user_data );
	gint pos, n_layouts;
	gdouble x_min, x_max, y_min, y_max;
	GObject *obj;
	MfLayout *layout;
	const GdkRGBA *rgba;
	graphene_vec4_t color;
	graphene_point_t position;
	graphene_point_t velocity;
	gfloat angle, angular_velocity, scale;
	GRand *rnd;
	MfFigure *fig;

	rnd = g_rand_new();

	/* layout */
	if( gtk_check_button_get_active( custom->drop_check ) )
	{
		n_layouts = g_list_model_get_n_items( G_LIST_MODEL( custom->layouts ) );
		if( n_layouts == 0 )
		{
			g_rand_free( rnd );
			return;
		}

		pos = g_rand_int_range( rnd, 0, n_layouts );
		layout = MF_LAYOUT( g_list_model_get_item( G_LIST_MODEL( custom->layouts ), pos ) );
	}
	else
	{
		obj = gtk_drop_down_get_selected_item( custom->drop_down );
		if( obj == NULL )
		{
			g_rand_free( rnd );
			return;
		}

		layout = MF_LAYOUT( g_object_ref( obj ) );
	}

	/* color */
	if( gtk_check_button_get_active( custom->color_check ) )
		graphene_vec4_init(
			&color,
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 ),
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 ),
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 ),
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 )
		);
	else
	{
		rgba = gtk_color_dialog_button_get_rgba( custom->color_button );
		graphene_vec4_init( &color, rgba->red, rgba->green, rgba->blue, rgba->alpha );
	}

	/* coordinates */
	if( gtk_check_button_get_active( custom->coord_check ) )
	{
		gtk_spin_button_get_range( custom->x_spin, &x_min, &x_max );
		gtk_spin_button_get_range( custom->y_spin, &y_min, &y_max );
		graphene_point_init(
			&position,
			(gfloat)g_rand_double_range( rnd, x_min, x_max ),
			(gfloat)g_rand_double_range( rnd, y_min, y_max )
		);
	}
	else
		graphene_point_init(
			&position,
			(gfloat)gtk_spin_button_get_value( custom->x_spin ),
			(gfloat)gtk_spin_button_get_value( custom->y_spin )
		);

	/* velocity */
	if( gtk_check_button_get_active( custom->vel_check ) )
	{
		gtk_spin_button_get_range( custom->velx_spin, &x_min, &x_max );
		gtk_spin_button_get_range( custom->vely_spin, &y_min, &y_max );
		graphene_point_init(
			&velocity,
			(gfloat)g_rand_double_range( rnd, x_min, x_max ),
			(gfloat)g_rand_double_range( rnd, y_min, y_max )
		);
	}
	else
		graphene_point_init(
			&velocity,
			(gfloat)gtk_spin_button_get_value( custom->velx_spin ),
			(gfloat)gtk_spin_button_get_value( custom->vely_spin )
		);
	
	/* angle */
	if( gtk_check_button_get_active( custom->a_check ) )
		angle = (gfloat)g_rand_double_range( rnd, 0.0, 360.0 );
	else
		angle = (gfloat)gtk_spin_button_get_value( custom->a_spin );

	/* angular velocity */
	if( gtk_check_button_get_active( custom->va_check ) )
	{
		gtk_spin_button_get_range( custom->vela_spin, &x_min, &x_max );
		angular_velocity = (gfloat)g_rand_double_range( rnd, x_min, x_max );
	}
	else
		angular_velocity = (gfloat)gtk_spin_button_get_value( custom->vela_spin );

	/* scale */
	if( gtk_check_button_get_active( custom->scale_check ) )
	{
		gtk_spin_button_get_range( custom->scale_spin, &x_min, &x_max );
		scale = (gfloat)g_rand_double_range( rnd, x_min, x_max );
	}
	else
		scale = (gfloat)gtk_spin_button_get_value( custom->scale_spin );

	g_rand_free( rnd );

	fig = mf_figure_new_full( &color, &position, &velocity, angle, angular_velocity, scale, layout );
	g_object_unref( G_OBJECT( layout ) );
	g_signal_emit( G_OBJECT( custom ), mf_custom_figure_signals[SIGNAL_CREATE], 0, fig );
	g_object_unref( G_OBJECT( fig ) );
}

static void
mf_custom_figure_init(
	MfCustomFigure *self )
{
	GtkListItemFactory *factory;
	GtkDropDown *drop_down;
	GtkColorDialog *color_dialog;
	GtkColorDialogButton *color_button;
	GtkBox *vbox, *main_box;
	GtkCheckButton *check;
	GtkSpinButton *spin;
	GtkFrame *frame;
	GtkButton *create_button;

	self->layouts = g_list_store_new( MF_TYPE_LAYOUT );

	main_box = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, BOX_SPACING ) );

	/* layout drop down */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 ) );

	check = GTK_CHECK_BUTTON( gtk_check_button_new_with_label( "Random" ) );
	g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( on_drop_check_toggled ), self );
	gtk_box_append( vbox, GTK_WIDGET( check ) );
	self->drop_check = check;

	factory = GTK_LIST_ITEM_FACTORY( gtk_signal_list_item_factory_new() );
	g_signal_connect( G_OBJECT( factory ), "setup", G_CALLBACK( on_factory_setup ), self );
	g_signal_connect( G_OBJECT( factory ), "bind", G_CALLBACK( on_factory_bind ), self );

	drop_down = GTK_DROP_DOWN( gtk_drop_down_new( G_LIST_MODEL( g_object_ref( G_OBJECT( self->layouts ) ) ), NULL ) );
	gtk_drop_down_set_factory( drop_down, factory );
	g_object_unref( G_OBJECT( factory ) );
	gtk_box_append( vbox, GTK_WIDGET( drop_down ) );
	self->drop_down = drop_down;

	frame = GTK_FRAME( gtk_frame_new( "Layout" ) );
	gtk_frame_set_child( frame, GTK_WIDGET( vbox ) );
	gtk_box_append( main_box, GTK_WIDGET( frame ) );

	/* color */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 ) );

	check = GTK_CHECK_BUTTON( gtk_check_button_new_with_label( "Random" ) );
	g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( on_color_check_toggled ), self );
	gtk_box_append( vbox, GTK_WIDGET( check ) );
	self->color_check = check;

	color_dialog = GTK_COLOR_DIALOG( gtk_color_dialog_new() );
	color_button = GTK_COLOR_DIALOG_BUTTON( gtk_color_dialog_button_new( color_dialog ) );
	gtk_color_dialog_button_set_rgba( color_button, &DEFAULT_RGBA);
	gtk_box_append( vbox, GTK_WIDGET( color_button ) );
	self->color_button = color_button;

	frame = GTK_FRAME( gtk_frame_new( "Color" ) );
	gtk_frame_set_child( frame, GTK_WIDGET( vbox ) );
	gtk_box_append( main_box, GTK_WIDGET( frame ) );

	/* coordinate */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 ) );

	check = GTK_CHECK_BUTTON( gtk_check_button_new_with_label( "Random" ) );
	g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( on_coordinate_check_toggled ), self );
	gtk_box_append( vbox, GTK_WIDGET( check ) );
	self->coord_check = check;

	spin = GTK_SPIN_BUTTON( gtk_spin_button_new_with_range( DEFAULT_X_MIN, DEFAULT_X_MAX, DEFAULT_SPIN_STEP ) );
	gtk_spin_button_set_numeric( spin, TRUE );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_wrap( spin, FALSE );
	gtk_spin_button_set_digits( spin, SPIN_DIGITS );
	gtk_spin_button_set_value( spin, DEFAULT_X );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	self->x_spin = spin;

	spin = GTK_SPIN_BUTTON( gtk_spin_button_new_with_range( DEFAULT_Y_MIN, DEFAULT_Y_MAX, DEFAULT_SPIN_STEP ) );
	gtk_spin_button_set_numeric( spin, TRUE );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_wrap( spin, FALSE );
	gtk_spin_button_set_digits( spin, SPIN_DIGITS );
	gtk_spin_button_set_value( spin, DEFAULT_Y );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	self->y_spin = spin;

	frame = GTK_FRAME( gtk_frame_new( "Coordinates" ) );
	gtk_frame_set_child( frame, GTK_WIDGET( vbox ) );
	gtk_box_append( main_box, GTK_WIDGET( frame ) );

	/* velocity */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, BOX_SPACING ) );

	check = GTK_CHECK_BUTTON( gtk_check_button_new_with_label( "Random" ) );
	g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( on_velocity_check_toggled ), self );
	gtk_box_append( vbox, GTK_WIDGET( check ) );
	self->vel_check = check;

	spin = GTK_SPIN_BUTTON( gtk_spin_button_new_with_range( DEFAULT_VELOCITY_X_MIN, DEFAULT_VELOCITY_X_MAX, DEFAULT_SPIN_STEP ) );
	gtk_spin_button_set_numeric( spin, TRUE );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_wrap( spin, FALSE );
	gtk_spin_button_set_digits( spin, SPIN_DIGITS );
	gtk_spin_button_set_value( spin, DEFAULT_VELOCITY_X );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	self->velx_spin = spin;

	spin = GTK_SPIN_BUTTON( gtk_spin_button_new_with_range( DEFAULT_VELOCITY_Y_MIN, DEFAULT_VELOCITY_Y_MAX, DEFAULT_SPIN_STEP ) );
	gtk_spin_button_set_numeric( spin, TRUE );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_wrap( spin, FALSE );
	gtk_spin_button_set_digits( spin, SPIN_DIGITS );
	gtk_spin_button_set_value( spin, DEFAULT_VELOCITY_Y );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	self->vely_spin = spin;

	frame = GTK_FRAME( gtk_frame_new( "Velocity" ) );
	gtk_frame_set_child( frame, GTK_WIDGET( vbox ) );
	gtk_box_append( main_box, GTK_WIDGET( frame ) );

	/* angle */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 ) );

	check = GTK_CHECK_BUTTON( gtk_check_button_new_with_label( "Random" ) );
	g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( on_angle_check_toggled ), self );
	gtk_box_append( vbox, GTK_WIDGET( check ) );
	self->a_check = check;

	spin = GTK_SPIN_BUTTON( gtk_spin_button_new_with_range( 0.0, 360.0, DEFAULT_SPIN_STEP ) );
	gtk_spin_button_set_numeric( spin, TRUE );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_wrap( spin, FALSE );
	gtk_spin_button_set_digits( spin, SPIN_DIGITS );
	gtk_spin_button_set_value( spin, 0.0 );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	self->a_spin = spin;

	frame = GTK_FRAME( gtk_frame_new( "Angle" ) );
	gtk_frame_set_child( frame, GTK_WIDGET( vbox ) );
	gtk_box_append( main_box, GTK_WIDGET( frame ) );

	/* angular velocity */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 ) );

	check = GTK_CHECK_BUTTON( gtk_check_button_new_with_label( "Random" ) );
	g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( on_angular_velocity_check_toggled ), self );
	gtk_box_append( vbox, GTK_WIDGET( check ) );
	self->va_check = check;

	spin = GTK_SPIN_BUTTON( gtk_spin_button_new_with_range( DEFAULT_ANGULAR_VELOCITY_MIN, DEFAULT_ANGULAR_VELOCITY_MAX, DEFAULT_SPIN_STEP ) );
	gtk_spin_button_set_numeric( spin, TRUE );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_wrap( spin, FALSE );
	gtk_spin_button_set_digits( spin, SPIN_DIGITS );
	gtk_spin_button_set_value( spin, DEFAULT_ANGULAR_VELOCITY );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	self->vela_spin = spin;

	frame = GTK_FRAME( gtk_frame_new( "Angular velocity" ) );
	gtk_frame_set_child( frame, GTK_WIDGET( vbox ) );
	gtk_box_append( main_box, GTK_WIDGET( frame ) );

	/* scale */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 ) );

	check = GTK_CHECK_BUTTON( gtk_check_button_new_with_label( "Random" ) );
	g_signal_connect( G_OBJECT( check ), "toggled", G_CALLBACK( on_scale_check_toggled ), self );
	gtk_box_append( vbox, GTK_WIDGET( check ) );
	self->scale_check = check;

	spin = GTK_SPIN_BUTTON( gtk_spin_button_new_with_range( DEFAULT_SCALE_MIN, DEFAULT_SCALE_MAX, DEFAULT_SPIN_STEP ) );
	gtk_spin_button_set_numeric( spin, TRUE );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );
	gtk_spin_button_set_wrap( spin, FALSE );
	gtk_spin_button_set_digits( spin, SPIN_DIGITS );
	gtk_spin_button_set_value( spin, DEFAULT_SCALE );
	gtk_box_append( vbox, GTK_WIDGET( spin ) );
	self->scale_spin = spin;

	frame = GTK_FRAME( gtk_frame_new( "Scale" ) );
	gtk_frame_set_child( frame, GTK_WIDGET( vbox ) );
	gtk_box_append( main_box, GTK_WIDGET( frame ) );

	/* create button */
	create_button = GTK_BUTTON( gtk_button_new_with_label( "Create" ) );
	g_signal_connect( G_OBJECT( create_button ), "clicked", G_CALLBACK( on_create_button_clicked ), self );
	gtk_box_append( main_box, GTK_WIDGET( create_button ) );

	/* main layout */
	self->main_box = main_box;
	gtk_widget_set_parent( GTK_WIDGET( self->main_box ), GTK_WIDGET( self ) );
}

static void
mf_custom_figure_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	MfCustomFigure *self = MF_CUSTOM_FIGURE( object );

	switch( (MfCustomFigurePropertyID)prop_id )
	{
		case PROP_LAYOUTS:
			g_value_set_object( value, self->layouts );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_custom_figure_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	MfCustomFigure *self = MF_CUSTOM_FIGURE( object );

	switch( (MfCustomFigurePropertyID)prop_id )
	{
		case PROP_LAYOUTS:
			mf_custom_figure_set_layouts( self, g_value_get_object( value ) );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_custom_figure_dispose(
	GObject *object )
{
	MfCustomFigure *self = MF_CUSTOM_FIGURE( object );

	g_clear_object( &self->layouts );
	gtk_widget_unparent( GTK_WIDGET( self->main_box ) );

	G_OBJECT_CLASS( mf_custom_figure_parent_class )->dispose( object );
}

static void
mf_custom_figure_class_init(
	MfCustomFigureClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS( klass );

	object_class->get_property = mf_custom_figure_get_property;
	object_class->set_property = mf_custom_figure_set_property;
	object_class->dispose = mf_custom_figure_dispose;
	object_props[PROP_LAYOUTS] = g_param_spec_object(
		"layouts",
		"List of layouts",
		"Layout list to represent in the drop box",
		G_TYPE_LIST_STORE,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	mf_custom_figure_signals[SIGNAL_CREATE] = g_signal_new(
		"create",
		G_TYPE_FROM_CLASS( klass ),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		MF_TYPE_FIGURE );

	gtk_widget_class_set_layout_manager_type( widget_class, GTK_TYPE_BIN_LAYOUT );
}

MfCustomFigure*
mf_custom_figure_new(
	void )
{
	return MF_CUSTOM_FIGURE( g_object_new( MF_TYPE_CUSTOM_FIGURE, NULL ) );
}

MfCustomFigure*
mf_custom_figure_new_with_layouts(
	GListStore *layouts )
{
	return MF_CUSTOM_FIGURE( g_object_new( MF_TYPE_CUSTOM_FIGURE, "layouts", layouts, NULL ) );
}

GListStore*
mf_custom_figure_get_layouts(
	MfCustomFigure *self )
{
	g_return_val_if_fail( MF_IS_CUSTOM_FIGURE( self ), NULL );

	return G_LIST_STORE( g_object_ref( G_OBJECT( self->layouts ) ) );
}

void
mf_custom_figure_set_layouts(
	MfCustomFigure *self,
	GListStore *layouts )
{
	g_return_if_fail( MF_IS_CUSTOM_FIGURE( self ) );
	g_return_if_fail( G_IS_LIST_STORE( layouts ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	g_clear_object( &self->layouts );
	self->layouts = G_LIST_STORE( g_object_ref( G_OBJECT( layouts ) ) );
	gtk_drop_down_set_model( self->drop_down, G_LIST_MODEL( g_object_ref( G_OBJECT( layouts ) ) ) );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_LAYOUTS] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

void
mf_custom_figure_set_size(
	MfCustomFigure *self,
	gdouble width,
	gdouble height )
{
	GtkAdjustment *adj;

	g_return_if_fail( MF_IS_CUSTOM_FIGURE( self ) );

	adj = gtk_spin_button_get_adjustment( self->x_spin );
	gtk_adjustment_set_upper( adj, width );
	if( gtk_adjustment_get_value( adj ) > width )
		gtk_adjustment_set_value( adj, width );

	adj = gtk_spin_button_get_adjustment( self->y_spin );
	gtk_adjustment_set_upper( adj, height );
	if( gtk_adjustment_get_value( adj ) > height )
		gtk_adjustment_set_value( adj, height );
}

