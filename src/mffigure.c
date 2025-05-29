#include "mffigure.h"

#include "mflayout.h"

#include <glib-object.h>
#include <glib.h>
#include <graphene.h>
#include <graphene-gobject.h>

struct _MfFigure
{
	GObject parent_instance;

	graphene_vec4_t color;
	graphene_point_t position;
	graphene_point_t velocity;
	gboolean velocity_set;
	gfloat angle;
	gfloat angular_velocity;
	gboolean angular_velocity_set;
	gfloat scale;
	MfLayout *layout;
};
typedef struct _MfFigure MfFigure;

enum _MfFigurePropertyID
{
	PROP_0, /* 0 is reserved for GObject */

	PROP_COLOR,
	PROP_POSITION,
	PROP_VELOCITY,
	PROP_VELOCITY_SET,
	PROP_ANGLE,
	PROP_ANGULAR_VELOCITY,
	PROP_ANGULAR_VELOCITY_SET,
	PROP_SCALE,
	PROP_LAYOUT,

	N_PROPS
};
typedef enum _MfFigurePropertyID MfFigurePropertyID;

static GParamSpec *object_props[N_PROPS] = { NULL, };

G_DEFINE_TYPE( MfFigure, mf_figure, G_TYPE_OBJECT )

static void
mf_figure_init(
	MfFigure *self )
{
	graphene_vec4_init( &self->color, 0.0f, 0.0f, 0.0f, 1.0f );
	self->position = GRAPHENE_POINT_INIT( 0.0f, 0.0f );
	self->velocity = GRAPHENE_POINT_INIT( 0.0f, 0.0f );
	self->velocity_set = FALSE;
	self->angle = 0.0f;
	self->angular_velocity = 0.0f;
	self->angular_velocity_set = FALSE;
	self->scale = 1.0f;
	self->layout = NULL;
}

static void
mf_figure_finalize(
	GObject *object )
{
	G_OBJECT_CLASS( mf_figure_parent_class )->finalize( object );
}

static void
mf_figure_dispose(
	GObject *object )
{
	MfFigure *self = MF_FIGURE( object );

	g_clear_object( &self->layout );

	G_OBJECT_CLASS( mf_figure_parent_class )->dispose( object );
}

