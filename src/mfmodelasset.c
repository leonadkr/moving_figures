#include "mfmodelasset.h"

#include "mflayout.h"

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <epoxy/gl.h>

struct _MfModelAsset
{
	GObject parent_instance;

	GBytes *vertices;
	GBytes *indices;
	GListStore *layouts;
	gboolean is_loaded;
};
typedef struct _MfModelAsset MfModelAsset;

G_DEFINE_ENUM_TYPE( MfModelAssetError, mf_model_asset_error,
	G_DEFINE_ENUM_VALUE( MF_MODEL_ASSET_ERROR_VERTEX_ARRAY_LENGTH_IS_NOT_EVEN, "vertex-array-length-is-not-even" ),
	G_DEFINE_ENUM_VALUE( MF_MODEL_ASSET_ERROR_INDEX_ARRAY_CONTAINS_NEGATIVE_INTEGER_LESS_MINUS_ONE, "index-array-contains-negative-integer-less-minus-one" ),
	G_DEFINE_ENUM_VALUE( MF_MODEL_ASSET_ERROR_INDEX_ARRAY_CONTAINS_INDEX_OUT_OF_VERTEX_NUMBER, "index-array-contains-index-out-of-vertex-number" ),
	G_DEFINE_ENUM_VALUE( MF_MODEL_ASSET_ERROR_UNKNOWN_MODE, "unknown-mode" ),
	G_DEFINE_ENUM_VALUE( MF_MODEL_ASSET_ERROR_ALREADY_LOADED, "already-loaded" ) )

G_DEFINE_QUARK( mf-model-asset-error, mf_model_asset_error )

G_DEFINE_TYPE( MfModelAsset, mf_model_asset, G_TYPE_OBJECT )

static void
mf_model_asset_init(
	MfModelAsset *self )
{
	self->vertices = NULL;
	self->indices = NULL;
	self->layouts = g_list_store_new( MF_TYPE_LAYOUT );
	self->is_loaded = FALSE;
}

static void
mf_model_asset_dispose(
	GObject *object )
{
	MfModelAsset *self = MF_MODEL_ASSET( object );

	g_clear_object( &self->layouts );

	G_OBJECT_CLASS( mf_model_asset_parent_class )->dispose( object );
}

static void
mf_model_asset_finalize(
	GObject *object )
{
	MfModelAsset *self = MF_MODEL_ASSET( object );

	g_bytes_unref( self->vertices );
	g_bytes_unref( self->indices );

	G_OBJECT_CLASS( mf_model_asset_parent_class )->finalize( object );
}

static void
mf_model_asset_class_init(
	MfModelAssetClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );

	object_class->dispose = mf_model_asset_dispose;
	object_class->finalize = mf_model_asset_finalize;
}

MfModelAsset*
mf_model_asset_new(
	void )
{
	return MF_MODEL_ASSET( g_object_new( MF_TYPE_MODEL_ASSET, NULL ) );
}

MfModelAsset*
mf_model_asset_new_from_bytes(
	GBytes *bytes,
	GError **error )
{
	MfModelAsset *self = MF_MODEL_ASSET( g_object_new( MF_TYPE_MODEL_ASSET, NULL ) );
	GError *loc_error = NULL;

	mf_model_asset_load_from_bytes( self, bytes, &loc_error );
	if( loc_error != NULL )
	{
		g_propagate_error( error, loc_error );
		g_object_unref( G_OBJECT( self ) );
		return NULL;
	}

	return self;
}

