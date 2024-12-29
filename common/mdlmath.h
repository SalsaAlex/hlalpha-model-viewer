//=============================================================================
//
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: 
//
//=============================================================================

#ifndef MDLMATH_H
#define MDLMATH_H

#include "mathlib.h"

#ifndef MIN
#define MIN( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#endif

#ifndef MAX
#define MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#endif

#ifndef BOUND
#define BOUND( min, x, max ) ( ( min ) >= ( max ) ? \
	( min ) : ( x ) < ( min ) ? ( min ) : ( x ) > ( max ) ? ( max ) : ( x ) )
#endif

inline void VectorScalMul( vec3_t vec, float mul )
{
	vec[ 0 ] *= mul;
	vec[ 1 ] *= mul;
	vec[ 2 ] *= mul;
}

#endif // MDLMATH_H


