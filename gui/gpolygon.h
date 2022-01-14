#ifndef G_POLYGON_H
#define G_POLYGON_H

#include <gtk/gtk.h>
#include <glib-object.h>

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

G_END_DECLS

#endif
