#include "gfigure.h"

struct _GFigurePrivate
{
	gint x, y;
	gint velx, vely;
};
typedef struct _GFigurePrivate GFigurePrivate;

enum _GFigurePropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_X,
	PROP_Y,
	PROP_VELX,
	PROP_VELY,

	N_PROPS
};
typedef enum _GFigurePropertyID GFigurePropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void g_figure_real_move( GFigure*, GdkRectangle* );

G_DEFINE_TYPE_WITH_PRIVATE( GFigure, g_figure, G_TYPE_OBJECT )

static void
g_figure_init(
	GFigure *self )
{
	GFigurePrivate *priv = g_figure_get_instance_private( self );
	const GValue *value;

	value = g_param_spec_get_default_value( object_props[PROP_X] );
	priv->x = g_value_get_int( value );
	
	value = g_param_spec_get_default_value( object_props[PROP_Y] );
	priv->y = g_value_get_int( value );

	value = g_param_spec_get_default_value( object_props[PROP_VELX] );
	priv->velx = g_value_get_int( value );

	value = g_param_spec_get_default_value( object_props[PROP_VELY] );
	priv->vely = g_value_get_int( value );
}

static void
g_figure_get_property(
	GObject *object,	
	guint prop_id,	
	GValue *value,	
	GParamSpec *pspec )
{
	GFigure *self = G_FIGURE( object );
	GFigurePrivate *priv = g_figure_get_instance_private( self );

	switch( (GFigurePropertyID)prop_id )
	{
		case PROP_X:
			g_value_set_int( value, priv->x );
			break;
		case PROP_Y:
			g_value_set_int( value, priv->y );
			break;
		case PROP_VELX:
			g_value_set_int( value, priv->velx );
			break;
		case PROP_VELY:
			g_value_set_int( value, priv->vely );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_figure_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	GFigure *self = G_FIGURE( object );
	GFigurePrivate *priv = g_figure_get_instance_private( self );

	switch( (GFigurePropertyID)prop_id )
	{
		case PROP_X:
			priv->x = g_value_get_int( value );
			break;
		case PROP_Y:
			priv->y = g_value_get_int( value );
			break;
		case PROP_VELX:
			priv->velx = g_value_get_int( value );
			break;
		case PROP_VELY:
			priv->vely = g_value_get_int( value );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
g_figure_class_init(
	GFigureClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );

	object_class->get_property = g_figure_get_property;
	object_class->set_property = g_figure_set_property;
	object_props[PROP_X] = g_param_spec_int(
		"x",
		"X",
		"X coordiante of the figure as int",
		-G_MAXINT,
		G_MAXINT,
		0,
		G_PARAM_READWRITE );
	object_props[PROP_Y] = g_param_spec_int(
		"y",
		"Y",
		"Y coordiante of the figure as int",
		-G_MAXINT,
		G_MAXINT,
		0,
		G_PARAM_READWRITE );
	object_props[PROP_VELX] = g_param_spec_int(
		"velx",
		"VelX",
		"X coordiante of the figure velocity as int",
		-G_MAXINT,
		G_MAXINT,
		0,
		G_PARAM_READWRITE );
	object_props[PROP_VELY] = g_param_spec_int(
		"vely",
		"VelY",
		"Y coordiante of the figure velocity as int",
		-G_MAXINT,
		G_MAXINT,
		0,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	klass->move = g_figure_real_move;
	klass->draw = NULL;
}

static void
g_figure_real_move(
	GFigure *self,
	GdkRectangle *rect )
{
	GFigurePrivate *priv;

	g_return_if_fail( G_IS_FIGURE( self ) );
	
	priv = g_figure_get_instance_private( self );

	if( priv->x + priv->velx >= rect->x + rect->width || priv->x + priv->velx <= rect->x )
		priv->velx = -priv->velx;
	if( priv->y + priv->vely >= rect->y + rect->height || priv->y + priv->vely <= rect->y )
		priv->vely = -priv->vely;

	priv->x += priv->velx;
	priv->y += priv->vely;
}

/*
	public
*/
GFigure*
g_figure_new(
	void )
{
	return G_FIGURE( g_object_new( G_TYPE_FIGURE, NULL ) );
}

void
g_figure_move(
	GFigure *self,
	GdkRectangle *rect )
{
	GFigureClass *klass;

	g_return_if_fail( G_IS_FIGURE( self ) );
	klass = G_FIGURE_GET_CLASS( self );

	g_return_if_fail( klass->move != NULL );
	klass->move( self, rect );
}

void
g_figure_draw(
	GFigure *self,
	cairo_t *cr )
{
	GFigureClass *klass;

	g_return_if_fail( G_IS_FIGURE( self ) );
	klass = G_FIGURE_GET_CLASS( self );

	g_return_if_fail( klass->draw != NULL );
	klass->draw( self, cr );
}

gboolean
g_figure_is_inside_rect(
	GFigure *self,
	GdkRectangle *rect )
{
	GFigurePrivate *priv;

	g_return_val_if_fail( G_IS_FIGURE( self ), FALSE );

	priv = g_figure_get_instance_private( self );

	return gdk_rectangle_contains_point( rect, priv->x, priv->y );
}

