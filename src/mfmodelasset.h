#ifndef MFMODELASSET_H
#define MFMODELASSET_H

#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

enum _MfModelAssetError
{
	MF_MODEL_ASSET_ERROR_VERTEX_ARRAY_LENGTH_IS_NOT_EVEN,
	MF_MODEL_ASSET_ERROR_INDEX_ARRAY_CONTAINS_NEGATIVE_INTEGER_LESS_MINUS_ONE,
	MF_MODEL_ASSET_ERROR_INDEX_ARRAY_CONTAINS_INDEX_OUT_OF_VERTEX_NUMBER,
	MF_MODEL_ASSET_ERROR_UNKNOWN_MODE,
	MF_MODEL_ASSET_ERROR_ALREADY_LOADED,

	N_MF_MODEL_ASSET_ERROR
};
typedef enum _MfModelAssetError MfModelAssetError;

#define MF_MODEL_ASSET_ERROR ( mf_model_asset_error_quark() )
GQuark mf_model_asset_error_quark( void ) G_GNUC_CONST;

#define MF_TYPE_MODEL_ASSET ( mf_model_asset_get_type() )
G_DECLARE_FINAL_TYPE( MfModelAsset, mf_model_asset, MF, MODEL_ASSET, GObject )

MfModelAsset* mf_model_asset_new( void );
MfModelAsset* mf_model_asset_new_from_bytes( GBytes *bytes, GError **error );
gboolean mf_model_asset_load_from_bytes( MfModelAsset *self, GBytes *bytes, GError **error );
gboolean mf_model_asset_is_loaded( MfModelAsset *self );
GListStore* mf_model_asset_get_layouts( MfModelAsset *self );
GBytes* mf_model_asset_get_vertices( MfModelAsset *self );
GBytes* mf_model_asset_get_indices( MfModelAsset *self );

G_END_DECLS

#endif
