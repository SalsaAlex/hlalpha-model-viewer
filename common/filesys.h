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

#ifndef FILESYS_H
#define FILESYS_H

#ifdef WIN32
#include "windows.h"
#endif

#if !defined( MAX_PATH )
#define MAX_PATH 256
#endif

int CreateDir( const char * cszDirName );
bool DirExists( const char * cszPath );

#endif // FILESYS_H
