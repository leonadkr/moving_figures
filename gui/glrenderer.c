#include <string.h>
#include <math.h>
#include "glrenderer.h"

#define GL_RENDERER_MSAA 4

#define GL_RENDERER_ERROR ( gl_renderer_error_quark() )
static G_DEFINE_QUARK( gl-renderer-error-quark, gl_renderer_error )

enum _GLRendererError
{
	GL_RENDERER_ERROR_SHADER_COMPILATION,
	GL_RENDERER_ERROR_PROGRAM_LINKING,
	GL_RENDERER_ERROR_FRAME_BUFFER_COMPLETENESS_STATUS_FAILED,

	N_GL_RENDERER_ERROR
};
typedef enum _GLRendererError GLRendererError;

static GLuint
gl_renderer_create_shader(
	GLenum shader_type,
	const char *shader_code,
	GError **error )
{
	GLuint shader;
	GLint params;
	char *shader_info;
	GLsizei shader_info_len;

	shader = glCreateShader( shader_type );
	glShaderSource( shader, 1, &shader_code, NULL );
	glCompileShader( shader );

	glGetShaderiv( shader, GL_COMPILE_STATUS, &params );
	if( params == GL_FALSE )
	{
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &shader_info_len);
		shader_info = g_new( char, shader_info_len + 1 );
		glGetShaderInfoLog( shader, shader_info_len, NULL, shader_info );

		g_set_error(
			error,
			GL_RENDERER_ERROR,
			GL_RENDERER_ERROR_SHADER_COMPILATION,
			"Shader compilation error: %s",
			shader_info );

		g_free( shader_info );
		glDeleteShader (shader);
		return 0;
	}

	return shader;
}

static GLuint
gl_renderer_create_program(
	GLuint vshader,
	GLuint fshader,
	GError **error )
{
	GLuint program;
	GLint link_status;
	char *program_info;
	GLsizei program_info_len;

	program = glCreateProgram();
	glAttachShader( program, fshader );
	glAttachShader( program, vshader );
	glLinkProgram( program );

	glGetProgramiv( program, GL_LINK_STATUS, &link_status );
	if( link_status == GL_FALSE )
	{
      glGetProgramiv( program, GL_INFO_LOG_LENGTH, &program_info_len );
      program_info = g_new( char, program_info_len + 1 );
      glGetProgramInfoLog( program, program_info_len, NULL, program_info );

		g_set_error(
			error,
			GL_RENDERER_ERROR,
			GL_RENDERER_ERROR_PROGRAM_LINKING,
			"Program linking fails: %s",
			program_info );

		glDetachShader( program, vshader );
		glDetachShader( program, fshader );
		glDeleteShader( vshader );
		glDeleteShader( fshader );
		glDeleteProgram( program );
		g_free( program_info );
		return 0;
	}

	glDetachShader( program, vshader );
	glDetachShader( program, fshader );
	glDeleteShader( vshader );
	glDeleteShader( fshader );

	return program;
}

static gboolean
gl_renderer_vertex_cmp(
	char *iv,
	char *ov,
	GLsizeiptr vs )
{
	const GLfloat epsilon = 1.0e-6;
	GLfloat *ix, *iy;
	GLfloat *ox, *oy;

	if( memcmp( iv, ov, vs ) == 0 )
		return TRUE;

	ix = (GLfloat*)iv;
	iy = (GLfloat*)POFFSET( iv, sizeof( GLfloat ) );
	ox = (GLfloat*)ov;
	oy = (GLfloat*)POFFSET( ov, sizeof( GLfloat ) );

	if( sqrtf( ( *ix - *ox ) * ( *ix - *ox ) + ( *iy - *oy ) * ( *iy - *oy ) ) < epsilon )
		return TRUE;

	return FALSE;
}

