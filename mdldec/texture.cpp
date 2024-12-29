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

#include "texture.h"

#include "shared.h"
#include "fileout.h"

#include "safestr.h"
#include "filesys.h"

#include "StudioModel.h"


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int WriteTextures( void )
{
	int nResult = 0;

	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	char szOutName[ MAX_PATH ];

	mstudiotexture_t * ptexture = ( mstudiotexture_t * )( pin + phdr->textureindex );

	for ( int nCnt = 0; nCnt < phdr->numtextures; nCnt++, ptexture++ )
	{
		snprintf( szOutName, sizeof( szOutName ), "%s%s%s", g_szPath, g_szPath[ 0 ] ? "/" : "", ptexture->name );
		LogPrint( stdout, "Texture: %s\n", szOutName );

		bool bResult = g_studioModel.WriteBitmap( nCnt, szOutName );

		if ( !bResult && g_bEndOnError )
		{
			LogPrint( stderr, "ERROR: Cannot write texture \"%s\"\n", szOutName );
			nResult = -2;
			break;
		}
	}

	return nResult;
}