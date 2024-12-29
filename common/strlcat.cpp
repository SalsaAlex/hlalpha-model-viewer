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
// Purpose: Appends src to string dst of size siz (unlike strncat, siz is the
//			full size of dst, not space left).  At most siz-1 characters
//			will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
// Returns: strlen(src) + MIN(siz, strlen(initial dst)).
//			If retval >= siz, truncation occurred.
//-----------------------------------------------------------------------------
size_t strlcat( char * dst, const char * src, size_t siz )
{
	assert( siz > 4 );

	char * d = dst;
	const char * s = src;
	size_t n = siz;
	size_t dlen;

	// Find the end of dst and adjust bytes left but don't go past end
	while ( n-- != 0 && *d != '\0' )
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if ( n == 0 )
		return( dlen + strlen( s ) );
	while ( *s != '\0' )
	{
		if ( n != 1 )
		{
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	// count does not include NUL
	return ( dlen + ( s - src ) );
}
