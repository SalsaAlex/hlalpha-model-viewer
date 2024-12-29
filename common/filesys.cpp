//=============================================================================
//
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: Filesystem utilities
//
//=============================================================================

#include "filesys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safestr.h"

#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

/*#ifdef WIN32
#include "windows.h"
#endif*/



//-----------------------------------------------------------------------------
// Purpose: Create directory
// TOMAS: CHECK THIS: Is this safe (it should work on both *nix and Windows)
// Or should we use WinAPI on Windows?
//-----------------------------------------------------------------------------
int CreateDir( const char * cszDirName )
{
	const char * cszMkDirCmd = "mkdir";
	char szCmd[ MAX_PATH + sizeof( cszMkDirCmd ) + 1 ];

	snprintf( szCmd, sizeof( szCmd ), "%s \"%s\"", cszMkDirCmd, cszDirName );

	return system( szCmd );
}

//-----------------------------------------------------------------------------
// Purpose: Checks if directory exists
//-----------------------------------------------------------------------------
bool DirExists( const char * cszPath )
{

	if ( _access( cszPath, 0 ) == 0 )
	{
		struct stat status;
		stat( cszPath, &status );

		return ( status.st_mode & S_IFDIR ) != 0;
	}

	return false;
}
