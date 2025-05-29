#include "mfrender.h"

#include "mffigure.h"

#include <glib-object.h>
#include <glib.h>
#include <graphene.h>
#include <epoxy/gl.h>

struct _MfRender
{
	GObject parent_instance;

	guint program;
	guint vao, vbo, ebo;

	guint position_index;
	guint color_uniform;
	guint vtm_uniform;
	guint srtm_uniform;

	gboolean is_completed;
};
typedef struct _MfRender MfRender;

G_DEFINE_ENUM_TYPE( MfRenderError, mf_render_error,
	G_DEFINE_ENUM_VALUE( MF_RENDER_ERROR_SHADER_COMPILATION, "shader-compilation" ),
	G_DEFINE_ENUM_VALUE( MF_RENDER_ERROR_PROGRAM_LINKING, "program-linking" ),
	G_DEFINE_ENUM_VALUE( MF_RENDER_ERROR_ALREADY_COMPLETED, "already-completed" ) )

G_DEFINE_QUARK( mf-render-error, mf_render_error )

G_DEFINE_TYPE( MfRender, mf_render, G_TYPE_OBJECT )

static void
mf_render_init(
	MfRender *self )
{
	self->program = 0;
	self->vao = 0;
	self->vbo = 0;
	self->ebo = 0;

	self->position_index = 0;
	self->color_uniform = 0;
	self->vtm_uniform = 0;
	self->srtm_uniform = 0;

	self->is_completed = FALSE;
}

static void
mf_render_finalize(
	GObject *object )
{
	MfRender *self = MF_RENDER( object );

	glDeleteProgram( self->program );
	glDeleteVertexArrays( 1, &self->vao );
	glDeleteBuffers( 1, &self->vbo );
	glDeleteBuffers( 1, &self->ebo );

	G_OBJECT_CLASS( mf_render_parent_class )->finalize( object );
}

static void
mf_render_class_init(
	MfRenderClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );

	object_class->finalize = mf_render_finalize;
}

static guint
mf_render_create_shader(
	gint shader_type,
	const char *shader_code,
	GError **error )
{
	guint shader;
	gint params;
	gchar *info;
	gint info_len;

	shader = glCreateShader( shader_type );
	glShaderSource( shader, 1, &shader_code, NULL );
	glCompileShader( shader );

	glGetShaderiv( shader, GL_COMPILE_STATUS, &params );
	if( params == GL_FALSE )
	{
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &info_len);
		info = g_new( char, info_len + 1 );
		glGetShaderInfoLog( shader, info_len, NULL, info );

		g_set_error(
			error,
			MF_RENDER_ERROR,
			MF_RENDER_ERROR_SHADER_COMPILATION,
			"Shader compilation fails: %s",
			info );

		g_free( info );
		glDeleteShader (shader);
		return 0;
	}

	return shader;
}

static guint
mf_render_create_program(
	guint vshader,
	guint fshader,
	GError **error )
{
	guint program;
	gint link_status;
	gchar *info;
	gint info_len;

	program = glCreateProgram();
	glAttachShader( program, fshader );
	glAttachShader( program, vshader );
	glLinkProgram( program );

	glGetProgramiv( program, GL_LINK_STATUS, &link_status );
	if( link_status == GL_FALSE )
	{
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &info_len );
		info = g_new( char, info_len + 1 );
		glGetProgramInfoLog( program, info_len, NULL, info );

		g_set_error(
			error,
			MF_RENDER_ERROR,
			MF_RENDER_ERROR_PROGRAM_LINKING,
			"Program linking fails: %s",
			info );

		g_free( info );
		glDetachShader( program, vshader );
		glDetachShader( program, fshader );
		glDeleteProgram( program );
		return 0;
	}

	glDetachShader( program, vshader );
	glDetachShader( program, fshader );

	return program;
}

MfRender*
mf_render_new(
	void )
{
	return MF_RENDER( g_object_new( MF_TYPE_RENDER, NULL ) );
}

