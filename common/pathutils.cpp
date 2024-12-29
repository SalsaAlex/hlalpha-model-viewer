//=============================================================================
//
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: Path utilities
//
//=============================================================================

#include "string.h"
#include "safestr.h"


//-----------------------------------------------------------------------------
// Purpose:
// TOMAS: UNSAFE
//-----------------------------------------------------------------------------
void StripExtension( char * in, char * out )
{
	char * dot;

	if ( !( dot = strrchr( in, '.' ) ) )
	{
		//strlcpy( out, in, sizeof( out ) );
		strcpy( out, in );
		return;
	}

	while ( * in && in != dot )
	{
		* out++ = * in++;
	}

	* out = 0;
}

//-----------------------------------------------------------------------------
// Purpose: If path doesn't have an extension or has a different extension,
//			append(!) specified extension. Extension should include the "."
// TOMAS: UNSAFE
//-----------------------------------------------------------------------------
void ForceExtension( char * path, char * extension )
{
	char * src;

	src = path + strlen( path ) - 1;

	while ( * src != '/' && src != path )
	{
		if ( * src-- == '.' )
		{
			StripExtension( path, path );
			//strlcat( path, extension, sizeof( path ) );
			strcat( path, extension );
			return;
		}
	}

	//strlcat( path, extension, sizeof( path ) );
	strcat( path, extension );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
char * SkipPath( char * pathname )
{
	char * last;

	last = pathname;
	while ( * pathname )
	{
		if ( * pathname == '/' || * pathname == '\\' )
			last = pathname + 1;
		pathname++;
	}
	return last;
}

//-----------------------------------------------------------------------------
// Purpose:
// TOMAS: UNSAFE
//-----------------------------------------------------------------------------
void GetPath( char * in, char * out )
{
	char * last = out;

	while ( *in )
	{
		if ( *in == '/' || * in == '\\' )
			last = out;
		*out++ = *in++;
	}
	*last = 0;
}
