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

#include "fileout.h"

#include <stdio.h>
#include <stdlib.h>

#include "shared.h"

#include "safestr.h"
#include "pathutils.h"
#include "filesys.h"

#include "mathlib.h"
#include "studio.h"



#define MAX_MSG 1024

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool FilePrint( char * file, char * fmt, ... )
{
	char buffer[ MAX_MSG ];

	// Put the message in a buffer
	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );

	FILE * pFile = fopen( file, "a" );

	if ( !pFile )
	{
		return false;
	}

	fprintf( pFile, "%s", buffer );
	fclose( pFile );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool FileClean( char * file )
{
	FILE * pFile = fopen( file, "w" );

	if ( !pFile )
	{
		return false;
	}

	fclose( pFile );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool LogPrint( FILE * pFile, char * fmt, ... )
{
	char buffer[ MAX_MSG ];

	// Put the message in a buffer
	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, sizeof( buffer ), fmt, args );
	va_end( args );

	bool bResult = true;


	// Write app title...

	static bool bFirstPrint = true;
	if ( bFirstPrint )
	{
		bFirstPrint = false;

		MakeLine( stdout, '-' );
		WriteTitle( stdout );
		MakeLine( stdout, '-' );

	}

	// If file is specified we will only print message to it
	if ( pFile != stdout && pFile != stderr && pFile != NULL )
	{
		fprintf( pFile, "%s", buffer );
		return bResult;
	}

	// Otherwise...

	// Print to console?

	if ( !pFile )
	{
		pFile = stdout;
	}

	// Print if output not suppresed
	if ( !( pFile == stdout && g_bSilent ) )
	{
		fprintf( pFile, "%s", buffer );
	}

	// Print to log file?

	// Only if log file name initialized and log output not suppressed
	if ( g_szLogFileName[ 0 ] && g_bEnableLog )
	{
		bResult = FilePrint( g_szLogFileName, "%s", buffer );
	}

	return bResult;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool LogClean( void )
{
	if ( !g_bEnableLog )
	{
		return false;
	}

	return FileClean( g_szLogFileName );
}

//-----------------------------------------------------------------------------
// Purpose: Print decompiler header
//-----------------------------------------------------------------------------
void WriteTitle( FILE * pFile )
{
	LogPrint( pFile, "%s\n", g_szAppTitle );
	LogPrint( pFile, "%s\n", g_szAppCredits );
	LogPrint( pFile, "%s\n", g_szAppContact );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#define LINE_SIZE 80
void MakeLine( FILE * pFile, char cSymbol /*= '='*/ )
{
	char szLine[ LINE_SIZE ];
	memset( szLine, cSymbol, LINE_SIZE );
	szLine[ LINE_SIZE - 1 ] = 0;

	LogPrint( pFile, "%s\n", szLine );
}

//-----------------------------------------------------------------------------
// Purpose: Build application title strings
//-----------------------------------------------------------------------------
void BuildTitleStrings( void )
{
	snprintf( g_szAppTitle, sizeof( g_szAppTitle ),
		"GoldSrc MDL v%d Decompiler %s (%s %s)",
		STUDIO_VERSION, MDLDEC_VERSION, __DATE__, __TIME__ );
	snprintf( g_szAppCredits, sizeof( g_szAppCredits ),
		"Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])" );
	snprintf( g_szAppContact, sizeof( g_szAppContact ),
		"E-mail: %s", "slavotinek@gmail.com" );
}