gboolean
mf_render_complete(
	MfRender *self,
	GBytes *vshader_bytes,
	GBytes *fshader_bytes,
	GBytes *vertices_bytes,
	GBytes *indices_bytes,
	GError **error )
{
	guint vshader, fshader, program;
	gboolean ret = FALSE;
	GError *loc_error = NULL;

	g_return_val_if_fail( MF_IS_RENDER( self ), FALSE );
	g_return_val_if_fail( vshader_bytes != NULL, FALSE );
	g_return_val_if_fail( fshader_bytes != NULL, FALSE );
	g_return_val_if_fail( vertices_bytes != NULL, FALSE );
	g_return_val_if_fail( indices_bytes != NULL, FALSE );

	g_bytes_ref( vshader_bytes );
	g_bytes_ref( fshader_bytes );
	g_bytes_ref( vertices_bytes );
	g_bytes_ref( indices_bytes );

	if( self->is_completed )
	{
		g_set_error(
			error,
			MF_RENDER_ERROR,
			MF_RENDER_ERROR_ALREADY_COMPLETED,
			"Renderer is already completed." );
		goto out;
	}

	vshader = mf_render_create_shader( GL_VERTEX_SHADER, g_bytes_get_data( vshader_bytes, NULL ), &loc_error );
	if( loc_error != NULL )
	{
		g_propagate_error( error, loc_error );
		goto out;
	}
		
	fshader = mf_render_create_shader( GL_FRAGMENT_SHADER, g_bytes_get_data( fshader_bytes, NULL ), &loc_error );
	if( loc_error != NULL )
	{
		glDeleteShader( vshader );
		g_propagate_error( error, loc_error );
		goto out;
	}

	program = mf_render_create_program( vshader, fshader, &loc_error );
	if( loc_error != NULL )
	{
		glDeleteShader( vshader );
		glDeleteShader( fshader );
		g_propagate_error( error, loc_error );
		goto out;
	}
	glDeleteShader( vshader );
	glDeleteShader( fshader );

	glUseProgram( program );
	self->program = program;
	self->position_index = glGetAttribLocation( program, "position" );
	self->color_uniform = glGetUniformLocation( program, "color" );
	self->vtm_uniform = glGetUniformLocation( program, "vtm" );
	self->srtm_uniform = glGetUniformLocation( program, "srtm" );

	/* create buffers */
	glGenVertexArrays( 1, &self->vao );
	glGenBuffers( 1, &self->vbo );
	glGenBuffers( 1, &self->ebo );

	glBindVertexArray( self->vao );

	/* VBO */
	glBindBuffer( GL_ARRAY_BUFFER, self->vbo );
	glBufferData( GL_ARRAY_BUFFER, g_bytes_get_size( vertices_bytes ), g_bytes_get_data( vertices_bytes, NULL ), GL_STATIC_DRAW );

	/* EBO */
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->ebo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, g_bytes_get_size( indices_bytes ), g_bytes_get_data( indices_bytes, NULL ), GL_STATIC_DRAW );

	/* bind attributes */
	glEnableVertexAttribArray( self->position_index );
	glVertexAttribPointer( self->position_index, 2, GL_FLOAT, GL_FALSE, 0, (void*)0L );

	/* enable restart index */
	glEnable( GL_PRIMITIVE_RESTART_FIXED_INDEX );

	/* set off drawing backward surface */
	glEnable( GL_CULL_FACE );
	glCullFace( GL_FRONT );
	glFrontFace( GL_CW );

	glBindVertexArray( 0 );

	self->is_completed = TRUE;
	ret = TRUE;

out:
	g_bytes_unref( vshader_bytes );
	g_bytes_unref( fshader_bytes );
	g_bytes_unref( vertices_bytes );
	g_bytes_unref( indices_bytes );

	return ret;
}

gboolean
mf_render_is_completed(
	MfRender *self )
{
	g_return_val_if_fail( MF_IS_RENDER( self ), FALSE );

	return self->is_completed;
}

void
mf_render_set_size(
	MfRender *self,
	gfloat width,
	gfloat height )
{
	graphene_matrix_t vtm;
	gfloat v[16];

	g_return_if_fail( MF_IS_RENDER( self ) );
	g_return_if_fail( width >= 0.0 );
	g_return_if_fail( height >= 0.0 );

	if( !self->is_completed )
		return;

	graphene_matrix_init_scale( &vtm, 2.0f / width, -2.0f / height, 1.0f );
	graphene_matrix_translate( &vtm, &GRAPHENE_POINT3D_INIT( -1.0f, 1.0f, 0.0f ) );
	graphene_matrix_to_float( &vtm, v );

	glUniformMatrix4fv( self->vtm_uniform, 1, GL_TRUE, v );
}

void
mf_render_draw(
	MfRender *self,
	MfFigure *fig )
{
	MfLayout *layout;
	graphene_vec4_t color;
	graphene_matrix_t srtm;
	gfloat c[4], s[16];

	g_return_if_fail( MF_IS_RENDER( self ) );
	g_return_if_fail( MF_IS_FIGURE( fig ) );

	if( !self->is_completed )
		return;

	g_object_ref( G_OBJECT( fig ) );
	layout = mf_figure_get_layout( fig );

	mf_figure_get_color( fig, &color );
	graphene_vec4_to_float( &color, c );
	mf_figure_get_srtm( fig, &srtm );
	graphene_matrix_to_float( &srtm, s );

	glBindVertexArray( self->vao );
	glUniform4fv( self->color_uniform, 1, c );
	glUniformMatrix4fv( self->srtm_uniform, 1, GL_TRUE, s );

	glDrawElements( mf_layout_get_mode( layout ), mf_layout_get_count( layout ), GL_UNSIGNED_INT, (void*)mf_layout_get_offset( layout ) );

	g_object_unref( G_OBJECT( layout ) );
	g_object_unref( G_OBJECT( fig ) );
}

