#include <stdlib.h>
#include <math.h>
#include <gtk/gtk.h>
#include <epoxy/gl.h>

#include "gresource.h"
#include "glrenderer.h"
#include "gtkmovingfiguresarea.h"

#include "gpoint.h"
#include "gcircle.h"
#include "gpolygon.h"
#include "gstar.h"


/*
	structures
*/

struct _GtkMovingFiguresArea
{
	GtkWidget parent_instance;

	guint minimum_width, minimum_height;
	gfloat fps;

	gboolean is_realized;
	GList *fig[N_GTK_MOVING_FIGURE_TYPE];
	guint fignum[N_GTK_MOVING_FIGURE_TYPE];
	GLsizeiptr offset[N_GTK_MOVING_FIGURE_TYPE];
	GLsizeiptr count[N_GTK_MOVING_FIGURE_TYPE];
	GLRectangle rect;

	GdkGLContext *gl_context;
	GLRenderer *renderer;
	GLuint frame_buffer;
};
typedef struct _GtkMovingFiguresArea GtkMovingFiguresArea;

/*
	properties
*/
enum _GtkMovingFiguresAreaPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_MINIMUM_WIDTH,
	PROP_MINIMUM_HEIGHT,
	PROP_FPS,

	N_PROPS
};
typedef enum _GtkMovingFiguresAreaPropertyID GtkMovingFiguresAreaPropertyID;

/*
	static members
*/
static GParamSpec *object_props[N_PROPS] = { NULL, };

/*
	private methods
*/
static void gtk_moving_figures_area_real_size_allocate( GtkWidget *widget, gint width, gint height, gint baseline );
static GtkSizeRequestMode gtk_moving_figures_area_real_get_request_mode( GtkWidget *widget );
static void gtk_moving_figures_area_real_measure( GtkWidget *widget, GtkOrientation orientation, gint for_size, gint *minimum, gint *natural, gint *minimum_baseline, gint *natural_baseline );
static void gtk_moving_figures_area_real_shapshot( GtkWidget *widget, GtkSnapshot *snapshot );
static void gtk_moving_figures_area_real_realize( GtkWidget *widget );
static void gtk_moving_figures_area_real_unrealize( GtkWidget *widget );

G_DEFINE_FINAL_TYPE( GtkMovingFiguresArea, gtk_moving_figures_area, GTK_TYPE_WIDGET )

static void
gtk_moving_figures_area_init(
	GtkMovingFiguresArea *self )
{
	const GValue *value;
	guint fig_type;

	value = g_param_spec_get_default_value( object_props[PROP_MINIMUM_WIDTH] );
	self->minimum_width = g_value_get_uint( value );

	value = g_param_spec_get_default_value( object_props[PROP_MINIMUM_HEIGHT] );
	self->minimum_height = g_value_get_uint( value );

	value = g_param_spec_get_default_value( object_props[PROP_FPS] );
	self->fps = g_value_get_float( value );

	self->is_realized = FALSE;

	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
	{
		self->fig[fig_type] = NULL;
		self->fignum[fig_type] = 0;
	}

	self->rect = (GLRectangle){
		.x = 0.0f,
		.y = 0.0f,
		.width = (gfloat)self->minimum_width,
		.height = (gfloat)self->minimum_height };

	self->gl_context = NULL;
	self->renderer = NULL;
	self->frame_buffer = 0;
}

