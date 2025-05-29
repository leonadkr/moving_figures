#ifndef MFFIGUREINFO_H
#define MFFIGUREINFO_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <graphene.h>

G_BEGIN_DECLS

#define MF_TYPE_FIGURE_INFO ( mf_figure_info_get_type() )
G_DECLARE_FINAL_TYPE( MfFigureInfo, mf_figure_info, MF, FIGURE_INFO, GtkWidget )

MfFigureInfo* mf_figure_info_new( void );
MfFigureInfo* mf_figure_info_new_full( const gchar *name, graphene_vec4_t *color );
gchar* mf_figure_info_get_name( MfFigureInfo *self );
void mf_figure_info_set_name( MfFigureInfo *self, const gchar *name );
void mf_figure_info_get_color( MfFigureInfo *self, graphene_vec4_t *color );
void mf_figure_info_set_color( MfFigureInfo *self, const graphene_vec4_t *color );

G_END_DECLS

#endif
