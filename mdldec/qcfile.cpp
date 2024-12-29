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

#include "qcfile.h"

#include <stdio.h>
#include <stdlib.h>

#include "shared.h"
#include "fileout.h"

#include "safestr.h"
#include "pathutils.h"
#include "filesys.h"

#include "mathlib.h"
#include "studio.h"

#include "StudioModel.h"


const char * g_cszMovFlags[] =
{
	"",
	"X",
	"Y",
	"Z",
	"XR",
	"YR",
	"ZR",
	"LX",
	"LY",
	"LZ",
	"AX",
	"AY",
	"AZ",
	"AXR",
	"AYR",
	"AZR",
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void QC_WriteHeaderBegin( FILE * pFile )
{
	LogPrint( pFile, "/*\n" );
	MakeLine( pFile );
	LogPrint( pFile, "\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void QC_WriteHeaderEnd( FILE * pFile )
{
	LogPrint( pFile, "\n" );
	MakeLine( pFile );
	LogPrint( pFile, "*/\n" );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void QC_WriteStringKey( FILE * pFile, char * szKey, char * szValue )
{
	fprintf( pFile, "$%s \"%s\"\n", szKey, szValue );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void QC_WriteFloatKey( FILE * pFile, char * szKey, float fValue )
{
	fprintf( pFile, "$%s %2.1f\n", szKey, fValue );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void QC_WriteKey( FILE * pFile, char * szKey )
{
	fprintf( pFile, "$%s\n", szKey );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
#define MOTION_FLAGS_LEN 32
void MotionFlags2Str( int nFlags, char * szOut, bool bAllowMultiple )
{
	szOut[ 0 ] = 0;

	nFlags = ( nFlags & STUDIO_TYPES );

	if ( nFlags == 0 )
	{
		return;
	}

	int nFlagPos = 1;
	int nFlagMask = 1;
	int nFlagCnt = sizeof( g_cszMovFlags ) / sizeof( char * );

	while ( nFlagPos < nFlagCnt )
	{
		if ( ( nFlags & nFlagMask ) > 0 )
		{
			// Add divider
			if ( szOut[ 0 ] != 0 )
			{
				strlcat( szOut, " ", MOTION_FLAGS_LEN );
			}

			// Add flag
			strlcat( szOut, g_cszMovFlags[ nFlagPos ], MOTION_FLAGS_LEN );
			if ( !bAllowMultiple )
			{
				break;
			}
		}
		nFlagMask = ( nFlagMask << 1 );
		nFlagPos++;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WriteBodyparts( FILE * pFile )
{
	fprintf( pFile, "// %s\n", "reference mesh(es)" );

	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	mstudiobodyparts_t * pbodypart = ( mstudiobodyparts_t * )( pin + phdr->bodypartindex );

	for ( int nCnt = 0; nCnt < phdr->numbodyparts; nCnt++, pbodypart++ )
	{
		mstudiomodel_t * pmodel = ( mstudiomodel_t * )( pin + pbodypart->modelindex );
		if ( pbodypart->nummodels > 1 )
		{
			fprintf( pFile, "$bodygroup \"%s\"\n", pbodypart->name );
			fprintf( pFile, "{\n" );
			for ( int nMdlCnt = 0; nMdlCnt < pbodypart->nummodels; nMdlCnt++, pmodel++ )
			{
				fprintf( pFile, "studio \"%s\" reverse\n", pmodel->name );
			}
			fprintf( pFile, "}\n\n" );
		}
		else
		{
			fprintf( pFile, "$body \"%s\" \"%s\" reverse\n\n", pbodypart->name, pmodel->name );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WriteControllers( FILE * pFile )
{
	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	if ( phdr->numbonecontrollers <= 0 )
	{
		return;
	}

	fprintf( pFile, "// %s\n", "bone controller(s)" );

	mstudiobonecontroller_t * pbonecontroller;
	pbonecontroller = ( mstudiobonecontroller_t * )( pin + phdr->bonecontrollerindex );

	for ( int nCnt = 0; nCnt < phdr->numbonecontrollers; nCnt++, pbonecontroller++ )
	{
		char szBoneName[ BONE_NAME_LEN ];
		GetBoneName( pbonecontroller->bone, szBoneName );

		char szFlags[ MOTION_FLAGS_LEN ];
		MotionFlags2Str( pbonecontroller->type, szFlags, false );

		float fStart = pbonecontroller->start;
		float fEnd = pbonecontroller->end;
		if ( fStart > fEnd )
		{
			float fTemp = fStart;
			fStart = fEnd;
			fEnd = fTemp;
		}

		fprintf( pFile, "$controller %d \"%s\" %s %f %f\n",
			nCnt,
			szBoneName,
			szFlags,
			fStart,
			fEnd );	
	}

	fprintf( pFile, "\n" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void WriteSequences( FILE * pFile )
{
	fprintf( pFile, "// %s\n", "animation seqence(s)" );

	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	mstudioseqdesc_t * pseqdesc;
	pseqdesc = ( mstudioseqdesc_t * )( pin + phdr->seqindex );

	for ( int nCnt = 0; nCnt < phdr->numseq; nCnt++, pseqdesc++ )
	{
		fprintf( pFile, "$sequence \"%s\" \"%s\"",
			pseqdesc->label,
			pseqdesc->label );

		//fprintf( pFile, " (%d)", pseqdesc->motiontype );
		if ( pseqdesc->motiontype > 0 )
		{
			char szFlags[ MOTION_FLAGS_LEN ];
			MotionFlags2Str( pseqdesc->motiontype, szFlags, true );
			fprintf( pFile, " %s", szFlags );
		}

		fprintf( pFile, " fps %3.2f",
			pseqdesc->fps );

		if ( pseqdesc->flags & STUDIO_LOOPING )
		{
			fprintf( pFile, " loop" );
		}

		fprintf( pFile, "\n" );

		mstudioevent_t * pevent;
		pevent = ( mstudioevent_t * )( pin + pseqdesc->eventindex );

		byte * peventb = ( byte * )pevent;

		// TOMAS: DELETE ME
		//printf( "events: %d\n", pseqdesc->numevents );
		for ( int nEventCnt = 0; nEventCnt < pseqdesc->numevents; nEventCnt++, pevent++ )
		{
			fprintf( pFile, "{ event %d %d }\n", pevent->type, pevent->frame );
			// TOMAS: DELETE ME
			//printf( "{ event %d %d }\n", pevent->type, pevent->frame );
		}
	}

	fprintf( pFile, "\n" );
}

//-----------------------------------------------------------------------------
// Purpose: Write QC script file
//-----------------------------------------------------------------------------
int WriteQCScript( void )
{
	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	char szFileName[ MAX_PATH ];
	char szOutName[ MAX_PATH ];

	strlcpy( szFileName, g_szInName, sizeof( szFileName ) );
	StripExtension( szFileName, szFileName );
	snprintf( szOutName, sizeof( szOutName ), "%s%s%s.qc", g_szPath, g_szPath[ 0 ] ? "/" : "", szFileName );

	LogPrint( stdout, "QC script: %s\n", szOutName );

	FILE * pFile = fopen( szOutName, "w" );
	if ( !pFile )
	{
		LogPrint( stderr, "ERROR: Cannot write QC file \"%s\"\n", szOutName );
		return -1;
	}

	QC_WriteHeaderBegin( pFile );
	fprintf( pFile, "QC script generated by " );
	WriteTitle( pFile );
	fprintf( pFile, "Original internal name:\n\"%s\"\n", phdr->name );
	QC_WriteHeaderEnd( pFile );
	fprintf( pFile, "\n" );

	QC_WriteStringKey( pFile, "modelname", g_szInName );
	QC_WriteStringKey( pFile, "cd", ".\\" );
	QC_WriteStringKey( pFile, "cdtexture", ".\\" );
	QC_WriteFloatKey( pFile, "scale", 1.0f );
	QC_WriteKey( pFile, "cliptotextures" );
	fprintf( pFile, "\n" );

	WriteBodyparts( pFile );

	WriteControllers( pFile );

	WriteSequences( pFile );


	fprintf( pFile, "// %s\n", "End of QC script." );

	fclose( pFile );

	return 0;
}