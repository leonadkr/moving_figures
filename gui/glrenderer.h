#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include <glib-2.0/glib.h>
#include <epoxy/gl.h>

#define BOFFSET( offset ) ( (char*)NULL + ( offset ) )
#define POFFSET( pointer, offset ) ( (char*)( pointer ) + ( offset ) )
#define GL_RENDERER_PRIMITIVE_RESTART_INDEX ( (GLuint)-1 )

struct _GLRectangle
{
	gfloat x, y;
	gfloat width, height;
};
typedef struct _GLRectangle GLRectangle;

struct _GLRendererData
{
	GLenum mode;
	GLsizeiptr offset, count;
	GLfloat color[4], srtm[16];
};
typedef struct _GLRendererData GLRendererData;

struct _GLRenderer
{
	GLuint vao, vbo, ibo;
	GLuint program;
	GLuint position_index;
	GLuint color_uniform;
	GLuint vtm_uniform;
	GLuint srtm_uniform;
};
typedef struct _GLRenderer GLRenderer;

struct _GLRendererLayout
{
	GLfloat *vertex;
	GLsizeiptr vertex_num;
	GLsizeiptr vertex_size;
	GLuint *index;
	GLsizeiptr index_num;
	GLsizeiptr index_size;
};
typedef struct _GLRendererLayout GLRendererLayout;

struct _GLTexture
{
	GLuint id;
	GLuint width, height, scale;
};
typedef struct _GLTexture GLTexture;

GLRenderer* gl_renderer_new( const char *vshader_code, const char *fshader_code, GLRendererLayout *layout, GError **error );
void gl_renderer_free( GLRenderer *renderer );
void gl_renderer_make_current( GLRenderer *self );
void gl_renderer_print_gl_info( void );

void gl_renderer_layout_free( GLRendererLayout *layout );
GLRendererLayout* gl_renderer_layout_new_merged( GLRendererLayout **layouts, GLsizeiptr layouts_count );
GLRendererLayout* gl_renderer_layout_new_optimized( GLRendererLayout *layout );

GLTexture* gl_texture_new( GLuint widht, GLuint height, GLuint scale );
void gl_texture_free( GLTexture *self );

#endif
