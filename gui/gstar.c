#include <math.h>
#include "gstar.h"

#define SRT( s ) #s
#define XSRT( s ) SRT( s )

#define G_STAR_MIN_CORNER 5
#define G_STAR_MAX_CORNER 10
#define G_STAR_CORNER_NUM ( G_STAR_MAX_CORNER - G_STAR_MIN_CORNER + 1 )
#define G_STAR_CORNER_PROP_BLURB "Number of star corners in [" XSRT( G_STAR_MIN_CORNER ) ":" XSRT( G_STAR_MAX_CORNER ) "]"

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

/*
	static members
*/
static GParamSpec *object_props[N_PROPS] = { NULL, };
static guint g_star_class_mode[2];
static guint g_star_class_offset[2][G_STAR_CORNER_NUM];
static guint g_star_class_count[2][G_STAR_CORNER_NUM];

/*
	private methods
*/
static void g_star_real_randomize( GFigure *figure, GRand *rnd, GLRectangle *rect, gfloat fps );
static GLRendererData g_star_real_get_data( GFigure *figure );
static GLRendererLayout* g_star_class_create_layout_odd( gboolean filled, guint corner, gfloat angle );
static GLRendererLayout* g_star_class_create_layout_even( gboolean filled, guint corner, gfloat angle );
static GLRendererLayout* g_star_class_create_layout( gboolean filled, guint corner );

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
		G_STAR_CORNER_PROP_BLURB,
		G_STAR_MIN_CORNER,
		G_STAR_MAX_CORNER,
		G_STAR_MIN_CORNER,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	figure_class->randomize = g_star_real_randomize;
	figure_class->get_data = g_star_real_get_data;
}

static void
g_star_real_randomize(
	GFigure *figure,
	GRand *rnd,
	GLRectangle *rect,
	gfloat fps )
{
	GStar *self;
	GStarPrivate *priv;

	g_return_if_fail( G_IS_FIGURE( figure ) );
	g_return_if_fail( fps > 0.0f );
	
	self = G_STAR( figure );
	priv = g_star_get_instance_private( self );

	G_FIGURE_CLASS( g_star_parent_class )->randomize( figure, rnd, rect, fps );

	priv->corner = g_rand_int_range( rnd, G_STAR_MIN_CORNER, G_STAR_MAX_CORNER + 1 );
}

static GLRendererData
g_star_real_get_data(
	GFigure *figure )
{
	GStar *self;
	GStarPrivate *priv;
	guint k;
	gboolean filled;
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

	self = G_STAR( figure );
	priv = g_star_get_instance_private( self );

	data = G_FIGURE_CLASS( g_star_parent_class )->get_data( figure );

	g_object_get( G_OBJECT( self ),
		"filled", &filled,
		NULL );

	k = priv->corner - G_STAR_MIN_CORNER;
	data.mode = g_star_class_mode[filled];
	data.offset = g_star_class_offset[filled][k];
	data.count = g_star_class_count[filled][k];

	return data;
}

static GLRendererLayout*
g_star_class_create_layout_odd(
	gboolean filled,
	guint corner,
	gfloat angle )
{
	guint i, k, dk, dc;
	GLsizeiptr j;
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
	k = 0;
	dk = ( corner - 1 ) / 2;
	da = (gfloat)2.0 * G_PI / (gfloat)corner;
	for( i = 0; i < corner; ++i )
	{
		layout->vertex[j++] = sinf( angle + (gfloat)k * da );
		layout->vertex[j++] = cosf( angle + (gfloat)k * da );
		k += dk;
	}
	if( filled )
	{
		layout->vertex[j++] = sinf( angle + (gfloat)k * da );
		layout->vertex[j++] = cosf( angle + (gfloat)k * da );
	}

	/* indices */
	layout->index_num = layout->vertex_num;
	layout->index_size = sizeof( GLuint ) * layout->index_num;
	layout->index = (GLuint*)g_malloc( layout->index_size );

	for( j = 0; j < layout->index_num; ++j )
		layout->index[j] = j;

	return layout;
}

static GLRendererLayout*
g_star_class_create_layout_even(
	gboolean filled,
	guint corner,
	gfloat angle )
{
	guint i, sub_corner, layouts_count;
	GLfloat da;
	GLRendererLayout **layouts, *layout;

	/* a even stellated star is constructed by even number of odd stars */
	sub_corner = corner;
	layouts_count = 1;
	while( sub_corner % 2 == 0 && sub_corner > 4 )
	{
		sub_corner /= 2;
		layouts_count *= 2;
	}

	layouts = (GLRendererLayout**)g_malloc( sizeof( GLRendererLayout* ) * layouts_count );

	da = (gfloat)2.0 * G_PI / (gfloat)corner;
	for( i = 0; i < layouts_count - 1; ++i )
	{
		layouts[i] = g_star_class_create_layout_odd( filled, sub_corner, angle + (gfloat)i * da );

		layouts[i]->index_num++;
		layouts[i]->index_size += sizeof( GLuint );
		layouts[i]->index = (GLuint*)g_realloc( layouts[i]->index, layouts[i]->index_size );

		layouts[i]->index[layouts[i]->index_num-1] = GL_RENDERER_PRIMITIVE_RESTART_INDEX;
	}
	layouts[i] = g_star_class_create_layout_odd( filled, sub_corner, angle + (gfloat)i * da );

	layout = gl_renderer_layout_new_merged( layouts, layouts_count );

	for( i = 0; i < layouts_count; ++i )
		gl_renderer_layout_free( layouts[i] );

	return layout;
}

static GLRendererLayout*
g_star_class_create_layout(
	gboolean filled,
	guint corner )
{
	if( corner % 2 == 0 )
		return g_star_class_create_layout_even( filled, corner, 0.0f );

	return g_star_class_create_layout_odd( filled, corner, 0.0f );
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

GLRendererLayout*
g_star_class_get_layout(
	void )
{
	guint i, j;
	GLsizeiptr k, offset;
	GLRendererLayout *layouts[2*G_STAR_CORNER_NUM], *layout;

	g_star_class_mode[FALSE] = GL_LINE_LOOP;
	g_star_class_mode[TRUE] = GL_TRIANGLE_FAN;

	offset = 0;
	k = 0;
	for( i = 0; i < 2; ++i )
		for( j = 0; j < G_STAR_CORNER_NUM; ++j )
		{
			layouts[k] = g_star_class_create_layout( i, G_STAR_MIN_CORNER + j );
			g_star_class_offset[i][j] = offset;
			g_star_class_count[i][j] = layouts[k]->index_num;
			offset += layouts[k]->index_size;
			k++;
		}

	layout = gl_renderer_layout_new_merged( layouts, 2 * G_STAR_CORNER_NUM );
	for( k = 0; k < 2 * G_STAR_CORNER_NUM; ++k )
		gl_renderer_layout_free( layouts[k] );

	return layout;
}

