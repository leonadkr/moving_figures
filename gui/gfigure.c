#include "gfigure.h"

#define G_FIGURE_DEFAULT_VELOCITY_MAX ( 200.0 )

struct _GFigurePrivate
{
	gfloat x, y;
	gfloat velx, vely;
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

/*
	static members
*/
static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void g_figure_real_move( GFigure *self, GLRectangle *rect );
static void g_figure_real_randomize( GFigure *self, GRand *rnd, GLRectangle *rect, gfloat fps );
static GLRendererData g_figure_real_get_data( GFigure *self );

G_DEFINE_TYPE_WITH_PRIVATE( GFigure, g_figure, G_TYPE_OBJECT )

static void
g_figure_init(
	GFigure *self )
{
	GFigurePrivate *priv = g_figure_get_instance_private( self );
	const GValue *value;

	value = g_param_spec_get_default_value( object_props[PROP_X] );
	priv->x = g_value_get_float( value );
	
	value = g_param_spec_get_default_value( object_props[PROP_Y] );
	priv->y = g_value_get_float( value );

	value = g_param_spec_get_default_value( object_props[PROP_VELX] );
	priv->velx = g_value_get_float( value );

	value = g_param_spec_get_default_value( object_props[PROP_VELY] );
	priv->vely = g_value_get_float( value );
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
			g_value_set_float( value, priv->x );
			break;
		case PROP_Y:
			g_value_set_float( value, priv->y );
			break;
		case PROP_VELX:
			g_value_set_float( value, priv->velx );
			break;
		case PROP_VELY:
			g_value_set_float( value, priv->vely );
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
			priv->x = g_value_get_float( value );
			break;
		case PROP_Y:
			priv->y = g_value_get_float( value );
			break;
		case PROP_VELX:
			priv->velx = g_value_get_float( value );
			break;
		case PROP_VELY:
			priv->vely = g_value_get_float( value );
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
	object_props[PROP_X] = g_param_spec_float(
		"x",
		"X",
		"X coordiante of the figure as float",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0f,
		G_PARAM_READWRITE );
	object_props[PROP_Y] = g_param_spec_float(
		"y",
		"Y",
		"Y coordiante of the figure as float",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0f,
		G_PARAM_READWRITE );
	object_props[PROP_VELX] = g_param_spec_float(
		"velx",
		"VelX",
		"X coordiante of the figure velocity as float",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0f,
		G_PARAM_READWRITE );
	object_props[PROP_VELY] = g_param_spec_float(
		"vely",
		"VelY",
		"Y coordiante of the figure velocity as float",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0f,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	klass->move = g_figure_real_move;
	klass->randomize = g_figure_real_randomize;
	klass->get_data = g_figure_real_get_data;
}

static void
g_figure_real_move(
	GFigure *self,
	GLRectangle *rect )
{
	GFigurePrivate *priv;

	g_return_if_fail( G_IS_FIGURE( self ) );

	if( !g_figure_is_inside_rect( self, rect ) )
		return;
	
	priv = g_figure_get_instance_private( self );

	if( priv->x + priv->velx > rect->x + rect->width || priv->x + priv->velx < rect->x )
		priv->velx = -priv->velx;
	if( priv->y + priv->vely > rect->y + rect->height || priv->y + priv->vely < rect->y )
		priv->vely = -priv->vely;

	priv->x += priv->velx;
	priv->y += priv->vely;
}

static void
g_figure_real_randomize(
	GFigure *self,
	GRand *rnd,
	GLRectangle *rect,
	gfloat fps )
{
	GFigurePrivate *priv;

	g_return_if_fail( G_IS_FIGURE( self ) );
	g_return_if_fail( fps > 0.0f );
	
	priv = g_figure_get_instance_private( self );

	priv->x = (gfloat)g_rand_double_range( rnd, 0.0, (gdouble)rect->width );
	priv->y = (gfloat)g_rand_double_range( rnd, 0.0, (gdouble)rect->height );
	priv->velx = (gfloat)g_rand_double_range( rnd, -G_FIGURE_DEFAULT_VELOCITY_MAX, G_FIGURE_DEFAULT_VELOCITY_MAX ) / fps;
	priv->vely = (gfloat)g_rand_double_range( rnd, -G_FIGURE_DEFAULT_VELOCITY_MAX, G_FIGURE_DEFAULT_VELOCITY_MAX ) / fps;
}

static GLRendererData
g_figure_real_get_data(
	GFigure *self )
{
	GFigurePrivate *priv;
	GLRendererData data = (GLRendererData){
	.mode = GL_POINTS,
	.offset = 0,
	.count = 0,
	.color = {
		0.0f, 0.0f, 0.0f, 1.0f },
	.srtm = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f } };

	g_return_val_if_fail( G_IS_FIGURE( self ), data );

	priv = g_figure_get_instance_private( self );

	data.srtm[3] = priv->x;
	data.srtm[7] = priv->y;

	return data;
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
	GLRectangle *rect )
{
	GFigureClass *klass;

	g_return_if_fail( G_IS_FIGURE( self ) );
	klass = G_FIGURE_GET_CLASS( self );

	g_return_if_fail( klass->move != NULL );
	klass->move( self, rect );
}

void
g_figure_randomize(
	GFigure *self,
	GRand *rnd,
	GLRectangle *rect,
	gfloat fps )
{
	GFigureClass *klass;

	g_return_if_fail( G_IS_FIGURE( self ) );
	klass = G_FIGURE_GET_CLASS( self );

	g_return_if_fail( klass->randomize != NULL );
	klass->randomize( self, rnd, rect, fps );
}

GLRendererData
g_figure_get_data(
	GFigure *self )
{
	GFigureClass *klass;
	GLRendererData data = (GLRendererData){
	.mode = GL_POINTS,
	.offset = 0,
	.count = 0,
	.color = {
		0.0f, 0.0f, 0.0f, 1.0f },
	.srtm = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f } };

	g_return_val_if_fail( G_IS_FIGURE( self ), data );
	klass = G_FIGURE_GET_CLASS( self );

	g_return_val_if_fail( klass->get_data != NULL, data );
	return klass->get_data( self );
}

gboolean
g_figure_is_inside_rect(
	GFigure *self,
	GLRectangle *rect )
{
	GFigurePrivate *priv;

	g_return_val_if_fail( G_IS_FIGURE( self ), FALSE );

	priv = g_figure_get_instance_private( self );

	if(	priv->x < rect->x ||
			priv->x > rect->x + rect->width ||
			priv->y < rect->y ||
			priv->y > rect->y + rect->height )
		return FALSE;

	return TRUE;
}

GLRendererLayout*
g_figure_class_get_layout(
	void )
{
	GLRendererLayout *layout = g_new( GLRendererLayout, 1 );

	layout->vertex = NULL;
	layout->vertex_num = 0;
	layout->vertex_size = 0;
	layout->index = NULL;
	layout->index_num = 0;
	layout->index_size = 0;

	return layout;
}

