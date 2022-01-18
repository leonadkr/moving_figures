#include <stdlib.h>
#include <gtk/gtk.h>
#include "moving_figures.h"
#include "fps.h"
#include "gpoint.h"
#include "gcircle.h"
#include "gpolygon.h"
#include "gstar.h"


/*
	definitions
*/
#define MOVING_FIGURES_STRUCT_TYPE ( (gint)sizeof( MovingFigures ) )


/*
	structures
*/
struct _MovingFigure
{
	gint struct_type;
	GList *fig[N_MOVING_FIGURE_TYPE];
	gint fignum[N_MOVING_FIGURE_TYPE];
	GdkRectangle rect;
};
typedef struct _MovingFigure MovingFigures;


/*
	function prototypes
*/
static void figure_randomize( GFigure *figure, GRand *rnd, MovingFigureType fig_type, GdkRectangle *rect );


/*
	private
*/
static void
figure_randomize(
	GFigure *figure,
	GRand *rnd,
	MovingFigureType fig_type,
	GdkRectangle *rect )
{
	GdkRGBA color;

	switch( fig_type )
	{
		case MOVING_FIGURE_TYPE_STAR:
		case MOVING_FIGURE_TYPE_POLYGON:
			g_object_set( G_OBJECT( figure ),
				"corner", g_rand_int_range( rnd, 5, 10 ),
				"rvel", g_rand_int_range( rnd, -180, 180 ) / FPS,
				NULL );
		case MOVING_FIGURE_TYPE_CIRCLE:
			g_object_set( G_OBJECT( figure ),
				"radius", g_rand_int_range( rnd, 10, 20 ),
				"fill-mode", g_rand_int_range( rnd, 0, N_G_FIGURE_FILL_MODE ),
				NULL );
		case MOVING_FIGURE_TYPE_POINT:
			color = (GdkRGBA){
				.red = g_rand_double( rnd ),
				.green = g_rand_double( rnd ),
				.blue = g_rand_double( rnd ),
				.alpha = 1.0 };
			g_object_set( G_OBJECT( figure ),
				"x", rect->x + g_rand_int_range( rnd, 0, rect->width ),
				"y", rect->y + g_rand_int_range( rnd, 0, rect->height ),
				"velx", g_rand_int_range( rnd, -200, 200 ) / FPS,
				"vely", g_rand_int_range( rnd, -200, 200 ) / FPS,
				"color", &color,
				NULL );
			break;
		default:
			g_return_if_fail( fig_type < N_MOVING_FIGURE_TYPE );
			break;
	}
}


/*
	public
*/
gboolean
is_moving_figures_struct(
	MovingFigures *mf )
{
	if( mf == NULL )
		return FALSE;
		
	if( mf->struct_type != MOVING_FIGURES_STRUCT_TYPE )
		return FALSE;

	return TRUE;
}

MovingFigures*
moving_figures_new(
	void )
{
	MovingFigures *mf;
	int fig_type;

	mf = g_new( MovingFigures, 1 );
	mf->struct_type = MOVING_FIGURES_STRUCT_TYPE;
	for( fig_type = 0; fig_type < N_MOVING_FIGURE_TYPE; ++fig_type )
	{
		mf->fig[fig_type] = NULL;
		mf->fignum[fig_type] = 0;
	}
	mf->rect = (GdkRectangle){
		.x = 0,
		.y = 0,
		.width = 1,
		.height = 1 };

	return mf;
}

void
moving_figures_free(
	MovingFigures *mf )
{
	int fig_type;

	g_return_if_fail( is_moving_figures_struct( mf ) );

	for( fig_type = 0; fig_type < N_MOVING_FIGURE_TYPE; ++fig_type )
		g_list_free_full( mf->fig[fig_type], (GDestroyNotify)g_object_unref );
	g_free( mf );
}

void
moving_figures_set_rectangle(
	MovingFigures *mf,
	GdkRectangle *rect )
{
	g_return_if_fail( is_moving_figures_struct( mf ) );

	mf->rect = *rect;
}

