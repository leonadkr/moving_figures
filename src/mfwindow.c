#include "mfwindow.h"

#include "config.h"
#include "resource.h"
#include "mflayout.h"
#include "mfmodelasset.h"
#include "mfrender.h"
#include "mffigure.h"
#include "mfapplication.h"
#include "mfswitchbutton.h"
#include "mfdrawarea.h"
#include "mfcustomfigure.h"
#include "mffigureinfo.h"

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <graphene.h>
#include <epoxy/gl.h>

#define FPS 15.0f

static G_DEFINE_QUARK( mf-draw-area-render, mf_draw_area_render )

struct _MfWindow
{
	GtkApplicationWindow parent_instance;

	MfModelAsset *model_asset;
	GListStore *figures;
	GtkFrame *draw_area_frame;
	MfCustomFigure *custom_figure;
	MfDrawArea *draw_area;
	GtkListView *list_view;
	gint timer;
};
typedef struct _MfWindow MfWindow;

G_DEFINE_TYPE( MfWindow, mf_window, GTK_TYPE_APPLICATION_WINDOW )

static gboolean
on_event_key_pressed(
	GtkEventControllerKey *self,
	guint keyval,
	guint keycode,
	GdkModifierType state,
	gpointer user_data )
{
	MfWindow *window = MF_WINDOW( user_data );
	GtkSelectionModel *selection;
	GtkBitset *bitset;
	GtkBitsetIter iter;
	guint pos;

	/* quit program */
	if( keyval == GDK_KEY_q && ( state & GDK_CONTROL_MASK ) )
	{
		gtk_window_destroy( GTK_WINDOW( window ) );
		return GDK_EVENT_STOP;
	}

	/* remove selected figures */
	if( keyval == GDK_KEY_Delete && ( state & GDK_CONTROL_MASK ) )
	{
		selection = gtk_list_view_get_model( window->list_view );
		bitset = gtk_selection_model_get_selection( selection );

		if( gtk_bitset_iter_init_last( &iter, bitset, &pos ) )
		{
			g_list_store_remove( window->figures, pos );
			while( gtk_bitset_iter_previous( &iter, &pos ) )
				g_list_store_remove( window->figures, pos );
		}

		gtk_bitset_unref( bitset );

		gtk_widget_queue_draw( GTK_WIDGET( window->draw_area ) );

		return GDK_EVENT_STOP;
	}

	return GDK_EVENT_PROPAGATE;
}

static gboolean
timer_timeout_callback(
	gpointer user_data )
{
	MfWindow *window = MF_WINDOW( user_data );
	gint n_figures, i;
	MfFigure *figure;
	graphene_rect_t rect;

	mf_draw_area_get_rectangle( window->draw_area, &rect );

	n_figures = g_list_model_get_n_items( G_LIST_MODEL( window->figures ) );
	for( i = 0; i < n_figures; ++i )
	{
		figure = MF_FIGURE( g_list_model_get_item( G_LIST_MODEL( window->figures ), i ) );
		if( mf_figure_is_inside_rectangle( figure, &rect ) )
			mf_figure_move( figure, &rect, FPS );
		g_object_unref( G_OBJECT( figure ) );
	}

	gtk_widget_queue_draw( GTK_WIDGET( window->draw_area ) );

	return G_SOURCE_CONTINUE;
}

static void
on_switch_button_first_clicked(
	GtkButton *self,
	gpointer user_data )
{
	MfWindow *window = MF_WINDOW( user_data );

	window->timer = g_timeout_add( 1000 / (gint)FPS, timer_timeout_callback, window );
}

static void
on_switch_button_second_clicked(
	GtkButton *self,
	gpointer user_data )
{
	MfWindow *window = MF_WINDOW( user_data );

	if( window->timer > 0  )
	{
		g_source_remove( window->timer );
		window->timer = 0;
	}
}

