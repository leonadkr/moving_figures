#include <math.h>
#include "gstar.h"

#define MAX_VERTEX_NUM 22

struct _GStarPrivate
{
	guint corner;

	gfloat vertex[2*MAX_VERTEX_NUM];
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
static void g_star_calculate_vertices( GStar *self );
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

	g_star_calculate_vertices( self );
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
			g_star_calculate_vertices( self );
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
		"Number of star corners as int",
		5,
		MAX_VERTEX_NUM - 2,
		5,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->draw = g_star_real_draw;
}

static void
g_star_calculate_vertices(
	GStar *self )
{
	GStarPrivate *priv;
	gfloat da;
	gint i, j, k, dk;

	g_return_if_fail( G_IS_STAR( self ) );

	priv = g_star_get_instance_private( self );

	if( priv->corner % 2 == 0 )
	{
		priv->vertex[0] = 0.0;
		priv->vertex[1] = 0.0;
		da = 2.0 * G_PI / (gfloat)priv->corner;
		dk = 2;
		for( i = 0, j = 2, k = 0; i < priv->corner / 2; ++i, j += 2, k += dk )
		{
			priv->vertex[j] = sinf( (gfloat)k * da );
			priv->vertex[j+1] = cosf( (gfloat)k * da );
		}

		j += 2;
		priv->vertex[j] = 0.0;
		priv->vertex[j+1] = 0.0;
		for( i = 0, j += 2, k = 1; i < priv->corner / 2; ++i, j += 2, k += dk )
		{
			priv->vertex[j] = sinf( (gfloat)k * da );
			priv->vertex[j+1] = cosf( (gfloat)k * da );
		}
	} else
	{
		priv->vertex[0] = 0.0;
		priv->vertex[1] = 0.0;
		da = 2.0 * G_PI / (gfloat)priv->corner;
		dk = ( priv->corner - 1 ) / 2;
		for( i = 0, j = 2, k = 0; i < priv->corner; ++i, j += 2, k += dk )
		{
			priv->vertex[j] = sinf( (gfloat)k * da );
			priv->vertex[j+1] = cosf( (gfloat)k * da );
		}
	}
}

static void
g_star_real_draw(
	GFigure *figure,
	cairo_t *cr )
{
	GStar *self;
	GStarPrivate *priv;
	gfloat radius, angle;
	gfloat x, y;
	gint i, j;
	gboolean filled;
	GdkRGBA *color;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	self = G_STAR( figure );
	priv = g_star_get_instance_private( self );

	g_object_get( G_OBJECT( figure ),
		"x", &x,
		"y", &y,
		"radius", &radius,
		"filled", &filled,
		"color", &color,
		"angle", &angle,
		NULL );

	cairo_save( cr );

	cairo_set_source_rgba( cr, color->red, color->green, color->blue, color->alpha );
	cairo_set_line_width( cr, 1.0 );

	if( priv->corner % 2 == 0 )
	{
		cairo_move_to(
			cr,
			x + radius * ( sinf( angle ) * priv->vertex[2] + cosf( angle ) * priv->vertex[3] ),
			y + radius * ( -cosf( angle ) * priv->vertex[2] + sinf( angle ) * priv->vertex[3] ) );
		for( i = 1, j = 4; i < priv->corner / 2; ++i, j += 2 )
			cairo_line_to(
				cr,
				x + radius * ( sinf( angle ) * priv->vertex[j] + cosf( angle ) * priv->vertex[j+1] ),
				y + radius * ( -cosf( angle ) * priv->vertex[j] + sinf( angle ) * priv->vertex[j+1] ) );
		cairo_close_path( cr );

		j += 4;
		cairo_move_to(
			cr,
			x + radius * ( sinf( angle ) * priv->vertex[j] + cosf( angle ) * priv->vertex[j+1] ),
			y + radius * ( -cosf( angle ) * priv->vertex[j] + sinf( angle ) * priv->vertex[j+1] ) );
		for( i = 1, j += 2; i < priv->corner / 2; ++i, j += 2 )
			cairo_line_to(
				cr,
				x + radius * ( sinf( angle ) * priv->vertex[j] + cosf( angle ) * priv->vertex[j+1] ),
				y + radius * ( -cosf( angle ) * priv->vertex[j] + sinf( angle ) * priv->vertex[j+1] ) );
		cairo_close_path( cr );
	}
	else
	{
		cairo_move_to(
			cr,
			x + radius * ( sinf( angle ) * priv->vertex[2] + cosf( angle ) * priv->vertex[3] ),
			y + radius * ( -cosf( angle ) * priv->vertex[2] + sinf( angle ) * priv->vertex[3] ) );
		for( i = 1, j = 4; i < priv->corner; ++i, j += 2 )
			cairo_line_to(
				cr,
				x + radius * ( sinf( angle ) * priv->vertex[j] + cosf( angle ) * priv->vertex[j+1] ),
				y + radius * ( -cosf( angle ) * priv->vertex[j] + sinf( angle ) * priv->vertex[j+1] ) );
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

