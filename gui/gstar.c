#include <math.h>
#include "gstar.h"

struct _GStarPrivate
{
	guint corner;
};
typedef struct _GStarPrivate GStarPrivate;

enum _GStarPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_CORNER,

	N_PROPS
};
typedef enum _GStarPropertyID GStarPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void g_star_real_draw( GFigure*, cairo_t* );

G_DEFINE_TYPE_WITH_PRIVATE( GStar, g_star, G_TYPE_POLYGON )

static void
g_star_init(
	GStar *self )
{
	GStarPrivate *priv = g_star_get_instance_private( self );
	const GValue *value;

	value = g_param_spec_get_default_value( object_props[PROP_CORNER] );
	priv->corner = g_value_get_uint( value );
}

static void
g_star_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	GStar *self = G_STAR( object );
	GStarPrivate *priv = g_star_get_instance_private( self );

	switch( (GStarPropertyID)prop_id )
	{
		case PROP_CORNER:
			g_value_set_uint( value, priv->corner );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_star_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	GStar *self = G_STAR( object );
	GStarPrivate *priv = g_star_get_instance_private( self );

	switch( (GStarPropertyID)prop_id )
	{
		case PROP_CORNER:
			priv->corner = g_value_get_uint( value );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_star_class_init(
	GStarClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GFigureClass *figure_class = G_FIGURE_CLASS( klass );

	object_class->get_property = g_star_get_property;
	object_class->set_property = g_star_set_property;
	object_props[PROP_CORNER] = g_param_spec_uint(
		"corner",
		"Number of corners",
		"Number of star corners in [5:MAXINT] as int",
		5,
		G_MAXINT,
		5,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->draw = g_star_real_draw;
}

static void
g_star_real_draw(
	GFigure *figure,
	cairo_t *cr )
{
	gfloat x, y, angle;
	gint i, j, dj, corner;
	gfloat px, py, da, radius;
	gboolean filled;
	GdkRGBA *color;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	g_object_get( G_OBJECT( figure ),
		"x", &x,
		"y", &y,
		"radius", &radius,
		"filled", &filled,
		"color", &color,
		"corner", &corner,
		"angle", &angle,
		NULL );

	cairo_save( cr );

	cairo_set_source_rgba( cr, color->red, color->green, color->blue, color->alpha );
	cairo_set_line_width( cr, 1.0 );

	if( corner % 2 == 0 )
	{
		dj = 2;
		da = (gfloat)2.0 * G_PI / corner;

		j = 0;
		px = x + radius * cosf( angle );
		py = y + radius * sinf( angle );
		cairo_move_to( cr, px, py );
		for( i = 1; i < corner / 2; ++i )
		{
			j += dj;
			px = x + radius * cosf( angle + da * (gfloat)j );
			py = y + radius * sinf( angle + da * (gfloat)j );
			cairo_line_to( cr, px, py );
		}
		cairo_close_path( cr );

		angle += da;
		j = 0;
		px = x + radius * cosf( angle );
		py = y + radius * sinf( angle );
		cairo_move_to( cr, px, py );
		for( i = 1; i < corner / 2; ++i )
		{
			j += dj;
			px = x + radius * cosf( angle + da * (gfloat)j );
			py = y + radius * sinf( angle + da * (gfloat)j );
			cairo_line_to( cr, px, py );
		}
		cairo_close_path( cr );
	} else
	{
		dj = ( corner - 1 ) / 2;
		da = (gfloat)2.0 * G_PI / corner;
		
		j = 0;
		px = x + radius * cosf( angle );
		py = y + radius * sinf( angle );
		cairo_move_to( cr, px, py );
		for( i = 1; i < corner; ++i )
		{
			j += dj;
			px = x + radius * cosf( angle + da * (gfloat)j );
			py = y + radius * sinf( angle + da * (gfloat)j );
			cairo_line_to( cr, px, py );
		}
		cairo_close_path( cr );
	}

	if( filled )
		cairo_fill( cr );
	else
		cairo_stroke( cr );

	cairo_restore( cr );
}

/*
	public
*/
GStar*
g_star_new(
	void )
{
	return G_STAR( g_object_new( G_TYPE_STAR, NULL ) );
}

