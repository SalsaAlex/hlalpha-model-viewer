//=============================================================================
//
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: Model decompiler
//
//=============================================================================

#include "smdfile.h"

#include <stdio.h>
#include <stdlib.h>

#include "shared.h"
#include "fileout.h"

#include "safestr.h"
#include "pathutils.h"
#include "filesys.h"

#include "mathlib.h"
#include "studio.h"
#include "mdlmath.h"

#include "StudioModel.h"




// bone transformation matrix
float g_bonetransform[ MAXSTUDIOBONES ][ 3 ][ 4 ];
// reference bone data ([0]=position, [1]=rotation)
vec3_t g_refbonedata[ MAXSTUDIOBONES ][ 2 ];

//-----------------------------------------------------------------------------
// Purpose: 
// TOMAS: HACK: I hope this will work properly for all models
//-----------------------------------------------------------------------------
#define BONE_Z_FIX ( -Q_PI_F / 2 )
void SetUpRootBone( vec3_t pos, vec3_t angle )
{
	// Rotate bone

	angle[ 2 ] += BONE_Z_FIX;

	// Transform bone position

	vec3_t origpos;
	VectorCopy( pos, origpos );

	float cosz = cos( BONE_Z_FIX );
	float sinz = sin( BONE_Z_FIX );

	pos[ 0 ] = cosz * origpos[ 0 ] - sinz * origpos[ 1 ];
	pos[ 1 ] = sinz * origpos[ 0 ] + cosz * origpos[ 1 ];
	pos[ 2 ] = origpos[ 2 ];
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void SetUpBones( void )
{
	mstudiobone_t * pbones;
	mstudioseqdesc_t * pseqdesc;
	mstudioanim_t * panim;

	vec3_t pos;
	vec3_t rot;
	float bonematrix[3][4];
	vec4_t q;

	int m_sequence = 0;
	int m_frame = 0;

	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;


	pseqdesc = ( mstudioseqdesc_t * )( pin + phdr->seqindex ) + m_sequence;

	panim = g_studioModel.GetAnim( pseqdesc );

	pbones = ( mstudiobone_t * )( pin + phdr->boneindex );

	
	for ( int i = 0; i < phdr->numbones; i++ )
	{
		VectorCopy( g_refbonedata[ i ][ 0 ], pos );
		VectorCopy( g_refbonedata[ i ][ 1 ], rot );

		AngleQuaternion( rot, q );
		QuaternionMatrix( q, bonematrix );

		bonematrix[0][3] = pos[0];
		bonematrix[1][3] = pos[1];
		bonematrix[2][3] = pos[2];

		if ( pbones[i].parent == -1 )
		{
			memcpy( g_bonetransform[i], bonematrix, sizeof( float ) * 12 );
		}
		else
		{
			R_ConcatTransforms( g_bonetransform[pbones[i].parent], bonematrix, g_bonetransform[i] );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void WriteTriangles( FILE * pFile, mstudiomodel_t * pmodel )
{
	SetUpBones();

	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	mstudiotexture_t * ptexture = ( mstudiotexture_t * )( pin + phdr->textureindex );

	mstudiotexture_t * pcurtexture;


	//mstudiobone_t * pbone = (mstudiobone_t *)( pin + phdr->boneindex );

	mstudiomodeldata_t * pmodeldata = ( mstudiomodeldata_t * )( pin + pmodel->modeldataindex );
	vec3_t * pnorm = ( vec3_t * )( pin + pmodeldata->normindex );
	vec3_t * pvert = ( vec3_t * )( pin + pmodeldata->vertindex );

	byte * pvertbone = ( pin + pmodel->vertinfoindex );
	byte * pnormbone = ( pin + pmodel->norminfoindex );

	fprintf( pFile, "triangles\n" );

	mstudiomesh_t * pmesh = ( mstudiomesh_t * )( pin + pmodel->meshindex );

	for ( int nMeshCnt = 0; nMeshCnt < pmodel->nummesh; nMeshCnt++, pmesh++ )
	{
		// TOMAS: DELETE ME
		//LogPrint( stdout, "Mesh %d/%d\n", nMeshCnt, pmodel->nummesh );

		// TEXTURES

		short * pskinref;
		pskinref = ( short * )( pin + phdr->skinindex );

		pcurtexture = ptexture + pskinref[pmesh->skinref];

		// Texture "scale" factor
		float ss = 1.0f / ( float )pcurtexture->width;
		float st = 1.0f / ( float )pcurtexture->height;

		// VERTICIES

		mstudiotrivert_t * ptrivert = ( mstudiotrivert_t * )( pin + pmesh->triindex );

		for ( int nTriCnt = 0; nTriCnt < pmesh->numtris * 3; nTriCnt++, ptrivert++ )
		{
			if ( nTriCnt % 3 == 0 )
			{
				fprintf( pFile, "%s\n", pcurtexture->name );
			}

			// Transform vertex position
			vec3_t vecin, vecpos, vecnorm;
			VectorCopy( pvert[ ptrivert->vertindex ], vecin );
			VectorTransform( vecin,
				g_bonetransform[ pvertbone[ ptrivert->vertindex ] ], vecpos );

			// Transform vertex normal
			VectorCopy( pnorm[ ptrivert->normindex ], vecin );
			VectorRotate( vecin,
				g_bonetransform[ pnormbone[ ptrivert->normindex ] ], vecnorm );
			VectorNormalize( vecnorm );

			int nBone = pvertbone[ ptrivert->vertindex ];

			fprintf( pFile, "%3d %f %f %f %f %f %f %f %f\n",
				nBone,

				vecpos[ 0 ],
				vecpos[ 1 ],
				vecpos[ 2 ],

				vecnorm[ 0 ],
				vecnorm[ 1 ],
				vecnorm[ 2 ],

				( float )ptrivert->s * ss,
				1.0f - ( float )ptrivert->t * st );
		}
	}

	fprintf( pFile, "end\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void WriteBones( FILE * pFile )
{
	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	mstudiobone_t * pbone = ( mstudiobone_t * )( pin + phdr->boneindex );

	fprintf( pFile, "nodes\n" );

	for ( int nCnt = 0; nCnt < phdr->numbones; nCnt++, pbone++ )
	{
		char szBoneName[ BONE_NAME_LEN ];
		GetBoneName( nCnt, szBoneName );
		fprintf( pFile, "%3d \"%s\" %d\n",
			nCnt,
			szBoneName,
			pbone->parent );
	}

	fprintf( pFile, "end\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int g_lastIndex[ MAXSTUDIOBONES ][ 2 ];
void WriteSkeleton( FILE * pFile, mstudioseqdesc_t * pseqdesc, bool bReference )
{
	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	// Set all rotation/position indices to 0
	memset( g_lastIndex, 0, sizeof( g_lastIndex ) );

	fprintf( pFile, "skeleton\n" );

	// Only write first frame to reference file
	int nFrameNum = bReference ? 1 : pseqdesc->numframes;

	for ( int nFrameCnt = 0; nFrameCnt < nFrameNum; nFrameCnt++ )
	{
		mstudiobone_t * pbone = ( mstudiobone_t * )( pin + phdr->boneindex );

		fprintf( pFile, "time %d\n", nFrameCnt );
		for ( int nBoneCnt = 0; nBoneCnt < phdr->numbones; nBoneCnt++, pbone++ )
		{
			mstudioanim_t * panim = ( mstudioanim_t * )( pin + pseqdesc->animindex ) + nBoneCnt;

			// Write bone positions

			mstudiobonepos_t * pbonepos;
			pbonepos = ( mstudiobonepos_t * )( pin + panim->posindex );

			int lastposindex = g_lastIndex[ nBoneCnt ][ 0 ];
			if ( ( lastposindex + 1 ) < panim->numpos &&
			        ( pbonepos + lastposindex + 1 )->frame == nFrameCnt )
			{
				lastposindex++;
			}

			pbonepos += lastposindex;

			vec3_t pos;
			VectorCopy( pbonepos->pos, pos );
		
			g_lastIndex[ nBoneCnt ][ 0 ] = lastposindex;

			// Write bone rotations

			mstudiobonerot_t * pbonerot;
			pbonerot = ( mstudiobonerot_t * )( pin + panim->rotindex );

			int lastrotindex = g_lastIndex[ nBoneCnt ][ 1 ];
			if ( ( lastrotindex + 1 ) < panim->numrot &&
				( pbonerot + lastrotindex + 1 )->frame == nFrameCnt )
			{
				lastrotindex++;
			}

			pbonerot += lastrotindex;

			vec3_t rot;
			VectorCopy( pbonerot->angle, rot );
			VectorScalMul( rot, STUDIO_TO_RAD );

			g_lastIndex[ nBoneCnt ][ 1 ] = lastrotindex;

			if ( pbone->parent == -1 )
			{
				SetUpRootBone( pos, rot );
			}

			fprintf( pFile, "%3d\t%f %f %f %f %f %f\n",
				nBoneCnt,

				pos[ 0 ],
				pos[ 1 ],
				pos[ 2 ],
					 
				rot[ 0 ],
				rot[ 1 ],
				rot[ 2 ]
			);

			// Save reference data for vertex transformation
			if ( bReference )
			{
				VectorCopy( rot, g_refbonedata[ nBoneCnt ][ 1 ] );
				VectorCopy( pos, g_refbonedata[ nBoneCnt ][ 0 ] );
			}
		}
	}

	fprintf( pFile, "end\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int WriteSequence( mstudioseqdesc_t * pseqdesc, mstudiomodel_t * pmodel )
{
	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	// Is this reference file?
	bool bReference = ( pmodel != NULL );

	char szOutName[ MAX_PATH ];

	char * szFileName = pmodel ? pmodel->name : pseqdesc->label;
	snprintf( szOutName, sizeof( szOutName ), "%s%s%s.smd", g_szPath, g_szPath[ 0 ] ? "/" : "", szFileName );

	LogPrint( stdout, "%s: %s\n", pmodel ? "Reference" : "Sequence", szOutName );

	FILE * pFile = fopen( szOutName, "w" );
	if ( !pFile )
	{
		LogPrint( stderr, "ERROR: Cannot write SMD file \"%s\"\n", szOutName );
		return -1;
	}

	fprintf( pFile, "version %d\n", 1 );

	// Write bones (index, name, parent)

	WriteBones( pFile );

	// Write skeleton (bone animations: positions and rotations)

	WriteSkeleton( pFile, pseqdesc, bReference );

	// Write extra info for reference frames

	if ( bReference )
	{
		WriteTriangles( pFile, pmodel );
	}

	fclose( pFile );

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Write animations to (SMD) Studio Model Data files
//-----------------------------------------------------------------------------
int WriteAnimSMDFiles( void )
{
	int nResult = 0;

	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	mstudioseqdesc_t * pseqdesc;
	pseqdesc = ( mstudioseqdesc_t * )( pin + phdr->seqindex );

	for ( int nCnt = 0; nCnt < phdr->numseq; nCnt++, pseqdesc++ )
	{
		nResult = WriteSequence( pseqdesc, NULL );

		if ( nResult < 0 && g_bEndOnError )
		{
			break;
		}
	}

	return nResult;
}

//-----------------------------------------------------------------------------
// Purpose: Write reference data to (SMD) Studio Model Data files
//-----------------------------------------------------------------------------
int WriteRefSMDFiles( void )
{
	int nResult = 0;

	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	mstudioseqdesc_t * pseqdesc;
	pseqdesc = ( mstudioseqdesc_t * )( pin + phdr->seqindex );

	mstudiobodyparts_t * pbodypart = ( mstudiobodyparts_t * )( pin + phdr->bodypartindex );

	for ( int nCnt = 0; nCnt < phdr->numbodyparts; nCnt++, pbodypart++ )
	{
		mstudiomodel_t * pmodel = ( mstudiomodel_t * )( pin + pbodypart->modelindex );

		for ( int nMdlCnt = 0; nMdlCnt < pbodypart->nummodels; nMdlCnt++, pmodel++ )
		{
			nResult = WriteSequence( pseqdesc, pmodel );

			if ( nResult < 0 && g_bEndOnError )
			{
				break;
			}
		}
	}

	return nResult;
}