static guint
mode_string_to_enum(
	const gchar *mode_str,
	GError **error )
{
	if( g_strcmp0( mode_str, "GL_POINTS" ) == 0 )
		return GL_POINTS;
	if( g_strcmp0( mode_str, "GL_LINE_STRIP" ) == 0 )
		return GL_LINE_STRIP;
	if( g_strcmp0( mode_str, "GL_LINE_LOOP" ) == 0 )
		return GL_LINE_LOOP;
	if( g_strcmp0( mode_str, "GL_LINES" ) == 0 )
		return GL_LINES;
	if( g_strcmp0( mode_str, "GL_LINE_STRIP_ADJACENCY" ) == 0 )
		return GL_LINE_STRIP_ADJACENCY;
	if( g_strcmp0( mode_str, "GL_LINES_ADJACENCY" ) == 0 )
		return GL_LINES_ADJACENCY;
	if( g_strcmp0( mode_str, "GL_TRIANGLE_STRIP" ) == 0 )
		return GL_TRIANGLE_STRIP;
	if( g_strcmp0( mode_str, "GL_TRIANGLE_FAN" ) == 0 )
		return GL_TRIANGLE_FAN;
	if( g_strcmp0( mode_str, "GL_TRIANGLES" ) == 0 )
		return GL_TRIANGLES;
	if( g_strcmp0( mode_str, "GL_TRIANGLE_STRIP_ADJACENCY" ) == 0 )
		return GL_TRIANGLE_STRIP_ADJACENCY;
	if( g_strcmp0( mode_str, "GL_TRIANGLES_ADJACENCY" ) == 0 )
		return GL_TRIANGLES_ADJACENCY;
	if( g_strcmp0( mode_str, "GL_PATCHES" ) == 0 )
		return GL_PATCHES;

	g_set_error(
		error,
		MF_MODEL_ASSET_ERROR,
		MF_MODEL_ASSET_ERROR_UNKNOWN_MODE,
		"Unknown mode \"%s\".", mode_str );

	return GL_POINTS;
}

