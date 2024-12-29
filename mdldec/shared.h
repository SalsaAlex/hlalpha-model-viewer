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

#ifndef SHARED_H
#define SHARED_H

#include "StudioModel.h"

#include "filesys.h"
#include "safestr.h"


#define MDLDEC_VERSION	"0.4a BETA1"

extern StudioModel g_studioModel;

extern char g_szAppTitle[ 256 ];
extern char g_szAppCredits[ 256 ];
extern char g_szAppContact[ 256 ];

extern char g_szInFullName[ MAX_PATH ];
extern char * g_szInName;
extern char g_szPath[ MAX_PATH ];

extern char g_szLogFileName[ MAX_PATH ];
extern bool g_bEnableLog;

extern bool g_bSilent;

extern bool g_bEndOnError;
extern int g_nErrorCnt;

// Error codes
enum eErrCode
{
    ERR_NO_ERROR = 0,
    ERR_HELP,
    ERR_MISSING_ARG,
    ERR_UNK_ARG,
    ERR_PATH_EXPECTED,
    ERR_QC,
    ERR_SMD,
    ERR_BMP,
    ERR_UNKNOWN,
};

#define BONE_NAME_LEN 32

inline void GetBoneName( int nBoneId, char * szOutName )
{
	studiohdr_t * phdr = g_studioModel.getStudioHeader();
	byte * pin = ( byte * )phdr;

	szOutName[ 0 ] = 0;

	if ( !phdr )
	{
		return;
	}

	if ( nBoneId < 0 || nBoneId >= phdr->numbones )
	{
		return;
	}

	mstudiobone_t * pbone = ( mstudiobone_t * )( pin + phdr->boneindex ) + nBoneId;

	if ( pbone->name[ 0 ] )
	{
		strncpy( szOutName, pbone->name, BONE_NAME_LEN );
	}
	else
	{
		snprintf( szOutName, BONE_NAME_LEN, "unnamed%03d", nBoneId );
	}
}

#endif // SHARED_H
