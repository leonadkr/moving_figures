#include "gcircle.h"

struct _GCirclePrivate
{
	gboolean filled;
	gfloat radius;
};
typedef struct _GCirclePrivate GCirclePrivate;

enum _GCirclePropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_FILLED,
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

	value = g_param_spec_get_default_value( object_props[PROP_FILLED] );
	priv->filled = g_value_get_boolean( value );

	value = g_param_spec_get_default_value( object_props[PROP_RADIUS] );
	priv->radius = g_value_get_float( value );
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
		case PROP_FILLED:
			g_value_set_boolean( value, priv->filled );
			break;
		case PROP_RADIUS:
			g_value_set_float( value, priv->radius );
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
		case PROP_FILLED:
			priv->filled = g_value_get_boolean( value );
			break;
		case PROP_RADIUS:
			priv->radius = g_value_get_float( value );
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
	object_props[PROP_FILLED] = g_param_spec_boolean(
		"filled",
		"Filling the figure",
		"Indication of filling the figure",
		FALSE,
		G_PARAM_READWRITE );
	object_props[PROP_RADIUS] = g_param_spec_float(
		"radius",
		"Radius",
		"Radius of figure",
		2.0,
		G_MAXFLOAT,
		10.0,
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
	gfloat x, y;
	gboolean filled;
	GdkRGBA *color;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	circle = G_CIRCLE( figure );
	priv = g_circle_get_instance_private( circle );
	g_object_get( G_OBJECT( circle ),
		"x", &x,
		"y", &y,
		"filled", &filled,
		"color", &color,
		NULL );

	cairo_save( cr );

	cairo_set_source_rgba( cr, color->red, color->green, color->blue, color->alpha );
	cairo_set_line_width( cr, 1.0 );
	cairo_arc( cr, x, y, priv->radius, 0.0, 2.0 * G_PI );

	if( filled )
		cairo_fill( cr );
	else
		cairo_stroke( cr );

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
