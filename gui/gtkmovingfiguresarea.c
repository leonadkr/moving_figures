#include <stdlib.h>
#include <gtk/gtk.h>

#include "gtkmovingfiguresarea.h"

#include "fps.h"
#include "gpoint.h"
#include "gcircle.h"
#include "gpolygon.h"
#include "gstar.h"

struct _GtkMovingFiguresAreaPrivate
{
	gboolean is_realized;
	guint minimum_width, minimum_height;
	GList *fig[N_GTK_MOVING_FIGURE_TYPE];
	gint fignum[N_GTK_MOVING_FIGURE_TYPE];
};
typedef struct _GtkMovingFiguresAreaPrivate GtkMovingFiguresAreaPrivate;

enum _GtkMovingFiguresAreaPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_MINIMUM_WIDTH,
	PROP_MINIMUM_HEIGHT,

	N_PROPS
};
typedef enum _GtkMovingFiguresAreaPropertyID GtkMovingFiguresAreaPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void g_figure_randomize( GFigure *figure, GRand *rnd, GtkMovingFigureType fig_type, GdkRectangle *rect );
static void gtk_moving_figures_area_get_max_size_rect( GtkMovingFiguresArea *self, GdkRectangle *rect );
static GtkSizeRequestMode gtk_moving_figures_area_real_get_request_mode( GtkWidget *widget );
static void gtk_moving_figures_area_real_measure( GtkWidget *widget, GtkOrientation orientation, gint for_size, gint *minimum, gint *natural, gint *minimum_baseline, gint *natural_baseline );
static void gtk_moving_figures_area_real_shapshot( GtkWidget *widget, GtkSnapshot *snapshot );

G_DEFINE_TYPE_WITH_PRIVATE( GtkMovingFiguresArea, gtk_moving_figures_area, GTK_TYPE_WIDGET )

static void
gtk_moving_figures_area_init(
	GtkMovingFiguresArea *self )
{
	GtkMovingFiguresAreaPrivate *priv = gtk_moving_figures_area_get_instance_private( self );
	const GValue *value;
	guint fig_type;

	value = g_param_spec_get_default_value( object_props[PROP_MINIMUM_WIDTH] );
	priv->minimum_width = g_value_get_uint( value );

	value = g_param_spec_get_default_value( object_props[PROP_MINIMUM_HEIGHT] );
	priv->minimum_height = g_value_get_uint( value );

	priv->is_realized = FALSE;
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
	{
		priv->fig[fig_type] = NULL;
		priv->fignum[fig_type] = 0;
	}
}

static void
gtk_moving_figures_area_dispose(
	GObject *object )
{
	GtkMovingFiguresArea *self = GTK_MOVING_FIGURES_AREA( object );
	GtkMovingFiguresAreaPrivate *priv = gtk_moving_figures_area_get_instance_private( self );
	guint fig_type;

	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		if( priv->fig[fig_type] != NULL )
		{
			g_list_free_full( priv->fig[fig_type], (GDestroyNotify)g_object_unref );
			priv->fig[fig_type] = NULL;
		}

	G_OBJECT_CLASS( gtk_moving_figures_area_parent_class )->dispose( object );
}

