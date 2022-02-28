#ifndef G_POINT_H
#define G_POINT_H

#include "gfigure.h"

G_BEGIN_DECLS

#define G_TYPE_POINT ( g_point_get_type() )
G_DECLARE_DERIVABLE_TYPE( GPoint, g_point, G, POINT, GFigure )

struct _GPointClass
{
	GFigureClass parent_class;
};
typedef struct _GPointClass GPointClass;

GPoint* g_point_new( void );
GLRendererLayout* g_point_class_get_layout( void );

G_END_DECLS

#endif
