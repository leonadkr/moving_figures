#include <math.h>
#include "gpoint.h"

struct _GPointPrivate
{
	gfloat color[4];
};
typedef struct _GPointPrivate GPointPrivate;

enum _GPointPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_COLOR,

	N_PROPS
};
typedef enum _GPointPropertyID GPointPropertyID;

/*
	static members
*/
static GParamSpec *object_props[N_PROPS] = { NULL, };
static GLenum g_point_class_mode[1];
static GLsizeiptr g_point_class_offset[1];
static GLsizeiptr g_point_class_count[1];

/*
	private methods
*/
static void g_point_real_randomize( GFigure *figure, GRand *rnd, GLRectangle *rect, gfloat fps );
static GLRendererData g_point_real_get_data( GFigure *figure );

G_DEFINE_TYPE_WITH_PRIVATE( GPoint, g_point, G_TYPE_FIGURE )

static void
g_point_init(
	GPoint *self )
{
	GPointPrivate *priv = g_point_get_instance_private( self );

	priv->color[0] = 0.0f;
	priv->color[1] = 0.0f;
	priv->color[2] = 0.0f;
	priv->color[3] = 1.0f;
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
			g_value_set_pointer( value, &( priv->color[0] ) );
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
	gfloat *color;

	switch( (GPointPropertyID)prop_id )
	{
		case PROP_COLOR:
			color = g_value_get_pointer( value );
			priv->color[0] = color[0];
			priv->color[1] = color[1];
			priv->color[2] = color[2];
			priv->color[3] = color[3];
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
	object_props[PROP_COLOR] = g_param_spec_pointer(
		"color",
		"Color",
		"Color of the figure as gfloat[4]",
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->randomize = g_point_real_randomize;
	figure_class->get_data = g_point_real_get_data;
}

static void
g_point_real_randomize(
	GFigure *figure,
	GRand *rnd,
	GLRectangle *rect,
	gfloat fps )
{
	GPoint *self;
	GPointPrivate *priv;

	g_return_if_fail( G_IS_FIGURE( figure ) );
	g_return_if_fail( fps > 0.0f );
	
	self = G_POINT( figure );
	priv = g_point_get_instance_private( self );

	G_FIGURE_CLASS( g_point_parent_class )->randomize( figure, rnd, rect, fps );

	priv->color[0] = (GLfloat)g_rand_double( rnd );
	priv->color[1] = (GLfloat)g_rand_double( rnd );
	priv->color[2] = (GLfloat)g_rand_double( rnd );
	priv->color[3] = 1.0f;
}

static GLRendererData
g_point_real_get_data(
	GFigure *figure )
{
	GPoint *self;
	GPointPrivate *priv;
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

	g_return_val_if_fail( G_IS_FIGURE( figure ), data );

	self = G_POINT( figure );
	priv = g_point_get_instance_private( self );

	data = G_FIGURE_CLASS( g_point_parent_class )->get_data( figure );

	data.mode = g_point_class_mode[0];
	data.offset = g_point_class_offset[0];
	data.count = g_point_class_count[0];
	data.color[0] = priv->color[0];
	data.color[1] = priv->color[1];
	data.color[2] = priv->color[2];
	data.color[3] = priv->color[3];

	return data;
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

GLRendererLayout*
g_point_class_get_layout(
	void )
{
	GLRendererLayout *layout = g_new( GLRendererLayout, 1 );

	/* vertices */
	layout->vertex_num = 1;
	layout->vertex_size = sizeof( GLfloat ) * 2 * layout->vertex_num;
	layout->vertex = (GLfloat*)g_malloc( layout->vertex_size );

	layout->vertex[0] = 0.0f;
	layout->vertex[1] = 0.0f;

	/* indices */
	layout->index_num = layout->vertex_num;
	layout->index_size = sizeof( GLuint ) * layout->index_num;
	layout->index = (GLuint*)g_malloc( layout->index_size );

	layout->index[0] = 0;

	g_point_class_mode[0] = GL_POINTS;
	g_point_class_offset[0] = 0;
	g_point_class_count[0] = 1;

	return layout;
}

