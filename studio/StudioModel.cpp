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
// Purpose:
//
//=============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mx/gl.h>
#include <mx/mxBmp.h>

#include <GL/glu.h>

#include "StudioModel.h"

#include "safestr.h"
#include "mdlmath.h"


#pragma warning( disable : 4244 ) // double to float

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
/*StudioModel::StudioModel( void )
{
	m_pstudiohdr = NULL;
	m_pmodel = NULL;

	memset( m_panimhdr, 0, sizeof( m_panimhdr ) );
}*/

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool StudioModel::WriteBitmap( int index, char * filename )
{
	char name[256] = { 0 };

	if ( filename && filename[ 0 ] )
	{
		strlcpy( name, filename, sizeof( name ) );
	}

	studiohdr_t * phdr = m_pstudiohdr;

	if ( !phdr )
	{
		return false;
	}

	mstudiotexture_t * ptexture = ( mstudiotexture_t * )( ( byte * ) phdr + phdr->textureindex ) + index;
	if ( ptexture->name[ 0 ] == 0 )
	{
		return false;
	}

	if ( !name[ 0 ] )
	{
		strlcpy( name, ptexture->name, sizeof( name ) );
	}

	// TOMAS: HACK: mxBmpWrite is unsafe, so let's check if we can
	// open dest. file for writing
	FILE * pFile = fopen( name, "w" );
	if ( !pFile )
	{
		return false;
	}
	fclose( pFile );

	mxImage image;
	image.width = ptexture->width;
	image.height = ptexture->height;
	image.bpp = 8;
	image.data = ( void * )( ( byte * ) phdr + ptexture->index );
	image.palette = ( void * )( ( byte * ) phdr + ptexture->width * ptexture->height + ptexture->index );

	if ( !mxBmpWrite( name, &image ) )
	{
		return false;
	}

	image.data = 0;
	image.palette = 0;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioModel::FreeModel( void )
{
	if ( m_pstudiohdr )
		free( m_pstudiohdr );

	int i;
	for ( i = 0; i < 32; i++ )
	{
		if ( m_panimhdr[i] )
		{
			free( m_panimhdr[i] );
			m_panimhdr[i] = 0;
		}
	}

	m_pstudiohdr = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
studiohdr_t * StudioModel::LoadModel( char * modelname )
{
	FILE * fp;
	long size;
	void * buffer;

	if ( !modelname )
		return 0;

	// load the model
	if ( ( fp = fopen( modelname, "rb" ) ) == NULL )
		return 0;

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	buffer = malloc( size );
	if ( !buffer )
	{
		fclose( fp );
		return 0;
	}

	fread( buffer, size, 1, fp );
	fclose( fp );

	byte * pin = ( byte * )buffer;
	studiohdr_t * phdr = ( studiohdr_t * )pin;

	// TOMAS:
	// IDPO 6 - Quake model
	// IDST 4 - Pre Alpha Leak HL model
	// IDST 6 - Alpha Leak HL model (<-supported in this version)
	// IDST 10 - Retail HL model
	// IDSQ 10 - Retail HL model sequence

	if ( phdr->id != IDSTUDIOHEADER )
	{
		free( buffer );
		return NULL;
	}

	if ( phdr->version != STUDIO_VERSION )
	{
		free( buffer );
		return NULL;
	}

	if ( !m_pstudiohdr )
	{
		m_pstudiohdr = ( studiohdr_t * )buffer;
	}

	return ( studiohdr_t * )buffer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool StudioModel::PostLoadModel( char * modelname )
{
	SetSequence( 0 );
	
	for ( int i = 0; i < m_pstudiohdr->numbonecontrollers; i++ )
	{
		SetController( i, 0.0f );
	}

	for ( int i = 0; i < m_pstudiohdr->numbodyparts; i++ )
	{
		SetBodygroup( i, 0 );
	}
	SetSkin( 0 );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool StudioModel::SaveModel( char * modelname )
{
	if ( !modelname )
		return false;

	if ( !m_pstudiohdr )
		return false;

	FILE * file;

	file = fopen( modelname, "wb" );
	if ( !file )
		return false;

	fwrite( m_pstudiohdr, sizeof( byte ), m_pstudiohdr->length, file );
	fclose( file );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
mstudioanim_t * StudioModel::GetAnim( mstudioseqdesc_t * pseqdesc )
{
	return ( mstudioanim_t * )( ( byte * )m_pstudiohdr + pseqdesc->animindex );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int StudioModel::GetSequence( void )
{
	return m_sequence;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int StudioModel::SetSequence( int iSequence )
{
	if ( iSequence > m_pstudiohdr->numseq )
		return m_sequence;

	m_sequence = iSequence;
	m_frame = 0;

	return m_sequence;
}

// TOMAS: UNUSED
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
/*void StudioModel::GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed )
{
	mstudioseqdesc_t	*pseqdesc;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + (int)m_sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}*/

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float StudioModel::SetController( int nCtrlId, float flValue )
{
	if ( !m_pstudiohdr )
		return 0.0f;

	if ( nCtrlId >= m_pstudiohdr->numbonecontrollers )
		return 0.0f;

	mstudiobonecontroller_t	* pbonecontroller = ( mstudiobonecontroller_t * )
		( ( byte * )m_pstudiohdr + m_pstudiohdr->bonecontrollerindex ) +
		nCtrlId;

	float fStart = pbonecontroller->end;
	float fEnd = pbonecontroller->start;

	if ( fStart > fEnd )
	{
		float fTemp = fStart;
		fStart = fEnd;
		fEnd = fTemp;
	}

	// wrap 0..360 if it's a rotational controller
	if ( pbonecontroller->type & ( STUDIO_XR | STUDIO_YR | STUDIO_ZR ) )
	{
		// does the controller not wrap?
		if ( fStart + 359.0 >= fEnd )
		{
			if ( flValue > ( ( fStart + fEnd ) / 2.0 ) + 180 )
				flValue = flValue - 360;
			if ( flValue < ( ( fStart + fEnd ) / 2.0 ) - 180 )
				flValue = flValue + 360;
		}
		else
		{
			if ( flValue > 360 )
				flValue = flValue - ( int )( flValue / 360.0 ) * 360.0;
			else if ( flValue < 0 )
				flValue = flValue + ( int )( ( flValue / -360.0 ) + 1 ) * 360.0;
		}
	}

	int setting = ( int )( 255 * ( flValue - fStart ) / ( fEnd - fStart ) );
	BOUND( 0, setting, 255 );

	m_controller[ nCtrlId ] = setting;

	return setting * ( 1.0 / 255.0 ) * ( fEnd - fStart ) + fStart;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int StudioModel::SetBodygroup( int iGroup, int iValue )
{
	if ( !m_pstudiohdr )
		return 0;

	if ( iGroup > m_pstudiohdr->numbodyparts )
		return -1;

	mstudiobodyparts_t * pbodypart = ( mstudiobodyparts_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->bodypartindex ) + iGroup;

	int iCurrent = ( m_bodynum / pbodypart->base ) % pbodypart->nummodels;

	if ( iValue >= pbodypart->nummodels )
		return iCurrent;

	m_bodynum = ( m_bodynum - ( iCurrent * pbodypart->base ) + ( iValue * pbodypart->base ) );

	return iValue;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int StudioModel::SetSkin( int iValue )
{
	if ( !m_pstudiohdr )
		return 0;

	if ( iValue >= m_pstudiohdr->numskinfamilies )
	{
		return m_skinnum;
	}

	m_skinnum = iValue;

	return iValue;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioModel::SlerpBones( vec4_t q1[], vec3_t pos1[], vec4_t q2[],
	vec3_t pos2[], float s )
{
	int			i;
	vec4_t		q3;
	float		s1;

	if ( s < 0 ) s = 0;
	else if ( s > 1.0 ) s = 1.0;

	s1 = 1.0 - s;

	for ( i = 0; i < m_pstudiohdr->numbones; i++ )
	{
		QuaternionSlerp( q1[i], q2[i], s, q3 );
		q1[i][0] = q3[0];
		q1[i][1] = q3[1];
		q1[i][2] = q3[2];
		q1[i][3] = q3[3];
		pos1[i][0] = pos1[i][0] * s1 + pos2[i][0] * s;
		pos1[i][1] = pos1[i][1] * s1 + pos2[i][1] * s;
		pos1[i][2] = pos1[i][2] * s1 + pos2[i][2] * s;
	}
}