static void
on_randomize_button_clicked(
	GtkButton *self,
	gpointer user_data )
{
	const gint N_FIGURES_MIN = 10;
	const gint N_FIGURES_MAX = 100;

	MfWindow *window = MF_WINDOW( user_data );
	gint pos, n_layouts, i, n_figures_max;
	GListStore *layouts;
	MfLayout *layout;
	graphene_rect_t rect;
	graphene_vec4_t color;
	graphene_point_t position;
	graphene_point_t velocity;
	gfloat angle, angular_velocity, scale;
	GRand *rnd;
	MfFigure *fig;

	/* no layouts -- do nothing */
	layouts = mf_model_asset_get_layouts( window->model_asset );
	n_layouts = g_list_model_get_n_items( G_LIST_MODEL( layouts ) );
	g_object_unref( G_OBJECT( layouts ) );
	if( n_layouts == 0 )
		return;

	/* erase previous figure list */
	g_list_store_remove_all( window->figures );

	/* fill the figure list */
	layouts = mf_model_asset_get_layouts( window->model_asset );
	rnd = g_rand_new();
	n_figures_max = g_rand_int_range( rnd, N_FIGURES_MIN, N_FIGURES_MAX );
	for( i = 0; i < n_figures_max; ++i )
	{
		/* layout */
		pos = g_rand_int_range( rnd, 0, n_layouts );
		layout = MF_LAYOUT( g_list_model_get_item( G_LIST_MODEL( layouts ), pos ) );

		/* color */
		graphene_vec4_init(
			&color,
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 ),
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 ),
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 ),
			(gfloat)g_rand_double_range( rnd, 0.0, 1.0 )
		);

		/* coordinates */
		mf_draw_area_get_rectangle( window->draw_area, &rect );
		graphene_point_init(
			&position,
			(gfloat)g_rand_double_range( rnd, 0.0, graphene_rect_get_width( &rect ) ),
			(gfloat)g_rand_double_range( rnd, 0.0, graphene_rect_get_height( &rect ))
		);

		/* velocity */
		graphene_point_init(
			&velocity,
			(gfloat)g_rand_double_range( rnd, -50.0, 50.0 ),
			(gfloat)g_rand_double_range( rnd, -50.0, 50.0 )
		);
		
		/* angle */
		angle = (gfloat)g_rand_double_range( rnd, 0.0, 360.0 );

		/* angular velocity */
		angular_velocity = (gfloat)g_rand_double_range( rnd, -50.0, 50.0 );

		/* scale */
		scale = (gfloat)g_rand_double_range( rnd, 10.0, 100.0 );

		fig = mf_figure_new_full( &color, &position, &velocity, angle, angular_velocity, scale, layout );
		g_object_unref( G_OBJECT( layout ) );
		g_list_store_append( window->figures, G_OBJECT( fig ) );
		g_object_unref( G_OBJECT( fig ) );
	}
	g_object_unref( G_OBJECT( layouts ) );
	g_rand_free( rnd );

	gtk_widget_queue_draw( GTK_WIDGET( window->draw_area ) );
}

static void
on_draw_area_realize(
	GtkWidget *widget,
	gpointer user_data )
{
	MfDrawArea *self = MF_DRAW_AREA( widget );
	MfWindow *window = MF_WINDOW( user_data );
	GdkGLContext *context;
	GBytes *vshader_bytes, *fshader_bytes;
	GBytes *vertices_bytes, *indices_bytes;
	MfRender *render;
	GError *error = NULL;

  vshader_bytes = g_resource_lookup_data(
		resource_get_resource(),
		"/resource/shaders/vertex.glsl",
		G_RESOURCE_LOOKUP_FLAGS_NONE,
		&error );
	if( error != NULL )
	{
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "MESSAGE", error->message, NULL );
		g_clear_error( &error );
		return;
	}

  fshader_bytes = g_resource_lookup_data(
		resource_get_resource(),
		"/resource/shaders/fragment.glsl",
		G_RESOURCE_LOOKUP_FLAGS_NONE,
		&error );
	if( error != NULL )
	{
		g_bytes_unref( vshader_bytes );
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "MESSAGE", error->message, NULL );
		g_clear_error( &error );
		return;
	}

	/* get renderer */
	context = mf_draw_area_get_context( self );
	gdk_gl_context_make_current( context );
	g_object_unref( G_OBJECT( context ) );

	render = mf_render_new();
	g_object_set_qdata( G_OBJECT( self ), mf_draw_area_render_quark(), render );

	vertices_bytes = mf_model_asset_get_vertices( window->model_asset );
	indices_bytes = mf_model_asset_get_indices( window->model_asset );
	mf_render_complete( render, vshader_bytes, fshader_bytes, vertices_bytes, indices_bytes, &error );
	g_bytes_unref( vshader_bytes );
	g_bytes_unref( fshader_bytes );
	g_bytes_unref( vertices_bytes );
	g_bytes_unref( indices_bytes );
	if( error != NULL )
	{
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "MESSAGE", error->message, NULL );
		g_clear_error( &error );
		return;
	}
}

