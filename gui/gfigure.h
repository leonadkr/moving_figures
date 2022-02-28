#ifndef G_FIGURE_H
#define G_FIGURE_H

#include <glib-object.h>
#include <glib.h>
#include "glrenderer.h"

G_BEGIN_DECLS

#define G_TYPE_FIGURE ( g_figure_get_type() )
G_DECLARE_DERIVABLE_TYPE( GFigure, g_figure, G, FIGURE, GObject )

struct _GFigureClass
{
	GObjectClass parent_class;

	void (*move)( GFigure *self, GLRectangle *rect );
	void (*randomize)( GFigure *self, GRand *rnd, GLRectangle *rect, gfloat fps );
	GLRendererData (*get_data)( GFigure *self );
};
typedef struct _GFigureClass GFigureClass;

GFigure* g_figure_new( void );
void g_figure_move( GFigure *self, GLRectangle *rect );
void g_figure_randomize( GFigure *self, GRand *rnd, GLRectangle *rect, gfloat fps );
gboolean g_figure_is_inside_rect( GFigure *self, GLRectangle *rect );
GLRendererData g_figure_get_data( GFigure *self );
GLRendererLayout* g_figure_class_get_layout( void );

G_END_DECLS

#endif
