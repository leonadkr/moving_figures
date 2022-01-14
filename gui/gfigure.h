#ifndef G_FIGURE_H
#define G_FIGURE_H

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

enum _GFigureFillMode
{
	G_FIGURE_FILL_MODE_FILL,
	G_FIGURE_FILL_MODE_UNFILL,

	N_G_FIGURE_FILL_MODE
};
typedef enum _GFigureFillMode GFigureFillMode;

#define G_TYPE_FIGURE ( g_figure_get_type() )
G_DECLARE_DERIVABLE_TYPE( GFigure, g_figure, G, FIGURE, GObject )

struct _GFigureClass
{
	GObjectClass parent_class;

	void (*move)( GFigure*, GdkRectangle* );
	void (*draw)( GFigure*, cairo_t* );
};
typedef struct _GFigureClass GFigureClass;

GFigure* g_figure_new( void );
void g_figure_move( GFigure*, GdkRectangle* );
void g_figure_draw( GFigure*, cairo_t* );

G_END_DECLS

#endif
