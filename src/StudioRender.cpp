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

#include "StudioRender.h"

#include <mx/gl.h>
#include <GL/glu.h>

#include "ViewerSettings.h"

#include "mdlmath.h"

#pragma warning( disable : 4244 ) // double to float

StudioRender g_studioRender;

//=============================================================================

vec3_t			g_xformverts[MAXSTUDIOVERTS];	// transformed vertices
vec3_t			g_lightvalues[MAXSTUDIOVERTS];	// light surface normals
vec3_t		*	g_pxformverts;
vec3_t		*	g_pvlightvalues;

vec3_t			g_lightvec;						// light vector in model reference frame
vec3_t			g_blightvec[MAXSTUDIOBONES];	// light vectors in bone reference frames
int				g_ambientlight;					// ambient world light
float			g_shadelight;					// direct world light
vec3_t			g_lightcolor;

int				g_smodels_total;				// cookie

float			g_bonetransform[MAXSTUDIOBONES][3][4];	// bone transformation matrix

int				g_chrome[MAXSTUDIOVERTS][2];	// texture coords for surface normals
int				g_chromeage[MAXSTUDIOBONES];	// last time chrome vectors were updated
vec3_t			g_chromeup[MAXSTUDIOBONES];		// chrome vector "up" in bone reference frames
vec3_t			g_chromeright[MAXSTUDIOBONES];	// chrome vector "right" in bone reference frames

// TOMAS: UNDONE
bool bFilterTextures = true;


//=============================================================================

static int g_texnum = 3;

