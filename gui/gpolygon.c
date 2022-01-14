#include <math.h>
#include "gpolygon.h"

struct _GPolygonPrivate
{
	gint rvel;
	guint corner;
	gint angle;
};
typedef struct _GPolygonPrivate GPolygonPrivate;

enum _GPolygonPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_RVEL,
	PROP_CORNER,
	PROP_ANGLE,

	N_PROPS
};
typedef enum _GPolygonPropertyID GPolygonPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void g_polygon_real_move( GFigure*, GdkRectangle* );
static inline double degree_to_radian( double degree );
static void g_polygon_real_draw( GFigure*, cairo_t* );

G_DEFINE_TYPE_WITH_PRIVATE( GPolygon, g_polygon, G_TYPE_CIRCLE )

static void
g_polygon_init(
	GPolygon *self )
{
	GPolygonPrivate *priv = g_polygon_get_instance_private( self );
	const GValue *value;

	value = g_param_spec_get_default_value( object_props[PROP_RVEL] );
	priv->rvel = g_value_get_int( value );
	
	value = g_param_spec_get_default_value( object_props[PROP_CORNER] );
	priv->corner = g_value_get_uint( value );

	value = g_param_spec_get_default_value( object_props[PROP_ANGLE] );
	priv->angle = g_value_get_int( value );
}

static void
g_polygon_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	GPolygon *self = G_POLYGON( object );
	GPolygonPrivate *priv = g_polygon_get_instance_private( self );

	switch( (GPolygonPropertyID)prop_id )
	{
		case PROP_RVEL:
			g_value_set_int( value, priv->rvel );
			break;
		case PROP_CORNER:
			g_value_set_uint( value, priv->corner );
			break;
		case PROP_ANGLE:
			g_value_set_int( value, priv->angle );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_polygon_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	GPolygon *self = G_POLYGON( object );
	GPolygonPrivate *priv = g_polygon_get_instance_private( self );

	switch( (GPolygonPropertyID)prop_id )
	{
		case PROP_RVEL:
			priv->rvel = g_value_get_int( value );
			break;
		case PROP_CORNER:
			priv->corner = g_value_get_uint( value );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_polygon_class_init(
	GPolygonClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GFigureClass *figure_class = G_FIGURE_CLASS( klass );

	object_class->get_property = g_polygon_get_property;
	object_class->set_property = g_polygon_set_property;
	object_props[PROP_RVEL] = g_param_spec_int(
		"rvel",
		"Rotate velocity",
		"Rotate velocity in degrees as int",
		-G_MAXINT,
		G_MAXINT,
		0,
		G_PARAM_READWRITE );
	object_props[PROP_CORNER] = g_param_spec_uint(
		"corner",
		"Number of corners",
		"Number of polygon corners in [3:MAXINT] as int",
		3,
		G_MAXINT,
		3,
		G_PARAM_READWRITE );
	object_props[PROP_ANGLE] = g_param_spec_int(
		"angle",
		"Angle from x-axis",
		"Angle from x-axis in [-180:180] degrees as int",
		-180,
		180,
		0,
		G_PARAM_READABLE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->move = g_polygon_real_move;
	figure_class->draw = g_polygon_real_draw;
}

static void
g_polygon_real_move(
	GFigure *figure,
	GdkRectangle *rect )
{
	GPolygon *polygon;
	GFigureClass *figure_class;
	GPolygonPrivate *priv;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	/* chaining up */
	figure_class = G_FIGURE_CLASS( g_polygon_parent_class );
	figure_class->move( figure, rect );
	
	polygon = G_POLYGON( figure );
	priv = g_polygon_get_instance_private( polygon );

	priv->angle += priv->rvel;
	if( priv->angle >= 180 )
		priv->angle -= 360;
	if( priv->angle < -180 )
		priv->angle += 360;
}

static inline double
degree_to_radian(
	double degree )
{
	return G_PI * degree / 180.0;
}

static void
g_polygon_real_draw(
	GFigure *figure,
	cairo_t *cr )
{
	GPolygon *polygon;
	GPolygonPrivate *priv;
	gint x, y;
	gint i, px, py;
	double da;
	guint radius, fill_mode;
	GdkRGBA *color;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	polygon = G_POLYGON( figure );
	priv = g_polygon_get_instance_private( polygon );
	g_object_get( G_OBJECT( polygon ),
		"x", &x,
		"y", &y,
		"radius", &radius,
		"fill-mode", &fill_mode,
		"color", &color,
		NULL );

	cairo_save( cr );

	cairo_set_source_rgba( cr, color->red, color->green, color->blue, color->alpha );
	cairo_set_line_width( cr, 1.0 );

	da = (double)360.0 / priv->corner;
	px = x + radius * cos( degree_to_radian( (double)priv->angle ) );
	py = y + radius * sin( degree_to_radian( (double)priv->angle ) );
	cairo_move_to( cr, px, py );
	for( i = 1; i < priv->corner; ++i )
	{
		px = x + radius * cos( degree_to_radian( (double)priv->angle + da * i ) );
		py = y + radius * sin( degree_to_radian( (double)priv->angle + da * i ) );
		cairo_line_to( cr, px, py );
	}
	cairo_close_path( cr );

	switch( fill_mode )
	{
		case G_FIGURE_FILL_MODE_FILL:
			cairo_fill( cr );
			break;
		case G_FIGURE_FILL_MODE_UNFILL:
		default:
			cairo_stroke( cr );
			break;
	}

	cairo_restore( cr );
}

/*
	public
*/
GPolygon*
g_polygon_new(
	void )
{
	return G_POLYGON( g_object_new( G_TYPE_POLYGON, NULL ) );
}

