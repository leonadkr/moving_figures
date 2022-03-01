#include <math.h>
#include "gpolygon.h"

#define SRT( s ) #s
#define XSRT( s ) SRT( s )

#define G_POLYGON_MIN_CORNER 3
#define G_POLYGON_MAX_CORNER 10
#define G_POLYGON_CORNER_NUM ( G_POLYGON_MAX_CORNER - G_POLYGON_MIN_CORNER + 1 )
#define G_POLYGON_CORNER_PROP_BLURB "Number of polygon corners in [" XSRT( G_POLYGON_MIN_CORNER ) ":" XSRT( G_POLYGON_MAX_CORNER ) "]"

struct _GPolygonPrivate
{
	gfloat rvel;
	guint corner;
	gfloat angle;
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

/*
	static members
*/
static GParamSpec *object_props[N_PROPS] = { NULL, };
static guint g_polygon_class_mode[2];
static guint g_polygon_class_offset[2][G_POLYGON_CORNER_NUM];
static guint g_polygon_class_count[2][G_POLYGON_CORNER_NUM];

/*
	private methods
*/
static void g_polygon_real_move( GFigure *figure, GLRectangle *rect );
static void g_polygon_real_randomize( GFigure *figure, GRand *rnd, GLRectangle *rect, gfloat fps );
static GLRendererData g_polygon_real_get_data( GFigure *figure );
static GLRendererLayout* g_polygon_class_create_layout( gboolean filled, guint corner );

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
		0.0f,
		G_PARAM_READWRITE );
	object_props[PROP_CORNER] = g_param_spec_uint(
		"corner",
		"Number of corners",
		G_POLYGON_CORNER_PROP_BLURB,
		G_POLYGON_MIN_CORNER,
		G_POLYGON_MAX_CORNER,
		G_POLYGON_MIN_CORNER,
		G_PARAM_READWRITE );
	object_props[PROP_ANGLE] = g_param_spec_float(
		"angle",
		"Angle from x-axis",
		"Angle from x-axis in radians as float",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0f,
		G_PARAM_READABLE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->move = g_polygon_real_move;
	figure_class->randomize = g_polygon_real_randomize;
	figure_class->get_data = g_polygon_real_get_data;
}

static void
g_polygon_real_move(
	GFigure *figure,
	GLRectangle *rect )
{
	GPolygon *polygon;
	GPolygonPrivate *priv;

	g_return_if_fail( G_IS_FIGURE( figure ) );

	G_FIGURE_CLASS( g_polygon_parent_class )->move( figure, rect );
	
	polygon = G_POLYGON( figure );
	priv = g_polygon_get_instance_private( polygon );

	priv->angle += priv->rvel;
	if( priv->angle > (gfloat)G_PI )
		priv->angle -= (gfloat)2.0 * G_PI;
	if( priv->angle < (gfloat)-G_PI )
		priv->angle += (gfloat)2.0 * G_PI;
}

static void
g_polygon_real_randomize(
	GFigure *figure,
	GRand *rnd,
	GLRectangle *rect,
	gfloat fps )
{
	GPolygon *self;
	GPolygonPrivate *priv;

	g_return_if_fail( G_IS_FIGURE( figure ) );
	g_return_if_fail( fps > 0.0f );
	
	self = G_POLYGON( figure );
	priv = g_polygon_get_instance_private( self );

	G_FIGURE_CLASS( g_polygon_parent_class )->randomize( figure, rnd, rect, fps );

	priv->rvel = (gfloat)g_rand_double_range( rnd, -G_PI, G_PI ) / fps;
	priv->corner = g_rand_int_range( rnd, G_POLYGON_MIN_CORNER, G_POLYGON_MAX_CORNER + 1 );
}

static GLRendererData
g_polygon_real_get_data(
	GFigure *figure )
{
	GPolygon *self;
	GPolygonPrivate *priv;
	guint k;
	gfloat rsin, rcos;
	gboolean filled;
	gfloat radius;
	GLRendererData data = GL_RENDERER_DATA_DEFAULT;

	g_return_val_if_fail( G_IS_FIGURE( figure ), GL_RENDERER_DATA_DEFAULT );

	self = G_POLYGON( figure );
	priv = g_polygon_get_instance_private( self );

	data = G_FIGURE_CLASS( g_polygon_parent_class )->get_data( figure );

	g_object_get( G_OBJECT( self ),
		"filled", &filled,
		"radius", &radius,
		NULL );

	k = priv->corner - G_POLYGON_MIN_CORNER;
	data.mode = g_polygon_class_mode[filled];
	data.offset = g_polygon_class_offset[filled][k];
	data.count = g_polygon_class_count[filled][k];

	rsin = radius * sinf( priv->angle );
	rcos = radius * cosf( priv->angle );
	data.srtm[0] = rsin;
	data.srtm[1] = rcos;
	data.srtm[4] = -rcos;
	data.srtm[5] = rsin;

	return data;
}

static GLRendererLayout*
g_polygon_class_create_layout(
	gboolean filled,
	guint corner )
{
	GLsizeiptr i, j, dc;
	GLfloat da;
	GLRendererLayout *layout = g_new( GLRendererLayout, 1 );

	dc = filled ? 2 : 0;

	/* vertices */
	layout->vertex_num = corner + dc;
	layout->vertex_size = sizeof( GLfloat ) * 2 * layout->vertex_num;
	layout->vertex = (GLfloat*)g_malloc( layout->vertex_size );

	j = 0;
	if( filled )
	{
		layout->vertex[j++] = 0.0f;
		layout->vertex[j++] = 0.0f;
	}
	da = (gfloat)2.0 * G_PI / (gfloat)corner;
	for( i = 0; i < corner; ++i )
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

/*
	public
*/
GPolygon*
g_polygon_new(
	void )
{
	return G_POLYGON( g_object_new( G_TYPE_POLYGON, NULL ) );
}

GLRendererLayout*
g_polygon_class_get_layout(
	void )
{
	GLsizeiptr i, j, k, offset;
	GLRendererLayout *layouts[2*G_POLYGON_CORNER_NUM], *layout;

	g_polygon_class_mode[FALSE] = GL_LINE_LOOP;
	g_polygon_class_mode[TRUE] = GL_TRIANGLE_FAN;

	offset = 0;
	k = 0;
	for( i = 0; i < 2; ++i )
		for( j = 0; j < G_POLYGON_CORNER_NUM; ++j )
		{
			layouts[k] = g_polygon_class_create_layout( i, G_POLYGON_MIN_CORNER + j );
			g_polygon_class_offset[i][j] = offset;
			g_polygon_class_count[i][j] = layouts[k]->index_num;
			offset += layouts[k]->index_size;
			k++;
		}

	layout = gl_renderer_layout_new_merged( layouts, 2 * G_POLYGON_CORNER_NUM );
	for( k = 0; k < 2 * G_POLYGON_CORNER_NUM; ++k )
		gl_renderer_layout_free( layouts[k] );

	return layout;
}

