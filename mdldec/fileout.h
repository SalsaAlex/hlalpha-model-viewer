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

#ifndef FILEOUT_H
#define FILEOUT_H

#include <stdio.h>

bool FilePrint( char * file, char * fmt, ... );
bool FileClean( char * file );
void MakeLine( FILE * pFile, char cSymbol = '=' );
bool LogPrint( FILE * pFile, char * fmt, ... );
bool LogClean( void );
void WriteTitle( FILE * pFile );
void BuildTitleStrings( void );

#endif // FILEOUT_H
