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

#ifndef PATHUTILS_H
#define PATHUTILS_H

void StripExtension( char * in, char * out );
void ForceExtension( char * path, char * extension );
char * SkipPath( char * pathname );
void GetPath( char * in, char * out );

#endif // PATHUTILS_H
