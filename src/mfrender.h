#ifndef MFRENDER_H
#define MFRENDER_H

#include "mffigure.h"

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

enum _MfRenderError
{
	MF_RENDER_ERROR_SHADER_COMPILATION,
	MF_RENDER_ERROR_PROGRAM_LINKING,
	MF_RENDER_ERROR_ALREADY_COMPLETED,

	N_MF_RENDER_ERROR
};
typedef enum _MfRenderError MfRenderError;

#define MF_RENDER_ERROR ( mf_render_error_quark() )
GQuark mf_render_error_quark( void ) G_GNUC_CONST;

#define MF_TYPE_RENDER ( mf_render_get_type() )
G_DECLARE_FINAL_TYPE( MfRender, mf_render, MF, RENDER, GObject )

MfRender* mf_render_new( void );
gboolean mf_render_complete( MfRender *self, GBytes *vshader_bytes, GBytes *fshader_bytes, GBytes *vertices_bytes, GBytes *indices_bytes, GError **error );
gboolean mf_render_is_completed( MfRender *self );
void mf_render_set_size( MfRender *self, gfloat width, gfloat height );
void mf_render_draw( MfRender *self, MfFigure *fig );

G_END_DECLS

#endif
