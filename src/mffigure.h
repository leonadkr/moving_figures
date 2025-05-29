#ifndef MFFIGURE_H
#define MFFIGURE_H

#include "mflayout.h"

#include <glib-object.h>
#include <glib.h>
#include <graphene.h>

G_BEGIN_DECLS

#define MF_TYPE_FIGURE ( mf_figure_get_type() )
G_DECLARE_FINAL_TYPE( MfFigure, mf_figure, MF, FIGURE, GObject )

MfFigure* mf_figure_new( void );
MfFigure* mf_figure_new_full( const graphene_vec4_t *color, const graphene_point_t *pos, const graphene_point_t *vel, gfloat angle, gfloat angular_velocity, gfloat scale, MfLayout *layout );
void mf_figure_get_color( MfFigure *self, graphene_vec4_t *color );
void mf_figure_get_colorv( MfFigure *self, gfloat *cv );
void mf_figure_set_color( MfFigure *self, const graphene_vec4_t *color );
void mf_figure_get_position( MfFigure *self, graphene_point_t *pos );
void mf_figure_set_position( MfFigure *self, const graphene_point_t *pos );
void mf_figure_get_velocity( MfFigure *self, graphene_point_t *vel );
void mf_figure_set_velocity( MfFigure *self, const graphene_point_t *vel );
gboolean mf_figure_get_velocity_set( MfFigure *self );
void mf_figure_set_velocity_set( MfFigure *self, gboolean vel_set );
gfloat mf_figure_get_angle( MfFigure *self );
void mf_figure_set_angle( MfFigure *self, gfloat angle );
gfloat mf_figure_get_angular_velocity( MfFigure *self );
void mf_figure_set_angular_velocity( MfFigure *self, gfloat vel );
gboolean mf_figure_get_angular_velocity_set( MfFigure *self );
void mf_figure_set_angular_velocity_set( MfFigure *self, gboolean vel_set );
gfloat mf_figure_get_scale( MfFigure *self );
void mf_figure_set_scale( MfFigure *self, gfloat scale );
MfLayout* mf_figure_get_layout( MfFigure *self );
void mf_figure_set_layout( MfFigure *self, MfLayout *layout );
void mf_figure_get_srtm( MfFigure *self, graphene_matrix_t *srtm );
gboolean mf_figure_is_inside_rectangle( MfFigure *self, const graphene_rect_t *rect );
void mf_figure_move( MfFigure *self, const graphene_rect_t *rect, gfloat fps );

G_END_DECLS

#endif
