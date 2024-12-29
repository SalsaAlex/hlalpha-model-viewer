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
// Purpose: Studio model (no render stuff here, no GL stuff here)
//
//=============================================================================

#ifndef STUDIOMODEL_H
#define STUDIOMODEL_H

#ifndef byte
typedef unsigned char byte;
#endif // byte

#include "mathlib.h"
#include "studio.h"

//=============================================================================
//
//=============================================================================
class StudioModel
{
public:
	//StudioModel( void );

	studiohdr_t	*	getStudioHeader() { return m_pstudiohdr; }
	studiohdr_t	*	getAnimHeader( int i ) { return m_panimhdr[i]; }

	bool			WriteBitmap( int index, char * filename );
	void					FreeModel();
	studiohdr_t	*	LoadModel( char * modelname );
	bool			PostLoadModel( char * modelname );
	bool			SaveModel( char * modelname );

	int				SetSequence( int iSequence );
	int				GetSequence( void );
	void			GetSequenceInfo( float * pflFrameRate, float * pflGroundSpeed );

	mstudioanim_t *	GetAnim( mstudioseqdesc_t * pseqdesc );

	float			SetController( int nCtrlId, float flValue );
	int				SetBodygroup( int iGroup, int iValue );
	int				SetSkin( int iValue );

	void			SlerpBones( vec4_t q1[], vec3_t pos1[], vec4_t q2[],
						vec3_t pos2[], float s );

protected:
	int				m_sequence;			// sequence index
	float			m_frame;			// frame
	int				m_bodynum;			// bodypart selection
	int				m_skinnum;			// skin group selection
	byte			m_controller[ MAXSTUDIOCONTROLLERS ];	// bone controllers

	// internal data
	studiohdr_t	*	m_pstudiohdr;
	mstudiomodel_t*	m_pmodel;

	studiohdr_t	*	m_panimhdr[32];
};

#endif // STUDIOMODEL_H
