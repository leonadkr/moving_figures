#ifndef GTK_MOVING_FIGURES_AREA_H
#define GTK_MOVING_FIGURES_AREA_H

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

enum _GtkMovingFigureType
{
	GTK_MOVING_FIGURE_TYPE_POINT,
	GTK_MOVING_FIGURE_TYPE_CIRCLE,
	GTK_MOVING_FIGURE_TYPE_POLYGON,
	GTK_MOVING_FIGURE_TYPE_STAR,

	N_GTK_MOVING_FIGURE_TYPE
};
typedef enum _GtkMovingFigureType GtkMovingFigureType;

#define GTK_TYPE_MOVING_FIGURES_AREA ( gtk_moving_figures_area_get_type() )
G_DECLARE_DERIVABLE_TYPE( GtkMovingFiguresArea, gtk_moving_figures_area, GTK, MOVING_FIGURES_AREA, GtkWidget )

struct _GtkMovingFiguresAreaClass
{
	GtkWidgetClass parent_class;
};
typedef struct _GtkMovingFiguresAreaClass GtkMovingFiguresAreaClass;

GtkMovingFiguresArea* gtk_moving_figures_area_new( guint, guint );
void gtk_moving_figures_area_reallocate( GtkMovingFiguresArea* );
void gtk_moving_figures_area_move( GtkMovingFiguresArea* );
void gtk_moving_figures_area_append( GtkMovingFiguresArea*, GtkMovingFigureType, gint );
void gtk_moving_figures_area_remove( GtkMovingFiguresArea*, GtkMovingFigureType, gint );
void gtk_moving_figures_area_set_number( GtkMovingFiguresArea*, GtkMovingFigureType, gint );

G_END_DECLS

#endif