static void
gtk_moving_figures_area_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	GtkMovingFiguresArea *self = GTK_MOVING_FIGURES_AREA( object );
	GtkMovingFiguresAreaPrivate *priv = gtk_moving_figures_area_get_instance_private( self );

	switch( (GtkMovingFiguresAreaPropertyID)prop_id )
	{
		case PROP_MINIMUM_WIDTH:
			g_value_set_uint( value, priv->minimum_width );
			break;
		case PROP_MINIMUM_HEIGHT:
			g_value_set_uint( value, priv->minimum_height );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
gtk_moving_figures_area_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	GtkMovingFiguresArea *self = GTK_MOVING_FIGURES_AREA( object );
	GtkMovingFiguresAreaPrivate *priv = gtk_moving_figures_area_get_instance_private( self );

	switch( (GtkMovingFiguresAreaPropertyID)prop_id )
	{
		case PROP_MINIMUM_WIDTH:
			priv->minimum_width = g_value_get_uint( value );
			break;
		case PROP_MINIMUM_HEIGHT:
			priv->minimum_height = g_value_get_uint( value );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
gtk_moving_figures_area_class_init(
	GtkMovingFiguresAreaClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS( klass );

	object_class->dispose = gtk_moving_figures_area_dispose;
	object_class->get_property = gtk_moving_figures_area_get_property;
	object_class->set_property = gtk_moving_figures_area_set_property;
	object_props[PROP_MINIMUM_WIDTH] = g_param_spec_uint(
		"minimum-width",
		"Minimum width",
		"Set minimum width of the canvas",
		1,
		G_MAXUINT,
		1,
		G_PARAM_READWRITE );
	object_props[PROP_MINIMUM_HEIGHT] = g_param_spec_uint(
		"minimum-height",
		"Minimum height",
		"Set minimum height of the canvas",
		1,
		G_MAXUINT,
		1,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	widget_class->get_request_mode = gtk_moving_figures_area_real_get_request_mode;
	widget_class->measure = gtk_moving_figures_area_real_measure;
	widget_class->snapshot = gtk_moving_figures_area_real_shapshot;
}

static GtkSizeRequestMode
gtk_moving_figures_area_real_get_request_mode(
	GtkWidget *widget )
{
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
gtk_moving_figures_area_real_measure(
	GtkWidget *widget,
	GtkOrientation orientation,
	gint for_size,
	gint *minimum,
	gint *natural,
	gint *minimum_baseline, gint *natural_baseline )
{
	GtkMovingFiguresArea *self;
	GtkMovingFiguresAreaPrivate *priv;

	g_return_if_fail( GTK_WIDGET( widget ) );

	self = GTK_MOVING_FIGURES_AREA( widget );
	priv = gtk_moving_figures_area_get_instance_private( self );

	if( orientation == GTK_ORIENTATION_HORIZONTAL )
		*minimum = *natural = priv->minimum_width;
	else
		*minimum = *natural = priv->minimum_height;
}

static void
gtk_moving_figures_area_real_shapshot(
	GtkWidget *widget,
	GtkSnapshot *snapshot )
{
	GtkMovingFiguresArea *self;
	GtkMovingFiguresAreaPrivate *priv;
	cairo_t *cr;
	int fig_type;
	GList *l;
	GdkRGBA color;
	GdkRectangle rect;
	
	g_return_if_fail( GTK_WIDGET( widget ) );

	self = GTK_MOVING_FIGURES_AREA( widget );
	priv = gtk_moving_figures_area_get_instance_private( self );

	/* make all figures placed in the initially allocated rectangle */
	if( !priv->is_realized )
	{
		priv->is_realized = TRUE;
		gtk_moving_figures_area_reallocate( self );
	}
		
	/* get the allocated ractangle */
	gtk_moving_figures_area_get_max_size_rect( self, &rect );
	cr = gtk_snapshot_append_cairo( snapshot, &GRAPHENE_RECT_INIT( rect.x, rect.y, rect.width, rect.height ) );

	/* make white canvas */
	color = (GdkRGBA){
		.red = 1.0,
		.green = 1.0,
		.blue = 1.0,
		.alpha = 1.0 };
	gdk_cairo_set_source_rgba( cr, &color );
	gdk_cairo_rectangle( cr, &( rect ) );
	cairo_fill( cr );

	/* draw all figures */
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = priv->fig[fig_type]; l != NULL; l = l->next )
			if( g_figure_is_inside_rect( G_FIGURE( l->data ), &( rect ) ) )
				g_figure_draw( G_FIGURE( l->data ), cr );

	cairo_destroy( cr );
}

static void
g_figure_randomize(
	GFigure *figure,
	GRand *rnd,
	GtkMovingFigureType fig_type,
	GdkRectangle *rect )
{
	GdkRGBA color;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	switch( fig_type )
	{
		case GTK_MOVING_FIGURE_TYPE_STAR:
		case GTK_MOVING_FIGURE_TYPE_POLYGON:
			g_object_set( G_OBJECT( figure ),
				"corner", g_rand_int_range( rnd, 5, 10 ),
				"rvel", g_rand_int_range( rnd, -180, 180 ) / FPS,
				NULL );
		case GTK_MOVING_FIGURE_TYPE_CIRCLE:
			g_object_set( G_OBJECT( figure ),
				"radius", g_rand_int_range( rnd, 10, 20 ),
				"fill-mode", g_rand_int_range( rnd, 0, N_G_FIGURE_FILL_MODE ),
				NULL );
		case GTK_MOVING_FIGURE_TYPE_POINT:
			color = (GdkRGBA){
				.red = g_rand_double( rnd ),
				.green = g_rand_double( rnd ),
				.blue = g_rand_double( rnd ),
				.alpha = 1.0 };
			g_object_set( G_OBJECT( figure ),
				"x", rect->x + g_rand_int_range( rnd, 0, rect->width ),
				"y", rect->y + g_rand_int_range( rnd, 0, rect->height ),
				"velx", g_rand_int_range( rnd, -200, 200 ) / FPS,
				"vely", g_rand_int_range( rnd, -200, 200 ) / FPS,
				"color", &color,
				NULL );
			break;
		default:
			g_return_if_fail( fig_type < N_GTK_MOVING_FIGURE_TYPE );
			break;
	}
}

static void
gtk_moving_figures_area_get_max_size_rect(
	GtkMovingFiguresArea *self,
	GdkRectangle *rect )
{
	GtkMovingFiguresAreaPrivate *priv;
	guint max_w, max_h;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );

	priv = gtk_moving_figures_area_get_instance_private( self );
	
	max_w = MAX( priv->minimum_width, gtk_widget_get_width( GTK_WIDGET( self ) ) );
	max_h = MAX( priv->minimum_height, gtk_widget_get_height( GTK_WIDGET( self ) ) );
	*rect = (GdkRectangle){
		.x = 0,
		.y = 0,
		.width = max_w,
		.height = max_h
	};
}


/*
	public
*/
GtkMovingFiguresArea*
gtk_moving_figures_area_new(
	guint width,
	guint height )
{
	return GTK_MOVING_FIGURES_AREA( g_object_new( GTK_TYPE_MOVING_FIGURES_AREA,
		"minimum-width", width,
		"minimum-height", height,
		NULL ) );
}

void
gtk_moving_figures_area_reallocate(
	GtkMovingFiguresArea *self )
{
	GtkMovingFiguresAreaPrivate *priv;
	GdkRectangle rect;
	guint fig_type;
	GList *l;
	GRand *rnd;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );

	priv = gtk_moving_figures_area_get_instance_private( self );
	gtk_moving_figures_area_get_max_size_rect( self, &rect );

	rnd = g_rand_new();
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = priv->fig[fig_type]; l != NULL; l = l->next )
			g_figure_randomize( G_FIGURE( l->data ), rnd, fig_type, &rect );
	g_rand_free( rnd );
}