GLRenderer*
gl_renderer_new(
	const char *vshader_code,
	const char *fshader_code,
	GLRendererLayout *layout,
	GError **error )
{
	GLRenderer *renderer;
	GLuint program;
	GLuint vshader, fshader;
	GError *shader_error = NULL;
	GError *program_error = NULL;

	g_return_val_if_fail( vshader_code != NULL, NULL );
	g_return_val_if_fail( fshader_code != NULL, NULL );
	g_return_val_if_fail( layout != NULL, NULL );

	vshader = gl_renderer_create_shader(
		GL_VERTEX_SHADER,
		vshader_code,
		&shader_error );
	if( shader_error != NULL )
	{
		g_propagate_error( error, shader_error );
		return NULL;
	}
		
	fshader = gl_renderer_create_shader(
		GL_FRAGMENT_SHADER,
		fshader_code,
		&shader_error );
	if( shader_error != NULL )
	{
		g_propagate_error( error, shader_error );
		return NULL;
	}

	program = gl_renderer_create_program(
		vshader,
		fshader,
		&program_error );
	if( program_error != NULL )
	{
		g_propagate_error( error, program_error );
		return NULL;
	}

	renderer = g_new( GLRenderer, 1 );

	/* program */
	glUseProgram( program );
	renderer->program = program;
	renderer->position_index = glGetAttribLocation( program, "position" );
	renderer->color_uniform = glGetUniformLocation( program, "color" );
	renderer->vtm_uniform = glGetUniformLocation( program, "vtm" );
	renderer->srtm_uniform = glGetUniformLocation( program, "srtm" );

	/* VBO */
	glGenBuffers( 1, &( renderer->vbo ) );
	glBindBuffer( GL_ARRAY_BUFFER, renderer->vbo );
	glBufferData( GL_ARRAY_BUFFER, layout->vertex_size, layout->vertex, GL_STATIC_DRAW );

	/* VAO */
	glGenVertexArrays( 1, &( renderer->vao ) );
	glBindVertexArray( renderer->vao );
	glEnableVertexAttribArray( renderer->position_index );
	glVertexAttribPointer( renderer->position_index, 2, GL_FLOAT, GL_FALSE, 0, NULL );

	/* IBO */
	glGenBuffers( 1, &( renderer->ibo ) );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, renderer->ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, layout->index_size, layout->index, GL_STATIC_DRAW );

	/* frame buffers */
	glGenFramebuffers( 1, &( renderer->msfbo ) );
	glBindFramebuffer( GL_FRAMEBUFFER, renderer->msfbo );
	glGenFramebuffers( 1, &( renderer->fbo ) );
	glBindFramebuffer( GL_FRAMEBUFFER, renderer->fbo );

	/* use primitive restart index as GL_RENDERER_PRIMITIVE_RESTART_INDEX */
	glEnable( GL_PRIMITIVE_RESTART );
	glPrimitiveRestartIndex( GL_RENDERER_PRIMITIVE_RESTART_INDEX );

	/* set off drawing backward surface */
	glEnable( GL_CULL_FACE );
	glCullFace( GL_FRONT );
	glFrontFace( GL_CW );

	/* do not use depth test */
	glDisable( GL_DEPTH_TEST );

	/* enable MSAA */
	glEnable( GL_MULTISAMPLE );

	return renderer;
}

void
gl_renderer_free(
	GLRenderer *self )
{
	if( self == NULL )
		return;

	glDeleteProgram( self->program );
	glDeleteVertexArrays( 1, &( self->vao ) );
	glDeleteBuffers( 1, &( self->vbo ) );
	glDeleteBuffers( 1, &( self->ibo ) );
	glDeleteFramebuffers( 1, &( self->fbo ) );
	glDeleteFramebuffers( 1, &( self->msfbo ) );

	g_free( self );
}

void
gl_renderer_make_current(
	GLRenderer *self )
{
	glBindFramebuffer( GL_FRAMEBUFFER, self->fbo );
	glBindVertexArray( self->vao );
	glBindBuffer( GL_ARRAY_BUFFER, self->vbo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, self->ibo );
	glUseProgram( self->program );
}

