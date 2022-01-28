#include <math.h>
#include "gpolygon.h"

#define MAX_VERTEX_NUM 21

struct _GPolygonPrivate
{
	gfloat rvel;
	guint corner;
	gfloat angle;

	gfloat vertex[2*MAX_VERTEX_NUM];
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
static void g_polygon_calculate_vertices( GPolygon *self );
static void g_polygon_real_move( GFigure*, GdkRectangle* );
static void g_polygon_real_draw( GFigure*, cairo_t* );

G_DEFINE_TYPE_WITH_PRIVATE( GPolygon, g_polygon, G_TYPE_CIRCLE )

static void
g_polygon_init(
	GPolygon *self )
{
	GPolygonPrivate *priv = g_polygon_get_instance_private( self );
	const GValue *value;

	value = g_param_spec_get_default_value( object_props[PROP_RVEL] );
	priv->rvel = g_value_get_float( value );
	
	value = g_param_spec_get_default_value( object_props[PROP_CORNER] );
	priv->corner = g_value_get_uint( value );

	value = g_param_spec_get_default_value( object_props[PROP_ANGLE] );
	priv->angle = g_value_get_float( value );

	g_polygon_calculate_vertices( self );
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
			g_value_set_float( value, priv->rvel );
			break;
		case PROP_CORNER:
			g_value_set_uint( value, priv->corner );
			break;
		case PROP_ANGLE:
			g_value_set_float( value, priv->angle );
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
			priv->rvel = g_value_get_float( value );
			break;
		case PROP_CORNER:
			priv->corner = g_value_get_uint( value );
			g_polygon_calculate_vertices( self );
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
	object_props[PROP_RVEL] = g_param_spec_float(
		"rvel",
		"Rotate velocity",
		"Rotate velocity in radians as float",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0,
		G_PARAM_READWRITE );
	object_props[PROP_CORNER] = g_param_spec_uint(
		"corner",
		"Number of corners",
		"Number of polygon corners as unsigned int",
		3,
		MAX_VERTEX_NUM - 1,
		3,
		G_PARAM_READWRITE );
	object_props[PROP_ANGLE] = g_param_spec_float(
		"angle",
		"Angle from x-axis",
		"Angle from x-axis in radians as float",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0,
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
	if( priv->angle > G_PI )
		priv->angle -= 2.0 * G_PI;
	if( priv->angle < -G_PI )
		priv->angle += 2.0 * G_PI;
}

static void
g_polygon_calculate_vertices(
	GPolygon *self )
{
	GPolygonPrivate *priv;
	gfloat da;
	gint i, j;

	g_return_if_fail( G_IS_POLYGON( self ) );

	priv = g_polygon_get_instance_private( self );

	priv->vertex[0] = 0.0;
	priv->vertex[1] = 0.0;
	da = 2.0 * G_PI / (gfloat)priv->corner;
	for( i = 0, j = 2; i < priv->corner; ++i, j += 2 )
	{
		priv->vertex[j] = sinf( (gfloat)i * da );
		priv->vertex[j+1] = cosf( (gfloat)i * da );
	}
}

static void
g_polygon_real_draw(
	GFigure *figure,
	cairo_t *cr )
{
	GPolygon *polygon;
	GPolygonPrivate *priv;
	gfloat x, y;
	gint i, j;
	gfloat radius;
	gboolean filled;
	GdkRGBA *color;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	polygon = G_POLYGON( figure );
	priv = g_polygon_get_instance_private( polygon );
	g_object_get( G_OBJECT( polygon ),
		"x", &x,
		"y", &y,
		"radius", &radius,
		"filled", &filled,
		"color", &color,
		NULL );

	cairo_save( cr );

	cairo_set_source_rgba( cr, color->red, color->green, color->blue, color->alpha );
	cairo_set_line_width( cr, 1.0 );

	cairo_move_to(
		cr,
		x + radius * ( sinf( priv->angle ) * priv->vertex[2] + cosf( priv->angle ) * priv->vertex[3] ),
		y + radius * ( -cosf( priv->angle ) * priv->vertex[2] + sinf( priv->angle ) * priv->vertex[3] ) );
	for( i = 1, j = 4; i < priv->corner; ++i, j += 2 )
		cairo_line_to(
			cr,
			x + radius * ( sinf( priv->angle ) * priv->vertex[j] + cosf( priv->angle ) * priv->vertex[j+1] ),
			y + radius * ( -cosf( priv->angle ) * priv->vertex[j] + sinf( priv->angle ) * priv->vertex[j+1] ) );
	cairo_close_path( cr );

	if( filled )
		cairo_fill( cr );
	else
		cairo_stroke( cr );

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

