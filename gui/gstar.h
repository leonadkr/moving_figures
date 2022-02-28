#ifndef G_STAR_H
#define G_STAR_H

#include "gpolygon.h"


G_BEGIN_DECLS

#define G_TYPE_STAR ( g_star_get_type() )
G_DECLARE_DERIVABLE_TYPE( GStar, g_star, G, STAR, GPolygon )

struct _GStarClass
{
	GPolygonClass parent_class;
};
typedef struct _GStarClass GStarClass;

GStar* g_star_new( void );
GLRendererLayout* g_star_class_get_layout( void );

G_END_DECLS

#endif