static void
mf_figure_get_property(
	GObject *object,	
	guint prop_id,	
	GValue *value,	
	GParamSpec *pspec )
{
	MfFigure *self = MF_FIGURE( object );

	switch( (MfFigurePropertyID)prop_id )
	{
		case PROP_COLOR:
			g_value_set_boxed( value, &self->color );
			break;
		case PROP_POSITION:
			g_value_set_boxed( value, &self->position );
			break;
		case PROP_VELOCITY:
			g_value_set_boxed( value, &self->velocity );
			break;
		case PROP_VELOCITY_SET:
			g_value_set_boolean( value, self->velocity_set );
			break;
		case PROP_ANGLE:
			g_value_set_float( value, self->angle );
			break;
		case PROP_ANGULAR_VELOCITY:
			g_value_set_float( value, self->angular_velocity );
			break;
		case PROP_ANGULAR_VELOCITY_SET:
			g_value_set_boolean( value, self->angular_velocity_set );
			break;
		case PROP_SCALE:
			g_value_set_float( value, self->scale );
			break;
		case PROP_LAYOUT:
			g_value_set_object( value, self->layout );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_figure_set_property(
	GObject *object,
	guint prop_id,
	const GValue *value,
	GParamSpec *pspec )
{
	MfFigure *self = MF_FIGURE( object );

	switch( (MfFigurePropertyID)prop_id )
	{
		case PROP_COLOR:
			mf_figure_set_color( self, g_value_get_boxed( value ) );
			break;
		case PROP_POSITION:
			mf_figure_set_position( self, g_value_get_boxed( value ) );
			break;
		case PROP_VELOCITY:
			mf_figure_set_velocity( self, g_value_get_boxed( value ) );
			break;
		case PROP_VELOCITY_SET:
			mf_figure_set_velocity_set( self, g_value_get_boolean( value ) );
			break;
		case PROP_ANGLE:
			mf_figure_set_angle( self, g_value_get_float( value ) );
			break;
		case PROP_ANGULAR_VELOCITY:
			mf_figure_set_angular_velocity( self, g_value_get_float( value ) );
			break;
		case PROP_ANGULAR_VELOCITY_SET:
			mf_figure_set_angular_velocity_set( self, g_value_get_boolean( value ) );
			break;
		case PROP_SCALE:
			mf_figure_set_scale( self, g_value_get_float( value ) );
			break;
		case PROP_LAYOUT:
			mf_figure_set_layout( self, g_value_get_object( value ) );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
			break;
	}
}

static void
mf_figure_class_init(
	MfFigureClass *klass )
{
	GObjectClass *object_class = G_OBJECT_CLASS( klass );

	object_class->finalize = mf_figure_finalize;
	object_class->dispose = mf_figure_dispose;
	object_class->get_property = mf_figure_get_property;
	object_class->set_property = mf_figure_set_property;
	object_props[PROP_COLOR] = g_param_spec_boxed(
		"color",
		"Color",
		"Color of the figure as graphene_vec4_t",
		GRAPHENE_TYPE_VEC4,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS );
	object_props[PROP_POSITION] = g_param_spec_boxed(
		"position",
		"Position",
		"Position of the figure as graphene_point_t",
		GRAPHENE_TYPE_POINT,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_VELOCITY] = g_param_spec_boxed(
		"velocity",
		"Velocity",
		"Velocity of the figure as graphene_point_t",
		GRAPHENE_TYPE_POINT,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_VELOCITY_SET] = g_param_spec_boolean(
		"velocity-set",
		"Velocity set",
		"if velocity is set",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_ANGLE] = g_param_spec_float(
		"angle",
		"Angle",
		"Initial angle of rotation",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0f,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_ANGULAR_VELOCITY] = g_param_spec_float(
		"angular-velocity",
		"Angular velocity",
		"Velocity of the rotation",
		-G_MAXFLOAT,
		G_MAXFLOAT,
		0.0f,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_ANGULAR_VELOCITY_SET] = g_param_spec_boolean(
		"angular-velocity-set",
		"Angular velocity set",
		"if angular velocity is set",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_SCALE] = g_param_spec_float(
		"scale",
		"Scale",
		"Scale of the figure",
		G_MINFLOAT,
		G_MAXFLOAT,
		1.0f,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	object_props[PROP_LAYOUT] = g_param_spec_object(
		"layout",
		"Layout",
		"Layout of the figure",
		MF_TYPE_LAYOUT,
		G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS );
	g_object_class_install_properties( object_class, N_PROPS, object_props );
}

static gfloat
adjust_angle(
	gfloat angle )
{
	/* adjust angle to [0.0;360.0] */
	if( angle < 0.0 )
		while( TRUE )
			if( ( angle += 360.0f ) > 0.0f )
				break;

	if( angle > 360.0f )
		while( TRUE )
			if( ( angle -= 360.0f ) < 360.0f )
				break;
	
	return angle;
}

MfFigure*
mf_figure_new(
	void )
{
	return MF_FIGURE( g_object_new( MF_TYPE_FIGURE, NULL ) );
}

MfFigure*
mf_figure_new_full(
	const graphene_vec4_t *color,
	const graphene_point_t *pos,
	const graphene_point_t *vel,
	gfloat angle,
	gfloat angular_velocity,
	gfloat scale,
	MfLayout *layout )
{
	g_return_val_if_fail( color != NULL, NULL );
	g_return_val_if_fail( pos != NULL, NULL );
	g_return_val_if_fail( vel != NULL, NULL );
	g_return_val_if_fail( scale >= 0.0f, NULL );
	g_return_val_if_fail( MF_IS_LAYOUT( layout ), NULL );

	return MF_FIGURE( g_object_new( MF_TYPE_FIGURE, "color", color, "position", pos, "velocity", vel, "angle", angle, "angular_velocity", angular_velocity, "scale", scale, "layout", layout, NULL ) );
}

void
mf_figure_get_color(
	MfFigure *self,
	graphene_vec4_t *color )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( color != NULL );

	graphene_vec4_init_from_vec4( color, &self->color );
}

void
mf_figure_get_colorv(
	MfFigure *self,
	gfloat *cv )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( cv != NULL );

	graphene_vec4_to_float( &self->color, cv );
}

void
mf_figure_set_color(
	MfFigure *self,
	const graphene_vec4_t *color )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( color != NULL );

	g_object_freeze_notify( G_OBJECT( self ) );

	graphene_vec4_init_from_vec4( &self->color, color );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_COLOR] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

void
mf_figure_get_position(
	MfFigure *self,
	graphene_point_t *pos )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( pos != NULL );

	graphene_point_init_from_point( pos, &self->position );
}

void
mf_figure_set_position(
	MfFigure *self,
	const graphene_point_t *pos )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( pos != NULL );

	g_object_freeze_notify( G_OBJECT( self ) );

	graphene_point_init_from_point( &self->position, pos );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_POSITION] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

void
mf_figure_get_velocity(
	MfFigure *self,
	graphene_point_t *vel )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( vel != NULL );

	graphene_point_init_from_point( vel, &self->velocity );
}

void
mf_figure_set_velocity(
	MfFigure *self,
	const graphene_point_t *vel )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( vel != NULL );

	g_object_freeze_notify( G_OBJECT( self ) );

	graphene_point_init_from_point( &self->velocity, vel );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_VELOCITY] );

	self->velocity_set = TRUE;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_VELOCITY_SET] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gboolean