void
moving_figures_reallocate(
	MovingFigures *mf )
{
	int fig_type;
	GList *l;
	GRand *rnd;

	g_return_if_fail( is_moving_figures_struct( mf ) );

	rnd = g_rand_new();
	for( fig_type = 0; fig_type < N_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = mf->fig[fig_type]; l != NULL; l = l->next )
			figure_randomize( G_FIGURE( l->data ), rnd, fig_type, &( mf->rect ) );
	g_rand_free( rnd );
}

void
moving_figures_move(
	MovingFigures *mf )
{
	int fig_type;
	GList *l;

	g_return_if_fail( is_moving_figures_struct( mf ) );

	for( fig_type = 0; fig_type < N_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = mf->fig[fig_type]; l != NULL; l = l->next )
			if( g_figure_is_inside_rect( G_FIGURE( l->data ), &( mf->rect ) ) )
				g_figure_move( G_FIGURE( l->data ), &( mf->rect ) );
}

void
moving_figures_draw(
	MovingFigures *mf,
	cairo_t *cr )
{
	int fig_type;
	GList *l;
	
	g_return_if_fail( is_moving_figures_struct( mf ) );

	for( fig_type = 0; fig_type < N_MOVING_FIGURE_TYPE; ++fig_type )
		for( l = mf->fig[fig_type]; l != NULL; l = l->next )
			if( g_figure_is_inside_rect( G_FIGURE( l->data ), &( mf->rect ) ) )
				g_figure_draw( G_FIGURE( l->data ), cr );
}

void
moving_figures_append(
	MovingFigures *mf,
	MovingFigureType fig_type,
	gint num )
{
	gint i;
	GFigure *figure;
	GRand *rnd;

	g_return_if_fail( is_moving_figures_struct( mf ) );
	g_return_if_fail( num >= 0 );

	rnd = g_rand_new();
	for( i = 0; i < num; ++i )
	{
		switch( fig_type )
		{
			case MOVING_FIGURE_TYPE_POINT:
				figure = G_FIGURE( g_point_new() );
				break;
			case MOVING_FIGURE_TYPE_CIRCLE:
				figure = G_FIGURE( g_circle_new() );
				break;
			case MOVING_FIGURE_TYPE_POLYGON:
				figure = G_FIGURE( g_polygon_new() );
				break;
			case MOVING_FIGURE_TYPE_STAR:
				figure = G_FIGURE( g_star_new() );
				break;
			default:
				g_return_if_fail( fig_type < N_MOVING_FIGURE_TYPE );
				figure = NULL;
				break;
		}
		figure_randomize( figure, rnd, fig_type, &( mf->rect ) );
		mf->fig[fig_type] = g_list_prepend( mf->fig[fig_type], figure );
	}
	mf->fignum[fig_type] += num;
	g_rand_free( rnd );
}

void
moving_figures_remove(
	MovingFigures *mf,
	MovingFigureType fig_type,
	gint num )
{
	gint i;
	GList *l;

	g_return_if_fail( is_moving_figures_struct( mf ) );
	g_return_if_fail( num >= 0 );

	for( i = 0, l = mf->fig[fig_type]; i < num && l != NULL; ++i, l = l->next )
		g_object_unref( G_OBJECT( l->data ) );
	mf->fig[fig_type] = l;

	if( num > mf->fignum[fig_type] )
		mf->fignum[fig_type] = 0;
	else
		mf->fignum[fig_type] -= num;
}

void
moving_figures_set_number(
	MovingFigures *mf,
	MovingFigureType fig_type,
	gint num )
{
	g_return_if_fail( is_moving_figures_struct( mf ) );
	g_return_if_fail( num >= 0 );

	if( num > mf->fignum[fig_type] )
		moving_figures_append( mf, fig_type, num - mf->fignum[fig_type] );
	else
		moving_figures_remove( mf, fig_type, mf->fignum[fig_type] - num );
}

