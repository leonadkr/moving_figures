#include <math.h>
#include "gcircle.h"

#define G_CIRCLE_CORNER_NUM 30

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
static guint g_circle_class_mode[2];
static guint g_circle_class_offset[2];
static guint g_circle_class_count[2];

static void g_circle_real_randomize( GFigure *figure, GRand *rnd, GLRectangle *rect, gfloat fps );
static GLRendererData g_circle_real_get_data( GFigure *figure );
static GLRendererLayout* g_circle_class_create_layout( gboolean filled );

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
		2.0f,
		G_MAXFLOAT,
		10.0f,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->randomize = g_circle_real_randomize;
	figure_class->get_data = g_circle_real_get_data;
}

static void
g_circle_real_randomize(
	GFigure *figure,
	GRand *rnd,
	GLRectangle *rect,
	gfloat fps )
{
	GCircle *self;
	GCirclePrivate *priv;

	g_return_if_fail( G_IS_FIGURE( figure ) );
	g_return_if_fail( fps > 0.0f );
	
	self = G_CIRCLE( figure );
	priv = g_circle_get_instance_private( self );

	G_FIGURE_CLASS( g_circle_parent_class )->randomize( figure, rnd, rect, fps );

	priv->filled = (gboolean)g_rand_int_range( rnd, 0, 2 );
	priv->radius = (gfloat)g_rand_double_range( rnd, 10.0, 20.0 );
}

static GLRendererData
g_circle_real_get_data(
	GFigure *figure )
{
	GCircle *self;
	GCirclePrivate *priv;
	GLRendererData data = GL_RENDERER_DATA_DEFAULT;

	g_return_val_if_fail( G_IS_FIGURE( figure ), GL_RENDERER_DATA_DEFAULT );

	self = G_CIRCLE( figure );
	priv = g_circle_get_instance_private( self );

	data = G_FIGURE_CLASS( g_circle_parent_class )->get_data( figure );

	data.mode = g_circle_class_mode[priv->filled];
	data.offset = g_circle_class_offset[priv->filled];
	data.count = g_circle_class_count[priv->filled];

	data.srtm[0] = priv->radius;
	data.srtm[5] = priv->radius;

	return data;
}

static GLRendererLayout*
g_circle_class_create_layout(
	gboolean filled )
{
	GLsizeiptr i, j, dc;
	GLfloat da;
	GLRendererLayout *layout = g_new( GLRendererLayout, 1 );

	dc = filled ? 2 : 0;

	/* vertices */
	layout->vertex_num = G_CIRCLE_CORNER_NUM + dc;
	layout->vertex_size = sizeof( GLfloat ) * 2 * layout->vertex_num;
	layout->vertex = (GLfloat*)g_malloc( layout->vertex_size );

	j = 0;
	if( filled )
	{
		layout->vertex[j++] = 0.0f;
		layout->vertex[j++] = 0.0f;
	}
	da = (gfloat)2.0 * G_PI / (gfloat)G_CIRCLE_CORNER_NUM;
	for( i = 0; i < G_CIRCLE_CORNER_NUM; ++i )
	{
		layout->vertex[j++] = sinf( (gfloat)i * da );
		layout->vertex[j++] = cosf( (gfloat)i * da );
	}
	if( filled )
	{
		layout->vertex[j++] = sinf( (gfloat)i * da );
		layout->vertex[j++] = cosf( (gfloat)i * da );
	}

	/* indices */
	layout->index_num = layout->vertex_num;
	layout->index_size = sizeof( GLuint ) * layout->index_num;
	layout->index = (GLuint*)g_malloc( layout->index_size );

	for( i = 0; i < layout->index_num; ++i )
		layout->index[i] = i;

	return layout;
}

GCircle*
g_circle_new(
	void )
{
	return G_CIRCLE( g_object_new( G_TYPE_CIRCLE, NULL ) );
}

GLRendererLayout*
g_circle_class_get_layout(
	void )
{
	GLsizeiptr i, offset;
	GLRendererLayout *layouts[2], *layout;

	g_circle_class_mode[FALSE] = GL_LINE_LOOP;
	g_circle_class_mode[TRUE] = GL_TRIANGLE_FAN;
	offset = 0;
	for( i = 0; i < 2; ++i )
	{
		layouts[i] = g_circle_class_create_layout( i );
		g_circle_class_offset[i] = offset;
		g_circle_class_count[i] = layouts[i]->index_num;
		offset += layouts[i]->index_size;
	}

	layout = gl_renderer_layout_new_merged( layouts, 2 );
	for( i = 0; i < 2; ++i )
		gl_renderer_layout_free( layouts[i] );

	return layout;
}