mf_figure_get_velocity_set(
	MfFigure *self )
{
	g_return_val_if_fail( MF_IS_FIGURE( self ), FALSE );

	return self->velocity_set;
}

void
mf_figure_set_velocity_set(
	MfFigure *self,
	gboolean vel_set )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->velocity_set = vel_set;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_VELOCITY_SET] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gfloat
mf_figure_get_angle(
	MfFigure *self )
{
	g_return_val_if_fail( MF_IS_FIGURE( self ), 0.0f );

	return self->angle;
}

void
mf_figure_set_angle(
	MfFigure *self,
	gfloat angle )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->angle = adjust_angle( angle );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_ANGLE] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gfloat
mf_figure_get_angular_velocity(
	MfFigure *self )
{
	g_return_val_if_fail( MF_IS_FIGURE( self ), 0.0f );

	return self->angular_velocity;
}

void
mf_figure_set_angular_velocity(
	MfFigure *self,
	gfloat vel )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->angular_velocity = vel;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_ANGULAR_VELOCITY] );

	self->angular_velocity_set = TRUE;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_ANGULAR_VELOCITY_SET] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gboolean
mf_figure_get_angular_velocity_set(
	MfFigure *self )
{
	g_return_val_if_fail( MF_IS_FIGURE( self ), FALSE );

	return self->angular_velocity_set;
}

void
mf_figure_set_angular_velocity_set(
	MfFigure *self,
	gboolean vel_set )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->angular_velocity_set = vel_set;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_ANGULAR_VELOCITY_SET] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

gfloat
mf_figure_get_scale(
	MfFigure *self )
{
	g_return_val_if_fail( MF_IS_FIGURE( self ), 1.0f );

	return self->scale;
}

void
mf_figure_set_scale(
	MfFigure *self,
	gfloat scale )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( scale > 0.0 );

	g_object_freeze_notify( G_OBJECT( self ) );

	self->scale = scale;
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_SCALE] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

MfLayout*
mf_figure_get_layout(
	MfFigure *self )
{
	g_return_val_if_fail( MF_IS_FIGURE( self ), NULL );

	return MF_LAYOUT( g_object_ref( G_OBJECT( self->layout ) ) );
}

void
mf_figure_set_layout(
	MfFigure *self,
	MfLayout *layout )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( MF_IS_LAYOUT( layout ) );

	g_object_freeze_notify( G_OBJECT( self ) );

	g_clear_object( &self->layout );
	self->layout = MF_LAYOUT( g_object_ref( G_OBJECT( layout ) ) );
	g_object_notify_by_pspec( G_OBJECT( self ), object_props[PROP_LAYOUT] );

	g_object_thaw_notify( G_OBJECT( self ) );
}

void
mf_figure_get_srtm(
	MfFigure *self,
	graphene_matrix_t *srtm )
{
	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( srtm != NULL );

	graphene_matrix_init_identity( srtm );
	graphene_matrix_scale( srtm, self->scale, self->scale, 1.0f );
	graphene_matrix_rotate_z( srtm, self->angle );
	graphene_matrix_translate( srtm, &GRAPHENE_POINT3D_INIT( self->position.x, self->position.y, 0.0f ) );
}

gboolean
mf_figure_is_inside_rectangle(
	MfFigure *self,
	const graphene_rect_t *rect )
{
	g_return_val_if_fail( MF_IS_FIGURE( self ), FALSE );
	g_return_val_if_fail( rect != NULL, FALSE );

	return graphene_rect_contains_point( rect, &self->position );
}

void
mf_figure_move(
	MfFigure *self,
	const graphene_rect_t *rect,
	gfloat fps )
{
	graphene_point_t v, tl, br;

	g_return_if_fail( MF_IS_FIGURE( self ) );
	g_return_if_fail( rect != NULL );
	g_return_if_fail( fps > 0.0 );

	/* if the figure is outside the rectangle, nothing to do */
	if( !mf_figure_is_inside_rectangle( self, rect ) )
		return;

	/* moving */
	if( self->velocity_set )
	{
		graphene_rect_get_top_left( rect, &tl );
		graphene_rect_get_bottom_right( rect, &br );
		v = GRAPHENE_POINT_INIT( self->velocity.x / fps, self->velocity.y / fps );

		if( self->position.x + v.x > br.x || self->position.x + v.x < tl.x )
			self->velocity.x = -self->velocity.x;
		if( self->position.y + v.y > br.y || self->position.y + v.y < tl.y )
			self->velocity.y = -self->velocity.y;

		self->position.x += self->velocity.x / fps;
		self->position.y += self->velocity.y / fps;
	}

	/* rotating */
	if(  self->angular_velocity_set )
		self->angle = adjust_angle( self->angle + self->angular_velocity / fps );
}