gboolean
mf_model_asset_load_from_bytes(
	MfModelAsset *self,
	GBytes *bytes,
	GError **error )
{
	gchar *name, *mode_str;
	gint id, mode;
	gdouble *coords;
	gint *indices;
	gfloat *v;
	gsize coords_num, indices_num;
	gsize vertices_num, vertices_num_total;
	gsize i;
	gulong offset;
	GArray *vertices_arr, *indices_arr;
	GStrv models, m;
	MfLayout *layout;
	GKeyFile *key_file;
	gboolean ret = FALSE;
	GError *loc_error = NULL;

	g_return_val_if_fail( MF_IS_MODEL_ASSET( self ), FALSE );
	g_return_val_if_fail( bytes != NULL, FALSE );

	if( mf_model_asset_is_loaded( self ) )
	{
		g_set_error(
			error,
			MF_MODEL_ASSET_ERROR,
			MF_MODEL_ASSET_ERROR_ALREADY_LOADED,
			"The model asset is already loaded." );
		return FALSE;
	}

	g_bytes_ref( bytes );
	key_file = g_key_file_new();

	g_key_file_load_from_bytes( key_file, bytes, G_KEY_FILE_NONE, &loc_error );
	if( loc_error != NULL )
	{
		g_propagate_error( error, loc_error );
		goto out1;
	}

	/* scan model array */
	models = g_key_file_get_string_list( key_file, "Main", "models", NULL, &loc_error );
	if( loc_error != NULL )
	{
		g_propagate_error( error, loc_error );
		goto out1;
	}

	/* scan models */
	vertices_num_total = 0;
	offset = 0;
	vertices_arr = g_array_new( FALSE, FALSE, sizeof( gfloat ) );
	indices_arr = g_array_new( FALSE, FALSE, sizeof( guint ) );
	for( m = models; *m != NULL; ++m )
	{
		name = g_key_file_get_string( key_file, *m, "name", &loc_error );
		if( loc_error != NULL )
		{
			g_propagate_error( error, loc_error );
			goto out2;
		}

		id = g_key_file_get_integer( key_file, *m, "ID", &loc_error );
		if( loc_error != NULL )
		{
			g_propagate_error( error, loc_error );
			g_free( name );
			goto out2;
		}

		mode_str = g_key_file_get_string( key_file, *m, "mode", &loc_error );
		if( loc_error != NULL )
		{
			g_propagate_error( error, loc_error );
			g_free( name );
			goto out2;
		}
		mode = mode_string_to_enum( mode_str, &loc_error );
		g_free( mode_str );
		if( loc_error != NULL )
		{
			g_propagate_error( error, loc_error );
			g_free( name );
			goto out2;
		}

		coords = g_key_file_get_double_list( key_file, *m, "vertices", &coords_num, &loc_error );
		if( loc_error != NULL )
		{
			g_propagate_error( error, loc_error );
			g_free( name );
			goto out2;
		}

		indices = g_key_file_get_integer_list( key_file, *m, "indices", &indices_num, &loc_error );
		if( loc_error != NULL )
		{
			g_propagate_error( error, loc_error );
			g_free( name );
			g_free( coords );
			goto out2;
		}

		if( coords_num % 2 != 0 )
		{
			g_set_error(
				error,
				MF_MODEL_ASSET_ERROR,
				MF_MODEL_ASSET_ERROR_VERTEX_ARRAY_LENGTH_IS_NOT_EVEN,
				"Vertex array length is not even in group \"%s\".", *m );
			g_free( name );
			g_free( coords );
			g_free( indices );
			goto out2;
		}

		vertices_num = coords_num / 2;
		for( i = 0; i < indices_num; ++i )
		{
			if( indices[i] < -1 )
			{
				g_set_error(
					error,
					MF_MODEL_ASSET_ERROR,
					MF_MODEL_ASSET_ERROR_INDEX_ARRAY_CONTAINS_NEGATIVE_INTEGER_LESS_MINUS_ONE,
					"Index array contains a negative integer less -1 in group \"%s\".", *m );
				g_free( name );
				g_free( coords );
				g_free( indices );
				goto out2;
			}
			if( indices[i] != -1 && (gsize)indices[i] >= vertices_num )
			{
				g_set_error(
					error,
					MF_MODEL_ASSET_ERROR,
					MF_MODEL_ASSET_ERROR_INDEX_ARRAY_CONTAINS_INDEX_OUT_OF_VERTEX_NUMBER,
					"Index array contains index out of vertex number in group \"%s\".", *m );
				g_free( name );
				g_free( coords );
				g_free( indices );
				goto out2;
			}
		}

		/* convert vertices' coordinates from double to float */
		v = g_new( gfloat, coords_num );
		for( i = 0; i < coords_num; ++i )
			v[i] = coords[i];
		g_free( coords );
		g_array_append_vals( vertices_arr, v, coords_num );
		g_free( v );

		/* append shifting to indices, ignoring -1 */
		for( i = 0; i < indices_num; ++i )
			indices[i] += indices[i] == -1 ? 0 : vertices_num_total;
		g_array_append_vals( indices_arr, indices, indices_num );
		g_free( indices );

		/* create layout */
		layout = mf_layout_new_full( name, id, mode, indices_num, offset );
		g_free( name );
		g_list_store_append( self->layouts, G_OBJECT( layout ) );

		vertices_num_total += vertices_num;
		offset += indices_num * sizeof( guint );
	}

	self->vertices = g_bytes_new( vertices_arr->data, g_array_get_element_size( vertices_arr ) * vertices_arr->len );
	self->indices = g_bytes_new( indices_arr->data, g_array_get_element_size( indices_arr ) * indices_arr->len );
	self->is_loaded = TRUE;

	ret = TRUE;

out2:
	g_array_unref( vertices_arr );
	g_array_unref( indices_arr );
	g_strfreev( models );

out1:
	g_key_file_unref( key_file );
	g_bytes_unref( bytes );

	return ret;
}

gboolean
mf_model_asset_is_loaded(
	MfModelAsset *self )
{
	g_return_val_if_fail( MF_IS_MODEL_ASSET( self ), FALSE );

	return self->is_loaded;
}

GListStore*
mf_model_asset_get_layouts(
	MfModelAsset *self )
{
	g_return_val_if_fail( MF_IS_MODEL_ASSET( self ), NULL );

	return G_LIST_STORE( g_object_ref( G_OBJECT( self->layouts ) ) );
}

GBytes*
mf_model_asset_get_vertices(
	MfModelAsset *self )
{
	g_return_val_if_fail( MF_IS_MODEL_ASSET( self ), NULL );

	return g_bytes_ref( self->vertices );
}

GBytes*
mf_model_asset_get_indices(
	MfModelAsset *self )
{
	g_return_val_if_fail( MF_IS_MODEL_ASSET( self ), NULL );

	return g_bytes_ref( self->indices );
}