//Mugsy - upped the maximum texture size to 512. All changes are the replacement of '256'
//with this define, MAX_TEXTURE_DIMS
#define MAX_TEXTURE_DIMS 512

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::UploadTexture( mstudiotexture_t * ptexture, byte * data,
	byte * pal, int texindex /*= -1*/ )
{
	if ( texindex < 0 )
	{
		texindex = g_texnum++;
	}

	
	int	i, j;
	int	row1[MAX_TEXTURE_DIMS], row2[MAX_TEXTURE_DIMS],
		col1[MAX_TEXTURE_DIMS], col2[MAX_TEXTURE_DIMS];
	byte * pix1, *pix2, *pix3, *pix4;
	byte * tex, *out;

	// convert texture to power of 2
	int outwidth;
	for ( outwidth = 1; outwidth < ptexture->width; outwidth <<= 1 )
		;

	if ( outwidth > MAX_TEXTURE_DIMS )
		outwidth = MAX_TEXTURE_DIMS;

	int outheight;
	for ( outheight = 1; outheight < ptexture->height; outheight <<= 1 )
		;

	if ( outheight > MAX_TEXTURE_DIMS )
		outheight = MAX_TEXTURE_DIMS;

	tex = out = ( byte * )malloc( outwidth * outheight * 4 );
	if ( !out )
	{
		return;
	}

	for ( i = 0; i < outwidth; i++ )
	{
		col1[i] = ( int )( ( i + 0.25 ) *
			( ptexture->width / ( float )outwidth ) );
		col2[i] = ( int )( ( i + 0.75 ) *
			( ptexture->width / ( float )outwidth ) );
	}

	for ( i = 0; i < outheight; i++ )
	{
		row1[i] = ( int )( ( i + 0.25 ) *
			( ptexture->height / ( float )outheight ) ) * ptexture->width;
		row2[i] = ( int )( ( i + 0.75 ) *
			( ptexture->height / ( float )outheight ) ) * ptexture->width;
	}

	// scale down and convert to 32bit RGB
	for ( i = 0 ; i < outheight ; i++ )
	{
		for ( j = 0 ; j < outwidth ; j++, out += 4 )
		{
			pix1 = &pal[data[row1[i] + col1[j]] * 3];
			pix2 = &pal[data[row1[i] + col2[j]] * 3];
			pix3 = &pal[data[row2[i] + col1[j]] * 3];
			pix4 = &pal[data[row2[i] + col2[j]] * 3];

			out[0] = ( pix1[0] + pix2[0] + pix3[0] + pix4[0] ) >> 2;
			out[1] = ( pix1[1] + pix2[1] + pix3[1] + pix4[1] ) >> 2;
			out[2] = ( pix1[2] + pix2[2] + pix3[2] + pix4[2] ) >> 2;
			out[3] = 0xFF;
		}
	}

	glBindTexture( GL_TEXTURE_2D, texindex );
	glTexImage2D( GL_TEXTURE_2D, 0, 3/*??*/, outwidth, outheight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, tex );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		bFilterTextures ? GL_LINEAR : GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		bFilterTextures ? GL_LINEAR : GL_NEAREST );

	// ptexture->width = outwidth;
	// ptexture->height = outheight;
	//ptexture->index = name; //g_texnum;

	free( tex );

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::UnloadTextures( void )
{
	// deleting textures
	g_texnum -= 3;
	int textures[MAXSTUDIOSKINS];
	for ( int i = 0; i < g_texnum; i++ )
		textures[i] = i + 3;

	glDeleteTextures( g_texnum, ( const GLuint * ) textures );

	g_texnum = 3;

	m_ptexturehdr = 0;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::UploadTextures( void )
{
	studiohdr_t * phdr = ( studiohdr_t * )m_pstudiohdr;
	byte * pin = ( byte * )phdr;
	mstudiotexture_t * ptexture =
		( mstudiotexture_t * )( pin + phdr->textureindex );

	// Upload textures
	if ( phdr->textureindex > 0 && phdr->numtextures <= MAXSTUDIOSKINS )
	{
		int n = phdr->numtextures;
		for ( int i = 0; i < n; i++ )
		{
			// strcpy( name, mod->name );
			// strcpy( name, ptexture[i].name );
			UploadTexture( &ptexture[i], pin + ptexture[i].index,
				pin + ptexture[i].width * ptexture[i].height +
				ptexture[i].index );
		}
	}

	// UNDONE: free texture memory

	// TOMAS: UNDONE
	// preload textures
	/*if (m_pstudiohdr->numtextures == 0)
	{
		// TOMAS: should we really fail?
		// No textures
		return false;
	}*/

	m_ptexturehdr = phdr;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::AdvanceFrame( float dt )
{
	if ( !m_pstudiohdr )
		return;

	mstudioseqdesc_t * pseqdesc;
	pseqdesc = ( mstudioseqdesc_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	if ( dt > 0.1 )
		dt = 0.1f;
	m_frame += dt * pseqdesc->fps;

	if ( pseqdesc->numframes <= 1 )
	{
		m_frame = 0;
	}
	else
	{
		// wrap
		m_frame -= ( int )( m_frame / ( pseqdesc->numframes - 1 ) ) * ( pseqdesc->numframes - 1 );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int StudioRender::SetFrame( int nFrame )
{
	if ( nFrame == -1 )
		return m_frame;

	if ( !m_pstudiohdr )
		return 0;

	mstudioseqdesc_t * pseqdesc;
	pseqdesc = ( mstudioseqdesc_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	m_frame = nFrame;

	if ( pseqdesc->numframes <= 1 )
	{
		m_frame = 0;
	}
	else
	{
		// wrap
		m_frame -= ( int )( m_frame / ( pseqdesc->numframes - 1 ) ) * ( pseqdesc->numframes - 1 );
	}

	return m_frame;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::CalcBoneAdj( void )
{
	float value;

	mstudiobonecontroller_t * pbonecontroller;
	pbonecontroller = ( mstudiobonecontroller_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->bonecontrollerindex );

	for ( int j = 0; j < m_pstudiohdr->numbonecontrollers; j++ )
	{
		// check for 360deg wrapping
		if ( pbonecontroller[j].type & STUDIO_RLOOP )
		{
			value = m_controller[j] * ( 360.0 / 256.0 ) + pbonecontroller[j].start;
		}
		else
		{
			value = m_controller[j] / 255.0;
			if ( value < 0 ) value = 0;
			if ( value > 1.0 ) value = 1.0;
			value = ( 1.0 - value ) * pbonecontroller[j].start + value * pbonecontroller[j].end;
		}
		// Con_DPrintf( "%d %d %f : %f\n", m_controller[j], m_prevcontroller[j], value, dadt );

		switch ( pbonecontroller[j].type & STUDIO_TYPES )
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			m_adj[j] = value * ( Q_PI / 180.0 );
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			m_adj[j] = value;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// TOMAS: MDL v6:
//-----------------------------------------------------------------------------
void StudioRender::CalcBoneQuaternion( int frame, float s, int nBoneId,
	mstudiobone_t * pbone, mstudioanim_t * panim, vec4_t q )
{
	mstudioseqdesc_t * pseqdesc = ( mstudioseqdesc_t * )
		( ( byte * )m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	int nextframe = frame + 1;
	if ( nextframe >= ( pseqdesc->numframes ) )
	{
		nextframe = pseqdesc->flags & STUDIO_LOOPING ? 0 : frame;
	}

	vec3_t anglecur, angleprev;

	mstudiobonerot_t * pbonerot = ( mstudiobonerot_t * )
		( ( byte * )m_pstudiohdr + panim->rotindex );

	// TOMAS: This is not nice nor effective, but it works...

	// Find index of current frame (or nearest "lower")
	int angleindex = 0;
	for ( int i = 0; i < panim->numrot; i++ )
	{
		if ( pbonerot[ i ].frame > nextframe )
		{
			angleindex = i;
			break;
		}

		if ( pbonerot[ i ].frame != nextframe )
		{
			continue;
		}

		angleindex = i;
		break;
	}

	int previndex = MAX( 0, angleindex - 1 );

	VectorCopy( pbonerot[ angleindex ].angle, anglecur );
	VectorScalMul( anglecur, STUDIO_TO_RAD );

	VectorCopy( pbonerot[ previndex ].angle, angleprev );
	VectorScalMul( angleprev, STUDIO_TO_RAD );

	
	studiohdr_t * phdr = ( studiohdr_t * )m_pstudiohdr;
	byte * pin = ( byte * )phdr;
	mstudiobonecontroller_t * pcontroller =
		( mstudiobonecontroller_t * )( pin + phdr->bonecontrollerindex );
	for ( int i = 0; i < phdr->numbonecontrollers; i++, pcontroller++ )
	{
		if ( nBoneId != pcontroller->bone )
			continue;

		switch ( pcontroller->type & STUDIO_TYPES )
		{
		case STUDIO_XR:
			anglecur[ 0 ] += m_adj[ i ];
			angleprev[ 0 ] += m_adj[ i ];
			break;
		case STUDIO_YR:
			anglecur[ 1 ] += m_adj[ i ];
			angleprev[ 1 ] += m_adj[ i ];
			break;
		case STUDIO_ZR:
			anglecur[ 2 ] += m_adj[ i ];
			angleprev[ 2 ] += m_adj[ i ];
			break;
		default:
			break;
		}
	}
	/*if (pbone->bonecontroller[j+3] != -1)
	{
		angle1[j] += m_adj[pbone->bonecontroller[j+3]];
		angle2[j] += m_adj[pbone->bonecontroller[j+3]];
	}*/

	vec4_t q1, q2;
	if ( !VectorCompare( anglecur, angleprev ) )
	{
		AngleQuaternion( anglecur, q1 );
		AngleQuaternion( angleprev, q2 );
		QuaternionSlerp( q2, q1, s, q );
	}
	else
	{
		AngleQuaternion( anglecur, q );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// TOMAS: MDL v6:
//-----------------------------------------------------------------------------
void StudioRender::CalcBonePosition( int frame, float s, int nBoneId, mstudiobone_t * pbone,
	mstudioanim_t * panim, vec3_t pos )
{
	mstudioseqdesc_t * pseqdesc = ( mstudioseqdesc_t * )
		( ( byte * )m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	int nextframe = frame + 1;
	if ( nextframe >= ( pseqdesc->numframes ) )
	{
		nextframe = ( ( pseqdesc->flags & STUDIO_LOOPING ) != 0 ) ? 0 : frame;
	}

	mstudiobonepos_t * pbonepos = ( mstudiobonepos_t * )
		( ( byte * )m_pstudiohdr + panim->posindex );

	// TOMAS: This is not nice nor effective, but it works...

	int posindex = 0;
	for ( int i = 0; i < panim->numpos; i++ )
	{
		if ( pbonepos[ i ].frame > nextframe )
		{
			posindex = i;
			break;
		}

		if ( pbonepos[ i ].frame != nextframe )
		{
			continue;
		}

		posindex = i;
		break;
	}

	int previndex = MAX( 0, posindex - 1 );


	vec3_t posprev;
	VectorCopy( pbonepos[ previndex ].pos, posprev );

	vec3_t poscur;
	VectorCopy( pbonepos[ posindex ].pos, poscur );

	studiohdr_t * phdr = ( studiohdr_t * )m_pstudiohdr;
	byte * pin = ( byte * )phdr;
	mstudiobonecontroller_t * pcontroller =
		( mstudiobonecontroller_t * )( pin + phdr->bonecontrollerindex );
	for ( int i = 0; i < phdr->numbonecontrollers; i++, pcontroller++ )
	{
		if ( nBoneId != pcontroller->bone )
			continue;

		switch ( pcontroller->type & STUDIO_TYPES )
		{
		case STUDIO_X:
			poscur[ 0 ] += m_adj[ i ];
			posprev[ 0 ] += m_adj[ i ];
			break;
		case STUDIO_Y:
			poscur[ 1 ] += m_adj[ i ];
			posprev[ 1 ] += m_adj[ i ];
			break;
		case STUDIO_Z:
			poscur[ 2 ] += m_adj[ i ];
			posprev[ 2 ] += m_adj[ i ];
			break;
		default:
			break;
		}
	}

	pos[ 0 ] = poscur[ 0 ] * ( s ) + posprev[ 0 ] * ( 1.0f - s );
	pos[ 1 ] = poscur[ 1 ] * ( s ) + posprev[ 1 ] * ( 1.0f - s );
	pos[ 2 ] = poscur[ 2 ] * ( s ) + posprev[ 2 ] * ( 1.0f - s );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::CalcBoneTransformation( vec3_t pos, vec4_t q,
	mstudioseqdesc_t * pseqdesc, mstudioanim_t * panim, mstudiobone_t * pbone,
	int nBoneId, float f )
{
	int frame = ( int )f;
	float s = ( f - frame );

	CalcBoneQuaternion( frame, s, nBoneId, pbone, panim, q );
	CalcBonePosition( frame, s, nBoneId, pbone, panim, pos );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::SetUpBones( void )
{
	int					i;

	mstudiobone_t	*	pbones;
	mstudioseqdesc_t	* pseqdesc;
	mstudioanim_t	*	panim;

	vec3_t				pos;
	float				bonematrix[3][4];
	vec4_t				q;

	if ( m_sequence >= m_pstudiohdr->numseq )
	{
		m_sequence = 0;
	}

	pseqdesc = ( mstudioseqdesc_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;
	panim = GetAnim( pseqdesc );

	// add in programatic controllers
	CalcBoneAdj();


	pbones = ( mstudiobone_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->boneindex );

	for ( i = 0; i < m_pstudiohdr->numbones; i++, pbones++, panim++ )
	{
		CalcBoneTransformation( pos, q, pseqdesc, panim, pbones, i, m_frame );

		if ( i == pseqdesc->motionbone )
		{
			if ( pseqdesc->motiontype & STUDIO_X )
				pos[0] = 0.0;
			if ( pseqdesc->motiontype & STUDIO_Y )
				pos[1] = 0.0;
			if ( pseqdesc->motiontype & STUDIO_Z )
				pos[2] = 0.0;
		}


		QuaternionMatrix( q, bonematrix );

		bonematrix[0][3] = pos[0];
		bonematrix[1][3] = pos[1];
		bonematrix[2][3] = pos[2];

		if ( pbones->parent == -1 )
		{
			memcpy( g_bonetransform[i], bonematrix, sizeof( float ) * 12 );
		}
		else
		{
			R_ConcatTransforms( g_bonetransform[pbones->parent], bonematrix, g_bonetransform[i] );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::CalcBbox( int nSeqence, vec3_t bmin, vec3_t bmax )
{
	int					i, j, k, n;

	mstudiobone_t	*	pbones;
	mstudioseqdesc_t	* pseqdesc;
	mstudioanim_t	*	panim;

	static vec3_t		pos;
	float				bonematrix[3][4];
	static vec4_t		q;

	float bonetransform[MAXSTUDIOBONES][3][4];	// bone transformation matrix

	if ( nSeqence >= m_pstudiohdr->numseq )
	{
		nSeqence = 0;
	}

	//vec3_t bmin, bmax;

	// find intersection box volume for each bone
	for ( j = 0; j < 3; j++ )
	{
		bmin[j] = 9999.0;
		bmax[j] = -9999.0;
	}

	pseqdesc = ( mstudioseqdesc_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->seqindex ) + nSeqence;


	// add in programatic controllers
	CalcBoneAdj();


	for ( n = 0; n < pseqdesc->numframes; n++ )
	{
		pbones = ( mstudiobone_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->boneindex );
		panim = GetAnim( pseqdesc );

		for ( i = 0; i < m_pstudiohdr->numbones; i++, pbones++, panim++ )
		{
			CalcBoneTransformation( pos, q, pseqdesc, panim, pbones, i, n );

			if ( i == pseqdesc->motionbone )
			{
				if ( pseqdesc->motiontype & STUDIO_X )
					pos[0] = 0.0;
				if ( pseqdesc->motiontype & STUDIO_Y )
					pos[1] = 0.0;
				if ( pseqdesc->motiontype & STUDIO_Z )
					pos[2] = 0.0;
			}


			QuaternionMatrix( q, bonematrix );

			bonematrix[0][3] = pos[0];
			bonematrix[1][3] = pos[1];
			bonematrix[2][3] = pos[2];

			if ( pbones->parent == -1 )
			{
				memcpy( bonetransform[i], bonematrix, sizeof( float ) * 12 );
			}
			else
			{
				R_ConcatTransforms( bonetransform[pbones->parent], bonematrix, bonetransform[i] );
			}
		}

		mstudiobodyparts_t * pbodypart = ( mstudiobodyparts_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->bodypartindex );
		for ( int l = 0; l < m_pstudiohdr->numbodyparts; l++, pbodypart++ )
		{
			mstudiomodel_t * pmodel = ( mstudiomodel_t * )( ( byte * )m_pstudiohdr + pbodypart->modelindex );
			for ( k = 0; k < pbodypart->nummodels; k++, pmodel++ )
			{
				mstudiomodeldata_t	* pmodeldata;
				pmodeldata = ( mstudiomodeldata_t * )( ( byte * )m_pstudiohdr + pmodel->modeldataindex );

				vec3_t		*		pstudioverts;
				pstudioverts = ( vec3_t * )( ( byte * )m_pstudiohdr + pmodeldata->vertindex );

				byte * pvertbone = ( ( byte * )m_pstudiohdr + pmodel->vertinfoindex );

				for ( j = 0; j < pmodel->numverts; j++ )
				{
					VectorTransform( pstudioverts[j], bonetransform[pvertbone[j]], pos );

					if ( pos[0] < bmin[0] ) bmin[0] = pos[0];
					if ( pos[1] < bmin[1] ) bmin[1] = pos[1];
					if ( pos[2] < bmin[2] ) bmin[2] = pos[2];
					if ( pos[0] > bmax[0] ) bmax[0] = pos[0];
					if ( pos[1] > bmax[1] ) bmax[1] = pos[1];
					if ( pos[2] > bmax[2] ) bmax[2] = pos[2];
				}
			}
		}

	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::Lighting( float * lv, int bone, int flags, vec3_t normal )
{
	float 	illum;
	float	lightcos;

	illum = g_ambientlight;

	if ( flags & STUDIO_NF_FLATSHADE )
	{
		illum += g_shadelight * 0.8;
	}
	else
	{
		float r;
		lightcos = DotProduct( normal, g_blightvec[bone] ); // -1 colinear, 1 opposite

		if ( lightcos > 1.0 )
			lightcos = 1;

		illum += g_shadelight;

		r = g_lambert;
		if ( r <= 1.0 ) r = 1.0;

		lightcos = ( lightcos + ( r - 1.0 ) ) / r; 		// do modified hemispherical lighting
		if ( lightcos > 0.0 )
		{
			illum -= g_shadelight * lightcos;
		}
		if ( illum <= 0 )
			illum = 0;
	}

	if ( illum > 255 )
		illum = 255;
	*lv = illum / 255.0;	// Light from 0 to 1.0
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::Chrome( int * pchrome, int bone, vec3_t normal )
{
	float n;

	if ( g_chromeage[bone] != g_smodels_total )
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		vec3_t chromeupvec;		// g_chrome t vector in world reference frame
		vec3_t chromerightvec;	// g_chrome s vector in world reference frame
		vec3_t tmp;				// vector pointing at bone in world reference frame
		VectorScale( m_origin, -1, tmp );
		tmp[0] += g_bonetransform[bone][0][3];
		tmp[1] += g_bonetransform[bone][1][3];
		tmp[2] += g_bonetransform[bone][2][3];
		VectorNormalize( tmp );
		CrossProduct( tmp, g_vright, chromeupvec );
		VectorNormalize( chromeupvec );
		CrossProduct( tmp, chromeupvec, chromerightvec );
		VectorNormalize( chromerightvec );

		VectorIRotate( chromeupvec, g_bonetransform[bone], g_chromeup[bone] );
		VectorIRotate( chromerightvec, g_bonetransform[bone], g_chromeright[bone] );

		g_chromeage[bone] = g_smodels_total;
	}

	// calc s coord
	n = DotProduct( normal, g_chromeright[bone] );
	pchrome[0] = ( n + 1.0 ) * 32; // FIX: make this a float

	// calc t coord
	n = DotProduct( normal, g_chromeup[bone] );
	pchrome[1] = ( n + 1.0 ) * 32; // FIX: make this a float
}

//-----------------------------------------------------------------------------
// Purpose: Set some global variables based on entity position
//-----------------------------------------------------------------------------
void StudioRender::SetupLighting( void )
{
	int i;
	g_ambientlight = 32;
	g_shadelight = 192;

	g_lightvec[0] = 0;
	g_lightvec[1] = 0;
	g_lightvec[2] = -1.0;

	g_lightcolor[0] = g_viewerSettings.lColor[0];
	g_lightcolor[1] = g_viewerSettings.lColor[1];
	g_lightcolor[2] = g_viewerSettings.lColor[2];

	// TODO: only do it for bones that actually have textures
	for ( i = 0; i < m_pstudiohdr->numbones; i++ )
	{
		VectorIRotate( g_lightvec, g_bonetransform[i], g_blightvec[i] );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Based on the body part, figure out which mesh it should be using.
//-----------------------------------------------------------------------------
void StudioRender::SetupModel( int bodypart )
{
	int index;

	if ( bodypart > m_pstudiohdr->numbodyparts )
	{
		// Con_DPrintf ("StudioRender::SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}

	mstudiobodyparts_t  * pbodypart = ( mstudiobodyparts_t * )( ( byte * )m_pstudiohdr + m_pstudiohdr->bodypartindex ) + bodypart;

	index = m_bodynum / pbodypart->base;
	index = index % pbodypart->nummodels;

	m_pmodel = ( mstudiomodel_t * )( ( byte * )m_pstudiohdr + pbodypart->modelindex ) + index;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::DrawModel( void )
{
	int i;

	if ( !m_pstudiohdr )
		return;

	g_smodels_total++; // render data cache cookie

	g_pxformverts = &g_xformverts[0];
	g_pvlightvalues = &g_lightvalues[0];

	if ( m_pstudiohdr->numbodyparts == 0 )
		return;

	glPushMatrix();

	glTranslatef( m_origin[0],  m_origin[1],  m_origin[2] );

	glRotatef( m_angles[1],  0, 0, 1 );
	glRotatef( m_angles[0],  0, 1, 0 );
	glRotatef( m_angles[2],  1, 0, 0 );

	SetUpBones( );

	SetupLighting( );

	for ( i = 0 ; i < m_pstudiohdr->numbodyparts ; i++ )
	{
		SetupModel( i );
		if ( g_viewerSettings.transparency > 0.0f )
			DrawPoints( );
	}

	// draw bones
	if ( g_viewerSettings.showBones && !g_viewerSettings.use3dfx )
	{
		mstudiobone_t * pbones = ( mstudiobone_t * )( ( byte * ) m_pstudiohdr + m_pstudiohdr->boneindex );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );

		for ( i = 0; i < m_pstudiohdr->numbones; i++ )
		{
			if ( pbones[i].parent >= 0 )
			{
				glPointSize( 3.0f );
				glColor3f( 1, 0.7f, 0 );
				glBegin( GL_LINES );
				glVertex3f( g_bonetransform[pbones[i].parent][0][3], g_bonetransform[pbones[i].parent][1][3], g_bonetransform[pbones[i].parent][2][3] );
				glVertex3f( g_bonetransform[i][0][3], g_bonetransform[i][1][3], g_bonetransform[i][2][3] );
				glEnd();

				glColor3f( 0, 0, 0.8f );
				glBegin( GL_POINTS );
				if ( pbones[pbones[i].parent].parent != -1 )
					glVertex3f( g_bonetransform[pbones[i].parent][0][3], g_bonetransform[pbones[i].parent][1][3], g_bonetransform[pbones[i].parent][2][3] );
				glVertex3f( g_bonetransform[i][0][3], g_bonetransform[i][1][3], g_bonetransform[i][2][3] );
				glEnd();
			}
			else
			{
				// draw parent bone node
				glPointSize( 5.0f );
				glColor3f( 0.8f, 0, 0 );
				glBegin( GL_POINTS );
				glVertex3f( g_bonetransform[i][0][3], g_bonetransform[i][1][3], g_bonetransform[i][2][3] );
				glEnd();
			}
		}

		glPointSize( 1.0f );
	}

	glPopMatrix();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void StudioRender::DrawPoints( void )
{
	int					i, j;
	mstudiomesh_t	*	pmesh;
	byte		*		pvertbone;
	byte		*		pnormbone;
	vec3_t		*		pstudioverts;
	vec3_t		*		pstudionorms;
	mstudiotexture_t	* ptexture;
	float 		*		av;
	float		*		lv;
	float				lv_tmp;
	short		*		pskinref;

	// TOMAS: MDL v6
	mstudiomodeldata_t	* pmodeldata;
	// TOMAS: MDL v6
	pmodeldata = ( mstudiomodeldata_t * )( ( byte * )m_pstudiohdr + m_pmodel->modeldataindex );

	pvertbone = ( ( byte * )m_pstudiohdr + m_pmodel->vertinfoindex );
	pnormbone = ( ( byte * )m_pstudiohdr + m_pmodel->norminfoindex );
	ptexture = ( mstudiotexture_t * )( ( byte * )m_ptexturehdr + m_ptexturehdr->textureindex );

	pmesh = ( mstudiomesh_t * )( ( byte * )m_pstudiohdr + m_pmodel->meshindex );

	// TOMAS: MDL v6
	pstudioverts = ( vec3_t * )( ( byte * )m_pstudiohdr + pmodeldata->vertindex );
	pstudionorms = ( vec3_t * )( ( byte * )m_pstudiohdr + pmodeldata->normindex );

	pskinref = ( short * )( ( byte * )m_ptexturehdr + m_ptexturehdr->skinindex );
	if ( m_skinnum != 0 && m_skinnum < m_ptexturehdr->numskinfamilies )
		pskinref += ( m_skinnum * m_ptexturehdr->numskinref );

	for ( i = 0; i < m_pmodel->numverts; i++ )
	{
		//vec3_t tmp;
		//VectorScale (pstudioverts[i], 12, tmp);
		VectorTransform( pstudioverts[i], g_bonetransform[pvertbone[i]], g_pxformverts[i] );
	}

	if ( g_viewerSettings.transparency < 1.0f )
	{
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}

//
// clip and draw all triangles
//

	lv = ( float * )g_pvlightvalues;
	for ( j = 0; j < m_pmodel->nummesh; j++ )
	{
		int flags;
		flags = ptexture[pskinref[pmesh[j].skinref]].flags;
		for ( i = 0; i < pmesh[j].numnorms; i++, lv += 3, pstudionorms++, pnormbone++ )
		{
			Lighting( &lv_tmp, *pnormbone, flags, ( float * )pstudionorms );

			// FIX: move this check out of the inner loop
			if ( flags & STUDIO_NF_CHROME )
				Chrome( g_chrome[( float( * )[3] )lv - g_pvlightvalues], *pnormbone, ( float * )pstudionorms );

			lv[0] = lv_tmp * g_lightcolor[0];
			lv[1] = lv_tmp * g_lightcolor[1];
			lv[2] = lv_tmp * g_lightcolor[2];
		}
	}

	// glCullFace(GL_FRONT);

	for ( j = 0; j < m_pmodel->nummesh; j++ )
	{
		float ss, st;
		mstudiotrivert_t * ptricmds;

		pmesh = ( mstudiomesh_t * )( ( byte * )m_pstudiohdr + m_pmodel->meshindex ) + j;
		ptricmds = ( mstudiotrivert_t * )( ( byte * )m_pstudiohdr + pmesh->triindex );

		ss = 1.0f / ( float )ptexture[pskinref[pmesh->skinref]].width;
		st = 1.0f / ( float )ptexture[pskinref[pmesh->skinref]].height;

		//glBindTexture( GL_TEXTURE_2D, ptexture[pskinref[pmesh->skinref]].index );
		glBindTexture( GL_TEXTURE_2D, pskinref[pmesh->skinref] + 3 );

		// TOMAS: MDL v6

		glBegin( GL_TRIANGLES );

		if ( ptexture[pskinref[pmesh->skinref]].flags & STUDIO_NF_CHROME )
		{
			for ( i = 0; i < pmesh->numtris * 3; i++, ptricmds++ )
			{
				// FIX: put these in as integer coords, not floats
				glTexCoord2f( g_chrome[ptricmds->normindex][0] * ss,
					g_chrome[ptricmds->normindex][1] * st );

				lv = g_pvlightvalues[ ptricmds->normindex ];
				glColor4f( lv[0], lv[1], lv[2], g_viewerSettings.transparency );

				av = g_pxformverts[ptricmds->vertindex];
				glVertex3f( av[0], av[1], av[2] );
			}

		}
		else
		{
			for ( i = 0; i < pmesh->numtris * 3; i++, ptricmds++ )
			{
				// FIX: put these in as integer coords, not floats
				glTexCoord2f( ptricmds->s * ss, ptricmds->t * st );

				lv = g_pvlightvalues[ptricmds->normindex];
				glColor4f( lv[0], lv[1], lv[2], g_viewerSettings.transparency );

				av = g_pxformverts[ ptricmds->vertindex ];
				glVertex3f( av[0], av[1], av[2] );
			}
		}

		glEnd();
	}
}
