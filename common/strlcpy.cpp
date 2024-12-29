//=============================================================================
//
// Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: String utilities
//
//=============================================================================

#include <sys/types.h>
#include <string.h>
#include <assert.h>

//-----------------------------------------------------------------------------
// Purpose: Copy src to string dst of size siz.  At most siz-1 characters
//			will be copied.  Always NUL terminates (unless siz == 0).
// Returns: strlen(src); if retval >= siz, truncation occurred.
//-----------------------------------------------------------------------------
size_t strlcpy( char * dst, const char * src, size_t siz )
{
	assert( siz > 4 );

	char * d = dst;
	const char * s = src;
	size_t n = siz;

	// Copy as many bytes as will fit
	if ( n != 0 )
	{
		while ( --n != 0 )
		{
			if ( ( *d++ = *s++ ) == '\0' )
				break;
		}
	}

	// Not enough room in dst, add NUL and traverse rest of src
	if ( n == 0 )
	{
		// NUL-terminate dst
		if ( siz != 0 )
			*d = '\0';
		while ( *s++ )
			;
	}

	// count does not include NUL
	return ( s - src - 1 );
}
