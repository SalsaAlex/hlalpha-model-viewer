//=============================================================================
//
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: String utilities
//
//=============================================================================

#ifndef SAFESTR_H
#define SAFESTR_H

#include <sys/types.h>
#include "stdio.h"

size_t strlcat( char * dst, const char * src, size_t siz );
size_t strlcpy( char * dst, const char * src, size_t siz );

#define snprintf( buffer, size, format, ... ) \
	sprintf_s( buffer, size, format, ##__VA_ARGS__ )

#if !defined( vsnprintf )
#define vsnprintf( buffer, size, format, arg ) \
	vsnprintf_s( buffer, size, format, arg )
#endif

#endif // SAFESTR_H
