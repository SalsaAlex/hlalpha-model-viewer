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

#include "shared.h"

StudioModel g_studioModel;

char g_szAppTitle[ 256 ] = { 0 };
char g_szAppCredits[ 256 ] = { 0 };
char g_szAppContact[ 256 ] = { 0 };

char g_szInFullName[ MAX_PATH ] = { 0 };
char * g_szInName;
char g_szPath[ MAX_PATH ] = { 0 };

char g_szLogFileName[ MAX_PATH ] = { 0 };
bool g_bEnableLog = true;

bool g_bSilent = false;

bool g_bEndOnError = false;
int g_nErrorCnt = 0;
