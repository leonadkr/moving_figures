#include "gcircle.h"

struct _GCirclePrivate
{
	guint fill_mode;
	guint radius;
	GdkRGBA color;
};
typedef struct _GCirclePrivate GCirclePrivate;

enum _GCirclePropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_FILL_MODE,
	PROP_RADIUS,

	N_PROPS
};
typedef enum _GCirclePropertyID GCirclePropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void g_circle_real_draw( GFigure*, cairo_t* );

G_DEFINE_TYPE_WITH_PRIVATE( GCircle, g_circle, G_TYPE_POINT )

static void
g_circle_init(
	GCircle *self )
{
	GCirclePrivate *priv = g_circle_get_instance_private( self );
	const GValue *value;

	value = g_param_spec_get_default_value( object_props[PROP_FILL_MODE] );
	priv->fill_mode = g_value_get_uint( value );

	value = g_param_spec_get_default_value( object_props[PROP_RADIUS] );
	priv->radius = g_value_get_uint( value );
}

static void
g_circle_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	GCircle *self = G_CIRCLE( object );
	GCirclePrivate *priv = g_circle_get_instance_private( self );

	switch( (GCirclePropertyID)prop_id )
	{
		case PROP_FILL_MODE:
			g_value_set_uint( value, priv->fill_mode );
			break;
		case PROP_RADIUS:
			g_value_set_uint( value, priv->radius );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_circle_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	GCircle *self = G_CIRCLE( object );
	GCirclePrivate *priv = g_circle_get_instance_private( self );

	switch( (GCirclePropertyID)prop_id )
	{
		case PROP_FILL_MODE:
			priv->fill_mode = g_value_get_uint( value );
			break;
		case PROP_RADIUS:
			priv->radius = g_value_get_uint( value );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_circle_class_init(
	GCircleClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GFigureClass *figure_class = G_FIGURE_CLASS( klass );

	object_class->get_property = g_circle_get_property;
	object_class->set_property = g_circle_set_property;
	object_props[PROP_FILL_MODE] = g_param_spec_uint(
		"fill-mode",
		"Fill mode",
		"Mode of filling the figure",
		0,
		N_G_FIGURE_FILL_MODE - 1,
		0,
		G_PARAM_READWRITE );
	object_props[PROP_RADIUS] = g_param_spec_uint(
		"radius",
		"Radius",
		"Radius of figure",
		0,
		G_MAXINT,
		10,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->draw = g_circle_real_draw;
}

static void
g_circle_real_draw(
	GFigure *figure,
	cairo_t *cr )
{
	GCircle *circle;
	GCirclePrivate *priv;
	gint x, y;
	guint fill_mode;
	GdkRGBA *color;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	circle = G_CIRCLE( figure );
	priv = g_circle_get_instance_private( circle );
	g_object_get( G_OBJECT( circle ),
		"x", &x,
		"y", &y,
		"fill-mode", &fill_mode,
		"color", &color,
		NULL );

	cairo_save( cr );

	cairo_set_source_rgba( cr, color->red, color->green, color->blue, color->alpha );
	cairo_set_line_width( cr, 1.0 );
	cairo_arc( cr, x, y, priv->radius, 0.0, 2.0 * G_PI );

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
GCircle*
g_circle_new(
	void )
{
	return G_CIRCLE( g_object_new( G_TYPE_CIRCLE, NULL ) );
}