static void
gtk_moving_figures_area_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	GtkMovingFiguresArea *self = GTK_MOVING_FIGURES_AREA( object );

	switch( (GtkMovingFiguresAreaPropertyID)prop_id )
	{
		case PROP_MINIMUM_WIDTH:
			g_value_set_uint( value, self->minimum_width );
			break;
		case PROP_MINIMUM_HEIGHT:
			g_value_set_uint( value, self->minimum_height );
			break;
		case PROP_FPS:
			g_value_set_float( value, self->fps );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
gtk_moving_figures_area_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	GtkMovingFiguresArea *self = GTK_MOVING_FIGURES_AREA( object );

	switch( (GtkMovingFiguresAreaPropertyID)prop_id )
	{
		case PROP_MINIMUM_WIDTH:
			self->minimum_width = g_value_get_uint( value );
			break;
		case PROP_MINIMUM_HEIGHT:
			self->minimum_height = g_value_get_uint( value );
			break;
		case PROP_FPS:
			self->fps = g_value_get_float( value );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
gtk_moving_figures_area_class_init(
	GtkMovingFiguresAreaClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS( klass );

	object_class->get_property = gtk_moving_figures_area_get_property;
	object_class->set_property = gtk_moving_figures_area_set_property;
	object_props[PROP_MINIMUM_WIDTH] = g_param_spec_uint(
		"minimum-width",
		"Minimum width",
		"Set minimum width of the canvas",
		1,
		G_MAXUINT,
		1,
		G_PARAM_READWRITE );
	object_props[PROP_MINIMUM_HEIGHT] = g_param_spec_uint(
		"minimum-height",
		"Minimum height",
		"Set minimum height of the canvas",
		1,
		G_MAXUINT,
		1,
		G_PARAM_READWRITE );
	object_props[PROP_FPS] = g_param_spec_float(
		"fps",
		"Frame per second",
		"Frame per second",
		1.0,
		G_MAXFLOAT,
		1.0,
		G_PARAM_READWRITE );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	widget_class->size_allocate = gtk_moving_figures_area_real_size_allocate;
	widget_class->get_request_mode = gtk_moving_figures_area_real_get_request_mode;
	widget_class->measure = gtk_moving_figures_area_real_measure;
	widget_class->snapshot = gtk_moving_figures_area_real_shapshot;
	widget_class->realize = gtk_moving_figures_area_real_realize;
	widget_class->unrealize = gtk_moving_figures_area_real_unrealize;
}

/*
	private methods
*/
static void
gtk_moving_figures_area_real_size_allocate(
	GtkWidget *widget,
	gint width,
	gint height,
	gint baseline )
{
	GtkMovingFiguresArea *self;
	GLfloat vertex_transform_matrix[16] = {
		1.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	g_return_if_fail( GTK_IS_WIDGET( widget ) );

	self = GTK_MOVING_FIGURES_AREA( widget );

	gdk_gl_context_make_current( self->gl_context );

	/* store new size */
	self->rect.width = (gfloat)width;
	self->rect.height = (gfloat)height;

	/* update viewport */
	glViewport( 0, 0, width, height );

	/* update vertex transform matrix */
	vertex_transform_matrix[0] = 2.0f / (GLfloat)width;
	vertex_transform_matrix[5] = -2.0f / (GLfloat)height;
	glUniformMatrix4fv( self->renderer->vtm_uniform, 1, GL_TRUE, vertex_transform_matrix );
}

static GtkSizeRequestMode
gtk_moving_figures_area_real_get_request_mode(
	GtkWidget *widget )
{
	/* using minimal sizes makes this unnecessary */
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
gtk_moving_figures_area_real_measure(
	GtkWidget *widget,
	GtkOrientation orientation,
	gint for_size,
	gint *minimum,
	gint *natural,
	gint *minimum_baseline, gint *natural_baseline )
{
	GtkMovingFiguresArea *self;

	g_return_if_fail( GTK_WIDGET( widget ) );

	self = GTK_MOVING_FIGURES_AREA( widget );

	if( orientation == GTK_ORIENTATION_HORIZONTAL )
		*minimum = *natural = self->minimum_width;
	else
		*minimum = *natural = self->minimum_height;
}

static void
gtk_moving_figures_area_real_shapshot(
	GtkWidget *widget,
	GtkSnapshot *snapshot )
{
	GList *l;
	guint fig_type, scale;
	GLRendererData fig_data;
	GLTexture *texture;
	GdkGLTexture *gl_texture;
	GtkMovingFiguresArea *self;
	GLint frame_buffer_status;

	g_return_if_fail( GTK_WIDGET( widget ) );

	self = GTK_MOVING_FIGURES_AREA( widget );

	/* make all figures initially placed in the realized rectangle */
	if( !self->is_realized )
	{
		self->is_realized = TRUE;
		gtk_moving_figures_area_reallocate( self );
	}

	gdk_gl_context_make_current( self->gl_context );
	glBindFramebuffer( GL_FRAMEBUFFER, self->frame_buffer );

	/* make new texture and attach to frame buffer */
	scale = gtk_widget_get_scale_factor( widget );
	texture = gl_texture_new( self->rect.width, self->rect.height, scale );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id, 0 );
	frame_buffer_status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if( frame_buffer_status != GL_FRAMEBUFFER_COMPLETE )
	{
		g_warning( "Frame buffer completeness status failed\n" );
		return;
	}

	/* make white canvas */
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	/* draw all figures */
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = self->fig[fig_type]; l != NULL; l = l->next )
		{
			fig_data = g_figure_get_data( G_FIGURE( l->data ) );
			glUniform4fv( self->renderer->color_uniform, 1, fig_data.color );
			glUniformMatrix4fv( self->renderer->srtm_uniform, 1, GL_TRUE, fig_data.srtm );
			glDrawElements(
				fig_data.mode,
				fig_data.count,
				GL_UNSIGNED_INT,
				BOFFSET( self->offset[fig_type] + fig_data.offset ) );
		}

	/* set complete texture to the the stack for drawing */
	gl_texture = GDK_GL_TEXTURE( gdk_gl_texture_new(
		self->gl_context,
		texture->id,
		texture->width,
		texture->height,
		(GDestroyNotify)gl_texture_free,
		texture ) );
	gtk_snapshot_save( snapshot );
	gtk_snapshot_translate( snapshot, &GRAPHENE_POINT_INIT( 0, self->rect.height ) );
	gtk_snapshot_scale( snapshot, 1.0f, -1.0f );
	gtk_snapshot_append_texture(
		snapshot,
		GDK_TEXTURE( gl_texture ),
		&GRAPHENE_RECT_INIT( 0, 0, self->rect.width, self->rect.height ) );
	gtk_snapshot_restore( snapshot );
	g_object_unref( G_OBJECT( gl_texture ) );
}

static void
gtk_moving_figures_area_real_realize(
	GtkWidget *widget )
{
	GtkMovingFiguresArea *self;
	GtkNative *native;
	GdkSurface *surface;
	guint fig_type;
	GLsizeiptr offset;
	GBytes *vshader_source, *fshader_source;
	GLRendererLayout *layouts[N_GTK_MOVING_FIGURE_TYPE], *layout;
	GError *error = NULL;

	GTK_WIDGET_CLASS( gtk_moving_figures_area_parent_class )->realize( widget );

	self = GTK_MOVING_FIGURES_AREA( widget );

	/* get GL context */
	native = gtk_widget_get_native( widget );
	surface = gtk_native_get_surface( native );
	self->gl_context = gdk_surface_create_gl_context( surface, &error );
	if( error != NULL )
	{
		g_warning( "%s\n", error->message );
		g_clear_error( &error );
		g_clear_object( &( self->gl_context ) );
		return;
	}

	/* realize GL context */
	gdk_gl_context_realize( self->gl_context, &error );
	if( error != NULL )
	{
		g_warning( "%s\n", error->message );
		g_clear_error( &error );
		g_clear_object( &( self->gl_context ) );
		return;
	}
	gdk_gl_context_make_current( self->gl_context );

	/* get shaders' sources */
  vshader_source = g_resource_lookup_data(
		gresource_get_resource(),
		"/gresource/shaders/vertex.glsl",
		G_RESOURCE_LOOKUP_FLAGS_NONE,
		&error );
	if( error != NULL )
	{
		g_warning( "%s\n", error->message );
		g_clear_error( &error );
		return;
	}

  fshader_source = g_resource_lookup_data(
		gresource_get_resource(),
		"/gresource/shaders/fragment.glsl",
		G_RESOURCE_LOOKUP_FLAGS_NONE,
		&error );
	if( error != NULL )
	{
		g_warning( "%s\n", error->message );
		g_clear_error( &error );
		g_bytes_unref( vshader_source );
		return;
	}

	/* get layouts from GFigures' classes and combine them to the one layout */
	layouts[GTK_MOVING_FIGURE_TYPE_POINT] = g_point_class_get_layout();
	layouts[GTK_MOVING_FIGURE_TYPE_CIRCLE] = g_circle_class_get_layout();
	layouts[GTK_MOVING_FIGURE_TYPE_POLYGON] = g_polygon_class_get_layout();
	layouts[GTK_MOVING_FIGURE_TYPE_STAR] = g_star_class_get_layout();

	offset = 0;
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
	{
		self->offset[fig_type] = offset;
		self->count[fig_type] = layouts[fig_type]->index_num;
		offset += layouts[fig_type]->index_size;
	}

	layout = gl_renderer_layout_new_merged( layouts, N_GTK_MOVING_FIGURE_TYPE );
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		gl_renderer_layout_free( layouts[fig_type] );

	/* compile shaders, link to program, create vao, vbo, ibo; include all to GLRenderer */
	self->renderer = gl_renderer_new(
		g_bytes_get_data( vshader_source, NULL ),
		g_bytes_get_data( fshader_source, NULL ),
		layout,
		&error );
	g_bytes_unref( vshader_source );
	g_bytes_unref( fshader_source );
	gl_renderer_layout_free( layout );
	if( error != NULL )
	{
		g_warning( "%s\n", error->message );
		g_clear_error( &error );
		return;
	}

	/* create frame buffer */
	glGenFramebuffers( 1, &( self->frame_buffer ) );
	glBindFramebuffer( GL_FRAMEBUFFER, self->frame_buffer );
}

static void
gtk_moving_figures_area_real_unrealize(
	GtkWidget *widget )
{
	GtkMovingFiguresArea *self;
	guint fig_type;

	g_return_if_fail( GTK_IS_WIDGET( widget ) );

	self = GTK_MOVING_FIGURES_AREA( widget );

	gdk_gl_context_make_current( self->gl_context );

	/* free all resources related to GL */
	gl_renderer_free( self->renderer );
	glDeleteFramebuffers( 1, &( self->frame_buffer ) );
	gdk_gl_context_clear_current();
	g_clear_object( &( self->gl_context ) );

	/* destroy all GFigure's objects */
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		g_list_free_full( self->fig[fig_type], (GDestroyNotify)g_object_unref );

	GTK_WIDGET_CLASS( gtk_moving_figures_area_parent_class )->unrealize( widget );
}

/*
	public
*/
GtkMovingFiguresArea*
gtk_moving_figures_area_new(
	guint width,
	guint height,
	gfloat fps )
{
	return GTK_MOVING_FIGURES_AREA( g_object_new( GTK_TYPE_MOVING_FIGURES_AREA,
		"minimum-width", width,
		"minimum-height", height,
		"fps", fps,
		NULL ) );
}

void
gtk_moving_figures_area_reallocate(
	GtkMovingFiguresArea *self )
{
	guint fig_type;
	GList *l;
	GRand *rnd;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );

	rnd = g_rand_new();
	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = self->fig[fig_type]; l != NULL; l = l->next )
			g_figure_randomize( G_FIGURE( l->data ), rnd, &( self->rect ), self->fps );
	g_rand_free( rnd );
}

