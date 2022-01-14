#ifndef MOVING_FIGURES_H
#define MOVING_FIGURES_H

#include <gtk/gtk.h>

enum _MovingFigureType
{
	MOVING_FIGURE_TYPE_POINT,
	MOVING_FIGURE_TYPE_CIRCLE,
	MOVING_FIGURE_TYPE_POLYGON,
	MOVING_FIGURE_TYPE_STAR,

	N_MOVING_FIGURE_TYPE
};
typedef enum _MovingFigureType MovingFigureType;

typedef struct _MovingFigure MovingFigures;

gboolean is_moving_figures_struct( MovingFigures *mf );
MovingFigures* moving_figures_new( void );
void moving_figures_free( MovingFigures *mf );
void moving_figures_set_rectangle( MovingFigures *mf, GdkRectangle *rect );
void moving_figures_reallocate( MovingFigures *mf );
void moving_figures_move( MovingFigures *mf );
void moving_figures_draw( MovingFigures *mf, cairo_t *cr );
void moving_figures_append( MovingFigures *mf, MovingFigureType fig_type, gint num );
void moving_figures_remove( MovingFigures *mf, MovingFigureType fig_type, gint num );
void moving_figures_set_number( MovingFigures *mf, MovingFigureType fig_type, gint num );

#endif