static void
on_draw_area_unrealize(
	GtkWidget *widget,
	gpointer user_data )
{
	MfDrawArea *self = MF_DRAW_AREA( widget );
	GdkGLContext *context;
	MfRender *render;

	/* get renderer */
	context = mf_draw_area_get_context( self );
	gdk_gl_context_make_current( context );
	g_object_unref( G_OBJECT( context ) );

	render = MF_RENDER( g_object_get_qdata( G_OBJECT( self ), mf_draw_area_render_quark() ) );
	g_object_unref( G_OBJECT( render ) );
}

static void
on_draw_area_render(
	MfDrawArea *self,
	gpointer user_data )
{
	MfWindow *window = MF_WINDOW( user_data );
	MfRender *render;
	gint n_figures, i;
	MfFigure *figure;

	render = MF_RENDER( g_object_get_qdata( G_OBJECT( self ), mf_draw_area_render_quark() ) );

	/* make white canvas */
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	/* draw figures */
	n_figures = g_list_model_get_n_items( G_LIST_MODEL( window->figures ) );
	for( i = 0; i < n_figures; ++i )
	{
		figure = MF_FIGURE( g_list_model_get_item( G_LIST_MODEL( window->figures ), i ) );
		mf_render_draw( render, figure );
		g_object_unref( G_OBJECT( figure ) );
	}
}

static void on_draw_area_resize(
	MfDrawArea *self,
	gint width,
	gint height,
	gpointer user_data )
{
	MfWindow *window = MF_WINDOW( user_data );
	MfRender *render;
	gchar *text;

	render = MF_RENDER( g_object_get_qdata( G_OBJECT( self ), mf_draw_area_render_quark() ) );

	/* update renderer size */
	mf_render_set_size( render, (gfloat)width, (gfloat)height );

	/* update custom figure's size */
	mf_custom_figure_set_size( window->custom_figure, (gdouble)width, (gdouble)height );

	/* show size at the frame */
	text = g_strdup_printf( "%dx%d", width, height );
	gtk_frame_set_label( window->draw_area_frame, text );
	g_free( text );
}

static void on_custom_figure_create(
	MfCustomFigure *self,
	MfFigure *fig,
	gpointer user_data )
{
	MfWindow *window = MF_WINDOW( user_data );

	g_list_store_append( window->figures, G_OBJECT( fig ) );

	gtk_widget_queue_draw( GTK_WIDGET( window->draw_area ) );
}

static void
on_factory_setup(
	GtkSignalListItemFactory *self,
	GObject *object,
	gpointer user_data )
{
	MfFigureInfo *info;

	info = mf_figure_info_new();
	gtk_list_item_set_child( GTK_LIST_ITEM( object ), GTK_WIDGET( info ) );
}

static void
on_factory_bind(
	GtkSignalListItemFactory *self,
	GObject *object,
	gpointer user_data )
{
	GtkListItem *list_item = GTK_LIST_ITEM( object );
	MfFigure *figure;
	MfFigureInfo *info;
	MfLayout *layout;
	gchar *name;
	graphene_vec4_t color;

	figure = MF_FIGURE( gtk_list_item_get_item( list_item ) );
	info = MF_FIGURE_INFO( gtk_list_item_get_child( list_item ) );

	layout = mf_figure_get_layout( figure );
	name = mf_layout_get_name( layout );
	g_object_unref( G_OBJECT( layout ) );
	mf_figure_info_set_name( info, name );
	g_free( name );

	mf_figure_get_color( figure, &color );
	mf_figure_info_set_color( info, &color );
}