void
gtk_moving_figures_area_move(
	GtkMovingFiguresArea *self )
{
	guint fig_type;
	GList *l;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );

	for( fig_type = 0; fig_type < N_GTK_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = self->fig[fig_type]; l != NULL; l = l->next )
			g_figure_move( G_FIGURE( l->data ), &( self->rect ) );
}

void
gtk_moving_figures_area_append(
	GtkMovingFiguresArea *self,
	GtkMovingFigureType fig_type,
	gint num )
{
	guint i;
	GFigure *figure;
	GRand *rnd;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );
	g_return_if_fail( num >= 0 );

	rnd = g_rand_new();
	for( i = 0; i < num; ++i )
	{
		switch( fig_type )
		{
			case GTK_MOVING_FIGURE_TYPE_POINT:
				figure = G_FIGURE( g_point_new() );
				break;
			case GTK_MOVING_FIGURE_TYPE_CIRCLE:
				figure = G_FIGURE( g_circle_new() );
				break;
			case GTK_MOVING_FIGURE_TYPE_POLYGON:
				figure = G_FIGURE( g_polygon_new() );
				break;
			case GTK_MOVING_FIGURE_TYPE_STAR:
				figure = G_FIGURE( g_star_new() );
				break;
			default:
				g_return_if_fail( fig_type < N_GTK_MOVING_FIGURE_TYPE );
				figure = NULL;
				break;
		}
		g_figure_randomize( figure, rnd, &( self->rect ), self->fps );
		self->fig[fig_type] = g_list_prepend( self->fig[fig_type], figure );
	}
	self->fignum[fig_type] += num;
	g_rand_free( rnd );
}

void
gtk_moving_figures_area_remove(
	GtkMovingFiguresArea *self,
	GtkMovingFigureType fig_type,
	gint num )
{
	guint i;
	GList *l;

	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );
	g_return_if_fail( num >= 0 );

	for( i = 0, l = self->fig[fig_type]; i < num && l != NULL; ++i, l = l->next )
		g_object_unref( G_OBJECT( l->data ) );
	self->fig[fig_type] = l;

	if( num > self->fignum[fig_type] )
		self->fignum[fig_type] = 0;
	else
		self->fignum[fig_type] -= num;
}

void
gtk_moving_figures_area_set_number(
	GtkMovingFiguresArea *self,
	GtkMovingFigureType fig_type,
	gint num )
{
	g_return_if_fail( GTK_IS_MOVING_FIGURES_AREA( self ) );
	g_return_if_fail( num >= 0 );

	if( num > self->fignum[fig_type] )
		gtk_moving_figures_area_append( self, fig_type, num - self->fignum[fig_type] );
	else
		gtk_moving_figures_area_remove( self, fig_type, self->fignum[fig_type] - num );
}