void
gl_renderer_print_gl_info(
	void )
{
	g_print(
		"GL vendor: %s\n"
		"GL renderer: %s\n"
		"GL version: %s\n"
		"GLSL version %s\n"
		"GL extensions: %s\n",
		glGetString( GL_VENDOR ),
		glGetString( GL_RENDERER ),
		glGetString( GL_VERSION ),
		glGetString( GL_SHADING_LANGUAGE_VERSION ),
		glGetString( GL_EXTENSIONS ) );
}

void
gl_renderer_viewport(
	GLRenderer *self,
	GLint x,
	GLint y,
	GLsizei width,
	GLsizei height )
{
	GLfloat vtm[16] = {
		1.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	g_return_if_fail( self != NULL );

	/* update viewport */
	glViewport( x, y, width, height );

	/* update vertex transform matrix */
	vtm[0] = 2.0f / (GLfloat)width;
	vtm[5] = -2.0f / (GLfloat)height;
	glUniformMatrix4fv( self->vtm_uniform, 1, GL_TRUE, vtm );
}

void
gl_renderer_bind_texture(
	GLRenderer *self,
	GLRendererTexture *texture,
	GError **error )
{
	g_return_if_fail( self != NULL );
	g_return_if_fail( texture != NULL );

	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, self->fbo );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->tex, 0 );
	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		g_set_error(
			error,
			GL_RENDERER_ERROR,
			GL_RENDERER_ERROR_FRAME_BUFFER_COMPLETENESS_STATUS_FAILED,
			"Frame buffer completeness status failed" );
}

void
gl_renderer_bind_ms_texture(
	GLRenderer *self,
	GLRendererTexture *texture,
	GError **error )
{
	g_return_if_fail( self != NULL );
	g_return_if_fail( texture != NULL );

	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, self->msfbo );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture->mstex, 0 );
	if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		g_set_error(
			error,
			GL_RENDERER_ERROR,
			GL_RENDERER_ERROR_FRAME_BUFFER_COMPLETENESS_STATUS_FAILED,
			"Frame buffer completeness status failed" );
}

void
gl_renderer_blit_texture(
	GLRenderer *self,
	GLRendererTexture *texture )
{
	g_return_if_fail( self != NULL );
	g_return_if_fail( texture != NULL );

	glBindFramebuffer( GL_READ_FRAMEBUFFER, self->msfbo );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, self->fbo );
	glBlitFramebuffer( 0, 0, texture->width, texture->height, 0, 0, texture->width, texture->height, GL_COLOR_BUFFER_BIT, GL_NEAREST );
}

void
gl_renderer_draw(
	GLRenderer *self,
	GLRendererData *data,
	GLsizeiptr offset )
{
	g_return_if_fail( self != NULL );
	g_return_if_fail( data != NULL );

	glUniform4fv( self->color_uniform, 1, data->color );
	glUniformMatrix4fv( self->srtm_uniform, 1, GL_TRUE, data->srtm );

	glDrawElements( data->mode, data->count, GL_UNSIGNED_INT, BOFFSET( data->offset + offset ) );
}

void
gl_renderer_layout_free(
	GLRendererLayout *self )
{
	if( self == NULL )
		return;

	g_free( self->vertex );
	g_free( self->index );
	g_free( self );
}

