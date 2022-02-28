#ifndef G_POLYGON_H
#define G_POLYGON_H

#include "gcircle.h"

G_BEGIN_DECLS

#define G_TYPE_POLYGON ( g_polygon_get_type() )
G_DECLARE_DERIVABLE_TYPE( GPolygon, g_polygon, G, POLYGON, GCircle )

struct _GPolygonClass
{
	GCircleClass parent_class;
};
typedef struct _GPolygonClass GPolygonClass;

GPolygon* g_polygon_new( void );
GLRendererLayout* g_polygon_class_get_layout( void );

G_END_DECLS

#endif