static void
mf_window_init(
	MfWindow *self )
{
	GBytes *models_bytes;
	GtkEventControllerKey *event_key;
	GtkBox *hbox, *vbox, *button_box;
	MfSwitchButton *switch_button;
	GtkButton *randomize_button;
	GtkNotebook *notebook;
	GtkFrame *draw_area_frame;
	MfDrawArea *draw_area;
	GListStore *layouts;
	MfCustomFigure *custom_figure;
	GtkListItemFactory *factory;
	GtkSelectionModel *selection;
	GtkListView *list_view;
	GtkScrolledWindow *scrolled, *scroll_custom;
	GError *error = NULL;

	/* create model list */
  models_bytes = g_resource_lookup_data(
		resource_get_resource(),
		"/resource/models/models.ini",
		G_RESOURCE_LOOKUP_FLAGS_NONE,
		&error );
	if( error != NULL )
	{
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "MESSAGE", error->message, NULL );
		g_clear_error( &error );
		return;
	}

	self->model_asset = mf_model_asset_new_from_bytes( models_bytes, &error );
	g_bytes_unref( models_bytes );
	if( error != NULL )
	{
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, "MESSAGE", error->message, NULL );
		g_clear_error( &error );
		return;
	}

	/* setup private */
	self->figures = g_list_store_new( MF_TYPE_FIGURE );
	self->timer = 0;

	/* setup window */
	gtk_window_set_title( GTK_WINDOW( self ), PROGRAM_NAME );
	gtk_window_set_resizable( GTK_WINDOW( self ), TRUE );
	gtk_window_set_decorated( GTK_WINDOW( self ), TRUE );
	gtk_window_set_default_size( GTK_WINDOW( self ), PROGRAM_WINDOW_WIDTH, PROGRAM_WINDOW_HEIGHT );

	/* add event controller */
	event_key = GTK_EVENT_CONTROLLER_KEY( gtk_event_controller_key_new() );
	gtk_widget_add_controller( GTK_WIDGET( self ), GTK_EVENT_CONTROLLER( event_key ) );
	g_signal_connect( G_OBJECT( event_key ), "key-pressed", G_CALLBACK( on_event_key_pressed ), self );

	/* create widgets */
	vbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_VERTICAL, 1 ) );
	gtk_widget_set_halign( GTK_WIDGET( vbox ), GTK_ALIGN_END );

	hbox = GTK_BOX( gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 1 ) );

	button_box = GTK_BOX( gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 1 ) );
	gtk_widget_set_valign( GTK_WIDGET( button_box ), GTK_ALIGN_END );
	gtk_box_set_homogeneous( button_box, TRUE );

	switch_button = mf_switch_button_new();
	mf_switch_button_set_first_label( switch_button, "Start" );
	mf_switch_button_set_second_label( switch_button, "Stop" );
	g_signal_connect( G_OBJECT( switch_button ), "first-clicked", G_CALLBACK( on_switch_button_first_clicked ), self );
	g_signal_connect( G_OBJECT( switch_button ), "second-clicked", G_CALLBACK( on_switch_button_second_clicked ), self );

	randomize_button = GTK_BUTTON( gtk_button_new_with_label( "Randomize" ) );
	g_signal_connect( G_OBJECT( randomize_button ), "clicked", G_CALLBACK( on_randomize_button_clicked ), self );
	
	draw_area_frame = GTK_FRAME( gtk_frame_new( NULL ) );
	self->draw_area_frame = draw_area_frame;

	draw_area = mf_draw_area_new();
	gtk_widget_set_hexpand( GTK_WIDGET( draw_area ), TRUE );
	gtk_widget_set_halign( GTK_WIDGET( draw_area ), GTK_ALIGN_FILL );
	g_signal_connect( G_OBJECT( draw_area ), "realize", G_CALLBACK( on_draw_area_realize ), self );
	g_signal_connect( G_OBJECT( draw_area ), "unrealize", G_CALLBACK( on_draw_area_unrealize ), self );
	g_signal_connect( G_OBJECT( draw_area ), "render", G_CALLBACK( on_draw_area_render ), self );
	g_signal_connect( G_OBJECT( draw_area ), "resize", G_CALLBACK( on_draw_area_resize ), self );
	self->draw_area = draw_area;

	layouts = mf_model_asset_get_layouts( self->model_asset );
	custom_figure = mf_custom_figure_new_with_layouts( layouts );
	g_object_unref( G_OBJECT( layouts ) );
	g_signal_connect( G_OBJECT( custom_figure ), "create", G_CALLBACK( on_custom_figure_create ), self );
	self->custom_figure = custom_figure;

	factory = GTK_LIST_ITEM_FACTORY( gtk_signal_list_item_factory_new() );
	g_signal_connect( G_OBJECT( factory ), "setup", G_CALLBACK( on_factory_setup ), self );
	g_signal_connect( G_OBJECT( factory ), "bind", G_CALLBACK( on_factory_bind ), self );

	selection = GTK_SELECTION_MODEL( gtk_multi_selection_new( G_LIST_MODEL( g_object_ref( G_OBJECT( self->figures ) ) ) ) );
	list_view = GTK_LIST_VIEW( gtk_list_view_new( selection, factory ) );
	gtk_list_view_set_enable_rubberband( list_view, FALSE );
	gtk_list_view_set_header_factory( list_view, NULL );
	gtk_list_view_set_show_separators( list_view, TRUE );
	gtk_list_view_set_single_click_activate( list_view, FALSE );
	gtk_list_view_set_tab_behavior( list_view, GTK_LIST_TAB_CELL );
	self->list_view = list_view;

	scroll_custom = GTK_SCROLLED_WINDOW( gtk_scrolled_window_new() );
	gtk_scrolled_window_set_policy( scroll_custom, GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
	gtk_scrolled_window_set_has_frame( scroll_custom, FALSE );
	gtk_scrolled_window_set_kinetic_scrolling( scroll_custom, FALSE );
	gtk_scrolled_window_set_overlay_scrolling( scroll_custom, FALSE );
	gtk_scrolled_window_set_propagate_natural_height( scroll_custom, TRUE );
	gtk_scrolled_window_set_child( scroll_custom, GTK_WIDGET( custom_figure ) );

	scrolled = GTK_SCROLLED_WINDOW( gtk_scrolled_window_new() );
	gtk_scrolled_window_set_policy( scrolled, GTK_POLICY_NEVER, GTK_POLICY_ALWAYS );
	gtk_scrolled_window_set_has_frame( scrolled, FALSE );
	gtk_scrolled_window_set_kinetic_scrolling( scrolled, FALSE );
	gtk_scrolled_window_set_overlay_scrolling( scrolled, FALSE );
	gtk_scrolled_window_set_propagate_natural_height( scrolled, TRUE );
	gtk_scrolled_window_set_child( scrolled, GTK_WIDGET( list_view ) );

	notebook = GTK_NOTEBOOK( gtk_notebook_new() );
	gtk_widget_set_vexpand( GTK_WIDGET( notebook ), TRUE );
	gtk_widget_set_valign( GTK_WIDGET( notebook ), GTK_ALIGN_FILL );
	gtk_notebook_append_page( notebook, GTK_WIDGET( scroll_custom ), gtk_label_new( "Create" ) );
	gtk_notebook_append_page( notebook, GTK_WIDGET( scrolled ), gtk_label_new( "Remove" ) );

	/* layout widgets */
	gtk_window_set_child( GTK_WINDOW( self ), GTK_WIDGET( hbox ) );
	gtk_frame_set_child( draw_area_frame, GTK_WIDGET( draw_area ) );
	gtk_box_append( hbox, GTK_WIDGET( draw_area_frame ) );
	gtk_box_append( hbox, GTK_WIDGET( vbox ) );
	gtk_box_append( vbox, GTK_WIDGET( notebook ) );
	gtk_box_append( vbox, GTK_WIDGET( button_box ) );
	gtk_box_append( button_box, GTK_WIDGET( switch_button ) );
	gtk_box_append( button_box, GTK_WIDGET( randomize_button ) );
}

static void
mf_window_dispose(
	GObject *object )
{
	MfWindow *self = MF_WINDOW( object );

	g_clear_object( &self->model_asset );
	g_clear_object( &self->figures );

	G_OBJECT_CLASS( mf_window_parent_class )->dispose( object );
}

static void
mf_window_finalize(
	GObject *object )
{
	MfWindow *self = MF_WINDOW( object );

	if( self->timer > 0 )
		g_source_remove( self->timer );

	G_OBJECT_CLASS( mf_window_parent_class )->finalize( object );
}

static void
mf_window_class_init(
	MfWindowClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );

	object_class->dispose = mf_window_dispose;
	object_class->finalize = mf_window_finalize;
}

MfWindow*
mf_window_new(
	MfApplication *app )
{
	g_return_val_if_fail( MF_IS_APPLICATION( app ), NULL );

	return MF_WINDOW( g_object_new( MF_TYPE_WINDOW, "application", app, NULL ) );
}

