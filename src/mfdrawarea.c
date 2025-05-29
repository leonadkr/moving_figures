#include "mfdrawarea.h"

#include <gtk/gtk.h>
#include <graphene.h>
#include <graphene-gobject.h>
#include <epoxy/gl.h>

struct _MfDrawArea
{
	GtkWidget parent_instance;

	graphene_rect_t rect;
	GdkGLContext *context;

	gint min_width;
	gint min_height;
	guint msfbo, fbo;
};
typedef struct _MfDrawArea MfDrawArea;

struct _MfTexture
{
	guint mstex, tex;
	guint width, height, scale;
};
typedef struct _MfTexture MfTexture;

enum _MfDrawAreaPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_RECTANGLE,
	PROP_CONTEXT,

	N_PROPS
};
typedef enum _MfDrawAreaPropertyID MfDrawAreaPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

enum _MfDrawAreaSignalID
{
	SIGNAL_RENDER,
	SIGNAL_RESIZE,

	N_SIGNALS
};
typedef enum _MfDrawAreaSignalID MfDrawAreaSignalID;

static guint mf_draw_area_signals[N_SIGNALS] = { 0, };

G_DEFINE_FINAL_TYPE( MfDrawArea, mf_draw_area, GTK_TYPE_WIDGET )

static void
mf_draw_area_init(
	MfDrawArea *self )
{
	graphene_rect_init_from_rect( &self->rect, graphene_rect_zero() );
	self->context = NULL;

	self->min_width = 50;
	self->min_height = 50;
}

static void
mf_draw_area_get_property(
	GObject *object,
	guint prop_id,
	GValue *value,
	GParamSpec *pspec )
{
	MfDrawArea *self = MF_DRAW_AREA( object );

	switch( (MfDrawAreaPropertyID)prop_id )
	{
		case PROP_RECTANGLE:
			g_value_set_boxed( value, &self->rect );
			break;
		case PROP_CONTEXT:
			g_value_set_object( value, self->context );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_draw_area_size_allocate(
	GtkWidget *widget,
	gint width,
	gint height,
	gint baseline )
{
	MfDrawArea *self = MF_DRAW_AREA( widget );

	/* store new size */
	graphene_rect_init( &self->rect, 0.0f, 0.0f, (gfloat)width, (gfloat)height );

	/* update GL viewport */
	gdk_gl_context_make_current( self->context );
	glViewport( 0, 0, width, height );

	g_signal_emit( self, mf_draw_area_signals[SIGNAL_RESIZE], 0, width, height );
}

static GtkSizeRequestMode
mf_draw_area_get_request_mode(
	GtkWidget *widget )
{
	/* using minimal sizes makes this unnecessary */
	return GTK_SIZE_REQUEST_CONSTANT_SIZE;
}

static void
mf_draw_area_measure(
	GtkWidget *widget,
	GtkOrientation orientation,
	gint for_size,
	gint *minimum,
	gint *natural,
	gint *minimum_baseline, gint *natural_baseline )
{
	MfDrawArea *self = MF_DRAW_AREA( widget );

	if( orientation == GTK_ORIENTATION_HORIZONTAL )
		*minimum = *natural = self->min_width;
	else
		*minimum = *natural = self->min_height;
}

static MfTexture*
mf_texture_new(
	guint width,
	guint height,
	guint scale )
{
	MfTexture *self = g_new( MfTexture, 1 );
	gint max_samples;

	glGetIntegerv( GL_MAX_SAMPLES, &max_samples );

	self->width = width * scale;
	self->height = height * scale;
	self->scale = scale;

	glGenRenderbuffers( 1, &self->mstex );
	glBindRenderbuffer( GL_RENDERBUFFER, self->mstex );
	glRenderbufferStorageMultisample( GL_RENDERBUFFER, max_samples, GL_RGBA8, self->width, self->height );

	glGenTextures( 1, &self->tex );
	glBindTexture( GL_TEXTURE_2D, self->tex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, self->width, self->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	return self;
}

static void
mf_texture_free(
	MfTexture *self )
{
	if( self == NULL )
		return;

	glDeleteRenderbuffers( 1, &self->mstex );
	glDeleteTextures( 1, &self->tex );
	g_free( self );
}

static void
mf_draw_area_shapshot(
	GtkWidget *widget,
	GtkSnapshot *snapshot )
{
	MfDrawArea *self = MF_DRAW_AREA( widget );
	guint scale;
	MfTexture *texture;
	GdkGLTextureBuilder *builder;
	GdkTexture *gdk_texture;

	gdk_gl_context_make_current( self->context );

	/* create new textures */
	scale = gtk_widget_get_scale_factor( widget );
	texture = mf_texture_new( graphene_rect_get_width( &self->rect ), graphene_rect_get_height( &self->rect ), scale );

	/* attach the texture to the frame buffer */
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, self->msfbo );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, texture->mstex );
	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		mf_texture_free( texture );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
			"MESSAGE", "Frame buffer completeness status failed",
			NULL );
		return;
	}
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, self->fbo );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->tex, 0 );
	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
	{
		mf_texture_free( texture );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
			"MESSAGE", "Frame buffer completeness status failed",
			NULL );
		return;
	}

	/* actual drawing */
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, self->msfbo );
	g_signal_emit( self, mf_draw_area_signals[SIGNAL_RENDER], 0 );

	/* copy from the multi-sample texture to the draw texture */
	glBindFramebuffer( GL_READ_FRAMEBUFFER, self->msfbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, self->fbo );
	glBlitFramebuffer( 0, 0, texture->width, texture->height, 0, 0, texture->width, texture->height, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );

	/* build the GDK texture */
	builder = gdk_gl_texture_builder_new();
	gdk_gl_texture_builder_set_id( builder, texture->tex );
	gdk_gl_texture_builder_set_context( builder, self->context );
	gdk_gl_texture_builder_set_width( builder, texture->width );
	gdk_gl_texture_builder_set_height( builder, texture->height );
	gdk_gl_texture_builder_set_format( builder, GDK_MEMORY_R8G8B8A8 );
	gdk_texture = gdk_gl_texture_builder_build( builder, (GDestroyNotify)mf_texture_free, texture );
	g_object_unref( G_OBJECT( builder ) );

	/* add the GDK texture to the list for drawing */
	gtk_snapshot_save( snapshot );
	gtk_snapshot_translate( snapshot, &GRAPHENE_POINT_INIT( 0, graphene_rect_get_height( &self->rect ) ) );
	gtk_snapshot_scale( snapshot, 1.0f, -1.0f );
	gtk_snapshot_append_texture( snapshot, gdk_texture, &GRAPHENE_RECT_INIT( 0, 0, graphene_rect_get_width( &self->rect ), graphene_rect_get_height( &self->rect ) ) );
	gtk_snapshot_restore( snapshot );
	g_object_unref( G_OBJECT( gdk_texture ) );
}

