#ifndef MFDRAWAREA_H
#define MFDRAWAREA_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include <graphene.h>

G_BEGIN_DECLS

#define MF_TYPE_DRAW_AREA ( mf_draw_area_get_type() )
G_DECLARE_FINAL_TYPE( MfDrawArea, mf_draw_area, MF, DRAW_AREA, GtkWidget )

MfDrawArea* mf_draw_area_new( void );
void mf_draw_area_get_rectangle( MfDrawArea *self, graphene_rect_t *rect );
GdkGLContext* mf_draw_area_get_context( MfDrawArea *self );

G_END_DECLS

#endif
