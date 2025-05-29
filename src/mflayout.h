#ifndef MFLAYOUT_H
#define MFLAYOUT_H

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define MF_TYPE_LAYOUT ( mf_layout_get_type() )
G_DECLARE_FINAL_TYPE( MfLayout, mf_layout, MF, LAYOUT, GObject )

MfLayout* mf_layout_new( void );
MfLayout* mf_layout_new_full( const gchar *name, guint id, guint mode, gulong count, gulong offset );
gchar* mf_layout_get_name( MfLayout *self );
void mf_layout_set_name( MfLayout *self, const gchar *name );
guint mf_layout_get_id( MfLayout *self );
void mf_layout_set_id( MfLayout *self, guint id );
guint mf_layout_get_mode( MfLayout *self );
void mf_layout_set_mode( MfLayout *self, guint mode );
gulong mf_layout_get_count( MfLayout *self );
void mf_layout_set_count( MfLayout *self, gulong count );
gulong mf_layout_get_offset( MfLayout *self );
void mf_layout_set_offset( MfLayout *self, gulong offset );

G_END_DECLS

#endif