static void
mf_draw_area_realize(
	GtkWidget *widget )
{
	MfDrawArea *self = MF_DRAW_AREA( widget );
	GtkNative *native;
	GdkSurface *surface;
	GError *error = NULL;

	GTK_WIDGET_CLASS( mf_draw_area_parent_class )->realize( widget );

	/* create GL context */
	native = gtk_widget_get_native( widget );
	surface = gtk_native_get_surface( native );
	self->context = gdk_surface_create_gl_context( surface, &error );
	if( error != NULL )
	{
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
			"MESSAGE", error->message,
			NULL );
		g_clear_error( &error );
		g_clear_object( &self->context );
		return;
	}

	/* realize GL context */
	gdk_gl_context_realize( self->context, &error );
	if( error != NULL )
	{
		g_log_structured( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
			"MESSAGE", error->message,
			NULL );
		g_clear_error( &error );
		g_clear_object( &self->context );
		return;
	}

	/* create frame buffers and bind to the GL context */
	gdk_gl_context_make_current( self->context );

	glGenFramebuffers( 1, &self->msfbo );
	glBindFramebuffer( GL_FRAMEBUFFER, self->msfbo );
	glGenFramebuffers( 1, &self->fbo );
	glBindFramebuffer( GL_FRAMEBUFFER, self->fbo );

	/* do not use depth test */
	glDisable( GL_DEPTH_TEST );

	/* enable MSAA */
	glEnable( GL_MULTISAMPLE );
}

static void
mf_draw_area_unrealize(
	GtkWidget *widget )
{
	MfDrawArea *self = MF_DRAW_AREA( widget );

	gdk_gl_context_make_current( self->context );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	glDeleteFramebuffers( 1, &self->msfbo );
	glDeleteFramebuffers( 1, &self->fbo );

	gdk_gl_context_clear_current();
	g_clear_object( &self->context );

	GTK_WIDGET_CLASS( mf_draw_area_parent_class )->unrealize( widget );
}

static void
mf_draw_area_class_init(
	MfDrawAreaClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS( klass );

	object_class->get_property = mf_draw_area_get_property;
	object_props[PROP_RECTANGLE] = g_param_spec_boxed(
		"rectangle",
		"Rectangle",
		"Reactangle area for drawing as graphene_rect_t",
		GRAPHENE_TYPE_RECT,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS );
	object_props[PROP_CONTEXT] = g_param_spec_object(
		"context",
		"Context",
		"GdkGLContext for the widget",
		GDK_TYPE_GL_CONTEXT,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS );
	g_object_class_install_properties( object_class, N_PROPS, object_props );

	mf_draw_area_signals[SIGNAL_RENDER] = g_signal_new(
		"render",
		G_TYPE_FROM_CLASS( klass ),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0 );
	mf_draw_area_signals[SIGNAL_RESIZE] = g_signal_new(
		"resize",
		G_TYPE_FROM_CLASS( klass ),
		G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		G_TYPE_INT,
		G_TYPE_INT );

	widget_class->size_allocate = mf_draw_area_size_allocate;
	widget_class->get_request_mode = mf_draw_area_get_request_mode;
	widget_class->measure = mf_draw_area_measure;
	widget_class->snapshot = mf_draw_area_shapshot;
	widget_class->realize = mf_draw_area_realize;
	widget_class->unrealize = mf_draw_area_unrealize;
}

MfDrawArea*
mf_draw_area_new(
	void )
{
	return MF_DRAW_AREA( g_object_new( MF_TYPE_DRAW_AREA, NULL ) );
}

void
mf_draw_area_get_rectangle(
	MfDrawArea *self,
	graphene_rect_t *rect )
{
	g_return_if_fail( MF_IS_DRAW_AREA( self ) );
	g_return_if_fail( rect != NULL );

	graphene_rect_init_from_rect( rect, &self->rect );
}

GdkGLContext*
mf_draw_area_get_context(
	MfDrawArea *self )
{
	g_return_val_if_fail( MF_IS_DRAW_AREA( self ), NULL );

	return GDK_GL_CONTEXT( g_object_ref( G_OBJECT( self->context ) ) );
}