GLRendererLayout*
gl_renderer_layout_new_merged(
	GLRendererLayout **layouts,
	GLsizeiptr layouts_count )
{
	GLsizeiptr i, j, k;
	GLsizeiptr index_norestart_num, index_norestart_num_offset;
	GLsizeiptr index_offset, vertex_offset;
	GLRendererLayout *layout;

	g_return_if_fail( layouts != NULL );

	layout = g_new( GLRendererLayout, 1 );

	layout->vertex_num = 0;
	layout->vertex_size = 0;
	layout->index_num = 0;
	layout->index_size = 0;
	for( i = 0; i < layouts_count; ++i )
	{
		layout->vertex_num += layouts[i]->vertex_num;
		layout->vertex_size += layouts[i]->vertex_size;
		layout->index_num += layouts[i]->index_num;
		layout->index_size += layouts[i]->index_size;
	}
	layout->vertex = (GLfloat*)g_malloc( layout->vertex_size );
	layout->index = (GLuint*)g_malloc( layout->index_size );
		
	index_norestart_num_offset = 0;
	vertex_offset = 0;
	index_offset = 0;
	k = 0;
	for( i = 0; i < layouts_count; ++i )
	{
		memcpy( POFFSET( layout->vertex, vertex_offset ), layouts[i]->vertex, layouts[i]->vertex_size );
		memcpy( POFFSET( layout->index, index_offset ), layouts[i]->index, layouts[i]->index_size );

		index_norestart_num = 0;
		for( j = 0; j < layouts[i]->index_num; ++j, ++k )
			if( layout->index[k] != GL_RENDERER_PRIMITIVE_RESTART_INDEX )
			{
				layout->index[k] += index_norestart_num_offset;
				index_norestart_num++;
			}
		index_norestart_num_offset += index_norestart_num;

		vertex_offset += layouts[i]->vertex_size;
		index_offset += layouts[i]->index_size;
	}

	return layout;
}

GLRendererLayout*
gl_renderer_layout_new_optimized(
	GLRendererLayout *layout )
{
	GLRendererLayout *opt_layout;
	GLsizeiptr i, j;
	GLsizeiptr vertex_size, vertex_num, vs;
	char *iv, *ov;
	gboolean is_vertex_found;

	g_return_val_if_fail( layout != NULL, NULL );

	opt_layout = g_new( GLRendererLayout, 1 );
	
	opt_layout->index_num = layout->index_num;
	opt_layout->index_size = layout->index_size;
	opt_layout->index = (GLuint*)g_malloc( layout->index_size );

	opt_layout->vertex = (GLfloat*)g_malloc( layout->vertex_size );
	vs = layout->vertex_size / layout->vertex_num;
	vertex_num = 0;
	vertex_size = 0;
	for( i = 0; i < layout->index_num; ++i )
	{
		if( layout->index[i] == GL_RENDERER_PRIMITIVE_RESTART_INDEX )
		{
			opt_layout->index[i] = GL_RENDERER_PRIMITIVE_RESTART_INDEX;
			continue;
		}

		iv = POFFSET( layout->vertex, vs * layout->index[i] );

		is_vertex_found = FALSE;
		for( j = 0; j < vertex_num; ++j )
		{
			ov = POFFSET( opt_layout->vertex, vs * j );
			if( gl_renderer_vertex_cmp( iv, ov, vs ) )
			{
				opt_layout->index[i] = j;
				is_vertex_found = TRUE;
				break;
			}
		}

		if( is_vertex_found )
			continue;

		ov = POFFSET( opt_layout->vertex, vertex_size );
		memcpy( ov, iv, vs );
		opt_layout->index[i] = vertex_num;
		vertex_num++;
		vertex_size += vs;
	}
	opt_layout->vertex_num = vertex_num;
	opt_layout->vertex_size = vertex_size;
	opt_layout->vertex = (GLfloat*)g_realloc( opt_layout->vertex, vertex_size );

	return opt_layout;
}

GLRendererTexture*
gl_renderer_texture_new(
	GLuint width,
	GLuint height,
	GLuint scale )
{
	GLRendererTexture *self = g_new( GLRendererTexture, 1 );

	self->width = width * scale;
	self->height = height * scale;
	self->scale = scale;

	glGenTextures( 1, &( self->mstex ) );
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, self->mstex );
	glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, GL_RENDERER_MSAA, GL_RGBA8, self->width, self->height, GL_TRUE );

	glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glGenTextures( 1, &( self->tex ) );
	glBindTexture( GL_TEXTURE_2D, self->tex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, self->width, self->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	return self;
}

void
gl_renderer_texture_free(
	GLRendererTexture *self )
{
	if( self == NULL )
		return;

	glDeleteTextures( 1, &( self->mstex ) );
	glDeleteTextures( 1, &( self->tex ) );
	g_free( self );
}

