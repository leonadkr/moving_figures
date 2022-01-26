#include "gpoint.h"

struct _GPointPrivate
{
	GdkRGBA color;
};
typedef struct _GPointPrivate GPointPrivate;

enum _GPointPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_COLOR,

	N_PROPS
};
typedef enum _GPointPropertyID GPointPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void g_point_real_draw( GFigure*, cairo_t* );

G_DEFINE_TYPE_WITH_PRIVATE( GPoint, g_point, G_TYPE_FIGURE )

static void
g_point_init(
	GPoint *self )
{
	GPointPrivate *priv = g_point_get_instance_private( self );

	priv->color = (GdkRGBA){ 0.0, 0.0, 0.0, 1.0 };
}

static void
g_point_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	GPoint *self = G_POINT( object );
	GPointPrivate *priv = g_point_get_instance_private( self );

	switch( (GPointPropertyID)prop_id )
	{
		case PROP_COLOR:
			g_value_set_boxed( value, &( priv->color ) );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_point_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	GPoint *self = G_POINT( object );
	GPointPrivate *priv = g_point_get_instance_private( self );

	switch( (GPointPropertyID)prop_id )
	{
		case PROP_COLOR:
			priv->color = *(GdkRGBA*)g_value_get_boxed( value );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_point_class_init(
	GPointClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GFigureClass *figure_class = G_FIGURE_CLASS( klass );

	object_class->get_property = g_point_get_property;
	object_class->set_property = g_point_set_property;
	object_props[PROP_COLOR] = g_param_spec_boxed(
		"color",
		"Color",
		"Color of the figure as GdkRGBA",
		GDK_TYPE_RGBA,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->draw = g_point_real_draw;
}

static void
g_point_real_draw(
	GFigure *figure,
	cairo_t *cr )
{
	GPoint *point;
	GPointPrivate *priv;
	gfloat x, y;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	point = G_POINT( figure );
	priv = g_point_get_instance_private( point );
	g_object_get( G_OBJECT( point ),
		"x", &x,
		"y", &y,
		NULL );

	cairo_save( cr );

	cairo_set_source_rgba( cr, priv->color.red, priv->color.green, priv->color.blue, priv->color.alpha );
	cairo_set_line_width( cr, 1.0 );
	cairo_arc( cr, x, y, 2.0, 0.0, 2.0 * G_PI );
	cairo_fill( cr );

	cairo_restore( cr );
}

/*
	public
*/
GPoint*
g_point_new(
	void )
{
	return G_POINT( g_object_new( G_TYPE_POINT, NULL ) );
}
