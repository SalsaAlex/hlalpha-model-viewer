//=============================================================================
//
// Copyright (c) 1998, Valve LLC. All rights reserved.
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// This product contains software technology licensed from Id
// Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
// All Rights Reserved.
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: Routines for drawing Half-Life 3DStudio models
//
//=============================================================================

#ifndef STUDIORENDER_H
#define STUDIORENDER_H

#include "StudioModel.h"

//=============================================================================
//
//=============================================================================
class StudioRender : public StudioModel
{
public:
	studiohdr_t	*	getTextureHeader() const { return m_ptexturehdr; }

	void			UploadTexture( mstudiotexture_t * ptexture,
						byte * data, byte * pal, int texindex = -1 );
	void			UploadTextures();
	void			UnloadTextures();
	void			DrawModel( void );
	void			AdvanceFrame( float dt );
	int				SetFrame( int nFrame );

	void			CalcBbox( int nSeqence, vec3_t bmin, vec3_t bmax );

protected:
	void			CalcBoneAdj( void );
	void			CalcBoneQuaternion( int frame, float s, int nBoneId,
						mstudiobone_t * pbone, mstudioanim_t * panim,
						vec4_t q );
	void			CalcBonePosition( int frame, float s, int nBoneId,
						mstudiobone_t * pbone, mstudioanim_t * panim,
						vec3_t pos );
	void			CalcBoneTransformation( vec3_t pos, vec4_t q,
						mstudioseqdesc_t * pseqdesc,
						mstudioanim_t * panim, mstudiobone_t * pbone,
						int nBoneId, float f );
	void			SetUpBones( void );

	void			DrawPoints( void );

	void			Lighting( float * lv, int bone, int flags, vec3_t normal );
	void			Chrome( int * chrome, int bone, vec3_t normal );

	void			SetupLighting( void );

	void			SetupModel( int bodypart );

protected:
	// entity settings
	vec3_t			m_origin;
	vec3_t			m_angles;
	float			m_frame;

	studiohdr_t	*	m_ptexturehdr;

	// FIX: non persistant, make static
	vec4_t			m_adj;				

};

extern StudioRender g_studioRender;

extern vec3_t g_vright;		// needs to be set to viewer's right in order for chrome to work
extern float g_lambert;		// modifier for pseudo-hemispherical lighting

#endif // STUDIORENDER_H
