#include <stdlib.h>
#include <gtk/gtk.h>
#include "gtkmovingfiguresarea.h"


/*
	definitions
*/
#define FPS ( 30.0 )

enum _ButtonState
{
	BUTTON_STATE_START,
	BUTTON_STATE_STOP,

	N_BUTTON_STATE
};
typedef enum _ButtonState ButtonState;


/*
	structures
*/
struct _WindowPrivate
{
	GtkMovingFiguresArea *moving_figures_area;
	GtkSpinButton *point_spin, *circle_spin, *polygon_spin, *star_spin;
	ButtonState button_state;
	gint timer;
};
typedef struct _WindowPrivate WindowPrivate;


/*
	function prototypes
*/
static G_DEFINE_QUARK( window-private, window_private )
static WindowPrivate* window_private_new( void );
static void window_private_free( WindowPrivate *priv );
static WindowPrivate* window_get_private( GtkWindow *window );

static GtkMovingFiguresArea* moving_figures_area_new( GtkWindow *window );

static G_DEFINE_QUARK( spin-figure-type, spin_figure_type )
static GtkSpinButton* spin_new( GtkWindow *window, gint figure_type );
static void on_spin_value_changed( GtkSpinButton *spin, gpointer user_data );

static GtkButton* startstop_button_new( GtkWindow *window );
static void on_startstop_button_clicked( GtkButton *startstop_button, gpointer user_data );
static gboolean timer_timeout_callback( gpointer user_data );

static GtkButton* reallocate_button_new( GtkWindow *window );
static void on_reallocate_button_clicked( GtkButton *reallocate_button, gpointer user_data );

static gboolean on_key_pressed( GtkEventControllerKey *self, guint keyval, guint keycode, GdkModifierType state, gpointer user_data );


/*
	private
*/
static WindowPrivate*
window_private_new(
	void )
{
	return g_new( WindowPrivate, 1 );
}

static void
window_private_free(
	WindowPrivate *priv )
{
	if( priv->timer )
		g_source_remove( priv->timer );
	g_free( priv );
}

static WindowPrivate*
window_get_private(
	GtkWindow *window )
{
	g_return_val_if_fail( GTK_IS_WINDOW( window ), NULL );

	return g_object_get_qdata( G_OBJECT( window ), window_private_quark() );
}


/*
	moving figures area
*/
static GtkMovingFiguresArea*
moving_figures_area_new(
	GtkWindow *window )
{
	GtkMovingFiguresArea *moving_figures_area;

	moving_figures_area = gtk_moving_figures_area_new( 100, 100, FPS );
	gtk_widget_set_vexpand( GTK_WIDGET( moving_figures_area ), TRUE );
	gtk_widget_set_hexpand( GTK_WIDGET( moving_figures_area ), TRUE );

	return moving_figures_area;
}


/*
	spins
*/
static GtkSpinButton*
spin_new(
	GtkWindow *window,
	gint figure_type )
{
	GtkSpinButton *spin;
	GtkAdjustment *adj;
	gint *spin_figure_type;

	g_return_val_if_fail( GTK_IS_WINDOW( window ), NULL );

	spin_figure_type = g_new( gint, 1 );
	*spin_figure_type = figure_type;
	adj = GTK_ADJUSTMENT( gtk_adjustment_new( 20.0, 0.0, 100.0, 1.0, 1.0, 1.0 ) );
	spin = GTK_SPIN_BUTTON( gtk_spin_button_new( adj, 1.0, 0 ) );
	gtk_spin_button_set_update_policy( spin, GTK_UPDATE_IF_VALID );

	g_object_set_qdata_full( G_OBJECT( spin ), spin_figure_type_quark(), spin_figure_type, (GDestroyNotify)g_free );
	g_signal_connect( GTK_WIDGET( spin ), "value-changed", G_CALLBACK( on_spin_value_changed ), window );

	return spin;
}

static void
on_spin_value_changed(
	GtkSpinButton *spin,
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );
	gint num, *spin_figure_type;

	num = gtk_spin_button_get_value_as_int( spin );
	spin_figure_type = g_object_get_qdata( G_OBJECT( spin ), spin_figure_type_quark() );
	gtk_moving_figures_area_set_number( priv->moving_figures_area, *spin_figure_type, num );

	if( priv->button_state == BUTTON_STATE_START )
		gtk_widget_queue_draw( GTK_WIDGET( priv->moving_figures_area ) );
}


/*
	start-stop button
*/
static GtkButton*
startstop_button_new(
	GtkWindow *window )
{
	GtkButton *startstop_button;

	g_return_val_if_fail( GTK_IS_WINDOW( window ), NULL );

	startstop_button = GTK_BUTTON( gtk_button_new_with_label( "Start" ) );
	g_signal_connect( GTK_WIDGET( startstop_button ), "clicked", G_CALLBACK( on_startstop_button_clicked ), window );

	return startstop_button;
}

static void
on_startstop_button_clicked(
	GtkButton *startstop_button,
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );

	switch( priv->button_state )
	{
		case BUTTON_STATE_START:
			gtk_button_set_label( startstop_button, "Stop" );
			priv->button_state = BUTTON_STATE_STOP;
			priv->timer = g_timeout_add( 1000 / (gint)FPS, timer_timeout_callback, user_data );
			break;
		case BUTTON_STATE_STOP:
			gtk_button_set_label( startstop_button, "Start" );
			priv->button_state = BUTTON_STATE_START;
			g_source_remove( priv->timer );
			priv->timer = 0;
			break;
		default:
			break;
	}
}

static gboolean
timer_timeout_callback(
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );

	gtk_moving_figures_area_move( priv->moving_figures_area );
	gtk_widget_queue_draw( GTK_WIDGET( priv->moving_figures_area ) );

	return TRUE;
}