void
gtk_moving_figures_area_move(
	GtkMovingFiguresArea *self )
{
	GtkMovingFiguresAreaPrivate *priv;
	GdkRectangle rect;
	guint fig_type;
	GList *l;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );

	priv = gtk_moving_figures_area_get_instance_private( self );
	gtk_moving_figures_area_get_max_size_rect( self, &rect );

	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = priv->fig[fig_type]; l != NULL; l = l->next )
			if( g_figure_is_inside_rect( G_FIGURE( l->data ), &rect ) )
				g_figure_move( G_FIGURE( l->data ), &rect );
}

void
gtk_moving_figures_area_append(
	GtkMovingFiguresArea *self,
	GtkMovingFigureType fig_type,
	gint num )
{
	GtkMovingFiguresAreaPrivate *priv;
	GdkRectangle rect;
	guint i;
	GFigure *figure;
	GRand *rnd;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );
	g_return_if_fail( num >= 0 );

	priv = gtk_moving_figures_area_get_instance_private( self );
	gtk_moving_figures_area_get_max_size_rect( self, &rect );

	rnd = g_rand_new();
	for( i = 0; i < num; ++i )
	{
		switch( fig_type )
		{
			case GTK_MOVING_FIGURE_TYPE_POINT:
				figure = G_FIGURE( g_point_new() );
				break;
			case GTK_MOVING_FIGURE_TYPE_CIRCLE:
				figure = G_FIGURE( g_circle_new() );
				break;
			case GTK_MOVING_FIGURE_TYPE_POLYGON:
				figure = G_FIGURE( g_polygon_new() );
				break;
			case GTK_MOVING_FIGURE_TYPE_STAR:
				figure = G_FIGURE( g_star_new() );
				break;
			default:
				g_return_if_fail( fig_type < N_GTK_MOVING_FIGURE_TYPE );
				figure = NULL;
				break;
		}
		g_figure_randomize( figure, rnd, fig_type, &rect );
		priv->fig[fig_type] = g_list_prepend( priv->fig[fig_type], figure );
	}
	priv->fignum[fig_type] += num;
	g_rand_free( rnd );
}

void
gtk_moving_figures_area_remove(
	GtkMovingFiguresArea *self,
	GtkMovingFigureType fig_type,
	gint num )
{
	GtkMovingFiguresAreaPrivate *priv;
	guint i;
	GList *l;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );
	g_return_if_fail( num >= 0 );

	priv = gtk_moving_figures_area_get_instance_private( self );

	for( i = 0, l = priv->fig[fig_type]; i < num && l != NULL; ++i, l = l->next )
		g_object_unref( G_OBJECT( l->data ) );
	priv->fig[fig_type] = l;

	if( num > priv->fignum[fig_type] )
		priv->fignum[fig_type] = 0;
	else
		priv->fignum[fig_type] -= num;
}

void
gtk_moving_figures_area_set_number(
	GtkMovingFiguresArea *self,
	GtkMovingFigureType fig_type,
	gint num )
{
	GtkMovingFiguresAreaPrivate *priv;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );
	g_return_if_fail( num >= 0 );

	priv = gtk_moving_figures_area_get_instance_private( self );

	if( num > priv->fignum[fig_type] )
		gtk_moving_figures_area_append( self, fig_type, num - priv->fignum[fig_type] );
	else
		gtk_moving_figures_area_remove( self, fig_type, priv->fignum[fig_type] - num );
}

