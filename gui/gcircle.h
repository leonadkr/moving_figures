#ifndef G_CIRCLE_H
#define G_CIRCLE_H

#include <gtk/gtk.h>
#include <glib-object.h>

#include "gpoint.h"


G_BEGIN_DECLS

#define G_TYPE_CIRCLE ( g_circle_get_type() )
G_DECLARE_DERIVABLE_TYPE( GCircle, g_circle, G, CIRCLE, GPoint )

struct _GCircleClass
{
	GPointClass parent_class;
};
typedef struct _GCircleClass GCircleClass;

GCircle* g_circle_new( void );

G_END_DECLS

#endif
