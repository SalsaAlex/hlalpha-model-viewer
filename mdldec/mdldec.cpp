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

#include <stdio.h>
#include <stdlib.h>

#include "safestr.h"
#include "pathutils.h"
#include "filesys.h"

#include "mathlib.h"
#include "studio.h"
#include "StudioModel.h"

#include "fileout.h"
#include "qcfile.h"
#include "smdfile.h"
#include "texture.h"

#include "shared.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void PrintHelpLine( const char * cszArg, const char * cszDesc )
{
	LogPrint( stdout, "    %-14s\t%s\n", cszArg, cszDesc );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void PrintHelp( void )
{
	LogPrint( stdout, "Usage: mdl6dec.exe <path\\file.mdl> "
	          "[-p <destpath>] [-e] [-n] [-s]\n\n" );
	LogPrint( stdout, "Options:\n" );
	PrintHelpLine( "<path\\file.mdl>", "Input file" );
	PrintHelpLine( "-p <destpath>", "Destination path" );
	PrintHelpLine( "-e", "Stop on first error" );
	PrintHelpLine( "-n", "Do not create log file" );
	PrintHelpLine( "-s", "Silent mode (print only error messages)" );
}

//-----------------------------------------------------------------------------
// Purpose: Check program arguments
//-----------------------------------------------------------------------------
int CheckArgs( int argc, char * argv[] )
{
	// At least one parameter (file name)
	if ( argc < 2 )
	{
		PrintHelp();
		g_bEndOnError = true;
		return -1;
	}

	strlcpy( g_szInFullName, argv[ 1 ], sizeof( g_szInFullName ) );

	// Get input file name
	g_szInName = SkipPath( g_szInFullName );
	// Get input/output path
	GetPath( g_szInFullName, g_szPath );

	bool bUnkArg = false;

	char * pCurArg;
	int nParCnt = 2;
	while ( nParCnt < argc && !bUnkArg )
	{
		pCurArg = argv[ nParCnt ];
		if ( ( pCurArg[ 0 ] != '-' && pCurArg[ 0 ] != '/' ) || strlen( pCurArg ) != 2 )
		{
			bUnkArg = true;
			break;
		}

		switch ( pCurArg[ 1 ] )
		{
		case 'p':
		case 'P':
			if ( nParCnt + 1 == argc )
			{
				LogPrint( stderr, "ERROR: Path expected after \"%s\"\n", pCurArg );
				// TOMAS: Fatal error - end now!
				PrintHelp();
				g_bEndOnError = true;
				return ERR_MISSING_ARG;
			}

			nParCnt++;

			strlcpy( g_szPath, argv[ nParCnt ], sizeof( g_szPath ) );

			// Create output directory

			if ( !DirExists( g_szPath ) )
				CreateDir( g_szPath );

			break;
		case 'e':
		case 'E':
			g_bEndOnError = true;
			break;
		case 'n':
		case 'N':
			g_bEnableLog = false;
			break;
		case 's':
		case 'S':
			g_bSilent = true;
			break;
		case '?':
		case 'h':
		case 'H':
			PrintHelp();
			// TOMAS: HACK: We need to end now (by forcing "end on error")
			g_bEndOnError = true;
			return -1;
		default:
			bUnkArg = true;
			break;
		}

		nParCnt++;
	}

	if ( bUnkArg )
	{
		LogPrint( stderr, "ERROR: Unknown parameter \"%s\"\n", pCurArg );
		// TOMAS: Fatal error - end now!
		PrintHelp();
		g_bEndOnError = true;
		return ERR_UNK_ARG;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose:
// TOMAS: UNDONE: This is fugly solution
//-----------------------------------------------------------------------------
void HandleResultCode( int nResultCode )
{
	if ( nResultCode == 0 )
	{
		// No error
		return;
	}

	// Should we stop on error?
	if ( g_bEndOnError )
	{
		// TOMAS: UNDONE: Ugly
		exit( nResultCode );
	}

	g_nErrorCnt++;
}

//-----------------------------------------------------------------------------
// Purpose: Entry point for the console application
//-----------------------------------------------------------------------------
int main( int argc, char * argv[] )
{
	int nResult = 0;

	// Build application title strings
	BuildTitleStrings();

	// ARGUMENTS

	// Check arguments

	nResult = CheckArgs( argc, argv );
	HandleResultCode( nResult );


	// LOG FILE STUFF

	// Build log file name

	char szFileName[ MAX_PATH ];
	strlcpy( szFileName, g_szInName, sizeof( szFileName ) );
	StripExtension( szFileName, szFileName );
	snprintf( g_szLogFileName, sizeof( g_szLogFileName ), "%s%s%s.log",
		g_szPath, g_szPath[ 0 ] ? "/" : "", szFileName );

	// Clean log file

	LogClean();

	LogPrint( stdout, "MDL: %s\n", g_szInFullName );

	if ( !g_studioModel.LoadModel( g_szInFullName ) )
	{
		LogPrint( stderr, "ERROR: Cannot open \"%s\"\n", g_szInFullName );
		return -1;
	}


	// MODEL DECOMPILATION

	// Write QC Script file

	nResult = WriteQCScript();
	HandleResultCode( nResult );

	// Write SMD files

	// Reference files
	nResult = WriteRefSMDFiles();
	HandleResultCode( nResult );
	// Animation files
	nResult = WriteAnimSMDFiles();
	HandleResultCode( nResult );

	// Write BMP textures

	nResult = WriteTextures();
	HandleResultCode( nResult );

	LogPrint( stdout, "Done%s.\n", g_nErrorCnt > 0 ? " with error(s)" : "" );
	MakeLine( stdout, '-' );

	return nResult;
}