/*
	reallocate button
*/
static GtkButton*
reallocate_button_new(
	GtkWindow *window )
{
	GtkButton *reallocate_button;

	g_return_val_if_fail( GTK_IS_WINDOW( window ), NULL );

	reallocate_button = GTK_BUTTON( gtk_button_new_with_label( "Reallocate" ) );
	g_signal_connect( GTK_WIDGET( reallocate_button ), "clicked", G_CALLBACK( on_reallocate_button_clicked ), window );

	return reallocate_button;
}

static void
on_reallocate_button_clicked(
	GtkButton *reallocate_button,
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );
	WindowPrivate *priv = window_get_private( window );

	gtk_moving_figures_area_reallocate( priv->moving_figures_area );
	if( priv->button_state == BUTTON_STATE_START )
		gtk_widget_queue_draw( GTK_WIDGET( priv->moving_figures_area ) );
}


/*
	window
*/
static gboolean
on_key_pressed(
	GtkEventControllerKey *self,
	guint keyval,
	guint keycode,
	GdkModifierType state,
	gpointer user_data )
{
	GtkWindow *window = GTK_WINDOW( user_data );

	if( keyval == GDK_KEY_q && ( state & GDK_CONTROL_MASK ) )
	{
		gtk_window_destroy( window );
		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}


/*
	public
*/
GtkWindow*
window_new(
	GtkApplication *app )
{
	GtkWindow *window;
	GtkEventControllerKey *key;
	GtkBox *hbox, *vbox;
	GtkFrame *point_frame, *circle_frame, *polygon_frame, *star_frame;
	GtkSpinButton *point_spin, *circle_spin, *polygon_spin, *star_spin;
	GtkButton *startstop_button, *reallocate_button;
	GtkMovingFiguresArea *moving_figures_area;
	WindowPrivate *priv;

	/* create window */
	window = GTK_WINDOW( gtk_application_window_new( app ) );
	gtk_window_set_default_size( window, 700, 700 );
	gtk_window_set_title( window, "Moving figures" );
	gtk_window_set_resizable( window, TRUE );
	key = GTK_EVENT_CONTROLLER_KEY( gtk_event_controller_key_new() );
	gtk_widget_add_controller( GTK_WIDGET( window ), GTK_EVENT_CONTROLLER( key ) );
	g_signal_connect( G_OBJECT( key ), "key-pressed", G_CALLBACK( on_key_pressed ), window );

	/* create widgets */
	hbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 1 ) );
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 1 ) );
	point_frame = GTK_FRAME( gtk_frame_new( "Point" ) );
	circle_frame = GTK_FRAME( gtk_frame_new( "Circle" ) );
	polygon_frame = GTK_FRAME( gtk_frame_new( "Polygon" ) );
	star_frame = GTK_FRAME( gtk_frame_new( "Star" ) );
	point_spin = spin_new( window, GTK_MOVING_FIGURE_TYPE_POINT );
	circle_spin = spin_new( window, GTK_MOVING_FIGURE_TYPE_CIRCLE );
	polygon_spin = spin_new( window, GTK_MOVING_FIGURE_TYPE_POLYGON );
	star_spin = spin_new( window, GTK_MOVING_FIGURE_TYPE_STAR );
	moving_figures_area = moving_figures_area_new( window );
	gtk_moving_figures_area_append( moving_figures_area, GTK_MOVING_FIGURE_TYPE_POINT, gtk_spin_button_get_value_as_int( point_spin ) );
	gtk_moving_figures_area_append( moving_figures_area, GTK_MOVING_FIGURE_TYPE_CIRCLE, gtk_spin_button_get_value_as_int( circle_spin ) );
	gtk_moving_figures_area_append( moving_figures_area, GTK_MOVING_FIGURE_TYPE_POLYGON, gtk_spin_button_get_value_as_int( polygon_spin ) );
	gtk_moving_figures_area_append( moving_figures_area, GTK_MOVING_FIGURE_TYPE_STAR, gtk_spin_button_get_value_as_int( star_spin ) );
	startstop_button = startstop_button_new( window );
	reallocate_button = reallocate_button_new( window );

	/* layout widgets */
	gtk_frame_set_child( point_frame, GTK_WIDGET( point_spin ) );
	gtk_frame_set_child( circle_frame, GTK_WIDGET( circle_spin ) );
	gtk_frame_set_child( polygon_frame, GTK_WIDGET( polygon_spin ) );
	gtk_frame_set_child( star_frame, GTK_WIDGET( star_spin ) );
	gtk_box_append( vbox, GTK_WIDGET( point_frame ) );
	gtk_box_append( vbox, GTK_WIDGET( circle_frame ) );
	gtk_box_append( vbox, GTK_WIDGET( polygon_frame ) );
	gtk_box_append( vbox, GTK_WIDGET( star_frame ) );
	gtk_box_append( vbox, GTK_WIDGET( startstop_button ) );
	gtk_box_append( vbox, GTK_WIDGET( reallocate_button ) );
	gtk_box_append( hbox, GTK_WIDGET( moving_figures_area ) );
	gtk_box_append( hbox, GTK_WIDGET( vbox ) );
	gtk_window_set_child( window, GTK_WIDGET( hbox ) );

	/* collect private data */
	priv = window_private_new();
	priv->moving_figures_area = moving_figures_area;
	priv->point_spin = point_spin;
	priv->circle_spin = circle_spin;
	priv->polygon_spin = polygon_spin;
	priv->star_spin = star_spin;
	priv->button_state = BUTTON_STATE_START;
	priv->timer = 0;
	g_object_set_qdata_full( G_OBJECT( window ), window_private_quark(), priv, (GDestroyNotify)window_private_free );

	return window;
}

