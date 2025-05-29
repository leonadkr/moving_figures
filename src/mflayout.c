#include "mflayout.h"

#include <glib-object.h>
#include <glib.h>
#include <epoxy/gl.h>

struct _MfLayout
{
	GObject parent_instance;

	gchar *name;
	guint id;
	guint mode;
	gulong count;
	gulong offset;
};
typedef struct _MfLayout MfLayout;

enum _MfLayoutPropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_NAME,
	PROP_ID,
	PROP_MODE,
	PROP_COUNT,
	PROP_OFFSET,

	N_PROPS
};
typedef enum _MfLayoutPropertyID MfLayoutPropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE( MfLayout, mf_layout, G_TYPE_OBJECT )

static void
mf_layout_init(
	MfLayout *self )
{
	self->name = NULL;
	self->id = 0;
	self->mode = GL_POINTS;
	self->count = 0L;
	self->offset = 0L;
}

static void
mf_layout_finalize(
	GObject *object )
{
	MfLayout *self = MF_LAYOUT( object );

	g_free( self->name );

	G_OBJECT_CLASS( mf_layout_parent_class )->finalize( object );
}

static void
mf_layout_get_property(
	GObject *object,	
	guint prop_id,	
	GValue *value,	
	GParamSpec *pspec )
{
	MfLayout *self = MF_LAYOUT( object );

	switch( (MfLayoutPropertyID)prop_id )
	{
		case PROP_NAME:
			g_value_set_string( value, self->name );
			break;
		case PROP_ID:
			g_value_set_uint( value, self->id );
			break;
		case PROP_MODE:
			g_value_set_uint( value, self->mode );
			break;
		case PROP_COUNT:
			g_value_set_ulong( value, self->count );
			break;
		case PROP_OFFSET:
			g_value_set_ulong( value, self->offset );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_layout_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	MfLayout *self = MF_LAYOUT( object );

	switch( (MfLayoutPropertyID)prop_id )
	{
		case PROP_NAME:
			mf_layout_set_name( self, g_value_get_string( value ) );
			break;
		case PROP_ID:
			mf_layout_set_id( self, g_value_get_uint( value ) );
			break;
		case PROP_MODE:
			mf_layout_set_mode( self, g_value_get_uint( value ) );
			break;
		case PROP_COUNT:
			mf_layout_set_count( self, g_value_get_ulong( value ) );
			break;
		case PROP_OFFSET:
			mf_layout_set_offset( self, g_value_get_ulong( value ) );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_layout_class_init(
	MfLayoutClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );

	object_class->finalize = mf_layout_finalize;
	object_class->get_property = mf_layout_get_property;
	object_class->set_property = mf_layout_set_property;
	object_props[PROP_NAME] = g_param_spec_string(
		"name",
		"Name",
		"Name of the model",
		NULL,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_ID] = g_param_spec_uint(
		"id",
		"ID",
		"ID of the model",
		0,
		G_MAXUINT,
		0,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_MODE] = g_param_spec_uint(
		"mode",
		"Mode",
		"Drawing mode",
		0,
		G_MAXUINT,
		GL_POINTS,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_COUNT] = g_param_spec_ulong(
		"count",
		"Count",
		"Count of indices",
		0,
		G_MAXULONG,
		0,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_OFFSET] = g_param_spec_ulong(
		"offset",
		"Offset",
		"Offset in the index array",
		0,
		G_MAXULONG,
		0,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	g_object_class_install_properties( object_class, N_PROPS, object_props );
}

MfLayout*
mf_layout_new(
	void )
{
	return MF_LAYOUT( g_object_new( MF_TYPE_LAYOUT, NULL ) );
}

MfLayout*
mf_layout_new_full(
	const gchar *name,
	guint id,
	guint mode,
	gulong count,
	gulong offset )
{
	return MF_LAYOUT( g_object_new( MF_TYPE_LAYOUT, "name", name, "id", id, "mode", mode, "count", count, "offset", offset, NULL ) );
}

gchar*
mf_layout_get_name(
	MfLayout *self )
{
	g_return_val_if_fail( MF_IS_LAYOUT( self ), NULL );

	return g_strdup( self->name );
}

void
mf_layout_set_name(
	MfLayout *self,
	const gchar *name )
{
	g_return_if_fail( MF_IS_LAYOUT( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	g_free( self->name );
	self->name = g_strdup( name );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_NAME] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

guint
mf_layout_get_id(
	MfLayout *self )
{
	g_return_val_if_fail( MF_IS_LAYOUT( self ), 0 );

	return self->id;
}

void
mf_layout_set_id(
	MfLayout *self,
	guint id )
{
	g_return_if_fail( MF_IS_LAYOUT( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->id = id;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_ID] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

guint
mf_layout_get_mode(
	MfLayout *self )
{
	g_return_val_if_fail( MF_IS_LAYOUT( self ), GL_POINTS );

	return self->mode;
}

void
mf_layout_set_mode(
	MfLayout *self,
	guint mode )
{
	g_return_if_fail( MF_IS_LAYOUT( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->mode = mode;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_MODE] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gulong
mf_layout_get_count(
	MfLayout *self )
{
	g_return_val_if_fail( MF_IS_LAYOUT( self ), 0 );

	return self->count;
}

void
mf_layout_set_count(
	MfLayout *self,
	gulong count )
{
	g_return_if_fail( MF_IS_LAYOUT( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->count = count;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_COUNT] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gulong
mf_layout_get_offset(
	MfLayout *self )
{
	g_return_val_if_fail( MF_IS_LAYOUT( self ), 0L );

	return self->offset;
}

void
mf_layout_set_offset(
	MfLayout *self,
	gulong offset )
{
	g_return_if_fail( MF_IS_LAYOUT( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->offset = offset;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_OFFSET] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

