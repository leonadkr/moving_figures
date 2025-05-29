#ifndef MFCUSTOMFIGURE_H
#define MFCUSTOMFIGURE_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MF_TYPE_CUSTOM_FIGURE ( mf_custom_figure_get_type() )
G_DECLARE_FINAL_TYPE( MfCustomFigure, mf_custom_figure, MF, CUSTOM_FIGURE, GtkWidget )

MfCustomFigure* mf_custom_figure_new( void );
MfCustomFigure* mf_custom_figure_new_with_layouts( GListStore *layouts );
GListStore* mf_custom_figure_get_layouts( MfCustomFigure *self );
void mf_custom_figure_set_layouts( MfCustomFigure *self, GListStore *layouts );
void mf_custom_figure_set_size( MfCustomFigure *self, gdouble width, gdouble height );

G_END_DECLS

#endif
